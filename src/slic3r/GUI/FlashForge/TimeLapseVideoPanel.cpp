#include "TimeLapseVideoPanel.hpp"
#include <wx/dirdlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include "slic3r/GUI/FFUtils.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/FlashForge/MultiComMgr.hpp"

namespace Slic3r { namespace GUI {

wxDEFINE_EVENT(EVT_TIME_LAPSE_VIDEO_SELECT_TOGGLED, wxCommandEvent);

TimeLapseVideoItem::TimeLapseVideoItem(wxWindow *parent)
    : wxPanel(parent)
    , m_videoWidth(0)
    , m_videoHeight(0)
    , m_select(false)
    , m_hoverSelRect(false)
    , m_pressSelRect(false)
    , m_selRect(FromDIP(5), FromDIP(5), FromDIP(16), FromDIP(16))
    , m_selOnNormalIcon(this, "time_lapse_video_check_on", 16)
    , m_selOnHoverIcon(this, "time_lapse_video_check_on_hover", 16)
    , m_selOffNormalIcon(this, "time_lapse_video_check_off", 16)
    , m_selOffHoverIcon(this, "time_lapse_video_check_off_hover", 16)
{
    SetMinSize(wxSize(FromDIP(124), FromDIP(96)));
    SetMaxSize(wxSize(FromDIP(124), FromDIP(96)));

    Bind(wxEVT_PAINT, &TimeLapseVideoItem::onPaint, this);
    Bind(wxEVT_LEAVE_WINDOW, &TimeLapseVideoItem::onLeave, this);
    Bind(wxEVT_MOTION, &TimeLapseVideoItem::onMotion, this);
    Bind(wxEVT_LEFT_DOWN, &TimeLapseVideoItem::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &TimeLapseVideoItem::onLeftUp, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &TimeLapseVideoItem::onMouseCaptureLost, this);
}

void TimeLapseVideoItem::setData(const fnet_time_lapse_video_data_t &videoData)
{
    m_jobId = videoData.jobId;
    m_fileName = wxString::FromUTF8(videoData.fileName);
    m_videoUrl = videoData.videoUrl;
    m_videoWidth = videoData.width;
    m_videoHeight = videoData.height;
    Refresh();
    Update();
}

void TimeLapseVideoItem::onPaint(wxPaintEvent &event)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    wxSize size = GetSize();
    int imgHeight = size.GetHeight() * 0.73;
    gc->SetPen(wxColour("#e3e2e2"));
    gc->SetBrush(wxColour("#e3e2e2"));
    gc->DrawRectangle(0, 0, size.GetWidth(), imgHeight);

    wxBitmap *bmp = nullptr;
    if (m_select) {
        bmp = m_hoverSelRect ? &m_selOnHoverIcon.bmp() : &m_selOnNormalIcon.bmp();
    } else {
        bmp = m_hoverSelRect ? &m_selOffHoverIcon.bmp() : &m_selOffNormalIcon.bmp();
    }
    gc->DrawBitmap(*bmp, m_selRect.x, m_selRect.y, m_selRect.width, m_selRect.height);

    wxString elidedText = FFUtils::elideString(this, m_fileName, size.GetWidth());
    wxSize textSize = dc.GetTextExtent(elidedText);
    int textLineHeight = size.y - imgHeight - textSize.y;
    dc.DrawText(elidedText, (size.x - textSize.x) / 2, imgHeight + textLineHeight / 2);
}

void TimeLapseVideoItem::onLeave(wxEvent &event)
{
    event.Skip();
    if (m_hoverSelRect) {
        m_hoverSelRect = false;
        Refresh();
        Update();
    }
}

void TimeLapseVideoItem::onMotion(wxMouseEvent &event)
{
    event.Skip();
    bool hoverSelRect = m_selRect.Contains(event.GetPosition());
    if (hoverSelRect != m_hoverSelRect) {
        m_hoverSelRect = hoverSelRect;
        Refresh();
        Update();
    }
}

void TimeLapseVideoItem::onLeftDown(wxMouseEvent &event)
{
    event.Skip();
    m_pressSelRect = m_selRect.Contains(event.GetPosition());
    if (!HasCapture()) {
        CaptureMouse();
    }
}

void TimeLapseVideoItem::onLeftUp(wxMouseEvent &event)
{
    event.Skip();
    if (m_pressSelRect && m_selRect.Contains(event.GetPosition())) {
        m_select = !m_select;
        Refresh();
        Update();
        QueueEvent(new wxCommandEvent(EVT_TIME_LAPSE_VIDEO_SELECT_TOGGLED));
    }
    m_pressSelRect = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void TimeLapseVideoItem::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
    event.Skip();
    if (m_pressSelRect) {
        m_pressSelRect = false;
        Refresh();
        Update();
    }
}

TimeLapseVideoPanel::TimeLapseVideoPanel(wxWindow *parent)
    : wxPanel(parent)
    , m_comId(ComInvalidId)
    , m_downloadTool(5, 30000)
    , m_downloadingComId(ComInvalidId)
{
    SetBackgroundColour(*wxWHITE);
    SetDoubleBuffered(true);
    SetMinSize(wxSize(FromDIP(450), FromDIP(411)));
    SetMaxSize(wxSize(FromDIP(450), FromDIP(411)));

    m_scr = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_scr->SetMinSize(wxSize(FromDIP(450), FromDIP(351)));
    m_scr->SetMaxSize(wxSize(FromDIP(450), FromDIP(351)));
    m_scr->SetScrollRate(0, 30);

    m_deleteBtn = new FFButton(this);
    m_deleteBtn->SetFontColor("#419488");
    m_deleteBtn->SetBorderColor("#419488");
    m_deleteBtn->SetFontHoverColor("#65A79E");
    m_deleteBtn->SetBorderHoverColor("#65A79E");
    m_deleteBtn->SetFontPressColor("#1A8676");
    m_deleteBtn->SetBorderPressColor("#1A8676");
    m_deleteBtn->SetFontDisableColor("#dddddd");
    m_deleteBtn->SetBGDisableColor(*wxWHITE);
    m_deleteBtn->SetBorderDisableColor("#dddddd");
    m_deleteBtn->SetLabel(_L("Delete"), FromDIP(80), FromDIP(32));
    m_deleteBtn->Enable(false);

    m_downloadBtn = new FFButton(this);
    m_downloadBtn->SetFontColor(*wxWHITE);
    m_downloadBtn->SetBGColor("#419488");
    m_downloadBtn->SetBorderColor("#419488");
    m_downloadBtn->SetFontHoverColor(*wxWHITE);
    m_downloadBtn->SetBGHoverColor("#65A79E");
    m_downloadBtn->SetBorderHoverColor("#65A79E");
    m_downloadBtn->SetFontPressColor(*wxWHITE);
    m_downloadBtn->SetBGPressColor("#1A8676");
    m_downloadBtn->SetBorderPressColor("#1A8676");
    m_downloadBtn->SetFontDisableColor(*wxWHITE);
    m_downloadBtn->SetBGDisableColor("#dddddd");
    m_downloadBtn->SetBorderDisableColor("#dddddd");
    m_downloadBtn->SetLabel(_L("Download"), FromDIP(80), FromDIP(32));
    m_downloadBtn->Enable(false);

    m_btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnSizer->AddStretchSpacer(1);
    m_btnSizer->Add(m_deleteBtn);
    m_btnSizer->AddSpacer(FromDIP(16));
    m_btnSizer->Add(m_downloadBtn);

    m_itemSizer = new wxGridSizer(3, FromDIP(8), FromDIP(16));
    wxSizer *scrSizer = new wxBoxSizer(wxHORIZONTAL);
    scrSizer->AddStretchSpacer(1);
    scrSizer->Add(m_itemSizer, 0, wxTOP, FromDIP(16));
    scrSizer->AddStretchSpacer(1);
    m_scr->SetSizer(scrSizer);

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_scr);
    sizer->AddStretchSpacer(1);
    sizer->Add(m_btnSizer, 0, wxEXPAND | wxRIGHT, FromDIP(16));
    sizer->AddStretchSpacer(1);
    SetSizer(sizer);

    m_deleteBtn->Bind(wxEVT_BUTTON, &TimeLapseVideoPanel::onDelete, this);
    m_downloadBtn->Bind(wxEVT_BUTTON, &TimeLapseVideoPanel::onDownload, this);
    m_downloadTool.Bind(EVT_FF_DOWNLOAD_FINISHED, &TimeLapseVideoPanel::onDownloadFinish, this);
    MultiComMgr::inst()->Bind(COM_GET_TIME_LAPSE_VIDEO_LIST_EVENT, &TimeLapseVideoPanel::onGetVideoList, this);
    MultiComMgr::inst()->Bind(COM_DELETE_TIME_LAPSE_VIDEO_EVENT, &TimeLapseVideoPanel::onDeleteFinish, this);
}

TimeLapseVideoPanel::~TimeLapseVideoPanel()
{
    m_downloadTool.wait(true);
    for (int taskId : m_downloadingTaskSet) {
        wxRemoveFile(m_downloadDataMap.at(taskId).tmpSaveName);
    }
}

void TimeLapseVideoPanel::setComId(com_id_t comId)
{
    if (comId != m_comId) {
        m_comId = comId;
        m_itemSizer->Clear(true);
        m_deleteBtn->Enable(false);
        m_downloadBtn->Enable(false);
    }
}

void TimeLapseVideoPanel::updateVideoList()
{
    MultiComMgr::inst()->putCommand(m_comId, new ComGetTimeLapseVideoList);
    m_itemSizer->Clear(true);
    m_deleteBtn->Enable(false);
    m_downloadBtn->Enable(false);
}

void TimeLapseVideoPanel::onGetVideoList(ComGetTimeLapseVideoListEvent &event)
{
    event.Skip();
    if (event.id != m_comId) {
        return;
    }
    Freeze();
    m_itemSizer->Clear(true);
    m_deleteBtn->Enable(false);
    m_downloadBtn->Enable(false);
    auto &videoList = MultiComMgr::inst()->devData(m_comId).wanTimeLapseVideoList;
    for (int i = 0; i < videoList.videoCnt; ++i) {
        if (i < m_itemSizer->GetItemCount()) {
            TimeLapseVideoItem *item = (TimeLapseVideoItem *)m_itemSizer->GetItem(i)->GetWindow();
            item->setData(videoList.videoDatas[i]);
        } else {
            TimeLapseVideoItem *item = new TimeLapseVideoItem(m_scr);
            item->setData(videoList.videoDatas[i]);
            item->Bind(EVT_TIME_LAPSE_VIDEO_SELECT_TOGGLED, &TimeLapseVideoPanel::onSelectChange, this);
            m_itemSizer->Add(item, 0, wxALIGN_CENTER);
        }
    }
    while (m_itemSizer->GetItemCount() > videoList.videoCnt) {
        int backIdx = m_itemSizer->GetItemCount() - 1;
        TimeLapseVideoItem *item = (TimeLapseVideoItem *)m_itemSizer->GetItem(backIdx)->GetWindow();
        m_itemSizer->Remove(backIdx);
        delete item;
    }
    m_scr->SetVirtualSize(-1, m_itemSizer->GetMinSize().y);
    Thaw();
}

void TimeLapseVideoPanel::onSelectChange(wxCommandEvent &event)
{
    event.Skip();
    updateButtonState();
}

void TimeLapseVideoPanel::onDelete(wxCommandEvent &event)
{
    event.Skip();
    std::vector<std::string> jobIds;
    for (int i = 0; i < m_itemSizer->GetItemCount(); ++i) {
        TimeLapseVideoItem *item = (TimeLapseVideoItem *)m_itemSizer->GetItem(i)->GetWindow();
        if (item->getSelect()) {
            jobIds.push_back(item->getJobId());
        }
    }
    MultiComMgr::inst()->putCommand(m_comId, new ComDeleteTimeLapseVideo(jobIds));
}

void TimeLapseVideoPanel::onDeleteFinish(ComDeleteTimeLapseVideoEvent &event)
{
    event.Skip();
    MultiComMgr::inst()->putCommand(m_comId, new ComGetTimeLapseVideoList);
}

void TimeLapseVideoPanel::onDownload(wxCommandEvent &event)
{
    event.Skip();
    wxDirDialog saveDlg(wxGetApp().mainframe);
    if (saveDlg.ShowModal() != wxID_OK) {
        return;
    }
    m_downloadSaveDir = saveDlg.GetPath();
    m_downloadDataMap.clear();
    for (int i = 0; i < m_itemSizer->GetItemCount(); ++i) {
        TimeLapseVideoItem *item = (TimeLapseVideoItem *)m_itemSizer->GetItem(i)->GetWindow();
        if (item->getSelect()) {
            wxString fileName = item->getFileName();
            wxString tmpSaveName = getSaveName(m_downloadSaveDir, fileName, true);
            int taskId = m_downloadTool.downloadDisk(item->getVideoUrl(), tmpSaveName, ComTimeoutWanB, 600000);
            download_data_t downloadData = { i, false, tmpSaveName, fileName};
            m_downloadDataMap.emplace(taskId, downloadData);
            m_downloadingTaskSet.emplace(taskId);
        }
    }
    m_downloadingComId = m_comId;
    m_deleteBtn->Enable(false);
    m_downloadBtn->Enable(false);
    m_downloadBtn->SetLabel(_L("Downloading"), FromDIP(80), FromDIP(32));
    m_btnSizer->Layout();
}

void TimeLapseVideoPanel::onDownloadFinish(FFDownloadFinishedEvent &event)
{
    event.Skip();
    download_data_t &downloadData = m_downloadDataMap.at(event.taskId);
    if (event.succeed) {
        wxString saveName = getSaveName(m_downloadSaveDir, downloadData.fileName, false);
        downloadData.succeed = wxRenameFile(downloadData.tmpSaveName, saveName);
    } else {
        wxRemoveFile(downloadData.tmpSaveName);
        downloadData.succeed = false;
    }
    m_downloadingTaskSet.erase(event.taskId);
    if (m_downloadingTaskSet.empty()) {
        m_downloadDataMap.clear();
        m_downloadBtn->SetLabel(_L("Download"), FromDIP(80), FromDIP(32));
        m_btnSizer->Layout();
        updateButtonState();
    }
}

void TimeLapseVideoPanel::updateButtonState()
{
    bool hasSelecte = false;
    for (int i = 0; i < m_itemSizer->GetItemCount(); ++i) {
        TimeLapseVideoItem *item = (TimeLapseVideoItem *)m_itemSizer->GetItem(i)->GetWindow();
        if (item->getSelect()) {
            hasSelecte = true;
            break;
        }
    }
    m_deleteBtn->Enable(hasSelecte && (m_downloadingTaskSet.empty() || m_downloadingComId != m_comId));
    m_downloadBtn->Enable(hasSelecte && m_downloadingTaskSet.empty());
}

wxString TimeLapseVideoPanel::getSaveName(const wxString &dirName, const wxString &fileName, bool tmp)
{
    wxString tmpFileName;
    wxString forbiddenChars = wxFileName::GetForbiddenChars();
    for (int i = 0; i < fileName.size(); ++i) {
        if (forbiddenChars.find(fileName[i]) == wxNOT_FOUND) {
            tmpFileName.append(fileName[i]);
        }
    }
    wxString baseName;
    wxString extension;
    wxFileName::SplitPath(tmpFileName, nullptr, &baseName, &extension);
    wxString saveName = dirName + "/" + tmpFileName;
    if (tmp) {
        saveName += ".ffdownload";
    }
    for (int i = 1; saveName.empty() || wxFileExists(saveName); ++i) {
        saveName = wxString::Format("%s/%s(%d).%s", dirName, baseName, i, extension);
        if (tmp) {
            saveName += ".ffdownload";
        }
    }
    return saveName;
}

}} // namespace Slic3r::GUI
