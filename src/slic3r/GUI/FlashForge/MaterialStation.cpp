#include "MaterialStation.hpp"
#include <slic3r/GUI/I18N.hpp>
#include <slic3r/GUI/wxExtensions.hpp>

#define msbgWHITE wxColour(248, 248, 248)   //材料站背景颜色

namespace Slic3r {
namespace GUI {

MaterialSlot::MaterialSlot(wxWindow*       parent,
                           wxWindowID      id,
                           const wxColour&       color,
                           const wxBitmap&       bitmap,
                           const wxPoint&  pos,
                           const wxSize&   size,
                           long            style,
                           const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name) 
    , m_color(color), m_bitmap(bitmap)
{
    SetMinSize(wxSize(FromDIP(40), FromDIP(45)));
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &MaterialSlot::paintEvent, this);
}   

MaterialSlot::~MaterialSlot() {}

void MaterialSlot::paintEvent(wxPaintEvent& event) 
{ 
    wxPaintDC dc(this);
    auto      w = GetSize().GetWidth();
    auto      h = GetSize().GetHeight();
    dc.SetBrush(wxBrush(m_color));
    dc.DrawRectangle(0, 0, w, h);
    // 绘制bitmap
    dc.DrawBitmap(m_bitmap, 0, 0);
}


SlotNumber::SlotNumber(wxWindow*       parent,
                       wxWindowID      id,
                       const wxString& number,
                       const wxPoint&  pos,
                       const wxSize&   size,
                       long            style,
                       const wxString& name)
    : wxWindow(parent, id, pos, size, style, name) 
    , m_number(number)
{
    SetMinSize(wxSize(FromDIP(19), FromDIP(19)));
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &SlotNumber::paintEvent, this);
}


SlotNumber::~SlotNumber() {}

void SlotNumber::paintEvent(wxPaintEvent& event)
{
    // 绘制序号椭圆
    wxPaintDC dc(this);
    auto      w = GetSize().GetWidth();
    auto      h = GetSize().GetHeight();
    dc.SetBrush(wxBrush(wxColour(50, 141, 251)));
    dc.DrawEllipse(0, 0, w , h);
    // 绘制序号文本
    //dc.SetPen(wxPen(wxColour(255, 255, 255), FromDIP(1)));
    int    textX = (GetSize().GetWidth() - FromDIP(7)) / 2;
    int    textY = (GetSize().GetHeight() - FromDIP(16)) / 2;
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(m_number, textX, textY);
}

MaterialSlotWgt::MaterialSlotWgt(wxWindow*       parent,
                                 wxWindowID      id,
                                 const wxString& number,
                                 const wxColour& color,
                                 const wxBitmap& bitmap,
                                 const wxPoint&  pos,
                                 const wxSize&   size,
                                 long            style,
                                 const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name)
{ 
    SetMinSize(wxSize(FromDIP(40), FromDIP(73)));
    SetBackgroundColour(wxColour(255, 255, 255));
    setup_layout(this, number, color, bitmap);
}

MaterialSlotWgt::~MaterialSlotWgt() {}

void MaterialSlotWgt::setup_layout(wxWindow* parent, const wxString& number, const wxColour& color, const wxBitmap& bitmap)
{ 
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL); 

    m_number              = new SlotNumber(parent, wxID_ANY, number, wxDefaultPosition, wxSize(FromDIP(19), FromDIP(19))); 

    wxBoxSizer* slot_sizer = new wxBoxSizer(wxHORIZONTAL); 
    m_material_slot        = new MaterialSlot(parent, wxID_ANY, color, bitmap, wxDefaultPosition, wxSize(FromDIP(40), FromDIP(45)));

    wxBoxSizer* group_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow*   widget_group = new wxWindow(m_material_slot, wxID_ANY, wxDefaultPosition);
    widget_group->SetBackgroundColour(color);

    wxFont        font(FromDIP(5), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxStaticText* name_txt = new wxStaticText(widget_group, wxID_ANY, _L("ABS"), wxDefaultPosition, wxSize(FromDIP(16), FromDIP(13)), wxALIGN_CENTER);
    m_edit_btn             = new wxButton(widget_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(16), FromDIP(13)), wxNO_BORDER);
    name_txt->SetFont(font);
    m_edit_btn->SetBitmap(create_scaled_bitmap("edit_btn", nullptr, FromDIP(7)));//7待定
    
    name_txt->SetBackgroundColour(color);
    m_edit_btn->SetBackgroundColour(color);
    group_sizer->Add(name_txt, 0, wxLEFT | wxRIGHT, 0);
    group_sizer->Add(m_edit_btn, 0, wxLEFT | wxRIGHT, 0);
    widget_group->SetSizer(group_sizer);
    widget_group->Layout();
    group_sizer->Fit(widget_group);

    slot_sizer->AddSpacer(FromDIP(18));
    slot_sizer->Add(widget_group, 0, wxTOP | wxBOTTOM, (m_material_slot->GetSize().GetHeight() - widget_group->GetSize().GetHeight()) / 2);
    slot_sizer->AddStretchSpacer();

    m_material_slot->SetSizer(slot_sizer);
    m_material_slot->Layout();

    sizer->Add(m_number, 0, wxLEFT | wxRIGHT, (GetSize().GetWidth() - m_number->GetSize().GetWidth()) / 2);
    sizer->AddSpacer(FromDIP(9));
    sizer->Add(m_material_slot, 0, wxLEFT | wxRIGHT, 0);
    SetSizer(sizer);
    Layout();
    //sizer->Fit(this);

}    



TipsArea::TipsArea(wxWindow*       parent,
                   wxWindowID      id,
                   const wxPoint&  pos,
                   const wxSize&   size,
                   long            style,
                   const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name)
{
    SetBackgroundColour(wxColour(255, 255, 255));
}

TipsArea::~TipsArea() {}


LineArea::LineArea(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &LineArea::paintEvent, this);
}

LineArea::~LineArea() {}

void LineArea::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    // 设置画笔颜色和样式
    wxPen pen(wxColour(102, 102, 102), 2);
    dc.SetPen(pen);
    int     width  = GetSize().GetWidth();
    int     height = GetSize().GetHeight();
    // 绘制直线
    dc.DrawLine(width / 2, 0, width / 2, height - FromDIP(24));
}


ProgressArea::ProgressArea(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style, name)
{
    setup_layout(this);
}

ProgressArea::~ProgressArea() {}


void ProgressArea::setup_layout(wxWindow* parent)
{
    int         width          = GetSize().GetWidth();
    int         height         = GetSize().GetHeight();
    wxBoxSizer* progress_sizer = new wxBoxSizer(wxHORIZONTAL);
    //序号按钮区布局
    wxBoxSizer* num_btn_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow*   num_btn_area  = new LineArea(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(21), height));
    m_btn_group.reserve(4);
    for (int i = 0; i < 4; ++i) {
        ColorButton* col_btn = new ColorButton(num_btn_area, wxID_ANY, wxColour(0, 255, 0), "", wxDefaultPosition, wxSize(FromDIP(21), FromDIP(21)));
        m_btn_group.push_back(col_btn);
        num_btn_sizer->Add(col_btn, 0, wxLEFT | wxRIGHT, 0);
        if (i < 3) {
            num_btn_sizer->AddStretchSpacer();
        }
    }
    num_btn_area->SetSizer(num_btn_sizer);
    num_btn_area->Layout();
    //文本区布局

    wxBoxSizer* txt_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow*   txt_area  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(210), height));
    txt_area->SetBackgroundColour(wxColour(255, 255, 255));
    m_txt_group.reserve(4);
    wxStaticText* txt_1 = new wxStaticText(txt_area, wxID_ANY, _L("txt_1"), wxDefaultPosition, wxSize(FromDIP(210), FromDIP(17)),
                                           wxALIGN_LEFT);
    m_txt_group.push_back(txt_1);
    wxStaticText* txt_2 = new wxStaticText(txt_area, wxID_ANY, _L("txt_2"), wxDefaultPosition, wxSize(FromDIP(210), FromDIP(17)),
                                           wxALIGN_LEFT);
    m_txt_group.push_back(txt_2);
    wxStaticText* txt_3 = new wxStaticText(txt_area, wxID_ANY, _L("txt_3"), wxDefaultPosition, wxSize(FromDIP(210), FromDIP(17)),
                                           wxALIGN_LEFT);
    m_txt_group.push_back(txt_3);
    wxStaticText* txt_4 = new wxStaticText(txt_area, wxID_ANY, _L("txt_4"), wxDefaultPosition, wxSize(FromDIP(210), FromDIP(17)),
                                           wxALIGN_LEFT);
    m_txt_group.push_back(txt_4);
    for (int i = 0; i < 4; ++i) {
        txt_sizer->Add(m_txt_group[i], 0, wxLEFT , FromDIP(9));
        if (i < 3) {
            txt_sizer->AddStretchSpacer();
        }
    }
    txt_area->SetSizer(txt_sizer);
    txt_area->Layout();

    //取消按钮区
    wxBoxSizer* cancel_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow*   cancel_area  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(38), height));
    cancel_area->SetBackgroundColour(wxColour(255, 255, 255));
    LabelRoundedButton* cancel_btn = new LabelRoundedButton(cancel_area, wxID_ANY, false, wxColour(50, 141, 251), _L("Cancel"),
                                                            wxDefaultPosition,
                                                            wxSize(FromDIP(38), FromDIP(22)));
    cancel_btn->SetForegroundColour(wxColour(50, 141, 251));

    cancel_sizer->AddStretchSpacer();
    cancel_sizer->Add(cancel_btn, 0, wxLEFT | wxRIGHT, 0);
    cancel_area->SetSizer(cancel_sizer);
    cancel_area->Layout();

    //整体布局
    progress_sizer->Add(num_btn_area, 0, wxLEFT | wxRIGHT, 0);
    progress_sizer->Add(txt_area, 0, wxLEFT | wxRIGHT, 0);
    progress_sizer->Add(cancel_area, 0, wxLEFT | wxRIGHT, 0);
    progress_sizer->AddStretchSpacer();
    SetSizer(progress_sizer);
    Layout();
}

    
ColorButton::ColorButton(wxWindow*          parent,
                         wxWindowID         id,
                         wxColour&          color,
                         const wxString&    label,
                         const wxPoint&     pos,
                         const wxSize&      size,
                         long               style,
                         const wxValidator& validator,
                         const wxString&    name) 
    : wxButton(parent, id, label, pos, size, style, validator, name)
    , m_selected_color(color)
{ 
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &ColorButton::paintEvent, this); 
}

ColorButton::~ColorButton() {}


void ColorButton::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetBrush(wxBrush(m_selected_color));
    int x      = 0;
    int y      = 0;
    int width  = GetSize().GetWidth();
    int height = GetSize().GetWidth();
    dc.DrawEllipse(x, y, width, height);
}

RoundedButton::RoundedButton(wxWindow*          parent,
                             wxWindowID         id,
                             const wxString&    label,
                             const wxPoint&     pos,
                             const wxSize&      size,
                             long               style,
                             const wxValidator& validator,
                             const wxString&    name) 
    : wxButton(parent, id, label, pos, size, style, validator, name), m_state(ButtonState::Normal)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    connectEvent();
}

RoundedButton::~RoundedButton()
{}

void RoundedButton::set_bitmap(const wxBitmap& bitmap) 
{ 
    m_bitmap = bitmap;
}

void RoundedButton::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    // 根据状态绘制不同的背景颜色
    switch (m_state) {
    case ButtonState::Normal: {
        dc.SetBrush(wxBrush(wxColour(50, 141, 251)));
        break;
    }
    case ButtonState::Hovered: {
        dc.SetBrush(wxBrush(wxColour(149, 197, 255)));
        break;
    }
    case ButtonState::Pressed: {
        dc.SetBrush(wxBrush(wxColour(17, 111, 223)));
        break;
    }
    default: break;
    }
    dc.DrawRoundedRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight(), 4);
    // 绘制图标
    int iconX = (GetSize().GetWidth() - m_bitmap.GetWidth()) / 2;
    int iconY = (GetSize().GetHeight() - m_bitmap.GetHeight()) / 2;
    dc.DrawBitmap(m_bitmap, iconX, iconY);

}

void RoundedButton::OnMouseDown(wxMouseEvent& event){
    m_state = ButtonState::Pressed;
    Refresh();
    wxCommandEvent clickEvent(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
    ProcessWindowEvent(clickEvent);
}

void RoundedButton::OnMouseUp(wxMouseEvent& event) {
    m_state = ButtonState::Hovered;
    Refresh();
}

void RoundedButton::OnMouseEnter(wxMouseEvent& event){
    m_state = ButtonState::Hovered;
    Refresh();
}

void RoundedButton::OnMouseLeave(wxMouseEvent& event){
    m_state = ButtonState::Normal;
    Refresh();
}

void RoundedButton::connectEvent()
{
    Bind(wxEVT_PAINT, &RoundedButton::paintEvent, this);
    Bind(wxEVT_LEFT_DOWN, &RoundedButton::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &RoundedButton::OnMouseUp, this);
    Bind(wxEVT_ENTER_WINDOW, &RoundedButton::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &RoundedButton::OnMouseLeave, this);
}

LabelRoundedButton::LabelRoundedButton(wxWindow*          parent,
                                         wxWindowID         id,
                                         bool               isFill,
                                         const wxColour&    color,
                                         const wxString&    label,
                                         const wxPoint&     pos,
                                         const wxSize&      size,
                                         long               style,
                                         const wxValidator& validator,
                                         const wxString&    name)
    : wxButton(parent, id, label, pos, size, style, validator, name), m_isFill(isFill), m_color(color)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &LabelRoundedButton::paintEvent, this);
}

LabelRoundedButton::~LabelRoundedButton() {}

void LabelRoundedButton::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    if (m_isFill) {
        dc.SetBrush(wxBrush(m_color));
    } else {
        dc.SetPen(wxPen(m_color, FromDIP(1)));
    }
    dc.DrawRoundedRectangle(wxPoint(0, 0), GetSize(), 4);
    // 获取按钮的大小
    wxSize size = GetSize();
    // 计算文本的位置，使其在按钮中央
    int textX = (size.x - dc.GetTextExtent(GetLabel()).x) / 2;
    int textY = (size.y - dc.GetTextExtent(GetLabel()).y) / 2;
    dc.DrawText(GetLabel(), textX, textY);
}


IdentifyButton::IdentifyButton(wxWindow*          parent,
                               wxWindowID         id,
                               const wxString&    label,
                               const wxPoint&     pos,
                               const wxSize&      size,
                               long               style,
                               const wxValidator& validator,
                               const wxString&    name)
    : wxButton(parent, id, label, pos, size, style, validator, name), m_isSelected(false)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &IdentifyButton::paintEvent, this);
}

IdentifyButton::~IdentifyButton() {}

void IdentifyButton::set_bitmap(const wxBitmap& select, const wxBitmap& unselect) 
{ 
    m_select_bitmap = select; 
    m_unselect_bitmap = unselect;
}

void IdentifyButton::set_select_state(bool isSelected) 
{ 
    m_isSelected = isSelected; 
    Refresh();
}

void IdentifyButton::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxBitmap* bitmap = (m_isSelected) ? &m_select_bitmap : &m_unselect_bitmap;
    // 绘制图标
    int iconX = (GetSize().GetWidth() - bitmap->GetWidth()) / 2;
    int iconY = (GetSize().GetHeight() - bitmap->GetHeight()) / 2;
    dc.DrawBitmap(*bitmap, iconX, iconY);
    // 根据状态绘制下方横线
    if (m_isSelected) {
        dc.SetBrush(wxBrush(wxColour(50, 141, 251)));
        dc.DrawRectangle(0, GetSize().GetHeight() - FromDIP(2), GetSize().GetWidth(), FromDIP(2));
    }
}


Palette::Palette(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name) 
    : wxDialog(parent, id, title, pos, size, style, name)
{
    SetMinSize(wxSize(360, 580));
    SetWindowStyle(wxDEFAULT_DIALOG_STYLE & ~(wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU));
    setup_layout(this); 
    connectEvent();
}

Palette::~Palette() {}

void Palette::set_material_station_color_vector(std::vector<wxColour> color_vec) 
{ 
    
}

void Palette::resizeEvent(wxSizeEvent& event)
{
    wxDisplay display;
    wxRect    screenRect = display.GetGeometry();
    wxSize    size       = GetSize();
    int       x          = (screenRect.GetWidth() - GetSize().GetWidth()) / 2;
    int       y          = (screenRect.GetHeight() - GetSize().GetHeight()) / 2;
    SetPosition(wxPoint(x, y));
}

void Palette::setup_layout(wxWindow* parent) 
{ 
    wxBoxSizer* palette_sizer = new wxBoxSizer(wxVERTICAL);
    //材料站颜色标题布局
    wxBoxSizer* sizer_station_title = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_station_title  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(360, 40));
    m_station_color_lab = new wxStaticText(area_station_title, wxID_ANY, _L("Material Station"), wxDefaultPosition, wxSize(80, 30), wxALIGN_LEFT);
    sizer_station_title->Add(m_station_color_lab, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);
    sizer_station_title->AddStretchSpacer();
    area_station_title->SetSizer(sizer_station_title);
    area_station_title->Layout();
    // 材料站颜色按钮布局
    wxBoxSizer* sizer_station_color = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_station_color   = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(360, 60));
    for (int i = 0; i < 4; ++i) {
        ColorButton* color_btn = new ColorButton(area_station_color, wxID_ANY, wxColour(0, 255, 0), "", wxDefaultPosition, wxSize(50, 50));
        m_station_color_btns.push_back(color_btn);
        sizer_station_color->AddStretchSpacer();
        sizer_station_color->Add(color_btn, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);
    }
    sizer_station_color->AddStretchSpacer();
    area_station_color->SetSizer(sizer_station_color);
    area_station_color->Layout();
    // 颜色库标题布局
    wxBoxSizer* sizer_lib_title = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_lib_title  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(360, 40));
    m_color_lib_lab             = new wxStaticText(area_lib_title, wxID_ANY, _L("Color Library"), wxDefaultPosition, wxSize(80, 30), wxALIGN_LEFT);
    sizer_lib_title->Add(m_color_lib_lab, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);
    sizer_lib_title->AddStretchSpacer();
    area_lib_title->SetSizer(sizer_lib_title);
    area_lib_title->Layout();
    //颜色库按钮布局
    wxGridSizer* gridSizer      = new wxGridSizer(6, 4, 30, 30);// 6 行 4 列，垂直水平间距均为 20，15
    wxWindow*   area_lib_color  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(360, 480));
    for (int i = 0; i < 24; ++i) {
        ColorButton* color_btn = new ColorButton(area_lib_color, wxID_ANY, wxColour(0, 255, 0), "", wxDefaultPosition, wxSize(50, 50));
        color_btn->SetBackgroundColour(wxColour(255, 0, 0));
        m_color_lib_btns.push_back(color_btn);
        gridSizer->Add(color_btn, 0, wxALIGN_CENTRE | wxALL, 0);
    }
    area_lib_color->SetSizer(gridSizer);
    area_lib_color->Layout();
    //整体布局
    palette_sizer->Add(area_station_title, 0, wxLEFT | wxRIGHT, 0);
    palette_sizer->Add(area_station_color, 0, wxLEFT | wxRIGHT, 0);
    palette_sizer->Add(area_lib_title, 0, wxLEFT | wxRIGHT, 0);
    palette_sizer->Add(area_lib_color, 0, wxLEFT | wxRIGHT, 30);
    SetSizer(palette_sizer);
    Layout();

}

void Palette::connectEvent() 
{ 
    Bind(wxEVT_SIZE, &Palette::resizeEvent, this); 
}


MaterialDialog::MaterialDialog(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size,long style, const wxString& name )
    : wxDialog(parent, id, title, pos, size, style, name)
{
    SetBackgroundColour(*wxWHITE);
    SetWindowStyle(wxDEFAULT_DIALOG_STYLE & ~(wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU));
    setup_layout(this);
    connectEvent();
}
MaterialDialog::~MaterialDialog() {}

void MaterialDialog::on_resize(wxSizeEvent& event) 
{
    wxDisplay display;
    wxRect    screenRect = display.GetGeometry();
    wxSize    size       = GetSize();
    int x = (screenRect.GetWidth() - GetSize().GetWidth()) / 2;
    int y = (screenRect.GetHeight() - GetSize().GetHeight()) / 2;
    SetPosition(wxPoint(x, y));
}

void MaterialDialog::setup_layout(wxWindow* parent) 
{ 
    wxBoxSizer* dialog_sizer = new wxBoxSizer(wxVERTICAL);
    //上半部分材料和颜色选择区
    wxBoxSizer* select_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow*   select_area  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(356), FromDIP(129)));
    select_area->SetBackgroundColour(wxColour(255, 255, 255));

    m_type_lab = new wxStaticText(select_area, wxID_ANY, _L("Type of material"), wxDefaultPosition, wxSize(FromDIP(347), FromDIP(19)),
                                  wxALIGN_LEFT);

    m_comboBox = new wxComboBox(select_area, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(347), FromDIP(34)), 0, NULL, wxCB_READONLY);

    m_color_lab = new wxStaticText(select_area, wxID_ANY, _L("Color"), wxDefaultPosition, wxSize(FromDIP(347), FromDIP(19)), wxALIGN_LEFT);

    wxBoxSizer* color_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   color_area  = new wxWindow(select_area, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(347), FromDIP(26)));
    color_area->SetBackgroundColour(wxColour(255, 255, 255));
    m_color_btn = new ColorButton(color_area, wxID_ANY, wxColour(245, 154, 35), _L("?"), wxDefaultPosition,
                                  wxSize(FromDIP(26), FromDIP(26)), wxNO_BORDER);
    color_sizer->Add(m_color_btn, 0, wxTOP | wxBOTTOM, 0);
    color_sizer->AddStretchSpacer();
    color_area->SetSizer(color_sizer);
    color_area->Layout();

    select_sizer->Add(m_type_lab, 0, wxEXPAND |wxRIGHT, FromDIP(9));
    select_sizer->AddSpacer(FromDIP(7));
    select_sizer->Add(m_comboBox, 0, wxEXPAND | wxRIGHT, FromDIP(9));
    select_sizer->AddStretchSpacer();
    select_sizer->Add(m_color_lab, 0, wxEXPAND | wxRIGHT, FromDIP(9));
    select_sizer->AddSpacer(FromDIP(7));
    select_sizer->Add(color_area, 0, wxEXPAND | wxRIGHT, FromDIP(9));
    select_area->SetSizer(select_sizer);
    select_area->Layout();

    // 下半部分按钮区
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   button_area  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(GetSize().GetWidth(), FromDIP(36)));
    button_area->SetBackgroundColour(wxColour(255, 255, 255));
    m_cancel = new LabelRoundedButton(button_area, wxID_CANCEL, false, wxColour(65, 148, 136), _L("Cancel"), wxDefaultPosition,
                                      wxSize(FromDIP(87), FromDIP(36)));
    m_cancel->SetForegroundColour(wxColour(65, 148, 136));
    m_OK     = new LabelRoundedButton(button_area, wxID_OK, true, wxColour(65, 148, 136), _L("OK"), wxDefaultPosition,
                                      wxSize(FromDIP(87), FromDIP(36)));
    m_OK->SetForegroundColour(wxColour(255, 255, 255));
    button_sizer->AddSpacer(FromDIP(107));
    button_sizer->Add(m_cancel, 0, wxTOP | wxBOTTOM, 0);
    button_sizer->AddSpacer(FromDIP(52));
    button_sizer->Add(m_OK, 0, wxTOP | wxBOTTOM, 0);
    button_sizer->AddStretchSpacer();
    button_area->SetSizer(button_sizer);
    button_area->Layout();
    //对话框整体布局
    dialog_sizer->AddSpacer(FromDIP(17));
    dialog_sizer->Add(select_area, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(32));
    dialog_sizer->AddStretchSpacer();
    dialog_sizer->Add(button_area, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(32));
    dialog_sizer->AddSpacer(FromDIP(18));
    SetSizer(dialog_sizer);
    Layout();
}

void MaterialDialog::connectEvent()
{
    Bind(wxEVT_SIZE, &MaterialDialog::on_resize, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialDialog::on_color_btn_clicked, this, m_color_btn->GetId());
}

void MaterialDialog::on_color_btn_clicked(wxCommandEvent& event) 
{ 
    Palette* palette = new Palette(nullptr, wxID_ANY, "", wxDefaultPosition, wxSize(360, 580));
    palette->Show();
}



MaterialPanel::MaterialPanel(wxWindow*       parent,
                             wxWindowID      winid,
                             const wxPoint&  pos,
                             const wxSize&   size,
                             long            style,
                             const wxString& name)
    : wxPanel(parent, winid, pos, size, style, name), m_material_dialog(nullptr), m_tips_area_state(TipsAreaState::TAS_TIPS)
{
    SetBackgroundColour(wxColour(248, 248, 248));
    setup_layout(this);
    connectEvent();
}

MaterialPanel::~MaterialPanel() {}

void MaterialPanel::setup_layout(wxWindow* parent) 
{
    int width = GetSize().GetWidth();
    int height = GetSize().GetHeight();
    //MaterialPanel整体水平布局
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    //MaterialPanel左半部分操作区
    wxBoxSizer* operate_area_sizer = new wxBoxSizer(wxVERTICAL);
    m_operate_area                 = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(407), height));
    m_operate_area->SetBackgroundColour(wxColour(255, 255, 255));

    //左半部分操作区的上边的切换按钮区
    wxBoxSizer* switch_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   switch_group = new wxWindow(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(m_operate_area->GetSize().GetWidth(), FromDIP(34)));
    switch_group->SetBackgroundColour(wxColour(255, 255, 255));

    m_recognized_btn = new IdentifyButton(switch_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(51), FromDIP(34)));
    m_recognized_btn->set_bitmap(create_scaled_bitmap("four_color_select", nullptr, 24), create_scaled_bitmap("four_color_unselect", nullptr, 24));
    m_recognized_btn->set_select_state(true);
    m_unrecognized_btn = new IdentifyButton(switch_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(51), FromDIP(34)));
    m_unrecognized_btn->set_bitmap(create_scaled_bitmap("plug_slot_switch_btn_select", nullptr, 24), create_scaled_bitmap("plug_slot_switch_btn_unselect", nullptr, 24));
    m_unrecognized_btn->set_select_state(false);
    switch_sizer->Add(m_recognized_btn, 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
    switch_sizer->AddSpacer(FromDIP(32));
    switch_sizer->Add(m_unrecognized_btn, 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
    switch_sizer->AddStretchSpacer();
    switch_group->SetSizer(switch_sizer);
    switch_group->Layout();

    // 左半部分操作区的中间的料槽区
    wxBoxSizer* slot_group_sizer   = new wxBoxSizer(wxHORIZONTAL);
    m_material_slot_group = new wxWindow(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(m_operate_area->GetSize().GetWidth(), FromDIP(130)));
    m_material_slot_group->SetBackgroundColour(wxColour(0, 255, 0));

    wxSize slot_size(150, 200);
    for (int i = 0; i < 4; ++i) {
        wxString         number(wxString::Format(wxT("%i"), i + 1));
        wxColour         color(0, 255, 0);
        wxBitmap         bitmap(create_scaled_bitmap("transparent_slot", nullptr, FromDIP(30)));//这里FromDIP(30)是试出来的
        MaterialSlotWgt* material_slot = new MaterialSlotWgt(m_material_slot_group, wxID_ANY, number, color, bitmap,wxDefaultPosition,
                                                             wxSize(FromDIP(40), FromDIP(73)));
        slot_group_sizer->Add(material_slot, 0, wxEXPAND | wxTOP | wxBOTTOM,
                              (m_material_slot_group->GetSize().GetHeight() - material_slot->GetSize().GetHeight()) / 2);
        if (i < 3) {
            slot_group_sizer->AddSpacer(FromDIP(51));
        } else {
            slot_group_sizer->AddStretchSpacer();
        }
        m_material_slots.push_back(material_slot);
    }

    m_material_slot_group->SetSizer(slot_group_sizer);
    m_material_slot_group->Layout();

    // 左半部分操作区的下边的按钮区
    wxBoxSizer* btn_group_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_button_group = new wxWindow(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(m_operate_area->GetSize().GetWidth(), FromDIP(62)));
    m_button_group->SetBackgroundColour(wxColour(255, 255, 255));

    m_supply_wire = new RoundedButton(m_button_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(77), FromDIP(34)));
    m_supply_wire->set_bitmap(create_scaled_bitmap("supply_wire", nullptr, 31));
    m_withdrawn_wire = new RoundedButton(m_button_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(77), FromDIP(34)));
    m_withdrawn_wire->set_bitmap(create_scaled_bitmap("withdrawn_wire", nullptr, 29));

    btn_group_sizer->AddSpacer(FromDIP(122));
    btn_group_sizer->Add(m_supply_wire, 0, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(14));
    btn_group_sizer->AddSpacer(FromDIP(27));
    btn_group_sizer->Add(m_withdrawn_wire, 0, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(14));
    btn_group_sizer->AddStretchSpacer();
    m_button_group->SetSizer(btn_group_sizer);
    m_button_group->Layout();
    // MaterialPanel左半部分操作区整体布局
    operate_area_sizer->Add(switch_group, 0, wxEXPAND | wxALL, 0);
    operate_area_sizer->Add(m_material_slot_group, 0, wxEXPAND | wxALL, 0);
    operate_area_sizer->Add(m_button_group, 0, wxEXPAND | wxALL, 0);
    m_operate_area->SetSizer(operate_area_sizer);
    m_operate_area->Layout();
    operate_area_sizer->Fit(m_operate_area);


    // MaterialPanel右半部分提示区
    wxBoxSizer* tips_area_sizer = new wxBoxSizer(wxVERTICAL);
    m_tips_area                 = new TipsArea(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(322), height));
    m_tips_area->SetSizer(tips_area_sizer);

    m_tips_area_title = new wxStaticText(m_tips_area, wxID_ANY, _L("Tips"), wxDefaultPosition, wxSize(FromDIP(269), FromDIP(17)), wxALIGN_LEFT);
    m_tips_area_title->SetForegroundColour(wxColour(50, 141, 251));
    const wxString tips_text("Clickable slots, single feeding/unwinding for loading/unloading of yarns.");
    m_tips_text = new wxStaticText(m_tips_area, wxID_ANY, _L(tips_text), wxDefaultPosition, wxSize(FromDIP(269), FromDIP(149)), wxALIGN_LEFT);
    m_progress  = new ProgressArea(m_tips_area, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(269), FromDIP(149)));

    //layout_tips_info(m_tips_area);
    layout_progress_status(m_tips_area);
    
    //整体布局
    panel_sizer->Add(m_operate_area, 0, wxEXPAND | wxALL, 0);
    panel_sizer->AddSpacer(FromDIP(1));
    panel_sizer->Add(m_tips_area, 0, wxEXPAND | wxALL, 0);
    SetSizer(panel_sizer);
    Layout();
    panel_sizer->Fit(this);
}

void MaterialPanel::layout_tips_info(TipsArea* parent) 
{ 
    wxSizer* tips_area_sizer = parent->GetSizer();
    assert(tips_area_sizer);
    if (!tips_area_sizer->IsEmpty()) {
        tips_area_sizer->Remove(m_tips_area_title->GetId());
        tips_area_sizer->Remove(m_progress->GetId());
        assert(tips_area_sizer->IsEmpty());
    }
    m_progress->Hide();
    tips_area_sizer->AddSpacer(FromDIP(45));
    tips_area_sizer->Add(m_tips_area_title, 0, wxLEFT | wxRIGHT, FromDIP(27));
    tips_area_sizer->Add(m_tips_text, 0, wxLEFT | wxRIGHT, FromDIP(27));
    tips_area_sizer->AddStretchSpacer();
    parent->Layout();
}

void MaterialPanel::layout_progress_status(TipsArea* parent)
{
    wxSizer* tips_area_sizer = parent->GetSizer();
    assert(tips_area_sizer);
    if (!tips_area_sizer->IsEmpty()) {
        tips_area_sizer->Remove(m_tips_area_title->GetId());
        tips_area_sizer->Remove(m_tips_text->GetId());
        assert(tips_area_sizer->IsEmpty());//确保父窗口布局里的东西都被移走
    }
    m_tips_text->Hide();
    tips_area_sizer->AddSpacer(FromDIP(45));
    tips_area_sizer->Add(m_tips_area_title, 0, wxLEFT | wxRIGHT, FromDIP(27));
    tips_area_sizer->Add(m_progress, 0, wxLEFT | wxRIGHT, FromDIP(27));
    tips_area_sizer->AddStretchSpacer();
    parent->Layout();
}

void MaterialPanel::connectEvent() 
{ 
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_supply_wire_clicked, this, m_supply_wire->GetId()); 
}

void MaterialPanel::on_supply_wire_clicked(wxCommandEvent& event) 
{ 
    if (m_material_dialog)        return;
    m_material_dialog = new MaterialDialog(nullptr, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(422), FromDIP(224)));
    m_material_dialog->ShowModal();
}


MaterialStation::MaterialStation(wxWindow*       parent,
                                 wxWindowID      winid,
                                 const wxPoint&  pos,
                                 const wxSize&   size,
                                 long            style,
                                 const wxString& name)
    : wxPanel(parent, winid, pos, size, style, name)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    create_panel(this);
}

MaterialStation::~MaterialStation() {} 

void MaterialStation::create_panel(wxWindow* parent)
{
    int         width  = GetSize().GetWidth();
    int         height = GetSize().GetHeight();
    wxBoxSizer* sizer                 = new wxBoxSizer(wxVERTICAL);
    // 材料站标题布局
    wxBoxSizer* bSizer_material_title = new wxBoxSizer(wxHORIZONTAL);
    m_material_title                  = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(44)));
    m_material_title->SetBackgroundColour(wxColour(248, 248, 248));
    m_staticText_title = new wxStaticText(m_material_title, wxID_ANY, _L("FFM"));
    m_staticText_title->SetForegroundColour(wxColour(51, 51, 51));
    bSizer_material_title->Add(m_staticText_title, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(14));
    bSizer_material_title->Add(0, 0, 1, wxEXPAND, 0);
    m_material_title->SetSizer(bSizer_material_title);
    m_material_title->Layout();

    //标题和内容中间的间隔
    wxWindow* separator_middle = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(4)));
    separator_middle->SetBackgroundColour(wxColour(240, 240, 240));
    // 材料站内容布局
    MaterialPanel* m_material_panel      = new MaterialPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(226)));
    //整体布局
    sizer->Add(m_material_title, 0, wxEXPAND | wxALL, 0);
    sizer->Add(separator_middle, 0, wxEXPAND | wxALL, 0);
    sizer->Add(m_material_panel, 0, wxEXPAND | wxALL, 0);
    SetSizer(sizer);
    Layout();
    Fit();
}

wxPanel* MaterialStation::GetPrintTitlePanel() { return m_material_title; }



} // namespace GUI

} // namespace Slic3r