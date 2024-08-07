#ifndef slic3r_GUI_FFTransientWindow_hpp_
#define slic3r_GUI_FFTransientWindow_hpp_

#include <wx/popupwin.h>

namespace Slic3r { namespace GUI {

class FFTransientWindow : public wxPopupWindow
{
public:
    FFTransientWindow(wxWindow *parent, bool hasTitle, wxString titleText = "");

    int TitleHeight() { return m_titleHeight; }

protected:
    void OnSize(wxSizeEvent &evt);

    void OnPaint(wxPaintEvent &evt);

private:
    int m_titleHeight;
    int m_radius;
    wxString m_titleText;
};

}} // namespace Slic3r::GUI

#endif
