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
    MaterialSlot(wxWindow* parent, wxColour& wheel_colour, wxColour& bucket_colour);
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
    SlotNumber(wxWindow* parent, wxColour& number_clr);
    ~SlotNumber();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_number_clr;
};


class MaterialSlotWgt : public wxWindow
{
public:
    MaterialSlotWgt(wxWindow* parent, wxString& number, wxColour& colour);
    ~MaterialSlotWgt();

private:
    void setup_layout(wxWindow* parent, wxString& number, wxColour& colour);

private:
    MaterialSlot* m_material_slot;
    SlotNumber*   m_number;
    wxString      m_material_name;
    wxColour      m_colour;
    wxButton*     m_edit_btn;
};



class TipsArea : public wxWindow
{
public:
    TipsArea(wxWindow* parent);
    ~TipsArea();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    void setup_layout();
};

class ColorButton:public wxButton
{
public:
    ColorButton(wxWindow*          parent,
                wxWindowID         id,
                wxColour&          color,
                const wxString&    label     = wxEmptyString,
                const wxPoint&     pos       = wxDefaultPosition,
                const wxSize&      size      = wxDefaultSize,
                long               style     = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString&    name      = wxASCII_STR(wxButtonNameStr));
    ~ColorButton();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_selected_color;
};
#if 0
class Palette : public wxWindow
{
public:
    Palette(wxWindow*       parent,
            wxWindowID      id,
            const wxPoint&  pos   = wxDefaultPosition,
            const wxSize&   size  = wxDefaultSize,
            long            style = 0,
            const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~Palette();

private:
};
#endif

class Palette : public wxDialog
{
public:
    Palette(wxWindow*       parent,
            wxWindowID      id,
            const wxString& title,
            const wxPoint&  pos   = wxDefaultPosition,
            const wxSize&   size  = wxDefaultSize,
            long            style = wxDEFAULT_DIALOG_STYLE,
            const wxString& name  = wxASCII_STR(wxDialogNameStr));
    ~Palette();
    void set_material_station_color_vector(std::vector<wxColour> color_vec);

protected:
    void resizeEvent(wxSizeEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();

private:
    wxStaticText* m_station_color_lab;
    wxStaticText* m_color_lib_lab;
    std::vector<ColorButton*> m_station_color_btns;
    std::vector<ColorButton*> m_color_lib_btns;
};

class MaterialDialog : public wxDialog
{
public:
    MaterialDialog(wxWindow*       parent,
                   wxWindowID      id,
                   const wxString& title,
                   const wxPoint&  pos   = wxDefaultPosition,
                   const wxSize&   size  = wxDefaultSize,
                   long            style = wxDEFAULT_DIALOG_STYLE,
                   const wxString& name  = wxASCII_STR(wxDialogNameStr));
    ~MaterialDialog();

protected:
    void on_resize(wxSizeEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_color_btn_clicked(wxCommandEvent& event);

private:
    wxStaticText* m_type_lab;
    wxStaticText* m_color_lab;
    wxComboBox*   m_comboBox;
    ColorButton*  m_color_btn;
    wxButton*     m_OK;
    wxButton*     m_cancel;
    wxColour      m_material_color;
    wxString      m_material_name;
};


class MaterialPanel : public wxPanel
{
public:
    MaterialPanel(wxWindow* parent);
    ~MaterialPanel();

protected:

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_supply_wire_clicked(wxCommandEvent& event);

private:
    TipsArea*                     m_tips_area;
    wxStaticText*                 m_tips_title;
    wxStaticText*                 m_tips_text;
    wxWindow*                     m_operate_area;
    wxButton*                     m_supply_wire;
    wxButton*                     m_withdrawn_wire;
    wxButton*                     m_switch;
    wxWindow*                     m_button_group;
    wxWindow*                     m_material_slot_group;
    std::vector<MaterialSlotWgt*> m_material_slots;
    MaterialDialog*               m_material_dialog;
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