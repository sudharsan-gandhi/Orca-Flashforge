#ifndef _Slic3r_GUI_TimeLapseVideoPanel_hpp_
#define _Slic3r_GUI_TimeLapseVideoPanel_hpp_

#include <map>
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
#include "slic3r/GUI/FlashForge/FFDownloadTool.hpp"
#include "slic3r/GUI/FlashForge/MultiComEvent.hpp"

namespace Slic3r { namespace GUI {

class TimeLapseVideoItem : public wxPanel
{
public:
    TimeLapseVideoItem(wxWindow *parent);

    bool getSelect() const { return m_select; }

    const wxString getFileName() const { return m_fileName; }

    const std::string &getVideoUrl() { return m_videoUrl; }

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

struct download_data_t {
    int sequence;
    bool succeed;
    wxString tmpSaveName;
    wxString fileName;
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

    void onDownload(wxCommandEvent &event);

    void onDownloadFinish(FFDownloadFinishedEvent &event);

    void updateButtonState();

    wxString getSaveName(const wxString &dirName, const wxString &fileName, bool tmp);

    using download_data_map_t = std::map<int, download_data_t>;

private:
    com_id_t              m_comId;
    wxGridSizer          *m_itemSizer;
    wxScrolledWindow     *m_scr;
    wxBoxSizer           *m_btnSizer;
    FFButton             *m_deleteBtn;
    FFButton             *m_downloadBtn;
    FFDownloadTool        m_downloadTool;
    int                   m_downloadingComId;
    int                   m_downloadingCnt;
    wxString              m_downloadSaveDir;
    download_data_map_t   m_downloadDataMap;
};

}} // namespace Slic3r::GUI

#endif
