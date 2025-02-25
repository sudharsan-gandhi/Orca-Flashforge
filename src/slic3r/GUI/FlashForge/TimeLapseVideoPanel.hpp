#ifndef _Slic3r_GUI_TimeLapseVideoItem_hpp_
#define _Slic3r_GUI_TimeLapseVideoItem_hpp_

#include <wx/graphics.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wx.h>
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class TimeLapseVideoItem : public wxPanel
{
public:
    TimeLapseVideoItem(wxWindow *parent);

private:
    void onPaint(wxPaintEvent &event);
    void onLeave(wxEvent &event);
    void onMotion(wxMouseEvent &event);
    void onLeftDown(wxMouseEvent &event);
    void onLeftUp(wxMouseEvent &event);
    void onMouseCaptureLost(wxMouseCaptureLostEvent &event);

private:
    wxString       m_fileName;
    bool           m_select;
    bool           m_hoverSelRect;
    bool           m_pressSelRect;
    wxRect         m_selRect;
    ScalableBitmap m_selOnNormalIcon;
    ScalableBitmap m_selOnHoverIcon;
    ScalableBitmap m_selOffNormalIcon;
    ScalableBitmap m_selOffHoverIcon;
};

class TimeLapseVideoPanel : public wxPanel
{
public:
    TimeLapseVideoPanel(wxWindow *parent);

private:
    wxGridSizer      *m_itemSizer;
    wxScrolledWindow *m_scr;
    wxBoxSizer       *m_btnSizer;
    FFButton         *m_deleteBtn;
    FFButton         *m_downloadBtn;
};

}} // namespace Slic3r::GUI

#endif
