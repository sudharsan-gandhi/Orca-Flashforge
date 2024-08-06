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
    : FFTransientWindow(parent, true, "FF_TAG_AMS_MATERIAL_SELECT")
{
    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->AddSpacer(TitleHeight() + FromDIP(20));
    GetSizer()->Add(setupSlotInfoWgts());
    GetSizer()->AddSpacer(FromDIP(20));
    Layout();
    Fit();
}

wxBoxSizer *SlotSelectWnd::setupSlotInfoWgts()
{
    wxColour colors[4] = { *wxRED, *wxGREEN, *wxBLUE, *wxWHITE };
    wxString names[4] = { "PLA", "", "", "ABS" };
    bool emptyStates[4] = { false, true, false, false };

    std::unique_ptr<wxBoxSizer> slotWgtSizer(new wxBoxSizer(wxHORIZONTAL));
    for (int i = 0; i < 4; ++i) {
        SlotInfoWgt *slotInfoWgt = new SlotInfoWgt(this);
        slotInfoWgt->setInfo(1, colors[i], names[i], emptyStates[i]);
        slotInfoWgt->Bind(wxEVT_LEFT_DOWN, [this, slotInfoWgt](wxMouseEvent &) {
            onSlotSelected(slotInfoWgt);
        });
        slotWgtSizer->Add(slotInfoWgt, 0, wxALL, FromDIP(4));
    }
    return slotWgtSizer.release();
}

void SlotSelectWnd::onSlotSelected(SlotInfoWgt *slotInfoWgt)
{
    SlotSelectEvent *event = new SlotSelectEvent(
        SOLT_SELECT_EVENT, slotInfoWgt->slot(), slotInfoWgt->color());
    QueueEvent(event);
    Dismiss();
}

MaterialMapWgt::MaterialMapWgt(wxWindow *parent, wxColour color, wxString name)
    : wxPanel(parent)
    , m_color(color)
    , m_name(name)
    , m_amsColor(0xEE, 0xEE, 0xEE)
    , m_amsSlot(0)
    , m_selected(false)
    , m_size(FromDIP(70), FromDIP(58))
    , m_radius(FromDIP(3))
    , m_soltSelectWnd(new SlotSelectWnd(parent))
 {
    m_arrawBmpGray =  ScalableBitmap(this, "drop_down", FromDIP(12));
    m_arrawBmpWhite =  ScalableBitmap(this, "topbar_dropdown", FromDIP(12));

    SetSize(m_size);
    SetMinSize(m_size);
    SetMaxSize(m_size);

    Bind(wxEVT_PAINT, &MaterialMapWgt::onPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MaterialMapWgt::onLeftDown, this);
    m_soltSelectWnd->Bind(wxEVT_SHOW, &MaterialMapWgt::onSlotSelectWndShow, this);
    m_soltSelectWnd->Bind(SOLT_SELECT_EVENT, &MaterialMapWgt::onSlotSelected, this);
}

void MaterialMapWgt::onPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc != nullptr) {
        draw(dc, gc.get());
    }
}

void MaterialMapWgt::onLeftDown(wxMouseEvent &evt)
{
    if (m_selected) {
        return;
    }
    wxPoint pos = ClientToScreen(wxPoint(0, GetRect().height + FromDIP(2)));
    m_soltSelectWnd->Move(pos);
    m_soltSelectWnd->Popup();
}

void MaterialMapWgt::onSlotSelectWndShow(wxShowEvent &evt)
{
    CallAfter([this, isShown = evt.IsShown()]() {
        m_selected = isShown;
        Refresh();
        Update();
    });
}

void MaterialMapWgt::onSlotSelected(SlotSelectEvent &evt)
{
    m_amsColor = evt.color;
    m_amsSlot = evt.slot;
    Refresh();
    Update();
}

void MaterialMapWgt::draw(wxPaintDC &dc, wxGraphicsContext *gc)
{
    // top
    int halfHeight = m_size.y / 2;
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_color));
    gc->DrawRoundedRectangle(0, 0, m_size.x, halfHeight, m_radius);
    gc->DrawRectangle(0, halfHeight - m_radius, m_size.x, m_radius);

    // bottom
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(m_amsColor));
    gc->DrawRoundedRectangle(0, halfHeight, m_size.x, halfHeight, m_radius);
    gc->DrawRectangle(0, halfHeight, m_size.x, m_radius);

    // border
    if (m_selected) {
        gc->SetPen(wxColour(0x00, 0xAE, 0x42));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, m_size.x - 1, m_size.y - 1, m_radius);
    } else if (m_color == *wxWHITE || m_amsColor == *wxWHITE) {
        gc->SetPen(wxColour(0xAC, 0xAC, 0xAC));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(0, 0, m_size.x - 1, m_size.y - 1, m_radius);
    }

    // material name
    if (m_color.GetLuminance() < 0.6) {
        dc.SetTextForeground(*wxWHITE);
    } else {
        dc.SetTextForeground(wxColour("#404040"));
    }
    dc.SetFont(::Label::Body_13);
    wxSize nameSize = dc.GetTextExtent(m_name);
    if (nameSize.x > GetSize().x - FromDIP(10)) {
        dc.SetFont(::Label::Body_10);
        nameSize = dc.GetTextExtent(m_name);
    }
    dc.DrawText(m_name, (m_size.x - nameSize.x) / 2, (halfHeight - nameSize.y) / 2);

    // mapping slot
    if (m_amsColor.GetLuminance() < 0.6) {
        dc.SetTextForeground(*wxWHITE);
    } else {
        dc.SetTextForeground(wxColour("#404040"));
    }
    dc.SetFont(::Label::Body_13);
    wxString slotTxt;
    if (m_amsSlot <= 0) {
        slotTxt = "-";
    } else {
        slotTxt = std::to_string(m_amsSlot);
    }
    wxSize arrowBmpSize = m_arrawBmpWhite.GetBmpSize();
    wxSize slotSize = dc.GetTextExtent(slotTxt);
    int slotArrowSpace = FromDIP(6);
    int slotTxtX = FromDIP(2) + (m_size.x - slotSize.x - arrowBmpSize.x - slotArrowSpace) / 2;
    int slotTxtY = halfHeight + (halfHeight - slotSize.y) / 2;
    dc.DrawText(slotTxt, slotTxtX, slotTxtY);

    //arrow
    int arrowX = slotTxtX + slotSize.x + slotArrowSpace;
    int arrowY = halfHeight + (halfHeight - arrowBmpSize.y) / 2;
    if (m_amsColor.Red() > 160 && m_amsColor.Green() > 160 && m_amsColor.Blue() > 160
     && m_amsColor.Red() < 180 && m_amsColor.Green() < 180 && m_amsColor.Blue() < 180) {
        dc.DrawBitmap(m_arrawBmpWhite.bmp(), arrowX, arrowY);
    } else {
        dc.DrawBitmap(m_arrawBmpGray.bmp(), arrowX, arrowY);
    }
}

AmsTipWnd::AmsTipWnd(wxWindow *parent)
    : FFTransientWindow(parent, false)
{
    SetSize(wxSize(FromDIP(400), FromDIP(125)));
    Bind(wxEVT_PAINT, &AmsTipWnd::onPaint, this);
}

void AmsTipWnd::onPaint(wxPaintEvent &evt)
{
    FFTransientWindow::OnPaint(evt);

    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    wxSize size = GetSize();
    int iconLeft = size.x * 0.086;
    int iconVertMid = size.y * 0.5;
    int iconWidth = FromDIP(70);
    int iconHalfHeight = FromDIP(29);
    int iconRadius = FromDIP(3);
    int iconRight = iconLeft + iconWidth;
    int iconTop = iconVertMid - iconHalfHeight;
    int iconBottom = iconVertMid + iconHalfHeight;

    // icon
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxColour("#d9001b"));
    gc->DrawRoundedRectangle(iconLeft, iconTop, iconWidth, iconHalfHeight, iconRadius);
    gc->DrawRectangle(iconLeft, iconVertMid - iconRadius, iconWidth, iconRadius);

    gc->SetBrush(wxColour("#f59a23"));
    gc->DrawRoundedRectangle(iconLeft, iconVertMid, iconWidth, iconHalfHeight, iconRadius);
    gc->DrawRectangle(iconLeft, iconVertMid, iconWidth, iconRadius);

    // lines
    int topLineY = iconTop + FromDIP(6);
    int bottomLineY = iconBottom - FromDIP(6);
    int lineRight = iconRight + FromDIP(27);
    wxGraphicsPath path = gc->CreatePath();
    path.MoveToPoint(iconRight, topLineY);
    path.AddLineToPoint(lineRight, topLineY);
    path.MoveToPoint(iconRight, bottomLineY);
    path.AddLineToPoint(lineRight, bottomLineY);
    gc->SetPen(wxColour("#c1c1c1"));
    gc->StrokePath(path);

    // texts
    dc.SetFont(::Label::Body_13);
    dc.SetTextForeground(*wxWHITE);
    drawIconText(dc, "PLA", wxRect(iconLeft, iconTop, iconWidth, iconHalfHeight));
    drawIconText(dc, "1", wxRect(iconLeft, iconVertMid, iconWidth, iconHalfHeight));

    int tutotrialLeft = lineRight + FromDIP(5);
    dc.SetTextForeground(*wxBLACK);
    drawTutorialText(dc, "FF_TAG_AMS_TUTORIAL_1", tutotrialLeft, topLineY);
    drawTutorialText(dc, "FF_TAG_AMS_TUTORIAL_2", tutotrialLeft, bottomLineY);
}

void AmsTipWnd::drawIconText(wxPaintDC &dc, wxString text, wxRect rt)
{
    wxCoord width, height;
    dc.GetTextExtent(text, &width, &height);

    int x = rt.GetLeft() + (rt.width - width) / 2;
    int y = rt.GetTop() + (rt.height - height) / 2;
    dc.DrawText(text, x, y);
}

void AmsTipWnd::drawTutorialText(wxPaintDC &dc, wxString text, int left, int vertMid)
{
    wxCoord width, height;
    dc.GetTextExtent(text, &width, &height);
    dc.DrawText(text, left, vertMid - height / 2);
}

}} // namespace Slic3r::GUI
