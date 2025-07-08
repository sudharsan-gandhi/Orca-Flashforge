#include "FFTitleLessDialog.hpp"
#include <memory>
#include <wx/graphics.h>

namespace Slic3r { namespace GUI {

FFTitleLessDialog::FFTitleLessDialog(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxFRAME_SHAPED | wxNO_BORDER)
    , m_radius(FromDIP(6))
    , m_isPressClose(false)
    , m_closeBmp(this, "title_less_close", 12)
{
    SetBackgroundColour(*wxWHITE);
    Bind(wxEVT_PAINT, &FFTitleLessDialog::onPaint, this);
    Bind(wxEVT_SIZE, &FFTitleLessDialog::onSize, this);
    Bind(wxEVT_LEFT_DOWN, &FFTitleLessDialog::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &FFTitleLessDialog::onLeftUp, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &FFTitleLessDialog::onMouseCaptureLost, this);
}

void FFTitleLessDialog::onPaint(wxPaintEvent &event)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    drawBackground(dc, gc.get());
    gc->SetPen(wxColour("#c1c1c1"));
    gc->SetBrush(*wxTRANSPARENT_BRUSH);
    gc->DrawRoundedRectangle(0, 0, GetSize().x - 1, GetSize().y - 1, m_radius);
    gc->DrawBitmap(m_closeBmp.bmp(), m_closeRect.x, m_closeRect.y, m_closeRect.width, m_closeRect.height);
}

void FFTitleLessDialog::onSize(wxSizeEvent &event)
{
    wxEventBlocker blocker(this, wxEVT_SIZE);
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, GetSize().x, GetSize().y, m_radius);
    SetShape(path);

    int margin = FromDIP(10);
    m_closeRect.x = GetSize().x - margin - m_closeBmp.GetBmpSize().x;
    m_closeRect.y = margin;
    m_closeRect.width = m_closeBmp.GetBmpSize().x;
    m_closeRect.height = m_closeBmp.GetBmpSize().y;
}

void FFTitleLessDialog::onLeftDown(wxMouseEvent &event)
{
    if (!m_closeRect.Contains(event.GetPosition())) {
        event.Skip();
        return;
    }
    m_isPressClose = true;
    if (!HasCapture()) {
        CaptureMouse();
    }
}

void FFTitleLessDialog::onLeftUp(wxMouseEvent &event)
{
    if (!m_isPressClose) {
        event.Skip();
        return;
    }
    if (m_closeRect.Contains(event.GetPosition())) {
        EndModal(wxID_CANCEL);
    }
    m_isPressClose = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void FFTitleLessDialog::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
    m_isPressClose = false;
}

}} // Slic3r::GUI
