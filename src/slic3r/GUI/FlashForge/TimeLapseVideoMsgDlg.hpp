#ifndef slic3r_GUI_TimeLapseVideoMsgDlg_hpp_
#define slic3r_GUI_TimeLapseVideoMsgDlg_hpp_

#include <wx/wx.h>
#include <wx/Webview.h>
#include "slic3r/GUI/GUI_Utils.hpp"

namespace Slic3r { namespace GUI {


    class TimeLapseVideoMsgDlg : public DPIDialog
    {
    public:
        TimeLapseVideoMsgDlg(wxWindow* parent, const std::string& video_url, const std::string& filepath);
        ~TimeLapseVideoMsgDlg();

    protected:
        void on_dpi_changed(const wxRect& suggested_rect) override {}

    private:
        bool generate_html();
    private:
        wxWebView*  m_webview{nullptr};
        std::string m_video_url;
        std::string m_filepath;
    };

}}

#endif