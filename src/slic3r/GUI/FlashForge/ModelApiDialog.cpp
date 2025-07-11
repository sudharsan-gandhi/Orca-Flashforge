#include "ModelApiDialog.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/Utils/Http.hpp"
#include "slic3r/GUI/Plater.hpp"
#include "slic3r/GUI/FFUtils.hpp"
#include <wx/base64.h>

namespace Slic3r {
namespace GUI {

//wxDEFINE_EVENT(EVT_UPDATE_PROGRESS, wxCommandEvent);
//wxDEFINE_EVENT(EVT_CLOSE_PROGRESS, wxCommandEvent);
//wxDEFINE_EVENT(EVT_FINISH_CONVERT, wxCommandEvent);
//wxDEFINE_EVENT(EVT_INIT_PROGRESS, wxCommandEvent);
//wxDEFINE_EVENT(EVT_CANCEL_PROGRESS, wxCommandEvent);
//wxDEFINE_EVENT(EVT_START_TIMER, wxCommandEvent);
//wxDEFINE_EVENT(EVT_STOP_TIMER, wxCommandEvent);
//
//#define SERVER_ADDRESS "http://47.96.239.13:9050/job"
//#define GENERATE_INTERVAL 5000 
//
//ApiProgressDialog::ApiProgressDialog(wxWindow* parent) : 
//    wxDialog(parent, wxID_ANY, _L("Loading Model API")) 
//{
//    SetBackgroundColour(*wxWHITE);
//    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
//    m_text            = new Label(this, _L("Loading..."));
//    sizer->AddSpacer(FromDIP(30));
//    sizer->Add(m_text, 0, wxLEFT, FromDIP(30));
//    sizer->AddSpacer(FromDIP(20));
//    m_progress = new ProgressBar(this);
//    m_progress->SetRadius(4);
//    m_progress->ShowNumber(true);
//    m_progress->SetForegroundColour(wxColour(65, 148, 136));
//    m_progress->SetBackgroundColour(*wxWHITE);
//    m_progress->SetMinSize(wxSize(FromDIP(270), FromDIP(15)));
//    sizer->Add(m_progress, 0, wxLEFT | wxRIGHT, FromDIP(30));
//    sizer->AddSpacer(FromDIP(20));
//    m_cancelBtn = new FFButton(this, wxID_ANY, _L("cancel"), 10);
//    m_cancelBtn->SetMinSize(wxSize(FromDIP(60), FromDIP(30)));
//    m_cancelBtn->SetFontUniformColor(*wxBLACK);
//    m_cancelBtn->SetBGColor(wxColour(65, 148, 136));
//    m_cancelBtn->SetBGHoverColor(wxColour(101, 167, 158));
//    m_cancelBtn->SetBGDisableColor(wxColour(221, 221, 221));
//    m_cancelBtn->SetBGPressColor(wxColour(26, 134, 118));
//    m_cancelBtn->SetFont(Label::Body_14);
//    m_cancelBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent& event) { 
//        wxCommandEvent* e = new wxCommandEvent();
//        e->SetEventType(EVT_CANCEL_PROGRESS);
//        wxQueueEvent(this, e);
//    });
//    sizer->Add(m_cancelBtn, 0, wxALIGN_RIGHT | wxRIGHT, FromDIP(30));
//    sizer->AddSpacer(FromDIP(20));
//    this->SetSizer(sizer);
//    sizer->Fit(this);
//    Layout();
//    this->Move(FromDIP(800), FromDIP(500));
//
//    this->Bind(EVT_UPDATE_PROGRESS, [=](wxCommandEvent& event) { 
//        int progress = event.GetInt();
//        const wxString& msg = event.GetString();
//        update(msg, progress);
//    });
//
//    this->Bind(EVT_CLOSE_PROGRESS, [=](wxCommandEvent& event) { 
//        Close(true);
//    });
//}
//
//void ApiProgressDialog::update(const wxString& msg, int progress) 
//{ 
//    m_text->SetLabel(msg);
//    m_progress->SetProgress(progress);
//    if (progress >= 100) {
//        this->Close(true);
//        this->Destroy();
//    }
//}
//
//void ApiProgressDialog::sendEvent(int eventType, const wxString msg, int progress) 
//{
//    wxCommandEvent* e = new wxCommandEvent();
//    e->SetEventType(eventType);
//    if (eventType == EVT_UPDATE_PROGRESS) {
//        e->SetString(msg);
//        e->SetInt(progress);
//    }
//    wxQueueEvent(this, e);
//}
//
//void ApiProgressDialog::setTaskHandler(const shared_ptr<GenerateApiTask>& evtHandler) { 
//    this->m_taskHandler = evtHandler; 
//}
//
//const shared_ptr<GenerateApiTask>& ApiProgressDialog::getTaskHandler() { 
//    return m_taskHandler; 
//}
//
//ApiProgressDialog::~ApiProgressDialog() { 
//    m_taskHandler.reset(); 
//}
//
//GenerateApiTask::GenerateApiTask(ApiProgressDialog* parent)
//    : wxEvtHandler(), 
//    m_download_tool(4, 30000), 
//    m_timer(new wxTimer(this)), m_parent(parent)
//{
//    
//}
//
//bool GenerateApiTask::unzip_to_obj_model(const std::string& zip_file, std::string& obj_file)
//{
//    const std::regex pattern_drop(".*[.](zip)", std::regex::icase);
//    const std::regex pattern_obj(".*[.](obj)", std::regex::icase);
//    const std::regex pattern_other(".*[.](mtl|jpg|png)", std::regex::icase);
//    if (zip_file.empty() || !std::regex_match(zip_file, pattern_drop)) {
//        BOOST_LOG_TRIVIAL(error) << "Incorrect Zip File" << endl;
//        error_msg(_L("Incorrect Zip File"));
//        return false;
//    }
//    fs::path temp_dir      = wxStandardPaths::Get().GetTempDir().ToStdString();
//    fs::path zip_path      = zip_file;
//    auto     just_filename = zip_path.filename().string().substr(0, zip_path.filename().size() - zip_path.extension().size());
//    auto     path          = just_filename;
//    int      version       = 0;
//    while (fs::exists(temp_dir / path)) {
//        ++version;
//        path = just_filename + "(" + std::to_string(version) + ")";
//    }
//    fs::create_directory((temp_dir / path).string());
//    auto           dir_path = temp_dir / path;
//    mz_zip_archive archive;
//    mz_zip_zero_struct(&archive);
//
//    if (!open_zip_reader(&archive, zip_file)) {
//        BOOST_LOG_TRIVIAL(error) << "Open Zip Reader Failed" << endl;
//        error_msg(_L("Open Zip Reader Failed:") + zip_file);
//        return false;
//    }
//    try {
//        mz_uint                  num = mz_zip_reader_get_num_files(&archive);
//        mz_zip_archive_file_stat stat;
//        for (mz_uint i = 0; i < num; i++) {
//            if (mz_zip_reader_file_stat(&archive, i, &stat)) {
//                if (std::regex_match(stat.m_filename, pattern_obj)) {
//                    boost::filesystem::path path     = stat.m_filename;
//                    auto                    filepath = ((dir_path / path).string());
//                    mz_zip_reader_extract_to_file(&archive, i, filepath.data(), 0);
//                    obj_file = filepath;
//                } else if (std::regex_match(stat.m_filename, pattern_other)) {
//                    boost::filesystem::path path     = stat.m_filename;
//                    auto                    filepath = ((dir_path / path).string());
//                    mz_zip_reader_extract_to_file(&archive, i, filepath.data(), 0);
//                }
//            }
//        }
//        close_zip_reader(&archive);
//        return true;
//    } catch (std::exception& err) {
//        BOOST_LOG_TRIVIAL(error) << "Unzip Model Failed:" << err.what() << endl;
//        close_zip_reader(&archive);
//        error_msg(_L("Unzip Model Failed:") + err.what());
//        return false;
//    }
//}
//
//void GenerateApiTask::convert_obj_model_and_load(const std::string& obj_file)
//{
//    fs::path    convert_obj_file = obj_file;
//    std::string extension        = convert_obj_file.extension().string();
//    auto        just_filename    = obj_file.substr(0, obj_file.size() - extension.size()) + "_convert";
//    size_t      version          = 0;
//    convert_obj_file             = just_filename;
//    while (
//        fs::exists(boost::filesystem::path(wxStandardPaths::Get().GetTempDir().ToStdString()) / (convert_obj_file.string() + extension))) {
//        ++version;
//        convert_obj_file = just_filename + "(" + std::to_string(version) + ")";
//    }
//    wxString mtl_path    = convert_obj_file.string() + ".mtl";
//    convert_obj_file     = convert_obj_file.string() + extension;
//    auto            area = wxGetApp().plater()->build_volume().printable_area();
//    in_cvt_params_t params;
//    params.transCoordSys   = true;
//    params.maxPrintSize[0] = fabs(area[2].x() - area[0].x());
//    params.maxPrintSize[1] = fabs(area[2].y() - area[0].y());
//    params.maxPrintSize[2] = wxGetApp().plater()->build_volume().printable_height();
//    ConvertModel().convertObj(obj_file, convert_obj_file.string(), mtl_path.ToStdWstring(), params);
//    m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Loading Final OBJ Model..."), 100);
//    CallAfter([=]() {
//        wxArrayString arr;
//        arr.Add(convert_obj_file.string());
//        wxGetApp().plater()->load_files(arr);
//    });
//}
//
//void GenerateApiTask::error_msg(const wxString& msg) 
//{ 
//    CallAfter([=]() {
//        GUI::show_error(m_parent->GetParent(), msg);
//        m_download_tool.wait(true);
//        m_download_path = "";
//        if (m_timer->IsRunning()) {
//            m_timer->Stop();
//        }
//        m_parent->Close(); 
//    });
//}
//
//HunyuanApiTask::HunyuanApiTask(ApiProgressDialog* parent) : 
//    GenerateApiTask(parent) 
//{
//    m_timer->SetOwner(this);
//    Bind(wxEVT_TIMER, &HunyuanApiTask::generate_action, this);
//    m_parent->Bind(EVT_CANCEL_PROGRESS, &HunyuanApiTask::cancel_task, this);
//    m_download_tool.Bind(EVT_FF_DOWNLOAD_FINISHED, [this](FFDownloadFinishedEvent& event) {
//        int b = event.succeed;
//        if (!b) {
//            error_msg(_L("Download zip file failed"));
//            return;
//        }
//        std::thread([self = shared_from_this()]() {
//            self->m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Unzip OBJ Model And Loading..."), 90);
//            std::string obj_file;
//            self->unzip_to_obj_model(self->m_download_path, obj_file);
//            self->m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Convert Final OBJ Model..."), 97);
//            self->convert_obj_model_and_load(obj_file);
//        }).detach();
//    });
//}
//
//void HunyuanApiTask::submit_image_action(wxString& main_image,
//                                         wxString& left_image,
//                                         wxString& right_image,
//                                         wxString& back_image)
//{
//    string filename = main_image.ToStdString();
//    m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Generate Image Task..."), 10);
//    auto http = Http::post(SERVER_ADDRESS);
//    http.header("Content-Type", "multipart/form-data")
//        .form_add("prompt_type", "1")
//        .form_add("view_prompt_type", "1")
//        .form_add_file("file", filename);
//    if (left_image != "") {
//        filename = left_image.ToStdString();
//        http.form_add_file("view_left_file", filename);
//    }
//    if (right_image != "") {
//        filename = right_image.ToStdString();
//        http.form_add_file("view_right_file", filename);
//    }
//    if (back_image != "") {
//        filename = back_image.ToStdString();
//        http.form_add_file("view_back_file", filename);
//    }
//    http.on_complete([self = shared_from_this()](std::string body, unsigned status) {
//            try {
//                json     out_j = json::parse(body);
//                wxString str   = out_j["data"]["job_id"];
//                self->m_job_id       = str;
//                self->m_pollingCount = 0;
//                self->m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Polling Current Task..."), 20);
//                self->CallAfter([self]() { 
//                    self->m_timer->Start(GENERATE_INTERVAL);
//                });
//            } catch (std::exception& err) {
//                self->error_msg(_L("Job ID Not Found:") + err.what());
//                self->m_job_id = "";
//            }
//        })
//        .on_error([self = shared_from_this()](std::string body, std::string error, unsigned status) {
//            self->error_msg(_L("Network Error: ") + body);
//            BOOST_LOG_TRIVIAL(error) << "Network Error:   " << body << endl;
//        })
//        .perform();
//}
//
//void HunyuanApiTask::submit_text_action(wxString& text)
//{
//    m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Generate Text Task..."), 10);
//    auto http = Http::post(SERVER_ADDRESS);
//    http.header("Content-Type", "multipart/form-data")
//        .form_add("prompt_type", "0")
//        .form_add("prompt", text.utf8_string())
//        .on_complete([self = shared_from_this()](std::string body, unsigned status) {
//            try {
//                json     out_j = json::parse(body);
//                wxString str   = out_j["data"]["job_id"];
//                self->m_job_id       = str;
//                self->m_pollingCount = 0;
//                self->m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Polling Current Task..."), 20);
//                self->CallAfter([=]() { 
//                    self->m_timer->Start(GENERATE_INTERVAL); 
//                });
//            } catch (std::exception& err) {
//                self->error_msg(_L("Job ID Not Found:") + err.what());
//            }
//        })
//        .on_error([self = shared_from_this()](std::string body, std::string error, unsigned status) {
//            self->error_msg(_L("Network Error: ") + body);
//            BOOST_LOG_TRIVIAL(error) << "Network Error:   " << body << endl;
//        })
//        .perform();
//}
//
//void HunyuanApiTask::generate_action(wxTimerEvent& event)
//{
//    auto http = Http::get(SERVER_ADDRESS + string("?job_id=") + m_job_id.utf8_string());
//    http.header("Content-Type", "application/json")
//        .on_complete([self = shared_from_this()](std::string body, unsigned status) {
//            try {
//                json out_j     = json::parse(body);
//                int  out_state = out_j["data"]["job_status"];
//                if (!out_j["data"]["request_id"].is_null()) {
//                    if (self->isFirstGenerate) {
//                        self->isFirstGenerate = false;
//                        BOOST_LOG_TRIVIAL(info) << "HUNYUAN API REQUEST_ID:   " << (std::string) out_j["data"]["request_id"];
//                    }
//                }
//                string stateStr = "";
//                switch (out_state) {
//                case 0: stateStr = "Wait"; break;
//                case 1: stateStr = "Run"; break;
//                case 2: stateStr = "Failed"; break;
//                case 3: stateStr = "Success"; break;
//                case 4: stateStr = "Canceled"; break;
//                }
//                if (self->m_pollingCount < 59) {
//                    self->m_pollingCount++;
//                }
//                int  dot_count = (self->m_pollingCount - 1) % 3 + 1;
//                std::string dot_str = "";
//                while (dot_count--) {
//                    dot_str += ".";
//                }
//                auto str = _L("Polling Current Task") +  dot_str + "  [" + _L(stateStr) + "]";
//                self->m_parent->sendEvent(EVT_UPDATE_PROGRESS, str, 20 + self->m_pollingCount);
//                if (out_state == SUCCESS) {
//                    self->m_parent->sendEvent(EVT_UPDATE_PROGRESS, _L("Downloading OBJ Model..."), 80);
//                    wxString path;
//                    for (json str : out_j["data"]["files"]) {
//                        if (str["type"] == "OBJ") {
//                            path = (string) str["url"];
//                        }
//                    };
//                    self->m_download_path = (boost::filesystem::path(wxStandardPaths::Get().GetTempDir().ToStdString()) /
//                                       ("hunyuan_" + self->m_job_id + ".zip"))
//                                          .string();
//                    self->m_download_tool.downloadDisk(path.utf8_string(), self->m_download_path, 100000, 6000000);
//                    self->CallAfter([=]() { self->m_timer->Stop(); });
//                } else if (out_state == FAILED) {
//                    BOOST_LOG_TRIVIAL(error) << "Generate 3D API Response Failed: " << body << endl;
//                    self->error_msg("Generate 3D API Response Failed: " + body);
//                    self->CallAfter([=]() { self->m_timer->Stop(); });
//                } else if (out_state == CANCELED) {
//                    BOOST_LOG_TRIVIAL(error) << "Generate 3D API Has Canceled: " << body << endl;
//                    self->error_msg("Generate 3D API Has Canceled");
//                    self->CallAfter([=]() { self->m_timer->Stop(); });
//                }
//            } catch (std::exception & err) {
//                self->error_msg(_L("Job URL Not Found:") + err.what());
//            }
//        })
//        .on_error([self = shared_from_this()](std::string body, std::string error, unsigned status) {
//            self->error_msg(_L("Network Error: ") + body);
//            BOOST_LOG_TRIVIAL(error) << "Network Error:   " << body << endl;
//        })
//        .perform();
//}
//
//void HunyuanApiTask::cancel_task(wxCommandEvent& event)
//{
//    auto http = Http::post(SERVER_ADDRESS + string("/cancel"));
//    json j;
//    j["job_id"] = m_job_id.utf8_string();
//    http.header("Content-Type", "application/json")
//        .set_post_body(j.dump())
//        .on_complete([self = shared_from_this()](std::string body, unsigned status) {
//            try {
//                json out_j = json::parse(body);
//                if (out_j["code"] == 0) {
//                    self->CallAfter([=]() { 
//                        self->m_timer->Stop();
//                        self->m_download_tool.wait(true);
//                        self->m_download_path = "";
//                        self->m_parent->Close();
//                    });
//                } else {
//                    GUI::show_error(self->m_parent, _L("Cancel Not Successed: ") + body);
//                }
//            } catch (std::exception& err) {
//                self->error_msg(_L("Job ID Not Found:") + err.what());
//            }
//        })
//        .on_error([self = shared_from_this()](std::string body, std::string error, unsigned status) {
//            GUI::show_error(self->m_parent, _L("Network Error: ") + body);
//            BOOST_LOG_TRIVIAL(error) << "Network Error:   " << body << endl;
//        })
//        .perform_sync();
//}
//
//ModelApiPanel::ModelApiPanel(wxWindow* parent) : 
//    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(446, 570))
//{
//    this->SetSize(FromDIP(wxSize(446, 650)));
//}
//
//HunYuanModelApiPanel::HunYuanModelApiPanel(wxWindow* parent) : 
//    ModelApiPanel(parent)
//{
//    m_sizer   = new wxBoxSizer(wxVERTICAL);
//    wxBoxSizer*   radioSier  = new wxBoxSizer(wxHORIZONTAL);
//    wxStaticText* type_text  = new wxStaticText(this, wxID_ANY, _L("Generate Type"));
//    m_radio_text_btn  = new wxRadioButton(this, wxID_ANY, _L("Text Model"));
//    m_radio_image_btn = new wxRadioButton(this, wxID_ANY, _L("Image Model"));
//    radioSier->Add(type_text, wxSizerFlags(1).Left().CenterVertical().Border(FromDIP(10)));
//    radioSier->Add(m_radio_text_btn, wxSizerFlags(1).Left().CenterVertical().Expand().Border(FromDIP(10)));
//    radioSier->Add(m_radio_image_btn, wxSizerFlags(1).Left().CenterVertical().Expand().Border(FromDIP(10)));
//
//    m_text_panel  = new wxPanel(this, wxID_ANY);
//    m_image_panel = new wxPanel(this, wxID_ANY);
//    wxStaticText* input_text = new wxStaticText(m_text_panel, wxID_ANY, _L("Please Input Text"));
//    m_text_ctrl              = new wxTextCtrl(m_text_panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_BESTWRAP);
//    wxBoxSizer* text_sizer   = new wxBoxSizer(wxVERTICAL);
//    text_sizer->Add(input_text, wxSizerFlags(1).Left().Bottom().Expand());
//    text_sizer->Add(m_text_ctrl, wxSizerFlags(1).Expand().Proportion(5));
//    m_text_panel->SetSizerAndFit(text_sizer);
//
//    wxStaticText* input_text1= new wxStaticText(m_image_panel, wxID_ANY, _L("Please Upload Image"));
//    wxBoxSizer* image_sizer         = new wxBoxSizer(wxVERTICAL);
//    m_main_image                       = new ImageUploadPanel(m_image_panel, _L("Main") + "*", 90);
//    m_left_image                      = new ImageUploadPanel(m_image_panel, _L("Left"), 80);
//    m_right_image                      = new ImageUploadPanel(m_image_panel, _L("Right"), 80);
//    m_back_image                      = new ImageUploadPanel(m_image_panel, _L("Back"), 80);
//
//    image_sizer->Add(input_text1, wxSizerFlags(1).Left().Bottom().Expand());
//    auto flex_sizer = new wxFlexGridSizer(2, 3, FromDIP(5), FromDIP(5));
//    flex_sizer->Add(new wxPanel(m_image_panel), 0, wxALL, 0);
//    flex_sizer->Add(m_main_image, 0, wxALIGN_CENTER | wxALL, 0);
//    flex_sizer->Add(new wxPanel(m_image_panel), 0, wxALL, 0);
//    flex_sizer->Add(m_left_image, 0, wxALIGN_CENTER | wxALL, 0);
//    flex_sizer->Add(m_right_image, 0, wxALIGN_CENTER | wxALL, 0);
//    flex_sizer->Add(m_back_image, 0, wxALIGN_CENTER | wxALL, 0);
//    flex_sizer->AddGrowableCol(1);
//    image_sizer->Add(flex_sizer, wxSizerFlags(1).Center().Proportion(8));
//    image_sizer->AddSpacer(20);
//    m_image_panel->SetSizerAndFit(image_sizer);
//    StateColor btn_bg_blue(std::pair<wxColour, int>(wxColour(31, 142, 234), StateColor::Pressed),
//                           std::pair<wxColour, int>(wxColour(30, 152, 215), StateColor::Normal));
//    auto generate_model_btn  = new Button(this, _L("Generate Model"));
//    generate_model_btn->SetSize(FromDIP(140), FromDIP(40));
//    generate_model_btn->SetTextColor(*wxWHITE);
//    generate_model_btn->SetBorderColor(*wxWHITE);
//    generate_model_btn->SetBackgroundColor(btn_bg_blue);
//    generate_model_btn->SetCornerRadius(6);
//    m_sizer->Add(radioSier, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(20)));
//    m_sizer->Add(m_text_panel, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(10)).Proportion(4));
//    m_sizer->Add(m_image_panel, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(10)).Proportion(4));
//    m_sizer->Add(generate_model_btn, wxSizerFlags(1).Border(wxALL, FromDIP(10)).Center());
//    SetSizer(m_sizer);
//    Center();
//    m_radio_text_btn->SetValue(true);
//    m_image_panel->Hide();
//
//    m_radio_text_btn->Bind(wxEVT_RADIOBUTTON, &HunYuanModelApiPanel::radio_button_selected, this);
//    m_radio_image_btn->Bind(wxEVT_RADIOBUTTON, &HunYuanModelApiPanel::radio_button_selected, this);
//
//    generate_model_btn->Bind(wxEVT_BUTTON, &HunYuanModelApiPanel::ButtonClicked, this);
//}
//
//HunYuanModelApiPanel::~HunYuanModelApiPanel() 
//{ 
//}
//
//
//void HunYuanModelApiPanel::ButtonClicked(wxCommandEvent& event) 
//{
//    if (m_image_panel->IsShown()) {
//        auto main_image = judgeTransImage(m_main_image->image_file_path, _L("Main") + " ");
//        if (main_image == "") {
//            GUI::show_error(this->GetParent(), _L("Please Upload Image"));
//            return;
//        }
//        if (main_image == "error") {
//            return;
//        }
//        auto left_image = judgeTransImage(m_left_image->image_file_path, _L("Left") + " ");
//        if (left_image == "error") {
//            return;
//        }
//        auto right_image = judgeTransImage(m_right_image->image_file_path, _L("Right") + " ");
//        if (right_image == "error") {
//            return;
//        }
//        auto back_image = judgeTransImage(m_back_image->image_file_path, _L("Back") + " ");
//        if (back_image == "error") {
//            return;
//        }
//        this->GetParent()->Close();
//        auto dlg = new ApiProgressDialog(wxGetApp().GetMainTopWindow());
//        auto task = std::make_shared<HunyuanApiTask>(dlg);
//        dlg->setTaskHandler(task);
//        task->submit_image_action(main_image, left_image, right_image, back_image);
//        dlg->Show();
//    } else if (m_text_panel->IsShown()){
//        if (m_text_ctrl->IsEmpty()) {
//            GUI::show_error(this->GetParent(), _L("Please Input Text"));
//            return;
//        }
//        auto text = m_text_ctrl->GetValue();
//        this->GetParent()->Close();
//        auto dlg  = new ApiProgressDialog(wxGetApp().GetMainTopWindow());
//        auto task = std::make_shared<HunyuanApiTask>(dlg);
//        dlg->setTaskHandler(task);
//        task->submit_text_action(text);
//        dlg->Show();
//    }
//}
//
//void HunYuanModelApiPanel::radio_button_selected(wxCommandEvent& event) 
//{
//    if (m_image_panel->IsShown()) {
//        m_text_panel->Show();
//        m_image_panel->Hide();
//    }
//    else {
//        m_text_panel->Hide();
//        m_image_panel->Show();
//    }
//    Layout();
//    
//}
//
//

wxDEFINE_EVENT(EVT_LOADED_IMAGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_FINISH_TASK, wxCommandEvent);
wxDEFINE_EVENT(EVT_UPDATE_ICON, wxCommandEvent);

QuestionDialog::QuestionDialog(wxWindow* parent) : FFRoundedWindow(parent)
{
    SetSize(FromDIP(272), FromDIP(189));
    auto title = new Label(this, Label::Body_13, _L("Image Upload Tips"));
    auto info =
        new Label(this, Label::Body_12,
                  _L("It supports PNG, JPG, JPEG, and WebP.Images should be no larger than 6MB with a minimum resolution of 128*128."));
    auto text1   = new Label(this, Label::Body_12, _L("Simple background (preferably solid color)"));
    auto text2   = new Label(this, Label::Body_12, _L("No text included"));
    auto text3   = new Label(this, Label::Body_12, _L("Single model"));
    auto text4   = new Label(this, Label::Body_12, _L("The model should not be too small"));
    auto v_sizer = new wxBoxSizer(wxVERTICAL);
    v_sizer->AddSpacer(FromDIP(16));
    v_sizer->Add(title, 0, wxALIGN_LEFT | wxLEFT, FromDIP(16));
    v_sizer->AddSpacer(FromDIP(6));
    info->Wrap(FromDIP(240));
    v_sizer->Add(info, 0, wxLEFT | wxRIGHT, FromDIP(16));
    v_sizer->AddSpacer(FromDIP(12));
    auto h_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto v_sizer0 = new wxBoxSizer(wxVERTICAL);
    text1->Wrap(FromDIP(101));
    v_sizer0->Add(text1, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text2->Wrap(FromDIP(101));
    v_sizer0->Add(text2, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text3->Wrap(FromDIP(101));
    v_sizer0->Add(text3, 0, wxALL, 0);
    v_sizer0->AddSpacer(FromDIP(6));
    text4->Wrap(FromDIP(101));
    v_sizer0->Add(text4, 0, wxALL, 0);
    auto       text_height = FromDIP(6) * 3;
    wxCoord    w, h;
    wxMemoryDC dc;
    dc.SetFont(Label::Body_12);
    dc.GetMultiLineTextExtent(text1->GetLabel(), &w, &h);
    text_height += h;
    dc.GetMultiLineTextExtent(text2->GetLabel(), &w, &h);
    text_height += h;
    dc.GetMultiLineTextExtent(text3->GetLabel(), &w, &h);
    text_height += h;
    dc.GetMultiLineTextExtent(text4->GetLabel(), &w, &h);
    text_height += h;
    ScalableBitmap bmp(this, "question_tip_image", ToDIP(text_height));
    auto           img = new wxStaticBitmap(this, wxID_ANY, bmp.bmp());
    h_sizer->Add(img, 0, wxEXPAND | wxALL, 0);
    h_sizer->AddSpacer(FromDIP(16));
    h_sizer->Add(v_sizer0, 0, wxALL, 0);
    v_sizer->Add(h_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, FromDIP(16));
    v_sizer->AddSpacer(FromDIP(16));
    Layout();
    SetSizerAndFit(v_sizer);
}

ApiLoadingIcon::ApiLoadingIcon(wxDialog* parent) : wxEvtHandler()
{
    m_timer  = new wxTimer(this);
    Bind(wxEVT_TIMER, &ApiLoadingIcon::OnTimer, this);
    for (int i = 0; i < 4; i++) {
        auto           str = boost::format("api_loading_%1%") % (i + 1);
        ScalableBitmap bmp(parent, str.str(), 60);
        m_loadingIcons.emplace_back(std::move(bmp));
    }
}

void ApiLoadingIcon::paintInRect(wxGraphicsContext* gc, wxRect rect)
{
    wxBitmap& bmp = m_loadingIcons[m_loadingIdx].bmp();
    gc->DrawBitmap(bmp, rect.x, rect.y, rect.width, rect.height);
}

void ApiLoadingIcon::Loading(int interval)
{
    if (m_timer->IsRunning()) {
        m_timer->Stop();
    }
    m_loadingIdx  = 0;
    m_loadingTime = 0;
    m_timer->Start(interval);
}

void ApiLoadingIcon::End()
{
    if (m_timer->IsRunning()) {
        m_timer->Stop();
    }
}

bool ApiLoadingIcon::isLoading() { return m_timer->IsRunning(); }

void ApiLoadingIcon::OnTimer(wxTimerEvent& event)
{
    m_loadingTime++;
    m_loadingIdx = (m_loadingIdx + 1) % m_loadingIcons.size();
    this->QueueEvent(new wxCommandEvent(EVT_UPDATE_ICON));
}

ModelApiTask::ModelApiTask(std::function<void()> func) : wxEvtHandler()
{
    this->m_func = func; 
}

void ModelApiTask::start() 
{
    std::thread([self = shared_from_this()]() {
        self->m_func();
        auto e = new wxCommandEvent(EVT_FINISH_TASK);
        //e->SetString(id);
        self->QueueEvent(e);
    }).detach();
}

bool ImageUploadPanel::judgeTransImage(wxString& path)
{
    if (path.IsEmpty()) {
        return false;
    }
    fstream fs;
    fs.open(path.ToStdString(), ios::binary | ios::in);
    fs.seekg(0, ios::end);
    int size = fs.tellg();
    if (size == -1) {
        GUI::show_error(this, _L("Image Read Failed"));
        return false;
    } else if (size > 6 * 1024 * 1024) {
        GUI::show_error(this, _L("Image Too Large"));
        return false;
    }
    fs.close();
    string  buf;
    wxImage img;
    bool    flag = img.LoadFile(wxString::FromUTF8(path.utf8_string()), wxBITMAP_TYPE_ANY);
    if (!flag) {
        GUI::show_error(this, _L("Image Can't Read"));
        return false;
    }

    int    min_size     = min(img.GetHeight(), img.GetWidth());
    int    max_size     = max(img.GetHeight(), img.GetWidth());
    if (min_size < 50) {
        GUI::show_error(this, _L("Image Min Size Is 50"));
        return false;
    }
    if (max_size > 5000) {
        GUI::show_error(this, _L("Image Max Size Is 5000"));
        return false;
    }
    return true;
}

ImageUploadPanel::ImageUploadPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    SetMinSize(FromDIP(wxSize(160, 160)));
    SetMaxSize(FromDIP(wxSize(160, 160)));
    SetSize(FromDIP(wxSize(160, 160)));
    SetDoubleBuffered(true);
    m_upload_icon = ScalableBitmap(this, "model_api_upload_image", 24);
    m_delete_icon = ScalableBitmap(this, "model_api_delete_image", 29);
    Bind(wxEVT_PAINT, &ImageUploadPanel::onPaint, this);
    Bind(wxEVT_LEFT_DOWN, &ImageUploadPanel::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ImageUploadPanel::onLeftUp, this);
    Bind(wxEVT_ENTER_WINDOW, &ImageUploadPanel::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &ImageUploadPanel::OnMouseLeave, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &ImageUploadPanel::onMouseCaptureLost, this);
}

wxString ImageUploadPanel::getPath() { return m_path; }

void ImageUploadPanel::onPaint(wxPaintEvent& event) 
{
    auto size = GetClientSize();
    wxPaintDC                          dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (gc == nullptr) {
        return;
    }
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    if (m_path.empty() || !m_img.IsOk()) {
        gc->SetBrush(wxBrush(*wxWHITE));
        gc->DrawRectangle(0, 0, size.x, size.y);
        gc->DrawBitmap(m_upload_icon.bmp(), (size.x - m_upload_icon.GetBmpWidth()) / 2, FromDIP(62), m_upload_icon.GetBmpWidth(), m_upload_icon.GetBmpHeight());
        dc.SetFont(Label::Body_11);
        dc.SetTextForeground(wxColor("#999999"));
        wxString str       = _L("Please Upload Image");
        auto     text_size = dc.GetTextExtent(str);
        dc.DrawText(str, (size.x - text_size.x) / 2, FromDIP(98));
    }
    else {
        gc->SetBrush(wxBrush(*wxBLACK));
        gc->DrawRectangle(0, 0, size.x, size.y);
        auto rect = FFUtils::calcContainedRect(size, m_img.GetSize(), false);
        gc->DrawBitmap(m_img, rect.x, rect.y, rect.width, rect.height);
        if (m_isHovered) {    
            gc->SetBrush(wxBrush(wxColour(0, 0, 0, 128)));
            gc->DrawRectangle(0, 0, size.x, size.y);
            gc->DrawBitmap(m_delete_icon.bmp(), (size.x - m_delete_icon.GetBmpWidth()) / 2, (size.y - m_delete_icon.GetBmpHeight()) / 2,
                           m_delete_icon.GetBmpWidth(), m_delete_icon.GetBmpHeight());
        }
    }
}

void ImageUploadPanel::onLeftDown(wxMouseEvent& event) 
{
    m_isPressed = true;
    if (!HasCapture()) {
        CaptureMouse();
    }
}

void ImageUploadPanel::onLeftUp(wxMouseEvent& event) 
{
    if (!m_isPressed) {
        event.Skip();
        return;
    }

    bool isFunc = true;
    if (m_path.empty() || !m_img.IsOk()) {
        wxFileDialog  dlg(this, _L("Select Image"), wxGetApp().app_config->get_last_dir(), "",
                          "Image files (*.jpeg;*jpg;*.png;)|*.jpeg;*.jpg;*.png;", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        wxArrayString files;
        if (dlg.ShowModal() != wxID_OK)
            return;
        dlg.GetPaths(files);
        m_path = files[0];
        if (judgeTransImage(m_path)) {
            if (!m_img.LoadFile(m_path, wxBITMAP_TYPE_ANY)) {
                GUI::show_error(this, _L("Image Load Failed"));
                isFunc = false;
            }
        }
        else {
            m_path = "";
            isFunc = false;
        }
    }
    else {
        m_path = "";
        m_img.Clear();
    }

    if (isFunc) {
        Refresh();
        wxQueueEvent(this, new wxCommandEvent(EVT_LOADED_IMAGE));
    }

    m_isPressed = false;
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void ImageUploadPanel::onMouseCaptureLost(wxMouseCaptureLostEvent& event) 
{
    m_isPressed = false;
    Refresh();
}

void ImageUploadPanel::OnMouseEnter(wxMouseEvent& event) 
{
    m_isHovered = true;
    SetCursor(wxCURSOR_HAND);
    Refresh();
}

void ImageUploadPanel::OnMouseLeave(wxMouseEvent& event) 
{
    m_isHovered = false;
    SetCursor(wxCURSOR_ARROW);
    Refresh();
}

ModelApiDialog::ModelApiDialog(wxWindow* parent) : 
    FFTitleLessDialog(parent), m_generate_btn_rect(0, 0, 0, 0), m_question_link_rect(0, 0, 0, 0)
{
    this->SetSize(FromDIP(wxSize(393, 400)));
    this->SetMinSize(FromDIP(wxSize(393, 400)));
    this->SetDoubleBuffered(true);
    m_loadIcon        = std::make_shared<ApiLoadingIcon>(this);
    m_loadIcon->Bind(EVT_UPDATE_ICON, [=](wxCommandEvent& event) { 
        this->Refresh();
    });
    m_loadIcon->Loading(200);
    m_loadTask                 = std::make_shared<ModelApiTask>([=]() { 
        //std::this_thread::sleep_for(std::chrono::seconds(10));
    });
    m_loadTask->Bind(EVT_FINISH_TASK, [=](wxCommandEvent& event) { 
        this->m_loadIcon->End();
        m_cost_text = wxString(_L("Cost 111"));
        Refresh();
    });
    m_loadTask->start();
    m_question_dialog          = new QuestionDialog(this);
    m_question_dialog->Hide();
    m_bmp_map["bg"] = ScalableBitmap(this, "model_api_dlg_bg", ToDIP(GetSize().y));
    m_bmp_map["question_mark"] = ScalableBitmap(this, "model_api_question_mark", 12);
    m_cost_text     = wxString(_L("Cost Free"));
    m_score_text    = wxString(_L("Remaining Score") + wxString::Format(wxT(": %d"), 25));
    auto sizer      = new wxBoxSizer(wxVERTICAL);
    m_image_panel              = new ImageUploadPanel(this);
    m_image_panel->Bind(EVT_LOADED_IMAGE, [=](wxCommandEvent& event) { Refresh(); });
    sizer->AddSpacer(FromDIP(107));
    sizer->Add(m_image_panel, wxLEFT | wxRIGHT | wxALIGN_CENTER, FromDIP(117));
    sizer->AddSpacer(FromDIP(134));
    sizer->Fit(this);
    SetSizer(sizer);
    Layout();
    Center();

    Bind(wxEVT_LEFT_DOWN, &ModelApiDialog::onLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ModelApiDialog::onLeftUp, this);
    Bind(wxEVT_MOTION, &ModelApiDialog::OnMouseMove, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &ModelApiDialog::onMouseCaptureLost, this);
}

void ModelApiDialog::drawBackground(wxPaintDC& dc, wxGraphicsContext* gc)
{
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    auto size = this->GetClientSize();
    gc->DrawBitmap(m_bmp_map["bg"].bmp(), 0, 0, size.x, size.y);
    drawCenterText(gc, _L("AI Model Generate"), FromDIP(40), Label::Body_14, wxColor("#333333"));
    drawCenterText(gc, _L("Image Suggestion"), FromDIP(81), Label::Body_11, wxColor("#333333"), "question_mark");
    if (m_loadIcon->isLoading()) {
        const int loadSize = FromDIP(40);
        m_loadIcon->paintInRect(gc, wxRect((size.x - loadSize) / 2, FromDIP(283), loadSize, loadSize));
    } else {
        drawCenterText(gc, m_cost_text, FromDIP(283), Label::Body_12, wxColor("#333333"));
        drawCenterText(gc, m_score_text, FromDIP(306), Label::Body_12, wxColor("#419488"));
    }
    if (m_loadIcon->isLoading() || m_image_panel->getPath().empty()) {
        gc->SetBrush(wxColor("#D2D2D2"));
    }
    else {
        gc->SetBrush(wxColor("#419488"));
        if (m_isGenerateHovered) {
            gc->SetBrush(wxColor("#65A79E"));
        }
        if (m_isPressed) {
            gc->SetBrush(wxColor("#1A8676"));
        }
    }
    wxString btn_text(_L("Starting Generate"));
    auto     btn_text_size = dc.GetTextExtent(btn_text);
    wxSize btn_size(FromDIP(10) * 2 + btn_text_size.x, FromDIP(30));
    if (m_generate_btn_rect.IsEmpty()) {
        m_generate_btn_rect = wxRect((size.x - btn_size.x) / 2, FromDIP(339), btn_size.x, btn_size.y);
    }
    gc->DrawRoundedRectangle((size.x - btn_size.x) / 2, FromDIP(339), btn_size.x, btn_size.y, 4);
    dc.SetFont(Label::Body_12);
    dc.SetTextForeground(*wxWHITE);
    dc.DrawText(btn_text, (size.x - btn_text_size.x) / 2, FromDIP(346));
}

void ModelApiDialog::drawCenterText(wxGraphicsContext* gc, wxString& str, int height, wxFont& font, wxColour color, wxString iconName) 
{ 
    wxPaintDC dc(this);
    dc.SetFont(font);
    auto text_size = dc.GetTextExtent(str);
    auto size = this->GetClientSize();
    
    if (iconName.empty()) {
        dc.DrawText(str, (size.x - text_size.x) / 2, height);
    }
    else {
        auto& bmp = m_bmp_map[iconName.ToStdString()].bmp();
        const int icon_sper = 5;
        gc->DrawBitmap(bmp, (size.x - text_size.x - bmp.GetWidth() - icon_sper) / 2, height, bmp.GetWidth(), bmp.GetHeight());
        dc.DrawText(str, (size.x - text_size.x - bmp.GetWidth() - icon_sper) / 2 + icon_sper + bmp.GetWidth(), height);
        if (iconName == "question_mark" && m_question_link_rect.IsEmpty()) {
            m_question_link_rect = wxRect((size.x - text_size.x - bmp.GetWidth() - icon_sper) / 2, height, 
                text_size.x + bmp.GetWidth() + icon_sper, bmp.GetHeight());
        }
    }
}

void ModelApiDialog::onLeftDown(wxMouseEvent& event) 
{
    if (m_generate_btn_rect.IsEmpty()) {
        event.Skip();
        return;
    }
    if (!m_generate_btn_rect.Contains(event.GetPosition())) {
        event.Skip();
        return;
    }
    m_isPressed = true;
    Refresh();
    if (!HasCapture()) {
        CaptureMouse();
    }
}

void ModelApiDialog::onLeftUp(wxMouseEvent& event) 
{
    if (m_generate_btn_rect.IsEmpty()) {
        event.Skip();
        return;
    }
    if (!m_isPressed) {
        event.Skip();
        return;
    }
    if (!m_image_panel->getPath().empty() && m_generate_btn_rect.Contains(event.GetPosition())) {
        GenerateClicked();
    }
    m_isPressed = false;
    Refresh();
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void ModelApiDialog::onMouseCaptureLost(wxMouseCaptureLostEvent& event) 
{ 
    m_isPressed = false;
    Refresh();
    event.Skip();
}

void ModelApiDialog::OnMouseMove(wxMouseEvent& event) 
{
    if (m_isGenerateHovered && !m_generate_btn_rect.Contains(event.GetPosition())) {
        m_isGenerateHovered = false;
        SetCursor(wxCURSOR_ARROW);
        Refresh();
    } else if (!m_isGenerateHovered && m_generate_btn_rect.Contains(event.GetPosition())) {
        m_isGenerateHovered = true;
        SetCursor(wxCURSOR_HAND);
        Refresh();
    }
    if (m_isQuestionHovered && !m_question_link_rect.Contains(event.GetPosition())) {
        m_isQuestionHovered = false;
        SetCursor(wxCURSOR_ARROW);
        m_question_dialog->Show(false);
    }
    else if (!m_isQuestionHovered && m_question_link_rect.Contains(event.GetPosition())) {
        m_isQuestionHovered = true;
        SetCursor(wxCURSOR_HAND);
        m_question_dialog->Move(this->ClientToScreen(wxPoint((GetClientSize().x - m_question_dialog->GetSize().x) / 2, FromDIP(106))));
        m_question_dialog->Show(true);
    }
    
    event.Skip();
}

void ModelApiDialog::GenerateClicked() 
{ 
    Close();
    auto dlg = new ModelGenerateDialog(this); 
    dlg->Show();
}

ModelGenerateDialog::ModelGenerateDialog(wxWindow* parent) :
    FFTitleLessDialog(parent)
{
    this->SetSize(wxSize(FromDIP(393), -1));
    this->SetMinSize(wxSize(FromDIP(393), -1));
    this->SetDoubleBuffered(true);
    m_loadIcon = std::make_shared<ApiLoadingIcon>(this);
    m_loadIcon->Bind(EVT_UPDATE_ICON, [=](wxCommandEvent& event) { this->Refresh(); });
    m_loadIcon->Loading(200);
    m_info_text = new Label(this, Label::Body_14, "");
    m_queue_text = new Label(this, Label::Body_13, "");
    m_under_queue_sperator = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(300), FromDIP(16)));
    m_under_queue_sperator->SetBackgroundColour(*wxWHITE);
    m_under_queue_sperator->SetMinSize(wxSize(-1, FromDIP(16)));
    m_under_queue_sperator->SetMaxSize(wxSize(-1, FromDIP(16)));
    m_sizer = new wxBoxSizer(wxVERTICAL);
    m_sizer->AddSpacer(FromDIP(32));
    m_sizer->Add(m_info_text, 0, wxALIGN_CENTER | wxALL, 0);
    m_sizer->AddSpacer(FromDIP(16));
    m_sizer->Add(m_queue_text, 0, wxALIGN_CENTER | wxALL, 0);
    m_sizer->Add(m_under_queue_sperator, 0, wxALIGN_CENTER, wxALL, 0);
    m_sizer->AddSpacer(FromDIP(80));
    SetSizer(m_sizer);
    showCurState(true);
}

void ModelGenerateDialog::drawBackground(wxPaintDC& dc, wxGraphicsContext* gc) 
{
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    const int img_size = FromDIP(60);
    auto      size     = GetClientSize();
    m_loadIcon->paintInRect(gc, wxRect((size.x - img_size) / 2, FromDIP(96), img_size, img_size));
}

void ModelGenerateDialog::showCurState(bool isQueue) 
{
    m_queue_text->SetLabel(_L("Current queue") + wxString::Format(wxT(" %d/%d"), m_remainCount, m_totalCount));
    m_queue_text->Show(isQueue);
    if (isQueue) {
        m_info_text->SetLabel(_L("We're currently experiencing high demand. Please wait..."));
        m_queue_text->SetLabel(_L("Current queue") + wxString::Format(wxT(" %d/%d"), m_remainCount, m_totalCount));
        m_under_queue_sperator->Show();
        m_queue_text->Show();
    }
    else {
        m_info_text->SetLabel(_L("Generating, please wait..."));
        m_under_queue_sperator->Hide();
        m_queue_text->Hide();
    }
    Layout();
    Fit();
    Center();
}

} // namespace GUI
} // namespace Slic3r::GUI


