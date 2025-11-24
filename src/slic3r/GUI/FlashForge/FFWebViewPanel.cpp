#include "FFWebViewPanel.hpp"
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <wx/object.h>
#include <wx/sizer.h>
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

namespace Slic3r { namespace GUI {

NavMoreMenu::NavMoreMenu(wxWindow *parent)
    : FFTransientWindow(parent)
    , m_hoverItemIndex(-1)
{
    SetFont(Label::Body_14);
    Bind(wxEVT_PAINT, &NavMoreMenu::OnPaint, this);
    Bind(wxEVT_LEFT_UP, &NavMoreMenu::OnLeftUp, this);
    Bind(wxEVT_MOTION, &NavMoreMenu::OnMotion, this);
}

void NavMoreMenu::AddItem(const std::string &icon, int iconHeight, const wxString &text)
{
    int iconWidth;
    if (icon.empty()) {
        m_iconBmps.emplace_back(nullptr);
        iconWidth = 0;
    } else {
        m_iconBmps.emplace_back(std::make_unique<ScalableBitmap>(this, icon, iconHeight));
        iconWidth = m_iconBmps.back()->GetBmpWidth() + FromDIP(IconSpace);
    }
    m_texts.emplace_back(text);

    wxScreenDC dc;
    dc.SetFont(GetFont());
    m_textSizes.emplace_back(dc.GetTextExtent(text));
    
    int width = std::max(FromDIP(160), m_textSizes.back().x + iconWidth + FromDIP(16));
    int height = FromDIP(ItemHeight) * m_texts.size() + 2;
    SetSize(wxSize(width, height));
    SetMinSize(wxSize(width, height));
    SetMaxSize(wxSize(width, height));
}

void NavMoreMenu::OnPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    int width = GetSize().x;
    for (size_t i = 0; i < m_iconBmps.size(); ++i) {
        int itemHeightDIP = FromDIP(ItemHeight);
        int roundedHeight = m_radius * 2 + 1;
        int itemY = i * itemHeightDIP + 1;
        if (i == m_hoverItemIndex) {
            if (m_iconBmps.size() == 1) {
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.SetBrush(wxColour("#d9eaff"));
                dc.DrawRoundedRectangle(1, itemY, width - 2, itemHeightDIP, m_radius);
            } else if (i == 0) {
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.SetBrush(wxColour("#d9eaff"));
                dc.DrawRoundedRectangle(1, itemY, width - 2, roundedHeight, m_radius);
                dc.DrawRectangle(1, itemY + m_radius, width - 2, itemHeightDIP - m_radius);
            } else if (i == m_iconBmps.size() - 1) {
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.SetBrush(wxColour("#d9eaff"));
                dc.DrawRectangle(1, itemY, width - 2, itemHeightDIP - m_radius);
                dc.DrawRoundedRectangle(1, itemY + itemHeightDIP - roundedHeight, width - 2, roundedHeight, m_radius);
            } else {
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.SetBrush(wxColour("#d9eaff"));
                dc.DrawRectangle(1, itemY, width - 2, itemHeightDIP);
            }
        }
        if (m_iconBmps[i].get() == nullptr) {
            int x = (width - m_textSizes[i].x) / 2;
            int y = (itemHeightDIP - m_textSizes[i].y) / 2 + itemY;
            dc.DrawText(m_texts[i], x, y);
        } else {
            int contentWidth = m_iconBmps[i]->GetBmpWidth() + FromDIP(IconSpace) + m_textSizes[i].GetWidth();
            int iconX = (width - contentWidth) / 2;
            int iconY = (itemHeightDIP - m_iconBmps[i]->GetBmpHeight()) / 2 + itemY;
            int textX = iconX + m_iconBmps[i]->GetBmpWidth() + FromDIP(IconSpace);
            int textY = (itemHeightDIP - m_textSizes[i].y) / 2 + itemY;
            dc.DrawBitmap(m_iconBmps[i]->bmp(), iconX, iconY);
            dc.DrawText(m_texts[i], textX, textY);
        }
    }
    dc.SetPen(wxColour("#c1c1c1"));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRoundedRectangle(GetRect(), m_radius);
}

void NavMoreMenu::OnLeftUp(wxMouseEvent &evt)
{
    evt.Skip();
    if (m_hoverItemIndex == -1) {
        return;
    }
    wxCommandEvent event(wxEVT_MENU);
    event.SetEventObject(this);
    event.SetId(GetId());
    event.SetInt(m_hoverItemIndex);
    wxPostEvent(this, event);
    Show(false);
}

void NavMoreMenu::OnMotion(wxMouseEvent &evt)
{
    evt.Skip();
    int hoverItemIndex;
    wxPoint pos = evt.GetPosition();
    if (HitTest(pos) == wxHT_WINDOW_OUTSIDE) {
        hoverItemIndex = -1;
    } else {
        hoverItemIndex = (pos.y - 1) / FromDIP(ItemHeight);
    }
    if (hoverItemIndex != m_hoverItemIndex) {
        m_hoverItemIndex = hoverItemIndex;
        Refresh();
    }
}

ReportOptionItem::ReportOptionItem(wxWindow *parent, const wxString &text, int id)
    : wxPanel(parent)
    , m_selectedBmp(this, "report_item_selected", 13)
    , m_text(text)
    , m_id(id)
    , m_isHover(false)
    , m_isSelected(false)
{
    wxScreenDC dc;
    dc.SetFont(GetFont());
    m_textSize = dc.GetTextExtent(text);
    int contentWidth = m_textSize.x + FromDIP(Spacing) * 3 + FromDIP(IconSize);
    int minWidth = std::clamp(contentWidth, FromDIP(300), FromDIP(500));

    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);
    SetFont(Label::Body_12);
    SetSize(wxSize(minWidth, FromDIP(Height)));
    SetMinSize(wxSize(minWidth, FromDIP(Height)));
    SetMaxSize(wxSize(-1, FromDIP(Height)));

    Bind(wxEVT_PAINT, &ReportOptionItem::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &ReportOptionItem::OnLeftDown, this);
    Bind(wxEVT_ENTER_WINDOW, &ReportOptionItem::OnEnterWindow, this);
    Bind(wxEVT_LEAVE_WINDOW, &ReportOptionItem::OnLeaveWindow, this);
}

int ReportOptionItem::GetId() const
{
    return m_id;
}

bool ReportOptionItem::IsSelected() const
{
    return m_isSelected;
}

void ReportOptionItem::SetSelected(bool isSelected)
{
    m_isSelected = isSelected;
    Refresh();
}

void ReportOptionItem::OnPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(*wxWHITE);
    gc->DrawRectangle(0, 0, GetSize().x, GetSize().y);
    if (m_isHover || m_isSelected) {
        gc->SetBrush(wxColour("#328DFB"));
        dc.SetTextForeground(*wxWHITE);
    } else {
        gc->SetBrush(wxColour("#F5F5F5"));
        dc.SetTextForeground(wxColour("#3333333"));
    }
    gc->DrawRoundedRectangle(0, 0, GetSize().x, GetSize().y, FromDIP(6));
    if (m_isSelected) {
        int selectedWidth = m_selectedBmp.GetBmpWidth();
        int selectedHeight = m_selectedBmp.GetBmpHeight();
        int selectedX = GetSize().x - Spacing - selectedWidth;
        int selectedY = (GetSize().y - selectedHeight) / 2;
        gc->DrawBitmap(m_selectedBmp.bmp(), selectedX, selectedY, selectedWidth, selectedHeight);
    }
    dc.DrawText(m_text, FromDIP(Spacing), (FromDIP(Height) - m_textSize.y) / 2);
}

void ReportOptionItem::OnLeftDown(wxMouseEvent &evt)
{
    m_isSelected = true;
    wxCommandEvent event(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED);
    event.SetEventObject(this);
    event.SetId(GetId());
    event.SetInt(m_id);
    wxPostEvent(this, event);
    Refresh();
}

void ReportOptionItem::OnEnterWindow(wxMouseEvent &evt)
{
    m_isHover = true;
    Refresh();
}

void ReportOptionItem::OnLeaveWindow(wxMouseEvent &evt)
{
    m_isHover = false;
    Refresh();
}

ReportWindow::ReportWindow(wxWindow *parent, const nlohmann::json &data)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxFRAME_SHAPED | wxBORDER_NONE)
    , m_radius(FromDIP(6))
{
    try {
        Initialize(data);
        m_isOk = true;
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << "ReportWindow parse json Error, "
            << e.what() << ", " << nlohmann::to_string(data);
        m_isOk = false;
    }
}

bool ReportWindow::isOk() const
{
    return m_isOk;
}

void ReportWindow::Initialize(const nlohmann::json &data)
{
    SetBackgroundColour(*wxWHITE);
    wxString windowTitle = wxString::FromUTF8((std::string)data.at("name"));
    m_titleBar = new TitleBar(this, windowTitle, wxColour("#E1E2E6"), m_radius);

    wxString reportTitle = wxString::FromUTF8((std::string)data.at("title"));
    wxFont font = Label::Body_14;
    font.SetWeight(wxFONTWEIGHT_MEDIUM);
    m_reportTitleLbl = new wxStaticText(this, wxID_ANY, reportTitle);
    m_reportTitleLbl->SetForegroundColour(wxColour("#333333"));
    m_reportTitleLbl->SetFont(font);

    const nlohmann::json &itemArr = data.at("options");
    for (size_t i = 0; i < itemArr.size(); ++i) {
        wxString optionText = wxString::FromUTF8((std::string)itemArr[i]["option"]);
        m_optionItems.emplace_back(new ReportOptionItem(this, optionText, itemArr[i]["id"]));
        m_optionItems[i]->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &ReportWindow::OnItemSelected, this);
    }

    m_textCtrl = new FFTextCtrl(this, "", wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE);
    m_textCtrl->SetBackgroundColour(*wxWHITE);
    m_textCtrl->SetFont(Label::Body_12);
    m_textCtrl->SetSize(wxSize(-1, FromDIP(128)));
    m_textCtrl->SetMinSize(wxSize(-1, FromDIP(128)));
    m_textCtrl->SetMaxSize(wxSize(-1, FromDIP(128)));
    m_textCtrl->SetTextHint(wxString::FromUTF8((std::string)data.at("otherTips")));
    m_textCtrl->SetMaxLength(data.at("otherTipsMaxNum"));
    m_textCtrl->Hide();
    m_textCtrl->Bind(wxEVT_TEXT, &ReportWindow::OnTextChanged, this);

    m_textCtrlDummyPnl = new wxPanel(this);
    m_textCtrlDummyPnl->SetBackgroundColour(*wxWHITE);
    m_textCtrlDummyPnl->SetSize(wxSize(-1, FromDIP(128)));
    m_textCtrlDummyPnl->SetMinSize(wxSize(-1, FromDIP(128)));
    m_textCtrlDummyPnl->SetMaxSize(wxSize(-1, FromDIP(128)));

    wxFont reportBtnFont = Label::Body_14;
    reportBtnFont.SetWeight(wxFONTWEIGHT_MEDIUM);
    m_reportBtn = new FFButton(this, wxID_ANY, "", FromDIP(16));
    m_reportBtn->SetBackgroundColour(*wxWHITE);
    m_reportBtn->SetDoubleBuffered(true);
    m_reportBtn->SetFont(reportBtnFont);
    m_reportBtn->SetLabel(_L("Submit"), FromDIP(96), FromDIP(20), FromDIP(32), FromDIP(6));
    m_reportBtn->SetFontUniformColor(*wxWHITE);
    m_reportBtn->SetBorderWidth(0);
    m_reportBtn->SetBGColor(wxColour("#328DFB"));
    m_reportBtn->SetBGHoverColor(wxColour("#48AAFE"));
    m_reportBtn->SetBGPressColor(wxColour("#328DFB"));
    m_reportBtn->SetBGDisableColor(wxColour("#E5E5E5"));
    m_reportBtn->Enable(false);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddSpacer(1);
    sizer->Add(m_titleBar, 0, wxEXPAND | wxLEFT | wxRIGHT, 1);
    sizer->AddSpacer(FromDIP(16));
    sizer->Add(m_reportTitleLbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24));
    sizer->AddSpacer(FromDIP(15));
    for (size_t i = 0; i < m_optionItems.size(); ++i) {
        sizer->Add(m_optionItems[i], 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24));
        sizer->AddSpacer(FromDIP(11));
    }
    sizer->AddSpacer(FromDIP(TextCtrlSpacing));
    sizer->Add(m_textCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24) + FromDIP(TextCtrlSpacing));
    sizer->Add(m_textCtrlDummyPnl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(24) + FromDIP(TextCtrlSpacing));
    sizer->AddSpacer(FromDIP(12) + FromDIP(TextCtrlSpacing));
    sizer->Add(m_reportBtn, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, FromDIP(24));
    sizer->AddSpacer(FromDIP(12));
    SetSizer(sizer);

    Bind(wxEVT_PAINT, &ReportWindow::OnPaint, this);
    Bind(wxEVT_SIZE, &ReportWindow::OnSize, this);
    Layout();
    Fit();
    CenterOnParent();
}

void ReportWindow::OnPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    wxSize size = GetSize();
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxColour("#c1c1c1"));
    dc.DrawRectangle(0, 0, size.x, size.y);

    dc.SetBrush(*wxWHITE);
    dc.DrawRoundedRectangle(1, 1, size.x - 2, size.y - 2, m_radius);

    if (m_textCtrl->IsShown()) {
        wxRect textCtrlRect = m_textCtrl->GetRect();
        int textCtrlSpacingDIP = FromDIP(TextCtrlSpacing);
        int textCtrlX = textCtrlRect.x - textCtrlSpacingDIP;
        int textCtrlY = textCtrlRect.y - textCtrlSpacingDIP;
        int textCtrlWidth = textCtrlRect.width + 2 * textCtrlSpacingDIP;
        int textCtrlHeight = textCtrlRect.height + 2 * textCtrlSpacingDIP;
        dc.SetPen(wxColour("#c1c1c1"));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(textCtrlX, textCtrlY, textCtrlWidth, textCtrlHeight, m_radius);
    }
}

void ReportWindow::OnSize(wxSizeEvent &evt)
{
    evt.Skip();
    wxEventBlocker evtBlocker(this, wxEVT_SIZE);
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, GetSize().x, GetSize().y, m_radius);
    SetShape(path);
}

void ReportWindow::OnItemSelected(wxCommandEvent &evt)
{
    for (size_t i = 0; i < m_optionItems.size(); ++i) {
        if (m_optionItems[i]->GetId() != evt.GetInt()) {
            m_optionItems[i]->SetSelected(false);
        }
    }
    bool showTextCtrl = evt.GetId() == 0;
    if (showTextCtrl != m_textCtrl->IsShown()) {
        m_textCtrl->Show(showTextCtrl);
        m_textCtrlDummyPnl->Show(!showTextCtrl);
        Layout();
        Refresh();
    }
    m_reportBtn->Enable(!showTextCtrl || !m_textCtrl->GetValue().empty());
}

void ReportWindow::OnTextChanged(wxCommandEvent &evt)
{
    evt.Skip();
    m_reportBtn->Enable(!m_textCtrl->IsShown() || !m_textCtrl->GetValue().empty());
}

ViewNowWindow::ViewNowWindow(wxWindow *parent)
    : FFRoundedWindow(parent)
    , m_timer(this)
{
    SetBackgroundColour(*wxWHITE);
    SetSize(wxSize(-1, FromDIP(38)));
    SetMinSize(wxSize(FromDIP(256), FromDIP(38)));
    SetMaxSize(wxSize(-1, FromDIP(38)));

    m_addPrintListTipLbl = new wxStaticText(this, wxID_ANY, "add_print_list_tip");
    m_addPrintListTipLbl->SetForegroundColour(wxColour("#333333"));
    m_addPrintListTipLbl->SetFont(Label::Body_12);

    wxFont viewNowFont = Label::Body_12;
    viewNowFont.SetWeight(wxFONTWEIGHT_MEDIUM);
    m_button = new FFButton(this, wxID_ANY, "", FromDIP(10));
    m_button->SetBackgroundColour(*wxWHITE);
    m_button->SetDoubleBuffered(true);
    m_button->SetFont(viewNowFont);
    m_button->SetLabel(_L("View Now"), FromDIP(52), FromDIP(6), FromDIP(20), FromDIP(4));
    m_button->SetFontUniformColor(*wxWHITE);
    m_button->SetBorderWidth(0);
    m_button->SetBGColor(wxColour("#328DFB"));
    m_button->SetBGHoverColor(wxColour("#48AAFE"));
    m_button->SetBGPressColor(wxColour("#328DFB"));

    Bind(wxEVT_TIMER, [this](wxTimerEvent &) { Hide(); });
    m_button->Bind(wxEVT_BUTTON, &ViewNowWindow::OnViewNow, this);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_addPrintListTipLbl, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(20));
    sizer->AddStretchSpacer(1);
    sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(20));
    SetSizer(sizer);
    Layout();
    Fit();
}

void ViewNowWindow::ShowAutoClose(int msTime)
{
    if (msTime <= 0) {
        return;
    }
    Show();
    m_timer.StartOnce(msTime);
}

void ViewNowWindow::SetTipText(const wxString &text)
{
    m_addPrintListTipLbl->SetLabelText(text);
    Layout();
    Fit();
}

bool ViewNowWindow::IsAutoCloseTimerRunning()
{
    return m_timer.IsRunning();
}

void ViewNowWindow::OnViewNow(wxCommandEvent &evt)
{
    wxCommandEvent event(wxEVT_BUTTON);
    event.SetEventObject(this);
    event.SetId(GetId());
    wxPostEvent(this, event);
    Hide();
}

FFWebViewPanel::FFWebViewPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    if (!InitBrowser()) {
        return;
    }
    InitModelNav();
    m_viewNowWindow = new ViewNowWindow(this);
    CallAfter([this]() {
        wxGetApp().mainframe->Bind(wxEVT_ICONIZE, &FFWebViewPanel::OnMainFrameIconize, this);
        wxGetApp().mainframe->Bind(wxEVT_MOVE, &FFWebViewPanel::OnMainFrameMove, this);
        wxGetApp().mainframe->Bind(wxEVT_SIZE, &FFWebViewPanel::OnMainFrameSize, this);
    });

    wxPanel *spacerLine = new wxPanel(m_modelPnl, wxID_ANY, wxDefaultPosition, wxSize(-1, 1), wxTAB_TRAVERSAL);
    spacerLine->SetForegroundColour(wxColour("#dddddd"));
    spacerLine->SetBackgroundColour(wxColour("#dddddd"));

    wxBoxSizer *modelSizer = new wxBoxSizer(wxVERTICAL);
    modelSizer->Add(m_modelNavPnl, 1, wxEXPAND);
    modelSizer->Add(spacerLine, 0, wxEXPAND);
    modelSizer->Add(m_modelBrowser, 1, wxEXPAND);
    m_modelPnl->SetSizer(modelSizer);
    m_modelPnl->Layout();

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_mainBrowser, 1, wxEXPAND);
    sizer->Add(m_modelPnl, 1, wxEXPAND);
    SetSizer(sizer);
    Layout();
}

void FFWebViewPanel::LoadUrl(const wxString &url)
{
    if (m_mainBrowser == nullptr) {
        return;
    }
    m_mainBrowser->LoadURL(url);
    m_mainBrowser->SetFocus();
}

void FFWebViewPanel::RunScript(const wxString &javascript)
{
    if (m_mainBrowser == nullptr) {
        return;
    }
    WebView::RunScript(m_mainBrowser, javascript);
}

void FFWebViewPanel::SendRecentList(int images)
{
    if (m_mainBrowser == nullptr) {
        return;
    }
    boost::property_tree::wptree data;
    wxGetApp().mainframe->get_recent_projects(data, images);

    boost::property_tree::wptree req;
    req.put(L"sequence_id", "");
    req.put(L"command", L"get_recent_projects");
    req.put_child(L"response", data);

    std::wostringstream oss;
    boost::property_tree::write_json(oss, req, false);
    RunScript(wxString::Format("window.postMessage(%s)", oss.str()));
}

void FFWebViewPanel::ShowModelDeatil(const std::string &data)
{
    try {
        nlohmann::json json = nlohmann::json::parse(data);
        if (json.find("report_config") != json.end()) {
            m_reportConfig = json.at("report_config");
        }
        nlohmann::json &modelDetail = json.at("model_detail");
        SetupPrintListButton(!modelDetail.at("printAdded"));
        m_modelId = modelDetail.at("modelId");
        m_viewNowWindow->SetTipText(wxString::FromUTF8((std::string)json.at("add_print_tip")));
        m_modelBrowser->LoadURL(modelDetail.at("modelUrl"));
        m_mainBrowser->Hide();
        m_modelPnl->Show();
        Layout();
    } catch (std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << "FFWebViewPanel parse json error, " << e.what() << ", " << data;
    }
}

bool FFWebViewPanel::InitBrowser()
{
    wxString homePageUrl = wxGetApp().app_config->get("home_page_url");
    wxString homePageEnableDebug = wxGetApp().app_config->get("home_page_enable_debug");
    if (homePageUrl.empty()) {
        wxString lang = wxGetApp().current_language_code_safe();
        homePageUrl = wxString::Format("file://%s/web/homepage/index.html?lang=%s", from_u8(resources_dir()), lang);
    }
    m_mainBrowser = WebView::CreateWebView(this, homePageUrl);
    if (m_mainBrowser == nullptr) {
        return false;
    }
    m_mainBrowser->EnableAccessToDevTools(homePageEnableDebug == "true" || homePageEnableDebug == "1");

    m_modelPnl = new wxPanel(this);
    m_modelPnl->Hide();
    m_modelBrowser = WebView::CreateWebView(m_modelPnl, "");
    if (m_modelBrowser == nullptr) {
        return false;
    }
    m_modelBrowser->EnableAccessToDevTools(homePageEnableDebug == "true" || homePageEnableDebug == "1");

    Bind(wxEVT_WEBVIEW_NEWWINDOW, &FFWebViewPanel::OnMainNewWindow, this);
    Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &FFWebViewPanel::OnMainScriptMessageReceived, this);
    m_modelPnl->Bind(wxEVT_WEBVIEW_NAVIGATING, &FFWebViewPanel::OnModelNavigating, this);
    m_modelPnl->Bind(wxEVT_WEBVIEW_NEWWINDOW, &FFWebViewPanel::OnModelNewWindow, this);
    return true;
}

void FFWebViewPanel::InitModelNav()
{
    m_modelNavPnl = new wxPanel(m_modelPnl);
    m_modelNavPnl->SetBackgroundColour(*wxWHITE);
    m_modelNavPnl->SetSize(wxSize(-1, FromDIP(52)));
    m_modelNavPnl->SetMinSize(wxSize(-1, FromDIP(52)));
    m_modelNavPnl->SetMaxSize(wxSize(-1, FromDIP(52)));

    m_navBackBtn = new FFPushButton(m_modelNavPnl, wxID_ANY, "model_nav_back", "model_nav_back", "model_nav_back", "model_nav_back", 20);
    m_navBackBtn->SetBackgroundColour(*wxWHITE);
    m_navBackBtn->SetSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->SetMinSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->SetMaxSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->Bind(wxEVT_BUTTON, &FFWebViewPanel::OnBackButton, this);

    wxFont navDetailFont = Label::Head_18;
    navDetailFont.SetWeight(wxFONTWEIGHT_MEDIUM);
    m_navDetailLbl = new wxStaticText(m_modelNavPnl, wxID_ANY, "model_detail");
    m_navDetailLbl->SetForegroundColour(wxColour("#333333"));
    m_navDetailLbl->SetFont(navDetailFont);

    m_navMoreBtn = new FFPushButton(m_modelNavPnl, wxID_ANY, "model_nav_more", "model_nav_more", "model_nav_more", "model_nav_more", 26);
    m_navMoreBtn->SetBackgroundColour(*wxWHITE);
    m_navMoreBtn->SetSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->SetMinSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->SetMaxSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->Bind(wxEVT_BUTTON, &FFWebViewPanel::OnMoreButton, this);

    m_navMoreMenu = new NavMoreMenu(m_modelNavPnl);
    m_navMoreMenu->AddItem("model_nav_report", 20, "report_model");
    m_navMoreMenu->Bind(wxEVT_MENU, &FFWebViewPanel::OnMoreMenu, this);

    wxFont navPrintListFont = Label::Body_16;
    navPrintListFont.SetWeight(wxFONTWEIGHT_MEDIUM);
    m_navPrintListBtn = new FFButton(m_modelNavPnl, wxID_ANY, "", FromDIP(18));
    m_navPrintListBtn->SetBackgroundColour(*wxWHITE);
    m_navPrintListBtn->SetDoubleBuffered(true);
    m_navPrintListBtn->SetFont(navPrintListFont);
    m_navPrintListBtn->Bind(wxEVT_BUTTON, &FFWebViewPanel::OnPrintListButton, this);

    wxBoxSizer *modelNavSizer = new wxBoxSizer(wxHORIZONTAL);
    modelNavSizer->Add(m_navBackBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(14));
    modelNavSizer->Add(m_navDetailLbl, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(10));
    modelNavSizer->Add(m_navMoreBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(10));
    modelNavSizer->AddStretchSpacer(1);
    modelNavSizer->Add(m_navPrintListBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(14));
    m_modelNavPnl->SetSizer(modelNavSizer);
    m_modelNavPnl->Layout();
}

void FFWebViewPanel::SetupPrintListButton(bool printAdded)
{
    if (!printAdded) {
        m_navPrintListBtn->SetLabel("print_list_button", FromDIP(96), FromDIP(20), FromDIP(36), FromDIP(6));
        m_navPrintListBtn->SetFontUniformColor(*wxWHITE);
        m_navPrintListBtn->SetBorderWidth(0);
        m_navPrintListBtn->SetBGColor(wxColour("#328DFB"));
        m_navPrintListBtn->SetBGHoverColor(wxColour("#48AAFE"));
        m_navPrintListBtn->SetBGPressColor(wxColour("#328DFB"));
    } else {
        m_navPrintListBtn->SetLabel("print_list_button", FromDIP(96), FromDIP(20), FromDIP(36), FromDIP(6));
        m_navPrintListBtn->SetFontColor(wxColour("#328DFB"));
        m_navPrintListBtn->SetFontHoverColor(wxColour("#48AAFE"));
        m_navPrintListBtn->SetFontPressColor(wxColour("#328DFB"));
        m_navPrintListBtn->SetBorderWidth(2);
        m_navPrintListBtn->SetBorderColor(wxColour("#328DFB"));
        m_navPrintListBtn->SetBorderHoverColor(wxColour("#48AAFE"));
        m_navPrintListBtn->SetBorderPressColor(wxColour("#328DFB"));
        m_navPrintListBtn->SetBGUniformColor(*wxWHITE);
    }
}

void FFWebViewPanel::MoveViewNowWindow()
{
    int x = m_modelNavPnl->GetRect().GetRight() - m_viewNowWindow->GetSize().x - FromDIP(16);
    int y = m_modelNavPnl->GetRect().GetBottom() + FromDIP(20);
    m_viewNowWindow->Move(ClientToScreen(wxPoint(x, y)));
}

void FFWebViewPanel::OnBackButton(wxCommandEvent &evt)
{
    m_modelPnl->Hide();
    m_mainBrowser->Show();
    if (m_viewNowWindow->IsShownOnScreen()) {
        m_viewNowWindow->Hide();
    }
    Layout();
}

void FFWebViewPanel::OnMoreButton(wxCommandEvent &evt)
{
    int x = m_navMoreBtn->GetRect().x + m_navMoreBtn->GetSize().x / 2 - m_navMoreMenu->GetSize().x / 2;
    int y = m_modelNavPnl->GetRect().height - FromDIP(5);
    m_navMoreMenu->Move(ClientToScreen(wxPoint(x, y)));
    m_navMoreMenu->Show();
}

void FFWebViewPanel::OnPrintListButton(wxCommandEvent &evt)
{
    MoveViewNowWindow();
    m_viewNowWindow->ShowAutoClose(3000);
}

void FFWebViewPanel::OnMoreMenu(wxCommandEvent &evt)
{
    ReportWindow reportWnd(wxGetApp().mainframe, m_reportConfig);
    if (reportWnd.isOk()) {
        reportWnd.ShowModal();
    }
}

void FFWebViewPanel::OnMainNewWindow(wxWebViewEvent &evt)
{
    if (m_mainBrowser == nullptr) {
        return;
    }
    m_mainBrowser->LoadURL(evt.GetURL());
}

void FFWebViewPanel::OnMainScriptMessageReceived(wxWebViewEvent &evt)
{
    if (m_mainBrowser == nullptr) {
        return;
    }
    std::string response = wxGetApp().handle_web_request(evt.GetString().ToUTF8().data());
    response.erase(std::remove(response.begin(), response.end(), '\n'), response.end());
    if (response.empty()) {
        return;
    }
    RunScript(wxString::Format("window.postMessage('%s')", response));
}

void FFWebViewPanel::OnModelNavigating(wxWebViewEvent &evt)
{
    if (m_modelBrowser == nullptr) {
        return;
    }
    fs::path path(into_path(evt.GetURL()));
    const std::regex pattern(R"(^https?:\/\/.*\.(stp|step|stl|oltp|obj|amf|3mf|svg|zip|gcode|g)$)", std::regex::icase);
    if (std::regex_match(path.string(), pattern)) {
        wxGetApp().start_download("orcaflashforge://open/?file=" + path.string());
        evt.Veto();
    }
}

void FFWebViewPanel::OnModelNewWindow(wxWebViewEvent &evt)
{
    if (m_modelBrowser == nullptr) {
        return;
    }
    m_modelBrowser->LoadURL(evt.GetURL());
}

void FFWebViewPanel::OnMainFrameIconize(wxIconizeEvent &evt)
{
    evt.Skip();
    CallAfter([this, isIconized = evt.IsIconized()]() {
        if (m_viewNowWindow->IsAutoCloseTimerRunning()) {
            if (isIconized) {
                m_viewNowWindow->Hide();
            } else {
                MoveViewNowWindow();
                m_viewNowWindow->Show();
            }
        }
    });
}

void FFWebViewPanel::OnMainFrameMove(wxMoveEvent &evt)
{
    evt.Skip();
    CallAfter([this]() {
        if (m_viewNowWindow->IsShownOnScreen()) {
            MoveViewNowWindow();
        }
    });
}

void FFWebViewPanel::OnMainFrameSize(wxSizeEvent &evt)
{
    evt.Skip();
    CallAfter([this]() {
        if (m_viewNowWindow->IsShownOnScreen()) {
            MoveViewNowWindow();
        }
    });
}

}} // namespace Slic3r::GUI
