#include "TimeLapseVideoMsgDlg.hpp"

#include "slic3r/GUI/I18N.hpp"

std::string htmlTemplate = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Time Lapse Video</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            overflow: hidden;
        }
        
        video {
            width: 100vw;
            height: 100vh;
            object-fit: cover;
            position: fixed;
            top: 0;
            left: 0;
        }
    </style>
</head>
<body>
    <video controls>
        <source src="%s" type="video/mp4">
    </video>
</body>
</html>
)";

namespace Slic3r {
namespace GUI {

    TimeLapseVideoMsgDlg::TimeLapseVideoMsgDlg(wxWindow* parent, const std::string& filepath, const std::string& video_url)
        : wxDialog(parent, wxID_ANY, _L("Video"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
        , m_filepath(filepath)
        , m_video_url(video_url)
    {
        generate_html();


        SetSize(FromDIP(1000), FromDIP(500));

        std::string localUrl = "file:///" + m_filepath;
        m_webview            = wxWebView::New(this, wxID_ANY, localUrl);
        m_webview->SetSize(GetClientSize());

        CentreOnParent();

        m_webview->Bind(wxEVT_WEBVIEW_LOADED, [](wxWebViewEvent& event) {
            wxLogMessage("Page loaded successfully: %s", event.GetURL());
        });
        m_webview->Bind(wxEVT_WEBVIEW_ERROR, [](wxWebViewEvent& event) {
            wxString errorMsg;
            switch (event.GetInt()) {
            case wxWEBVIEW_NAV_ERR_CONNECTION: errorMsg = "Connection error"; break;
            case wxWEBVIEW_NAV_ERR_CERTIFICATE: errorMsg = "Certificate error"; break;
            case wxWEBVIEW_NAV_ERR_NOT_FOUND: errorMsg = "Page not found"; break;
            default: errorMsg = "Unknown error"; break;
            }
            wxLogError("Failed to load page: %s (%s)", event.GetURL(), errorMsg);
        });

    }

    TimeLapseVideoMsgDlg::~TimeLapseVideoMsgDlg()
    {

    }

    bool TimeLapseVideoMsgDlg::generate_html()
    {
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), htmlTemplate.c_str(), m_video_url.c_str());



        std::ofstream outFile(m_filepath);
        if (!outFile) {
            printf("Error: Unable to create or open the file! %s\n", m_filepath.c_str());
            return false;
        }
        outFile << buffer;
        outFile.close();
        return true;
    }

}}