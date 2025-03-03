#ifndef slic3r_GUI_TimeLapseVideoPlayDlg_hpp_
#define slic3r_GUI_TimeLapseVideoPlayDlg_hpp_

#include <wx/wx.h>
#include <wx/Webview.h>
#include "slic3r/GUI/GUI_Utils.hpp"

namespace Slic3r { namespace GUI {


    class TimeLapseVideoPlayDlg : public wxDialog
    {
    public:
        TimeLapseVideoPlayDlg(wxWindow* parent, const std::string& filepath, const std::string& video_url, int width, int height);
        ~TimeLapseVideoPlayDlg();

    private:
        bool generate_html();
    private:
        wxWebView*  m_webview{nullptr};
        std::string m_video_url;
        std::string m_filepath;
        int         m_width;
        int         m_height;
    };

}}

#endif