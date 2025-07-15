#ifndef slic3r_GUI_PromoShareDlg_hpp_
#define slic3r_GUI_PromoShareDlg_hpp_

#include <string>
#include <wx/colour.h>
#include <wx/event.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include "FFTitleLessDialog.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class PromoShareUrlInput : public wxPanel
{
public:
    PromoShareUrlInput(wxWindow *parent);

    void setText(const wxString &text);

private:
    void onPaint(wxPaintEvent &event);

    void onSize(wxSizeEvent &event);

private:
    int           m_radius;
    wxColour      m_backgroundColor;
    wxStaticText *m_staticTxt;
};

class PromoShareDlg : public FFTitleLessDialog
{
public:
    PromoShareDlg(wxWindow *parent);

private:
    void initData();

    void initSize();

private:
    PromoShareUrlInput *m_urlInput;
    FFButton           *m_copyBtn;
    std::string         m_sharingUrl;
    wxString            m_message1;
    wxString            m_message2;
    wxString            m_message3;
    ScalableBitmap      m_iconBmp;
    ScalableBitmap      m_background1Bmp;
    ScalableBitmap      m_background2Bmp;
    const int           m_totalWidth;
    const int           m_contentWidth;
    const int           m_topSpace;
    const int           m_iconHeight;
    const int           m_iconSpace;
    const int           m_titleSpace;
    const int           m_message1Space;
    const int           m_message2Space;
    const int           m_urlInputSpace;
    const int           m_buttonSpace;
    const int           m_message3Space;
    int                 m_titleHeight;
    int                 m_message1Height;
    int                 m_message2Height;
    int                 m_message3Height;
};

}} // namespace Slic3r::GUI

#endif
