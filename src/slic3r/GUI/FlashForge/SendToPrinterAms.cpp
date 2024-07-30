#include "SendToPrinterAms.hpp"
#include <string>
#include <wx/dcgraph.h>
#include "slic3r/GUI/GUI_App.hpp"

namespace Slic3r { namespace GUI {

SlotSelectWnd::SlotSelectWnd(wxWindow *parent)
    : PopupWindow(parent, wxBORDER_NONE)
{
    SetSize(wxSize(FromDIP(278), FromDIP(66)));
}

MaterialMatchWgt::MaterialMatchWgt(wxWindow *parent, wxColour color, wxString name)
    : wxPanel(parent)
    , m_color(color)
    , m_name(name)
    , m_amsColor(0xEE, 0xEE, 0xEE)
    , m_amsSlot(0)
    , m_selected(false)
    , m_soltSelectWnd(new SlotSelectWnd(parent))
 {
    m_size = wxSize(FromDIP(64), FromDIP(34));
    m_realSize = wxSize(FromDIP(62), FromDIP(32));
    m_arrawBmpGray =  ScalableBitmap(this, "drop_down", FromDIP(12));
    m_arrawBmpWhite =  ScalableBitmap(this, "topbar_dropdown", FromDIP(12));

    SetSize(m_size);
    SetMinSize(m_size);
    SetMaxSize(m_size);

#ifdef __WINDOWS__
    SetDoubleBuffered(true);
#endif
    SetBackgroundColour(*wxWHITE);
    Bind(wxEVT_PAINT, &MaterialMatchWgt::onPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MaterialMatchWgt::onLeftDown, this);
    wxGetApp().UpdateDarkUI(this);
}

void MaterialMatchWgt::onPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    if (gc != nullptr) {
        drawBackground(gc);
        drawForeground(dc);
        delete gc;
    }
}

void MaterialMatchWgt::onLeftDown(wxMouseEvent &evt)
{
    wxPoint pos = ClientToScreen(wxPoint(0, GetRect().height + FromDIP(2)));
    m_soltSelectWnd->Move(pos);
    m_soltSelectWnd->Popup();
}

void MaterialMatchWgt::drawBackground(wxGraphicsContext *gc)
{
    // top
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_color));
    gc->DrawRoundedRectangle(FromDIP(1), FromDIP(1), m_realSize.x, FromDIP(18), 5);

    // bottom
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(wxColour(m_amsColor)));
    gc->DrawRoundedRectangle(FromDIP(1), FromDIP(18), m_realSize.x, FromDIP(16), 5);
    
    // middle
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_color));
    gc->DrawRectangle(FromDIP(1), FromDIP(11), m_realSize.x, FromDIP(8));

    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_amsColor));
    gc->DrawRectangle(FromDIP(1), FromDIP(18), m_realSize.x, FromDIP(8));

    // border
    wxSize borderSize(m_size.x -1, m_size.y - 1);
    if (m_selected) {
        gc->SetPen(wxColour(0x00, 0xAE, 0x42));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, borderSize.x, borderSize.y, 5);
    } else if (m_color == *wxWHITE || m_amsColor == *wxWHITE) {
        gc->SetPen(wxColour(0xAC, 0xAC, 0xAC));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, borderSize.x, borderSize.y, 5);
    }
}

void MaterialMatchWgt::drawForeground(wxDC &dc)
{
    //arrow
    int arrowX = m_size.x - m_arrawBmpWhite.GetBmpSize().x - FromDIP(7);
    int arrowY = m_size.y - m_arrawBmpWhite.GetBmpSize().y;
    if (m_amsColor.Red() > 160 && m_amsColor.Green() > 160 && m_amsColor.Blue() > 160
     && m_amsColor.Red() < 180 && m_amsColor.Green() < 180 && m_amsColor.Blue() < 180) {
        dc.DrawBitmap(m_arrawBmpWhite.bmp(), arrowX, arrowY);
    } else {
        dc.DrawBitmap(m_arrawBmpGray.bmp(), arrowX, arrowY);
    }

    // material name
    if (m_color.GetLuminance() < 0.6) {
        dc.SetTextForeground(*wxWHITE);
    } else {
        dc.SetTextForeground(wxColour(0x26, 0x2E, 0x30));
    }
    wxSize nameTxtExtent = dc.GetTextExtent(m_name);
    if (nameTxtExtent.x > GetSize().x - FromDIP(10)) {
        dc.SetFont(::Label::Body_10);
    } else {
        dc.SetFont(::Label::Body_13);
    }
    dc.DrawText(m_name, (m_size.x - nameTxtExtent.x) / 2, (FromDIP(22) - nameTxtExtent.y) / 2);

    // mapping slot
    if (m_amsColor.GetLuminance() < 0.6) {
        dc.SetTextForeground(*wxWHITE);
    } else {
        dc.SetTextForeground(wxColour(0x26, 0x2E, 0x30));
    }
    dc.SetFont(::Label::Body_10);
    wxString slotTxt;
    if (m_amsSlot <= 0) {
        slotTxt = "-";
    } else {
        slotTxt = std::to_string(m_amsSlot);
    }
    wxSize slotTxtExtent = dc.GetTextExtent(slotTxt);
    int slotTxtX = (m_size.x - slotTxtExtent.x) / 2;
    int slotTxtY = FromDIP(20) + (FromDIP(14) - slotTxtExtent.y) / 2;
    dc.DrawText(slotTxt, slotTxtX, slotTxtY);
}

}} // namespace Slic3r::GUI
