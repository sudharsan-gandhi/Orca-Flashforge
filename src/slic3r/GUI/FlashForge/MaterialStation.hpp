#ifndef slic3r_GUI_MaterialStation_hpp_
#define slic3r_GUI_MaterialStation_hpp_
#include <wx/wx.h>
//#include <wx/intl.h>
#include <wx/panel.h>


namespace Slic3r {
namespace GUI {
class MaterialStation : public wxPanel
{
public:
    MaterialStation(wxWindow* parent);
    ~MaterialStation();
    void     create_panel(wxWindow* parent);
    wxPanel* GetPrintTitlePanel();

private:
    wxPanel*      m_panel_printing_title;
    wxStaticText* m_staticText_printing;
    wxStaticText* m_staticText_subtask_value;
};

} // namespace GUI

} // namespace Slic3r

#endif