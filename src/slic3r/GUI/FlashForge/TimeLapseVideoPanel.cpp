#include "TimeLapseVideoPanel.hpp"

namespace Slic3r { namespace GUI {

TimeLapseVideoItem::TimeLapseVideoItem(wxWindow *parent)
    : wxPanel(parent)
    , m_fileName("test_file_name")
{
    SetMinSize(wxSize(FromDIP(124), FromDIP(96)));
    SetMaxSize(wxSize(FromDIP(124), FromDIP(96)));

    m_checkBox = new FFCheckBox(this);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_checkBox, 0, wxLEFT | wxTOP, FromDIP(5));
    SetSizer(sizer);

    Bind(wxEVT_PAINT, &TimeLapseVideoItem::onPaint, this);
}

void TimeLapseVideoItem::onPaint(wxPaintEvent &event)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    wxSize size = GetSize();
    int imgHeight = size.GetHeight() * 0.73;
    gc->SetPen(wxColour("#e3e2e2"));
    gc->SetBrush(wxColour("#e3e2e2"));
    gc->DrawRectangle(0, 0, size.GetWidth(), imgHeight);

    wxSize textSize = dc.GetTextExtent(m_fileName);
    int textLineHeight = size.y - imgHeight - textSize.y;
    dc.DrawText(m_fileName, (size.x - textSize.x) / 2, imgHeight + textLineHeight / 2);
}

TimeLapseVideoPanel::TimeLapseVideoPanel(wxWindow *parent)
    : wxPanel(parent)
{
    SetBackgroundColour(*wxWHITE);
    SetDoubleBuffered(true);
    SetMinSize(wxSize(FromDIP(450), FromDIP(411)));
    SetMaxSize(wxSize(FromDIP(450), FromDIP(411)));

    m_scr = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_scr->SetMinSize(wxSize(FromDIP(450), FromDIP(351)));
    m_scr->SetMaxSize(wxSize(FromDIP(450), FromDIP(351)));
    m_scr->SetScrollRate(0, 30);

    m_deleteBtn = new FFButton(this);
    m_deleteBtn->SetFontColor("#419488");
    m_deleteBtn->SetBorderColor("#419488");
    m_deleteBtn->SetFontHoverColor("#65A79E");
    m_deleteBtn->SetBorderHoverColor("#65A79E");
    m_deleteBtn->SetFontPressColor("#1A8676");
    m_deleteBtn->SetBorderPressColor("#1A8676");
    m_deleteBtn->SetFontDisableColor("#dddddd");
    m_deleteBtn->SetBGDisableColor(*wxWHITE);
    m_deleteBtn->SetBorderDisableColor("#dddddd");
    m_deleteBtn->SetLabel(_L("Delete"), FromDIP(80), FromDIP(32));
    m_deleteBtn->SetEnable(false);

    m_downloadBtn = new FFButton(this);
    m_downloadBtn->SetFontColor(*wxWHITE);
    m_downloadBtn->SetBGColor("#419488");
    m_downloadBtn->SetBorderColor("#419488");
    m_downloadBtn->SetFontHoverColor(*wxWHITE);
    m_downloadBtn->SetBGHoverColor("#65A79E");
    m_downloadBtn->SetBorderHoverColor("#65A79E");
    m_downloadBtn->SetFontPressColor(*wxWHITE);
    m_downloadBtn->SetBGPressColor("#1A8676");
    m_downloadBtn->SetBorderPressColor("#1A8676");
    m_downloadBtn->SetFontDisableColor(*wxWHITE);
    m_downloadBtn->SetBGDisableColor("#dddddd");
    m_downloadBtn->SetBorderDisableColor("#dddddd");
    m_downloadBtn->SetLabel(_L("Download"), FromDIP(80), FromDIP(32));
    m_downloadBtn->SetEnable(false);

    m_btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnSizer->AddStretchSpacer(1);
    m_btnSizer->Add(m_deleteBtn);
    m_btnSizer->AddSpacer(FromDIP(16));
    m_btnSizer->Add(m_downloadBtn);

    m_itemSizer = new wxGridSizer(3, FromDIP(8), FromDIP(16));
    wxSizer *scrSizer = new wxBoxSizer(wxHORIZONTAL);
    scrSizer->AddStretchSpacer(1);
    scrSizer->Add(m_itemSizer, 0, wxTOP, FromDIP(16));
    scrSizer->AddStretchSpacer(1);
    m_scr->SetSizer(scrSizer);

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_scr);
    sizer->AddStretchSpacer(1);
    sizer->Add(m_btnSizer, 0, wxEXPAND | wxRIGHT, FromDIP(16));
    sizer->AddStretchSpacer(1);
    SetSizer(sizer);

#if 1
    size_t itemCnt = 20;
    for (size_t i = 0; i < itemCnt; ++i) {
        TimeLapseVideoItem *item = new TimeLapseVideoItem(m_scr);
        m_itemSizer->Add(item, 0, wxALIGN_CENTER);
    }
    m_scr->SetVirtualSize(-1, m_itemSizer->GetMinSize().y);
#endif
}

}} // namespace Slic3r::GUI
