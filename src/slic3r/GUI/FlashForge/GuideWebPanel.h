#ifndef GUIDE_WEB_PANEL_H
#define GUIDE_WEB_PANEL_H

#include <wx/wx.h>
#include <wx/dcclient.h>
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/I18N.hpp"
#include <slic3r/GUI/wxExtensions.hpp>
#include <slic3r/GUI/Widgets/WebView.hpp>

namespace Slic3r { namespace GUI {
    
class GuideWebPanel : public wxPanel
{
public:
    GuideWebPanel(wxWindow* parent, wxWindowID id);
    void OnPaint(wxPaintEvent& event);
    void OnTimer(wxTimerEvent& event);

private:
    enum WebState { NORMAL, PREPARE, NG };
    wxWebView* m_web_view;
    int m_angle = 0;
    int        m_loadTime = 0;
    wxTimer*   m_prepareTimer;
    WebState   m_status = NORMAL;
};


}} // namespace Slic3r::GUI

#endif