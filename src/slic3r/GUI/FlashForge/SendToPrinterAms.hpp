#ifndef slic3r_GUI_SendToPrinterAms_hpp_
#define slic3r_GUI_SendToPrinterAms_hpp_

#include <wx/panel.h>
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/PopupWindow.hpp"

namespace Slic3r { namespace GUI {

class SlotSelectWnd : public PopupWindow
{
public:
    SlotSelectWnd(wxWindow *parent);
};

class MaterialMatchWgt : public wxPanel
{
public:
    MaterialMatchWgt(wxWindow *parent, wxColour color, wxString name);

private:
    void onPaint(wxPaintEvent &evt);

    void onLeftDown(wxMouseEvent &evt);

    void drawBackground(wxDC &dc);

    void drawForeground(wxDC &dc);

private:
    wxColour m_color;
    wxString m_name;
    wxColour m_amsColor;
    int      m_amsSlot;
    bool     m_selected;
    wxSize   m_size;
    wxSize   m_realSize;
    ScalableBitmap m_arrawBmpGray;
    ScalableBitmap m_arrawBmpWhite;
    SlotSelectWnd *m_soltSelectWnd;
};

}} // namespace Slic3r::GUI

#endif
