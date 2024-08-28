#ifndef slic3r_GUI_MaterialStation_hpp_
#define slic3r_GUI_MaterialStation_hpp_
#include <wx/wx.h>
#include <wx/odcombo.h>
#include <wx/panel.h>
#include "../wxExtensions.hpp"
#include "MultiComDef.hpp"
#include "MultiComEvent.hpp"
#include <slic3r/GUI/I18N.hpp>

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
enum CurrentTask { 
    RequestSupplyWire = 0, 
    RequestWithdrawnWire = 1, 
    CancelRequest = 2, 
    NothingTask = 3 
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
    enum SlotType { Complete = 0, Unknown = 1, Empty = 2 };
    enum EditState { Normal = 0, Hover = 1, Press = 2 };

    MaterialInfo get_material_info();
    SlotType     get_slot_type();
    void         set_material_info(MaterialInfo& info);
    void         set_slot_type(SlotType type);
    void         set_edit_state(EditState type);
    

    bool     get_user_choices(); // 会弹出对话框
    bool     in_edit_scope(const wxPoint& pos);

protected:
    void connectEvent();
    void paintEvent(wxPaintEvent& event);
    void draw_edit_bmp(wxPaintDC& dc, wxBitmap& bitmap, wxPoint& point);

private:
    void render_name(const wxString& name, wxPaintDC& dc);
    std::pair<wxColour, wxBitmap*> compute_fore_color(const wxColour& color);

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

    ScalableBitmap m_unknow_name_bmp;

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
    void set_number(int number);

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
    enum PaintMode { NotProcess = 0, Processing = 1, Succeed = 2 };
    void set_state(PaintMode mode);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    ScalableBitmap m_process_num;
    ScalableBitmap m_not_process_num;
    ScalableBitmap m_succeed;
    PaintMode m_mode;
};


class MaterialSlotWgt : public wxWindow
{
public:
    MaterialSlotWgt(wxWindow* parent, 
                    wxWindowID id,
                    const int number,
                    const wxPoint&  pos   = wxDefaultPosition,
                    const wxSize&   size  = wxDefaultSize,
                    long            style = 0,
                    const wxString& name  = wxASCII_STR(wxPanelNameStr));
    ~MaterialSlotWgt();
    enum SlotWgtType { 
        MaterialStation = 0, IndependentMatl = 1
    };

    void set_selected(bool selected);
    void         set_slot_ID(int number);
    MaterialInfo get_material_info();
    int          get_slot_ID();
    void         set_material_info(MaterialInfo& info);
    void         set_slot_type(MaterialSlot::SlotType slot_type);
    void         set_conn_point(const wxPoint& point);
    wxPoint      get_conn_point();
    void         set_slot_wgt_type(SlotWgtType type);

    void     setCurId(int curId);
    //材料站可发出的命令
    bool send_config_command();
    bool start_supply_wire();
    bool start_withdrawn_wire();
    bool cancel_operation();
        
private:
    enum ComAction { SupplyWire = 0, WithdrawnWire = 1, CancelAction = 2 };
    void setup_layout(wxWindow* parent, const int& number);
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
    wxPoint       m_conn_point;

    SlotWgtType   m_slot_wgt_type;
    int           m_slot_ID;//从1开始
    com_id_t      m_cur_id; // ComInvalidId
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
    void set_wite_color(const wxColour& color);
    void set_conn_point(const wxPoint& point);
    wxPoint get_conn_point();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    ScalableBitmap m_bitmap;
    wxColour       m_wire_color;
    wxPoint        m_conn_point;
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
    enum class TipsAreaState : int { 
        Free = 0, 
        SupplyWire = 1, 
        WithdrawnWire = 2, 
        Canceling = 3, 
        Printing = 4, 
        Busy = 5,
        Undefine = 6
    };
    void Synchronize_printer_status(const com_dev_data_t& data);
    TipsAreaState get_tips_area_state();
    void          commit_task(CurrentTask task);
    CurrentTask   check_task();
    void          set_cancel_enable(bool enable);
    bool          is_heating();

private:
    void connectEvent();
    void on_cancel_clicked(wxCommandEvent& event);
    void prepare_layout(wxWindow* parent);
    void layout_tips_info();
    void layout_progress_status();
    void switch_layout_state(TipsAreaState state);

private:
    wxStaticText* m_tips_area_title;
    wxStaticText* m_tips_text;
    ProgressArea* m_progress;
    TipsAreaState m_state;
    int           m_hasMatlStation;
    int           m_stateAction;
    int           m_stateStep;
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
    enum class StateStep : int {
        NoProcessed       = 0,
        Heating           = 1,
        PushMaterials     = 2,
        WashOldMaterials  = 3,
        CutOffMaterials   = 4,
        PullBackMaterials = 5,
        Finish            = 6
    };
    enum class StateAction : int { 
        Free = 0, 
        SupplyWire = 1, 
        WithdrawnWire = 2, 
        Canceling = 3,
        Printing = 4,
        Busy = 5
    };
    void update_curr_task(StateAction action, StateStep step);
    void set_state_action(StateAction action);
    void set_state_step(StateStep step);
    void commit_task(CurrentTask task);
    CurrentTask check_task();
    void set_cancel_enable(bool enable);
    bool is_heating();

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void on_cancel_clicked(wxCommandEvent& event);

private:
    std::vector<ProgressNumber*>    m_btn_group;
    std::vector<wxStaticText*>      m_txt_group;
    RoundedButton*                  m_cancel_btn;
    StateStep                       m_state_step;
    StateAction                     m_state_action;
    CurrentTask                     m_curr_task;
    const std::vector<wxString>     m_supply_step = {_L("Heat up"), _L("Push filament"), _CTX("Purge old filament", "Flashforge"), _L("Complete")};
    const std::vector<wxString>     m_withdrawn_step = {_L("Heat up"), _L("Cut off filament"), _L("Retract filament"), _L("Complete")};
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
    static MaterialSlotArea* get_inst();
    void change_layout_mode(LayoutMode layout_model);
    MaterialSlotWgt*             get_radio_slot();
    void                         abandon_selected();
    std::vector<wxColour> get_all_material_color();
    void                         setCurId(int curId);
    MaterialSlotWgt*             get_curr_task_slot();   // 获取打印机currentSlot对应的槽
    MaterialSlotWgt*             get_supply_wire_slot(); //用于获取已进丝的料槽，传感器检测到就算
    bool                         is_executive_slot(MaterialSlotWgt* slot);
    void                         synchronize_printer_status(const com_dev_data_t& data);

    bool           start_supply_wire();
    bool           start_withdrawn_wire();
    bool           cancel_operation();

protected:
    void paintEvent(wxPaintEvent& event);
    void on_asides_mouse_down(wxMouseEvent& event);

private:
    void connectEvent();
    void calculate_connection_points(const wxPoint& slot_offset, const wxPoint& nozzle_offset);
    void prepare_layout(wxWindow* parent);
    void setup_layout_four(wxWindow* parent);
    void setup_layout_one(wxWindow* parent);
    void synchronize_matl_station(const com_dev_data_t& data);
    void synchronize_indep_matl(const com_dev_data_t& data);

    void slot_selected_event(wxCommandEvent& event);

private:
    std::vector<MaterialSlotWgt*>        m_material_slots_four;
    std::vector<MaterialSlotWgt*>        m_material_slot_one;
    std::vector<MaterialSlotWgt*>*       m_curr_slot_contaier;
    Nozzle*                              m_nozzle;
    wxWindow*                            m_slot_group;
    wxWindow*                            m_nozzle_win;
    MaterialSlotWgt*                     m_radio_slot;//表示当前用户鼠标选中的槽
    LayoutMode                           m_layout_mode;
    static MaterialSlotArea*                    s_self;
    
    int                                  m_hasMatlStation;
    int                                  m_nozzle_has_wire; // 只表示喷嘴传感器感知的是否有料进入喷嘴
    int                                  m_currentSlot;
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
    enum PaintMode { Color = 0, UnknowColor = 1, WhiteWithCircle = 2 };
    void set_color(const wxColour& color);
    wxColour& get_color();

protected:
    void paintEvent(wxPaintEvent& event);

private:
    void connectEvent();

private:
    wxColour m_color;
    ScalableBitmap m_unknow_color;
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
    void set_bitmap(const ScalableBitmap& bitmap, const ScalableBitmap& unselect);
    void set_select_state(bool isSelected);

protected:
    void paintEvent(wxPaintEvent& event);

private:
    bool     m_isSelected;
    ScalableBitmap m_select_bitmap;
    ScalableBitmap m_unselect_bitmap;
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
    void            paint_expanded_border(wxPaintDC& dc, wxRect& rect);
    void            paint_collapse_border(wxPaintDC& dc, wxRect& rect);
    void            paintEvent(wxPaintEvent& event);

private:
    void connectEvent();
    void OnDropdown(wxCommandEvent& event);
    void OnCloseUp(wxCommandEvent& event);

private:
    wxBitmap m_up;
    wxBitmap m_down;
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
    std::vector<wxString> m_options = {_CTX("Unknown", "filament"), "PLA", "ABS", "PETG", "TPU", 
                                        "COPA", "PLA-CF", "ABS-CF", "PETG-CF", "PET-CF", "PA-CF", "PC-ABS"};
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
    void setCurId(int curId);

protected:
    void OnMouseDown(wxMouseEvent& event);

private:
    void setup_layout(wxWindow* parent);
    void connectEvent();
    void update_wire_btn_state();
    void update_cancel_btn_state();

    void on_supply_wire_clicked(wxCommandEvent& event);
    void on_withdrawn_wire_clicked(wxCommandEvent& event);

    void on_recognized_clicked(wxCommandEvent& event);
    void on_unrecognized_clicked(wxCommandEvent& event);
    void on_slot_area_clicked(wxCommandEvent& event);
    void on_tips_area_cancel_clicked(wxCommandEvent& event);

    void onComDevDetailUpdate(ComDevDetailUpdateEvent& event);

private:
    TipsArea*                     m_tips_area;
    RoundedButton*                m_supply_wire;
    RoundedButton*                m_withdrawn_wire;
    IdentifyButton*               m_recognized_btn;
    IdentifyButton*               m_unrecognized_btn;
    MaterialSlotArea*             m_material_slot;
    com_id_t                      m_cur_id;
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
    void     show_material_panel(bool isShow = true);
    void     setCurId(int curId);

private:
    wxPanel*       m_material_title;
    wxStaticText*  m_staticText_title;
    MaterialPanel* m_material_panel;
};

} // namespace GUI

} // namespace Slic3r

#endif
