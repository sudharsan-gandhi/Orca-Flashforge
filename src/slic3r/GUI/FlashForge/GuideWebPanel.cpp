#include "GuideWebPanel.h"
#include <wx/graphics.h>

#define LOADING_INTERVAL 200

namespace Slic3r { namespace GUI {
GuideWebPanel::GuideWebPanel(wxWindow* parent, wxWindowID id) : 
	wxPanel(parent, id, wxDefaultPosition, wxDefaultSize), m_url("https://www.baidu.com/")
{ 
    for (int i = 0; i < 4; i++) {
        auto           str = boost::format("web_loading_%1%") % (i + 1);
        ScalableBitmap bmp(this, str.str(), FromDIP(60));
        m_loadingIcons.emplace_back(std::move(bmp));
    }

    SetBackgroundStyle(wxBG_STYLE_PAINT); 
	auto* sizer = new wxBoxSizer(wxVERTICAL);
    m_prepareTimer = new wxTimer(this);
    Bind(wxEVT_TIMER, &GuideWebPanel::OnTimer, this);
    Bind(wxEVT_PAINT, &GuideWebPanel::OnPaint, this);
    m_web_view        = WebView::CreateWebView(this, m_url);
    m_web_view->SetSize(GetClientSize());
    sizer->Add(m_web_view, wxSizerFlags().Expand().Proportion(1));
    m_error_panel                 = new wxPanel(this, wxID_ANY);
    wxPanel* error_content = new wxPanel(m_error_panel, wxID_ANY);
    wxBoxSizer* error_sizer   = new wxBoxSizer(wxVERTICAL);
    ScalableButton* error_icon    = new ScalableButton(m_error_panel, wxID_ANY, "web_error", "", FromDIP(wxSize(60, 60)), wxDefaultPosition, 2097153L, false, 60);
    auto            error_text    = new wxStaticText(m_error_panel, wxID_ANY, _L("Load failed, try again"));
    auto            again_btn     = new FFButton(m_error_panel, wxID_ANY, _L("Try Again"), 8);
    error_icon->SetBackgroundColour(*wxWHITE);
    auto font = error_text->GetFont();
    font.SetPointSize(FromDIP(20));
    error_text->SetFont(font);
    again_btn->SetFont(font);
    again_btn->SetMinSize(FromDIP(wxSize(160, 60)));
    again_btn->SetSize(FromDIP(wxSize(160, 60)));
    again_btn->SetBorderColor(wxColor(65, 148, 136));
    again_btn->SetFontColor(wxColor(65, 148, 136));
    again_btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evnet) { 
        WebView::LoadUrl(m_web_view, m_url);
    });
    error_sizer->AddStretchSpacer();
    error_sizer->Add(error_icon, 0, wxCENTER, FromDIP(20));
    error_sizer->AddSpacer(FromDIP(20));
    error_sizer->Add(error_text, 0, wxCENTER, FromDIP(10));
    error_sizer->AddSpacer(FromDIP(15));
    error_sizer->Add(again_btn, 0, wxCENTER, FromDIP(10));
    error_sizer->AddStretchSpacer();
    m_error_panel->SetSizer(error_sizer);
    sizer->Add(m_error_panel, wxSizerFlags().Expand().Proportion(1));
    SetSizer(sizer);
    m_web_view->Hide();
    m_error_panel->Hide();
    Layout();
    Bind(wxEVT_WEBVIEW_NAVIGATING, [&](wxWebViewEvent& event) {
        m_url = event.GetURL().utf8_string();
        m_web_view->Hide();
        m_error_panel->Hide();
        Layout();
        m_status = PREPARE;
        m_loadingIdx = 0;
        m_prepareTimer->Start(LOADING_INTERVAL);
        /*if (event.GetURL() == "https://www.flashforge.com/") {
            return;
        }
        if (wxLaunchDefaultBrowser(event.GetURL())) {
            event.Veto();
        }*/
    });
    Bind(wxEVT_WEBVIEW_ERROR, [&](wxWebViewEvent& event) { 
        m_status = NG; 
        m_prepareTimer->Stop();
        m_error_panel->Show();
        Layout();
    });
    Bind(wxEVT_WEBVIEW_LOADED, [&](wxWebViewEvent& event) { 
        m_status = NORMAL;
        m_web_view->Show();
        m_prepareTimer->Stop();
        Layout();
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
    PrepareDC(dc);
    wxGraphicsContext*    gc = wxGraphicsContext::Create(dc);
    if (!gc)
        return;
    
    if (m_status == PREPARE) {
        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        gc->SetBrush(wxColor(*wxWHITE));
        gc->DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        int     radius     = 40;
        int     rangeAngle = 120;
        wxPoint center(GetClientSize().x / 2, GetClientSize().y / 2);
        wxBitmap& bmp = m_loadingIcons[m_loadingIdx].bmp();
        gc->DrawBitmap(bmp, center.x - bmp.GetWidth() / 2, center.y - bmp.GetHeight(), bmp.GetWidth(), bmp.GetHeight());
        wxFont font;
        font.SetPointSize(FromDIP(20));
        gc->SetFont(font, *wxBLACK);
        gc->DrawText(_L("Loading..."), center.x - FromDIP(50), center.y + FromDIP(15));
    }
    else {
        wxPanel::OnPaint(event);
    }
    delete gc;
}

void GuideWebPanel::OnTimer(wxTimerEvent& event) 
{
    if (m_status == PREPARE) {
        m_loadTime++;
        m_loadingIdx = (m_loadingIdx + 1) % m_loadingIcons.size();
        if (m_loadTime > 10000 / LOADING_INTERVAL) {
            m_status = NG;
            m_loadingIdx = 0;
            m_prepareTimer->Stop();
            m_web_view->Stop();
            m_web_view->Hide();
            m_error_panel->Show();
            m_loadTime = 0;
            Layout();
        }
        Refresh();
    } else if (m_status == NORMAL) {
        m_loadTime = 0;
    }
}

} // namespace GUI
} // namespace Slic3r
