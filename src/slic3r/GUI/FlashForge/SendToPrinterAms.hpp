#ifndef slic3r_GUI_SendToPrinterAms_hpp_
#define slic3r_GUI_SendToPrinterAms_hpp_

#include <wx/panel.h>
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class MaterialMatchWgt: public wxPanel
{
public:
    MaterialMatchWgt(wxWindow *parent,wxColour mcolour, wxString mname);

    wxPanel*    m_main_panel;
    wxColour    m_material_coloul;
    wxString    m_material_name;

    wxColour m_ams_coloul;
    wxString m_ams_name;
    int      m_ams_ctype = 0;
    std::vector<wxColour> m_ams_cols = std::vector<wxColour>();

    ScalableBitmap m_arraw_bitmap_gray;
    ScalableBitmap m_arraw_bitmap_white;
    ScalableBitmap m_transparent_mitem;

    bool m_selected {false};
    bool m_warning{false};

    void msw_rescale();
    void set_ams_info(wxColour col, wxString txt, int ctype=0, std::vector<wxColour> cols= std::vector<wxColour>());

    void disable();
    void enable();
    void on_normal();
    void on_selected();
    void on_warning();

    void paintEvent(wxPaintEvent &evt);
    void render(wxDC &dc);
    void doRender(wxDC &dc);
};

}} // namespace Slic3r::GUI

#endif
