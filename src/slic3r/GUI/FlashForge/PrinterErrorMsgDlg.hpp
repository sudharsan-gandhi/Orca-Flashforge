#ifndef _Slic3r_GUI_PrinterErrorMsgDlg_hpp_
#define _Slic3r_GUI_PrinterErrorMsgDlg_hpp_

#include <set>
#include <string>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include "slic3r/GUI/FlashForge/MultiComDef.hpp"
#include "slic3r/GUI/FlashForge/MultiComEvent.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"

namespace Slic3r { namespace GUI {

class PrinterErrorMsgDlg : public wxDialog
{
public:
    PrinterErrorMsgDlg(wxWindow *parent, com_id_t comId, const std::string &errorCode);

    static bool isErrorCodeHandled(const std::string &errorCode);

private:
    void setupErrorCode(const std::string &errorCode);
    void onOperator1(wxCommandEvent &event);
    void onOperator2(wxCommandEvent &event);
    void onConnectionExit(ComConnectionExitEvent &event);
    void onDevDetailUpdate(ComDevDetailUpdateEvent &event);

private:
    com_id_t m_comId;
    std::string m_errorCode;
    wxStaticText *m_titleLbl;
    wxStaticText *m_msgLbl;
    FFButton *m_operator1Btn;
    FFButton *m_operator2Btn;
    static const std::set<std::string> s_filamentErrorCodeSet;
};

}} // namespace Slic3r::GUI

#endif
