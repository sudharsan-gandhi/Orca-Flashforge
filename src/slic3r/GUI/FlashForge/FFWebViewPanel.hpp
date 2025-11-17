#ifndef slic3r_FFWebViewPanel_hpp_
#define slic3r_FFWebViewPanel_hpp_

#include <wx/panel.h>
#include <wx/string.h>
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
    bool Initialize();
    void SetupLayout();
    void OnNavigating(wxWebViewEvent &evt);
    void OnNewWindow(wxWebViewEvent &evt);
    void OnScriptMessageReceived(wxWebViewEvent &evt);

private:
    wxWebView *m_browser;
};

}} // namespace Slic3r::GUI

#endif
