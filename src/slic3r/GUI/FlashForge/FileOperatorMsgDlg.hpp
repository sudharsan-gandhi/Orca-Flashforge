#ifndef slic3r_GUI_FileOperatorMsgDlg_hpp_
#define slic3r_GUI_FileOperatorMsgDlg_hpp_

#include <wx/wx.h>
#include <wx/intl.h>
#include "slic3r/GUI/TitleDialog.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"

namespace Slic3r { 
namespace GUI {

class FileOperatorMsgDlg : public wxDialog
{
public:
    enum class FILE_OPERATOR_TYPE { FILE_DOWNLOAD_SUCCEED, FILE_DOWNLOAD_FAILED, FILE_DELETE };

public:
    FileOperatorMsgDlg(wxWindow* parent, FILE_OPERATOR_TYPE type);
    ~FileOperatorMsgDlg();

private:
    void initWidget();

    void initDownloadSucceedWidget();
    void initDownloadFailedWidget();
    void initDeleteWidget();

private:
    FILE_OPERATOR_TYPE m_file_operator_type;

    wxBoxSizer* m_sizer_main{nullptr};

    FFButton* m_btn_yes{nullptr};
    FFButton* m_btn_no{nullptr};
    FFButton* m_btn_confirm{nullptr};
};

} // namespace GUI
} // namespace Slic3r

#endif