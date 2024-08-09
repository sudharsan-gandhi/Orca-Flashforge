#include "AmsPrintFileDlg.hpp"
#include "slic3r/GUI/FlashForge/MultiComMgr.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"

namespace Slic3r { namespace GUI {
    
AmsPrintFileDlg::AmsPrintFileDlg(wxWindow *parent)
    : TitleDialog(parent, "FF_TAG_AMS_PRINT_FILE")
    , m_amsTipWnd(new AmsTipWnd(this))
{
    SetDoubleBuffered(true);
    SetBackgroundColour(*wxWHITE);
    SetForegroundColour(wxColour("#333333"));
    SetFont(wxGetApp().normal_font());

    // top panel
    m_topPnl = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBitmap timeBmp = create_scaled_bitmap("ff_print_time", this, 14);
    wxStaticBitmap *timeWxBmp = new wxStaticBitmap(m_topPnl, wxID_ANY, timeBmp, wxDefaultPosition, wxSize(FromDIP(16), FromDIP(16)));
    m_timeLbl = new wxStaticText(m_topPnl, wxID_ANY, wxEmptyString);

    wxBitmap weightBmp = create_scaled_bitmap("ff_print_weight", this, 14);
    wxStaticBitmap *weightWxBmp = new wxStaticBitmap(m_topPnl, wxID_ANY, weightBmp, wxDefaultPosition, wxSize(FromDIP(16), FromDIP(16)));
    m_weightLbl = new wxStaticText(m_topPnl, wxID_ANY, wxEmptyString);

    wxBoxSizer *timeWeightSizer = new wxBoxSizer(wxHORIZONTAL);
    timeWeightSizer->Add(timeWxBmp, 1, wxEXPAND | wxALL, FromDIP(5));
    timeWeightSizer->Add(m_timeLbl, 0, wxALL, FromDIP(5));
    timeWeightSizer->Add(0, 0, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(30));
    timeWeightSizer->Add(weightWxBmp, 1, wxEXPAND | wxALL, FromDIP(5));
    timeWeightSizer->Add(m_weightLbl, 0, wxALL, FromDIP(5));

    m_nameLbl = new wxStaticText(m_topPnl, wxID_ANY, wxEmptyString);
    wxBoxSizer* rightTopSizer = new wxBoxSizer(wxVERTICAL);
    rightTopSizer->Add(m_nameLbl, 0, wxALIGN_LEFT | wxALIGN_BOTTOM, FromDIP(5));
    rightTopSizer->AddSpacer(FromDIP(5));
    rightTopSizer->Add(timeWeightSizer, 0, wxALIGN_LEFT | wxALIGN_TOP, 0);

    m_thumbWxBmp = new wxStaticBitmap(m_topPnl, wxID_ANY, wxNullBitmap);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->Add(m_thumbWxBmp, 0, wxALIGN_CENTER_VERTICAL, 0);
    topSizer->AddSpacer(FromDIP(5));
    topSizer->Add(rightTopSizer, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT, 0);
    m_topPnl->SetSizer(topSizer);

    // material panel
    m_materialSizer = new wxGridSizer(0, 4, FromDIP(16), FromDIP(24));
    m_materialPnl = new wxPanel(this, wxID_ANY);
    m_materialPnl->SetSizer(m_materialSizer);

    m_amsTipLbl = new wxStaticText(this, wxID_ANY, _L("FF_TAG_AMS_TIP_TEXT"));
    m_amsTipLbl->SetForegroundColour(wxColour("#F59A23"));

    // print config
    m_levelChk = new FFCheckBox(this);
    m_levelChk->SetValue(false);
    m_levelChk->Bind(wxEVT_TOGGLEBUTTON, &AmsPrintFileDlg::onLevellingStateChanged,this);
    m_levelLbl = new wxStaticText(this, wxID_ANY, _L("Levelling"));

    m_flowCalibrationChk = new FFCheckBox(this);
    m_flowCalibrationChk->SetValue(false);
    m_flowCalibrationChk->Bind(wxEVT_TOGGLEBUTTON, &AmsPrintFileDlg::onFlowCalibrationStateChanged, this);
    m_flowCalibrationLbl = new wxStaticText(this, wxID_ANY, _L("Flow Calibration"));

    m_enableAmsChk = new FFCheckBox(this);
    m_enableAmsChk->SetValue(false);
    m_enableAmsLbl = new wxStaticText(this, wxID_ANY, _L("Enable FFM"));

    wxBitmap amsTipBmp = create_scaled_bitmap("ams_tutorial_icon", this, 16);
    m_amsTipWxBmp = new wxStaticBitmap(this, wxID_ANY, amsTipBmp, wxDefaultPosition, wxSize(FromDIP(16), FromDIP(16)), 0);
    m_amsTipWxBmp->Bind(wxEVT_ENTER_WINDOW, &AmsPrintFileDlg::onEnterAmsTipWidget, this);
    m_amsTipWxBmp->Bind(wxEVT_LEAVE_WINDOW, &AmsPrintFileDlg::onEnterAmsTipWidget, this);

    auto printConfigSizer = new wxBoxSizer(wxHORIZONTAL);
    printConfigSizer->Add(m_levelChk, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->Add(m_levelLbl, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->AddStretchSpacer(1);
    printConfigSizer->Add(m_flowCalibrationChk, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->Add(m_flowCalibrationLbl, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->AddStretchSpacer(1);
    printConfigSizer->Add(m_enableAmsChk, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->Add(m_enableAmsLbl, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->Add(m_amsTipWxBmp, 0, wxLEFT | wxALIGN_LEFT, FromDIP(10));
    printConfigSizer->AddSpacer(FromDIP(10));

    // print button
    m_printBtn = new FFButton(this, wxID_ANY, _L("Print"), FromDIP(4), false);
    m_printBtn->SetFontColor(wxColour("#ffffff"));
    m_printBtn->SetFontHoverColor(wxColor("#ffffff"));
    m_printBtn->SetFontPressColor(wxColor("#ffffff"));
    m_printBtn->SetFontDisableColor(wxColor("#ffffff"));
    m_printBtn->SetBGColor(wxColour("#419488"));
    m_printBtn->SetBGHoverColor(wxColour("#65A79E"));
    m_printBtn->SetBGPressColor(wxColour("#1A8676"));
    m_printBtn->SetBGDisableColor(wxColour("#dddddd"));
    m_printBtn->SetSize(wxSize(FromDIP(101), FromDIP(44)));
    m_printBtn->SetMinSize(wxSize(FromDIP(101), FromDIP(44)));
    m_printBtn->SetMaxSize(wxSize(FromDIP(101), FromDIP(44)));

    // main sizer
    wxBoxSizer *mainSizer = MainSizer();
    mainSizer->AddSpacer(FromDIP(12));
    mainSizer->Add(m_topPnl, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT, FromDIP(30));
    mainSizer->AddSpacer(FromDIP(12));
    mainSizer->Add(m_materialPnl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(40));
    mainSizer->AddSpacer(FromDIP(22));
    mainSizer->Add(m_amsTipLbl, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(40));
    mainSizer->AddSpacer(FromDIP(22));
    mainSizer->Add(makeLineSpacer(), 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(30));
    mainSizer->AddSpacer(FromDIP(19));
    mainSizer->Add(printConfigSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(30));
    mainSizer->AddSpacer(FromDIP(19));
    mainSizer->Add(makeLineSpacer(), 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(30));
    mainSizer->AddSpacer(FromDIP(30));
    mainSizer->Add(m_printBtn, 0, wxEXPAND | wxALIGN_CENTER, FromDIP(40));
    mainSizer->AddSpacer(FromDIP(28));

    setupData();
    Centre(wxBOTH);
}

void AmsPrintFileDlg::setupData()
{
    // thumbnail
    Plater *plater = wxGetApp().plater();
    const ThumbnailData &thumbData = plater->get_partplate_list().get_curr_plate()->thumbnail_data;
    if (thumbData.is_valid()) {
        wxImage image(thumbData.width, thumbData.height);
        image.InitAlpha();
        for (unsigned int r = 0; r < thumbData.height; ++r) {
            unsigned int scanLine = (thumbData.height - 1 - r) * thumbData.width;
            for (unsigned int c = 0; c < thumbData.width; ++c) {
                unsigned char *px = (unsigned char *)thumbData.pixels.data() + 4 * (scanLine + c);
                image.SetRGB(c, r, px[0], px[1], px[2]);
                image.SetAlpha(c, r, px[3]);
            }
        }
        m_thumbWxBmp->SetBitmap(image.Rescale(FromDIP(108), FromDIP(117)));
    }

    // file name
    wxString fileName = plater->get_export_gcode_filename("", true, true);
    if (fileName.empty()) {
        fileName = _L("Untitled");
    }
    m_nameLbl->SetLabelText(fileName);

    // time
    wxString time;
    const PrintStatistics &printInfo = plater->get_partplate_list().get_current_fff_print().print_statistics();
    PartPlate *plate = plater->get_partplate_list().get_curr_plate();
    if (plate != nullptr) {
        if (plate->get_slice_result()) {
            time = wxString::Format("%s", short_time(get_time_dhms(plate->get_slice_result()->print_statistics.modes[0].time)));
        }
    }
    m_timeLbl->SetLabel(time);

    // weight
    char weight[64];
    if (wxGetApp().app_config->get("use_inches") == "1") {
        ::sprintf(weight, "  %.2f oz", printInfo.total_weight * 0.035274);
    } else {
        ::sprintf(weight, "  %.2f g", printInfo.total_weight);
    }
    m_weightLbl->SetLabel(weight);
    
    // materials
    std::vector<std::string> materials;
    auto preset_bundle = wxGetApp().preset_bundle;
    for (auto &filamentName : preset_bundle->filament_presets) {
        for (auto iter = preset_bundle->filaments.lbegin(); iter != preset_bundle->filaments.end(); iter++) {
            if (filamentName.compare(iter->name) == 0) {
                std::string displayFilamentType;
                iter->config.get_filament_type(displayFilamentType);
                materials.push_back(displayFilamentType);
            }
        }
    }
    BitmapCache bmpCache;
    m_materialSizer->Clear(true);
    std::vector<int> extruders = plater->get_partplate_list().get_curr_plate()->get_used_extruders();
    for (size_t i = 0; i < extruders.size(); ++i) {
        auto extruderIdx = extruders[i] - 1;
        if (extruderIdx < 0 || extruderIdx >= materials.size()) {
            continue;
        }
        unsigned char rgb[4];
        std::string color = preset_bundle->project_config.opt_string("filament_colour", (unsigned int)extruderIdx);
        bmpCache.parse_color4(color, rgb);

        wxColour colorRgb = wxColour((int)rgb[0], (int)rgb[1], (int)rgb[2], (int)rgb[3]);
        MaterialMapWgt* item = new MaterialMapWgt(m_materialPnl, colorRgb, materials[i]);
        m_materialSizer->Add(item);
    }
    m_materialSizer->SetCols(std::min((int)extruders.size(), 4));

    // levelling
    if (wxGetApp().app_config->get("levelling").empty()) {
        m_levelChk->SetValue(false);
    } else {
        m_levelChk->SetValue(wxGetApp().app_config->get("levelling") == "true");
    }

    // flow calibration
    if (wxGetApp().app_config->get("flowCalibration").empty()) {
        m_flowCalibrationChk->SetValue(false);
    } else {
        m_flowCalibrationChk->SetValue(wxGetApp().app_config->get("flowCalibration") == "true");
    }

    // layout/fit
    m_topPnl->Layout();
    m_topPnl->Fit();
    Layout();
    Fit();
    m_materialPnl->Layout();
    m_materialPnl->Fit();
    Layout();
    Fit();
}

void AmsPrintFileDlg::onLevellingStateChanged(wxCommandEvent& event)
{
    if (m_levelChk->GetValue()) {
        wxGetApp().app_config->set("levelling", "true");
    } else {
        wxGetApp().app_config->set("levelling", "false");
    }
    event.Skip();
}

void AmsPrintFileDlg::onFlowCalibrationStateChanged(wxCommandEvent& event)
{
    if (m_flowCalibrationChk->GetValue()) {
        wxGetApp().app_config->set("flowCalibration", "true");
    } else {
        wxGetApp().app_config->set("flowCalibration", "false");
    }
    event.Skip();
}

void AmsPrintFileDlg::onEnterAmsTipWidget(wxMouseEvent& event)
{
    if (event.Entering()) {
        int y = m_amsTipWxBmp->GetRect().height + FromDIP(1);
        wxPoint pos = m_amsTipWxBmp->ClientToScreen(wxPoint(0, y));
        m_amsTipWnd->Move(pos);
        m_amsTipWnd->Show(true);
    } else {
        m_amsTipWnd->Show(false);
    }
    event.Skip();
}

void AmsPrintFileDlg::onConnectionExit(ComConnectionExitEvent& event)
{
    event.Skip();
}

wxPanel *AmsPrintFileDlg::makeLineSpacer()
{
    wxPanel *pnl = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    pnl->SetForegroundColour(wxColour("#dddddd"));
    pnl->SetBackgroundColour(wxColour("#dddddd"));
    return pnl;
}

}} // namespace Slic3r::GUI
