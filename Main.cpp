// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
//                                                        NEWS READER
//                                                      © Lyndon S. 2025
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/event.h>
#include <wx/srchctrl.h>
#include <wx/richtext/richtextctrl.h>
#include <chrono>
#include <ctime>
#include "pugixml.hpp"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include "wx/textctrl.h"

// Curl callback function
size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* data = static_cast<std::string*>(userdata);
    data->append(ptr, size * nmemb);
        return size * nmemb;
}

// Main application
class App : public wxApp {
public:
    bool OnInit() {
        // Chrono con
        auto start = std::chrono::system_clock::now();
        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end - start;
        end_time = std::chrono::system_clock::to_time_t(end);

        wxString news = NewsReader();
        wxString output("Welcome to News!\nDownloading News... \n <-------------------------------------------------> \n" + news);
        // Prints time string
        wxString timeData = wxString(std::ctime(&end_time)) + "\n";

        // window output screen
        if (news.IsEmpty()) {
            wxMessageBox("No news items found.");
            return false;
        };

        wxFrame* window = new wxFrame(
            nullptr, wxID_ANY, "News Reader", wxDefaultPosition, wxSize(1280, 720)
        );

        wxPanel* panel = new wxPanel(window, wxID_ANY);

        richTextBox = new wxRichTextCtrl(
            panel, wxID_ANY, output + timeData,
            wxDefaultPosition, wxDefaultSize,
            wxRE_READONLY | wxVSCROLL | wxHSCROLL
		);

        /* Fallback retired default textbox
        textBox = new wxTextCtrl(
            panel, wxID_ANY, output + timeData,
            wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL
        );
        */

        timeBox = new wxTextCtrl(
            panel, wxID_ANY, timeData,
            wxPoint(10, 550), wxSize(200, 25),
            wxTE_MULTILINE | wxTE_READONLY
        );

        downloadTime = new wxTextCtrl(
            panel, wxID_ANY, wxString::Format("%lld", static_cast<long long>(end_time)),
            wxPoint(10, 550), wxSize(200, 25),
            wxTE_MULTILINE | wxTE_READONLY
        );

        wxButton* button = new wxButton(
            panel, wxID_ANY, "Refresh", wxPoint(550, 550)
        );

        // Sets news to Sky
        wxButton* SkyButton = new wxButton(
            panel, wxID_ANY, "Sky News"
        );

        wxButton* BBCButton = new wxButton(
            panel, wxID_ANY, "BBC News"
        );

        wxButton* FTButton = new wxButton(
            panel, wxID_ANY, "Financial Times"
        );

        wxButton* ReutersButton = new wxButton(
            panel, wxID_ANY, "Reuters News"
		);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* newsSelector = new wxBoxSizer(wxHORIZONTAL);
        m_fullText = richTextBox->GetValue();
        
        search = new wxSearchCtrl(
            panel, wxID_ANY, "",
            wxDefaultPosition, wxDefaultSize,
            wxTE_PROCESS_ENTER
		);

        wxArrayString lines = wxSplit(news, '\n');
        for (size_t i = 0; i < lines.size(); i++) {
            wxString line = lines[i];

            if (line.StartsWith("http")) {
                richTextBox->BeginURL(line);
                richTextBox->WriteText(line + "\n");
                richTextBox->EndURL();
            }
            else if (!line.IsEmpty()) {
                richTextBox->WriteText(line + "\n");
            }
            else
            {
				richTextBox->WriteText("\n");
            }
        };
		// Binding and initialisation
        button->Bind(wxEVT_BUTTON, &App::OnRefresh, this);
		BBCButton->Bind(wxEVT_BUTTON, &App::OnBBCNews, this);
        SkyButton->Bind(wxEVT_BUTTON, &App::OnSkyNews, this);
        FTButton->Bind(wxEVT_BUTTON, &App::OnFinancialTimes, this);
		ReutersButton->Bind(wxEVT_BUTTON, &App::OnReutersNews, this);
		search->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &App::OnSearch, this);
		sizer->Add(timeBox, 0, wxALIGN_CENTER | wxALL, 2);
        sizer->Add(downloadTime, 0, wxALIGN_CENTER | wxALL, 2);
        newsSelector->Add(SkyButton, 0, wxALIGN_CENTER | wxALL, 5);
        newsSelector->Add(BBCButton, 0, wxALIGN_CENTER | wxALL, 5);
        newsSelector->Add(FTButton, 0, wxALIGN_CENTER | wxALL, 5);
		newsSelector->Add(ReutersButton, 0, wxALIGN_CENTER | wxALL, 5);
        sizer->Add(newsSelector, 0, wxALIGN_CENTER | wxALL, 5);
        sizer->Add(richTextBox, 1, wxEXPAND | wxALL | wxTE_RICH | wxTE_AUTO_URL, 5);
        sizer->Add(button, 0, wxALIGN_CENTER | wxALL, 5);
		sizer->Add(search, 0, wxEXPAND | wxALL, 5);
        panel->SetSizer(sizer);
        richTextBox->Bind(wxEVT_TEXT_URL, &App::LinkClick, this);
        window->Show();

        return true;
    }

    void OnRefresh(wxCommandEvent&) {
        wxRichTextAttr boldAttr;
        boldAttr.SetFontWeight(wxFONTWEIGHT_BOLD);
        richTextBox->BeginStyle(boldAttr);
		richTextBox->Clear();
        richTextBox->WriteText("Now refreshing...\n");
        richTextBox->EndStyle();
        wxYield();

        // Checks which news station is currently selected and refreshes the presently selected station
            if (isSelected == "BBC News") {
                richTextBox->Clear();
                richTextBox->WriteText(NewsReader());
                // richTextBox->SetValue(NewsReader());
            } else if (isSelected == "Sky News") {
                richTextBox->Clear();
                richTextBox->WriteText(NewsReader());
                // richTextBox->SetValue(SkyNewsReader());
            } else if (isSelected == "Financial Times") {
                richTextBox->Clear();
                richTextBox->WriteText(NewsReader());
                // richTextBox->SetValue(FTReader());
            } else {
                richTextBox->Clear();
                richTextBox->WriteText(NewsReader());
                // richTextBox->SetValue(NewsReader());
            }

        end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        timeBox->SetValue(wxString(std::ctime(&end_time)) + "\n");
    }

    void OnBBCNews(wxCommandEvent&) {
		wxString news = NewsReader();

		wxArrayString lines = wxSplit(news, '\n');
        for (size_t i = 0; i < lines.size(); i++) {
            wxString line = lines[i];

            if (line.StartsWith("http")) {
                richTextBox->BeginURL(line);
                richTextBox->WriteText(line + "\n");
                richTextBox->EndURL();
            }
            else if (!line.IsEmpty()) {
                richTextBox->WriteText(line + "\n");
            }
            else
            {
				richTextBox->WriteText("\n");
            }
        };
  
        end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        timeBox->SetValue(wxString(std::ctime(&end_time)) + "\n");

        // Stores the currently selected news station in a variable for the OnRefresh function
        isSelected = "BBC News";
    }

    void OnSkyNews(wxCommandEvent&) {
        wxString news = SkyNewsReader();

        wxArrayString lines = wxSplit(news, '\n');
        for (size_t i = 0; i < lines.size(); i++) {
            wxString line = lines[i];

            if (line.StartsWith("http")) {
                richTextBox->BeginURL(line);
                richTextBox->WriteText(line + "\n");
                richTextBox->EndURL();
            }
            else if (!line.IsEmpty()) {
                richTextBox->WriteText(line + "\n");
            }
            else
            {
                richTextBox->WriteText("\n");
            }
        };
        end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        timeBox->SetValue(wxString(std::ctime(&end_time)) + "\n");

        // Stores the currently selected news station in a variable for the OnRefresh function
        isSelected = "Sky News";
	}

    void OnFinancialTimes(wxCommandEvent&) {
        wxRichTextAttr boldAttr;
        boldAttr.SetFontWeight(wxFONTWEIGHT_BOLD);
        richTextBox->BeginStyle(boldAttr);
        richTextBox->Clear();
        richTextBox->WriteText("Now refreshing...\n");
        richTextBox->EndStyle();
        wxYield();
        richTextBox->SetValue(FTReader());
        end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        timeBox->SetValue(wxString(std::ctime(&end_time)) + "\n");
        
        // Stores the curretly selected news station in a variable for the OnRefresh function
        isSelected = "Financial Times";
    }

    void OnReutersNews(wxCommandEvent&) {
        wxRichTextAttr boldAttr;
        boldAttr.SetFontWeight(wxFONTWEIGHT_BOLD);
        richTextBox->BeginStyle(boldAttr);
        richTextBox->Clear();
        richTextBox->WriteText("Now refreshing...\n");
        richTextBox->EndStyle();
        wxYield();
        richTextBox->SetValue(ReutersReader());
		end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		timeBox->SetValue(wxString(std::ctime(&end_time)) + "\n");

		// Stores the curretly selected news station in a variable for the OnRefresh function
		isSelected = "Reuters News";
    }

    void OnSearch(wxCommandEvent& event) {
        wxString query = event.GetString().Lower();

        if (query.IsEmpty()) {
            return;
        }

        wxString filtered;
        wxArrayString lines = wxSplit(m_fullText, '\n');

        bool keepBlock = false;

        for (const wxString& line : lines) {
            if (line.Lower().Contains(query)) {
                keepBlock = true;

                if (keepBlock) {
                    filtered += line + "\n";
                    if (line.StartsWith("-------------------------------------------------")) {
                        keepBlock = false;
                    }
                    richTextBox->Freeze();
                    richTextBox->SetValue(filtered);
                    richTextBox->Thaw();
                }
                if (filtered.IsEmpty())
                {
                    filtered = "No results found for " + query;
                    richTextBox->SetValue(filtered);
                };
            }
        }
    }

    wxString NewsReader() {
        // cURL initialisation
        CURL* curl = curl_easy_init();

        if (!curl) {
            return (stderr, "Failed to initialize cURL\n");
        }

        //cURL settings
        std::string xml_data;
        curl_easy_setopt(curl, CURLOPT_URL, R"(https://feeds.bbci.co.uk/news/rss.xml?edition=uk)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xml_data);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode base = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (base != CURLE_OK) {
            fprintf(stderr, "failed to download: %s\n", curl_easy_strerror(base));
        }

        if (xml_data.empty()) {
            return "No data downloaded...";
        }

        // create pugi document
        pugi::xml_document doc;
        // extracts from news.xml
        if (!doc.load_string(xml_data.c_str())) {
            return "Failed to parse XML data.";
        }

        // create buffer
        std::stringstream buffer;

        // parse XML and populate buffer
        // pugi::xml_parse_result result = doc.load_file("news.xml"));
        for (auto item : doc.child("rss").child("channel").children("item")) {
            buffer << item.child("title").text().get() << "\n";
            buffer << item.child("link").text().get() << "\n";
            buffer << item.child("description").text().get() << "\n";
            buffer << item.child("pubDate").text().get() << "\n";
            buffer << "-------------------------------------------------\n";
        }
        return wxString(buffer.str());
    }

    wxString SkyNewsReader() {
        // cURL initialisation
        CURL* curl = curl_easy_init();

        if (!curl) {
            return (stderr, "Failed to initialize cURL\n");
        }

        //cURL settings
        std::string xml_data;
        curl_easy_setopt(curl, CURLOPT_URL, R"(https://feeds.skynews.com/feeds/rss/uk.xml)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xml_data);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode base = curl_easy_perform(curl);

        // clean up curl
        curl_easy_cleanup(curl);

        if (base != CURLE_OK) {
            fprintf(stderr, "failed to download: %s\n", curl_easy_strerror(base));
        }

        if (xml_data.empty()) {
            return "No data downloaded...";
        }

        // create pugi document
        pugi::xml_document doc;
        // extracts from news.xml
        if (!doc.load_string(xml_data.c_str())) {
            return "Failed to parse XML data.";
        }

        // create buffer
        std::stringstream buffer;

        for (auto item : doc.child("rss").child("channel").children("item")) {
            buffer << item.child("title").text().get() << "\n";
            buffer << item.child("link").text().get() << "\n";
            buffer << item.child("description").text().get() << "\n";
            buffer << item.child("pubDate").text().get() << "\n";
            buffer << "-------------------------------------------------\n";
	}
        return wxString(buffer.str());
}

    wxString FTReader() {
        // cURL initialisation
        CURL* curl = curl_easy_init();

        if (!curl) {
            return (stderr, "Failed to initialize cURL\n");
        }

        //cURL settings
        std::string xml_data;
        curl_easy_setopt(curl, CURLOPT_URL, R"(https://www.ft.com/sitemaps/news.xml)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xml_data);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode base = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (base != CURLE_OK) {
            fprintf(stderr, "failed to download: %s\n", curl_easy_strerror(base));
        }

        if (xml_data.empty()) {
            return "No data downloaded...";
        }

        // create pugi document
        pugi::xml_document doc;
        // extracts from news.xml
        if (!doc.load_string(xml_data.c_str())) {
            return "Failed to parse XML data.";
        }

        // create buffer
        std::stringstream buffer;

        for (pugi::xml_node url : doc.child("urlset").children("url")) {
			
            // Financial times doesn't use typical XML, and instead uses a unique RSS feed. Requires an alternative method of parsing.
            buffer << url.child("lock").text().get() << "\n";

			pugi::xml_node news = url.child("news:news");
            if (!news) continue;
            
            buffer << news.child("news:title").text().get() << "\n";
            buffer << news.child("news:publication_date").text().get() << "\n";
            buffer << news.child("news:title").text().get() << "\n";
            buffer << news.child("news:keywords").text().get() << "\n";
            buffer << "-------------------------------------------------\n";
        }
        return wxString(buffer.str());
    }

    wxString ReutersReader() {
        // cURL initialisation
        CURL* curl = curl_easy_init();

        if (!curl) {
            return (stderr, "Failed to initialize cURL\n");
        }

        //cURL settings
        std::string xml_data;
        curl_easy_setopt(curl, CURLOPT_URL, R"(https://www.reuters.com/graphics/sitemap.xml)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xml_data);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode base = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (base != CURLE_OK) {
            fprintf(stderr, "failed to download: %s\n", curl_easy_strerror(base));
        }

        if (xml_data.empty()) {
            return "No data downloaded...";
        }

        // create pugi document
        pugi::xml_document doc;
        // extracts from news.xml
        if (!doc.load_string(xml_data.c_str())) {
            return "Failed to parse XML data.";
        }


        // create buffer
        std::stringstream buffer;

        for (pugi::xml_node url : doc.child("urlset").children("url")) {

            // Financial times doesn't use typical XML, and instead uses a unique RSS feed. Requires a new method of parsing.
            buffer << url.child("lock").text().get() << "\n";

            pugi::xml_node news = url.child("news:news");
            if (!news) continue;

            buffer << news.child("news:title").text().get() << "\n";
            buffer << news.child("news:publication_date").text().get() << "\n";
            buffer << news.child("news:title").text().get() << "\n";
            buffer << news.child("news:keywords").text().get() << "\n";
            buffer << "-------------------------------------------------\n";
        }
        return wxString(buffer.str());
    }

private:
    wxTextCtrl* textBox = nullptr;
    wxTextCtrl* downloadTime = nullptr;
	void LinkClick(wxTextUrlEvent& event);
	wxTextCtrl* timeBox = nullptr;
	wxRichTextCtrl* richTextBox = nullptr;
	wxSearchCtrl* searchBtn = nullptr;
	wxSearchCtrl* search = nullptr;
    std::time_t end_time;
    std::string isSelected = "";
    wxString m_fullText;
};

wxIMPLEMENT_APP(App);

// Opens browser upon link clicking
void App::LinkClick(wxTextUrlEvent& event) {
    wxString url = event.GetString();

    if (!url.IsEmpty()) {
        wxLaunchDefaultBrowser(url);
    }
}