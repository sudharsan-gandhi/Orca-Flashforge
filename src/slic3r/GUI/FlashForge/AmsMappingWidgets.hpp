#ifndef slic3r_GUI_SendToPrinterAms_hpp_
#define slic3r_GUI_SendToPrinterAms_hpp_

#include <vector>
#include <wx/dcclient.h>
#include <wx/event.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include "FFTransientWindow.hpp"
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/FlashForge/MultiComDef.hpp"
#include "slic3r/GUI/FlashForge/MultiComEvent.hpp"

namespace Slic3r { namespace GUI {

class SlotInfoWgt : public wxPanel
{
public:
    SlotInfoWgt(wxWindow *parent);

    void setInfo(int slotId, wxColour color, wxString name, bool empty, wxString mappingName);

    void setHover(bool hover);

    int slotId() const { return m_slotId; }

    wxColour color() const { return m_color; }

private:
    void onPaint(wxPaintEvent &evt);

private:
    int      m_slotId;
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
    SlotSelectEvent(wxEventType type, int _slotId, wxColour _color)
        : wxCommandEvent(type)
        , slotId(_slotId)
        , color(_color)
    {
    }
    SlotSelectEvent *Clone() const
    {
        return new SlotSelectEvent(GetEventType(), slotId, color);
    }
    int slotId;
    wxColour color;
};

wxDECLARE_EVENT(SOLT_SELECT_EVENT, SlotSelectEvent);

class SlotSelectWnd : public FFTransientWindow
{
public:
    SlotSelectWnd(wxWindow *parent, wxString mappingName);

    bool Show(bool show = true);

    void setComId(com_id_t id);

    com_id_t getComId() { return m_comId; }

private:
    void setupSlotInfoWgts();

    void onLeftDown(wxMouseEvent &evt);

    void onMotion(wxMouseEvent &evt);

    void onMouseCaptureLost(wxMouseCaptureLostEvent &evt);

    void onActivateApp(wxActivateEvent& event);

    void onComConnectionExit(ComConnectionExitEvent &evt);

    void onComDevDetailUpdate(ComDevDetailUpdateEvent &evt);

private:
    wxString m_mappingName;
    com_id_t m_comId;
    wxGridSizer *m_slotInfoWgtsSizer;
    std::vector<SlotInfoWgt *> m_slotInfoWgts;
};

class MaterialMapWgt : public wxPanel
{
public:
    MaterialMapWgt(wxWindow *parent, int toolId, wxColour color, wxString name);

    bool isSlotSelected() { return m_amsSlotId > 0; }

    void setEnable(bool enable);

    void setComId(com_id_t id) { m_soltSelectWnd->setComId(id); }

    void resetSlot();

    com_material_mapping_t getMaterialMapping();

private:
    void onPaint(wxPaintEvent &evt);

    void onLeftDown(wxMouseEvent &evt);

    void onSlotSelectWndShow(wxShowEvent &evt);

    void onSlotSelected(SlotSelectEvent &evt);

    void onComDevDetailUpdate(ComDevDetailUpdateEvent &evt);

    void draw(wxPaintDC &dc, wxGraphicsContext *gc);

private:
    int      m_toolId;
    wxColour m_color;
    wxString m_name;
    wxColour m_amsColor;
    int      m_amsSlotId;
    bool     m_selected;
    wxSize   m_size;
    int      m_radius;
    ScalableBitmap m_arrawWhiteBmp;
    ScalableBitmap m_arrawBlackBmp;
    SlotSelectWnd *m_soltSelectWnd;
    static wxColour DisbaleColor;
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
