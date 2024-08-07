#include "AmsMappingWidgets.hpp"
#include <memory>
#include <string>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/stattext.h>
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

namespace Slic3r { namespace GUI {

SlotInfoWgt::SlotInfoWgt(wxWindow *parent)
    : wxPanel(parent)
    , m_slot(0)
    , m_color(*wxWHITE)
    , m_empty(true)
    , m_hover(false)
    , m_transBmp(this, "filament_reel_trans", 68)
    , m_transStrokeBmp(this, "filament_reel_trans_stroke", 68)
    , m_unknownBmp(this, "filament_reel_unknown", 68)
    , m_emptyBmp(this, "filament_reel_empty", 68)
{
    SetDoubleBuffered(true);
    SetSize(wxSize(FromDIP(61), FromDIP(102)));
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

void SlotInfoWgt::setHover(bool hover)
{
    if (hover != m_hover) {
        m_hover = hover;
        Refresh();
        Update();
    }
}

void SlotInfoWgt::onPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    // slot
    if (m_hover) {
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxColour("#95c5ff"));
    } else {
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxColour("#dddddd"));
    }
    wxSize size = GetSize();
    int slotCircleSize = FromDIP(24);
    gc->DrawEllipse((size.x - slotCircleSize) / 2, FromDIP(0), slotCircleSize, slotCircleSize);
    wxString slotTxt = std::to_string(m_slot);
    wxSize slotTxtSize = dc.GetTextExtent(slotTxt);
    dc.SetFont(::Label::Body_13);
    dc.SetTextForeground(*wxWHITE);
    dc.DrawText(slotTxt, (size.x - slotTxtSize.x) / 2, (slotCircleSize - slotTxtSize.y) / 2);

    // filament reel
    int filamentReelHeight = FromDIP(68);
    bool useStrokeBmp = m_color.GetLuminance() > 0.95;
    if (m_empty) {
        gc->DrawBitmap(m_emptyBmp.bmp(), 0, size.y - filamentReelHeight, size.x, filamentReelHeight);
    } else if (m_name.empty()) {
        gc->DrawBitmap(m_unknownBmp.bmp(), 0, size.y - filamentReelHeight, size.x, filamentReelHeight);
    } else {
        const wxBitmap &bmp = useStrokeBmp ? m_transStrokeBmp.bmp() : m_transBmp.bmp();
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(m_color));
        gc->DrawRectangle(0, size.y - filamentReelHeight, size.x, filamentReelHeight);
        gc->DrawBitmap(bmp, 0, size.y - filamentReelHeight, size.x, filamentReelHeight);
    }

    // name
    if (!m_empty && !m_name.empty()) {
        wxSize nameTxtSize = dc.GetTextExtent(m_name);
        int nameTxtOfsX = useStrokeBmp ? FromDIP(13) : FromDIP(14);
        int nameTxtY = size.y - filamentReelHeight + (filamentReelHeight - nameTxtSize.y) / 2;
        if (!m_name.empty() && m_color.GetLuminance() < 0.6) {
            dc.SetTextForeground(*wxWHITE);
        } else {
            dc.SetTextForeground(wxColour("#434343"));
        }
        dc.SetFont(::Label::Body_10);
        dc.DrawText(m_name, (size.x - nameTxtSize.x) / 2 + nameTxtOfsX, nameTxtY + FromDIP(1));
    }
}

wxDEFINE_EVENT(SOLT_SELECT_EVENT, SlotSelectEvent);

SlotSelectWnd::SlotSelectWnd(wxWindow *parent)
    : FFTransientWindow(parent, true, "FF_TAG_AMS_MATERIAL_SELECT")
{
    wxStaticText *tipLbl = new wxStaticText(this, wxID_ANY, "FF_TAG_AMS_SELECT_TIP");
    tipLbl->SetForegroundColour(wxColour("#f59a23"));

    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->AddSpacer(TitleHeight() + FromDIP(9));
    GetSizer()->Add(setupSlotInfoWgts());
    GetSizer()->AddSpacer(FromDIP(10));
    GetSizer()->Add(tipLbl, 0, wxALIGN_CENTER);
    GetSizer()->AddSpacer(FromDIP(16));
    Layout();
    Fit();

    Bind(wxEVT_LEFT_DOWN, &SlotSelectWnd::onLeftDown, this);
    Bind(wxEVT_MOTION, &SlotSelectWnd::onMotion, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &SlotSelectWnd::onMouseCaptureLost, this);
    wxGetApp().Bind(wxEVT_ACTIVATE_APP, &SlotSelectWnd::onActivateApp, this);
}

bool SlotSelectWnd::Show(bool show /* = true */)
{
    if (FFTransientWindow::Show(show)) {
        if (show) {
            CaptureMouse();
        } else {
            ReleaseMouse();
        }
        return true;
    }
    return false;
}

wxBoxSizer *SlotSelectWnd::setupSlotInfoWgts()
{
    wxColour colors[4] = { *wxRED, *wxGREEN, *wxBLUE, *wxWHITE };
    wxString names[4] = { "PLA", "", "", "ABS" };
    bool emptyStates[4] = { false, true, false, false };

    std::unique_ptr<wxBoxSizer> slotWgtSizer(new wxBoxSizer(wxHORIZONTAL));
    slotWgtSizer->AddSpacer(FromDIP(72));
    for (int i = 0; i < 4; ++i) {
        SlotInfoWgt *slotInfoWgt = new SlotInfoWgt(this);
        slotInfoWgt->setInfo(i + 1, colors[i], names[i], emptyStates[i]);
        slotWgtSizer->Add(slotInfoWgt);
        slotWgtSizer->AddSpacer(FromDIP(20));
        m_slotInfoWgts.push_back(slotInfoWgt);
    }
    slotWgtSizer->AddSpacer(FromDIP(52));
    return slotWgtSizer.release();
}

void SlotSelectWnd::onLeftDown(wxMouseEvent &evt)
{
    wxPoint pos = evt.GetPosition();
    if (HitTest(pos) == wxHT_WINDOW_OUTSIDE) {
        Show(false);
        return;
    }
    for (auto slotInfoWgt : m_slotInfoWgts) {
        if (slotInfoWgt->IsEnabled()) {
            wxPoint pos1 = slotInfoWgt->ScreenToClient(ClientToScreen(pos));
            if (slotInfoWgt->HitTest(pos1) == wxHT_WINDOW_INSIDE) {
                SlotSelectEvent *event = new SlotSelectEvent(
                    SOLT_SELECT_EVENT, slotInfoWgt->slot(), slotInfoWgt->color());
                QueueEvent(event);
                Show(false);
            }
        }
    }
}

void SlotSelectWnd::onMotion(wxMouseEvent &evt)
{
    wxPoint pos = evt.GetPosition();
    if (HitTest(pos) == wxHT_WINDOW_OUTSIDE) {
        return;
    }
    for (auto slotInfoWgt : m_slotInfoWgts) {
        if (slotInfoWgt->IsEnabled()) {
            wxPoint pos1 = slotInfoWgt->ScreenToClient(ClientToScreen(pos));
            slotInfoWgt->setHover(slotInfoWgt->HitTest(pos1) == wxHT_WINDOW_INSIDE);
        }
    }
}

void SlotSelectWnd::onMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    FFTransientWindow::Show(false);
}

void SlotSelectWnd::onActivateApp(wxActivateEvent& event)
{
    if (!event.GetActive()) {
        Show(false);
    }
    event.Skip();
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
    , m_arrawWhiteBmp(this, "ff_drop_down_white", FromDIP(5))
    , m_arrawBlackBmp(this, "ff_drop_down_black", FromDIP(5))
    , m_soltSelectWnd(new SlotSelectWnd(parent))
 {
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
    if (!m_selected) {
        wxPoint pos = ClientToScreen(wxPoint(0, GetRect().height + FromDIP(1)));
        m_soltSelectWnd->Move(pos);
        m_soltSelectWnd->Show(true);
    } else {
        m_soltSelectWnd->Show(false);
    }
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
    } else if (m_color.GetLuminance() > 0.95 || m_amsColor.GetLuminance() > 0.95) {
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
    wxSize arrowBmpSize = m_arrawWhiteBmp.GetBmpSize();
    wxSize slotSize = dc.GetTextExtent(slotTxt);
    int slotArrowSpace = FromDIP(4);
    int slotTxtX = FromDIP(2) + (m_size.x - slotSize.x - arrowBmpSize.x - slotArrowSpace) / 2;
    int slotTxtY = halfHeight + (halfHeight - slotSize.y) / 2;
    dc.DrawText(slotTxt, slotTxtX, slotTxtY);

    //arrow
    int arrowX = slotTxtX + slotSize.x + slotArrowSpace;
    int arrowY = halfHeight + (halfHeight - arrowBmpSize.y) / 2;
    if (m_amsColor.GetLuminance() < 0.6) {
        dc.DrawBitmap(m_arrawWhiteBmp.bmp(), arrowX, arrowY);
    } else {
        dc.DrawBitmap(m_arrawBlackBmp.bmp(), arrowX, arrowY);
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
