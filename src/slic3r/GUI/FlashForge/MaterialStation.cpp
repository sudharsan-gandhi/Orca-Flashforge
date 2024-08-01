#include "MaterialStation.hpp"
#include <slic3r/GUI/I18N.hpp>

#define msbgWHITE wxColour(248, 248, 248)   //材料站背景颜色

namespace Slic3r {
namespace GUI {

MaterialSlot::MaterialSlot(wxWindow*       parent,
                           wxWindowID      id,
                           wxColour&       wheel_clr,
                           wxColour&       bucket_clr,
                           const wxPoint&  pos,
                           const wxSize&   size,
                           long            style,
                           const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name) 
    , m_wheel_clr(wheel_clr), m_bucket_clr(bucket_clr)
{
    SetMinSize(wxSize(150, 150));
    Bind(wxEVT_PAINT, &MaterialSlot::paintEvent, this);
}   

MaterialSlot::~MaterialSlot() {}

void MaterialSlot::paintEvent(wxPaintEvent& event) 
{ 
    wxPaintDC dc(this);
    auto      w = GetSize().GetWidth();
    auto      h = GetSize().GetHeight();
    dc.SetBrush(wxBrush(m_wheel_clr));
    dc.DrawEllipse(w * 3.0 / 4, 0, 1.0 * w / 4, h);

    dc.SetBrush(wxBrush(m_bucket_clr));
    dc.SetPen(wxPen(m_bucket_clr));
    dc.DrawRectangle(w / 8, h / 6.0, 6.0 * w / 8, 2.0 * h / 3);
    dc.DrawEllipticArc(7.0 * w / 8 - 20, h / 6.0, 40, 2.0 * h / 3, -90, 90);

    dc.SetBrush(wxColour(248, 248, 248));
    dc.DrawEllipse(0, 0, w / 4.0, h);

    dc.SetBrush(wxBrush(m_wheel_clr));
    dc.DrawEllipse(0, 0, w / 4.0 - 4, h);

    dc.SetBrush(wxColour(248, 248, 248));
    dc.DrawEllipse(w / 8.0 - 8, h / 2.0 - 10, 16, 20);

}


SlotNumber::SlotNumber(wxWindow*       parent,
                       wxWindowID      id,
                       wxColour&       number_clr,
                       const wxPoint&  pos,
                       const wxSize&   size,
                       long            style,
                       const wxString& name)
    : wxWindow(parent, id, pos, size, style, name) 
    , m_number_clr(number_clr)
{
    SetMinSize(wxSize(50, 30));
    Bind(wxEVT_PAINT, &SlotNumber::paintEvent, this);
}


SlotNumber::~SlotNumber() {}

void SlotNumber::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    auto      w = /*GetSize().GetWidth()*/50;
    auto      h = /*GetSize().GetHeight()*/30;
    dc.SetBrush(wxBrush(m_number_clr));
    dc.DrawEllipse(0, 0, w , h);
}

MaterialSlotWgt::MaterialSlotWgt(wxWindow*       parent,
                                 wxWindowID      id,
                                 wxString&       number,
                                 wxColour&       colour,
                                 const wxPoint&  pos,
                                 const wxSize&   size,
                                 long            style,
                                 const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name)
{ 
    setup_layout(this, number, colour);
}

MaterialSlotWgt::~MaterialSlotWgt() {}

void MaterialSlotWgt::setup_layout(wxWindow* parent, wxString& number, wxColour& colour)
{ 
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL); 

    wxBoxSizer* num_sizer = new wxBoxSizer(wxHORIZONTAL); 
    m_number              = new SlotNumber(parent, wxID_ANY, wxColour(0, 128, 0), wxDefaultPosition, wxSize(50, 30));
    m_number->SetBackgroundColour(msbgWHITE);
    wxStaticText* num_txt = new wxStaticText(m_number, wxID_ANY, _L(number), wxDefaultPosition, wxSize(20, 20), wxALIGN_CENTER);
    num_txt->SetBackgroundColour(wxColour(0, 128, 0));
    num_sizer->AddStretchSpacer();
    num_sizer->Add(num_txt, 0, wxTOP | wxBOTTOM, 5);
    num_sizer->AddStretchSpacer();
    m_number->SetSizer(num_sizer);
    m_number->Layout();

    wxBoxSizer* slot_sizer = new wxBoxSizer(wxHORIZONTAL); 
    m_material_slot        = new MaterialSlot(parent, wxID_ANY, colour, wxColour(128, 0, 0), wxDefaultPosition, wxSize(150, 150));
    m_material_slot->SetBackgroundColour(msbgWHITE);
    wxBoxSizer* group_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow*     widget_group = new wxWindow(m_material_slot, wxID_ANY, wxDefaultPosition, wxSize(50, 70));
    widget_group->SetBackgroundColour(wxColour(128, 0, 0));
    wxStaticText* name_txt = new wxStaticText(widget_group, wxID_ANY, _L("ABS"), wxDefaultPosition, wxSize(50, 30), wxALIGN_CENTER);
    m_edit_btn             = new wxButton(widget_group, wxID_ANY, _L("edit"), wxDefaultPosition, wxSize(50, 30), wxNO_BORDER);
    name_txt->SetBackgroundColour(wxColour(128, 0, 0));
    m_edit_btn->SetBackgroundColour(wxColour(128, 0, 0));
    group_sizer->AddStretchSpacer();
    group_sizer->Add(name_txt, 0, wxLEFT | wxRIGHT, 0);
    group_sizer->AddSpacer(10);
    group_sizer->Add(m_edit_btn, 0, wxLEFT | wxRIGHT, 0);
    group_sizer->AddStretchSpacer();
    widget_group->SetSizer(group_sizer);
    widget_group->Layout();

    slot_sizer->AddStretchSpacer();
    slot_sizer->Add(widget_group, 0, wxTOP | wxBOTTOM, (150 - 70)/2 );
    slot_sizer->AddStretchSpacer();

    m_material_slot->SetSizer(slot_sizer);
    m_material_slot->Layout();

    sizer->Add(m_number, 0, wxLEFT | wxRIGHT, (150 - 50)/2);
    sizer->Add(m_material_slot, 0, wxLEFT | wxRIGHT, 0);
    SetSizer(sizer);
    Layout();

}    



TipsArea::TipsArea(wxWindow*       parent,
                   wxWindowID      id,
                   const wxPoint&  pos,
                   const wxSize&   size,
                   long            style,
                   const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name)
{
    Bind(wxEVT_PAINT, &TipsArea::paintEvent, this);
}

TipsArea::~TipsArea() {}

void TipsArea::paintEvent(wxPaintEvent& event) 
{
    wxPaintDC dc(this);
    // 设置画笔颜色和样式
    wxPen pen(wxColour(255, 0, 0), 2);
    dc.SetPen(pen);
    // 计算圆角矩形的参数
    int x      = 20;
    int y      = 20;
    int width  = GetSize().GetWidth() - 2 * x;
    int height = GetSize().GetHeight() - 2 * y;
    int radius = 10;
    // 绘制圆角矩形
    dc.DrawRoundedRectangle(x, y, width, height, radius);
}

void TipsArea::setup_layout() {}
    
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
    Bind(wxEVT_PAINT, &ColorButton::paintEvent, this); 
}

ColorButton::~ColorButton() {}


void ColorButton::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetBrush(wxBrush(m_selected_color));
    int x      = 0;
    int y      = 0;
    int width  = 50;
    int height = 50;
    dc.DrawEllipse(x, y, width, height);
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
    wxWindow*   select_area = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(700, 150));
    //select_area->SetBackgroundColour(wxColour(0, 255, 0));

    wxBoxSizer* type_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   type_area  = new wxWindow(select_area, wxID_ANY, wxDefaultPosition, wxSize(700, 30));
    m_type_lab = new wxStaticText(type_area, wxID_ANY, _L("Type of material"), wxDefaultPosition, wxSize(80, 30), wxALIGN_LEFT);
    m_comboBox = new wxComboBox(type_area, wxID_ANY, "", wxDefaultPosition, wxSize(350, 30), 0, NULL, wxCB_READONLY);
    type_sizer->Add(m_type_lab, 0, wxTOP | wxBOTTOM, 0);
    type_sizer->AddSpacer(20);
    type_sizer->Add(m_comboBox, 0, wxTOP | wxBOTTOM, 0);
    type_sizer->AddStretchSpacer();
    type_area->SetSizer(type_sizer);
    type_area->Layout();

    wxBoxSizer* color_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   color_area  = new wxWindow(select_area, wxID_ANY, wxDefaultPosition, wxSize(700, 50));
    m_color_lab             = new wxStaticText(color_area, wxID_ANY, _L("Color"), wxDefaultPosition, wxSize(40, 30), wxALIGN_LEFT);
    m_color_btn             = new ColorButton(color_area, wxID_ANY, wxColour(0, 55, 0), _L("?"), wxDefaultPosition, wxSize(50, 50));
    color_sizer->Add(m_color_lab, 0, wxTOP | wxBOTTOM, 10);
    color_sizer->AddSpacer(20);
    color_sizer->Add(m_color_btn, 0, wxTOP | wxBOTTOM, 0);
    color_sizer->AddStretchSpacer();
    color_area->SetSizer(color_sizer);
    color_area->Layout();

    select_sizer->AddStretchSpacer();
    select_sizer->Add(type_area, 0, wxLEFT | wxRIGHT, 0);
    select_sizer->AddSpacer(20);
    select_sizer->Add(color_area, 0, wxLEFT | wxRIGHT, 0);
    select_sizer->AddStretchSpacer();
    select_area->SetSizer(select_sizer);
    select_area->Layout();
    // 下半部分按钮区
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   button_area  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(700, 50));
    //button_area->SetBackgroundColour(wxColour(255, 0, 0));
    m_OK                     = new wxButton(button_area, wxID_OK, _L("OK"), wxDefaultPosition, wxSize(100, 50));
    m_cancel                 = new wxButton(button_area, wxID_CANCEL, _L("Cancel"), wxDefaultPosition, wxSize(100, 50));
    button_sizer->AddStretchSpacer();
    button_sizer->Add(m_OK, 0, wxTOP | wxBOTTOM, 0);
    button_sizer->AddSpacer(10);
    button_sizer->Add(m_cancel, 0, wxTOP | wxBOTTOM, 0);
    button_area->SetSizer(button_sizer);
    button_area->Layout();
    //对话框整体布局
    dialog_sizer->AddSpacer(15);
    dialog_sizer->Add(select_area, 0, wxEXPAND | wxLEFT | wxRIGHT, 0);
    dialog_sizer->AddStretchSpacer();
    dialog_sizer->Add(button_area, 0, wxEXPAND | wxLEFT | wxRIGHT, 0);
    dialog_sizer->AddSpacer(15);
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
                             const wxString& name) /*wxSize(-1, FromDIP(208))*/
    : wxPanel(parent, winid, pos,  size, style, name), m_material_dialog(nullptr)
{
    setup_layout(this);
    connectEvent();
}

MaterialPanel::~MaterialPanel() {}

void MaterialPanel::setup_layout(wxWindow* parent) 
{
    //MaterialPanel整体垂直布局
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxVERTICAL);
    //MaterialPanel上半部分操作区（水平）
    wxBoxSizer* operate_area_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_operate_area                 = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 300));
    m_operate_area->SetBackgroundColour(msbgWHITE);

    wxBoxSizer* slot_group_sizer   = new wxBoxSizer(wxHORIZONTAL);
    m_material_slot_group          = new wxWindow(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(800, -1));
    m_material_slot_group->SetBackgroundColour(msbgWHITE);
    wxSize slot_size(150, 200);
    for (int i = 0; i < 3; ++i) {
        MaterialSlotWgt* material_slot = new MaterialSlotWgt(m_material_slot_group, wxID_ANY, wxString::Format(wxT("%i"), i + 1), wxColour(248, 0, 0));
        material_slot->SetMinSize(slot_size);
        material_slot->SetBackgroundColour(msbgWHITE);
        slot_group_sizer->Add(material_slot, 0, wxEXPAND | wxTOP | wxBOTTOM, (300 - slot_size.GetHeight())/2);
        slot_group_sizer->AddSpacer(10);
        m_material_slots.push_back(material_slot);
    }
    MaterialSlotWgt* material_slot = new MaterialSlotWgt(m_material_slot_group, wxID_ANY, wxString("4"), wxColour(248,0,0));
    material_slot->SetMinSize(slot_size);
    material_slot->SetBackgroundColour(msbgWHITE);
    slot_group_sizer->Add(material_slot, 0, wxEXPAND | wxTOP | wxBOTTOM, (300 - slot_size.GetHeight()) / 2);
    slot_group_sizer->AddStretchSpacer();
    m_material_slots.push_back(material_slot);
    m_material_slot_group->SetSizer(slot_group_sizer);
    m_material_slot_group->Layout();
    slot_group_sizer->Fit(m_material_slot_group);

    wxBoxSizer* btn_group_sizer    = new wxBoxSizer(wxVERTICAL);
    m_button_group                 = new wxWindow(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(1080 - 800, -1));
    m_button_group->SetBackgroundColour(msbgWHITE);
    m_supply_wire = new wxButton(m_button_group, wxID_ANY, _L("supply wire"),wxDefaultPosition, wxSize(150, 50));
    m_withdrawn_wire = new wxButton(m_button_group, wxID_ANY, _L("withdrawn wire"), wxDefaultPosition, wxSize(150, 50));
    m_supply_wire->SetBackgroundColour(wxColour(0, 248, 0));
    m_withdrawn_wire->SetBackgroundColour(wxColour(0, 248, 0));

    wxBoxSizer* switch_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   switch_win   = new wxWindow(m_button_group, wxID_ANY, wxDefaultPosition, wxSize(1080 - 800, -1));
    switch_win->SetBackgroundColour(msbgWHITE);
    m_switch                 = new wxButton(switch_win, wxID_ANY, _L("switch"), wxDefaultPosition, wxSize(50, 50));
    switch_sizer->AddStretchSpacer();
    switch_sizer->Add(m_switch, 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
    switch_win->SetSizer(switch_sizer);
    switch_win->Layout();
    switch_sizer->Fit(switch_win);

    btn_group_sizer->Add(switch_win, 0, wxEXPAND | wxLEFT | wxRIGHT, 0);
    btn_group_sizer->AddStretchSpacer();
    btn_group_sizer->Add(m_supply_wire, 0, wxEXPAND | wxLEFT | wxRIGHT, 40);
    btn_group_sizer->AddSpacer(20);
    btn_group_sizer->Add(m_withdrawn_wire, 0, wxEXPAND | wxLEFT | wxRIGHT, 40);
    btn_group_sizer->AddStretchSpacer();
    m_button_group->SetSizer(btn_group_sizer);
    m_button_group->Layout();
    btn_group_sizer->Fit(m_button_group);

    operate_area_sizer->Add(m_material_slot_group, 0, wxEXPAND | wxALL, 0);
    operate_area_sizer->Add(m_button_group, 0, wxEXPAND | wxALL, 0);
    m_operate_area->SetSizer(operate_area_sizer);
    m_operate_area->Layout();
    operate_area_sizer->Fit(m_operate_area);

    // MaterialPanel下半部分提示区
    wxBoxSizer* tips_area_sizer = new wxBoxSizer(wxVERTICAL);
    m_tips_area                 = new TipsArea(parent,wxID_ANY);
    m_tips_area->SetBackgroundColour(msbgWHITE);
    m_tips_area->SetMinSize(wxSize(1080, 150));
    m_tips_title = new wxStaticText(m_tips_area, wxID_ANY, _L("Tips"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    const wxString tips_text("Clickable slots, single feeding/unwinding for loading/unloading of yarns.");
    m_tips_text = new wxStaticText(m_tips_area, wxID_ANY, _L(tips_text), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    tips_area_sizer->AddStretchSpacer();
    tips_area_sizer->Add(m_tips_title, 0, wxEXPAND | wxLEFT | wxRIGHT, 30);
    tips_area_sizer->AddSpacer(10);
    tips_area_sizer->Add(m_tips_text, 0, wxEXPAND | wxLEFT | wxRIGHT, 30);
    tips_area_sizer->AddStretchSpacer();
    m_tips_area->SetSizer(tips_area_sizer);
    m_tips_area->Layout();
    tips_area_sizer->Fit(m_tips_area);
    
    panel_sizer->Add(m_operate_area, 0, wxEXPAND | wxALL, 0);
    panel_sizer->Add(m_tips_area, 0, wxEXPAND | wxALL, 0);
    SetSizer(panel_sizer);
    Layout();
    panel_sizer->Fit(this);

}

void MaterialPanel::connectEvent() 
{ 
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_supply_wire_clicked, this, m_supply_wire->GetId()); 
}

void MaterialPanel::on_supply_wire_clicked(wxCommandEvent& event) 
{ 
    if (m_material_dialog)        return;
    m_material_dialog = new MaterialDialog(nullptr, wxID_ANY, "", wxDefaultPosition, wxSize(700, 350));

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
    SetBackgroundColour(msbgWHITE);
     create_panel(this);
}

MaterialStation::~MaterialStation() {} 

void MaterialStation::create_panel(wxWindow* parent)
{
    wxBoxSizer* sizer                 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* bSizer_material_title = new wxBoxSizer(wxHORIZONTAL);
    m_material_title                  = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(36)));
    m_material_title->SetBackgroundColour(wxColour(*wxWHITE));

    // 材料站标题
    m_staticText_title = new wxStaticText(m_material_title, wxID_ANY, _L("Material Station"));
    m_staticText_title->SetForegroundColour(wxColour(51, 51, 51));

    bSizer_material_title->Add(m_staticText_title, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(17));
    bSizer_material_title->Add(0, 0, 1, wxEXPAND, 0);
    m_material_title->SetSizer(bSizer_material_title);
    m_material_title->Layout();
    bSizer_material_title->Fit(m_material_title);

    // 材料站内容
    wxBoxSizer* bSizer_material_panel    = new wxBoxSizer(wxHORIZONTAL);
    MaterialPanel* m_material_panel      = new MaterialPanel(parent);
    m_material_panel->SetMinSize(wxSize(1080, 450));
    m_material_panel->SetBackgroundColour(msbgWHITE);
    m_material_panel->SetSizer(bSizer_material_panel);
    m_material_panel->Layout();
    bSizer_material_panel->Fit(m_material_panel);

    sizer->Add(m_material_title, 0, wxEXPAND | wxALL, 0);
    // sizer->AddStretchSpacer();
    sizer->Add(m_material_panel, 0, wxEXPAND | wxALL, 0);
    // sizer->AddStretchSpacer();

    parent->SetSizer(sizer);
    parent->Layout();
    parent->Fit();
}

wxPanel* MaterialStation::GetPrintTitlePanel() { return m_material_title; }




} // namespace GUI

} // namespace Slic3r