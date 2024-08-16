#include "MaterialStation.hpp"
#include <slic3r/GUI/I18N.hpp>
#include <slic3r/GUI/wxExtensions.hpp>
#include <wx/graphics.h>

#define UNKNOWN_COLOR wxColour(248, 248, 248)   //材料站背景颜色

namespace Slic3r {
namespace GUI {

MaterialSlot::MaterialSlot(wxWindow*       parent,
                           wxWindowID      id,
                           const wxPoint&  pos,
                           const wxSize&   size,
                           long            style,
                           const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name) 
    , m_material_info{wxEmptyString, wxColour()}
    , m_type(MaterialSlot::Unknow)
    , m_edit_state(EditState::Normal)
    , m_edit_white_bmp(create_scaled_bitmap("edit_white_btn", nullptr, 14))
    , m_edit_black_bmp(create_scaled_bitmap("edit_black_btn", nullptr, 14))
    , m_edit_hover_bmp(create_scaled_bitmap("edit_hover_btn", nullptr, 14))
    , m_edit_press_bmp(create_scaled_bitmap("edit_press_btn", nullptr, 14))
    , m_seleced_bmp(create_scaled_bitmap("selected_slot", nullptr, 68))
    , m_unknow_bmp(create_scaled_bitmap("unknow_slot", nullptr, 68))
    , m_empty_bmp(create_scaled_bitmap("empty_slot", nullptr, 68))
    , m_unknow_name_bmp(create_scaled_bitmap("unknown_name", nullptr, 12))
{
    SetMinSize(wxSize(FromDIP(60), FromDIP(68)));
    m_edit_pos = wxPoint(FromDIP(32), FromDIP(37));
    m_edit_size = wxSize(FromDIP(14), FromDIP(14));
    SetBackgroundColour(wxColour(255, 255, 255));
    connectEvent();
}   

MaterialSlot::~MaterialSlot() {}

wxColour MaterialSlot::get_color() { return m_material_info.m_color; }

void MaterialSlot::set_slot_type(SlotType type){
    m_type = type;
    Refresh();
}

void MaterialSlot::set_edit_state(EditState state)
{
    m_edit_state = state;
    Refresh();
}

MaterialSlot::SlotType MaterialSlot::get_slot_type() { return m_type; }


bool MaterialSlot::start_supply_wire() 
{ 
    switch (m_type) {
    case MaterialSlot::Selected: {
        // 执行进丝操作
        return true;
        break;
    }
    case MaterialSlot::Unknow: {
        get_user_choices();
        return false;
        break;
    }
    case MaterialSlot::Empty: {
        return false;
        break;
    }
    default: {
        return false;
        break;
    }
    }
    
}

bool MaterialSlot::stop_supply_wire() { return true; }

bool MaterialSlot::start_withdrawn_wire() { return true; }

void MaterialSlot::connectEvent() 
{ 
    Bind(wxEVT_PAINT, &MaterialSlot::paintEvent, this); 
}

void MaterialSlot::paintEvent(wxPaintEvent& event)
{ 
    wxPaintDC dc(this);
    auto      w = GetSize().GetWidth();
    auto      h = GetSize().GetHeight();
    switch (m_type) {
    case MaterialSlot::Selected: {
        dc.SetBrush(wxBrush(m_material_info.m_color));
        dc.SetPen(wxPen(m_material_info.m_color, 0));
        dc.DrawRectangle(0, 0, w, h);//画背景
        int iconX = (w - m_seleced_bmp.GetWidth()) / 2;
        int iconY = (h - m_seleced_bmp.GetHeight()) / 2;
        dc.DrawBitmap(m_seleced_bmp, iconX, iconY);//画料槽

        wxFont font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        int    name_x = FromDIP(30);
        int    name_y = FromDIP(18);
        wxColour fore_color(255, 255, 255);
        wxBitmap* edit_bmp = &m_edit_white_bmp;
        if (m_material_info.m_color == fore_color) {
            fore_color = wxColour(51, 51, 51);
            edit_bmp   = &m_edit_black_bmp;
        }
        dc.SetTextForeground(fore_color);
        dc.SetFont(font);
        dc.DrawText(m_material_info.m_name, name_x, name_y); // 画名字
        draw_edit_bmp(dc, *edit_bmp, m_edit_pos);
        break;
    }
    case MaterialSlot::Unknow: {
        int iconX = (w - m_unknow_bmp.GetWidth()) / 2;
        int iconY = (h - m_unknow_bmp.GetHeight()) / 2;
        dc.DrawBitmap(m_unknow_bmp, iconX, iconY);
        int    name_x = FromDIP(35);
        int    name_y = FromDIP(18);
        dc.DrawBitmap(m_unknow_name_bmp, name_x, name_y); // 画名字
        draw_edit_bmp(dc, m_edit_black_bmp, m_edit_pos);
        break;
    }
    case MaterialSlot::Empty: {
        int iconX = (w - m_empty_bmp.GetWidth()) / 2;
        int iconY = (h - m_empty_bmp.GetHeight()) / 2;
        dc.DrawBitmap(m_empty_bmp, iconX, iconY);
        break;
    }
    default: break;
    }
    
}

void MaterialSlot::draw_edit_bmp(wxPaintDC& dc, wxBitmap& bitmap, wxPoint& point)
{
    switch (m_edit_state) {
    case MaterialSlot::Normal: {
        dc.DrawBitmap(bitmap, point);
        break;
    }
    case MaterialSlot::Hover: {
        dc.DrawBitmap(m_edit_hover_bmp, point);
        break;
    }
    case MaterialSlot::Press: {
        dc.DrawBitmap(m_edit_press_bmp, point);
        break;
    }
    default: break;
    }
}

bool MaterialSlot::in_edit_scope(const wxPoint& pos)
{
    return (pos.x >= m_edit_pos.x && pos.x <= m_edit_pos.x + m_edit_size.GetWidth() && pos.y >= m_edit_pos.y &&
            pos.y <= m_edit_pos.y + m_edit_size.GetHeight());
}

void MaterialSlot::get_user_choices()
{
    // 确定对话框弹出位置
    wxPoint        pos(GetScreenPosition().x + FromDIP(91), GetScreenPosition().y - FromDIP(47)); // 预计弹出位置
    wxSize         dialog_size(FromDIP(422), FromDIP(224));
    wxPoint        finally_pos = MaterialDialog::calculate_pop_position(pos, dialog_size);
    int            state       = 0;
    if (m_type == SlotType::Selected) {
        state = MaterialDialog::InfoState::NameKnown | MaterialDialog::InfoState::ColorKnown;
    }
    MaterialDialog material_dialog(this, wxID_ANY, wxEmptyString, state, finally_pos, dialog_size);
    if (m_material_info.m_color.IsOk()) {
        material_dialog.set_material_color(m_material_info.m_color);
    }
    if (!m_material_info.m_name.empty()) {
        material_dialog.set_material_name(m_material_info.m_name);
    }
    if (material_dialog.ShowModal() == wxID_OK) {
        m_material_info.m_name = material_dialog.get_material_name();
        m_material_info.m_color = material_dialog.get_material_color();
    }
    if (!m_material_info.m_name.empty() && m_material_info.m_color.IsOk()) {
        set_slot_type(SlotType::Selected);
    }
    Refresh();
}

SlotNumber::SlotNumber(wxWindow*       parent,
                       wxWindowID      id,
                       const wxString& number,
                       const wxPoint&  pos,
                       const wxSize&   size,
                       long            style,
                       const wxString& name)
    : wxWindow(parent, id, pos, size, style, name) 
    , m_number(number), m_mode(PaintMode::Normal), m_selected(false)
{
    SetMinSize(wxSize(FromDIP(19), FromDIP(19)));
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &SlotNumber::paintEvent, this);
}


SlotNumber::~SlotNumber() {}

void SlotNumber::set_paint_mode(PaintMode mode)
{
    m_mode = mode;
    Refresh();
}

void SlotNumber::set_selected(bool selected)
{
    m_selected = selected;
    Refresh();
}

void SlotNumber::paintEvent(wxPaintEvent& event)
{
    // 绘制序号椭圆
    wxPaintDC dc(this);
    auto      w = GetSize().GetWidth()- 1;
    auto      h = GetSize().GetHeight()- 1;
    wxColour  curr_color;
    switch (m_mode) {
    case SlotNumber::Normal: {
        if (m_selected) {
            curr_color = wxColour(50, 141, 251);
        } else {
            curr_color = wxColour(221, 221, 221);
        }
        break;
    }
    case SlotNumber::Hover: {
        curr_color = wxColour(149, 197, 255);
        break;
    }
    case SlotNumber::Press: {
        curr_color = wxColour(17, 111, 223);
        break;
    }
    default: break;
    }
    
    dc.SetBrush(wxBrush(curr_color));
    dc.SetPen(wxPen(curr_color, 0));
    dc.DrawEllipse(0, 0, w , h);
    // 绘制序号文本
    int    textX = (GetSize().GetWidth() - FromDIP(7)) / 2;
    int    textY = (GetSize().GetHeight() - FromDIP(16)) / 2;
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(m_number, textX, textY);
}

ProgressNumber::ProgressNumber(
    wxWindow* parent, wxWindowID id, const int number, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style, name)
    , m_process_num(create_scaled_bitmap(std::string("progress_num_") + std::to_string(number), nullptr, 19))
    , m_not_process_num(create_scaled_bitmap(std::string("unprogress_num_") + std::to_string(number), nullptr, 19))
    , m_succeed(create_scaled_bitmap("success_btn", nullptr, 19))
    , m_mode(PaintMode::NotProcess)
{
    SetMinSize(wxSize(FromDIP(19), FromDIP(19)));
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &ProgressNumber::paintEvent, this);
}

ProgressNumber::~ProgressNumber() {}

void ProgressNumber::set_state(PaintMode mode){
    m_mode = mode;
    Refresh();
}

void ProgressNumber::paintEvent(wxPaintEvent& event)
{
    // 绘制序号椭圆
    wxPaintDC dc(this);
    auto      w = GetSize().GetWidth();
    auto      h = GetSize().GetHeight();
    switch (m_mode) {
    case ProgressNumber::Processing: {
        int iconX = (w - m_process_num.GetWidth()) / 2;
        int iconY = (h - m_process_num.GetHeight()) / 2;
        dc.DrawBitmap(m_process_num, iconX, iconY);
        break;
    }
    case ProgressNumber::NotProcess: {
        int iconX = (w - m_not_process_num.GetWidth()) / 2;
        int iconY = (h - m_not_process_num.GetHeight()) / 2;
        dc.DrawBitmap(m_not_process_num, iconX, iconY);
        break;
    }
    case ProgressNumber::Succeed: {
        int iconX = (w - m_succeed.GetWidth()) / 2;
        int iconY = (h - m_succeed.GetHeight()) / 2;
        dc.DrawBitmap(m_succeed, iconX, iconY);
        return;
    }
    default: break;
    }
   
}

MaterialSlotWgt::MaterialSlotWgt(wxWindow*       parent,
                                 wxWindowID      id,
                                 const wxString& number,
                                 const wxPoint&  pos,
                                 const wxSize&   size,
                                 long            style,
                                 const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name)
{ 
    SetMinSize(wxSize(FromDIP(60), FromDIP(89)));
    SetBackgroundColour(wxColour(255, 255, 255));
    setup_layout(this, number);
    connectEvent();
}

MaterialSlotWgt::~MaterialSlotWgt() {}

void MaterialSlotWgt::set_selected(bool selected) { m_number->set_selected(selected); }

wxColour MaterialSlotWgt::get_color() { return m_material_slot->get_color(); }

bool MaterialSlotWgt::start_supply_wire() { return m_material_slot->start_supply_wire(); }

bool MaterialSlotWgt::stop_supply_wire() { return m_material_slot->stop_supply_wire(); }

bool MaterialSlotWgt::start_withdrawn_wire() { return m_material_slot->start_withdrawn_wire(); }

void MaterialSlotWgt::setup_layout(wxWindow* parent, const wxString& number)
{ 
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL); 
    m_number              = new SlotNumber(parent, wxID_ANY, number, wxDefaultPosition, wxSize(FromDIP(20), FromDIP(20))); 
    m_material_slot        = new MaterialSlot(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(60), FromDIP(68)));
    sizer->Add(m_number, 0, wxLEFT | wxRIGHT, (GetSize().GetWidth() - m_number->GetSize().GetWidth()) / 2);
    sizer->AddSpacer(FromDIP(2));
    sizer->Add(m_material_slot, 0, wxLEFT | wxRIGHT, 0);
    SetSizer(sizer);
    Layout();
    sizer->Fit(this);
}

void MaterialSlotWgt::connectEvent()
{
    m_material_slot->Bind(wxEVT_ENTER_WINDOW, &MaterialSlotWgt::OnMouseEnter, this);
    m_material_slot->Bind(wxEVT_LEAVE_WINDOW, &MaterialSlotWgt::OnMouseLeave, this);
    m_material_slot->Bind(wxEVT_LEFT_DOWN, &MaterialSlotWgt::OnMouseDown, this);
    m_material_slot->Bind(wxEVT_LEFT_UP, &MaterialSlotWgt::OnMouseUp, this);
    m_material_slot->Bind(wxEVT_LEFT_DCLICK, &MaterialSlotWgt::OnMouseDclick, this);
    m_material_slot->Bind(wxEVT_MOTION, &MaterialSlotWgt::OnMouseMove, this);
}

void MaterialSlotWgt::OnMouseDown(wxMouseEvent& event) 
{ 
    if (m_material_slot->get_slot_type() == MaterialSlot::SlotType::Empty)
        return;
    m_number->set_paint_mode(SlotNumber::Press); 
    if (m_material_slot->in_edit_scope(event.GetPosition())) {
        m_material_slot->set_edit_state(MaterialSlot::Press);
    }
}

void MaterialSlotWgt::OnMouseUp(wxMouseEvent& event)
{
    if (m_material_slot->get_slot_type() == MaterialSlot::SlotType::Empty)
        return;
    m_number->set_paint_mode(SlotNumber::Normal);
    wxCommandEvent click_event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
    ProcessWindowEvent(click_event);
    if (m_material_slot->in_edit_scope(event.GetPosition())) {
        m_material_slot->set_edit_state(MaterialSlot::Normal);
        m_material_slot->get_user_choices();
    } 
}

void MaterialSlotWgt::OnMouseEnter(wxMouseEvent& event)
{
    if (m_material_slot->get_slot_type() == MaterialSlot::SlotType::Empty)
        return;
    m_number->set_paint_mode(SlotNumber::Hover);
}

void MaterialSlotWgt::OnMouseLeave(wxMouseEvent& event)
{
    if (m_material_slot->get_slot_type() == MaterialSlot::SlotType::Empty)
        return;
    m_number->set_paint_mode(SlotNumber::Normal);
    
}

void MaterialSlotWgt::OnMouseDclick(wxMouseEvent& event)
{
    if (m_material_slot->get_slot_type() == MaterialSlot::SlotType::Empty)
        return;
    m_number->set_paint_mode(SlotNumber::Normal);
    wxCommandEvent click_event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
    ProcessWindowEvent(click_event);
    m_material_slot->get_user_choices();
}

void MaterialSlotWgt::OnMouseMove(wxMouseEvent& event)
{
    if (m_material_slot->in_edit_scope(event.GetPosition())) {
        m_material_slot->set_edit_state(MaterialSlot::Hover);
    } else {
        m_material_slot->set_edit_state(MaterialSlot::Normal);
    }
}

Nozzle::Nozzle(wxWindow*       parent,
               wxWindowID      id,
               const wxPoint&  pos,
               const wxSize&   size,
               long            style,
               const wxString& name)
    : wxWindow(parent, id, pos, size, style, name), m_bitmap(create_scaled_bitmap("nozzle", nullptr, 19))
{
    SetBackgroundColour(wxColour(255, 255, 255));
    SetMinSize(wxSize(FromDIP(32), FromDIP(19)));
    Bind(wxEVT_PAINT, &Nozzle::paintEvent, this);
}

Nozzle::~Nozzle() {}

void Nozzle::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    // 绘制图标
    int iconX = (GetSize().GetWidth() - m_bitmap.GetWidth()) / 2;
    int iconY = (GetSize().GetHeight() - m_bitmap.GetHeight()) / 2;
    dc.DrawBitmap(m_bitmap, iconX, iconY);
}

TipsArea::TipsArea(wxWindow*       parent,
                   wxWindowID      id,
                   const wxPoint&  pos,
                   const wxSize&   size,
                   long            style,
                   const wxString& name) 
    : wxWindow(parent, id, pos, size, style, name), m_state(TipsAreaState::TAS_TIPS)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    prepare_layout(this);
    switch_layout_state(TipsAreaState::TAS_TIPS);
    connectEvent();
}

TipsArea::~TipsArea() {}

void TipsArea::switch_layout_state(TipsAreaState state) 
{
    m_state = state;
    switch (m_state) {
    case TipsArea::TAS_TIPS: {
        m_tips_area_title->SetLabel(_L("Tips"));
        const wxString tips_text("Clickable slots, single feeding/unwinding for loading/unloading of yarns.");
        m_tips_text->SetLabel(_L(tips_text));
        layout_tips_info();
        break;
    }
    case Slic3r::GUI::TipsArea::TAS_SUPPLY: {
        m_tips_area_title->SetLabel(_L("supply wire"));
        layout_progress_status();
        break;
    }
    case Slic3r::GUI::TipsArea::TAS_WITHDRAWN: {
        m_tips_area_title->SetLabel(_L("withdrawn wire"));
        layout_progress_status();
        break;
    }
    default: break;
    }
}

void TipsArea::connectEvent() { Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TipsArea::on_cancel_clicked,this, m_progress->GetId()); }

void TipsArea::on_cancel_clicked(wxCommandEvent& event)
{
    wxCommandEvent cancel_clicked_event(wxEVT_COMMAND_BUTTON_CLICKED, GetId()); // 通知父窗口取消被按下
    ProcessWindowEvent(cancel_clicked_event);
}

void TipsArea::prepare_layout(wxWindow* parent)
{
    //控件的创建
    m_tips_area_title = new wxStaticText(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(269), FromDIP(17)), wxALIGN_LEFT);
    m_tips_area_title->SetForegroundColour(wxColour(50, 141, 251));
    m_tips_text = new wxStaticText(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(269), FromDIP(149)), wxALIGN_LEFT);
    m_progress  = new ProgressArea(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(269), FromDIP(141)));

}

void TipsArea::layout_tips_info()
{
    // 布局提示及文本控件
    wxBoxSizer* tips_sizer = new wxBoxSizer(wxVERTICAL);
    tips_sizer->AddSpacer(FromDIP(45));
    tips_sizer->Add(m_tips_area_title, 0, wxLEFT | wxRIGHT, FromDIP(27));
    tips_sizer->Add(m_tips_text, 0, wxLEFT | wxRIGHT, FromDIP(27));
    tips_sizer->AddStretchSpacer();
    m_progress->Hide();
    m_tips_text->Show();
    SetSizer(tips_sizer);
    Layout();
}

void TipsArea::layout_progress_status()
{
    // 布局进度信息控件
    wxBoxSizer* progress_sizer = new wxBoxSizer(wxVERTICAL);
    progress_sizer->AddSpacer(FromDIP(45));
    progress_sizer->Add(m_tips_area_title, 0, wxLEFT | wxRIGHT, FromDIP(27));
    progress_sizer->AddSpacer(FromDIP(8));
    progress_sizer->Add(m_progress, 0, wxLEFT | wxRIGHT, FromDIP(27));
    progress_sizer->AddStretchSpacer();
    m_tips_text->Hide();
    m_progress->Show();
    SetSizer(progress_sizer);
    Layout();
}


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
    dc.DrawLine(width / 2, 0, width / 2, height);
}


ProgressArea::ProgressArea(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style, name)
{
    setup_layout(this);
    connectEvent();
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
        ProgressNumber* col_btn = new ProgressNumber(num_btn_area, wxID_ANY, i + 1, wxDefaultPosition,
                                               wxSize(FromDIP(21), FromDIP(21)));
        m_btn_group.push_back(col_btn);
        num_btn_sizer->Add(col_btn, 0, wxLEFT | wxRIGHT, 0);
        if (i < 3) { 
            //col_btn->set_state(ProgressNumber::PaintMode::Succeed);
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
    m_cancel_btn = new RoundedButton(cancel_area, wxID_ANY, false, _L("Cancel"), wxDefaultPosition,
                                                            wxSize(FromDIP(38), FromDIP(22)));
    m_cancel_btn->set_state_color(wxColour(50, 141, 251), RoundedButton::Normal);
    m_cancel_btn->set_state_color(wxColour(149, 197, 255), RoundedButton::Hovered);
    m_cancel_btn->set_state_color(wxColour(17, 111, 223), RoundedButton::Pressed);
    m_cancel_btn->set_radius(4);
    m_cancel_btn->SetForegroundColour(wxColour(50, 141, 251));

    cancel_sizer->AddStretchSpacer();
    cancel_sizer->Add(m_cancel_btn, 0, wxLEFT | wxRIGHT, 0);
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

void ProgressArea::connectEvent()
{
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ProgressArea::on_cancel_clicked, this, m_cancel_btn->GetId());
}

void ProgressArea::on_cancel_clicked(wxCommandEvent& event)
{
    wxCommandEvent cancel_clicked_event(wxEVT_COMMAND_BUTTON_CLICKED, GetId()); //通知父窗口取消被按下
    ProcessWindowEvent(cancel_clicked_event);
}


MaterialSlotArea::MaterialSlotArea(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style, name), m_nozzle_point(wxPoint(-1, -1)), m_current_slot(nullptr)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    prepare_layout(this);
    setup_layout_four(this);
    connectEvent();
}

MaterialSlotArea::~MaterialSlotArea() {}

void MaterialSlotArea::change_layout_mode(LayoutMode layout_model) 
{
    switch (layout_model) {
    case LayoutMode::One: {
        setup_layout_one(this);
        connectEvent();
        break;
    }
    case LayoutMode::Four: {
        setup_layout_four(this);
        connectEvent();
        break;
    }
    default: break;
    }
}

MaterialSlotWgt* MaterialSlotArea::get_current_slot() { return m_current_slot; }

void MaterialSlotArea::abandon_selected()
{
    if (m_current_slot) {
        m_current_slot->set_selected(false);
        m_current_slot = nullptr;
    }
}

std::vector<wxColour> MaterialSlotArea::get_all_material_color() 
{ 
    std::vector<wxColour> color_all;
    color_all.reserve((*m_curr_slot_contaier).size());
    for (auto& slot : (*m_curr_slot_contaier)) {
        if (slot->get_color().IsOk()) {
            color_all.push_back(slot->get_color());
        }
    }
    return color_all;
}

bool MaterialSlotArea::start_supply_wire() { return m_current_slot->start_supply_wire(); }

bool MaterialSlotArea::stop_supply_wire() { return m_current_slot->stop_supply_wire(); }

bool MaterialSlotArea::start_withdrawn_wire() { return m_current_slot->start_withdrawn_wire(); }

void MaterialSlotArea::paintEvent(wxPaintEvent& event)
{
    if (m_slot_points.empty() || (m_nozzle_point.x == -1 && m_nozzle_point.y == -1))
        return;
    //先画中间贯通的直线
    wxPaintDC dc(this);
    dc.SetPen(wxPen(wxColour(221, 221, 221), FromDIP(2)));
    int x1 = m_slot_points.front().x;
    int x2 = m_slot_points.back().x;
    int Y  = (m_slot_points.front().y + m_nozzle_point.y) / 2;
    dc.DrawLine(x1, Y, x2, Y);
    //将料槽与直线相连
    for (auto& point : m_slot_points) {
        dc.DrawLine(point.x, point.y, point.x, Y);
    }
    //将喷嘴与直线相连
    dc.DrawLine(m_nozzle_point.x, m_nozzle_point.y, m_nozzle_point.x, Y);
}

void MaterialSlotArea::on_asides_mouse_down(wxMouseEvent& event)
{
    abandon_selected();
    wxCommandEvent clicked_event(wxEVT_COMMAND_BUTTON_CLICKED, GetId()); // 为了改变进丝按钮状态
    ProcessWindowEvent(clicked_event);
}

void MaterialSlotArea::connectEvent() 
{ 
    Bind(wxEVT_PAINT, &MaterialSlotArea::paintEvent, this);
    for (auto& slot : (*m_curr_slot_contaier)) {
        Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialSlotArea::slot_selected_event, this, slot->GetId());
    }
    
}

void MaterialSlotArea::calculate_connection_points(const wxPoint& slot_offset, const wxPoint& nozzle_offset)
{
    //分别计算槽和喷嘴的链接点
    std::vector<wxPoint> slot_points;
    slot_points.reserve((*m_curr_slot_contaier).size());
    for (auto& slot : (*m_curr_slot_contaier)) {
        wxPoint pos  = slot->GetPosition();
        wxSize  size = slot->GetSize();
        int     x    = pos.x + size.GetWidth() / 2;
        int     y    = pos.y + size.GetHeight();
        slot_points.push_back(wxPoint(x, y) + slot_offset);
    }
    wxPoint pos  = m_nozzle->GetPosition();
    wxSize  size = m_nozzle->GetSize();
    int     x    = pos.x + size.GetWidth() / 2;
    int     y    = pos.y;
    m_nozzle_point = wxPoint(x, y) + nozzle_offset;
    m_slot_points.swap(slot_points);
}

void MaterialSlotArea::prepare_layout(wxWindow* parent)
{
    // 上方料槽所需容器
    m_slot_group       = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(88)));
    m_slot_group->Bind(wxEVT_LEFT_DOWN, &MaterialSlotArea::on_asides_mouse_down, this);
    m_slot_group->SetBackgroundColour(wxColour(255, 255, 255));
    // 准备四色料槽所需料槽
    for (int i = 0; i < 4; ++i) {
        wxString         number(wxString::Format(wxT("%i"), i + 1));
        MaterialSlotWgt* material_slot = new MaterialSlotWgt(m_slot_group, wxID_ANY, number, wxDefaultPosition,
                                                             wxSize(FromDIP(60), FromDIP(89)));
        m_material_slots_four.push_back(material_slot);
    }
    // 准备外挂料槽所需料槽
    wxString         number(wxString::Format(wxT("%i"), 1));
    MaterialSlotWgt* material_slot = new MaterialSlotWgt(m_slot_group, wxID_ANY, number, wxDefaultPosition, wxSize(FromDIP(60), FromDIP(89)));
    m_material_slot_one.push_back(material_slot);

    // 准备下方喷嘴所需容器
    m_nozzle_win   = new wxWindow(parent, wxID_ANY, wxDefaultPosition,
                                            wxSize(m_slot_group->GetSize().GetWidth(), FromDIP(19))); // 与上边的四个料槽等宽
    m_nozzle_win->Bind(wxEVT_LEFT_DOWN, &MaterialSlotArea::on_asides_mouse_down, this);
    m_nozzle_win->SetBackgroundColour(wxColour(255, 255, 255));
    // 准备下方喷嘴
    m_nozzle = new Nozzle(m_nozzle_win, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(32), FromDIP(19)));
    m_nozzle->Bind(wxEVT_LEFT_DOWN, &MaterialSlotArea::on_asides_mouse_down, this);
   
}

void MaterialSlotArea::setup_layout_four(wxWindow* parent)
{
    // 整体布局所用的sizer
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    //布局上方四个料槽
    wxBoxSizer* slot_group_sizer = new wxBoxSizer(wxHORIZONTAL);
    for (int i = 0; i < m_material_slot_one.size(); ++i) {
        m_material_slot_one[i]->Hide();
    }
    for (int i = 0; i < 4; ++i) {
        slot_group_sizer->Add(m_material_slots_four[i], 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
        m_material_slots_four[i]->Show();
        if (i < 3) {
            slot_group_sizer->AddSpacer(FromDIP(33));
        }
    }
    m_slot_group->SetSizer(slot_group_sizer);
    m_slot_group->Layout();
    slot_group_sizer->Fit(m_slot_group);
    //布局下方喷嘴
    wxBoxSizer* nozzle_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_nozzle_win->SetMinSize(wxSize(m_slot_group->GetSize().GetWidth(), FromDIP(19)));
    nozzle_sizer->AddStretchSpacer();
    nozzle_sizer->Add(m_nozzle, 0, wxTOP | wxBOTTOM, 0);
    nozzle_sizer->AddStretchSpacer();
    m_nozzle_win->SetSizer(nozzle_sizer);
    m_nozzle_win->Layout();

    //整体布局
    sizer->AddSpacer(FromDIP(13));
    sizer->Add(m_slot_group, 0, wxLEFT, FromDIP(33));
    sizer->AddStretchSpacer();
    sizer->Add(m_nozzle_win, 0, wxLEFT, FromDIP(33));
    SetSizer(sizer);
    Layout();
    m_curr_slot_contaier = &m_material_slots_four;
    calculate_connection_points(m_slot_group->GetPosition(), m_nozzle_win->GetPosition());
    m_layout_mode = LayoutMode::Four;
}

void MaterialSlotArea::setup_layout_one(wxWindow* parent)
{
    // 整体布局所用的sizer
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    // 布局上方一个料槽
    wxBoxSizer* slot_group_sizer = new wxBoxSizer(wxHORIZONTAL);
    for (int i = 0; i < m_material_slots_four.size(); ++i) {
        m_material_slots_four[i]->Hide();
    }

    slot_group_sizer->Add(m_material_slot_one[0], 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
    m_material_slot_one[0]->Show();
    m_slot_group->SetSizer(slot_group_sizer);
    m_slot_group->Layout();
    slot_group_sizer->Fit(m_slot_group);
    // 布局下方喷嘴
    wxBoxSizer* nozzle_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_nozzle_win->SetMinSize(wxSize(m_slot_group->GetSize().GetWidth(), FromDIP(19)));
    nozzle_sizer->AddStretchSpacer();
    nozzle_sizer->Add(m_nozzle, 0, wxTOP | wxBOTTOM, 0);
    nozzle_sizer->AddStretchSpacer();
    m_nozzle_win->SetSizer(nozzle_sizer);
    m_nozzle_win->Layout();

    // 整体布局
    sizer->AddSpacer(FromDIP(13));
    sizer->Add(m_slot_group, 0, wxLEFT, FromDIP(33));
    sizer->AddStretchSpacer();
    sizer->Add(m_nozzle_win, 0, wxLEFT, FromDIP(33));
    SetSizer(sizer);
    Layout();
    m_curr_slot_contaier = &m_material_slot_one;
    calculate_connection_points(m_slot_group->GetPosition(), m_nozzle_win->GetPosition());
    m_layout_mode = LayoutMode::One;
}

void MaterialSlotArea::slot_selected_event(wxCommandEvent& event)
{
    //当有某个槽被点击了
    std::vector<int> ids;
    for (auto& slot : *m_curr_slot_contaier) {
        if (event.GetId() == slot->GetId()) {
            m_current_slot = slot;
            m_current_slot->set_selected(true); 
        } else {
            slot->set_selected(false);
        }
        ids.push_back(slot->GetId());
    }
    int            event_id = event.GetId();
    wxCommandEvent clicked_event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());//为了改变进丝按钮状态
    ProcessWindowEvent(clicked_event);
}
std::vector<MaterialSlotWgt*> MaterialSlotArea::m_material_slots_four;
std::vector<MaterialSlotWgt*> MaterialSlotArea::m_material_slot_one;
std::vector<MaterialSlotWgt*>* MaterialSlotArea::m_curr_slot_contaier = nullptr;
    
ColorButton::ColorButton(wxWindow*          parent,
                         wxWindowID         id,
                         const wxString&    label,
                         const wxPoint&     pos,
                         const wxSize&      size,
                         long               style,
                         const wxValidator& validator,
                         const wxString&    name) 
    : wxButton(parent, id, label, pos, size, style, validator, name)
    , m_color(wxColour(255, 255, 255))
    , m_unknow_color(create_scaled_bitmap("unknow_color_btn", nullptr, 26))
    , m_transparent(create_scaled_bitmap("transparent_circle", nullptr, 26))
    , m_white_circle(create_scaled_bitmap("color_lib_white", nullptr, 26))
    , m_mode(PaintMode::UnknowColor)
{ 
    SetBackgroundColour(wxColour(255, 255, 255));
    Bind(wxEVT_PAINT, &ColorButton::paintEvent, this); 
}

ColorButton::~ColorButton() {}

void ColorButton::set_color(const wxColour& color)
{
    m_color = color;
    if (m_color == wxColour("#FFFFFF")) {
        m_mode = PaintMode::WhiteWithCircle;
    } else {
        m_mode = PaintMode::Transparent;
    }
    Refresh();
}

wxColour& ColorButton::get_color() { return m_color; }

void ColorButton::paintEvent(wxPaintEvent& event)
{
    wxSize    size = GetSize();
    wxPaintDC dc(this);
    switch (m_mode) {
    case ColorButton::Transparent: {
        dc.SetBrush(wxBrush(m_color));
        dc.SetPen(wxPen(m_color, 0));
        int iconX = (size.GetWidth() - m_transparent.GetWidth()) / 2;
        int iconY = (size.GetHeight() - m_transparent.GetHeight()) / 2;
        dc.DrawRectangle(wxPoint(iconX, iconY), m_transparent.GetSize());
        dc.DrawBitmap(m_transparent, iconX, iconY);
        break;
    }
    case ColorButton::UnknowColor: {
        int iconX = (size.GetWidth() - m_unknow_color.GetWidth()) / 2;
        int iconY = (size.GetHeight() - m_unknow_color.GetHeight()) / 2;
        dc.DrawBitmap(m_unknow_color, iconX, iconY);
        break;
    }
    case ColorButton::WhiteWithCircle: {
        int iconX = (size.GetWidth() - m_white_circle.GetWidth()) / 2;
        int iconY = (size.GetHeight() - m_white_circle.GetHeight()) / 2;
        dc.DrawBitmap(m_white_circle, iconX, iconY);
        break;
    }
    default: break;
    }
}

RoundedButton::RoundedButton(wxWindow*          parent,
                             wxWindowID         id,
                             bool               isFill,
                             const wxString&    label,
                             const wxPoint&     pos,
                             const wxSize&      size,
                             long               style,
                             const wxValidator& validator,
                             const wxString&    name) 
    : wxButton(parent, id, label, pos, size, style, validator, name)
    , m_state(ButtonState::Normal)
    , m_is_fill(isFill)
    , m_bitmap_available(false)
    , m_radius(0.0)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    connectEvent();
}

RoundedButton::~RoundedButton()
{}

void RoundedButton::set_bitmap(const wxBitmap& bitmap) 
{ 
    m_bitmap = bitmap;
    m_bitmap_available = true;
}

void RoundedButton::set_state_color(const wxColour& color, ButtonState state) 
{
    switch (state) {
    case RoundedButton::Normal: {
        m_normal_color = color;
        break;
    }
    case RoundedButton::Hovered: {
        m_hovered_color = color;
        break;
    }
    case RoundedButton::Pressed: {
        m_pressed_color = color;
        break;
    }
    case RoundedButton::Inavaliable: {
        m_inavaliable_color = color;
        break;
    }
    default: break;
    }
}

void RoundedButton::set_radius(double radius) { m_radius = radius; }

void RoundedButton::set_state(ButtonState state)
{
    m_state = state;
    Refresh();
}

void RoundedButton::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    // 根据状态绘制不同的背景颜色
    wxSize size = GetSize();
    switch (m_state) {
    case ButtonState::Normal: {
        if (m_is_fill) {
            dc.SetBrush(wxBrush(m_normal_color));
            dc.SetPen(wxPen(m_normal_color, 0));
        } else {
            dc.SetPen(wxPen(m_normal_color));
        }
        break;
    }
    case ButtonState::Hovered: {
        if (m_is_fill) {
            dc.SetBrush(wxBrush(m_hovered_color));
            dc.SetPen(wxPen(m_hovered_color, 0));
        } else {
            dc.SetPen(wxPen(m_hovered_color));
        }
        break;
    }
    case ButtonState::Pressed: {
        if (m_is_fill) {
            dc.SetBrush(wxBrush(m_pressed_color));
            dc.SetPen(wxPen(m_pressed_color, 0));
        } else {
            dc.SetPen(wxPen(m_pressed_color));
        }
        break;
    }
    default: break;
    }
    // 不可用优先级最高
    if (!IsEnabled()) {
        if (m_is_fill) {
            dc.SetBrush(wxBrush(m_inavaliable_color));
            dc.SetPen(wxPen(m_inavaliable_color, 0));
        } else {
            dc.SetPen(wxPen(m_inavaliable_color));
        }
    }
    dc.DrawRoundedRectangle(0, 0, size.GetWidth() - FromDIP(1), size.GetHeight() - FromDIP(1), m_radius);
    // 绘制文本
    int textX = (size.x - dc.GetTextExtent(GetLabel()).x) / 2;
    int textY = (size.y - dc.GetTextExtent(GetLabel()).y) / 2;
    dc.DrawText(GetLabel(), textX, textY);

    if (m_bitmap_available) {
        // 绘制图标
        int iconX = (size.GetWidth() - m_bitmap.GetWidth()) / 2;
        int iconY = (size.GetHeight() - m_bitmap.GetHeight()) / 2;
        dc.DrawBitmap(m_bitmap, iconX, iconY);
    }
    
}

void RoundedButton::OnMouseDown(wxMouseEvent& event){
    m_state = ButtonState::Pressed;
    Refresh();
    wxCommandEvent clickEvent(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
    ProcessWindowEvent(clickEvent);
}

void RoundedButton::OnMouseUp(wxMouseEvent& event) {
    m_state = ButtonState::Normal;
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
        dc.SetPen(wxPen(wxColour(50, 141, 251)));
        dc.DrawRectangle(0, GetSize().GetHeight() - FromDIP(2), GetSize().GetWidth(), FromDIP(2));
    }
}


Palette::Palette(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name) 
    : wxDialog(parent, id, title, pos, size, wxNO_BORDER | wxFRAME_SHAPED, name)
    , m_seleced_color(wxColour(255, 255, 255))
{
    SetMinSize(wxSize(FromDIP(309), FromDIP(294)));
    setup_layout(this); 
    connectEvent();
}

Palette::~Palette() {}

void Palette::set_material_station_color_vector(std::vector<wxColour> color_vec) 
{ 
    
}

wxColour& Palette::get_seleced_color() { return m_seleced_color; }

void Palette::resizeEvent(wxSizeEvent& event)
{
    wxEventBlocker evtBlocker(this, wxEVT_SIZE);
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight(), 6);
    SetShape(path);
    event.Skip();
}

void Palette::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
    dc.SetPen(wxPen(wxColour(193, 193, 193), 1));
    int width  = GetSize().GetWidth();
    int height = GetSize().GetHeight();
    int radius = 6;
    dc.DrawRoundedRectangle(0, 0, width, height, radius);
}

void Palette::setup_layout(wxWindow* parent) 
{ 
    int         width         = GetSize().GetWidth() - FromDIP(4);//减去4是为了给paint时间画出的圆角矩形留空间
    int         height        = GetSize().GetHeight();
    wxBoxSizer* palette_sizer = new wxBoxSizer(wxVERTICAL);
    //关闭按钮布局
    wxBoxSizer* sizer_close    = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_close  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(10)));
    area_close->SetBackgroundColour(wxColour(255, 255, 255));
    wxButton* close_btn = new wxButton(area_close, wxID_CANCEL, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(10), FromDIP(10)), wxNO_BORDER);
    close_btn->SetBackgroundColour(wxColour(255, 255, 255));
    close_btn->SetBitmap(create_scaled_bitmap("color_close_btn", nullptr, 10));
    sizer_close->AddStretchSpacer();
    sizer_close->Add(close_btn, 0, wxTOP | wxBOTTOM, 0);
    sizer_close->AddSpacer(FromDIP(19));
    area_close->SetSizer(sizer_close);
    area_close->Layout();
    //材料站颜色标题布局
    wxBoxSizer* sizer_station_title = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_station_title  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(19)));
    area_station_title->SetBackgroundColour(wxColour(255, 255, 255));
    m_station_color_lab             = new wxStaticText(area_station_title, wxID_ANY, _L("Material Station"), wxDefaultPosition,
                                                       wxSize(FromDIP(249), FromDIP(19)), wxALIGN_LEFT);
    m_station_color_lab->SetBackgroundColour(wxColour(255, 255, 255));
    sizer_station_title->AddSpacer(FromDIP(27));
    sizer_station_title->Add(m_station_color_lab, 0, wxTOP | wxBOTTOM, 0);
    sizer_station_title->AddStretchSpacer();
    area_station_title->SetSizer(sizer_station_title);
    area_station_title->Layout();
    // 材料站颜色按钮布局
    wxBoxSizer* sizer_station_color = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_station_color  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(26)));
    area_station_color->SetBackgroundColour(wxColour(255, 255, 255));
    sizer_station_color->AddSpacer(FromDIP(27));
    std::vector<wxColour> all_color(MaterialSlotArea::get_all_material_color());
    for (auto& color : all_color) {
        ColorButton* color_btn = new ColorButton(area_station_color, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                                 wxSize(FromDIP(26), FromDIP(26)));
        color_btn->set_color(color);
        m_station_color_btns.push_back(color_btn);
        sizer_station_color->Add(color_btn, 0, wxTOP | wxBOTTOM, 0);
        sizer_station_color->AddSpacer(FromDIP(30));
    }
    sizer_station_color->AddStretchSpacer();
    area_station_color->SetSizer(sizer_station_color);
    area_station_color->Layout();
    // 颜色库标题布局
    wxBoxSizer* sizer_lib_title = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   area_lib_title  = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(width, FromDIP(19)));
    area_lib_title->SetBackgroundColour(wxColour(255, 255, 255));
    m_color_lib_lab = new wxStaticText(area_lib_title, wxID_ANY, _L("Color Library"), wxDefaultPosition, wxSize(FromDIP(249), FromDIP(19)),
                                       wxALIGN_LEFT);
    m_color_lib_lab->SetBackgroundColour(wxColour(255, 255, 255));
    sizer_lib_title->AddSpacer(FromDIP(27));
    sizer_lib_title->Add(m_color_lib_lab, 0, wxTOP | wxBOTTOM, 0);
    sizer_lib_title->AddStretchSpacer();
    area_lib_title->SetSizer(sizer_lib_title);
    area_lib_title->Layout();
    //颜色库按钮布局
    wxGridSizer* gridSizer      = new wxGridSizer(5, 5, FromDIP(7), FromDIP(30)); // 6 行 4 列，垂直水平间距为 7，30
    wxWindow*    area_lib_color = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(249), FromDIP(156)));
    area_lib_color->SetBackgroundColour(wxColour(255, 255, 255));
    for (int i = 0; i < 24; ++i) {
        ColorButton* color_btn = new ColorButton(area_lib_color, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                                 wxSize(FromDIP(26), FromDIP(26)));
        color_btn->set_color(wxColour(color_lib[i]));
        m_color_lib_btns.push_back(color_btn);
        gridSizer->Add(color_btn, 0, wxALIGN_CENTRE | wxALL, 0);
    }
    area_lib_color->SetSizer(gridSizer);
    area_lib_color->Layout();
    //整体布局
    palette_sizer->AddSpacer(FromDIP(9));
    palette_sizer->Add(area_close, 0, wxLEFT | wxRIGHT, FromDIP(2));
    palette_sizer->AddSpacer(FromDIP(6));
    palette_sizer->Add(area_station_title, 0, wxLEFT | wxRIGHT, FromDIP(2));
    palette_sizer->AddSpacer(FromDIP(7));
    palette_sizer->Add(area_station_color, 0, wxLEFT | wxRIGHT, FromDIP(2));
    palette_sizer->AddSpacer(FromDIP(17));
    palette_sizer->Add(area_lib_title, 0, wxLEFT | wxRIGHT, FromDIP(2));
    palette_sizer->AddSpacer(FromDIP(7));
    palette_sizer->Add(area_lib_color, 0, wxLEFT, FromDIP(27));
    palette_sizer->AddSpacer(FromDIP(17));
    SetSizer(palette_sizer);
    Layout();
    palette_sizer->Fit(this);
}

void Palette::connectEvent() 
{ 
    Bind(wxEVT_SIZE, &Palette::resizeEvent, this);
    Bind(wxEVT_PAINT, &Palette::paintEvent, this);
    assert(!m_color_lib_btns.empty());
    for (auto& btn : m_color_lib_btns) {
        Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Palette::on_color_lib_clicked, this, btn->GetId());
    } 
}

void Palette::on_color_lib_clicked(wxCommandEvent& event) 
{ 
    wxObject* btn = event.GetEventObject(); 
    ColorButton* color_btn = static_cast<ColorButton*>(btn);
    m_seleced_color        = color_btn->get_color();
    wxCommandEvent clickEvent(wxEVT_COMMAND_BUTTON_CLICKED, wxID_OK);
    ProcessWindowEvent(clickEvent);
}

const char* Palette::color_lib[] = {"#FFFFFF", "#FEF043", "#DCF478", "#0ACC38", "#067749", "#0C6283", "#0DE2A0", "#75D9F3",
                                    "#45A8F9", "#2750E0", "#46328E", "#A03CF7", "#F330F9", "#D4B0DC", "#F95D73", "#F72224",
                                    "#7C4B00", "#F98D33", "#FDEBD5", "#D3C4A3", "#AF7836", "#898989", "#BCBCBC", "#161616"};

MaterialDialog::MaterialDialog(wxWindow*       parent,
                               wxWindowID      id,
                               const wxString& title,
                               const int&      state,
                               const wxPoint&  pos,
                               const wxSize&   size,
                               long            style,
                               const wxString& name)
    : wxDialog(parent, id, title, pos, size, wxNO_BORDER | wxFRAME_SHAPED, name)
    , m_material_name(wxEmptyString)
    , m_material_color(wxColour())
    , m_state(state)
{
    setup_layout(this);
    connectEvent();
}
MaterialDialog::~MaterialDialog() {}

wxPoint MaterialDialog::calculate_pop_position(const wxPoint& point, const wxSize& size)
{
    wxDisplay display;
    wxRect    screenRect = display.GetClientArea();

    wxPoint finally_pos = point; // 最终弹出位置
    // 先横向比较
    int min_X = screenRect.x;
    int max_X = screenRect.x + screenRect.width;
    int min_x = point.x;
    int max_x = point.x + size.GetWidth();
    if (min_x < min_X) { // 对话框左溢出屏幕
        finally_pos.x += (min_X - min_x);
    }
    if (max_x > max_X) { // 对话框右溢出屏幕
        finally_pos.x -= (max_x - max_X);
    }
    // 再纵向比较
    int min_Y = screenRect.y;
    int max_Y = screenRect.y + screenRect.height;
    int min_y = point.y;
    int max_y = point.y + size.GetHeight();
    if (min_y < min_Y) { // 对话框上溢出屏幕
        finally_pos.y += (min_Y - min_y);
    }
    if (max_y > max_Y) { // 对话框下溢出屏幕
        finally_pos.y -= (max_y - max_Y);
    }
    return finally_pos;
}

void MaterialDialog::set_material_name(const wxString& name)
{
    m_material_name = name; // 这里还要同步名字到combobox
    int index = 0;
    for (; index < m_options.size(); ++index) {
        if (m_options[index] == name) {
            break;
        }
    }
    if (index != m_options.size()) {
        m_comboBox->SetSelection(index);
    }
}

void MaterialDialog::set_material_color(const wxColour& color)
{
    m_material_color = color;
    m_color_btn->set_color(m_material_color);
}

wxColour& MaterialDialog::get_material_color() { return m_material_color; }

wxString& MaterialDialog::get_material_name() { return m_material_name; }

int MaterialDialog::get_info_state() { return m_state; }

void MaterialDialog::set_info_state(int state){
    m_state = state;
    update_ok_state();
}

void MaterialDialog::resizeEvent(wxSizeEvent& event)
{
    wxEventBlocker evtBlocker(this, wxEVT_SIZE);
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight(), 6);
    SetShape(path);
    event.Skip();
}

void MaterialDialog::paintEvent(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
    dc.SetPen(wxPen(wxColour(193, 193, 193), 1));
    int width  = GetSize().GetWidth();
    int height = GetSize().GetHeight();
    int radius = 6; 
    dc.DrawRoundedRectangle(0, 0, width, height, radius); 
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

    m_comboBox = new CustomOwnerDrawnComboBox(select_area, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(347), FromDIP(34)), 0,
                                              NULL, wxCB_READONLY);
    init_comboBox();

    m_color_lab = new wxStaticText(select_area, wxID_ANY, _L("Color"), wxDefaultPosition, wxSize(FromDIP(347), FromDIP(19)), wxALIGN_LEFT);

    wxBoxSizer* color_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   color_area  = new wxWindow(select_area, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(347), FromDIP(26)));
    color_area->SetBackgroundColour(wxColour(255, 255, 255));
    m_color_btn = new ColorButton(color_area, wxID_ANY, wxEmptyString, wxDefaultPosition,
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
    m_cancel = new RoundedButton(button_area, wxID_CANCEL, false, _L("Cancel"), wxDefaultPosition, wxSize(FromDIP(87), FromDIP(36)));
    m_cancel->set_state_color(wxColour(65, 148, 136), RoundedButton::Normal);
    m_cancel->set_state_color(wxColour(101, 167, 158), RoundedButton::Hovered);
    m_cancel->set_state_color(wxColour(26, 134, 118), RoundedButton::Pressed);
    m_cancel->set_state_color(wxColour(221, 221, 221), RoundedButton::Inavaliable);
    m_cancel->set_radius(4);
    m_cancel->SetForegroundColour(wxColour(65, 148, 136));

    m_OK     = new RoundedButton(button_area, wxID_OK, true, _L("OK"), wxDefaultPosition, wxSize(FromDIP(87), FromDIP(36)));
    m_OK->set_state_color(wxColour(65, 148, 136), RoundedButton::Normal);
    m_OK->set_state_color(wxColour(101, 167, 158), RoundedButton::Hovered);
    m_OK->set_state_color(wxColour(26, 134, 118), RoundedButton::Pressed);
    m_OK->set_state_color(wxColour(221, 221, 221), RoundedButton::Inavaliable);
    m_OK->set_radius(4);
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
    Bind(wxEVT_SIZE, &MaterialDialog::resizeEvent, this);
    Bind(wxEVT_PAINT, &MaterialDialog::paintEvent, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialDialog::on_color_btn_clicked, this, m_color_btn->GetId());
    Bind(wxEVT_COMBOBOX, &MaterialDialog::on_comboBox_selected, this, m_comboBox->GetId());
}

void MaterialDialog::on_color_btn_clicked(wxCommandEvent& event) 
{ 
    wxPoint  pos(GetScreenPosition().x + GetSize().GetWidth() + FromDIP(5), GetScreenPosition().y); // 预计弹出位置
    wxSize  dialog_size(FromDIP(309), FromDIP(294));
    wxPoint  finally_pos = calculate_pop_position(pos, dialog_size);

    Palette palette(nullptr, wxID_ANY, wxEmptyString, finally_pos, dialog_size);
    if (palette.ShowModal() == wxID_OK) {
        set_material_color(palette.get_seleced_color());
        set_info_state(get_info_state() | InfoState::ColorKnown);
    } else {
        m_color_btn->Refresh();
    }
}

void MaterialDialog::on_comboBox_selected(wxCommandEvent& event)
{
    int      selectedIndex  = m_comboBox->GetSelection();
    wxString selectedString = m_comboBox->GetString(selectedIndex);
    set_material_name(selectedString);
    set_info_state(get_info_state() | InfoState::NameKnown);
}

void MaterialDialog::init_comboBox()
{
    for (const auto& option : m_options) {
        m_comboBox->Append(option);
    }
    m_comboBox->SetSelection(0);
}

void MaterialDialog::update_ok_state()
{
    m_OK->Enable((m_state & InfoState::NameKnown) > 0 == (m_state & InfoState::ColorKnown) > 0);
    m_OK->Refresh();
}

std::vector<wxString> MaterialDialog::m_options = {"unknow", "ABS", "ASA", "PETG", "PLA", "AUD", "OUH", "PWM", "PLA", "AUD", "OUH", "PWM"};

MaterialPanel::MaterialPanel(wxWindow*       parent,
                             wxWindowID      winid,
                             const wxPoint&  pos,
                             const wxSize&   size,
                             long            style,
                             const wxString& name)
    : wxPanel(parent, winid, pos, size, style, name)
{
    SetBackgroundColour(wxColour(248, 248, 248));
    //启动布局后各种界面均为默认状态
    setup_layout(this);
    connectEvent();
    //更新真实数据到界面

}

MaterialPanel::~MaterialPanel() {}

void MaterialPanel::init_material_panel() {}

void MaterialPanel::OnMouseDown(wxMouseEvent& event) { 
    m_material_slot->abandon_selected(); 
    m_supply_wire->Enable(false);
    m_withdrawn_wire->Enable(false);
}

void MaterialPanel::setup_layout(wxWindow* parent) 
{
    int width = GetSize().GetWidth();
    int height = GetSize().GetHeight();
    //MaterialPanel整体水平布局
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    //MaterialPanel左半部分操作区
    wxBoxSizer* operate_area_sizer = new wxBoxSizer(wxVERTICAL);
    wxWindow* operate_area         = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(407), height));
    operate_area->SetBackgroundColour(wxColour(255, 255, 255));
    operate_area->Bind(wxEVT_LEFT_DOWN, &MaterialPanel::OnMouseDown, this);

    //左半部分操作区的上边的切换按钮区
    wxBoxSizer* switch_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow*   switch_group = new wxWindow(operate_area, wxID_ANY, wxDefaultPosition,
                                            wxSize(operate_area->GetSize().GetWidth(), FromDIP(34)));
    switch_group->SetBackgroundColour(wxColour(255, 255, 255));
    switch_group->Bind(wxEVT_LEFT_DOWN, &MaterialPanel::OnMouseDown, this);

    m_recognized_btn = new IdentifyButton(switch_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(51), FromDIP(34)));
    m_recognized_btn->set_bitmap(create_scaled_bitmap("four_color_select", nullptr, 21), 
                                            create_scaled_bitmap("four_color_unselect", nullptr, 21));
    m_recognized_btn->set_select_state(true);
    m_unrecognized_btn = new IdentifyButton(switch_group, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(51), FromDIP(34)));
    m_unrecognized_btn->set_bitmap(create_scaled_bitmap("plug_slot_switch_btn_select", nullptr, 21),
                                   create_scaled_bitmap("plug_slot_switch_btn_unselect", nullptr, 21));
    m_unrecognized_btn->set_select_state(false);
    switch_sizer->Add(m_recognized_btn, 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
    switch_sizer->AddSpacer(FromDIP(32));
    switch_sizer->Add(m_unrecognized_btn, 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
    switch_sizer->AddStretchSpacer();
    switch_group->SetSizer(switch_sizer);
    switch_group->Layout();

    // 左半部分操作区的中间的料槽区
    m_material_slot = new MaterialSlotArea(operate_area, wxID_ANY, wxDefaultPosition,
                                           wxSize(operate_area->GetSize().GetWidth(), FromDIP(140)));//增加10
    // 左半部分操作区的下边的按钮区
    wxBoxSizer* btn_group_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxWindow* button_group = new wxWindow(operate_area, wxID_ANY, wxDefaultPosition, wxSize(operate_area->GetSize().GetWidth(), FromDIP(52)));
    button_group->SetBackgroundColour(wxColour(255, 255, 255));
    button_group->Bind(wxEVT_LEFT_DOWN, &MaterialPanel::OnMouseDown, this);

    m_supply_wire = new RoundedButton(button_group, wxID_ANY, true, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(66), FromDIP(30)));
    m_supply_wire->set_state_color(wxColour(50, 141, 251), RoundedButton::Normal);
    m_supply_wire->set_state_color(wxColour(149, 197, 255), RoundedButton::Hovered);
    m_supply_wire->set_state_color(wxColour(17, 111, 223), RoundedButton::Pressed);
    m_supply_wire->set_state_color(wxColour(221, 221, 221), RoundedButton::Inavaliable);
    m_supply_wire->set_radius(4);
    m_supply_wire->set_bitmap(create_scaled_bitmap("supply_wire", nullptr, 23));
    m_supply_wire->Enable(false);


    m_withdrawn_wire = new RoundedButton(button_group, wxID_ANY, true, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(66), FromDIP(30)));
    m_withdrawn_wire->set_state_color(wxColour(50, 141, 251), RoundedButton::Normal);
    m_withdrawn_wire->set_state_color(wxColour(149, 197, 255), RoundedButton::Hovered);
    m_withdrawn_wire->set_state_color(wxColour(17, 111, 223), RoundedButton::Pressed);
    m_withdrawn_wire->set_state_color(wxColour(221, 221, 221), RoundedButton::Inavaliable);
    m_withdrawn_wire->set_radius(4);
    m_withdrawn_wire->set_bitmap(create_scaled_bitmap("withdrawn_wire", nullptr, 23));
    m_withdrawn_wire->Enable(false);
    btn_group_sizer->AddSpacer(FromDIP(134));
    btn_group_sizer->Add(m_supply_wire, 0, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(11));
    btn_group_sizer->AddSpacer(FromDIP(23));
    btn_group_sizer->Add(m_withdrawn_wire, 0, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(11));
    btn_group_sizer->AddStretchSpacer();
    button_group->SetSizer(btn_group_sizer);
    button_group->Layout();
    // MaterialPanel左半部分操作区整体布局
    operate_area_sizer->Add(switch_group, 0, wxEXPAND | wxALL, 0);
    operate_area_sizer->Add(m_material_slot, 0, wxEXPAND | wxALL, 0);
    operate_area_sizer->Add(button_group, 0, wxEXPAND | wxALL, 0);
    operate_area->SetSizer(operate_area_sizer);
    operate_area->Layout();
    operate_area_sizer->Fit(operate_area);

    // MaterialPanel右半部分提示区
    m_tips_area                 = new TipsArea(parent, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(322), height));
    
    //整体布局
    panel_sizer->Add(operate_area, 0, wxEXPAND | wxALL, 0);
    panel_sizer->AddSpacer(FromDIP(1));
    panel_sizer->Add(m_tips_area, 0, wxEXPAND | wxALL, 0);
    SetSizer(panel_sizer);
    Layout();
    panel_sizer->Fit(this);
}

void MaterialPanel::connectEvent() 
{ 
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_supply_wire_clicked, this, m_supply_wire->GetId()); 
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_withdrawn_wire_clicked, this, m_withdrawn_wire->GetId()); 
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_recognized_clicked, this, m_recognized_btn->GetId());
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_unrecognized_clicked, this, m_unrecognized_btn->GetId());
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_slot_area_clicked, this, m_material_slot->GetId());
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MaterialPanel::on_tips_area_cancel_clicked, this, m_tips_area->GetId());

    m_material_slot->Bind(wxEVT_LEFT_DOWN, &MaterialPanel::OnMouseDown, this);
}

void MaterialPanel::on_supply_wire_clicked(wxCommandEvent& event) 
{ 
    if (m_material_slot->start_supply_wire()) {
        //成功开始进丝
        m_tips_area->switch_layout_state(TipsArea::TAS_SUPPLY); 
    } else {
        //进丝失败
    }
}

void MaterialPanel::on_withdrawn_wire_clicked(wxCommandEvent& event)
{
    if (m_material_slot->start_withdrawn_wire()) {
        // 成功开始退丝
        m_tips_area->switch_layout_state(TipsArea::TAS_WITHDRAWN);
    } else {
        // 进丝失败
    }
}

void MaterialPanel::on_recognized_clicked(wxCommandEvent& event) 
{ 
    m_material_slot->change_layout_mode(MaterialSlotArea::Four); 
    m_recognized_btn->set_select_state(true);
    m_unrecognized_btn->set_select_state(false);
    m_material_slot->abandon_selected();
    m_supply_wire->Enable(false);
    m_withdrawn_wire->Enable(false);
}

void MaterialPanel::on_unrecognized_clicked(wxCommandEvent& event) 
{ 
    m_material_slot->change_layout_mode(MaterialSlotArea::One);
    m_recognized_btn->set_select_state(false);
    m_unrecognized_btn->set_select_state(true);
    m_material_slot->abandon_selected();
    m_supply_wire->Enable(false);
    m_withdrawn_wire->Enable(false);
}

void MaterialPanel::on_slot_area_clicked(wxCommandEvent& event)
{
    bool enable = (m_material_slot->get_current_slot()) ? true : false;
    m_supply_wire->Enable(enable);
    m_withdrawn_wire->Enable(enable);
}

void MaterialPanel::on_tips_area_cancel_clicked(wxCommandEvent& event)
{
    if (m_material_slot->stop_supply_wire()) {
        // 成功停止进丝
        m_tips_area->switch_layout_state(TipsArea::TAS_TIPS);
    } else {
        // 进丝失败
    }
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




CustomOwnerDrawnComboBox::CustomOwnerDrawnComboBox(wxWindow*          parent,
                                                   wxWindowID         id,
                                                   const wxString&    value,
                                                   const wxPoint&     pos,
                                                   const wxSize&      size,
                                                   int                n,
                                                   const wxString     choices[],
                                                   long               style,
                                                   const wxValidator& validator,
                                                   const wxString&    name)
    : wxOwnerDrawnComboBox(parent, id, value, pos, size, n, choices, style, validator, name)
    , m_up(create_scaled_bitmap("arrow_up", nullptr, 5)) 
    , m_down(create_scaled_bitmap("arrow_down", nullptr, 5))
    , m_hover_item(-1), m_isExpanded(false)
{
    connectEvent();
}

wxCoord CustomOwnerDrawnComboBox::OnMeasureItem(size_t item) const
{
    return FromDIP(34); // 每个选项的高度; 
}

void CustomOwnerDrawnComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
{
    wxColour txt_color(51, 51, 51);
    wxColour background_color(255, 255, 255);
    if (flags & wxODCB_PAINTING_SELECTED) {
        txt_color        = wxColour(255, 255, 255);
        background_color = wxColour(50, 141, 251); // 悬停时的背景色
    }
    dc.SetBrush(wxBrush(background_color));
    dc.SetPen(wxPen(background_color, 0));
    dc.DrawRectangle(rect);

    wxString text = GetString(item);
    dc.SetTextBackground(txt_color);
    dc.DrawText(text, rect.x + FromDIP(17), rect.y + FromDIP(7));

}

void CustomOwnerDrawnComboBox::paintEvent(wxPaintEvent& event) 
{
    // 绘制圆角边框
    wxPaintDC dc(this);
    wxRect    rect = GetClientRect();

    dc.SetPen(wxPen(wxColour(193, 193, 193), 1)); // 边框颜色和宽度
    dc.SetBrush(wxBrush(wxColour(255, 255, 255))); // 背景颜色
    //if (m_isExpanded) {
    //    // 绘制上边两个圆角
    //    dc.DrawRoundedRectangle(rect, 6);
    //    // 绘制下边两个直角
    //    dc.DrawRectangle(rect.GetPosition().x, rect.GetHeight() / 3, rect.GetWidth(), rect.GetHeight() / 3 * 2);
    //} else {
    //    dc.DrawRoundedRectangle(rect, 6); // 10 为圆角半径
    //}
    dc.DrawRoundedRectangle(rect, 6); // 10 为圆角半径
    // 绘制内部文本
    dc.SetTextForeground(wxColour(51, 51, 51)); // 文本颜色
    dc.DrawText(GetValue(), rect.x + FromDIP(17), rect.y + FromDIP(7)); // 文本位置
    // 绘制箭头
    int symmetry_x = rect.GetRight() - FromDIP(15); //上下箭头对称轴x坐标
    int symmetry_y = (rect.GetHeight() - FromDIP(6)) / 2;   //上下箭头组成矩形的水平平分线y坐标
    wxPoint left_top(symmetry_x - FromDIP(6), symmetry_y - FromDIP(2));
    if (m_isExpanded) {
        dc.DrawBitmap(m_up, left_top);
    } else {
        dc.DrawBitmap(m_down, left_top);
    }
    
}

void CustomOwnerDrawnComboBox::connectEvent()
{ 
    Bind(wxEVT_PAINT, &CustomOwnerDrawnComboBox::paintEvent, this); 
    Bind(wxEVT_MOTION, &CustomOwnerDrawnComboBox::OnMouseMove, this); 
    Bind(wxEVT_COMBOBOX_DROPDOWN, &CustomOwnerDrawnComboBox::OnDropdown, this);
    Bind(wxEVT_COMBOBOX_CLOSEUP, &CustomOwnerDrawnComboBox::OnCloseUp, this);
}

void CustomOwnerDrawnComboBox::OnMouseMove(wxMouseEvent& event)
{
    int item = HitTest(event.GetPosition());
    if (item != wxNOT_FOUND) {
        m_hover_item = item;
        Refresh(); // 刷新以重新绘制
    } else {
        m_hover_item = -1;
        Refresh();
    }
    static std::vector<int> items;
    static std::vector<wxPoint> pos;
    items.push_back(item);
    pos.push_back(event.GetPosition());
    if (items.size() == 20) {
        Refresh();
    }
}

void CustomOwnerDrawnComboBox::OnDropdown(wxCommandEvent& event)
{
    m_isExpanded = true;
    Refresh();
}

void CustomOwnerDrawnComboBox::OnCloseUp(wxCommandEvent& event)
{
    m_isExpanded = false;
    Refresh();
}


} // namespace GUI

} // namespace Slic3r
