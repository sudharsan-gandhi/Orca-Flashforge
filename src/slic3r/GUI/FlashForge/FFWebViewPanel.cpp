#include "FFWebViewPanel.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <wx/sizer.h>
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"

namespace Slic3r { namespace GUI {

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

    m_navBackBtn = new FFPushButton(m_modelNavPnl, wxID_ANY, "mall_control_back", "mall_control_back", "mall_control_back", "mall_control_back", 20);
    m_navBackBtn->SetBackgroundColour(*wxWHITE);
    m_navBackBtn->SetSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->SetMinSize(wxSize(FromDIP(20), FromDIP(20)));
    m_navBackBtn->SetMaxSize(wxSize(FromDIP(20), FromDIP(20)));

    m_navDetailLbl = new wxStaticText(m_modelNavPnl, wxID_ANY, "model_detail");

    m_navMoreBtn = new FFPushButton(m_modelNavPnl, wxID_ANY, "link_more_error_open", "link_more_error_open", "link_more_error_open", "link_more_error_open", 26);
    m_navMoreBtn->SetBackgroundColour(*wxWHITE);
    m_navMoreBtn->SetSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->SetMinSize(wxSize(FromDIP(26), FromDIP(26)));
    m_navMoreBtn->SetMaxSize(wxSize(FromDIP(26), FromDIP(26)));

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
