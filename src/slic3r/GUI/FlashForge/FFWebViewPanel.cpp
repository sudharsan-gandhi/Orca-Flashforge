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
    , m_browser(nullptr)
{
    if (!Initialize()) {
        return;
    }
    SetupLayout();
}

bool FFWebViewPanel::Initialize()
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

void FFWebViewPanel::SetupLayout()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_browser, 1, wxEXPAND);
    SetSizer(sizer);
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
