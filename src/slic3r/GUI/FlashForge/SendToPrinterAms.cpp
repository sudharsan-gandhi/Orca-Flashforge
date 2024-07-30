#include "SendToPrinterAms.hpp"
#include <string>
#include <wx/dcgraph.h>
#include "slic3r/GUI/GUI_App.hpp"

namespace Slic3r { namespace GUI {

MaterialMatchWgt::MaterialMatchWgt(wxWindow *parent, wxColour color, wxString name)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
    , m_color(color)
    , m_name(name)
    , m_amsColor(0xEE, 0xEE, 0xEE)
    , m_amsSlot(0)
    , m_selected(false)
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

    Bind(wxEVT_PAINT, [this](wxPaintEvent &evt) {
        wxPaintDC dc(this);
        drawBackground(dc);
        drawForeground(dc);
    });
    wxGetApp().UpdateDarkUI(this);
}

void MaterialMatchWgt::drawBackground(wxDC &dc)
{
    // top
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(m_color));
    dc.DrawRoundedRectangle(FromDIP(1), FromDIP(1), m_realSize.x, FromDIP(18), 5);

    // bottom
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(wxColour(m_amsColor)));
    dc.DrawRoundedRectangle(FromDIP(1), FromDIP(18), m_realSize.x, FromDIP(16), 5);
    
    // middle
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(m_color));
    dc.DrawRectangle(FromDIP(1), FromDIP(11), m_realSize.x, FromDIP(8));

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(m_amsColor));
    dc.DrawRectangle(FromDIP(1), FromDIP(18), m_realSize.x, FromDIP(8));

    // border
#if __APPLE__
    wxSize borderSize(m_size.x -1, m_size.y - 1);
#else
    wxSize borderSize(m_size.x, m_size.y);
#endif
    if (m_selected) {
        dc.SetPen(wxColour(0x00, 0xAE, 0x42));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(0, 0, borderSize.x, borderSize.y, 5);
    } else if (m_color == *wxWHITE || m_amsColor == *wxWHITE) {
        dc.SetPen(wxColour(0xAC, 0xAC, 0xAC));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(0, 0, borderSize.x, borderSize.y, 5);
    }

    //arrow
    int arrowX = m_size.x - m_arrawBmpWhite.GetBmpSize().x - FromDIP(7);
    int arrowY = m_size.y - m_arrawBmpWhite.GetBmpSize().y;
    if (m_amsColor.Red() > 160 && m_amsColor.Green() > 160 && m_amsColor.Blue() > 160
     && m_amsColor.Red() < 180 && m_amsColor.Green() < 180 && m_amsColor.Blue() < 180) {
        dc.DrawBitmap(m_arrawBmpWhite.bmp(), arrowX, arrowY);
    } else {
        dc.DrawBitmap(m_arrawBmpGray.bmp(), arrowX, arrowY);
    }
}

void MaterialMatchWgt::drawForeground(wxDC &dc)
{
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
