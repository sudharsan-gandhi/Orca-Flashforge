#ifndef slic3r_FFWebViewPanel_hpp_
#define slic3r_FFWebViewPanel_hpp_

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/Widgets/WebView.hpp"

namespace Slic3r { namespace GUI {

class FFWebViewPanel : public wxPanel
{
public:
    FFWebViewPanel(wxWindow *parent);

    void LoadUrl(const wxString &url);
    void RunScript(const wxString &javascript);
    void SendRecentList(int images);

private:
    bool InitBrowser();
    void InitModelNav();
    void OnNavigating(wxWebViewEvent &evt);
    void OnNewWindow(wxWebViewEvent &evt);
    void OnScriptMessageReceived(wxWebViewEvent &evt);

private:
    wxPanel      *m_modelNavPnl;
    FFPushButton *m_navBackBtn;
    wxStaticText *m_navDetailLbl;
    FFPushButton *m_navMoreBtn;
    FFButton     *m_navPrintListBtn;
    wxWebView    *m_browser;
};

}} // namespace Slic3r::GUI

#endif
