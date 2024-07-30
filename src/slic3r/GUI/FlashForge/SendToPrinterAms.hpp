#ifndef slic3r_GUI_SendToPrinterAms_hpp_
#define slic3r_GUI_SendToPrinterAms_hpp_

#include <wx/panel.h>
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class MaterialMatchWgt: public wxPanel
{
public:
    MaterialMatchWgt(wxWindow *parent, wxColour color, wxString name);

private:
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
};

}} // namespace Slic3r::GUI

#endif
