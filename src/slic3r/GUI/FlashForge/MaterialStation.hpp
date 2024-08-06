#ifndef slic3r_GUI_MaterialStation_hpp_
#define slic3r_GUI_MaterialStation_hpp_
#include <wx/wx.h>
//#include <wx/intl.h>
#include <wx/panel.h>

namespace Slic3r {
namespace GUI {

class ColorButton;
class ProgressArea;

struct MaterialInfo
{
    wxString m_name;
    wxColour m_color;
    MaterialInfo(const wxString& name, const wxColour& color) : m_name(name), m_color(color) {}
};

class MaterialSlot : public wxWindow
{
public:
    MaterialSlot(wxWindow*       parent,
                 wxWindowID      id,
                 const wxPoint&  pos   = wxDefaultPosition,
                 const wxSize&   size  = wxDefaultSize,
                 long            style = 0,
                 const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~MaterialSlot();
    enum SlotType { Selected = 0, Unknow = 1 ,Empty = 2};
    void set_color(wxColour color);
    void set_slot_type(SlotType type);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_color;
    SlotType m_type;
    wxBitmap m_seleced_bmp;
    wxBitmap m_unknow_bmp;
    wxBitmap m_empty_bmp;

};

class SlotNumber : public wxWindow
{
public:
    SlotNumber(wxWindow*       parent,
               wxWindowID      id,
               const wxString&       number,
               const wxPoint&  pos   = wxDefaultPosition,
               const wxSize&   size  = wxDefaultSize,
               long            style = 0,
               const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~SlotNumber();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxString m_number;
};


class MaterialSlotWgt : public wxWindow
{
public:
    MaterialSlotWgt(wxWindow* parent, 
                    wxWindowID id,
                    const wxString& number,
                    const wxPoint&  pos   = wxDefaultPosition,
                    const wxSize&   size  = wxDefaultSize,
                    long            style = 0,
                    const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~MaterialSlotWgt();

private:
    void setup_layout(wxWindow* parent, const wxString& number);

private:
    MaterialSlot* m_material_slot;
    SlotNumber*   m_number;
    wxButton*     m_edit_btn;

    wxString      m_material_name;
    wxColour      m_material_color;
};

class Nozzle : public wxWindow
{
public:
    Nozzle(wxWindow*       parent,
           wxWindowID      id,
           const wxPoint&  pos   = wxDefaultPosition,
           const wxSize&   size  = wxDefaultSize,
           long            style = 0,
           const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~Nozzle();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxBitmap m_bitmap;
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
    enum TipsAreaState { TAS_TIPS = 0, TAS_SUPPLY = 1, TAS_WITHDRAWN = 2 };

private:
    void setup_layout(wxWindow* parent);
    void layout_tips_info();
    void layout_progress_status();

private:
    wxStaticText* m_tips_area_title;
    wxStaticText* m_tips_text;
    ProgressArea* m_progress;
    TipsAreaState m_state;

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

class MaterialSlotArea : public wxWindow
{
public:
    MaterialSlotArea(wxWindow*       parent,
                     wxWindowID      id,
                     const wxPoint&  pos   = wxDefaultPosition,
                     const wxSize&   size  = wxDefaultSize,
                     long            style = 0,
                     const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~MaterialSlotArea();
    enum LayoutMode { One = 0, Four = 1};
    void change_layout_mode(LayoutMode layout_model);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    void connectEvent();
    void clear_old_layout(wxWindow* parent);
    void calculate_connection_points(wxPoint& slot_offset, wxPoint& nozzle_offset);
    void setup_layout_four(wxWindow* parent);
    void setup_layout_one(wxWindow* parent);

private:
    std::vector<MaterialSlotWgt*> m_material_slots;
    Nozzle*                       m_nozzle;
    std::vector<wxPoint>          m_slot_points;
    wxPoint                       m_nozzle_point;
};


class ColorButton:public wxButton
{
public:
    ColorButton(wxWindow*          parent,
                wxWindowID         id,
                const wxString&    label     = wxEmptyString,
                const wxPoint&     pos       = wxDefaultPosition,
                const wxSize&      size      = wxDefaultSize,
                long               style     = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString&    name      = wxASCII_STR(wxButtonNameStr));
    ~ColorButton();
    enum PaintMode { ColoredRound = 1, Icon = 1 << 2, Text = 1 << 3 };
    void set_color(const wxColour& color);
    wxColour& get_color();
    void change_paint_mode(int mode);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_color;
    int m_paint_mode;
};

class RoundedButton : public wxButton
{
public:
    RoundedButton(wxWindow*          parent,
                  wxWindowID         id,
                  bool               isFill,
                  const wxString&    label     = wxEmptyString,
                  const wxPoint&     pos       = wxDefaultPosition,
                  const wxSize&      size      = wxDefaultSize,
                  long               style     = 0,
                  const wxValidator& validator = wxDefaultValidator,
                  const wxString&    name      = wxASCII_STR(wxButtonNameStr));
    ~RoundedButton();
    enum ButtonState { Normal = 0, Hovered = 1, Pressed = 2 };
    void set_bitmap(const wxBitmap& bitmap);
    void set_state_color(const wxColour& color, ButtonState state);
    void set_radius(double radius);

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
    bool        m_is_fill;
    bool        m_bitmap_available;
    wxColour    m_normal_color;
    wxColour    m_hovered_color;
    wxColour    m_pressed_color;
    double      m_radius;
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
    wxColour& get_seleced_color();

protected:
    void resizeEvent(wxSizeEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_color_lib_clicked(wxCommandEvent& event);

private:
    wxStaticText* m_station_color_lab;
    wxStaticText* m_color_lib_lab;
    std::vector<ColorButton*> m_station_color_btns;
    std::vector<ColorButton*> m_color_lib_btns;
    wxColour                  m_seleced_color;
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
    static wxPoint calculate_pop_position(const wxPoint& point, const wxSize& size);
    void           set_material_color(const wxColour& color);
    wxColour&      get_material_color();
    wxString&      get_material_name();

protected:
    void resizeEvent(wxSizeEvent& event);
    void paintEvent(wxPaintEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_color_btn_clicked(wxCommandEvent& event);

private:
    wxStaticText* m_type_lab;
    wxStaticText* m_color_lab;
    wxComboBox*   m_comboBox;
    ColorButton*  m_color_btn;
    RoundedButton* m_OK;
    RoundedButton* m_cancel;
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
    void init_material_panel();

protected:

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void pop_dialog();
    void on_supply_wire_clicked(wxCommandEvent& event);
    void on_recognized_clicked(wxCommandEvent& event);
    void on_unrecognized_clicked(wxCommandEvent& event);

private:
    TipsArea*                     m_tips_area;
    RoundedButton*                m_supply_wire;
    RoundedButton*                m_withdrawn_wire;
    IdentifyButton*               m_recognized_btn;
    IdentifyButton*               m_unrecognized_btn;
    MaterialSlotArea*             m_material_slot;

    std::vector<MaterialInfo>         m_material;
    
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
};

} // namespace GUI

} // namespace Slic3r

#endif