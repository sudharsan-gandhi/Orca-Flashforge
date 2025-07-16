#include "PromoShareDlg.hpp"
#include <nlohmann/json.hpp>
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

namespace Slic3r { namespace GUI {

PromoShareUrlInput::PromoShareUrlInput(wxWindow *parent)
    : wxPanel(parent)
    , m_radius(FromDIP(12))
    , m_backgroundColor("#dedede")
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_staticTxt = new wxStaticText(this, wxID_ANY, "");
    m_staticTxt->SetWindowStyleFlag(wxALIGN_CENTER | wxST_ELLIPSIZE_MIDDLE);
    m_staticTxt->SetBackgroundColour(m_backgroundColor);

    Bind(wxEVT_PAINT, &PromoShareUrlInput::onPaint, this);
    Bind(wxEVT_SIZE, &PromoShareUrlInput::onSize, this);
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
    int textWidth = event.m_size.x - m_radius * 2;
    int textHeight = m_staticTxt->GetCharHeight();
    m_staticTxt->Move(m_radius, (event.m_size.y - textHeight) / 2);
    m_staticTxt->SetSize(wxSize(textWidth, textHeight));
    m_staticTxt->SetMinSize(wxSize(textWidth, textHeight));
    m_staticTxt->SetMaxSize(wxSize(textWidth, textHeight));
}

PromoShareDlg::PromoShareDlg(wxWindow *parent, const std::string &data)
    : FFTitleLessDialog(parent)
    , m_title(_CTX("Refer & Earn", "Flashforge"))
    , m_iconBmp(this, "promo_share_icon", 88)
    , m_bg1Bmp(this, "promo_share_bg1", 38)
    , m_bg2Bmp(this, "promo_share_bg2", 80)
    , m_totalWidth(FromDIP(393))
    , m_contentWidth(FromDIP(329))
    , m_topSpace(FromDIP(32))
    , m_iconHeight(FromDIP(88))
    , m_iconSpace(FromDIP(14))
    , m_titleSpace(FromDIP(14))
    , m_message1Space(FromDIP(6))
    , m_message2Space(FromDIP(13))
    , m_urlInputSpace(FromDIP(15))
    , m_buttonSpace(FromDIP(32))
{
    m_urlInput = new PromoShareUrlInput(this);
    m_urlInput->SetSize(wxSize(m_contentWidth, FromDIP(32)));
    m_urlInput->SetMinSize(wxSize(m_contentWidth, FromDIP(32)));
    m_urlInput->SetMaxSize(wxSize(m_contentWidth, FromDIP(32)));

    m_copyBtn = new FFButton(this, wxID_ANY, _L("Copy Referral Link"), FromDIP(4), false);
    m_copyBtn->SetDoubleBuffered(true);
    m_copyBtn->SetFontUniformColor(*wxWHITE);
    m_copyBtn->SetBGColor(wxColour("#419488"));
    m_copyBtn->SetBGHoverColor(wxColour("#65A79E"));
    m_copyBtn->SetBGPressColor(wxColour("#1A8676"));

    initData(data);
    initSize();
    CenterOnParent();
    m_copyBtn->Bind(wxEVT_BUTTON, &PromoShareDlg::onCopyPromoShareUrl, this);
}

void PromoShareDlg::drawBackground(wxBufferedPaintDC &dc, wxGraphicsContext *gc)
{
    FFTitleLessDialog::drawBackground(dc, gc);
    int bg2X = m_totalWidth - m_bg2Bmp.GetBmpWidth();
    int bg2Y = GetSize().y - m_bg2Bmp.GetBmpHeight();
    int iconX = (m_totalWidth - m_iconBmp.GetBmpWidth()) / 2;
    gc->DrawBitmap(m_bg1Bmp.bmp(), 0, FromDIP(42), m_bg1Bmp.GetBmpWidth(), m_bg1Bmp.GetBmpHeight());
    gc->DrawBitmap(m_bg2Bmp.bmp(), bg2X, bg2Y, m_bg2Bmp.GetBmpWidth(), m_bg2Bmp.GetBmpHeight());
    gc->DrawBitmap(m_iconBmp.bmp(), iconX, m_topSpace, m_iconBmp.GetBmpWidth(), m_iconBmp.GetBmpHeight());

    int titleY = m_topSpace + m_iconBmp.GetBmpHeight() + m_iconSpace;
    dc.SetFont(Label::Body_15);
    dc.SetTextForeground("#333333");
    dc.DrawText(m_title, (m_totalWidth - m_titleSize.x) / 2, titleY);

    int messageX = (m_totalWidth - m_contentWidth) / 2;
    int message1Y = titleY + m_titleSize.y + m_titleSpace;
    dc.SetFont(Label::Body_13);
    dc.DrawText(m_message1, messageX, message1Y);

    int message2Y = message1Y + m_message1Size.y + m_message1Space;
    dc.DrawText(m_message2, messageX, message2Y);
}

void PromoShareDlg::initData(const std::string &data)
{
    auto getTempStr = [](int minCnt, int maxCnt, char space) {
        std::string ret;
        int cnt = rand() % (maxCnt - minCnt + 1) + minCnt;
        for (int i = 0; i < cnt; ++i) {
            int itemLen = rand() % 15 + 1;
            for (int j = 0; j < itemLen; ++j) {
                ret.push_back('a' + rand() % 26);
            }
            if (i != cnt - 1) {
                ret.push_back(space);
            }
        }
        return ret;
    };
    m_urlInput->setText("https://" + getTempStr(1, 5, '.') + ".com");
    m_message1 = getTempStr(10, 50, ' ');
    m_message2 = getTempStr(10, 50, ' ');
}

void PromoShareDlg::initSize()
{
    wxBufferedPaintDC dc(this);
    dc.SetFont(Label::Body_14);
    m_titleSize = dc.GetTextExtent(m_title);

    dc.SetFont(Label::Body_13);
    m_message1Size = Label::split_lines(dc, m_contentWidth, m_message1, m_message1);
    m_message2Size = Label::split_lines(dc, m_contentWidth, m_message2, m_message2);

    wxSize copyBtnTextSize = m_copyBtn->GetTextExtent(m_copyBtn->GetLabel());
    int copyBtnWidth = std::max(copyBtnTextSize.x + FromDIP(8), FromDIP(138));
    m_copyBtn->SetSize(wxSize(copyBtnWidth, FromDIP(30)));
    m_copyBtn->SetMinSize(wxSize(copyBtnWidth, FromDIP(30)));
    m_copyBtn->SetMaxSize(wxSize(copyBtnWidth, FromDIP(30)));

    int urlInputHeight = m_urlInput->GetSize().y;
    int copyBtnHeight = m_copyBtn->GetSize().y;
    int titleHeight = m_topSpace + m_iconHeight + m_iconSpace + m_titleSize.y + m_titleSpace;
    int urlInputTop = titleHeight+ m_message1Size.y + m_message1Space + m_message2Size.y + m_message2Space;
    int copyBtnTop = urlInputTop + urlInputHeight + m_urlInputSpace;
    int totalHeight = copyBtnTop + copyBtnHeight + m_buttonSpace;
    SetSize(wxSize(m_totalWidth, totalHeight));
    SetMinSize(wxSize(m_totalWidth, totalHeight));
    SetMaxSize(wxSize(m_totalWidth, totalHeight));

    m_urlInput->Move((m_totalWidth - m_urlInput->GetSize().x) / 2, urlInputTop);
    m_copyBtn->Move((m_totalWidth - m_copyBtn->GetSize().x) / 2, copyBtnTop);
}

void PromoShareDlg::onCopyPromoShareUrl(wxCommandEvent &event)
{
    if (!wxTheClipboard->Open()) {
        return;
    }
    wxTheClipboard->SetData(new wxTextDataObject(m_urlInput->getText()));
    wxTheClipboard->Flush();
    wxTheClipboard->Close();
}

}} // namespace Slic3r::GUI
