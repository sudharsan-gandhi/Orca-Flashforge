#ifndef slic3r_GUI_FFTransientWindow_hpp_
#define slic3r_GUI_FFTransientWindow_hpp_

#include <wx/popupwin.h>

namespace Slic3r { namespace GUI {

class FFRoundedWindow : public wxPopupWindow
{
public:
    FFRoundedWindow(wxWindow *parent);

protected:
    void OnSize(wxSizeEvent &evt);

    void OnPaint(wxPaintEvent &evt);

protected:
    const int m_radius;
};

class FFTransientWindow : public FFRoundedWindow
{
public:
    FFTransientWindow(wxWindow *parent, bool hasTitle, wxString titleText = "");

    bool Show(bool show = true);

    int TitleHeight() { return m_titleHeight; }

protected:
    void OnPaint(wxPaintEvent &evt);

    void OnLeftDown(wxMouseEvent &evt);

    void OnMouseCaptureLost(wxMouseCaptureLostEvent &evt);

    void OnActivateApp(wxActivateEvent& event);

private:
    int m_titleHeight;
    wxString m_titleText;
};

}} // namespace Slic3r::GUI

#endif
