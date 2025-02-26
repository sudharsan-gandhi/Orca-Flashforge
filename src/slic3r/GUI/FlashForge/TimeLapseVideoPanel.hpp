#ifndef _Slic3r_GUI_TimeLapseVideoItem_hpp_
#define _Slic3r_GUI_TimeLapseVideoItem_hpp_

#include <wx/event.h>
#include <wx/graphics.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wx.h>
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/FlashForge/MultiComEvent.hpp"

namespace Slic3r { namespace GUI {

class TimeLapseVideoItem : public wxPanel
{
public:
    TimeLapseVideoItem(wxWindow *parent);

    bool getSelect() const { return m_select; }
    void setData(const fnet_time_lapse_video_data_t &videoData);

private:
    void onPaint(wxPaintEvent &event);
    void onLeave(wxEvent &event);
    void onMotion(wxMouseEvent &event);
    void onLeftDown(wxMouseEvent &event);
    void onLeftUp(wxMouseEvent &event);
    void onMouseCaptureLost(wxMouseCaptureLostEvent &event);

private:
    std::string    m_videoUrl;
    wxString       m_fileName;
    int            m_videoWidth;
    int            m_videoHeight;
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

    void setComId(com_id_t comId);
    void updateVideoList();

private:
    void onGetVideoList(ComGetTimeLapseVideoListEvent &event);
    void onSelectChange(wxCommandEvent &event);

private:
    com_id_t          m_comId;
    wxGridSizer      *m_itemSizer;
    wxScrolledWindow *m_scr;
    wxBoxSizer       *m_btnSizer;
    FFButton         *m_deleteBtn;
    FFButton         *m_downloadBtn;
};

}} // namespace Slic3r::GUI

#endif
