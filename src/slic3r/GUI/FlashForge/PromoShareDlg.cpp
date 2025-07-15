#include "PromoShareDlg.hpp"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

namespace Slic3r { namespace GUI {

PromoShareUrlInput::PromoShareUrlInput(wxWindow *parent)
    : wxPanel(parent)
    , m_radius(FromDIP(12))
    , m_backgroundColor("#dedede")
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_staticTxt = new wxStaticText(this, wxID_ANY, "");
    m_staticTxt->SetBackgroundColour(m_backgroundColor);

    Bind(wxEVT_PAINT, &PromoShareUrlInput::onPaint, this);
    Bind(wxEVT_SIZE, &PromoShareUrlInput::onSize, this);
}

void PromoShareUrlInput::setText(const wxString &text)
{
    m_staticTxt->SetLabelText(text);
}

void PromoShareUrlInput::onPaint(wxPaintEvent &event)
{
    wxBufferedPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(*wxWHITE);
    gc->DrawRectangle(0, 0, GetSize().x, GetSize().y);
    gc->SetBrush(m_backgroundColor);
    gc->DrawRoundedRectangle(0, 0, GetSize().x, GetSize().y, m_radius);
}

void PromoShareUrlInput::onSize(wxSizeEvent &event)
{
    m_staticTxt->Move(m_radius, (event.m_size.y - m_staticTxt->GetCharHeight()) / 2);
    m_staticTxt->SetSize(event.m_size.x - m_radius * 2, m_staticTxt->GetCharHeight());
}

PromoShareDlg::PromoShareDlg(wxWindow *parent)
    : FFTitleLessDialog(parent)
    , m_totalWidth(FromDIP(393))
    , m_contentWidth(FromDIP(329))
    , m_topSpace(FromDIP(32))
    , m_iconHeight(FromDIP(88))
    , m_iconSpace(FromDIP(14))
    , m_titleSpace(FromDIP(14))
    , m_message1Space(FromDIP(6))
    , m_message2Space(FromDIP(13))
    , m_urlInputSpace(FromDIP(15))
    , m_buttonSpace(FromDIP(17))
    , m_message3Space(FromDIP(32))
    , m_iconBmp(this, "promo_share_icon", 32)
    , m_background1Bmp(this, "promo_share_bg1", 38)
    , m_background2Bmp(this, "promo_share_bg2", 80)
{
    m_urlInput = new PromoShareUrlInput(this);
    m_urlInput->SetSize(wxSize(m_contentWidth, FromDIP(40)));
    m_urlInput->SetMinSize(wxSize(m_contentWidth, FromDIP(40)));
    m_urlInput->SetMaxSize(wxSize(m_contentWidth, FromDIP(40)));

    m_copyBtn = new FFButton(this, wxID_ANY, "", FromDIP(4), false);
    m_copyBtn->SetDoubleBuffered(true);
    m_copyBtn->SetFontUniformColor(*wxWHITE);
    m_copyBtn->SetBGColor(wxColour("#419488"));
    m_copyBtn->SetBGHoverColor(wxColour("#65A79E"));
    m_copyBtn->SetBGPressColor(wxColour("#1A8676"));
    m_copyBtn->SetSize(wxSize(FromDIP(138), FromDIP(30)));
    m_copyBtn->SetMinSize(wxSize(FromDIP(138), FromDIP(30)));
    m_copyBtn->SetMaxSize(wxSize(FromDIP(138), FromDIP(30)));

    initData();
    initSize();
    CenterOnParent();
}

void PromoShareDlg::initData()
{
}

void PromoShareDlg::initSize()
{
    m_titleHeight = 0;
    m_message1Height = 0;
    m_message2Height = 0;
    m_message3Height = 0;

    int urlInputHeight = m_urlInput->GetSize().y;
    int copyBtnHeight = m_copyBtn->GetSize().y;
    int titleHeight = m_topSpace + m_iconHeight + m_iconSpace + m_titleHeight + m_titleSpace;
    int urlInputTop = titleHeight+ m_message1Height + m_message1Space + m_message2Height + m_message2Space;
    int copyBtnTop = urlInputTop + urlInputHeight + m_urlInputSpace;
    int totalHeight = copyBtnTop + copyBtnHeight + m_buttonSpace + m_message3Height + m_message3Space;
    SetSize(wxSize(m_totalWidth, totalHeight));
    SetMinSize(wxSize(m_totalWidth, totalHeight));
    SetMaxSize(wxSize(m_totalWidth, totalHeight));

    m_urlInput->Move((m_totalWidth - m_urlInput->GetSize().x) / 2, urlInputTop);
    m_copyBtn->Move((m_totalWidth - m_copyBtn->GetSize().x) / 2, copyBtnTop);
}

}} // namespace Slic3r::GUI
