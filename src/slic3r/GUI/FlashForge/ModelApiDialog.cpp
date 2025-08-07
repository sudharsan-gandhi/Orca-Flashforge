#include "ModelApiDialog.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/Utils/Http.hpp"
#include "slic3r/GUI/Plater.hpp"
#include "slic3r/GUI/FFUtils.hpp"
#include "slic3r/GUI/FlashForge/MultiComHelper.hpp"
#include "slic3r/GUI/FlashForge/MultiComMgr.hpp"
#include <wx/base64.h>
#include <curl/curl.h>

namespace Slic3r {
namespace GUI {

wxDEFINE_EVENT(EVT_LOADED_IMAGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_FINISH_TASK, wxCommandEvent);
wxDEFINE_EVENT(EVT_UPDATE_ICON, wxCommandEvent);
wxDEFINE_EVENT(EVT_ERROR_MSG, wxCommandEvent);
wxDEFINE_EVENT(EVT_FINISH_SCORE, FinishScoreEvent);
wxDECLARE_EVENT(EVT_STORE_PROMO, wxCommandEvent);

std::string ModelApiDialog::m_dir_path = "";

QuestionDialog::QuestionDialog(wxWindow* parent) : FFRoundedWindow(parent)
{
    SetSize(FromDIP(320), FromDIP(198));
    SetBackgroundColour(wxColour("#333333"));
    auto title = new Label(this, Label::Body_14, _L("Image Upload Tips"));
    title->SetForegroundColour(*wxWHITE);
    auto info = new Label(this, Label::Body_12,
                  _L("It supports PNG, JPG, JPEG. Images should be no larger than 6MB with a minimum resolution of 128*128."));
    info->SetForegroundColour(*wxWHITE);
    auto text1   = new Label(this, Label::Body_12, _L("Simple background (preferably solid color)"));
    text1->SetForegroundColour(*wxWHITE);
    auto text2   = new Label(this, Label::Body_12, _L("No text included"));
    text2->SetForegroundColour(*wxWHITE);
    auto text3   = new Label(this, Label::Body_12, _L("Single model"));
    text3->SetForegroundColour(*wxWHITE);
    auto text4   = new Label(this, Label::Body_12, _L("The model should not be too small."));
    text4->SetForegroundColour(*wxWHITE);
    auto v_sizer = new wxBoxSizer(wxVERTICAL);
    v_sizer->AddSpacer(FromDIP(16));
    v_sizer->Add(title, 0, wxALIGN_CENTER, 0);
    v_sizer->AddSpacer(FromDIP(6));
    info->Wrap(FromDIP(282));
    v_sizer->Add(info, 0, wxLEFT | wxRIGHT, FromDIP(19));
    v_sizer->AddSpacer(FromDIP(12));
    auto h_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto v_sizer0 = new wxBoxSizer(wxVERTICAL);
    text1->Wrap(FromDIP(168));
    v_sizer0->Add(text1, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text2->Wrap(FromDIP(168));
    v_sizer0->Add(text2, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text3->Wrap(FromDIP(168));
    v_sizer0->Add(text3, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text4->Wrap(FromDIP(168));
    v_sizer0->Add(text4, 0, wxALL, 0);
    auto       text_height = FromDIP(6) * 3;
    wxCoord    w, h;
    wxMemoryDC dc;
    dc.SetFont(Label::Body_12);
    dc.GetMultiLineTextExtent(text1->GetLabel(), &w, &h);
    text_height += h;
    dc.GetMultiLineTextExtent(text2->GetLabel(), &w, &h);
    text_height += h;
    dc.GetMultiLineTextExtent(text3->GetLabel(), &w, &h);
    text_height += h;
    dc.GetMultiLineTextExtent(text4->GetLabel(), &w, &h);
    text_height += h;
    ScalableBitmap bmp(this, "question_tip_image", ToDIP(text_height));
    auto           img = new wxStaticBitmap(this, wxID_ANY, bmp.bmp());
    h_sizer->Add(img, 0, wxEXPAND | wxALL, 0);
    h_sizer->AddSpacer(FromDIP(16));
    h_sizer->Add(v_sizer0, 0, wxALL, 0);
    v_sizer->Add(h_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, FromDIP(19));
    v_sizer->AddSpacer(FromDIP(25));
    Layout();
    SetSizerAndFit(v_sizer);
}

ApiLoadingIcon::ApiLoadingIcon(wxDialog* parent) : wxEvtHandler()
{
    this->m_parent = parent;
    m_timer  = new wxTimer(this);
    Bind(wxEVT_TIMER, &ApiLoadingIcon::OnTimer, this);
    for (int i = 0; i < 4; i++) {
        auto           str = boost::format("api_loading_%1%") % (i + 1);
        ScalableBitmap bmp(parent, str.str(), 60);
        m_loadingIcons.emplace_back(std::move(bmp));
    }
}

void ApiLoadingIcon::paintInRect(wxGraphicsContext* gc, wxRect rect)
{
    wxBitmap& bmp = m_loadingIcons[m_loadingIdx].bmp();
    gc->DrawBitmap(bmp, rect.x, rect.y, rect.width, rect.height);
}

void ApiLoadingIcon::Loading(int interval)
{
    if (m_timer->IsRunning()) {
        m_timer->Stop();
    }
    m_loadingIdx  = 0;
    m_loadingTime = 0;
    m_timer->Start(interval);
}

void ApiLoadingIcon::End()
{
    if (m_timer->IsRunning()) {
        m_timer->Stop();
    }
}

bool ApiLoadingIcon::isLoading() { return m_timer->IsRunning(); }

void ApiLoadingIcon::OnTimer(wxTimerEvent& event)
{
    m_loadingTime++;
    m_loadingIdx = (m_loadingIdx + 1) % m_loadingIcons.size();
    this->QueueEvent(new wxCommandEvent(EVT_UPDATE_ICON));
}

ModelApiTask::ModelApiTask(wxEvtHandler* parent) : 
    wxEvtHandler(), m_sem(1) 
{
    m_parent = parent;
    m_isFinish.store(false);
}

void ModelApiTask::setThreadFunc(std::function<void()> func) 
{ 
    this->m_func = func; 
}

wxSemaphore& ModelApiTask::Sem() { 
    return m_sem; 
}

std::mutex& ModelApiTask::Lock()
{ 
    return m_lock; 
}

wxEvtHandler* ModelApiTask::Parent() { return m_parent; }

void ModelApiTask::safeFunc(std::function<void()> func) 
{ 
    std::lock_guard<std::mutex> lock(m_lock);
    if (!m_isFinish.load()) func();
}

std::atomic_bool& ModelApiTask::FinishLoop()
{ 
    return m_isFinish; 
}

void ModelApiTask::start() 
{
    std::thread([self = shared_from_this()]() {
        self->m_func();
        self->safeFunc([self]() {
            auto e = new wxCommandEvent(EVT_FINISH_TASK);
            //e->SetString(id);
            wxQueueEvent(self->m_parent, e);
        });
    }).detach();
}

bool ImageUploadPanel::judgeTransImage(wxString& path)
{
    if (path.IsEmpty()) {
        return false;
    }
    fstream fs;
    fs.open(path.ToStdString(), ios::binary | ios::in);
    fs.seekg(0, ios::end);
    int size = fs.tellg();
    if (size == -1) {
        GUI::show_error(this, _L("Failed to load image"));
        return false;
    } else if (size > 6 * 1024 * 1024) {
        GUI::show_error(this, _L("Maximum image size: 6MB"));
        return false;
    }
    fs.close();
    string  buf;
    wxImage img;
    bool    flag = img.LoadFile(wxString::FromUTF8(path.utf8_string()), wxBITMAP_TYPE_ANY);
    if (!flag) {
        GUI::show_error(this, _L("Failed to load image"));
        return false;
    }

    int    min_size     = min(img.GetHeight(), img.GetWidth());
    int    max_size     = max(img.GetHeight(), img.GetWidth());
    if (min_size < 128) {
        GUI::show_error(this, _L("Minimum image resolution: 128*128"));
        return false;
    }
    if (max_size > 5000) {
        GUI::show_error(this, _L("Maximum image resolution: 5000*5000"));
        return false;
    }
    return true;
}

ImageUploadPanel::ImageUploadPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    SetMinSize(FromDIP(wxSize(320, 160)));
    SetMaxSize(FromDIP(wxSize(320, 160)));
    SetSize(FromDIP(wxSize(320, 160)));
    SetDoubleBuffered(true);
    m_upload_icon = ScalableBitmap(this, "model_api_upload_image", 24);
    m_delete_icon = ScalableBitmap(this, "model_api_delete_image", 32);
    wxString str  = _L("It supports PNG, JPG, JPEG. Images should be no larger than 6MB with a minimum resolution of 128*128.");
    {
        Label label(this, Label::Body_10, str);
        label.Wrap(FromDIP(234));
        std::string sstr = label.GetLabel().utf8_string();
        boost::algorithm::split(m_vs, sstr, boost::is_any_of("\n"));
    }
    Bind(wxEVT_PAINT, &ImageUploadPanel::onPaint, this);
    Bind(wxEVT_LEFT_DOWN, &ImageUploadPanel::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ImageUploadPanel::onLeftUp, this);
    Bind(wxEVT_ENTER_WINDOW, &ImageUploadPanel::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &ImageUploadPanel::OnMouseLeave, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &ImageUploadPanel::onMouseCaptureLost, this);
}

wxString ImageUploadPanel::getPath() { return m_path; }

void ImageUploadPanel::onPaint(wxPaintEvent& event) 
{
    auto size = GetClientSize();
    wxPaintDC                          dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    if (m_path.empty() || !m_img.IsOk()) {
        gc->SetBrush(wxBrush(*wxWHITE));
        gc->DrawRectangle(0, 0, size.x, size.y);
        gc->DrawBitmap(m_upload_icon.bmp(), (size.x - m_upload_icon.GetBmpWidth()) / 2, FromDIP(37), m_upload_icon.GetBmpWidth(), m_upload_icon.GetBmpHeight());
        dc.SetFont(Label::Body_12);
        dc.SetTextForeground(wxColor("#328DF8"));
        auto text_size0 = dc.GetTextExtent(_L("Please upload the image."));
        dc.DrawText(_L("Please upload the image."), (size.x - text_size0.x) / 2, FromDIP(70));
        dc.SetFont(Label::Body_10);
        dc.SetTextForeground(wxColor("#B3B3B3"));
        for (int i = 0; i < m_vs.size(); i++){
            auto text_size = dc.GetTextExtent(wxString::FromUTF8(m_vs[i]));
            dc.DrawText(wxString::FromUTF8(m_vs[i]), (size.x - text_size.x) / 2, FromDIP(116) + i * text_size.y);
        }
    }
    else {
        gc->SetBrush(wxBrush(*wxBLACK));
        gc->DrawRectangle(0, 0, size.x, size.y);
        auto rect = FFUtils::calcContainedRect(size, m_img.GetSize(), false);
        gc->DrawBitmap(m_img, rect.x, rect.y, rect.width, rect.height);
        if (m_isHovered) {    
            gc->SetBrush(wxBrush(wxColour(0, 0, 0, 128)));
            gc->DrawRectangle(0, 0, size.x, size.y);
            gc->DrawBitmap(m_delete_icon.bmp(), (size.x - m_delete_icon.GetBmpWidth()) / 2, (size.y - m_delete_icon.GetBmpHeight()) / 2,
                           m_delete_icon.GetBmpWidth(), m_delete_icon.GetBmpHeight());
        }
    }
}

void ImageUploadPanel::onLeftDown(wxMouseEvent& event) 
{
    m_isGeneratePressed = true;
    if (!HasCapture()) {
        CaptureMouse();
    }
    event.Skip();
}

void ImageUploadPanel::onLeftUp(wxMouseEvent& event) 
{
    if (!m_isGeneratePressed) {    
        event.Skip();
        return;
    }

    m_isGeneratePressed = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
    
    bool isFunc = true;
    if (m_path.empty() || !m_img.IsOk()) {
        wxFileDialog  dlg(this, _L("Select Image"), wxGetApp().app_config->get_last_dir(), "",
                          "Image files (*.jpeg;*jpg;*.png)|*.jpeg;*.jpg;*.png", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        wxArrayString files;
        if (dlg.ShowModal() != wxID_OK) {
            event.Skip();
            return;
        }
        wxGetApp().app_config->update_skein_dir(dlg.GetDirectory().utf8_string());
        dlg.GetPaths(files);
        m_path = files[0];
        if (judgeTransImage(m_path)) {
            if (!m_img.LoadFile(m_path, wxBITMAP_TYPE_ANY)) {
                GUI::show_error(this, _L("Image Load Failed"));
                isFunc = false;
            }
        }
        else {
            m_path = "";
            isFunc = false;
        }
    }
    else {
        m_path = "";
        m_img.Clear();
    }

    if (isFunc) {
        Refresh();
        wxQueueEvent(this, new wxCommandEvent(EVT_LOADED_IMAGE));
    }
    event.Skip();
}

void ImageUploadPanel::onMouseCaptureLost(wxMouseCaptureLostEvent& event) 
{
    m_isGeneratePressed = false;
    Refresh();
    event.Skip();
}

void ImageUploadPanel::OnMouseEnter(wxMouseEvent& event) 
{
    m_isHovered = true;
    SetCursor(wxCURSOR_HAND);
    Refresh();
    event.Skip();
}

void ImageUploadPanel::OnMouseLeave(wxMouseEvent& event) 
{
    m_isHovered = false;
    SetCursor(wxCURSOR_ARROW);
    Refresh();
    event.Skip();
}

void ModelApiDialog::updateCustomModelDir() 
{
    wxString dir_path = data_dir() + "/AiModel";
    wxDir    dir(dir_path);
    if (!dir.IsOpened()) {
        bool ret = wxFileName::Mkdir(dir_path, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        if (!ret) {
            BOOST_LOG_TRIVIAL(error) << "Create Directory Failed: " << dir_path.utf8_string();
            m_dir_path = wxStandardPaths::Get().GetTempDir().utf8_string();
            return;
        }
    }
    m_dir_path = dir_path.utf8_string();

    wxDateTime cutoff = wxDateTime::Now();
    cutoff.Subtract(wxDateSpan::Month());

    wxString filename;
    bool     cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (cont) {
        wxFileName file(dir_path, filename);
        if (file.FileExists()) {
            wxDateTime modTime = file.GetModificationTime();
            if (modTime.IsValid() && modTime.IsEarlierThan(cutoff)) {
                if (!wxRemoveFile(file.GetFullPath())) {
                    BOOST_LOG_TRIVIAL(error) << "Delete File Failed: " << file.GetFullPath();
                }
            }
        }
        cont = dir.GetNext(&filename);
    }
}

const std::string& ModelApiDialog::GetDir()
{ 
    return m_dir_path; 
}

ModelApiDialog::ModelApiDialog(wxWindow* parent)
    : 
    FFTitleLessDialog(parent), m_generate_btn_rect(0, 0, 0, 0), 
    m_question_link_rect(0, 0, 0, 0), m_isGeneratePressed(false), 
    m_generateType(IMAGE_MODEL), 
    m_pretreat_link_rect(0, 0, 0, 0), m_pretreat_btn_rect(0, 0, 0, 0)
{
    this->SetSize(FromDIP(wxSize(393, 438)));
    this->SetMinSize(FromDIP(wxSize(393, 438)));
    this->SetDoubleBuffered(true);
    m_loadIcon        = std::make_shared<ApiLoadingIcon>(this);
    m_loadIcon->Bind(EVT_UPDATE_ICON, [=](wxCommandEvent& event) { 
        this->Refresh();
    });
    m_loadTask                 = std::make_shared<ModelApiTask>(this);
    m_loadTask->setThreadFunc([task = this->m_loadTask]() {
        com_user_ai_points_info_t data;
        auto                      ret = COM_OK;
        ret = MultiComHelper::inst()->getUserAiPointsInfo(data, 15000);
        if (ret != COM_OK) {
            task->safeFunc([task]() {
                auto event = new wxCommandEvent(EVT_ERROR_MSG);
                event->SetString(_L("Network Error"));
                event->SetInt(1);
                wxQueueEvent(task->Parent(), event);
            });
            return;
        }
        std::string promoData;
        //auto        language = wxGetApp().app_config->get_language_code();
        /*ret                  = MultiComHelper::inst()->getPromoShareData(language.substr(0, 2), promoData, 15000);
        if (ret != COM_OK) {
            return;
        }*/
        task->safeFunc([task, data, promoData]() {
            auto event          = new FinishScoreEvent();
            event->curCostScore = data.currAiGeneratePoints;
            event->totalScore   = data.totalPoints;
            event->promoData    = promoData;
            wxQueueEvent(task->Parent(), event);
        });
    });
    Bind(EVT_ERROR_MSG, [=](wxCommandEvent& event) {
        ErrorDialog edlg(this, event.GetString(), false);
        edlg.ShowModal();
        if (event.GetInt() == 1) {
            Close();
        }
    });
    Bind(EVT_FINISH_SCORE, [=](FinishScoreEvent& event) { 
        this->m_loadIcon->End();
        this->RefreshScore(event.curCostScore, event.totalScore);
        this->m_promoData = event.promoData;
    });
    m_loadTask->start();
    m_question_dialog          = new QuestionDialog(this);
    m_question_dialog->Hide();
    m_bmp_map["bg"] = ScalableBitmap(this, "model_api_dlg_bg", ToDIP(GetSize().y));
    m_bmp_map["question_mark"] = ScalableBitmap(this, "model_api_question_mark", 12);
    m_bmp_map["sw_off"] = ScalableBitmap(this, "switch_button_disabled", 16);
    m_bmp_map["sw_on"] = ScalableBitmap(this, "switch_button_enabled", 16);
    auto sizer      = new wxBoxSizer(wxVERTICAL);
    m_image_panel              = new ImageUploadPanel(this);
    m_image_panel->Bind(EVT_LOADED_IMAGE, [=](wxCommandEvent& event) { Refresh(); });
    sizer->AddSpacer(FromDIP(138));
    sizer->Add(m_image_panel, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(140));
    sizer->Fit(this);
    SetSizer(sizer);
    Layout();
    Center();

    MultiComMgr::inst()->Bind(COM_WAN_DEV_MAINTAIN_EVENT, [=](ComWanDevMaintainEvent& event) { 
        event.Skip();
        if (!event.login) {
            Close();
        }
    });
    Bind(wxEVT_LEFT_DOWN, &ModelApiDialog::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ModelApiDialog::onLeftUp, this);
    Bind(wxEVT_MOTION, &ModelApiDialog::OnMouseMove, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &ModelApiDialog::onMouseCaptureLost, this);
    m_loadIcon->Loading(200);
}

wxString ModelApiDialog::getImage() { 
    return this->m_image_panel->getPath(); 
}

void ModelApiDialog::drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc)
{
    FFTitleLessDialog::drawBackground(dc, gc);
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    auto size = this->GetClientSize();
    gc->DrawBitmap(m_bmp_map["bg"].bmp(), 0, 0, size.x, size.y);
    drawCenterText(dc, gc, _L("AI 3D Model Generator"), FromDIP(38), Label::sysFont(16, true), wxColor("#333333"));
    const int type_center_sper = FromDIP(88);
    gc->SetPen(wxPen(wxColor("#C8C8C8"), 1));
    gc->StrokeLine(size.x / 2, FromDIP(82), size.x / 2, FromDIP(90));
    auto text_type_str = _L("TEXT-3D");
    auto image_type_str = _L("IMAGE-3D");
    auto text_type_size = dc.GetTextExtent(text_type_str);
    auto image_type_size = dc.GetTextExtent(image_type_str);
    if (m_model_type_rects.empty()) {
        m_model_type_rects[TEXT_MODEL] = wxRect((size.x - text_type_size.x) / 2 - type_center_sper, FromDIP(76), text_type_size.x,
                                                FromDIP(20));
        m_model_type_rects[IMAGE_MODEL] = wxRect((size.x - image_type_size.x) / 2 + type_center_sper, FromDIP(76), image_type_size.x,
                                                FromDIP(20));
    }
    std::unordered_map<int, wxString> type_strs;
    type_strs[TEXT_MODEL] = text_type_str;
    type_strs[IMAGE_MODEL] = image_type_str;
    for (auto it : m_model_type_rects) {
        if (m_generateType == it.first) {
            dc.SetFont(Label::Head_14);
            dc.SetTextForeground(wxColor("#328DFB"));
            dc.DrawText(type_strs[it.first], it.second.x, it.second.y);
            gc->SetPen(wxPen(wxColor("#328DF8"), 2));
            gc->StrokeLine(it.second.x, it.second.y + it.second.height, it.second.x + it.second.width,
                           it.second.y + it.second.height);
        }
        else {
            dc.SetFont(Label::Body_14);
            dc.SetTextForeground(wxColor("#000000"));
            dc.DrawText(type_strs[it.first], it.second.x, it.second.y);
        }
    }
    gc->SetPen(*wxTRANSPARENT_PEN);
    if (m_generateType == IMAGE_MODEL) {
        dc.SetTextForeground(wxColor("#333333"));
        dc.SetFont(Label::Body_11);
        auto      text_size = dc.GetTextExtent(_L("Image Upload Tips"));
        auto      size      = this->GetClientSize();
        auto&     bmp       = m_bmp_map["question_mark"];
        const int icon_sper = 5;
        int       startPos  = m_image_panel->GetPosition().x;
        gc->DrawBitmap(bmp.bmp(), startPos, FromDIP(115), bmp.GetBmpWidth(), bmp.GetBmpHeight());
        dc.DrawText(_L("Image Upload Tips"), startPos + icon_sper + bmp.GetBmpWidth(), FromDIP(115));
        if (m_question_link_rect.IsEmpty()) {
            m_question_link_rect = wxRect(startPos, FromDIP(115), text_size.x + bmp.GetBmpWidth() + icon_sper, bmp.GetBmpHeight());
        }
        auto      pretreat_text_size = dc.GetTextExtent(_L("Image Pretreat"));
        auto&     pretreat_btn = m_can_image_pretreat ? m_bmp_map["sw_on"] : m_bmp_map["sw_off"];
        int       startPos0  = m_image_panel->GetPosition().x + m_image_panel->GetClientSize().x - 
            (icon_sper * 2 + pretreat_btn.GetBmpWidth() + bmp.GetBmpWidth() + pretreat_text_size.x);
        gc->DrawBitmap(bmp.bmp(), startPos0, FromDIP(115), bmp.GetBmpWidth(), bmp.GetBmpHeight());
        dc.DrawText(_L("Image Pretreat"), startPos0 + icon_sper + bmp.GetBmpWidth(), FromDIP(115));
        gc->DrawBitmap(pretreat_btn.bmp(), startPos0 + icon_sper * 2 + pretreat_text_size.x + bmp.GetBmpWidth(), 
            FromDIP(115), pretreat_btn.GetBmpWidth(), pretreat_btn.GetBmpHeight());
        if (m_pretreat_link_rect.IsEmpty()) {
            m_pretreat_link_rect = wxRect(startPos0, FromDIP(115), bmp.GetBmpWidth(), bmp.GetBmpHeight());
        }
        if (m_pretreat_btn_rect.IsEmpty()) {
            m_pretreat_btn_rect = wxRect(startPos0 + bmp.GetBmpWidth() + icon_sper, FromDIP(115), pretreat_text_size.x + icon_sper + 
                pretreat_btn.GetBmpWidth(), pretreat_btn.GetBmpHeight());
        }
    }
    if (m_loadIcon->isLoading()) {
        const int loadSize = FromDIP(40);
        m_loadIcon->paintInRect(gc, wxRect((size.x - loadSize) / 2, FromDIP(314), loadSize, loadSize));
    } else {
        drawCenterText(dc, gc, m_cost_text, FromDIP(314), Label::Body_12, wxColor("#333333"));
        drawCenterText(dc, gc, m_score_text, FromDIP(338), Label::Body_12, wxColor("#419488"));
    }
    if (m_loadIcon->isLoading() || m_image_panel->getPath().empty()) {
        gc->SetBrush(wxColor("#D2D2D2"));
    }
    else {
        gc->SetBrush(wxColor("#419488"));
        if (m_isGenerateHovered) {
            gc->SetBrush(wxColor("#65A79E"));
        }
        if (m_isGeneratePressed) {
            gc->SetBrush(wxColor("#1A8676"));
        }
    }
    wxString btn_text(_L("Start generating"));
    dc.SetFont(Label::Body_12);
    auto btn_text_size = dc.GetTextExtent(btn_text);
    wxSize btn_size(FromDIP(320), FromDIP(30));
    if (m_generate_btn_rect.IsEmpty()) {
        m_generate_btn_rect = wxRect((size.x - btn_size.x) / 2, FromDIP(370), btn_size.x, btn_size.y);
    }
    gc->DrawRoundedRectangle((size.x - btn_size.x) / 2, FromDIP(370), btn_size.x, btn_size.y, 4);
    dc.SetTextForeground(*wxWHITE);
    dc.DrawText(btn_text, (size.x - btn_text_size.x) / 2, FromDIP(377));
}

ModelApiDialog::~ModelApiDialog() 
{ 
    wxEventBlocker              block(this);
    std::lock_guard<std::mutex> lock(m_loadTask->Lock());
    m_loadTask->FinishLoop().store(true);
    m_loadTask.reset();
}

void ModelApiDialog::drawCenterText(wxBufferedPaintDC& dc, wxGraphicsContext* gc, const wxString& str, int height, wxFont& font, wxColour color, wxString iconName /* = "" */)
{ 
    dc.SetFont(font);
    auto text_size = dc.GetTextExtent(str);
    auto size = this->GetClientSize();
    dc.SetTextForeground(color);
    if (iconName.empty()) {
        dc.DrawText(str, (size.x - text_size.x) / 2, height);
    }
    else {
        auto& bmp = m_bmp_map[iconName.utf8_string()];
        const int icon_sper = 5;
        gc->DrawBitmap(bmp.bmp(), (size.x - text_size.x - bmp.GetBmpWidth() - icon_sper) / 2, height, bmp.GetBmpWidth(), bmp.GetBmpHeight());
        dc.DrawText(str, (size.x - text_size.x - bmp.GetBmpWidth() - icon_sper) / 2 + icon_sper + bmp.GetBmpWidth(), height);
        if (iconName == "question_mark" && m_question_link_rect.IsEmpty()) {
            m_question_link_rect = wxRect((size.x - text_size.x - bmp.GetBmpWidth() - icon_sper) / 2, height, 
                text_size.x + bmp.GetBmpWidth() + icon_sper, bmp.GetBmpHeight());
        }
    }
}

void ModelApiDialog::onLeftDown(wxMouseEvent& event) 
{
    if (m_generateType == IMAGE_MODEL) {
        if (!m_model_type_rects.empty() && m_model_type_rects[TEXT_MODEL].Contains(event.GetPosition())) {
            m_isTextTypePressed = true;
        }
        if (!m_generate_btn_rect.IsEmpty() && m_generate_btn_rect.Contains(event.GetPosition())) {
            m_isGeneratePressed = true;
        }
        if (!m_pretreat_btn_rect.IsEmpty() && m_pretreat_btn_rect.Contains(event.GetPosition())) {
            m_can_image_pretreat = !m_can_image_pretreat;
        }
        Refresh();
        if (!HasCapture()) {
            CaptureMouse();
        }
        event.Skip();
        return;
    }
    else if (m_generateType == TEXT_MODEL) {
        if (!m_model_type_rects.empty() && m_model_type_rects[IMAGE_MODEL].Contains(event.GetPosition())) {
            m_isImageTypePressed = true;
        }
        Refresh();
        if (!HasCapture()) {
            CaptureMouse();
        }
        event.Skip();
        return;
    }
    event.Skip();
}

void ModelApiDialog::onLeftUp(wxMouseEvent& event) 
{
    if (m_generateType == IMAGE_MODEL) {
        if (!m_generate_btn_rect.IsEmpty() && m_isGeneratePressed && !m_image_panel->getPath().empty() &&
            m_generate_btn_rect.Contains(event.GetPosition())) {
            GenerateClicked(); 
            m_isGeneratePressed = false;
        }
        if (!m_model_type_rects.empty() && m_isTextTypePressed) {
            changeModelType(TEXT_MODEL);
            m_isTextTypePressed = false;
        }
       
        Refresh();
        if (HasCapture()) {
            ReleaseMouse();
        }
        event.Skip();
        return;
    } 
    else if (m_generateType == TEXT_MODEL) {
        if (!m_model_type_rects.empty() && m_isImageTypePressed) {
            changeModelType(IMAGE_MODEL);
            m_isImageTypePressed = false;
        }

        Refresh();
        if (HasCapture()) {
            ReleaseMouse();
        }
        event.Skip();
        return;
    }
    if (HasCapture()) {
        ReleaseMouse();
    }
    event.Skip();
}

void ModelApiDialog::onMouseCaptureLost(wxMouseCaptureLostEvent& event) 
{ 
    m_isGeneratePressed = false;
    m_isTextTypePressed = false;
    m_isImageTypePressed = false;
    Refresh();
    event.Skip();
}

void ModelApiDialog::OnMouseMove(wxMouseEvent& event) 
{
    if (m_generateType == IMAGE_MODEL) {
        if (m_isGenerateHovered && !m_generate_btn_rect.Contains(event.GetPosition())) {
            m_isGenerateHovered = false;
            SetCursor(wxCURSOR_ARROW);
            Refresh();
        } else if (!m_isGenerateHovered && m_generate_btn_rect.Contains(event.GetPosition())) {
            m_isGenerateHovered = true;
            SetCursor(wxCURSOR_HAND);
            Refresh();
        }
        if (m_isQuestionHovered && !m_question_link_rect.Contains(event.GetPosition())) {
            m_isQuestionHovered = false;
            SetCursor(wxCURSOR_ARROW);
            m_question_dialog->Show(false);
        } else if (!m_isQuestionHovered && m_question_link_rect.Contains(event.GetPosition())) {
            m_isQuestionHovered = true;
            SetCursor(wxCURSOR_HAND);
            m_question_dialog->Move(this->ClientToScreen(wxPoint((GetClientSize().x - m_question_dialog->GetSize().x) / 2, FromDIP(134))));
            m_question_dialog->Show(true);
        }
        if (m_isPretreatHovered && !m_pretreat_link_rect.Contains(event.GetPosition())) {
            m_isPretreatHovered = false;
            SetCursor(wxCURSOR_ARROW);
            m_question_dialog->Show(false);
        } else if (!m_isPretreatHovered && m_pretreat_link_rect.Contains(event.GetPosition())) {
            m_isPretreatHovered = true;
            SetCursor(wxCURSOR_HAND);
            m_question_dialog->Move(this->ClientToScreen(wxPoint((GetClientSize().x - m_question_dialog->GetSize().x) / 2, FromDIP(134))));
            m_question_dialog->Show(true);
        }
    }
    event.Skip();
}

void ModelApiDialog::GenerateClicked() 
{ 
    if (m_total_score < 0 || m_cost_score < 0) {
        WarningDialog dlg(this, _L("Not enough points. Please earn more points."), _L("Info"));
        dlg.SetButtonLabel(wxID_OK, _L("Get Now"));
        if (dlg.ShowModal() == wxID_OK) {
            Close();
            wxGetApp().jump_to_user_points();
        }
        //if (m_promoData != "") {
        //    PromoShareDlg pro(this, m_promoData);
        //    pro.ShowModal();
        //}
        return;
    }
    if (!ifstream(this->m_image_panel->getPath().ToStdString()).good()) {
        GUI::show_error(this, _L("Failed to load image"));
        return;
    }
    EndModal(wxID_OK);
}

void ModelApiDialog::RefreshScore(int cost, int total) 
{
    if (cost < 0 || total < 0) {
        BOOST_LOG_TRIVIAL(error) << "AI MODEL: cost score or total score should be nonnegative number";
    }
    m_cost_score = cost;
    m_total_score = total;

    if (cost <= 0) {
        m_cost_text = wxString(_L("This generation is free."));
    } else {
        m_cost_text = wxString(_L("Points consumed")) + wxString::Format(wxT(":  %d"), cost);
    }
    m_score_text = wxString(_L("Remaining points") + wxString::Format(wxT(":  %d"), total));

    Refresh();
}

void ModelApiDialog::changeModelType(ModelType type) 
{ 
    m_generateType = type; 
    if (type == TEXT_MODEL) {
        m_image_panel->Hide();
    }
    else if (type == IMAGE_MODEL) {
        m_image_panel->Show();
    }
    //Layout();
}

wxDEFINE_EVENT(EVT_OLD_TASK, wxCommandEvent);
wxDEFINE_EVENT(EVT_SET_ID, wxCommandEvent);
wxDEFINE_EVENT(EVT_SET_STATE, ApiSetStateEvent);
wxDEFINE_EVENT(EVT_COMPLETE_MODEL, CompleteModelEvent);
wxDEFINE_EVENT(EVT_CHOICE_COLOR, ChoiceColorEvent);
wxDEFINE_EVENT(EVT_COMPLETE_CONVERT, CompleteConvertEvent);
wxDEFINE_EVENT(EVT_REAL_CLOSE, wxCommandEvent);

ModelGenerateDialog::ModelGenerateDialog(wxWindow* parent) : 
    FFTitleLessDialog(parent), m_download_tool(4, 30000)
{
    this->SetSize(wxSize(FromDIP(393), FromDIP(176)));
    this->SetMinSize(wxSize(FromDIP(393), FromDIP(176)));
    this->SetDoubleBuffered(true);
    m_job_id   = std::make_shared<int64_t>(-1);
    m_loadIcon = std::make_shared<ApiLoadingIcon>(this);
    m_loadIcon->Bind(EVT_UPDATE_ICON, [=](wxCommandEvent& event) { this->Refresh(); });
    m_generateTask = std::make_shared<ModelApiTask>(this);
    m_abortTask = std::make_shared<ModelApiTask>(this);
    Bind(EVT_OLD_TASK, [=](wxCommandEvent& event) { 
        WarningDialog dlg(this, _L("A model is currently being generated. Please wait."), _L("Warning"));
        dlg.ShowModal();
    });
    Bind(EVT_SET_STATE, [=](ApiSetStateEvent& event) {
        if (!event.isQueuePanel && m_isQueuePanel) {
            wxGetApp().update_user_points();
        }
        m_remainCount = event.remainCount;
        m_totalCount  = event.totalCount;
        showCurState(event.isQueuePanel, event.isShowQueue);
    });
    Bind(EVT_ERROR_MSG, [=](wxCommandEvent& event) {
        if (event.GetString().ToStdString() == "NOT_ENOUGH_POINTS") {
            WarningDialog dlg(this, _L("Not enough points. Please earn more points."), _L("Info"));
            dlg.SetButtonLabel(wxID_OK, _L("Get Now"));
            if (dlg.ShowModal() == wxID_OK) {    
                wxGetApp().jump_to_user_points();
            }
            Close();
            return;
        }
        ErrorDialog edlg(this, event.GetString(), false);
        edlg.ShowModal();
        m_can_cancel = true;
        if (event.GetInt() == 1) {
            Close();
        } else if (event.GetInt() == 2) {
            *m_job_id = -1;
            Close(true);
        }
    });
    Bind(EVT_COMPLETE_MODEL, [=](CompleteModelEvent& event) { 
        m_download_path = (boost::filesystem::path(ModelApiDialog::GetDir()) /
            ("hunyuan_" + std::to_string(event.job_id) + ".glb")).string();
        m_src_path = event.path;
        m_download_tool.downloadDisk(m_src_path, m_download_path, 100000, 6000000);
    });
    m_download_tool.Bind(EVT_FF_DOWNLOAD_FINISHED, [this](FFDownloadFinishedEvent& event) {
        if (!event.succeed) {
            if (m_download_try_angin) {
                GUI::show_error(this, _L("AI Model Generation Failed"));
                *m_job_id = -1;
                Close(true);
            } else {
                m_download_try_angin = true;
                m_download_tool.downloadDisk(m_src_path, m_download_path, 100000, 6000000);
            }
            return;
        }
        m_download_try_angin = false;
        auto        task = this->m_generateTask;
        std::string path = this->m_download_path;
        /*path             = (boost::filesystem::path(ModelApiDialog::GetDir()) /
                ("hunyuan_" + std::to_string(191) + ".glb"))
                   .string();*/
        m_generateTask->setThreadFunc([task, path]() {
            ConvertModel    cm;
            auto            area = wxGetApp().plater()->build_volume().printable_area();
            in_cvt_params_t params;
            params.transCoordSys   = true;
            params.maxPrintSize[0] = fabs(area[2].x() - area[0].x());
            params.maxPrintSize[1] = fabs(area[2].y() - area[0].y());
            params.maxPrintSize[2] = wxGetApp().plater()->build_volume().printable_height();
            auto model_data = std::make_shared<convert_model_data_t>();
            cm.initConvertGlb(path, params, *model_data);
            //cm.initConvertObj(path0, params, *model_data);
            auto colors = cm.clusterColors(*model_data, 4);
            task->safeFunc([=]() {
                auto e = new ChoiceColorEvent();
                e->data = model_data;
                e->colors = colors;
                wxQueueEvent(task->Parent(), e);
            });
        });
        m_generateTask->start();
    });
    //m_download_tool.downloadDisk("./a.jpg", "https://p3-aiop-sign.byteimg.com/tos-cn-i-vuqhorh59i/202508061712198A68F2BD6B95B7C2B8B4-7249-0~tplv-vuqhorh59i-image.image?rk3s=7f9e702d\\u0026x-expires=1754557940\\u0026x-signature=ZRnD%2FowB4omjRcHLHPKF4hbUoTg%3D", 100000, 6000000);
    Bind(EVT_CHOICE_COLOR, [=](ChoiceColorEvent& event) {
        this->m_modelData = event.data;
        this->m_cvt_colors = event.colors;
        EndModal(wxID_OK);
    });
    Bind(EVT_SET_ID, [job_id = this->m_job_id](wxCommandEvent& event) { 
        *job_id = event.GetInt();
    });
    m_abortTask->setThreadFunc([task = this->m_abortTask, job_id = this->m_job_id]() {
        auto ret = MultiComHelper::inst()->abortAiModelJob(*job_id, 10000);
        if (ret != COM_OK) {
            task->safeFunc([task]() {
                auto event = new wxCommandEvent(EVT_ERROR_MSG);
                event->SetString(_L("Failed to cancel the task"));
                event->SetInt(0);
                wxQueueEvent(task->Parent(), event);
            });
        }
        else {
            task->safeFunc([task]() {
                wxQueueEvent(task->Parent(), new wxCommandEvent(EVT_REAL_CLOSE));
            });
        }
    });
    Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
        if (*m_job_id < 0) {
            event.Skip();
            return;
        }
        if (!m_isQueuePanel) {
            event.Skip();
            return;
        }
        if (m_can_cancel) {
            m_can_cancel = false;
            m_abortTask->start();
        }
        if (m_isOffline) {
            event.Skip();
        }
    });
    Bind(EVT_REAL_CLOSE, [=](wxCommandEvent& event) { 
        *m_job_id = -1;
        m_can_cancel = true;
        Close(true);
    });
    MultiComMgr::inst()->Bind(COM_WAN_DEV_MAINTAIN_EVENT, [=](ComWanDevMaintainEvent& event) {
        event.Skip();
        if (!event.login) {
            m_isOffline = true;
            Close();
        }
    });

    m_info_text = new Label(this, Label::Body_14, "");
    m_info_text->SetBackgroundColour(*wxWHITE);
    m_queue_text = new Label(this, Label::Body_13, "");
    m_queue_text->SetBackgroundColour(*wxWHITE);
    m_sizer = new wxBoxSizer(wxVERTICAL);
    m_sizer->AddSpacer(FromDIP(32));
    m_sizer->Add(m_info_text, 0, wxALIGN_CENTER | wxALL, 0);
    m_sizer->AddSpacer(FromDIP(16));
    m_sizer->Add(m_queue_text, 0, wxALIGN_CENTER | wxALL, 0);
    m_sizer->AddSpacer(FromDIP(96));
    SetSizer(m_sizer);
    showCurState(true, false);// init state
    Fit();
    m_loadIcon->Loading(200);
}

void ModelGenerateDialog::SetImgPath(wxString path) 
{ 
    this->m_img_path = path; 
    m_generateTask->setThreadFunc([task = this->m_generateTask, img_path = this->m_img_path]() {
        const std::string generateFormat       = "GLB";
        const int         maxNetworkErrorCount = 3;
        const int         msTimeout            = 15000;

        auto        imgName       = fs::path(img_path.utf8_string()).extension().string();
        std::string img_url       = "";
        auto        callback_func = [](long long now, long long total, void* data) {
            std::atomic_bool* isFinish = static_cast<std::atomic_bool*>(data);
            if (isFinish->load()) {
                return -1;
            }
            return 0;
        };
        ComErrno ret = COM_OK;
        BOOST_LOG_TRIVIAL(warning) << "AI IMAGE PATH: " << img_path.utf8_string();
        ret = MultiComHelper::inst()->uploadAiImageClound(img_path.utf8_string(), imgName, img_url, callback_func, &task->FinishLoop(), msTimeout);
        if (ret != COM_OK) {
            task->safeFunc([task]() {
                auto event = new wxCommandEvent(EVT_ERROR_MSG);
                event->SetString(_L("Network Error"));
                event->SetInt(1);
                wxQueueEvent(task->Parent(), event);
            });
            return;
        }
        com_ai_model_job_result_t result;
        //result.jobId = 0;
        //result.isOldJob = false;
        ret = MultiComHelper::inst()->startAiModelJob(1, img_url, generateFormat, result, msTimeout);
        if (ret != COM_OK) {
            task->safeFunc([task, ret]() {
                auto event = new wxCommandEvent(EVT_ERROR_MSG);
                if (ret == COM_AI_MODEL_JOB_NOT_ENOUGH_POINTS) {
                    event->SetString("NOT_ENOUGH_POINTS");
                }
                else{
                    event->SetString(_L("Network Error"));
                }
                
                event->SetInt(1);
                wxQueueEvent(task->Parent(), event);
            });
            return;
        }
        if (result.isOldJob) {
            task->safeFunc([=]() {
                wxQueueEvent(task->Parent(), new wxCommandEvent(EVT_OLD_TASK));
                // task->Sem().Wait();
            });
        }
        const int64_t job_id = result.jobId;
        BOOST_LOG_TRIVIAL(info) << "AI MODEL: CURRENT JOB ID ------ " << job_id;
        task->safeFunc([=]() {
            auto event = new wxCommandEvent(EVT_SET_ID);
            event->SetInt(job_id);
            wxQueueEvent(task->Parent(), event);
        });
        bool isFirstLoop       = true;
        int  networkErrorCount = 0;
        while (!task->FinishLoop().load()) {
            com_ai_model_job_state_t state;
            //state.status = 3;
            ret = MultiComHelper::inst()->getAiModelJobState(job_id, state, msTimeout);
            if (ret != COM_OK) {
                if (networkErrorCount < maxNetworkErrorCount) {
                    networkErrorCount++;
                } else {
                    task->safeFunc([task]() {
                        auto event = new wxCommandEvent(EVT_ERROR_MSG);
                        event->SetString(_L("Network Error"));
                        event->SetInt(2);
                        wxQueueEvent(task->Parent(), event);
                    });
                    return;
                }
            }
            else {
                networkErrorCount = 0;
            }
            if (isFirstLoop) {
                BOOST_LOG_TRIVIAL(info) << "AI MODEL: CURRENT HUNYUAN JOB_ID ------ " << state.externalJobId;
                isFirstLoop = false;
            }
            if (state.status == 2) { // generating failed
                task->safeFunc([task]() {
                    auto event = new wxCommandEvent(EVT_ERROR_MSG);
                    event->SetString(_L("AI Model Generation Failed"));
                    event->SetInt(1);
                    wxQueueEvent(task->Parent(), event);
                });
                return;
            }
            if (state.status == 4) { // canceled
                task->safeFunc([task]() {
                    auto event = new wxCommandEvent(EVT_REAL_CLOSE);
                    wxQueueEvent(task->Parent(), event);
                });
                return;
            }
            if (state.status == 3) { // completed
                auto event = new CompleteModelEvent();
                for (auto it : state.models) {
                    if (it.modelType == generateFormat) {
                        event->path   = it.modelUrl;
                        event->job_id = job_id;
                        break;
                    }
                }
                wxQueueEvent(task->Parent(), event);
                return;
            }

            task->safeFunc([=]() {
                auto e = new ApiSetStateEvent();
                if (state.posInQueue == 0) {
                    e->isQueuePanel = false;
                } else {
                    e->isShowQueue  = true;
                    e->isQueuePanel = true;
                    e->remainCount  = state.posInQueue;
                    e->totalCount   = state.queueLength;
                }
                wxQueueEvent(task->Parent(), e);
            });

            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    });
    m_generateTask->start();
}

void ModelGenerateDialog::drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc)
{
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    gc->SetBrush(*wxWHITE);
    gc->DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);
    const int img_size = FromDIP(60);
    auto      size     = GetClientSize();
    m_loadIcon->paintInRect(gc, wxRect((size.x - img_size) / 2, size.y - FromDIP(80), img_size, img_size));
}

std::shared_ptr<convert_model_data_t> ModelGenerateDialog::getModelData() 
{ 
    return this->m_modelData; 
}

cvt_colors_t ModelGenerateDialog::getCvtColors() 
{ 
    return m_cvt_colors; 
}

std::string ModelGenerateDialog::getDownloadPath() 
{ 
    return m_download_path; 
}

void ModelGenerateDialog::showCurState(bool isQueuePanel, bool isShowQueue) 
{
    m_isQueuePanel = isQueuePanel;
    m_isShowQueue  = isShowQueue;
    if (isQueuePanel) {
        m_info_text->SetLabel(_L("We're currently experiencing high demand. Please wait..."));
        m_info_text->Wrap(FromDIP(313));
        if (isShowQueue) {
            m_queue_text->SetLabel(_L("Current queue") + wxString::Format(wxT(" %d/%d"), m_remainCount, m_totalCount));
        }
        else {
            m_queue_text->SetLabel("");
        }
    }
    else {
        m_info_text->SetLabel(_L("Generating, please wait..."));
        m_info_text->Wrap(FromDIP(313));
        m_queue_text->SetLabel("");
    }
    Layout();
    Center();
}

ModelGenerateDialog::~ModelGenerateDialog() 
{
    m_loadIcon->End();
    wxEventBlocker              block(this);
    {
        std::lock_guard<std::mutex> lock(m_generateTask->Lock());
        m_generateTask->FinishLoop().store(true);
        m_generateTask.reset();
    }
    {
        std::lock_guard<std::mutex> lock(m_abortTask->Lock());
        m_abortTask->FinishLoop().store(true);
        m_abortTask.reset();
    }
}

ApiSetStateEvent::ApiSetStateEvent(): wxCommandEvent(EVT_SET_STATE) {}

ModelColorDialog::ModelColorDialog(wxWindow* parent) : 
    FFTitleLessDialog(parent)
{ 
    this->SetSize(wxSize(FromDIP(393), FromDIP(233)));
    this->SetMinSize(wxSize(FromDIP(393), FromDIP(233)));
    this->SetDoubleBuffered(true);
    auto title = new Label(this, Label::Body_14, _L("Generation successful!"));
    title->SetBackgroundColour(*wxWHITE);
    auto inputLabel = new Label(this, Label::Body_13, _L("You can specify the number of colors for the model."));
    inputLabel->SetBackgroundColour(*wxWHITE);
    m_text_ctrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE | wxTE_CENTRE);
    m_text_ctrl->SetMaxSize(FromDIP(wxSize(20, 20)));
    m_text_ctrl->SetMinSize(FromDIP(wxSize(20, 20)));
    m_text_ctrl->SetValue("4");
    m_text_ctrl->SetBackgroundColour(*wxWHITE);
    m_text_ctrl->SetFont(Label::Body_13);
    m_text_ctrl->SetMaxLength(1);
    wxTextValidator validator(wxFILTER_DIGITS, nullptr);
    m_text_ctrl->SetValidator(validator);
    m_text_ctrl->Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
        wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
        wxString    str      = textCtrl->GetValue();
        if (str.empty()) {
            return;
        }
        int         number   = wxAtoi(str);
        const int min_num        = 1;
        const int max_num        = 4;
        if (number > max_num || number < min_num) {
            number = number < min_num ? min_num : max_num;
            str    = wxString::Format(("%d"), number);
            textCtrl->SetValue(str);
            textCtrl->SetInsertionPointEnd();
        }
        if (m_last_color_count != number) {
            m_last_color_count = number;
            auto color = ConvertModel().clusterColors(*m_modelData, number);
            changeColor(color);
        }
    });
    m_text_ctrl->Bind(wxEVT_CHAR, [this](wxKeyEvent& e) {
        int      keycode    = e.GetKeyCode();
        wxString input_char = wxString::Format("%c", keycode);
        long     value;
        if (!input_char.ToLong(&value) && input_char.ToStdString() != "\b")
            return;
        e.Skip();
    });
    m_btn   = new FFButton(this, wxID_ANY, _L("Import"), FromDIP(4), false); 
    m_btn->SetSize(FromDIP(wxSize(134, 30)));
    m_btn->SetMinSize(FromDIP(wxSize(134, 30)));
    m_btn->SetFontUniformColor(*wxWHITE);
    m_btn->SetBGColor(wxColour("#419488"));
    m_btn->SetBGHoverColor(wxColor("#65A79E"));
    m_btn->SetBGPressColor(wxColor("#1A8676"));
    m_btn->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) { 
        m_text_ctrl->SetEditable(false);
        m_btn->Hide();
        m_loadIcon->Loading(200);
        m_convertTask->setThreadFunc([task = m_convertTask, data = m_modelData, count = m_last_color_count, path = m_filepath]() {
            ConvertModel cm;
            auto         color            = cm.clusterColors(*data, count);
            std::string  convert_obj_file = path;
            std::string  extension        = fs::path(path).extension().string();
            auto         just_filename    = path.substr(0, path.size() - extension.size()) + "_convert";
            size_t       version          = 0;
            convert_obj_file              = just_filename;
            auto tempdir                  = ModelApiDialog::GetDir();
            while (fs::exists(boost::filesystem::path(tempdir) / (convert_obj_file + ".obj"))) {
                ++version;
                convert_obj_file = just_filename + "(" + std::to_string(version) + ")";
            }
            std::string mtl_path = convert_obj_file + ".mtl";
            std::string obj_path = convert_obj_file + ".obj";
            cm.doConvert(*data, color, obj_path, mtl_path);
            task->safeFunc([=]() {
                auto event      = new CompleteConvertEvent();
                event->colors   = color;
                event->obj_path = obj_path;
                event->mtl_path = mtl_path;
                wxQueueEvent(task->Parent(), event);
            });
        });
        m_convertTask->start();
    });
    Bind(EVT_COMPLETE_CONVERT, [=](CompleteConvertEvent& event) {
        Close();
        std::vector<std::string> arr;
        arr.emplace_back(event.obj_path);
        auto origin_path = wxGetApp().app_config->get_last_dir();
        wxGetApp().plater()->load_files(arr, LoadStrategy::LoadModel, false, event.colors);
        wxGetApp().app_config->update_skein_dir(origin_path);
    });
    m_loadIcon = std::make_shared<ApiLoadingIcon>(this);
    m_loadIcon->Bind(EVT_UPDATE_ICON, [=](wxCommandEvent& event) { this->Refresh(); });
    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddSpacer(FromDIP(32));
    sizer->Add(title, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(16));
    auto h_sizer = new wxBoxSizer(wxHORIZONTAL);
    h_sizer->AddSpacer(FromDIP(20));
    h_sizer->Add(inputLabel, 0, wxALL | wxALIGN_CENTER, 0);
    h_sizer->AddSpacer(FromDIP(10));
    h_sizer->Add(m_text_ctrl, 0, wxALIGN_CENTER, 0);
    h_sizer->AddSpacer(FromDIP(20));
    sizer->Add(h_sizer, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(83));
    sizer->Add(m_btn, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(32));
    sizer->SetMinSize(wxSize(FromDIP(393), FromDIP(233)));
    SetSizerAndFit(sizer);
    Layout(); 
    Center();
}

void ModelColorDialog::drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc)
{ 
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT); 
    gc->SetBrush(*wxWHITE);
    gc->DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);
    dc.SetFont(Label::Body_13);
    wxPoint start_pos(FromDIP(70), FromDIP(104));
    const int     grid_sper = FromDIP(16);
    const int size = FromDIP(51);
    
    start_pos.x             = GetClientSize().x / 2 - (size * m_last_color_count + (m_last_color_count - 1) * grid_sper) / 2;
    for (int i = 0; i < m_color_grids.size(); i++) {
        gc->SetBrush(m_color_grids[i]);
        gc->DrawRectangle(start_pos.x + i * (grid_sper + size), start_pos.y, size, size);
        auto luminance = m_color_grids[i].GetLuminance();
        dc.SetTextForeground(luminance > 0.6 ? wxColor("#333333") : *wxWHITE);
        auto num_str   = wxString::Format(wxT("%d"), i + 1);
        auto text_size = dc.GetTextExtent(num_str);
        dc.DrawText(num_str, start_pos.x + i * (grid_sper + size) + (size - text_size.x) / 2, start_pos.y + (size - text_size.y) / 2);
    }
    if (m_loadIcon->isLoading()) {
        dc.SetTextForeground(*wxBLACK);
        dc.SetFont(Label::Body_12);
        auto text      = _L("Importing...");
        auto text_size = dc.GetTextExtent(text);
        dc.DrawText(text, (GetClientSize().x - text_size.x) / 2, FromDIP(171));
        auto load_size = FromDIP(22);
        m_loadIcon->paintInRect(gc, wxRect((GetClientSize().x - load_size) / 2, FromDIP(194), load_size, load_size));
    }
}

void ModelColorDialog::changeColor(const cvt_colors_t& colors) 
{
    m_color_grids.clear();
    for (auto color : colors) {
        wxColour c;
        c.SetRGB((color[2] << 16) + (color[1] << 8) + color[0]);
        m_color_grids.emplace_back(c);
    }
    m_text_ctrl->SetValue(wxString::Format(wxT("%d"), m_last_color_count));
    Refresh();
}

void ModelColorDialog::setModelData(const std::shared_ptr<convert_model_data_t>& data) 
{ 
    this->m_modelData = data; 
    m_convertTask     = std::make_shared<ModelApiTask>(this);
}

void ModelColorDialog::setDownloadFile(const std::string& path) 
{ 
    this->m_filepath = path; 
}

ModelColorDialog::~ModelColorDialog() 
{
    wxEventBlocker              block(this);
    std::lock_guard<std::mutex> lock(m_convertTask->Lock());
    m_convertTask->FinishLoop().store(true);
    m_convertTask.reset();
}

VerticalCenterTextCtrl::VerticalCenterTextCtrl(wxWindow* parent): 
    wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE | wxTE_CENTRE)
{
    Bind(wxEVT_PAINT, &VerticalCenterTextCtrl::OnPaint, this);
}

void VerticalCenterTextCtrl::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxSize    size = GetClientSize();
    wxString  text = GetValue();
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.DrawRectangle(GetClientRect());
    dc.SetFont(this->GetFont());
    auto tsize = dc.GetTextExtent(text);
    int yPos = (size.y - tsize.y) / 2;
    dc.DrawText(text, (size.x - tsize.x) / 2, (size.y - tsize.y) / 2);
}

FinishScoreEvent::FinishScoreEvent() : wxCommandEvent(EVT_FINISH_SCORE) {}

CompleteModelEvent::CompleteModelEvent() : wxCommandEvent(EVT_COMPLETE_MODEL) {}

ChoiceColorEvent::ChoiceColorEvent() : wxCommandEvent(EVT_CHOICE_COLOR) {}

CompleteConvertEvent::CompleteConvertEvent() : wxCommandEvent(EVT_COMPLETE_CONVERT) {}

}} // namespace Slic3r::GUI


