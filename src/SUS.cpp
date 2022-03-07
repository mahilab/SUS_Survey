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

#define MAHI_GUI_NO_CONSOLE
#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>
#include <fstream>

#if defined(__linux__)
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #include <filesystem>
    namespace fs = std::filesystem;
#endif

using namespace mahi::gui;
using namespace mahi::util;

//-----------------------------------------------------------------------------
// SUS survey application declaration
//-----------------------------------------------------------------------------
class SUS : public Application {
public:

    enum Sex { NoGender, Male, Female };

    enum Response {
        NoResponse = -3,
        StronglyDisagree = -2,
        Disagree = -1,
        Neutral = 0,
        Agree = 1,
        StronglyAgree = 2
    };


    std::string subject;                 ///< subject input text
    bool loaded = false;                 ///< was the Likert config loaded?
    std::string title;                   ///< survey title
    std::vector<std::string> questions;  ///< survey questions
    std::vector<Response> responses;     ///< survey responses
    Sex sex = NoGender;                  ///< subject's biological sex
    int age = -1;                        ///< subject age
    bool autoClose = false;              ///< should the app close when the user submits a response?
    float width, height;                 ///< window width/height
    float qWidth, rowHeight;;            ///< row width/height
    std::string message;                 ///< error message to display


    // construction methods
    //-----------------------------------
    /// constructor
    SUS() : Application(500,500,"",false) { 
        ImGui::DisableViewports();
        loaded = load();
    }

    // update method
    //-----------------------------------
    /// SUS survey main update code
    void update() override {
        ImGui::BeginFixed("##SUS", {0,0}, {width, height}, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
        if (loaded) {
            // Subject Info
            ImGui::SetNextItemWidth(100);
            ImGui::InputText("Subject     ", &subject);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50);
            if (ImGui::BeginCombo("Age     ", age != -1 ? std::to_string(age).c_str() : "")) {
                for (int i = 0; i < 100; ++i) {
                    if (ImGui::Selectable(std::to_string(i).c_str(), i == age))
                        age = i;
                    if (age == i)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Male", sex == Male))
                sex = Male;
            ImGui::SameLine();
            if (ImGui::RadioButton("Female", sex == Female))
                sex = Female;
            ImGui::SameLine();
            ImGui::SameLine(ImGui::GetWindowWidth() - 105);
            if (ImGui::ButtonColored("Submit", Reds::FireBrick, {100,0})) {
                bool result = saveResponse();
                if (result && autoClose)
                    quit();
            }
            // Header
            ImGui::Separator();
            ImGui::Separator();
            ImGui::Text("\nQuestion");
            ImGui::SameLine(qWidth - 20);
            ImGui::Text("Strongly\nDiagree");
            ImGui::SameLine(qWidth + 65);
            ImGui::Text("\nDisagree");
            ImGui::SameLine(qWidth + 145);
            ImGui::Text("\nNeutral");
            ImGui::SameLine(qWidth + 230);
            ImGui::Text("\nAgree");
            ImGui::SameLine(qWidth + 305);
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
                ImGui::SameLine(qWidth+80);
                if (ImGui::RadioButton("##D",responses[i] == Disagree))
                    responses[i] = Disagree;
                ImGui::SameLine(qWidth+160);
                if (ImGui::RadioButton("##N",responses[i] == Neutral))
                    responses[i] = Neutral;
                ImGui::SameLine(qWidth+240);
                if (ImGui::RadioButton("##A",responses[i] == Agree))
                    responses[i] = Agree;
                ImGui::SameLine(qWidth+320);
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
            ImGui::Text("SUS survey failed to load! :(");
        }
        ImGui::End();
    }

    // load methods
    //-----------------------------------
    /// load in SUS config file
    bool load() {
        if (fs::exists("SUS.json")) {
            try {
                std::ifstream file("SUS.json");
                json j;
                file >> j;
                title = j["title"].get<std::string>();
                questions = j["questions"].get<std::vector<std::string>>();
                autoClose = j["autoClose"].get<bool>();
                rowHeight = j.count("rowHeight") > 0 ? j["rowHeight"].get<float>() : 30;
                for (auto& q : questions)
                    qWidth = 7 * q.length() > qWidth ? 7 * q.length() : qWidth;
                qWidth += 75;
                width = qWidth + 385;
                height = 85 + rowHeight * questions.size();
                responses = std::vector<Response>(questions.size(), NoResponse); 
                set_window_title(title);
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
        // make sure subject has age
        if (age == -1) {
            message = "Please enter your age";
            ImGui::OpenPopup("Message");
            return false;
        }
        if (sex == NoGender) {
            message = "Please enter your biological sex";
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
        j["age"] = age;
        j["sex"] = (sex == Male ? "Male" : "Female");
        j["responses"] = responses;
        j["responsesText"] = responsesText;
        std::ofstream file(subject + ".json");
        if (file.is_open())
            file << std::setw(4) << j << std::endl;
        // reset state
        subject = "";
        sex = NoGender;
        age = -1;
        responses = std::vector<Response>(responses.size(), NoResponse);
        message = "Thank you for participating!";
        ImGui::OpenPopup("Message");
        return true;
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
        std::ofstream file("SUS.json");
        if (file.is_open())
            file << std::setw(4) << j;
    }

    // run the survey application
    SUS sus;
    sus.run();
    return 0;
}