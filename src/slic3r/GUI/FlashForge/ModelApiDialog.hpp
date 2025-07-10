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
#include "slic3r/GUI/FlashForge/FFTransientWindow.hpp"

namespace Slic3r { namespace GUI {

//wxDECLARE_EVENT(EVT_UPDATE_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_CLOSE_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_FINISH_CONVERT, wxCommandEvent);
//wxDECLARE_EVENT(EVT_INIT_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_CANCEL_PROGRESS, wxCommandEvent);
//wxDECLARE_EVENT(EVT_START_TIMER, wxCommandEvent);
//wxDECLARE_EVENT(EVT_STOP_TIMER, wxCommandEvent);
//
// 

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
wxDECLARE_EVENT(EVT_FINISH_TASK, wxCommandEvent);
wxDECLARE_EVENT(EVT_UPDATE_ICON, wxCommandEvent);


class ModelApiTask : public wxEvtHandler, public std::enable_shared_from_this<ModelApiTask>
{
public:
    ModelApiTask(std::function<void()> func);
    void start();

private:
    std::function<void()> m_func;
};

class ApiLoadingIcon : public wxEvtHandler
{
public:
    ApiLoadingIcon(wxDialog* parent);
    void paintInRect(wxGraphicsContext* gc, wxRect rect);
    void Loading(int interval);
    void End();
    bool isLoading();
    ~ApiLoadingIcon() { End(); }

private:
    void                        OnTimer(wxTimerEvent& event);
    int                         m_loadingIdx = 0;
    int                         m_loadingTime = 0;
    wxTimer* m_timer{nullptr};
    std::vector<ScalableBitmap> m_loadingIcons;
};

class QuestionDialog : public FFRoundedWindow
{
public:
    QuestionDialog(wxWindow* parent = nullptr);
};

class ImageUploadPanel : public wxPanel
{
public: 
    ImageUploadPanel(wxWindow* parent);
    bool     judgeTransImage(wxString& path);
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
    wxRect                                          m_generate_btn_rect;
    wxRect                                          m_question_link_rect;
    ImageUploadPanel*                               m_image_panel{nullptr};
    QuestionDialog*                                 m_question_dialog{nullptr};
    std::shared_ptr<ApiLoadingIcon>                                 m_loadIcon;
    std::shared_ptr<ModelApiTask>                                   m_loadTask;
    void drawCenterText(wxGraphicsContext* gc, wxString& str, int height, wxFont& font, wxColour color, wxString iconName = "");
    void onLeftDown(wxMouseEvent& event);
    void onLeftUp(wxMouseEvent& event);
    void onMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void GenerateClicked();
    bool m_isPressed;
    bool m_isGenerateHovered;
    bool m_isQuestionHovered;
};

class ModelGenerateDialog : public FFTitleLessDialog
{
public:
    ModelGenerateDialog(wxWindow* parent = nullptr);
    void drawBackground(wxPaintDC& dc, wxGraphicsContext* gc);
    void            showCurState(bool isQueue);

private:
    Label*          m_info_text{nullptr};
    Label*          m_queue_text{nullptr};
    wxPanel*        m_under_queue_sperator{nullptr};
    std::shared_ptr<ApiLoadingIcon> m_loadIcon;
    wxBoxSizer*     m_sizer{nullptr};
    int             m_remainCount = 5, m_totalCount = 20;
    
};

}} // namespace Slic3r::GUI

#endif