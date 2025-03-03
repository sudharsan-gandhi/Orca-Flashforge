#ifndef _Slic3r_GUI_VideoDownloadErrorDlg_hpp_
#define _Slic3r_GUI_VideoDownloadErrorDlg_hpp_

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "slic3r/GUI/TitleDialog.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"

namespace Slic3r { namespace GUI {

class VideoDownloadErrorDlg : public TitleDialog
{
public:
    VideoDownloadErrorDlg(wxWindow* parent, const std::vector<std::string>& file_infos);
    ~VideoDownloadErrorDlg();


    void test_add_items();

protected:
    void on_dpi_changed(const wxRect& suggested_rect) override {}
private:


private:
    wxBoxSizer* m_sizer_main{nullptr};
    wxBoxSizer* m_sizer_video_info{nullptr};
    wxBoxSizer* m_sizer_scroll{nullptr};

    wxStaticText*       m_msg_Lbl{nullptr};
    wxStaticText*       m_list_Lbl{nullptr};
    FFButton*           m_btn_confirm{nullptr};
    wxScrolledWindow*   m_scroll_wgt{nullptr};

    std::vector<std::string> m_file_infos;
};



class TestScrollWidget : public wxDialog
{
public:
    TestScrollWidget(wxWindow* parent);
    ~TestScrollWidget();

private:
    void OnScroll(wxScrollEvent& event)
    {

        int pos = event.GetPosition();


        wxScrolledWindow* scrolledWindow = (wxScrolledWindow*) FindWindowById(wxID_ANY);
        if (scrolledWindow) {
            scrolledWindow->Scroll(0, pos);
        }
    }

private:
    wxScrolledWindow* m_scroll_window{nullptr};
    FFButton*         m_btn_confirm{nullptr};
};

}} // namespace Slic3r::GUI

#endif
