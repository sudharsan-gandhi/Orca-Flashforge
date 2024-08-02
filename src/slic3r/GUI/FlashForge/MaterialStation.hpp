#ifndef slic3r_GUI_MaterialStation_hpp_
#define slic3r_GUI_MaterialStation_hpp_
#include <wx/wx.h>
//#include <wx/intl.h>
#include <wx/panel.h>

namespace Slic3r {
namespace GUI {

class ColorButton;


class MaterialSlot : public wxWindow
{
public:
    MaterialSlot(wxWindow*       parent,
                 wxWindowID      id,
                 wxColour&       wheel_clr,
                 wxColour&       bucket_clr,
                 const wxPoint&  pos   = wxDefaultPosition,
                 const wxSize&   size  = wxDefaultSize,
                 long            style = 0,
                 const wxString& name  = wxASCII_STR(wxPanelNameStr));
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
    SlotNumber(wxWindow*       parent,
               wxWindowID      id,
               wxColour&       number_clr,
               const wxPoint&  pos   = wxDefaultPosition,
               const wxSize&   size  = wxDefaultSize,
               long            style = 0,
               const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~SlotNumber();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_number_clr;
};


class MaterialSlotWgt : public wxWindow//wxWindow* parent, wxString& number, wxColour& colour
{
public:
    MaterialSlotWgt(wxWindow*       parent,
                    wxWindowID      id,
                    wxString&       number,
                    wxColour&       colour,
                    const wxPoint&  pos   = wxDefaultPosition,
                    const wxSize&   size  = wxDefaultSize,
                    long            style = 0,
                    const wxString& name  = wxASCII_STR(wxPanelNameStr));
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
    TipsArea(wxWindow*       parent,
             wxWindowID      id,
             const wxPoint&  pos   = wxDefaultPosition,
             const wxSize&   size  = wxDefaultSize,
             long            style = 0,
             const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~TipsArea();

protected:

};

class LineArea : public wxWindow
{
public:
    LineArea(wxWindow*       parent,
                 wxWindowID      id,
                 const wxPoint&  pos   = wxDefaultPosition,
                 const wxSize&   size  = wxDefaultSize,
                 long            style = 0,
                 const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~LineArea();

protected:
    void paintEvent(wxPaintEvent& event);

};

class ProgressArea : public wxWindow
{
public:
    ProgressArea(wxWindow*       parent,
             wxWindowID      id,
             const wxPoint&  pos   = wxDefaultPosition,
             const wxSize&   size  = wxDefaultSize,
             long            style = 0,
             const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~ProgressArea();

private:
    void setup_layout(wxWindow* parent);

private:
    std::vector<ColorButton*> m_btn_group;
    std::vector<wxStaticText*> m_txt_group;
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

class RoundedButton : public wxWindow
{
public:
    RoundedButton(wxWindow*       parent,
                  wxWindowID      id,
                  const wxPoint&  pos   = wxDefaultPosition,
                  const wxSize&   size  = wxDefaultSize,
                  long            style = 0,
                  const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~RoundedButton();
    enum ButtonState { Normal = 0, Hovered = 1, Pressed = 2 };
    void set_bitmap(const wxBitmap& bitmap);

protected:
    void paintEvent(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);

private:
    void connectEvent();

private:
    ButtonState m_state;
    wxBitmap    m_bitmap;
};

class IdentifyButton : public wxButton
{
public:
    IdentifyButton(wxWindow*          parent,
                   wxWindowID         id,
                   const wxString&    label     = wxEmptyString,
                   const wxPoint&     pos       = wxDefaultPosition,
                   const wxSize&      size      = wxDefaultSize,
                   long               style     = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString&    name      = wxASCII_STR(wxButtonNameStr));
    ~IdentifyButton();
    void set_bitmap(const wxBitmap& bitmap, const wxBitmap& unselect);
    void set_select_state(bool isSelected);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    bool     m_isSelected;
    wxBitmap m_select_bitmap;
    wxBitmap m_unselect_bitmap;
};

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
    MaterialPanel(wxWindow*       parent,
                  wxWindowID      winid = wxID_ANY,
                  const wxPoint&  pos   = wxDefaultPosition,
                  const wxSize&   size  = wxDefaultSize,
                  long            style = wxTAB_TRAVERSAL | wxNO_BORDER,
                  const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~MaterialPanel();
    enum TipsAreaState {
        TAS_TIPS = 0,
        TAS_SUPPLY = 1,
        TAS_WITHDRAWN = 2
    };

protected:

private:
    void setup_layout(wxWindow* parent);
    void setup_tips_layout();
    void layout_tips_info(TipsArea* parent);
    void layout_progress_status(TipsArea* parent);
    void connectEvent();
    void on_supply_wire_clicked(wxCommandEvent& event);

private:
    TipsArea*                     m_tips_area;
    wxStaticText*                 m_tips_area_title;
    wxStaticText*                 m_tips_text;
    ProgressArea*                 m_progress;
    wxWindow*                     m_operate_area;
    RoundedButton*                m_supply_wire;
    RoundedButton*                m_withdrawn_wire;
    IdentifyButton*                 m_recognized_btn;
    IdentifyButton*                 m_unrecognized_btn;
    wxWindow*                     m_button_group;
    wxWindow*                     m_material_slot_group;
    std::vector<MaterialSlotWgt*> m_material_slots;
    MaterialDialog*               m_material_dialog;
    TipsAreaState                 m_tips_area_state;
};




class MaterialStation : public wxPanel
{
public:
    MaterialStation(wxWindow*       parent,
                    wxWindowID      winid = wxID_ANY,
                    const wxPoint&  pos   = wxDefaultPosition,
                    const wxSize&   size  = wxDefaultSize,
                    long            style = wxTAB_TRAVERSAL | wxNO_BORDER,
                    const wxString& name  = wxASCII_STR(wxPanelNameStr));
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