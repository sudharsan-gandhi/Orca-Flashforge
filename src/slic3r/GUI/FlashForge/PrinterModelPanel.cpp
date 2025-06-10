#include "PrinterModelPanel.hpp"
#include <wx/dcgraph.h>
#include <wx/image.h>
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/Plater.hpp"

namespace Slic3r { namespace GUI {

PrinterModelPanel::PrinterModelPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
    , m_printerCmb(nullptr)
    , m_editBtn(nullptr)
    , m_connectionBtn(nullptr)
{
}

void PrinterModelPanel::setup(PlaterPresetComboBox *printerCmb, ScalableButton *editBtn, ScalableButton *connectionBtn)
{
    m_printerCmb = printerCmb;
    m_editBtn = editBtn;
    m_connectionBtn = connectionBtn;

    SetMinSize(wxSize(-1, FromDIP(56)));
    SetMaxSize(wxSize(-1, FromDIP(56)));

    wxBoxSizer *horzSizer = new wxBoxSizer(wxHORIZONTAL);
    horzSizer->Add(m_printerCmb, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(56));
    horzSizer->Add(m_editBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(SidebarProps::ElementSpacing()));
    horzSizer->Add(m_connectionBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(SidebarProps::IconSpacing()));
    horzSizer->AddSpacer(FromDIP(SidebarProps::ContentMargin()));

    Bind(wxEVT_PAINT, &PrinterModelPanel::onPaint, this);
    SetSizer(horzSizer);
    Layout();
    Fit();
}

void PrinterModelPanel::updatePrinterIcon()
{
    PresetBundle* presetBundle = wxGetApp().preset_bundle;
    if (presetBundle == nullptr) {
        return;
    }
    Preset &preset = presetBundle->printers.get_edited_preset();
    if (preset.vendor == nullptr) {
        return;
    }
    std::string printerType = preset.config.opt_string("printer_model");
    std::string iconPath = resources_dir() + "/profiles/" + preset.vendor->id + "/" + printerType + "_cover.png";
    if (iconPath == m_iconPath) {
        return;
    }
    wxImage image;
    if (!image.LoadFile(iconPath)) {
        return;
    }
    m_iconBmp = image.Rescale(FromDIP(48), FromDIP(48));
    m_iconPath = iconPath;
}

void PrinterModelPanel::onPaint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    gc->SetPen(wxColour(0xDB, 0xDB, 0xDB));
    gc->SetBrush(*wxTRANSPARENT_BRUSH);
    gc->DrawRectangle(0, 0, GetSize().x - 1, GetSize().y - 1);
    gc->DrawBitmap(m_iconBmp, FromDIP(4), FromDIP(4), m_iconBmp.GetWidth(), m_iconBmp.GetHeight());
}

}} // namespace Slic3r::GUI
