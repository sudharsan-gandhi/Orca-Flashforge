#include "FFTransientWindow.hpp"
#include <wx/event.h>
#include <wx/graphics.h>

namespace Slic3r { namespace GUI {

FFTransientWindow::FFTransientWindow(wxWindow *parent, bool hasTitle, wxString titleText /* = "" */)
    : PopupWindow(parent, wxBORDER_NONE | wxFRAME_SHAPED)
    , m_titleHeight(hasTitle ? FromDIP(38) : 0)
    , m_radius(FromDIP(6))
    , m_titleText(titleText)
{
    SetBackgroundColour(*wxWHITE);
    Bind(wxEVT_PAINT, &FFTransientWindow::OnPaint, this);
    Bind(wxEVT_SIZE, &FFTransientWindow::OnSize, this);
}

void FFTransientWindow::OnSize(wxSizeEvent &evt)
{
    wxEventBlocker evtBlocker(this, wxEVT_SIZE);
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, GetSize().x, GetSize().y, m_radius);
    SetShape(path);
    evt.Skip();
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

}} // namespace Slic3r::GUI
