#ifndef slic3r_GUI_SendToPrinterAms_hpp_
#define slic3r_GUI_SendToPrinterAms_hpp_

#include <wx/event.h>
#include <wx/panel.h>
#include "FFTransientWindow.hpp"
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class SlotInfoWgt : public wxPanel
{
public:
    SlotInfoWgt(wxWindow *parent);

    void setInfo(int slot, wxColour color, wxString name, bool empty);

    int slot() const { return m_slot; }

    wxColour color() const { return m_color; }

private:
    void onPaint(wxPaintEvent &evt);

    void onEnterWindow(wxMouseEvent &evt);

private:
    int      m_slot;
    wxColour m_color;
    wxString m_name;
    bool     m_empty;
    bool     m_hover;
    ScalableBitmap m_transBmp;
    ScalableBitmap m_transStrokeBmp;
    ScalableBitmap m_unknownBmp;
    ScalableBitmap m_emptyBmp;
};

struct SlotSelectEvent : public wxCommandEvent {
    SlotSelectEvent(wxEventType type, int _slot, wxColour _color)
        : wxCommandEvent(type)
        , slot(_slot)
        , color(_color)
    {
    }
    SlotSelectEvent *Clone() const
    {
        return new SlotSelectEvent(GetEventType(), slot, color);
    }
    int slot;
    wxColour color;
};

wxDECLARE_EVENT(SOLT_SELECT_EVENT, SlotSelectEvent);

class SlotSelectWnd : public FFTransientWindow
{
public:
    SlotSelectWnd(wxWindow *parent);

private:
    wxBoxSizer *setupSlotInfoWgts();

    void onSlotSelected(SlotInfoWgt *slotInfoWgt);
};

class MaterialMapWgt : public wxPanel
{
public:
    MaterialMapWgt(wxWindow *parent, wxColour color, wxString name);

private:
    void onPaint(wxPaintEvent &evt);

    void onLeftDown(wxMouseEvent &evt);

    void onSlotSelectWndShow(wxShowEvent &evt);

    void onSlotSelected(SlotSelectEvent &evt);

    void draw(wxPaintDC &dc, wxGraphicsContext *gc);

private:
    wxColour m_color;
    wxString m_name;
    wxColour m_amsColor;
    int      m_amsSlot;
    bool     m_selected;
    wxSize   m_size;
    int      m_radius;
    ScalableBitmap m_arrawWhiteBmp;
    ScalableBitmap m_arrawBlackBmp;
    SlotSelectWnd *m_soltSelectWnd;
};

class AmsTipWnd : public FFTransientWindow
{
public:
    AmsTipWnd(wxWindow *parent);

private:
    void onPaint(wxPaintEvent &evt);

    void drawIconText(wxPaintDC &dc, wxString text, wxRect rt);

    void drawTutorialText(wxPaintDC &dc, wxString text, int left, int vertMid);
};

}} // namespace Slic3r::GUI

#endif
