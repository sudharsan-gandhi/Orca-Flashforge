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
#include "slic3r/GUI/ConvertModel/ConvertModel.hpp"
#include "libslic3r/miniz_extension.hpp"
#include "slic3r/GUI/FlashForge/FFTitleLessDialog.hpp"
#include "slic3r/GUI/FlashForge/FFTransientWindow.hpp"
#include "slic3r/GUI/FlashForge/PromoShareDlg.hpp"

namespace Slic3r { namespace GUI {

class FinishScoreEvent : public wxCommandEvent
{
public:
    FinishScoreEvent();
    int  curCostScore = 0;
    int  totalScore  = 0;
    std::string promoData;
};

wxDECLARE_EVENT(EVT_LOADED_IMAGE, wxCommandEvent);
wxDECLARE_EVENT(EVT_FINISH_TASK, wxCommandEvent);
wxDECLARE_EVENT(EVT_UPDATE_ICON, wxCommandEvent);
wxDECLARE_EVENT(EVT_ERROR_MSG, wxCommandEvent);
wxDECLARE_EVENT(EVT_FINISH_SCORE, FinishScoreEvent);


class ModelApiTask : public wxEvtHandler, public std::enable_shared_from_this<ModelApiTask>
{
public:
    ModelApiTask(wxEvtHandler* parent);
    void setThreadFunc(std::function<void()> func);
    wxSemaphore& Sem();
    void              safeFunc(std::function<void()> func);
    std::atomic_bool& FinishLoop();
    std::mutex&       Lock();
    wxEvtHandler*     Parent();
    void start();

private:
    std::function<void()> m_func;
    wxSemaphore           m_sem;
    std::atomic_bool      m_isFinish;
    std::mutex            m_lock;
    wxEvtHandler*         m_parent{nullptr};
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
    wxDialog*                   m_parent{nullptr};
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
    std::vector<std::string> m_vs;
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
    wxString getImage();
    void drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc);
    ~ModelApiDialog();

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
    void drawCenterText(wxBufferedPaintDC& dc, wxGraphicsContext* gc, const wxString& str, int height, wxFont& font, wxColour color, wxString iconName = "");
    void onLeftDown(wxMouseEvent& event);
    void onLeftUp(wxMouseEvent& event);
    void onMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void GenerateClicked();
    void RefreshScore(int cost, int total);
    int  m_cost_score = 0;
    int  m_total_score = 0;
    std::string m_promoData;
    bool m_isPressed{false};
    bool m_isGenerateHovered{false};
    bool m_isQuestionHovered{false};
};

class ApiSetStateEvent : public wxCommandEvent
{
public:
    ApiSetStateEvent();
    bool isQueuePanel{false};
    bool isShowQueue{false};
    int  remainCount = 0;
    int  totalCount  = 0;
};

class CompleteModelEvent : public wxCommandEvent
{
public:
    CompleteModelEvent();
    std::string path;
    int64_t     job_id;
};

class ChoiceColorEvent : public wxCommandEvent
{
public:
    ChoiceColorEvent();
    std::shared_ptr<convert_model_data_t> data;
    cvt_colors_t                          colors;
};

class CompleteConvertEvent : public wxCommandEvent
{
public:
    CompleteConvertEvent();
    cvt_colors_t                          colors;
    std::string                           obj_path;
    std::string                           mtl_path;
};

wxDECLARE_EVENT(EVT_OLD_TASK, wxCommandEvent);
wxDECLARE_EVENT(EVT_SET_ID, wxCommandEvent);
wxDECLARE_EVENT(EVT_SET_STATE, ApiSetStateEvent);
wxDECLARE_EVENT(EVT_COMPLETE_MODEL, CompleteModelEvent);
wxDECLARE_EVENT(EVT_CHOICE_COLOR, ChoiceColorEvent);
wxDECLARE_EVENT(EVT_COMPLETE_CONVERT, CompleteConvertEvent);
wxDECLARE_EVENT(EVT_REAL_CLOSE, wxCommandEvent);

class ModelGenerateDialog : public FFTitleLessDialog
{
public:
    ModelGenerateDialog(wxWindow* parent = nullptr);
    void            SetImgPath(wxString path);
    void drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc);
    std::shared_ptr<convert_model_data_t> getModelData();
    cvt_colors_t                          getCvtColors();
    std::string                           getDownloadPath();
    void            showCurState(bool isQueuePanel, bool isShowQueue = true);
    ~ModelGenerateDialog();

private:
    Label*          m_info_text{nullptr};
    Label*          m_queue_text{nullptr};
    wxPanel*        m_under_queue_sperator{nullptr};
    std::shared_ptr<ApiLoadingIcon> m_loadIcon;
    wxBoxSizer*     m_sizer{nullptr};
    std::shared_ptr<int64_t>        m_job_id;
    bool                            m_isOffline{false};
    bool                            m_isShowQueue{false};
    bool                            m_isQueuePanel{true};
    int             m_remainCount = 5, m_totalCount = 20;
    wxString                        m_img_path;
    std::string                     m_download_path;
    std::shared_ptr<ModelApiTask>   m_generateTask;
    std::shared_ptr<ModelApiTask>   m_abortTask;
    FFDownloadTool                  m_download_tool;
    std::shared_ptr<convert_model_data_t> m_modelData;
    cvt_colors_t                          m_cvt_colors;
 
};

class VerticalCenterTextCtrl : public wxTextCtrl
{
public:
    VerticalCenterTextCtrl(wxWindow* parent);

protected:
    void OnPaint(wxPaintEvent& event);
};

class ModelColorDialog : public FFTitleLessDialog
{
public:
    ModelColorDialog(wxWindow* parent = nullptr);
    void drawBackground(wxBufferedPaintDC& dc, wxGraphicsContext* gc);
    void changeColor(const cvt_colors_t& colors);
    void setModelData(const std::shared_ptr<convert_model_data_t>& data);
    void setDownloadFile(const std::string& path);
    ~ModelColorDialog();

private:
    FFButton*                             m_btn;
    std::shared_ptr<ModelApiTask> m_convertTask;
    std::vector<wxColour> m_color_grids;
    std::shared_ptr<convert_model_data_t> m_modelData;
    std::shared_ptr<ApiLoadingIcon> m_loadIcon;
    int                   m_last_color_count = 4;
    std::string                           m_filepath;
    VerticalCenterTextCtrl*               m_text_ctrl{nullptr};
};

}} // namespace Slic3r::GUI

#endif