// MIT License
//
// Copyright (c) 2022 Mechatronics and Haptic Interfaces Lab - Rice University
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// Author(s): Zane Zook (zaz2@rice.edu) based on likert survey example by Evan Pezent


//-----------------------------------------------------------------------------
// preprocessor directives
//-----------------------------------------------------------------------------

/// mahi includes
#define MAHI_GUI_NO_CONSOLE // turns off conole output
#include <Mahi/Gui.hpp>     // MAHI libraries for GUI functionality
#include <Mahi/Util.hpp>    // MAHI libaries for clock, logging, and other functionality

// other includes
#include <fstream>

// definition of filesystem import based on os
#if defined(__linux__)
    #include <experimental/filesystem>  // for linux
    namespace fs = std::experimental::filesystem;
#else
    #include <filesystem>               // for windows
    namespace fs = std::filesystem;
#endif

// relevant MAHI namespaces to use
using namespace mahi::gui;
using namespace mahi::util;

//-----------------------------------------------------------------------------
// SUS survey application declaration
//-----------------------------------------------------------------------------
class SUS : public Application 
{
public:
    // private variables
    //-----------------------------------
    /// attribute for possible survey responses
    enum Response {
        NoResponse = -3,
        StronglyDisagree = -2,
        Disagree = -1,
        Neutral = 0,
        Agree = 1,
        StronglyAgree = 2
    };

    /// gui rendering related variables
    bool            loaded{false};  // was the config file loaded?
    std::string     title_;         // survey title
    float           scale_{1.0f};   // scale factor for the gui
    float           fontsize_{16};  // font size for the gui
    float           width_{1920};   // window width
    float           height_{1080};  // window height
    float width, height;            // window width/height
    float qWidth, qWidthOffset;     // question width/offset
    float rowHeight;                // row height
    ImFont*         font_;          // font pointer for the gui

    /// gui behavior variables
    std::string message;            // error message to display
    bool autoClose{false};          // should the app close when the user submits a response?
    
    /// survey variables
    std::vector<std::string> questions;  // survey questions
    std::vector<Response>    responses;  // survey responses
    std::string              subject;    // subject input text


    // construction methods
    //-----------------------------------
    /// constructor
    SUS() : Application(500,500,"",false) 
    { 
        ImGui::DisableViewports();
        loaded = load();
    }

    // update method
    //-----------------------------------
    /// SUS survey main update code
    void update() override {
        ImGui::BeginFixed("##SUS", {0,0}, {width, height}, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
        if (loaded) {
            
            // change font used
            ImGui::PushFont(font_);      

            // Subject Info
            ImGui::SetNextItemWidth(100*scale_);
            ImGui::InputText("Subject     ", &subject);
            ImGui::SameLine();
            ImGui::SameLine(ImGui::GetWindowWidth() - 105*scale_);
            if (ImGui::ButtonColored("Submit", Reds::FireBrick, {100*scale_,0})) {
                bool result = saveResponse();
                if (result && autoClose)
                    quit();
            }

            // Header
            ImGui::Separator();
            ImGui::Separator();
            ImGui::Text("\nQuestion");
            ImGui::SameLine(qWidth - 20*scale_);
            ImGui::Text("Strongly\nDiagree");
            ImGui::SameLine(qWidth + 65*scale_);
            ImGui::Text("\nDisagree");
            ImGui::SameLine(qWidth + 145*scale_);
            ImGui::Text("\nNeutral");
            ImGui::SameLine(qWidth + 230*scale_);
            ImGui::Text("\nAgree");
            ImGui::SameLine(qWidth + 305*scale_);
            ImGui::Text("Strongly\n Agree");

            // render questions
            float initialY = ImGui::GetCursorPos().y;
            for (unsigned int i = 0; i < questions.size(); ++i) {
                ImGui::PushID(i);
                ImGui::SetCursorPosY(initialY + rowHeight * i);
                ImGui::Separator();
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
                ImGui::Text("[Q.%02d]",i+1); 
                ImGui::PopStyleVar();
                ImGui::SameLine(); 
                ImGui::TextUnformatted(questions[i].c_str());
                ImGui::SameLine(qWidth);
                if (ImGui::RadioButton("##SD",responses[i] == StronglyDisagree))
                    responses[i] = StronglyDisagree;
                ImGui::SameLine(qWidth+80*scale_);
                if (ImGui::RadioButton("##D",responses[i] == Disagree))
                    responses[i] = Disagree;
                ImGui::SameLine(qWidth+160*scale_);
                if (ImGui::RadioButton("##N",responses[i] == Neutral))
                    responses[i] = Neutral;
                ImGui::SameLine(qWidth+240*scale_);
                if (ImGui::RadioButton("##A",responses[i] == Agree))
                    responses[i] = Agree;
                ImGui::SameLine(qWidth+320*scale_);
                if (ImGui::RadioButton("##SA",responses[i] == StronglyAgree))
                    responses[i] = StronglyAgree;
                ImGui::PopID();
            }

            // begin message modal if opened this frame
            bool dummy = true;
            if (ImGui::BeginPopupModal("Message", &dummy, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted(message.c_str());
                ImGui::EndPopup();
            }
        }
        else {
            // error output message
            ImGui::Text("SUS survey failed to load! :(");
        }
        ImGui::End();
    }

    // load methods
    //-----------------------------------
    /// load in SUS config file
    bool load() {
        // checks for config file
        if (fs::exists("SUS.json")) {
            // load in config settings
            try {
                // open and load in config file
                std::ifstream file("SUS.json");
                json j;
                file >> j;
                
                // convert relevant variables for use in GUI
                title_ = j["title"].get<std::string>();
                fontsize_   =   j["fontsize"].get<float>();
                questions = j["questions"].get<std::vector<std::string>>();
                autoClose = j["autoClose"].get<bool>();
                rowHeight = j.count("rowHeight") > 0 ? j["rowHeight"].get<float>() : 30;
                qWidthOffset = j.count("qWidthOffset") > 0 ? j["qWidthOffset"].get<float>() : 175;

                // scale based on desired font size compared to default
                scale_ = fontsize_ / 16.0f;

                // load font and set font size
                ImGuiIO& io     =   ImGui::GetIO();
                font_           =   io.Fonts->AddFontFromMemoryTTF(mahi::gui::Roboto_Bold_ttf, mahi::gui::Roboto_Bold_ttf_len, fontsize_);

                // calculate question width
                for (auto& q : questions)
                    qWidth = 7 * q.length() > qWidth ? 7 * q.length() : qWidth;
                qWidth += qWidthOffset;
                qWidth *= scale_;
                width = qWidth + 385 * scale_;
                height = 85 * scale_ + rowHeight * questions.size();
                responses = std::vector<Response>(questions.size(), NoResponse); 

                // set window title and size
                set_window_title(title_);
                set_window_size((int)width, (int)height);
                center_window();
            }
            catch(...) {
                return false;
            }
            return true;
        }
        return false;
    }

    // save methods
    //-----------------------------------
    /// save and export survey responses
    bool saveResponse() 
    {
        // make sure subject has value
        if (subject == "") {
            message = "Please enter your subject identifier";
            ImGui::OpenPopup("Message");
            return false;
        }
        // make sure every question answered
        for (unsigned int i = 0; i < responses.size();  ++i) {
            if (responses[i] == NoResponse) {
                message = "Please respond to Question " + std::to_string(i+1);
                ImGui::OpenPopup("Message");
                return false;
            }
        }
        // generate text responses
        std::vector<std::string> responsesText(responses.size());
        static std::map<Response, std::string> responseMap = {
            {StronglyDisagree, "Strongly Disagree"}, 
            {Disagree, "Disagree"},
            {Neutral, "Neutral"},
            {Agree, "Agree"},
            {StronglyAgree, "Strongly Agree"}
        };
        for (unsigned int i = 0; i < responses.size(); ++i)
            responsesText[i] = responseMap[responses[i]];
        // save data
        json j;
        j["subject"] = subject;
        j["responses"] = responses;
        j["responsesText"] = responsesText;

        // get next available filename
        std::string filename = "Subject" + subject + "_SUS" + ".json";
        filename = getNextFilename(filename);
        std::ofstream file(filename);
        if (file.is_open())
            file << std::setw(4) << j << std::endl;

        // reset state
        subject = "";
        responses = std::vector<Response>(responses.size(), NoResponse);
        message = "Thank you for participating!";
        ImGui::OpenPopup("Message");
        return true;
    }
    /// returns the next available filename without overwriting
    std::string getNextFilename(std::string filename)
    {
        int i = 0;
        while(fs::exists(filename))
        {
            if(i == 0)
                filename = filename.substr(0, filename.find_last_of(".")) + "_" + std::to_string(i) + filename.substr(filename.find_last_of("."));
            else
                filename = filename.substr(0, filename.find_last_of("_")) + "_" + std::to_string(i) + filename.substr(filename.find_last_of("."));
            ++i;
        }
        std::cout << "Saving to " << filename << std::endl;
        return filename;
    }
};


//-----------------------------------------------------------------------------
// main method
//-----------------------------------------------------------------------------
int main(int argc, char const *argv[])
{
    // if there doesn't exist a "SUS.json" file, make a default one
    if (!fs::exists("SUS.json")) {
        json j;
        j["title"] = "System Usability Survey";
        j["fontsize"]  =  16;
        j["questions"] = {"I think I would like to use this device frequently.", 
                          "I found the device unnecessarily complex.",
                          "I thought the device was easy to use.",
                          "I think that I would need the support of a technical person to use this device.",
                          "I found the various functions in this device were well integrated.",
                          "I thought there was too much inconsistency in this device.",
                          "I would imagine that most people would learn to use this device very quickly.",
                          "I found the device very cumbersome to use.",
                          "I felt very confident using the tool.",
                          "I needed to learn a lot of things before I could get going with this device."};
        j["autoClose"] = true;
        j["rowHeight"] = 30;
        j["qWidthOffset"] = 175;
        std::ofstream file("SUS.json");
        if (file.is_open())
            file << std::setw(4) << j;
    }

    // run the survey application
    SUS sus;
    sus.run();
    return 0;
}