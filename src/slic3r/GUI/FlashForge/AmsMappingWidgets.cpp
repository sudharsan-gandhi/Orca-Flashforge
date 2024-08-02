#include "AmsMappingWidgets.hpp"
#include <memory>
#include <string>
#include <wx/dcgraph.h>
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

namespace Slic3r { namespace GUI {

wxColour SlotInfoWgt::DisabledColor(0xEE, 0xEE, 0xEE);

SlotInfoWgt::SlotInfoWgt(wxWindow *parent)
    : wxPanel(parent)
    , m_slot(0)
    , m_color(DisabledColor)
    , m_empty(true)
{
    SetSize(wxSize(FromDIP(64), FromDIP(34)));
    SetMinSize(GetSize());
    SetMaxSize(GetSize());
    Enable(false);
    Bind(wxEVT_PAINT, &SlotInfoWgt::onPaint, this);
}

void SlotInfoWgt::setInfo(int slot, wxColour color, wxString name, bool empty)
{
    m_slot = slot;
    m_color = color;
    m_name = name;
    m_empty = empty;
    Enable(!m_empty && !m_name.empty());
    Update();
}

void SlotInfoWgt::onPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    // background
    gc->SetPen(*wxTRANSPARENT_PEN);
    if (m_empty || m_name.empty()) {
        gc->SetBrush(wxBrush(DisabledColor));
    } else {
        gc->SetBrush(wxBrush(m_color));
    }
    wxDouble fromDip1 = FromDIP(1);
    gc->DrawRoundedRectangle(fromDip1, fromDip1, GetSize().x - fromDip1, GetSize().y - fromDip1, 5);

    // border
    if (m_color == *wxWHITE) {
        gc->SetPen(wxColour(0xAC, 0xAC, 0xAC));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, GetSize().x - 1, GetSize().y - 1, 5);
    }

    // slot
    if (m_empty || m_name.empty()) {
        dc.SetTextForeground(*wxWHITE);
    } else {
        dc.SetTextForeground(*wxBLACK);
    }
    dc.SetFont(::Label::Body_10);
    wxString slotTxt = std::to_string(m_slot);
    wxSize slotTxtExtent = dc.GetTextExtent(slotTxt);
    dc.DrawText(slotTxt, (GetSize().x - slotTxtExtent.x) / 2, (FromDIP(22) - slotTxtExtent.y) / 2);

    // name
    wxString nameTxt;
    if (m_empty) {
        nameTxt = "/";
    } else if (m_name.empty()) {
        nameTxt = "?";
    } else {
        nameTxt = m_name;
    }
    wxSize nameTxtExtent = dc.GetTextExtent(nameTxt);
    int nameTxtX = (GetSize().x - nameTxtExtent.x) / 2;
    int nameTxtY = FromDIP(14) + (FromDIP(22) - nameTxtExtent.y) / 2;
    dc.DrawText(nameTxt, nameTxtX, nameTxtY);
}

wxDEFINE_EVENT(SOLT_SELECT_EVENT, SlotSelectEvent);

SlotSelectWnd::SlotSelectWnd(wxWindow *parent)
    : PopupWindow(parent, wxBORDER_NONE)
{
    SetBackgroundColour(*wxLIGHT_GREY);
    SetSizer(new wxGridSizer(0, 4, FromDIP(5), FromDIP(5)));

    SlotInfoWgt *slotInfoWgt = new SlotInfoWgt(this);
    slotInfoWgt->setInfo(1, *wxRED, "PLA", false);
    slotInfoWgt->Bind(wxEVT_LEFT_DOWN, [this, slotInfoWgt](wxMouseEvent &) {
        onSlotSelected(slotInfoWgt);
    });
    GetSizer()->Add(slotInfoWgt, 0, wxALL, FromDIP(4));

    slotInfoWgt = new SlotInfoWgt(this);
    slotInfoWgt->setInfo(2, *wxGREEN, "", true);
    slotInfoWgt->Bind(wxEVT_LEFT_DOWN, [this, slotInfoWgt](wxMouseEvent &) {
        onSlotSelected(slotInfoWgt);
    });
    GetSizer()->Add(slotInfoWgt, 0, wxALL, FromDIP(4));

    slotInfoWgt = new SlotInfoWgt(this);
    slotInfoWgt->setInfo(3, *wxBLUE, "", false);
    slotInfoWgt->Bind(wxEVT_LEFT_DOWN, [this, slotInfoWgt](wxMouseEvent &) {
        onSlotSelected(slotInfoWgt);
    });
    GetSizer()->Add(slotInfoWgt, 0, wxALL, FromDIP(4));

    slotInfoWgt = new SlotInfoWgt(this);
    slotInfoWgt->setInfo(4, *wxWHITE, "ABS", false);
    slotInfoWgt->Bind(wxEVT_LEFT_DOWN, [this, slotInfoWgt](wxMouseEvent &) {
        onSlotSelected(slotInfoWgt);
    });
    GetSizer()->Add(slotInfoWgt, 0, wxALL, FromDIP(4));

    Layout();
    Fit();
}

void SlotSelectWnd::onSlotSelected(SlotInfoWgt *slotInfoWgt)
{
    SlotSelectEvent *event = new SlotSelectEvent(
        SOLT_SELECT_EVENT, slotInfoWgt->slot(), slotInfoWgt->color());
    QueueEvent(event);
    Dismiss();
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
    m_arrawBmpGray =  ScalableBitmap(this, "drop_down", FromDIP(12));
    m_arrawBmpWhite =  ScalableBitmap(this, "topbar_dropdown", FromDIP(12));

    SetSize(m_size);
    SetMinSize(m_size);
    SetMaxSize(m_size);

    SetDoubleBuffered(true);
    SetBackgroundColour(*wxWHITE);

    Bind(wxEVT_PAINT, &MaterialMatchWgt::onPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MaterialMatchWgt::onLeftDown, this);
    m_soltSelectWnd->Bind(wxEVT_SHOW, &MaterialMatchWgt::onSlotSelectWndShow, this);
    m_soltSelectWnd->Bind(SOLT_SELECT_EVENT, &MaterialMatchWgt::onSlotSelected, this);
    wxGetApp().UpdateDarkUI(this);
}

void MaterialMatchWgt::onPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc != nullptr) {
        drawBackground(gc.get());
        drawForeground(dc);
    }
}

void MaterialMatchWgt::onLeftDown(wxMouseEvent &evt)
{
    wxPoint pos = ClientToScreen(wxPoint(0, GetRect().height + FromDIP(2)));
    m_soltSelectWnd->Move(pos);
    m_soltSelectWnd->Popup();
}

void MaterialMatchWgt::onSlotSelectWndShow(wxShowEvent &evt)
{
    m_selected = evt.IsShown();
    Refresh();
    Update();
}

void MaterialMatchWgt::onSlotSelected(SlotSelectEvent &evt)
{
    m_amsColor = evt.color;
    m_amsSlot = evt.slot;
    Refresh();
    Update();
}

void MaterialMatchWgt::drawBackground(wxGraphicsContext *gc)
{
    // top
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_color));
    gc->DrawRoundedRectangle(1, 1, m_size.x - 2, FromDIP(18), 5);

    // bottom
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(wxColour(m_amsColor)));
    gc->DrawRoundedRectangle(1, FromDIP(18), m_size.x - 2, FromDIP(16), 5);
    
    // middle
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_color));
    gc->DrawRectangle(1, FromDIP(11), m_size.x - 2, FromDIP(8));

    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_amsColor));
    gc->DrawRectangle(1, FromDIP(18), m_size.x - 2, FromDIP(8));

    // border
    if (m_selected) {
        gc->SetPen(wxColour(0x00, 0xAE, 0x42));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, m_size.x - 1, m_size.y - 1, 5);
    } else if (m_color == *wxWHITE || m_amsColor == *wxWHITE) {
        gc->SetPen(wxColour(0xAC, 0xAC, 0xAC));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, m_size.x - 1, m_size.y - 1, 5);
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
    int slotTxtY = FromDIP(14) + (FromDIP(20) - slotTxtExtent.y) / 2;
    dc.DrawText(slotTxt, slotTxtX, slotTxtY);
}

AmsTipWnd::AmsTipWnd(wxWindow *parent)
    :PopupWindow(parent, wxBORDER_NONE)
{
    SetSize(wxSize(FromDIP(320), FromDIP(240)));
}

}} // namespace Slic3r::GUI
