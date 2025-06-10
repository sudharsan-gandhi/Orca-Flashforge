#ifndef slic3r_GUI_PrinterModelPanel_hpp_
#define slic3r_GUI_PrinterModelPanel_hpp_

#include <wx/panel.h>
#include <wx/bitmap.h>
#include "slic3r/GUI/PresetComboBoxes.hpp"
#include "slic3r/GUI/wxExtensions.hpp"

namespace Slic3r { namespace GUI {

class PrinterModelPanel : public wxPanel
{
public:
    PrinterModelPanel(wxWindow *parent);

    void setup(PlaterPresetComboBox *printerCmb, ScalableButton *editBtn, ScalableButton *connectionBtn);

    void updatePrinterIcon();

private:
    void onPaint(wxPaintEvent &evt);

private:
    std::string           m_iconPath;
    wxBitmap              m_iconBmp;
    PlaterPresetComboBox *m_printerCmb;
    ScalableButton       *m_editBtn;
    ScalableButton       *m_connectionBtn;
};

}} // namespace Slic3r::GUI

#endif
