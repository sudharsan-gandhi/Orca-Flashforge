#include "TimeLapseVideoPlayDlg.hpp"

#include "slic3r/GUI/I18N.hpp"

std::string htmlTemplate = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>time lapse video</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            overflow: hidden;
        }
        body {
            width: 100vw;
            height: 100vh;
            background: black;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        #video-container {
            position: relative;
            max-width: 100%;
            max-height: 100%;
        }
        video {
            width: auto;
            height: auto;
            max-width: 100%;
            max-height: 100%;
            vertical-align: middle;
        }
    </style>
</head>
<body>
    <div id="video-container">
        <video controls>
            <source src="%s">
        </video>
    </div>
    <script>
        function resizeVideo() {
            const video = document.querySelector('video');
            const container = document.getElementById('video-container');
            if (!video.videoWidth) return;
            const videoRatio = video.videoWidth / video.videoHeight;
            const windowRatio = window.innerWidth / window.innerHeight;
            if (windowRatio > videoRatio) {
                video.style.height = '100vh';
                video.style.width = 'auto';
            } else {
                video.style.width = '100vw';
                video.style.height = 'auto';
            }
        }
        window.addEventListener('pageshow', () => {
            resizeVideo();
        });
        window.addEventListener('resize', resizeVideo);
    </script>
</body>
</html>

)";

namespace Slic3r {
namespace GUI {

    TimeLapseVideoPlayDlg::TimeLapseVideoPlayDlg(wxWindow* parent, const std::string& filepath, const std::string& video_url, int width, int height)
        : wxDialog(parent, wxID_ANY, _L("Video"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
        , m_filepath(filepath)
        , m_video_url(video_url)
        , m_width(width)
        , m_height(height)
    {
        generate_html();


        SetSize(FromDIP(m_width), FromDIP(m_height));

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

    TimeLapseVideoPlayDlg::~TimeLapseVideoPlayDlg()
    {

    }

    bool TimeLapseVideoPlayDlg::generate_html()
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