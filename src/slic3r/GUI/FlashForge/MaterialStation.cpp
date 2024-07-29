#include "MaterialStation.hpp"
#include <slic3r/GUI/I18N.hpp>

namespace Slic3r {
namespace GUI {

MaterialStation::MaterialStation(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    SetBackgroundColour(*wxWHITE);
    create_panel(this);
}

MaterialStation::~MaterialStation() {} 

void MaterialStation::create_panel(wxWindow* parent)
{
    wxBoxSizer* sizer                 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bSizer_printing_title = new wxBoxSizer(wxHORIZONTAL);

    m_panel_printing_title = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(36)), wxTAB_TRAVERSAL);
    m_panel_printing_title->SetBackgroundColour(wxColour(248, 248, 248));

    // 材料站标题
    m_staticText_printing = new wxStaticText(m_panel_printing_title, wxID_ANY, _L("Material Station"));
    // m_staticText_printing->SetForegroundColour(wxColour(51,51,51));
    m_staticText_printing->SetForegroundColour(wxColour(248, 248, 248));

    bSizer_printing_title->Add(m_staticText_printing, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(17));
    bSizer_printing_title->Add(0, 0, 1, wxEXPAND, 0);
    m_panel_printing_title->SetSizer(bSizer_printing_title);
    m_panel_printing_title->Layout();
    bSizer_printing_title->Fit(m_panel_printing_title);

    // 材料站内容
    wxBoxSizer* bSizer_task_name_hor = new wxBoxSizer(wxHORIZONTAL);
    wxPanel*    task_name_panel      = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(208)), wxTAB_TRAVERSAL);
    // m_staticText_subtask_value = new wxStaticText(task_name_panel, wxID_ANY, _L("Unconnected"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER
    // | wxST_ELLIPSIZE_END); m_staticText_subtask_value->Wrap(-1); m_staticText_subtask_value->SetForegroundColour(wxColour(255, 255, 255));

    // bSizer_task_name_hor->Add(m_staticText_subtask_value, 0, wxALIGN_CENTER, 0);

    task_name_panel->SetSizer(bSizer_task_name_hor);
    task_name_panel->Layout();
    bSizer_task_name_hor->Fit(task_name_panel);

    sizer->Add(m_panel_printing_title, 0, wxEXPAND | wxALL, 0);
    // sizer->AddStretchSpacer();
    sizer->Add(task_name_panel, 0, wxALIGN_CENTER, 0);
    // sizer->AddStretchSpacer();

    parent->SetSizer(sizer);
    parent->Layout();
    parent->Fit();
}

wxPanel* MaterialStation::GetPrintTitlePanel() { return m_panel_printing_title; }

} // namespace GUI

} // namespace Slic3r