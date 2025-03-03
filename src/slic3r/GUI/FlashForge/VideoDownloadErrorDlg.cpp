#include "VideoDownloadErrorDlg.hpp"

#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "../wxExtensions.hpp"

namespace Slic3r {
namespace GUI {

VideoDownloadErrorDlg::VideoDownloadErrorDlg(wxWindow* parent, const std::vector<std::string>& file_infos) 
    :TitleDialog(parent, _L("Tips"), 6), m_file_infos(file_infos)
{

    m_sizer_main = MainSizer();
    m_sizer_main->SetMinSize(wxSize(FromDIP(380), FromDIP(370)));


    m_msg_Lbl = new wxStaticText(this, wxID_ANY, _L("Partial video download failed, please try again later!"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    m_msg_Lbl->SetFont(::Label::Body_16);
    m_msg_Lbl->SetForegroundColour("#333333");

    m_list_Lbl = new wxStaticText(this, wxID_ANY, _L("Failed video"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    m_list_Lbl->SetFont(::Label::Body_14);
    m_list_Lbl->SetForegroundColour("#333333");


    m_btn_confirm = new FFButton(this, wxID_ANY, wxEmptyString);
    m_btn_confirm->SetLabel(_L("Confirm"), FromDIP(100), FromDIP(44));
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


    m_scroll_wgt = new wxScrolledWindow(this, wxID_ANY);
    m_scroll_wgt->SetSize(wxSize(FromDIP(380), FromDIP(446)));

    m_scroll_wgt->SetVirtualSize(1000, 500);
    m_scroll_wgt->SetScrollRate(10, 10);

    m_sizer_scroll = new wxBoxSizer(wxVERTICAL);
    for (int i = 0; i < m_file_infos.size(); i++) {
        std::string file_info = m_file_infos.at(i);
        wxBoxSizer* sizer_item = new wxBoxSizer(wxHORIZONTAL);
        wxBitmap    error_bmp(create_scaled_bitmap("video_download_error", nullptr, 16));
        wxStaticBitmap* staticBitmap = new wxStaticBitmap(m_scroll_wgt, wxID_ANY, error_bmp);
        wxStaticText*   file_Lbl     = new wxStaticText(m_scroll_wgt, wxID_ANY, _L(file_info.c_str()),
                                                 wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxALIGN_CENTER);
        file_Lbl->SetFont(::Label::Body_14);
        file_Lbl->SetForegroundColour("#333333");
        sizer_item->Add(staticBitmap);
        sizer_item->AddSpacer(FromDIP(5));
        sizer_item->Add(file_Lbl);
        m_sizer_scroll->AddSpacer(12);
        m_sizer_scroll->Add(sizer_item);
    }
    m_scroll_wgt->SetSizer(m_sizer_scroll);


    wxSizer* sizer_btn = new wxBoxSizer(wxHORIZONTAL);
    sizer_btn->AddStretchSpacer(1);
    sizer_btn->Add(m_btn_confirm);
    sizer_btn->AddStretchSpacer(1);


    m_sizer_main->AddSpacer(FromDIP(38));
    m_sizer_main->Add(m_msg_Lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(25));
    m_sizer_main->Add(m_list_Lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->Add(m_scroll_wgt, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(40));
    m_sizer_main->Add(sizer_btn, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    m_sizer_main->AddSpacer(FromDIP(40));

    //SetSize(wxSize(400, 300));
    this->Layout();
    this->Fit();
    CentreOnParent();
    Layout();

    m_btn_confirm->Bind(wxEVT_BUTTON, ([this](wxCommandEvent& event) { EndModal(wxOK); }));

}

VideoDownloadErrorDlg::~VideoDownloadErrorDlg()
{

}

void VideoDownloadErrorDlg::test_add_items() {

    for (int i = 0; i < 20; i++) {
        wxButton* button = new wxButton(m_scroll_wgt, wxID_ANY, wxString::Format("Button %d", i + 1));
        m_sizer_scroll->Add(button, 0, wxALL, 5);
    }

}


TestScrollWidget::TestScrollWidget(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxEmptyString)
{
    // 创建一个垂直方向的 wxBoxSizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 创建一个无滚动条的 wxScrolledWindow
    wxScrolledWindow* scrolledWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    scrolledWindow->SetScrollRate(10, 10);

    // 创建一个垂直滚动条
    wxScrollBar* scrollBar = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
    scrollBar->SetScrollbar(0, 10, 100, 10);

    // 设置滚动条的背景色为透明
    scrollBar->SetBackgroundColour(wxColour(255, 0, 0)); // 设置为白色或其他颜色

    // 设置布局
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(scrolledWindow, 1, wxEXPAND);

    wxBoxSizer* sizer_scroll_area = new wxBoxSizer(wxHORIZONTAL);
    sizer_scroll_area->Add(sizer, 1, wxEXPAND);
    sizer_scroll_area->Add(scrollBar, 0, wxEXPAND);


    wxStaticText* m_msg_Lbl = new wxStaticText(this, wxID_ANY, _L("Partial video download failed, please try again later!"),
                                             wxDefaultPosition,
                                 wxDefaultSize, wxALIGN_CENTER);
    m_msg_Lbl->SetFont(::Label::Body_16);
    m_msg_Lbl->SetForegroundColour("#333333");

    wxStaticText* m_list_Lbl = new wxStaticText(this, wxID_ANY, _L("Failed video"), wxDefaultPosition, wxDefaultSize,
                                  wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    m_list_Lbl->SetFont(::Label::Body_12);
    m_list_Lbl->SetForegroundColour("#333333");

    // 在 scrolledWindow 中添加内容
    wxBoxSizer* scrollSizer = new wxBoxSizer(wxVERTICAL);
    for (int i = 0; i < 20; i++) {
        wxButton* button = new wxButton(scrolledWindow, wxID_ANY, wxString::Format("Button %d", i + 1));
        scrollSizer->Add(button, 0, wxALL, 5);
    }
    scrolledWindow->SetSizer(scrollSizer);

    // 将 scrolledWindow 添加到主 sizer 中，并设置其扩展比例
    mainSizer->AddSpacer(FromDIP(38));
    mainSizer->Add(m_msg_Lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    mainSizer->AddSpacer(FromDIP(25));
    mainSizer->Add(m_list_Lbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    mainSizer->Add(sizer_scroll_area, 1, wxEXPAND | wxALL, 5);
    mainSizer->AddSpacer(FromDIP(40));

    FFButton* m_btn_confirm = new FFButton(this, wxID_ANY, wxEmptyString);
    m_btn_confirm->SetLabel(_L("Confirm"), FromDIP(100), FromDIP(44));
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


    wxSizer* sizer_btn = new wxBoxSizer(wxHORIZONTAL);
    sizer_btn->AddStretchSpacer(1);
    sizer_btn->Add(m_btn_confirm);
    sizer_btn->AddStretchSpacer(1);


    // 将按钮区域添加到主 sizer 中
    mainSizer->Add(sizer_btn, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(15));
    mainSizer->AddSpacer(FromDIP(40));

    // 设置主布局管理器
    SetSizer(mainSizer);

    // 调整窗口大小
    SetSize(380, 370);
}

//TestScrollWidget::TestScrollWidget(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxEmptyString)
//{
//    // 创建一个垂直方向的 wxBoxSizer
//    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
//
//    // 创建一个 wxScrolledWindow
//    wxScrolledWindow* scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
//
//    // 设置虚拟大小和滚动步长
//    scrolledWindow->SetVirtualSize(1000, 1000);
//    scrolledWindow->SetScrollRate(10, 10);
//
//    // 在 scrolledWindow 中添加内容
//    wxBoxSizer* scrollSizer = new wxBoxSizer(wxVERTICAL);
//    for (int i = 0; i < 20; i++) {
//        wxButton* button = new wxButton(scrolledWindow, wxID_ANY, wxString::Format("Button %d", i + 1));
//        scrollSizer->Add(button, 0, wxALL, 5);
//    }
//    scrolledWindow->SetSizer(scrollSizer);
//
//    // 将 scrolledWindow 添加到主 sizer 中，并设置其扩展比例
//    mainSizer->Add(scrolledWindow, 1, wxEXPAND | wxALL, 5);
//
//    // 创建一个按钮区域
//    wxPanel*    buttonPanel = new wxPanel(this, wxID_ANY);
//    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
//
//    wxButton* okButton     = new wxButton(buttonPanel, wxID_ANY, "OK");
//    wxButton* cancelButton = new wxButton(buttonPanel, wxID_ANY, "Cancel");
//
//    buttonSizer->Add(okButton, 0, wxALL, 5);
//    buttonSizer->Add(cancelButton, 0, wxALL, 5);
//
//    buttonPanel->SetSizer(buttonSizer);
//
//    // 将按钮区域添加到主 sizer 中
//    mainSizer->Add(buttonPanel, 0, wxALIGN_CENTER | wxBOTTOM, 10);
//
//    // 设置主布局管理器
//    SetSizer(mainSizer);
//
//    // 调整窗口大小
//    SetSize(400, 300);
//}

TestScrollWidget::~TestScrollWidget()
{

}


}} // namespace Slic3r::GUI