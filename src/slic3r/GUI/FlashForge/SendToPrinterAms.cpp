#include "SendToPrinterAms.hpp"
#include <wx/dcgraph.h>
#include "slic3r/GUI/GUI_App.hpp"

namespace Slic3r { namespace GUI {

#define MATERIAL_ITEM_SIZE wxSize(FromDIP(64), FromDIP(34))
#define MATERIAL_ITEM_REAL_SIZE wxSize(FromDIP(62), FromDIP(32))

MaterialMatchWgt::MaterialMatchWgt(wxWindow *parent, wxColour mcolour, wxString mname)
 : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
 {
    m_arraw_bitmap_gray =  ScalableBitmap(this, "drop_down", FromDIP(12));
    m_arraw_bitmap_white =  ScalableBitmap(this, "topbar_dropdown", FromDIP(12));
    m_transparent_mitem = ScalableBitmap(this, "transparent_material_item", FromDIP(32));

    m_material_coloul = mcolour;
    m_material_name = mname;
    m_ams_coloul      = wxColour(0xEE,0xEE,0xEE);

#ifdef __WINDOWS__
    SetDoubleBuffered(true);
#endif //__WINDOWS__

    SetSize(MATERIAL_ITEM_SIZE);
    SetMinSize(MATERIAL_ITEM_SIZE);
    SetMaxSize(MATERIAL_ITEM_SIZE);
    SetBackgroundColour(*wxWHITE);

    Bind(wxEVT_PAINT, &MaterialMatchWgt::paintEvent, this);
    wxGetApp().UpdateDarkUI(this);
}

void MaterialMatchWgt::msw_rescale() {
    m_arraw_bitmap_gray  = ScalableBitmap(this, "drop_down", FromDIP(12));
    m_arraw_bitmap_white = ScalableBitmap(this, "topbar_dropdown", FromDIP(12));
    m_transparent_mitem  = ScalableBitmap(this, "transparent_material_item", FromDIP(32));
}

void MaterialMatchWgt::set_ams_info(wxColour col, wxString txt, int ctype, std::vector<wxColour> cols)
{
    auto need_refresh = false;
    if (m_ams_cols != cols) { m_ams_cols = cols; need_refresh = true; }
    if (m_ams_ctype != ctype) { m_ams_ctype = ctype; need_refresh = true; }
    if (m_ams_coloul != col) { m_ams_coloul = col; need_refresh = true;}
    if (m_ams_name != txt) { m_ams_name = txt; need_refresh = true; }
    if (need_refresh) { Refresh();}
}

void MaterialMatchWgt::disable()
{
    if (IsEnabled()) {
        this->Disable();
        Refresh();
    }
}

void MaterialMatchWgt::enable()
{
    if (!IsEnabled()) {
        this->Enable();
        Refresh();
    }
}

void MaterialMatchWgt::on_selected()
{
    if (!m_selected) {
        m_selected = true;
        Refresh();
    }
}

void MaterialMatchWgt::on_warning()
{
    if (!m_warning) {
        m_warning = true;
        Refresh();
    }
}

void MaterialMatchWgt::on_normal()
{
    if (m_selected || m_warning) {
        m_selected = false;
        m_warning  = false;
        Refresh();
    }
}


void MaterialMatchWgt::paintEvent(wxPaintEvent &evt)
{  
    wxPaintDC dc(this);
    render(dc);
}

void MaterialMatchWgt::render(wxDC &dc)
{
#ifdef __WXMSW__
    wxSize     size = GetSize();
    wxMemoryDC memdc;
    wxBitmap   bmp(size.x, size.y);
    memdc.SelectObject(bmp);
    memdc.Blit({0, 0}, size, &dc, {0, 0});

    {
        wxGCDC dc2(memdc);
        doRender(dc2);
    }

    memdc.SelectObject(wxNullBitmap);
    dc.DrawBitmap(bmp, 0, 0);
#else
    doRender(dc);
#endif

    auto mcolor = m_material_coloul;
    auto acolor = m_ams_coloul;
    change_the_opacity(acolor);
    if (!IsEnabled()) {
        mcolor = wxColour(0x90, 0x90, 0x90);
        acolor = wxColour(0x90, 0x90, 0x90);
    }

    // materials name
    dc.SetFont(::Label::Body_13);

    auto material_name_colour = mcolor.GetLuminance() < 0.6 ? *wxWHITE : wxColour(0x26, 0x2E, 0x30);
    if (mcolor.Alpha() == 0) {material_name_colour = wxColour(0x26, 0x2E, 0x30);}
    dc.SetTextForeground(material_name_colour);

    if (dc.GetTextExtent(m_material_name).x > GetSize().x - 10) {
        dc.SetFont(::Label::Body_10);
    }

    auto material_txt_size = dc.GetTextExtent(m_material_name);
    dc.DrawText(m_material_name, wxPoint((MATERIAL_ITEM_SIZE.x - material_txt_size.x) / 2, (FromDIP(22) - material_txt_size.y) / 2));

    // mapping num
    dc.SetFont(::Label::Body_10);
    dc.SetTextForeground(acolor.GetLuminance() < 0.6 ? *wxWHITE : wxColour(0x26, 0x2E, 0x30));
    if (acolor.Alpha() == 0) {
        dc.SetTextForeground(wxColour(0x26, 0x2E, 0x30));
    }

    wxString mapping_txt = wxEmptyString;
    if (m_ams_name.empty()) {
        mapping_txt = "-";
    } else {
        mapping_txt = m_ams_name;
    }

    auto mapping_txt_size = dc.GetTextExtent(mapping_txt);
    dc.DrawText(mapping_txt, wxPoint((MATERIAL_ITEM_SIZE.x - mapping_txt_size.x) / 2, FromDIP(20) + (FromDIP(14) - mapping_txt_size.y) / 2));
}

void MaterialMatchWgt::doRender(wxDC &dc)
{
    wxSize size = GetSize();
    auto mcolor = m_material_coloul;
    auto acolor = m_ams_coloul;
    change_the_opacity(acolor);

    if (mcolor.Alpha() == 0 || acolor.Alpha() == 0) {
        dc.DrawBitmap(m_transparent_mitem.bmp(), FromDIP(1), FromDIP(1));
    }

    if (!IsEnabled()) {
        mcolor = wxColour(0x90, 0x90, 0x90);
        acolor = wxColour(0x90, 0x90, 0x90);
    }

    //top
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(mcolor));
    dc.DrawRoundedRectangle(FromDIP(1), FromDIP(1), MATERIAL_ITEM_REAL_SIZE.x, FromDIP(18), 5);
    
    //bottom
    if (m_ams_cols.size() > 1) {
        int left = FromDIP(1);
        int gwidth = std::round(MATERIAL_ITEM_REAL_SIZE.x / (m_ams_cols.size() - 1));
        //gradient
        if (m_ams_ctype == 0) {
            for (int i = 0; i < m_ams_cols.size() - 1; i++) {
                auto rect = wxRect(left, FromDIP(18), MATERIAL_ITEM_REAL_SIZE.x, FromDIP(16));
                dc.GradientFillLinear(rect, m_ams_cols[i], m_ams_cols[i + 1], wxEAST);
                left += gwidth;
            }
        }
        else {
            int cols_size = m_ams_cols.size();
            for (int i = 0; i < cols_size; i++) {
                dc.SetBrush(wxBrush(m_ams_cols[i]));
                float x = left + ((float)MATERIAL_ITEM_REAL_SIZE.x) * i / cols_size;
                if (i != cols_size - 1) {
                    dc.DrawRoundedRectangle(x, FromDIP(18), ((float)MATERIAL_ITEM_REAL_SIZE.x) / cols_size + FromDIP(3), FromDIP(16), 3);
                }
                else {
                    dc.DrawRoundedRectangle(x, FromDIP(18), ((float)MATERIAL_ITEM_REAL_SIZE.x) / cols_size , FromDIP(16), 3);
                }
            }
 
        }
    }
    else {
        
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(wxColour(acolor)));
        dc.DrawRoundedRectangle(FromDIP(1), FromDIP(18), MATERIAL_ITEM_REAL_SIZE.x, FromDIP(16), 5);
        ////middle

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(acolor));
        dc.DrawRectangle(FromDIP(1), FromDIP(18), MATERIAL_ITEM_REAL_SIZE.x, FromDIP(8));
    }
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(mcolor));
    dc.DrawRectangle(FromDIP(1), FromDIP(11), MATERIAL_ITEM_REAL_SIZE.x, FromDIP(8));



    ////border
#if __APPLE__
    if (mcolor == *wxWHITE || acolor == *wxWHITE) {
        dc.SetPen(wxColour(0xAC, 0xAC, 0xAC));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(1, 1, MATERIAL_ITEM_SIZE.x - 1, MATERIAL_ITEM_SIZE.y - 1, 5);
    }

    if (m_selected) {
        dc.SetPen(wxColour(0x00, 0xAE, 0x42));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(1, 1, MATERIAL_ITEM_SIZE.x - 1, MATERIAL_ITEM_SIZE.y - 1, 5);
    }
#else
    if (mcolor == *wxWHITE || acolor == *wxWHITE || acolor.Alpha() == 0) {
        dc.SetPen(wxColour(0xAC, 0xAC, 0xAC));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(0, 0, MATERIAL_ITEM_SIZE.x, MATERIAL_ITEM_SIZE.y, 5);
    }

    if (m_selected) {
        dc.SetPen(wxColour(0x00, 0xAE, 0x42));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(0, 0, MATERIAL_ITEM_SIZE.x, MATERIAL_ITEM_SIZE.y, 5);
    }
#endif
    //arrow
    if ( (acolor.Red() > 160 && acolor.Green() > 160 && acolor.Blue() > 160) &&
        (acolor.Red() < 180 && acolor.Green() < 180 && acolor.Blue() < 180)) {
        dc.DrawBitmap(m_arraw_bitmap_white.bmp(), size.x - m_arraw_bitmap_white.GetBmpSize().x - FromDIP(7), size.y - m_arraw_bitmap_white.GetBmpSize().y);
    }
    else {
        dc.DrawBitmap(m_arraw_bitmap_gray.bmp(), size.x - m_arraw_bitmap_gray.GetBmpSize().x - FromDIP(7), size.y - m_arraw_bitmap_gray.GetBmpSize().y);
    }
}

}} // namespace Slic3r::GUI
