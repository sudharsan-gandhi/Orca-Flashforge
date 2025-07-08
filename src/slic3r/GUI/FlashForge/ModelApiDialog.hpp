#ifndef slic3r_GUI_TripoModelApiDialog_h_
#define slic3r_GUI_TripoModelApiDialog_h_

#include <wx/wx.h>
#include <wx/webview.h>
#include <wx/graphics.h>
#include "slic3r/GUI/GUI_Utils.hpp"
#include "slic3r/GUI/Widgets/ProgressBar.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/FlashForge/FFDownloadTool.hpp"
#include "slic3r/GUI/MsgDialog.hpp"
//#include "slic3r/GUI/ConvertModel/ConvertModel.hpp"
#include "libslic3r/miniz_extension.hpp"
#include "slic3r/GUI/FlashForge/FFTitleLessDialog.hpp"

namespace Slic3r { namespace GUI {

//wxDECLARE_EVENT(EVT_UPDATE_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_CLOSE_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_FINISH_CONVERT, wxCommandEvent);
//wxDECLARE_EVENT(EVT_INIT_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_CANCEL_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_START_TIMER, wxCommandEvent);
//wxDECLARE_EVENT(EVT_STOP_TIMER, wxCommandEvent);
//
//class ApiProgressDialog;
//class GenerateApiTask : public wxEvtHandler
//{
//public:
//    GenerateApiTask(ApiProgressDialog* parent = nullptr);
//    bool         unzip_to_obj_model(const std::string& zip_file, std::string& obj_file);
//    void         convert_obj_model_and_load(const std::string& obj_file);
//    void         error_msg(const wxString& msg);
//    
//protected:
//    FFDownloadTool m_download_tool;
//    std::string    m_download_path;
//    wxTimer*       m_timer{nullptr};
//    int                m_pollingCount = 0;
//    ApiProgressDialog* m_parent;
//};
//
//class HunyuanApiTask : public GenerateApiTask, public std::enable_shared_from_this<HunyuanApiTask>
//{
//public:
//    HunyuanApiTask(ApiProgressDialog* parent = nullptr);
//    void submit_image_action(wxString& main_image, wxString& left_image, wxString& right_image, wxString& back_image);
//    void submit_text_action(wxString& text);
//
//private:
//    void generate_action(wxTimerEvent& event);
//    void cancel_task(wxCommandEvent& event);
//    wxString     m_job_id;
//    bool        isFirstGenerate{true};
//    enum RunState { WAIT = 0, RUN, FAILED, SUCCESS, CANCELED };
//};
//
//class ApiProgressDialog : public wxDialog
//{
//public:
//    ApiProgressDialog(wxWindow* parent = NULL);
//    void sendEvent(int eventType, const wxString msg = "", int progress = 0);
//    void update(const wxString& msg, int progress);
//    void setTaskHandler(const std::shared_ptr<GenerateApiTask>& evtHandler);
//    const std::shared_ptr<GenerateApiTask>& getTaskHandler();
//    ~ApiProgressDialog();
//
//private:
//    Label*       m_text{nullptr};
//    ProgressBar* m_progress{nullptr};
//    FFButton*    m_cancelBtn{nullptr};
//    std::shared_ptr<GenerateApiTask> m_taskHandler;
//};
//

//
//class ModelApiPanel : public wxPanel
//{
//public:
//    ModelApiPanel(wxWindow* parent = nullptr);
//    
//};
//
////class TripoModelApiPanel : public ModelApiPanel 
////{
////public:
////    TripoModelApiPanel(wxWindow* parent = nullptr);
////    void ButtonClicked(wxCommandEvent& event);
////
////protected:
////    void finish_event_action(wxCommandEvent& event);
////
////private:
////    wxTextCtrl*    m_key_ctrl;
////    wxTextCtrl*    m_text_ctrl;
////    wxStaticText*  m_wallet_text;
////    std::string m_key;
////    std::string m_url_task;
////    std::string m_url_upload;
////    std::string m_url_wallet;
////    std::string m_image_extention;
////
////private:
////    void image_upload();
////    void generate_text_task(std::string body, unsigned status);
////    void generate_image_task(std::string body, unsigned status);
////    void run_task_and_convert(std::string body, unsigned status);
////    void download_obj_model(std::string body, unsigned status);
////    bool polling_task_for_tripo(const std::string& task_id, std::string& download_link);
////    int  get_tripo_wallet();
////};
//
//class HunYuanModelApiPanel : public ModelApiPanel
//{
//public:
//    HunYuanModelApiPanel(wxWindow* parent = nullptr);
//    ~HunYuanModelApiPanel();
//    void ButtonClicked(wxCommandEvent& event);
//    wxString judgeTransImage(wxString& path, wxString& prefixError);
//
//private:
//    wxTextCtrl* m_text_ctrl;
//    wxPanel *   m_text_panel, *m_image_panel;
//    ImageUploadPanel *m_main_image, *m_left_image, *m_right_image, *m_back_image;
//    wxRadioButton *m_radio_text_btn, *m_radio_image_btn; 
//    wxBoxSizer*    m_sizer;
//    void        radio_button_selected(wxCommandEvent& event);
//};
//

wxDECLARE_EVENT(EVT_LOADED_IMAGE, wxCommandEvent);

class ImageUploadPanel : public wxPanel
{
public: 
    ImageUploadPanel(wxWindow* parent);
    wxString getPath();

private:
    wxString m_path;
    wxImage m_img;
    ScalableBitmap m_upload_icon, m_delete_icon;

private:
    void onPaint(wxPaintEvent& event);
    void onLeftDown(wxMouseEvent& event);
    void onLeftUp(wxMouseEvent& event);
    void onMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    bool m_isPressed;
    bool m_isHovered;
};

class ModelApiDialog : public FFTitleLessDialog
{
public:
    ModelApiDialog(wxWindow* parent = nullptr);
    void drawBackground(wxPaintDC& dc, wxGraphicsContext* gc);

private:
    std::unordered_map<std::string, ScalableBitmap> m_bmp_map;
    wxString m_cost_text;
    wxString m_score_text;
    ImageUploadPanel*                               m_image_panel{nullptr};
    void drawCenterText(wxGraphicsContext* gc, wxString& str, int height, wxFont& font, wxColour color, wxString iconName = "");
};

}} // namespace Slic3r::GUI

#endif