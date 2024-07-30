#ifndef slic3r_GUI_MaterialStation_hpp_
#define slic3r_GUI_MaterialStation_hpp_
#include <wx/wx.h>
//#include <wx/intl.h>
#include <wx/panel.h>

namespace Slic3r {
namespace GUI {
class MaterialSlot : public wxWindow
{
public:
    MaterialSlot(wxWindow* parent);
    ~MaterialSlot();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_wheel_clr;
    wxColour m_bucket_clr;

};

class SlotNumber : public wxWindow
{
public:
    SlotNumber(wxWindow* parent);
    ~SlotNumber();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_number_clr;
};


class MaterialSlotWgt : public wxWindow
{
public:
    MaterialSlotWgt(wxWindow* parent, wxString& number);
    ~MaterialSlotWgt();

private:
    void setup_layout(wxWindow* parent, wxString& number);

private:
    MaterialSlot* m_material_slot;
    SlotNumber*   m_number;
    wxString      m_material_name;
};



class TipsArea : public wxPanel
{
public:
    TipsArea(wxWindow* parent);
    ~TipsArea();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    void setup_layout();
};


class MaterialPanel : public wxPanel
{
public:
    MaterialPanel(wxWindow* parent);
    ~MaterialPanel();

protected:

private:
    void setup_layout(wxWindow* parent);

private:
    TipsArea* m_tips_area;
    wxStaticText* m_tips_title;
    wxStaticText* m_tips_text;
    wxPanel*  m_operate_area;
    wxButton* m_supply_wire;
    wxButton* m_withdrawn_wire;
    wxPanel*  m_button_group;
    wxPanel*  m_material_slot_group;
    std::vector<MaterialSlotWgt*> m_material_slots;
};




class MaterialStation : public wxPanel
{
public:
    MaterialStation(wxWindow* parent);
    ~MaterialStation();
    void     create_panel(wxWindow* parent);
    wxPanel* GetPrintTitlePanel();

private:
    wxPanel*       m_material_title;
    wxStaticText*  m_staticText_title;
    MaterialPanel* m_material_panel;
    wxStaticText* m_staticText_subtask_value;
};

} // namespace GUI

} // namespace Slic3r

#endif