#include "GuideWebPanel.h"
#include <wx/graphics.h>

namespace Slic3r { namespace GUI {
GuideWebPanel::GuideWebPanel(wxWindow* parent, wxWindowID id) : 
	wxPanel(parent, id, wxDefaultPosition, wxDefaultSize) 
{ 
    SetBackgroundStyle(wxBG_STYLE_PAINT); 
	auto* sizer = new wxBoxSizer(wxVERTICAL);
    m_prepareTimer = new wxTimer(this);
    Bind(wxEVT_TIMER, &GuideWebPanel::OnTimer, this);
    Bind(wxEVT_PAINT, &GuideWebPanel::OnPaint, this);
    m_web_view        = WebView::CreateWebView(this, "http://www.flashforge.com");
    m_web_view->SetSize(GetClientSize());
    sizer->Add(m_web_view, wxSizerFlags().Expand().Proportion(1));
    SetSizer(sizer);
    m_web_view->Hide();
    Layout();
    Bind(wxEVT_WEBVIEW_NAVIGATING, [&](wxWebViewEvent& event) {
        m_web_view->Hide();
        m_status = PREPARE;
        m_prepareTimer->Start(20);
        /*if (event.GetURL() == "https://www.baidu.com/") {
            return;
        }
        if (wxLaunchDefaultBrowser(event.GetURL())) {
            event.Veto();
        }*/
    });
    Bind(wxEVT_WEBVIEW_ERROR, [&](wxWebViewEvent& event) { 
        m_status = NG; 
        m_prepareTimer->Stop();
    });
    Bind(wxEVT_WEBVIEW_LOADED, [&](wxWebViewEvent& event) { 
        m_status = NORMAL;
        //m_web_view->Show();
        m_prepareTimer->Stop();
    });
    Bind(wxEVT_WEBVIEW_NEWWINDOW, [&](wxWebViewEvent& event) { 
        //wxLaunchDefaultBrowser("https://github.com/", wxBROWSER_NEW_WINDOW);
        WebView::LoadUrl(m_web_view, event.GetURL());
    });
}

GuideWebPanel::~GuideWebPanel()
{
    m_prepareTimer->Stop();
}

void GuideWebPanel::OnPaint(wxPaintEvent& event) 
{
    wxAutoBufferedPaintDC dc(this);
    wxGraphicsContext*    gc = wxGraphicsContext::Create(dc);
    if (!gc)
        return;
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    gc->SetBrush(wxColor(*wxWHITE));
    gc->DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);
    if (m_status == PREPARE) {
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        int     radius     = 40;
        int     rangeAngle = 120;
        wxPoint center(GetClientSize().x / 2, GetClientSize().y / 2);
        gc->SetPen(wxPen(wxColour(125, 125, 125), 8));
        gc->DrawEllipse(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
        gc->SetPen(wxPen(wxColour("#009688"), 7));
        auto path = gc->CreatePath();
        path.AddArc((wxDouble) center.x, (wxDouble) center.y, (wxDouble) radius, wxDegToRad(m_angle), wxDegToRad(m_angle + rangeAngle), 1);
        gc->DrawPath(path);
    }
    else if (m_status == NG) {
        gc->SetFont(wxFont(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT), wxColour(*wxBLACK));
        gc->DrawText(_L("The Network Not Found, Please Try Again!"), 0, 0);
    }
    delete gc;
}

void GuideWebPanel::OnTimer(wxTimerEvent& event) 
{
    if (m_status == PREPARE) {
        m_loadTime++;
        m_angle = (m_angle + 5) % 360;
        if (m_loadTime > 3000) {
            m_status = NG;
            m_angle  = 0;
            m_prepareTimer->Stop();
        }
        Refresh();
    } else if (m_status == NORMAL) {
        m_loadTime = 0;
    }
}

} // namespace GUI
} // namespace Slic3r
