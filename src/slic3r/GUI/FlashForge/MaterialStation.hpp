#ifndef slic3r_GUI_MaterialStation_hpp_
#define slic3r_GUI_MaterialStation_hpp_
#include <wx/wx.h>
#include <wx/odcombo.h>
#include <wx/panel.h>

namespace Slic3r {
namespace GUI {

class ColorButton;
class ProgressArea;
class MaterialDialog;
class RoundedButton;

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
    enum EditState { Normal = 0, Hover = 1, Press = 2 };

    wxColour get_color();
    void     set_slot_type(SlotType type);
    void     set_edit_state(EditState type);
    SlotType get_slot_type();
    EditState get_edit_state();

    void     get_user_choices(); // 会弹出对话框
    bool     in_edit_scope(const wxPoint& pos);

    bool start_supply_wire();
    bool stop_supply_wire();
    bool start_withdrawn_wire();
    bool stop_withdrawn_wire();

protected:
    void connectEvent();
    void paintEvent(wxPaintEvent& event);
    void draw_edit_bmp(wxPaintDC& dc, wxBitmap& bitmap, wxPoint& point);

private:
    

private:
    SlotType     m_type;
    EditState    m_edit_state;
    MaterialInfo m_material_info;
    wxBitmap     m_edit_white_bmp;
    wxBitmap     m_edit_black_bmp;
    wxBitmap     m_edit_hover_bmp;
    wxBitmap     m_edit_press_bmp;

    wxBitmap m_seleced_bmp;
    wxBitmap m_unknow_bmp;
    wxBitmap m_empty_bmp;

    wxBitmap m_unknow_name_bmp;

    wxPoint m_edit_pos;
    wxSize  m_edit_size;
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
    enum PaintMode { Normal = 1, Hover = 2, Press = 3};
    void set_paint_mode(PaintMode mode);
    void set_selected(bool selected);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxString m_number;
    PaintMode m_mode;
    bool      m_selected;
};

class ProgressNumber : public wxWindow
{
public:
    ProgressNumber(wxWindow*       parent,
               wxWindowID      id,
               const int       number,
               const wxPoint&  pos   = wxDefaultPosition,
               const wxSize&   size  = wxDefaultSize,
               long            style = 0,
               const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~ProgressNumber();
    enum PaintMode { Processing = 1, NotProcess = 2 , Succeed = 3};
    void set_state(PaintMode mode);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxBitmap  m_process_num;
    wxBitmap  m_not_process_num;
    wxBitmap  m_succeed;
    PaintMode m_mode;
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
    void set_selected(bool selected);
    wxColour get_color();
    bool start_supply_wire();
    bool stop_supply_wire();
    bool start_withdrawn_wire();
    bool stop_withdrawn_wire();
        
private:
    void setup_layout(wxWindow* parent, const wxString& number);
    void connectEvent();
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMouseDclick(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

private:
    MaterialSlot* m_material_slot;
    SlotNumber*   m_number;
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
    void switch_layout_state(TipsAreaState state);

private:
    void connectEvent();
    void on_cancel_clicked(wxCommandEvent& event);
    void prepare_layout(wxWindow* parent);
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
    void connectEvent();
    void on_cancel_clicked(wxCommandEvent& event);

private:
    std::vector<ProgressNumber*>    m_btn_group;
    std::vector<wxStaticText*>      m_txt_group;
    RoundedButton*                  m_cancel_btn;
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
    MaterialSlotWgt*             get_current_slot();
    void                         abandon_selected();
    static std::vector<wxColour> get_all_material_color();

    bool           start_supply_wire();
    bool           stop_supply_wire();
    bool           start_withdrawn_wire();
    bool           stop_withdrawn_wire();

protected:
    void paintEvent(wxPaintEvent& event);
    void on_asides_mouse_down(wxMouseEvent& event);

private:
    void connectEvent();
    void calculate_connection_points(const wxPoint& slot_offset, const wxPoint& nozzle_offset);
    void prepare_layout(wxWindow* parent);
    void setup_layout_four(wxWindow* parent);
    void setup_layout_one(wxWindow* parent);

    void slot_selected_event(wxCommandEvent& event);

private:
    static std::vector<MaterialSlotWgt*> m_material_slots_four;
    static std::vector<MaterialSlotWgt*> m_material_slot_one;
    static std::vector<MaterialSlotWgt*>*       m_curr_slot_contaier;
    Nozzle*                              m_nozzle;
    wxWindow*                            m_slot_group;
    wxWindow*                            m_nozzle_win;
    MaterialSlotWgt*                     m_current_slot;
    LayoutMode                           m_layout_mode;
    std::vector<wxPoint>                 m_slot_points;
    wxPoint                              m_nozzle_point;
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
    enum PaintMode { Transparent = 0, UnknowColor = 1, WhiteWithCircle = 2 };
    void set_color(const wxColour& color);
    wxColour& get_color();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    wxColour m_color;
    wxBitmap m_unknow_color;
    wxBitmap  m_transparent;
    wxBitmap  m_white_circle;
    PaintMode m_mode;
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
    enum ButtonState { Normal = 0, Hovered = 1, Pressed = 2, Inavaliable };//Inavaliable仅用于设置颜色
    void set_bitmap(const wxBitmap& bitmap);
    void set_state_color(const wxColour& color, ButtonState state);
    void set_radius(double radius);
    void set_state(ButtonState state);

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
    wxColour    m_inavaliable_color;
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
    void paintEvent(wxPaintEvent& event);

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
    static const char* color_lib[24];
};

class CustomOwnerDrawnComboBox : public wxOwnerDrawnComboBox
{
public:
    CustomOwnerDrawnComboBox(wxWindow*          parent,
                             wxWindowID         id,
                             const wxString&    value,
                             const wxPoint&     pos,
                             const wxSize&      size,
                             int                n,
                             const wxString     choices[],
                             long               style     = 0,
                             const wxValidator& validator = wxDefaultValidator,
                             const wxString&    name      = wxASCII_STR(wxComboBoxNameStr));

protected:
    virtual wxCoord OnMeasureItem(size_t item) const override;
    virtual void    OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const override;
    void            paintEvent(wxPaintEvent& event);

private:
    void connectEvent();
    void OnMouseMove(wxMouseEvent& event);
    void OnDropdown(wxCommandEvent& event);
    void OnCloseUp(wxCommandEvent& event);

private:
    wxBitmap m_up;
    wxBitmap m_down;
    int m_hover_item;
    int m_isExpanded;
};

class MaterialDialog : public wxDialog
{
public:
    MaterialDialog(wxWindow*       parent,
                   wxWindowID      id,
                   const wxString& title,
                   const int&      state,
                   const wxPoint&  pos   = wxDefaultPosition,
                   const wxSize&   size  = wxDefaultSize,
                   long            style = wxDEFAULT_DIALOG_STYLE,
                   const wxString& name  = wxASCII_STR(wxDialogNameStr));
    ~MaterialDialog();
    enum InfoState { NameKnown = 1, ColorKnown = 1 << 1 };
    static wxPoint calculate_pop_position(const wxPoint& point, const wxSize& size);
    void           set_material_name(const wxString& name);
    void           set_material_color(const wxColour& color);
    wxColour&      get_material_color();
    wxString&      get_material_name();
    int            get_info_state();
    void           set_info_state(int state);
    void           set_color_button_color(const wxColour& color);
    void           set_combobox_text(const wxString& name);

protected:
    void resizeEvent(wxSizeEvent& event);
    void paintEvent(wxPaintEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_color_btn_clicked(wxCommandEvent& event);
    void on_comboBox_selected(wxCommandEvent& event);
    void init_comboBox();
    void update_ok_state();

private:
    wxStaticText* m_type_lab;
    wxStaticText* m_color_lab;

    CustomOwnerDrawnComboBox* m_comboBox;
    ColorButton*  m_color_btn;

    RoundedButton* m_OK;
    RoundedButton* m_cancel;

    wxColour      m_material_color;
    wxString      m_material_name;
    int           m_state;
    static std::vector<wxString> m_options;
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
    void OnMouseDown(wxMouseEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_supply_wire_clicked(wxCommandEvent& event);
    void on_withdrawn_wire_clicked(wxCommandEvent& event);
    void on_recognized_clicked(wxCommandEvent& event);
    void on_unrecognized_clicked(wxCommandEvent& event);
    void on_slot_area_clicked(wxCommandEvent& event);
    void on_tips_area_cancel_clicked(wxCommandEvent& event);


private:
    TipsArea*                     m_tips_area;
    RoundedButton*                m_supply_wire;
    RoundedButton*                m_withdrawn_wire;
    IdentifyButton*               m_recognized_btn;
    IdentifyButton*               m_unrecognized_btn;
    MaterialSlotArea*             m_material_slot;
    
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
