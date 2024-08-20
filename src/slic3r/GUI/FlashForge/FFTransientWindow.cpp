#include "FFTransientWindow.hpp"
#include <memory>
#include <wx/event.h>
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include "slic3r/GUI/GUI_App.hpp"

namespace Slic3r { namespace GUI {

FFRoundedWindow::FFRoundedWindow(wxWindow *parent)
    : wxPopupWindow(parent, wxBORDER_NONE | wxFRAME_SHAPED)
    , m_radius(FromDIP(6))
{
    SetBackgroundColour(*wxWHITE);
    Bind(wxEVT_SIZE, &FFRoundedWindow::OnSize, this);
    Bind(wxEVT_PAINT, &FFRoundedWindow::OnPaint, this);
}

void FFRoundedWindow::OnSize(wxSizeEvent &evt)
{
    evt.Skip();
    wxEventBlocker evtBlocker(this, wxEVT_SIZE);
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, GetSize().x, GetSize().y, m_radius);
    SetShape(path);
}

void FFRoundedWindow::OnPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    gc->SetPen(wxColour("#c1c1c1"));
    gc->SetBrush(*wxTRANSPARENT_BRUSH);
    gc->DrawRoundedRectangle(0, 0, GetSize().x - 1, GetSize().y - 1, m_radius);
}

FFTransientWindow::FFTransientWindow(wxWindow *parent, bool hasTitle, wxString titleText /* = "" */)
    : FFRoundedWindow(parent)
    , m_titleHeight(hasTitle ? FromDIP(38) : 0)
    , m_titleText(titleText)
{
    Bind(wxEVT_PAINT, &FFTransientWindow::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FFTransientWindow::OnLeftDown, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &FFTransientWindow::OnMouseCaptureLost, this);
    wxGetApp().Bind(wxEVT_ACTIVATE_APP, &FFTransientWindow::OnActivateApp, this);
}

bool FFTransientWindow::Show(bool show /* = true */)
{
    if (FFRoundedWindow::Show(show)) {
        if (show) {
            CaptureMouse();
        } else {
            ReleaseMouse();
        }
        return true;
    }
    return false;
}

void FFTransientWindow::OnPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    if (m_titleHeight != 0) {
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxColour("#e1e2e6"));
        gc->DrawRectangle(0, 0, GetSize().x, m_titleHeight);

        wxSize textSize = dc.GetTextExtent(m_titleText);
        dc.SetTextForeground(wxColour("#333333"));
        dc.DrawText(m_titleText, (GetSize().x - textSize.x) / 2, (m_titleHeight - textSize.y) / 2);
    }
    gc->SetPen(wxColour("#c1c1c1"));
    gc->SetBrush(*wxTRANSPARENT_BRUSH);
    gc->DrawRoundedRectangle(0, 0, GetSize().x - 1, GetSize().y - 1, m_radius);
}

void FFTransientWindow::OnLeftDown(wxMouseEvent &evt)
{
    evt.Skip();
    if (HitTest(evt.GetPosition()) == wxHT_WINDOW_OUTSIDE) {
        Show(false);
    }
}

void FFTransientWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent &evt)
{
    evt.Skip();
    FFRoundedWindow::Show(false);
}

void FFTransientWindow::OnActivateApp(wxActivateEvent& event)
{
    event.Skip();
    if (event.GetActive()) {
        return;
    }
    Show(false);
}

}} // namespace Slic3r::GUI
