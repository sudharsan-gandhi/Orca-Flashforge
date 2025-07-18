#include "ModelApiDialog.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/Utils/Http.hpp"
#include "slic3r/GUI/Plater.hpp"
#include "slic3r/GUI/FFUtils.hpp"
#include "slic3r/GUI/FlashForge/MultiComHelper.hpp"
#include <wx/base64.h>

namespace Slic3r {
namespace GUI {

wxDEFINE_EVENT(EVT_LOADED_IMAGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_FINISH_TASK, wxCommandEvent);
wxDEFINE_EVENT(EVT_UPDATE_ICON, wxCommandEvent);
wxDEFINE_EVENT(EVT_ERROR_MSG, wxCommandEvent);
wxDEFINE_EVENT(EVT_FINISH_SCORE, FinishScoreEvent);
wxDECLARE_EVENT(EVT_STORE_PROMO, wxCommandEvent);


QuestionDialog::QuestionDialog(wxWindow* parent) : FFRoundedWindow(parent)
{
    SetSize(FromDIP(272), FromDIP(189));
    auto title = new Label(this, Label::Body_13, _L("Image Upload Tips"));
    auto info =
        new Label(this, Label::Body_12,
                  _L("It supports PNG, JPG, JPEG, and WebP.Images should be no larger than 6MB with a minimum resolution of 128*128."));
    auto text1   = new Label(this, Label::Body_12, _L("Simple background (preferably solid color)"));
    auto text2   = new Label(this, Label::Body_12, _L("No text included"));
    auto text3   = new Label(this, Label::Body_12, _L("Single model"));
    auto text4   = new Label(this, Label::Body_12, _L("The model should not be too small"));
    auto v_sizer = new wxBoxSizer(wxVERTICAL);
    v_sizer->AddSpacer(FromDIP(16));
    v_sizer->Add(title, 0, wxALIGN_LEFT | wxLEFT, FromDIP(16));
    v_sizer->AddSpacer(FromDIP(6));
    info->Wrap(FromDIP(240));
    v_sizer->Add(info, 0, wxLEFT | wxRIGHT, FromDIP(16));
    v_sizer->AddSpacer(FromDIP(12));
    auto h_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto v_sizer0 = new wxBoxSizer(wxVERTICAL);
    text1->Wrap(FromDIP(101));
    v_sizer0->Add(text1, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text2->Wrap(FromDIP(101));
    v_sizer0->Add(text2, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text3->Wrap(FromDIP(101));
    v_sizer0->Add(text3, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text4->Wrap(FromDIP(101));
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
    v_sizer->Add(h_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, FromDIP(16));
    v_sizer->AddSpacer(FromDIP(16));
    Layout();
    SetSizerAndFit(v_sizer);
}

ApiLoadingIcon::ApiLoadingIcon(wxDialog* parent) : wxEvtHandler()
{
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
    SetMinSize(FromDIP(wxSize(160, 160)));
    SetMaxSize(FromDIP(wxSize(160, 160)));
    SetSize(FromDIP(wxSize(160, 160)));
    SetDoubleBuffered(true);
    m_upload_icon = ScalableBitmap(this, "model_api_upload_image", 24);
    m_delete_icon = ScalableBitmap(this, "model_api_delete_image", 29);
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
        gc->DrawBitmap(m_upload_icon.bmp(), (size.x - m_upload_icon.GetBmpWidth()) / 2, FromDIP(62), m_upload_icon.GetBmpWidth(), m_upload_icon.GetBmpHeight());
        dc.SetFont(Label::Body_11);
        dc.SetTextForeground(wxColor("#999999"));
        wxString str       = _L("Please Upload Image");
        auto     text_size = dc.GetTextExtent(str);
        dc.DrawText(str, (size.x - text_size.x) / 2, FromDIP(98));
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
    m_isPressed = true;
    if (!HasCapture()) {
        CaptureMouse();
    }
}

void ImageUploadPanel::onLeftUp(wxMouseEvent& event) 
{
    if (!m_isPressed) {
        event.Skip();
        return;
    }

    bool isFunc = true;
    if (m_path.empty() || !m_img.IsOk()) {
        wxFileDialog  dlg(this, _L("Select Image"), wxGetApp().app_config->get_last_dir(), "",
                          "Image files (*.jpeg;*jpg;*.png;*.webp)|*.jpeg;*.jpg;*.png;*.webp", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        wxArrayString files;
        if (dlg.ShowModal() != wxID_OK)
            return;
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

    m_isPressed = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void ImageUploadPanel::onMouseCaptureLost(wxMouseCaptureLostEvent& event) 
{
    m_isPressed = false;
    Refresh();
}

void ImageUploadPanel::OnMouseEnter(wxMouseEvent& event) 
{
    m_isHovered = true;
    SetCursor(wxCURSOR_HAND);
    Refresh();
}

void ImageUploadPanel::OnMouseLeave(wxMouseEvent& event) 
{
    m_isHovered = false;
    SetCursor(wxCURSOR_ARROW);
    Refresh();
}

ModelApiDialog::ModelApiDialog(wxWindow* parent) : 
    FFTitleLessDialog(parent), m_generate_btn_rect(0, 0, 0, 0), m_question_link_rect(0, 0, 0, 0), m_isPressed(false)
{
    this->SetSize(FromDIP(wxSize(393, 400)));
    this->SetMinSize(FromDIP(wxSize(393, 400)));
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
            return;
        }
        std::string promoData;
        auto        language = wxGetApp().app_config->get_language_code();
        ret                  = MultiComHelper::inst()->getPromoShareData(language.substr(0, 2), promoData, 15000);
        if (ret != COM_OK) {
            return;
        }
        task->safeFunc([task, data, promoData]() {
            auto event          = new FinishScoreEvent();
            event->curCostScore = data.currAiGeneratePoints;
            event->totalScore   = data.totalPoints;
            event->promoData    = promoData;
            wxQueueEvent(task->Parent(), event);
        });
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
    auto sizer      = new wxBoxSizer(wxVERTICAL);
    m_image_panel              = new ImageUploadPanel(this);
    m_image_panel->Bind(EVT_LOADED_IMAGE, [=](wxCommandEvent& event) { Refresh(); });
    sizer->AddSpacer(FromDIP(107));
    sizer->Add(m_image_panel, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(134));
    sizer->Fit(this);
    SetSizer(sizer);
    Layout();
    Center();

    Bind(wxEVT_LEFT_DOWN, &ModelApiDialog::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ModelApiDialog::onLeftUp, this);
    Bind(wxEVT_MOTION, &ModelApiDialog::OnMouseMove, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &ModelApiDialog::onMouseCaptureLost, this);
    m_loadIcon->Loading(200);
}

void ModelApiDialog::drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc)
{
    FFTitleLessDialog::drawBackground(dc, gc);
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    auto size = this->GetClientSize();
    gc->DrawBitmap(m_bmp_map["bg"].bmp(), 0, 0, size.x, size.y);
    drawCenterText(dc, gc, _L("AI 3D Model Generator"), FromDIP(40), Label::Body_14, wxColor("#333333"));
    drawCenterText(dc, gc, _L("Image Upload Tips"), FromDIP(81), Label::Body_11, wxColor("#333333"), "question_mark");
    if (m_loadIcon->isLoading()) {
        const int loadSize = FromDIP(40);
        m_loadIcon->paintInRect(gc, wxRect((size.x - loadSize) / 2, FromDIP(283), loadSize, loadSize));
    } else {
        drawCenterText(dc, gc, m_cost_text, FromDIP(283), Label::Body_12, wxColor("#333333"));
        drawCenterText(dc, gc, m_score_text, FromDIP(306), Label::Body_12, wxColor("#419488"));
    }
    if (m_loadIcon->isLoading() || m_image_panel->getPath().empty()) {
        gc->SetBrush(wxColor("#D2D2D2"));
    }
    else {
        gc->SetBrush(wxColor("#419488"));
        if (m_isGenerateHovered) {
            gc->SetBrush(wxColor("#65A79E"));
        }
        if (m_isPressed) {
            gc->SetBrush(wxColor("#1A8676"));
        }
    }
    wxString btn_text(_L("Start generating"));
    dc.SetFont(Label::Body_12);
    auto btn_text_size = dc.GetTextExtent(btn_text);
    wxSize btn_size(FromDIP(10) * 2 + btn_text_size.x, FromDIP(30));
    if (m_generate_btn_rect.IsEmpty()) {
        m_generate_btn_rect = wxRect((size.x - btn_size.x) / 2, FromDIP(339), btn_size.x, btn_size.y);
    }
    gc->DrawRoundedRectangle((size.x - btn_size.x) / 2, FromDIP(339), btn_size.x, btn_size.y, 4);
    dc.SetTextForeground(*wxWHITE);
    dc.DrawText(btn_text, (size.x - btn_text_size.x) / 2, FromDIP(346));
}

ModelApiDialog::~ModelApiDialog() 
{ 
    m_loadIcon->End();
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
    
    if (iconName.empty()) {
        dc.DrawText(str, (size.x - text_size.x) / 2, height);
    }
    else {
        auto& bmp = m_bmp_map[iconName.ToStdString()].bmp();
        const int icon_sper = 5;
        gc->DrawBitmap(bmp, (size.x - text_size.x - bmp.GetWidth() - icon_sper) / 2, height, bmp.GetWidth(), bmp.GetHeight());
        dc.DrawText(str, (size.x - text_size.x - bmp.GetWidth() - icon_sper) / 2 + icon_sper + bmp.GetWidth(), height);
        if (iconName == "question_mark" && m_question_link_rect.IsEmpty()) {
            m_question_link_rect = wxRect((size.x - text_size.x - bmp.GetWidth() - icon_sper) / 2, height, 
                text_size.x + bmp.GetWidth() + icon_sper, bmp.GetHeight());
        }
    }
}

void ModelApiDialog::onLeftDown(wxMouseEvent& event) 
{
    if (m_generate_btn_rect.IsEmpty()) {
        event.Skip();
        return;
    }
    if (!m_generate_btn_rect.Contains(event.GetPosition())) {
        event.Skip();
        return;
    }
    m_isPressed = true;
    Refresh();
    if (!HasCapture()) {
        CaptureMouse();
    }
}

void ModelApiDialog::onLeftUp(wxMouseEvent& event) 
{
    if (m_generate_btn_rect.IsEmpty()) {
        event.Skip();
        return;
    }
    if (!m_isPressed) {
        event.Skip();
        return;
    }
    if (!m_image_panel->getPath().empty() && m_generate_btn_rect.Contains(event.GetPosition())) {
        GenerateClicked();
    }
    m_isPressed = false;
    Refresh();
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void ModelApiDialog::onMouseCaptureLost(wxMouseCaptureLostEvent& event) 
{ 
    m_isPressed = false;
    Refresh();
    event.Skip();
}

void ModelApiDialog::OnMouseMove(wxMouseEvent& event) 
{
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
    }
    else if (!m_isQuestionHovered && m_question_link_rect.Contains(event.GetPosition())) {
        m_isQuestionHovered = true;
        SetCursor(wxCURSOR_HAND);
        m_question_dialog->Move(this->ClientToScreen(wxPoint((GetClientSize().x - m_question_dialog->GetSize().x) / 2, FromDIP(106))));
        m_question_dialog->Show(true);
    }
    
    event.Skip();
}

void ModelApiDialog::GenerateClicked() 
{ 
    if (m_total_score - m_cost_score < 0) {
        WarningDialog dlg(this, _L("Insufficient points"), _L("Warning"));
        dlg.ShowModal();
        if (m_promoData != "") {
            PromoShareDlg pro(this, m_promoData);
            pro.ShowModal();
        }
        return;
    }
    if (!ifstream(this->m_image_panel->getPath()).good()) {
        GUI::show_error(this, _L("Failed to load image"));
        return;
    }
    Close();
    ModelGenerateDialog dlg(this->m_parent); 
    dlg.SetImgPath(this->m_image_panel->getPath());
    dlg.ShowModal();
}

void ModelApiDialog::RefreshScore(int cost, int total) 
{
    if (cost < 0 || total < 0) {
        BOOST_LOG_TRIVIAL(error) << "AI MODEL: cost score or total score should be nonnegative number";
        return;
    }
    m_cost_score = cost;
    m_total_score = total;

    if (cost <= 0) {
        m_cost_text = wxString(_L("This generation is free"));
    } else {
        m_cost_text = wxString(_L("Points consumed")) + wxString::Format(wxT(":  %d"), cost);
    }

    if (total >= 0) {
        m_score_text = wxString(_L("Remaining points") + wxString::Format(wxT(":  %d"), total));
    }
    Refresh();
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

    m_generateTask->setThreadFunc([task = this->m_generateTask, img_path = this->m_img_path]() {
        const std::string generateFormat = "GLB";
        const int         maxNetworkErrorCount = 5;
        const int         msTimeout = 15000;

        auto imgName = fs::path(img_path.ToStdString()).filename().string();
        std::string img_url = "";
        auto        callback_func = [](long long now, long long total, void* data) { 
            std::atomic_bool* isFinish = static_cast<std::atomic_bool*>(data);
            if (isFinish->load()) {
                return -1;
            }
            return 0;
        };
        ComErrno ret = COM_OK;
        ret = MultiComHelper::inst()->uploadAiImageClound(img_path.ToStdString(), imgName, img_url, 
            callback_func, &task->FinishLoop(), msTimeout);
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
        ret = MultiComHelper::inst()->startAiModelJob(img_url, generateFormat, result, msTimeout);
        if (ret != COM_OK) {
            task->safeFunc([task]() {
                auto event = new wxCommandEvent(EVT_ERROR_MSG);
                event->SetString(_L("Network Error"));
                event->SetInt(1);
                wxQueueEvent(task->Parent(), event);
            });
            return;
        }
        if (result.isOldJob) {
            task->safeFunc([=]() {
                wxQueueEvent(task->Parent(), new wxCommandEvent(EVT_OLD_TASK));
                //task->Sem().Wait();
            });
        }
        const int64_t job_id = result.jobId;
        BOOST_LOG_TRIVIAL(info) << "AI MODEL: CURRENT JOB ID ------ " << job_id;
        task->safeFunc([=]() {
            auto event = new wxCommandEvent(EVT_SET_ID);
            event->SetInt(job_id);
            wxQueueEvent(task->Parent(), event);
        });
        bool isFirstLoop = true;
        int  networkErrorCount    = 0;
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
                        event->SetInt(1);
                        wxQueueEvent(task->Parent(), event);
                    });
                    return;
                }
            }
            if (isFirstLoop) {
                BOOST_LOG_TRIVIAL(info) << "AI MODEL: CURRENT HUNYUAN JOB_ID ------ " << state.externalJobId;
                isFirstLoop = false;
            }
            if (state.status == 4) {// generating failed
                task->safeFunc([task]() {
                    auto event = new wxCommandEvent(EVT_ERROR_MSG);
                    event->SetString(_L("AI Generating Failed"));
                    event->SetInt(1);
                    wxQueueEvent(task->Parent(), event);
                });
                return;
            }
            if (state.status == 5) {//canceled
                return;
            }
            if (state.status == 3) {//completed
                auto event = new CompleteModelEvent();
                for (auto it : state.models) {
                    if (it.modelType == generateFormat) {
                        event->path = it.modelUrl;
                        event->job_id = job_id;        
                        break;
                    }
                }
                wxQueueEvent(task->Parent(), event);
                return;
            }

            task->safeFunc([=]() {
                auto e          = new ApiSetStateEvent();
                if (state.posInQueue == 0) {
                    e->isQueuePanel = false;
                }
                else {
                    e->isShowQueue = true;
                    e->isQueuePanel = true;
                    e->remainCount  = state.posInQueue;
                    e->totalCount   = state.queueLength;
                }
                wxQueueEvent(task->Parent(), e);
            });
            
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
    });
    Bind(EVT_OLD_TASK, [=](wxCommandEvent& event) { 
        WarningDialog dlg(this, _L("A model is currently being generated. Please wait."), _L("Warning"));
        dlg.Show();
    });
    Bind(EVT_SET_STATE, [=](ApiSetStateEvent& event) { 
        m_remainCount = event.remainCount;
        m_totalCount  = event.totalCount;
        showCurState(event.isQueuePanel, event.isShowQueue);
    });
    Bind(EVT_ERROR_MSG, [=](wxCommandEvent& event) {
        GUI::show_error(this, event.GetString());
        if (event.GetInt() == 1) {
            this->m_loadIcon->End();
            EndModal(wxID_CANCEL);
        }
    });
    Bind(EVT_COMPLETE_MODEL, [=](CompleteModelEvent& event) { 
        wxGetApp().update_user_points();
        m_download_path = (boost::filesystem::path(wxStandardPaths::Get().GetTempDir().ToStdString()) /
            ("hunyuan_" + event.job_id + ".glb")).string();
        m_download_tool.downloadDisk(event.path, m_download_path, 100000, 6000000);
    });
    m_download_tool.Bind(EVT_FF_DOWNLOAD_FINISHED, [this](FFDownloadFinishedEvent& event) {
        if (!event.succeed) {
            GUI::show_error(this, _L("AI Generating Failed"));
            return;
        }
        auto        task = this->m_generateTask;
        std::string path = this->m_download_path;
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
    Bind(EVT_CHOICE_COLOR, [=](ChoiceColorEvent& event) {
        this->m_loadIcon->End();
        Close();
        ModelColorDialog dlg(this->m_parent);
        dlg.setDownloadFile(m_download_path);
        dlg.setModelData(event.data);
        dlg.changeColor(event.colors);
        //cvt_colors_t colors = {{30, 141, 213}, {249, 225, 129}, {166, 175, 182}, {41, 41, 41}};
        //dlg.changeColor(colors);
        dlg.ShowModal(); 
    });
    Bind(EVT_SET_ID, [job_id = this->m_job_id](wxCommandEvent& event) { 
        *job_id = event.GetInt();
    });
    m_abortTask->setThreadFunc([task = this->m_abortTask, job_id = this->m_job_id]() {
        auto ret = MultiComHelper::inst()->abortAiModelJob(*job_id, 10000);
        if (ret != COM_OK) {
            task->safeFunc([task]() {
                auto event = new wxCommandEvent(EVT_ERROR_MSG);
                event->SetString(_L("Network Error"));
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
        if (m_job_id < 0) {
            event.Skip();
            return;
        }
        if (!m_isQueuePanel) {
            event.Skip();
            return;
        }
        m_abortTask->start();
        event.Veto();
    });
    Bind(EVT_REAL_CLOSE, [=](wxCommandEvent& event) { 
        EndModal(wxID_CANCEL);
    });

    m_generateTask->start();
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
    auto title = new Label(this, Label::Body_14, _L("Generation successful!"));
    title->SetBackgroundColour(*wxWHITE);
    auto inputLabel = new Label(this, Label::Body_13, _L("You can specify the number of colors for the model."));
    inputLabel->SetBackgroundColour(*wxWHITE);
    m_text_ctrl     = new wxTextCtrl(this, wxID_ANY, "4", wxDefaultPosition, FromDIP(wxSize(24, 24)), wxBORDER_SIMPLE | wxTE_CENTRE);
    m_text_ctrl->SetBackgroundColour(*wxWHITE);
    m_text_ctrl->SetFont(Label::Body_13);
    m_text_ctrl->SetMaxLength(1);
    wxTextValidator validator(wxFILTER_DIGITS, nullptr);
    m_text_ctrl->SetValidator(validator);
    m_text_ctrl->Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
        wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
        wxString    str      = textCtrl->GetValue();
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
        if (!input_char.ToLong(&value))
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
        m_btn->Hide();
        m_loadIcon->Loading(200);
        m_convertTask->start();
    });
    m_loadIcon = std::make_shared<ApiLoadingIcon>(this);
    m_loadIcon->Bind(EVT_UPDATE_ICON, [=](wxCommandEvent& event) { this->Refresh(); });
    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddSpacer(FromDIP(32));
    sizer->Add(title, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(16));
    auto h_sizer = new wxBoxSizer(wxHORIZONTAL);
    h_sizer->Add(inputLabel, 0, wxALL | wxALIGN_CENTER, 0);
    h_sizer->AddSpacer(FromDIP(10));
    h_sizer->Add(m_text_ctrl, 0, wxALL | wxALIGN_CENTER, 0);
    sizer->Add(h_sizer, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(83));
    sizer->Add(m_btn, 0, wxALIGN_CENTER, 0);
    sizer->AddSpacer(FromDIP(32));
    SetSizer(sizer);
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
        dc.SetFont(Label::Body_11);
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

void ModelColorDialog::setModelData(std::shared_ptr<convert_model_data_t>& data) 
{ 
    this->m_modelData = data; 
    m_convertTask     = std::make_shared<ModelApiTask>(this);
    Bind(EVT_COMPLETE_CONVERT, [=](CompleteConvertEvent& event) {
        Close();
        std::vector<std::string> arr;
        arr.emplace_back(event.obj_path);
        wxGetApp().plater()->load_files(arr, LoadStrategy::LoadModel, false, event.colors);
    });
    m_convertTask->setThreadFunc([task = m_convertTask, data = m_modelData, count = m_last_color_count, path = m_filepath]() {
        std::this_thread::sleep_for(std::chrono::seconds(8));
        ConvertModel cm;
        auto         color            = cm.clusterColors(*data, count);
        std::string  convert_obj_file = path;
        std::string  extension        = fs::path(path).extension().string();
        auto         just_filename    = path.substr(0, path.size() - extension.size()) + "_convert";
        size_t       version          = 0;
        convert_obj_file              = just_filename;
        auto tempdir                  = wxStandardPaths::Get().GetTempDir().ToStdString();
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
}

void ModelColorDialog::setDownloadFile(const std::string& path) 
{ 
    this->m_filepath = path; 
}

ModelColorDialog::~ModelColorDialog() 
{
    m_loadIcon->End();
    wxEventBlocker              block(this);
    std::lock_guard<std::mutex> lock(m_convertTask->Lock());
    m_convertTask->FinishLoop().store(true);
    m_convertTask.reset();
}

FinishScoreEvent::FinishScoreEvent() : wxCommandEvent(EVT_FINISH_SCORE) {}

CompleteModelEvent::CompleteModelEvent() : wxCommandEvent(EVT_COMPLETE_MODEL) {}

ChoiceColorEvent::ChoiceColorEvent() : wxCommandEvent(EVT_CHOICE_COLOR) {}

CompleteConvertEvent::CompleteConvertEvent() : wxCommandEvent(EVT_COMPLETE_CONVERT) {}

} // namespace GUI
} // namespace Slic3r::GUI


