#include "FFWebViewPanel.hpp"
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <wx/sizer.h>
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"

namespace Slic3r { namespace GUI {

NavMoreMenu::NavMoreMenu(wxWindow *parent)
    : FFTransientWindow(parent)
    , m_hoverItemIndex(-1)
{
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
        int itemY = i * itemHeightDIP + 1;
        if (i == m_hoverItemIndex) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxColour("#d9eaff"));
            dc.DrawRectangle(1, itemY, width - 2, itemHeightDIP);
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
    dc.DrawRoundedRectangle(0, 0, GetSize().x, GetSize().y, m_radius);
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

FFWebViewPanel::FFWebViewPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    if (!InitBrowser()) {
        return;
    }
    InitModelNav();

    wxPanel *spacerLine = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1), wxTAB_TRAVERSAL);
    spacerLine->SetForegroundColour(wxColour("#dddddd"));
    spacerLine->SetBackgroundColour(wxColour("#dddddd"));

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_modelNavPnl, 1, wxEXPAND);
    sizer->Add(spacerLine, 0, wxEXPAND);
    sizer->Add(m_browser, 1, wxEXPAND);
    SetSizer(sizer);
    Layout();
}

bool FFWebViewPanel::InitBrowser()
{
    wxString homePageUrl = wxGetApp().app_config->get("home_page_url");
    wxString homePageEnableDebug = wxGetApp().app_config->get("home_page_enable_debug");
    if (homePageUrl.empty()) {
        wxString lang = wxGetApp().current_language_code_safe();
        homePageUrl = wxString::Format("file://%s/web/homepage/index.html?lang=%s", from_u8(resources_dir()), lang);
    }
    m_browser = WebView::CreateWebView(this, homePageUrl);
    if (m_browser == nullptr) {
        return false;
    }
    m_browser->EnableAccessToDevTools(homePageEnableDebug == "true" || homePageEnableDebug == "1");

    Bind(wxEVT_WEBVIEW_NAVIGATING, &FFWebViewPanel::OnNavigating, this);
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &FFWebViewPanel::OnNewWindow, this);
    Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &FFWebViewPanel::OnScriptMessageReceived, this);
    return true;
}

void FFWebViewPanel::InitModelNav()
{
    m_modelNavPnl = new wxPanel(this);
    m_modelNavPnl->SetBackgroundColour(*wxWHITE);
    m_modelNavPnl->SetSize(wxSize(-1, FromDIP(52)));
    m_modelNavPnl->SetMinSize(wxSize(-1, FromDIP(52)));
    m_modelNavPnl->SetMaxSize(wxSize(-1, FromDIP(52)));

    m_navBackBtn = new FFPushButton(m_modelNavPnl, wxID_ANY, "model_nav_back", "model_nav_back", "model_nav_back", "model_nav_back", 20);
    m_navBackBtn->SetBackgroundColour(*wxWHITE);
    m_navBackBtn->SetSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->SetMinSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->SetMaxSize(wxSize(FromDIP(20), FromDIP(20)));

    m_navDetailLbl = new wxStaticText(m_modelNavPnl, wxID_ANY, "model_detail");

    m_navMoreBtn = new FFPushButton(m_modelNavPnl, wxID_ANY, "model_nav_more", "model_nav_more", "model_nav_more", "model_nav_more", 26);
    m_navMoreBtn->SetBackgroundColour(*wxWHITE);
    m_navMoreBtn->SetSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->SetMinSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->SetMaxSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->Bind(wxEVT_BUTTON, &FFWebViewPanel::OnShowModelMore, this);

    m_navMoreMenu = new NavMoreMenu(m_modelNavPnl);
    m_navMoreMenu->AddItem("model_nav_report", 19, "report_model");

    m_navPrintListBtn = new FFButton(m_modelNavPnl, wxID_ANY, "", FromDIP(18));
    m_navPrintListBtn->SetBackgroundColour(*wxWHITE);
    m_navPrintListBtn->SetLabel("print_list_button", -1, FromDIP(36));

    wxBoxSizer *modelNavSizer = new wxBoxSizer(wxHORIZONTAL);
    modelNavSizer->Add(m_navBackBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(14));
    modelNavSizer->Add(m_navDetailLbl, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(10));
    modelNavSizer->Add(m_navMoreBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(10));
    modelNavSizer->AddStretchSpacer(1);
    modelNavSizer->Add(m_navPrintListBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(14));
    m_modelNavPnl->SetSizer(modelNavSizer);
    m_modelNavPnl->Layout();
}

void FFWebViewPanel::LoadUrl(const wxString &url)
{
    if (!m_browser) {
        return;
    }
    m_browser->LoadURL(url);
    m_browser->SetFocus();
}

void FFWebViewPanel::RunScript(const wxString &javascript)
{
    if (!m_browser) {
        return;
    }
    WebView::RunScript(m_browser, javascript);
}

void FFWebViewPanel::SendRecentList(int images)
{
    if (!m_browser) {
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

void FFWebViewPanel::OnShowModelMore(wxCommandEvent &evt)
{
    int x = m_navMoreBtn->GetRect().x + m_navMoreBtn->GetSize().x / 2 - m_navMoreMenu->GetSize().x / 2;
    int y = m_modelNavPnl->GetRect().height - FromDIP(5);
    m_navMoreMenu->Move(ClientToScreen(wxPoint(x, y)));
    m_navMoreMenu->Show();
}

void FFWebViewPanel::OnNavigating(wxWebViewEvent &evt)
{
    if (!m_browser) {
        return;
    }
    fs::path path(into_path(evt.GetURL()));
    const std::regex pattern(R"(^https?:\/\/.*\.(stp|step|stl|oltp|obj|amf|3mf|svg|zip|gcode|g)$)", std::regex::icase);
    if (std::regex_match(path.string(), pattern)) {
        wxGetApp().start_download("orcaflashforge://open/?file=" + path.string());
        evt.Veto();
    }
}

void FFWebViewPanel::OnNewWindow(wxWebViewEvent &evt)
{
    if (!m_browser) {
        return;
    }
    m_browser->LoadURL(evt.GetURL());
}

void FFWebViewPanel::OnScriptMessageReceived(wxWebViewEvent &evt)
{
    if (!m_browser) {
        return;
    }
    std::string response = wxGetApp().handle_web_request(evt.GetString().ToUTF8().data());
    response.erase(std::remove(response.begin(), response.end(), '\n'), response.end());
    if (response.empty()) {
        return;
    }
    RunScript(wxString::Format("window.postMessage('%s')", response));
}

}} // namespace Slic3r::GUI
