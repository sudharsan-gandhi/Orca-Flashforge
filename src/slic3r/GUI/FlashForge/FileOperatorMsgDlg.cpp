#include "FileOperatorMsgDlg.hpp"

#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

namespace Slic3r {
namespace GUI {

FileOperatorMsgDlg::FileOperatorMsgDlg(wxWindow* parent, FILE_OPERATOR_TYPE type)
    : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
    , m_file_operator_type(type)
{
    SetBackgroundColour("#FFFFFF");
    initWidget();
}

FileOperatorMsgDlg ::~FileOperatorMsgDlg() {}

void FileOperatorMsgDlg::initWidget()
{
    if (m_file_operator_type == FILE_OPERATOR_TYPE::FILE_DOWNLOAD_SUCCEED)
        initDownloadSucceedWidget();
    else if (m_file_operator_type == FILE_OPERATOR_TYPE::FILE_DOWNLOAD_FAILED)
        initDownloadFailedWidget();
    else if (m_file_operator_type == FILE_OPERATOR_TYPE::FILE_DELETE)
        initDeleteWidget();
}

void FileOperatorMsgDlg::initDownloadSucceedWidget()
{
    SetTitle(_L("Download succeed"));
    m_sizer_main = new wxBoxSizer(wxVERTICAL);
    m_sizer_main->SetMinSize(wxSize(FromDIP(380), FromDIP(200)));

    wxStaticText* msg_lbl = new wxStaticText(this, wxID_ANY, _L("Download completed!"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    msg_lbl->SetFont(::Label::Body_16);
    msg_lbl->SetForegroundColour("#419488");

    m_btn_confirm = new FFButton(this, wxID_ANY, wxEmptyString);
    m_btn_confirm->SetLabel(_L("Confirm"), FromDIP(80), FromDIP(44));
    m_btn_confirm->SetFontColor(*wxWHITE);
    m_btn_confirm->SetBGColor("#419488");
    m_btn_confirm->SetBorderColor("#419488");
    m_btn_confirm->SetFontHoverColor(*wxWHITE);
    m_btn_confirm->SetBGHoverColor("#65A79E");
    m_btn_confirm->SetBorderHoverColor("#65A79E");
    m_btn_confirm->SetFontPressColor(*wxWHITE);
    m_btn_confirm->SetBGPressColor("#1A8676");
    m_btn_confirm->SetBorderPressColor("#1A8676");
    m_btn_confirm->SetFontDisableColor(*wxWHITE);
    m_btn_confirm->SetBGDisableColor("#dddddd");
    m_btn_confirm->SetBorderDisableColor("#dddddd");


    wxSizer* sizer_lbl = new wxBoxSizer(wxHORIZONTAL);
    sizer_lbl->AddStretchSpacer(1);
    sizer_lbl->Add(msg_lbl);
    sizer_lbl->AddStretchSpacer(1);

    wxSizer* sizer_btn = new wxBoxSizer(wxHORIZONTAL);
    sizer_btn->AddStretchSpacer(1);
    sizer_btn->Add(m_btn_confirm);
    sizer_btn->AddStretchSpacer(1);


    m_sizer_main->AddSpacer(FromDIP(52));
    m_sizer_main->Add(msg_lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(30));
    m_sizer_main->Add(sizer_btn, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(52));

    SetSizer(m_sizer_main);

    m_btn_confirm->Bind(wxEVT_BUTTON, ([this](wxCommandEvent& event) { EndModal(wxOK); }));

    this->Layout();
    this->Fit();
    CentreOnParent();
    Layout();
}
void FileOperatorMsgDlg::initDownloadFailedWidget()
{
    SetTitle(_L("Download failed"));
    m_sizer_main = new wxBoxSizer(wxVERTICAL);
    m_sizer_main->SetMinSize(wxSize(FromDIP(380), FromDIP(200)));

    wxStaticText* msg_lbl = new wxStaticText(this, wxID_ANY, _L("Download failed, please try again later!"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    msg_lbl->SetFont(::Label::Body_16);
    msg_lbl->SetForegroundColour("#333333");

    m_btn_confirm = new FFButton(this, wxID_ANY, wxEmptyString);
    m_btn_confirm->SetLabel(_L("Confirm"), FromDIP(80), FromDIP(44));
    m_btn_confirm->SetFontColor(*wxWHITE);
    m_btn_confirm->SetBGColor("#419488");
    m_btn_confirm->SetBorderColor("#419488");
    m_btn_confirm->SetFontHoverColor(*wxWHITE);
    m_btn_confirm->SetBGHoverColor("#65A79E");
    m_btn_confirm->SetBorderHoverColor("#65A79E");
    m_btn_confirm->SetFontPressColor(*wxWHITE);
    m_btn_confirm->SetBGPressColor("#1A8676");
    m_btn_confirm->SetBorderPressColor("#1A8676");
    m_btn_confirm->SetFontDisableColor(*wxWHITE);
    m_btn_confirm->SetBGDisableColor("#dddddd");
    m_btn_confirm->SetBorderDisableColor("#dddddd");

    wxSizer* sizer_lbl = new wxBoxSizer(wxHORIZONTAL);
    sizer_lbl->AddStretchSpacer(1);
    sizer_lbl->Add(msg_lbl);
    sizer_lbl->AddStretchSpacer(1);

    wxSizer* sizer_btn = new wxBoxSizer(wxHORIZONTAL);
    sizer_btn->AddStretchSpacer(1);
    sizer_btn->Add(m_btn_confirm);
    sizer_btn->AddStretchSpacer(1);

    m_sizer_main->AddSpacer(FromDIP(52));
    m_sizer_main->Add(msg_lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(30));
    m_sizer_main->Add(sizer_btn, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(52));

    SetSizer(m_sizer_main);

    m_btn_confirm->Bind(wxEVT_BUTTON, ([this](wxCommandEvent& event) { EndModal(wxOK); }));

    this->Layout();
    this->Fit();
    CentreOnParent();
    Layout();
}
void FileOperatorMsgDlg::initDeleteWidget()
{
    SetTitle(_L("Delete file"));
    m_sizer_main = new wxBoxSizer(wxVERTICAL);
    m_sizer_main->SetMinSize(wxSize(FromDIP(380), FromDIP(200)));

    wxStaticText* msg_lbl = new wxStaticText(this, wxID_ANY, _L("Do you want to delete the selected file?"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    msg_lbl->SetFont(::Label::Body_16);
    msg_lbl->SetForegroundColour("#419488");

    m_btn_yes = new FFButton(this, wxID_ANY, wxEmptyString);
    m_btn_yes->SetLabel(_L("Yes"), FromDIP(80), FromDIP(44));
    m_btn_yes->SetFontColor(*wxWHITE);
    m_btn_yes->SetBGColor("#419488");
    m_btn_yes->SetBorderColor("#419488");
    m_btn_yes->SetFontHoverColor(*wxWHITE);
    m_btn_yes->SetBGHoverColor("#65A79E");
    m_btn_yes->SetBorderHoverColor("#65A79E");
    m_btn_yes->SetFontPressColor(*wxWHITE);
    m_btn_yes->SetBGPressColor("#1A8676");
    m_btn_yes->SetBorderPressColor("#1A8676");
    m_btn_yes->SetFontDisableColor(*wxWHITE);
    m_btn_yes->SetBGDisableColor("#dddddd");
    m_btn_yes->SetBorderDisableColor("#dddddd");

    m_btn_no = new FFButton(this, wxID_ANY, wxEmptyString);
    m_btn_no->SetLabel(_L("No"), FromDIP(80), FromDIP(44));
    m_btn_no->SetFontColor(*wxWHITE);
    m_btn_no->SetBGColor("#419488");
    m_btn_no->SetBorderColor("#419488");
    m_btn_no->SetFontHoverColor(*wxWHITE);
    m_btn_no->SetBGHoverColor("#65A79E");
    m_btn_no->SetBorderHoverColor("#65A79E");
    m_btn_no->SetFontPressColor(*wxWHITE);
    m_btn_no->SetBGPressColor("#1A8676");
    m_btn_no->SetBorderPressColor("#1A8676");
    m_btn_no->SetFontDisableColor(*wxWHITE);
    m_btn_no->SetBGDisableColor("#dddddd");
    m_btn_no->SetBorderDisableColor("#dddddd");

    wxSizer* sizer_lbl = new wxBoxSizer(wxHORIZONTAL);
    sizer_lbl->AddStretchSpacer(1);
    sizer_lbl->Add(msg_lbl);
    sizer_lbl->AddStretchSpacer(1);

    wxSizer* sizer_btn = new wxBoxSizer(wxHORIZONTAL);
    sizer_btn->AddStretchSpacer(1);
    sizer_btn->Add(m_btn_yes);
    sizer_btn->AddSpacer(FromDIP(30));
    sizer_btn->Add(m_btn_no);
    sizer_btn->AddStretchSpacer(1);

    m_sizer_main->AddSpacer(FromDIP(52));
    m_sizer_main->Add(msg_lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(30));
    m_sizer_main->Add(sizer_btn, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(52));

    SetSizer(m_sizer_main);

    m_btn_yes->Bind(wxEVT_BUTTON, ([this](wxCommandEvent& event) { EndModal(wxYES); }));
    m_btn_no->Bind(wxEVT_BUTTON, ([this](wxCommandEvent& event) { EndModal(wxNO); }));

    this->Layout();
    this->Fit();
    CentreOnParent();
    Layout();
}

} // namespace GUI
} // namespace Slic3r