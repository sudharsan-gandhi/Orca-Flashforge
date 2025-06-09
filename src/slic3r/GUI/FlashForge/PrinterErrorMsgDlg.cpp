#include "PrinterErrorMsgDlg.hpp"
#include <wx/utils.h>
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/FlashForge/MultiComMgr.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

namespace Slic3r { namespace GUI {

PrinterErrorMsgDlg::PrinterErrorMsgDlg(wxWindow *parent, com_id_t comId, const std::string &errorCode)
    : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxSYSTEM_MENU)
    , m_comId(comId)
    , m_errorCode(errorCode)
{
    if (s_errorCodeDataMap.empty()) {
        initErrorCodeDataMap();
    }
    SetBackgroundColour(*wxWHITE);
    SetDoubleBuffered(true);

    m_titleLbl = new wxStaticText(this, wxID_ANY, _L("Error"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    m_titleLbl->SetFont(::Label::Body_14);
    m_titleLbl->SetForegroundColour("#333333");

    m_msgLbl = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    m_msgLbl->SetMinSize(wxSize(FromDIP(361), -1));
    m_msgLbl->SetForegroundColour("#333333");

    m_operator1Btn = new FFButton(this, wxID_ANY, wxEmptyString);
    m_operator1Btn->SetFontColor("#419488");
    m_operator1Btn->SetBorderColor("#419488");
    m_operator1Btn->SetFontHoverColor("#65A79E");
    m_operator1Btn->SetBorderHoverColor("#65A79E");
    m_operator1Btn->SetFontPressColor("#1A8676");
    m_operator1Btn->SetBorderPressColor("#1A8676");

    m_operator2Btn = new FFButton(this, wxID_ANY, wxEmptyString);
    m_operator2Btn->SetFontColor("#419488");
    m_operator2Btn->SetBorderColor("#419488");
    m_operator2Btn->SetFontHoverColor("#65A79E");
    m_operator2Btn->SetBorderHoverColor("#65A79E");
    m_operator2Btn->SetFontPressColor("#1A8676");
    m_operator2Btn->SetBorderPressColor("#1A8676");

    wxSizer *sizerBtn = new wxBoxSizer(wxHORIZONTAL);
    sizerBtn->AddStretchSpacer(1);
    sizerBtn->Add(m_operator1Btn);
    sizerBtn->AddSpacer(FromDIP(23));
    sizerBtn->Add(m_operator2Btn);
    sizerBtn->AddStretchSpacer(1);

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddSpacer(FromDIP(34));
    sizer->Add(m_titleLbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    sizer->AddSpacer(FromDIP(15));
    sizer->Add(m_msgLbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    sizer->AddSpacer(FromDIP(34));
    sizer->Add(sizerBtn, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    sizer->AddSpacer(FromDIP(47));
    SetSizer(sizer);

    setupErrorCode(errorCode);
    m_operator1Btn->Bind(wxEVT_BUTTON, &PrinterErrorMsgDlg::onOperator1, this);
    m_operator2Btn->Bind(wxEVT_BUTTON, &PrinterErrorMsgDlg::onOperator2, this);
    MultiComMgr::inst()->Bind(COM_CONNECTION_EXIT_EVENT, &PrinterErrorMsgDlg::onConnectionExit, this);
    MultiComMgr::inst()->Bind(COM_DEV_DETAIL_UPDATE_EVENT, &PrinterErrorMsgDlg::onDevDetailUpdate, this);

    Layout();
    Fit();
    CenterOnScreen();
}

bool PrinterErrorMsgDlg::isErrorCodeHandled(const std::string &errorCode)
{
    if (s_errorCodeDataMap.empty()) {
        initErrorCodeDataMap();
    }
    return s_errorCodeDataMap.find(errorCode) != s_errorCodeDataMap.end()
        || errorCode == "E0088" || errorCode == "E0089";
}

void PrinterErrorMsgDlg::setupErrorCode(const std::string &errorCode)
{
    auto it = s_errorCodeDataMap.find(errorCode);
    if (it != s_errorCodeDataMap.end()) {
        m_msgLbl->SetLabelText(it->second.message);
        m_operator1Btn->SetLabel(_L("__SHOW_GUIDE__"), FromDIP(165), FromDIP(36));
        m_operator2Btn->SetLabel(_L("__CLEAR_TIPS__"), FromDIP(165), FromDIP(36));
    } else if (errorCode == "E0088") {
        m_msgLbl->SetLabelText(_L("Non-Flashforge build plate detected. Print quality may not be guaranteed."));
        m_operator1Btn->SetLabel(_L("Continue printing"), FromDIP(165), FromDIP(36));
        m_operator2Btn->SetLabel(_L("Stop printing (replace the build plate)"), FromDIP(165), FromDIP(36));
    } else if (errorCode == "E0089") {
        m_msgLbl->SetLabelText(_L("Lidar detected first-layer defects. Please check and decide whether to continue printing."));
        m_operator1Btn->SetLabel(_L("Continue printing (defects acceptable)"), FromDIP(165), FromDIP(36));
        m_operator2Btn->SetLabel(_L("Stop printing"), FromDIP(165), FromDIP(36));
    }
    if (!m_msgLbl->GetLabelText().empty()) {
        Layout();
        Fit();
        m_msgLbl->Wrap(m_msgLbl->GetSize().x);
    }
}

void PrinterErrorMsgDlg::onOperator1(wxCommandEvent &event)
{
    event.Skip();
    auto it = s_errorCodeDataMap.find(m_errorCode);
    if (s_errorCodeDataMap.find(m_errorCode) != s_errorCodeDataMap.end()) {
        wxLaunchDefaultBrowser(it->second.wikiUrl);
        return;
    } else if (m_errorCode == "E0088") {
        MultiComMgr::inst()->putCommand(m_comId, new ComPlateDetectCtrl("continue"));
    } else if (m_errorCode == "E0089") {
        MultiComMgr::inst()->putCommand(m_comId, new ComFirstLayerDetectCtrl("continue"));
    }
    EndModal(wxOK);
}

void PrinterErrorMsgDlg::onOperator2(wxCommandEvent &event)
{
    event.Skip();
    auto it = s_errorCodeDataMap.find(m_errorCode);
    if (it != s_errorCodeDataMap.end()) {
        MultiComMgr::inst()->putCommand(m_comId, new ComErrorCodeCtrl("clearErrorCode", m_errorCode));
    } else if (m_errorCode == "E0088") {
        MultiComMgr::inst()->putCommand(m_comId, new ComPlateDetectCtrl("stop"));
    } else if (m_errorCode == "E0089") {
        MultiComMgr::inst()->putCommand(m_comId, new ComFirstLayerDetectCtrl("stop"));
    }
    EndModal(wxOK);
}

void PrinterErrorMsgDlg::onConnectionExit(ComConnectionExitEvent &event)
{
    event.Skip();
    if (event.id != m_comId) {
        return;
    }
    EndModal(wxCANCEL);
}

void PrinterErrorMsgDlg::onDevDetailUpdate(ComDevDetailUpdateEvent &event)
{
    event.Skip();
    if (event.id != m_comId) {
        return;
    }
    if (strcmp(event.devDetail->status, "error") != 0 || event.devDetail->errorCode != m_errorCode) {
        EndModal(wxCANCEL);
    }
}

void PrinterErrorMsgDlg::initErrorCodeDataMap()
{
    auto pair = s_errorCodeDataMap.emplace("E0100", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0101", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0102", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0103", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0104", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0105", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0106", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0107", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0108", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0109", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0110", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0113", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";

    pair = s_errorCodeDataMap.emplace("E0200", error_code_data_t());
    pair.first->second.message = _L("");
    pair.first->second.wikiUrl = "";
}

}} // namespace Slic3r::GUI
