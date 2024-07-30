#include "MaterialStation.hpp"
#include <slic3r/GUI/I18N.hpp>

namespace Slic3r {
namespace GUI {

MaterialSlot::MaterialSlot(wxWindow* parent) 
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize) 
{
}

MaterialSlot::~MaterialSlot() {}
    

ButtonPanel::ButtonPanel(wxWindow* parent) {}

ButtonPanel::~ButtonPanel() {}


TipsArea::TipsArea(wxWindow* parent) 
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
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
    
MaterialPanel::MaterialPanel(wxWindow* parent) /*wxSize(-1, FromDIP(208))*/
    : wxPanel(parent, wxID_ANY, wxDefaultPosition,  wxDefaultSize)
{
    setup_layout(this);
}

MaterialPanel::~MaterialPanel() {}

void MaterialPanel::setup_layout(wxWindow* parent) 
{
    //MaterialPanel整体垂直布局
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxVERTICAL);
    //MaterialPanel上半部分操作区（水平）
    wxBoxSizer* operate_area_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_operate_area                 = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 300));
    m_material_slot_group          = new wxPanel(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(800, -1));
    m_material_slot_group->SetBackgroundColour(wxColour(255, 0, 0));

    wxBoxSizer* btn_group_sizer    = new wxBoxSizer(wxVERTICAL);
    m_button_group                 = new wxPanel(m_operate_area, wxID_ANY, wxDefaultPosition, wxSize(1080 - 800, -1));
    m_button_group->SetBackgroundColour(wxColour(0, 255, 0));
    m_supply_wire = new wxButton(m_button_group, wxID_ANY, _L("supply wire"),wxDefaultPosition, wxSize(150, 50));
    m_withdrawn_wire = new wxButton(m_button_group, wxID_ANY, _L("withdrawn wire"), wxDefaultPosition, wxSize(150, 50));
    m_supply_wire->SetBackgroundColour(wxColour(248, 248, 248));
    m_withdrawn_wire->SetBackgroundColour(wxColour(248, 248, 248));
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
    m_tips_area                 = new TipsArea(parent);
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


MaterialStation::MaterialStation(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    SetBackgroundColour(*wxWHITE);
     create_panel(this);
}

MaterialStation::~MaterialStation() {} 

void MaterialStation::create_panel(wxWindow* parent)
{
    wxBoxSizer* sizer                 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* bSizer_material_title = new wxBoxSizer(wxHORIZONTAL);
    m_material_title                  = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(36)));
    m_material_title->SetBackgroundColour(wxColour(248, 248, 248));

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
    m_material_panel->SetBackgroundColour(wxColour(248, 248, 248));
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