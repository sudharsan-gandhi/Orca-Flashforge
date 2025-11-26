#ifndef slic3r_FFWebViewPanel_hpp_
#define slic3r_FFWebViewPanel_hpp_

#include <cstdint>
#include <memory>
#include <vector>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/timer.h>
#include <nlohmann/json.hpp>
#include "slic3r/GUI/TitleDialog.hpp"
#include "slic3r/GUI/FlashForge/FFTextCtrl.hpp"
#include "slic3r/GUI/FlashForge/FFTransientWindow.hpp"
#include "slic3r/GUI/FlashForge/MultiComEvent.hpp"
#include "slic3r/GUI/Widgets/FFButton.hpp"
#include "slic3r/GUI/Widgets/WebView.hpp"
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class NavMoreMenu : public FFTransientWindow
{
public:
    NavMoreMenu(wxWindow *parent);

    void AddItem(const std::string &icon, int iconHeight, const wxString &text);

private:
    static const int IconSpace = 6;
    static const int ItemHeight = 45;

    void OnPaint(wxPaintEvent &evt);
    void OnLeftUp(wxMouseEvent &evt);
    void OnMotion(wxMouseEvent &evt);

private:
    int m_hoverItemIndex;
    std::vector<std::unique_ptr<ScalableBitmap>> m_iconBmps;
    std::vector<wxString> m_texts;
    std::vector<wxSize> m_textSizes;
};

class ReportOptionItem : public wxPanel
{
public:
    ReportOptionItem(wxWindow *parent, const wxString &text, int id);

    int GetId() const { return m_id; }
    bool IsSelected() const { return m_isSelected; }
    void SetSelected(bool isSelected);

private:
    const int Height = 32;
    const int Spacing = 16;
    const int IconSize = 16;

    void OnPaint(wxPaintEvent &evt);
    void OnLeftDown(wxMouseEvent &evt);
    void OnEnterWindow(wxMouseEvent &evt);
    void OnLeaveWindow(wxMouseEvent &evt);

private:
    ScalableBitmap m_selectedBmp;
    wxString       m_text;
    int            m_id;
    wxSize         m_textSize;
    bool           m_isHover;
    bool           m_isSelected;
};

class ReportWindow : public wxDialog
{
public:
    ReportWindow(wxWindow *parent, const nlohmann::json &data);

    bool isOk() const { return m_isOk; }
    wxString GetWindowTitle() const { return m_titleBar->GetTitle(); }

private:
    const int TextCtrlSpacing = 10;

    void Initialize(const nlohmann::json &data);
    void OnPaint(wxPaintEvent &evt);
    void OnSize(wxSizeEvent &evt);
    void OnItemSelected(wxCommandEvent &evt);
    void OnTextChanged(wxCommandEvent &evt);
    void OnReportButton(wxCommandEvent &evt);

private:
    TitleBar                      *m_titleBar;
    wxStaticText                  *m_reportTitleLbl;
    std::vector<ReportOptionItem*> m_optionItems;
    FFTextCtrl                    *m_textCtrl;
    wxPanel                       *m_textCtrlDummyPnl;
    FFButton                      *m_reportBtn;
    bool                           m_isOk;
    const int                      m_radius;
};

class ViewNowWindow : public FFRoundedWindow
{
public:
    ViewNowWindow(wxWindow *parent);

    void ShowAutoClose(int msTime);
    void SetTipText(const wxString &text);
    bool IsAutoCloseTimerRunning();

private:
    void OnViewNow(wxCommandEvent &evt);

private:
    FFButton *m_button;
    wxStaticText *m_addPrintListTipLbl;
    wxTimer m_timer;
};

class FFWebViewPanel : public wxPanel
{
public:
    FFWebViewPanel(wxWindow *parent);
    
    void RunScript(const wxString &jsStr);
    void SendRecentList(int images);
    void ShowModelDeatil(const std::string &data);
    bool ProcComBusGetRequest(const ComBusGetRequestEvent &evt);
    bool GetUserConfigData(bool &modelPersonalizedRecEnabled, wxString &modelPersonalizedRecText);

private:
    bool InitBrowser();
    void InitModelNav();
    void CheckGetUserConfig();
    void PostGetUserConfig();
    void SetupPrintListButton(bool printListAdded);
    void MoveViewNowWindow();
    void SyncModelAction(const std::string &action);
    void OnBackButton(wxCommandEvent &evt);
    void OnMoreButton(wxCommandEvent &evt);
    void OnPrintListButton(wxCommandEvent &evt);
    void OnMoreMenu(wxCommandEvent &evt);
    void OnReportButton(wxCommandEvent &evt);
    void OnViewNow(wxCommandEvent &evt);
    void OnMainNewWindow(wxWebViewEvent &evt);
    void OnMainScriptMessageReceived(wxWebViewEvent &evt);
    void OnModelNavigating(wxWebViewEvent &evt);
    void OnModelNewWindow(wxWebViewEvent &evt);
    void OnMainFrameIconize(wxIconizeEvent &evt);
    void OnMainFrameMove(wxMoveEvent &evt);
    void OnMainFrameSize(wxSizeEvent &evt);
    void OnComMaintain(ComWanDevMaintainEvent &evt);
    void OnComAddPrintListModel(ComBusRequestEvent &evt);
    void OnComRemovePrintListModel(ComBusRequestEvent &evt);
    void OnComReportModel(ComBusRequestEvent &evt);

private:
    wxPanel         *m_modelPnl;
    wxPanel         *m_modelNavPnl;
    FFPushButton    *m_navBackBtn;
    wxStaticText    *m_navDetailLbl;
    FFPushButton    *m_navMoreBtn;
    FFButton        *m_navPrintListBtn;
    NavMoreMenu     *m_navMoreMenu;
    ViewNowWindow   *m_viewNowWindow;
    wxWebView       *m_mainBrowser;
    wxWebView       *m_modelBrowser;
    std::string      m_modelId;
    wxString         m_homePageUrl;
    bool             m_printListAdded;
    wxString         m_viewNowTipText;
    nlohmann::json   m_reportConfig;
    nlohmann::json   m_userConfig;
    wxString         m_reportWndTitle;
    int              m_getUserConfigTryCnt;
    int64_t          m_getUserConfigReqId;
    int64_t          m_printListReqId;
    int64_t          m_reportReqId;
};

}} // namespace Slic3r::GUI

#endif
