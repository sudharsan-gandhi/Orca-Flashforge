#ifndef slic3r_FFWebViewPanel_hpp_
#define slic3r_FFWebViewPanel_hpp_

#include <memory>
#include <vector>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include "slic3r/GUI/FlashForge/FFTransientWindow.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/Widgets/WebView.hpp"
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class NavMoreMenu : public FFTransientWindow
{
public:
    NavMoreMenu(wxWindow *parent);

    void AddItem(const std::string &icon, int iconHeight, const wxString &text);

private:
    static const int IconSpace = 6;
    static const int ItemHeight = 45;

    void OnPaint(wxPaintEvent &evt);
    void OnLeftUp(wxMouseEvent &evt);
    void OnMotion(wxMouseEvent &evt);

private:
    int m_hoverItemIndex;
    std::vector<std::unique_ptr<ScalableBitmap>> m_iconBmps;
    std::vector<wxString> m_texts;
    std::vector<wxSize> m_textSizes;
};

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
    void OnShowModelMore(wxCommandEvent &evt);
    void OnNavigating(wxWebViewEvent &evt);
    void OnNewWindow(wxWebViewEvent &evt);
    void OnScriptMessageReceived(wxWebViewEvent &evt);

private:
    wxPanel         *m_modelNavPnl;
    FFPushButton    *m_navBackBtn;
    wxStaticText    *m_navDetailLbl;
    FFPushButton    *m_navMoreBtn;
    FFButton        *m_navPrintListBtn;
    NavMoreMenu     *m_navMoreMenu;
    wxWebView       *m_browser;
};

}} // namespace Slic3r::GUI

#endif
