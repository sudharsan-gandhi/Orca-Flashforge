#ifndef _slic3r_gui_FFUtils_hpp_
#define _slic3r_gui_FFUtils_hpp_

#include <string.h>
#include <wx/gdicmn.h>
#include <wx/string.h>

namespace Slic3r::GUI
{

enum FFPrinterPid { 
	ADVENTURER_5M     = 0x0023,
	ADVENTURER_5M_PRO = 0x0024,
	GUIDER_4          = 0x0025,
	AD5X              = 0x0026,
	GUIDER_4_PRO      = 0x0027,
	U1                = 0x0028,
	ADVENTURER_A5     = 0x00BB,
	GUIDER_3_ULTRA    = 0x001F,
	OTHER             = 0xFFFF
};

struct FFPrinterPreset
{
    std::string bmp_file_name;
    std::string name;
    std::string model_id;
    FFPrinterPreset() {};
    FFPrinterPreset(std::string bmp_file_name, std::string name, std::string model_id)
		: bmp_file_name(bmp_file_name), name(name), model_id(model_id) {}
};

class FFUtils
{
public:
    static std::unordered_map<unsigned short, FFPrinterPreset> printer_preset_map;

	static wxString getBitmapFileName(unsigned short pid);

	static std::string getPrinterName(unsigned short pid);

	static std::string getPrinterModelId(unsigned short pid);

	static unsigned short getPid(int curId);

	static bool isPrinterSupportAms(unsigned short pid);
    static bool isPrinterSupportCoolingFan(unsigned short pid);
    static bool isPrinterSupportDeviceFilter(unsigned short pid);
    static bool isNozzlesPrinter(unsigned short pid);
   
	static wxString convertStatus(const std::string& status);
	static wxString convertStatus(const std::string& status, wxColour& color);

	static wxString converDeviceError(const std::string &error);

	static std::string utf8Substr(const std::string& str, int start, int length);

	static std::string truncateString(const std::string &s, size_t length);
    static std::string wxString2StdString(const wxString& str);

	static wxString trimString(wxDC &dc, const wxString &str, int width);
    static wxString elideString(wxWindow* wnd, const wxString& str, int width);
	static wxString elideString(wxWindow* wnd, const wxString& str, int width, int lines);
	static wxString wrapString(wxWindow* wnd, const wxString& str, int width);
	static wxString wrapString(wxDC &dc, const wxString &str, int width);
	static int getStringLines(const wxString& str);

	static std::string flashforgeWebsite();
	static wxString privacyPolicy();
	static wxString userAgreement();
	static wxString userRegister();
	static wxString passwordForget();

	static wxRect calcContainedRect(const wxSize &containerSize, const wxSize &imgSize, bool enlarge);
};

}

#endif /* _slic3r_gui_FFUtils_hpp_ */
