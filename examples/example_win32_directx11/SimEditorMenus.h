#pragma once

#include "ParamUI.h"

static void SetAllParamsToSaved()
{
    for (auto id : perID_IsSaved)
    {
        *(id.second) = true;
    }
}

static bool SaveSimFile(const Document& editedSim, string filepath = "")
{
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    editedSim.Accept(writer);

    if (filepath == "")
    {
        if (mostRecentFilepath == "")
        {
            return false;
        }
        else // use mostRecentFilepath
        {
            filepath = mostRecentFilepath;
        }
    }
    else // use filepath
    {
        mostRecentFilepath = filepath;
    }


    std::ofstream of(filepath); //TODO
    of << sb.GetString();
    if (!of.good())
    {
        throw std::runtime_error("Can't write the JSON string to the file!");
        return false;
    }
    else // success
    {
        SetAllParamsToSaved();
        return true;
    }
}

static bool OpenSimFile(Document& simJSON, string filepath)
{
    std::stringstream ss;
    std::string buffer;
    std::ifstream infs(filepath);

    // check if file can be opened
    if (infs.is_open()) {
        while (!infs.eof()) {
            getline(infs, buffer);
            ss << buffer;
            ss << "\n";
        }
    }
    else {
        std::cout << "Error: File is not opening." << std::endl;
        return false;
    }

    std::string buffer_str = ss.str();
    const char* inputJson = buffer_str.c_str();

    if (simJSON.Parse(inputJson).HasParseError()) {
        std::cout << "ERROR: Invalid Json file" << std::endl;
        return false;
    }
    assert(simJSON.IsObject());
    assert(simJSON.HasMember("DataManager"));
    firstTimeThrough = true;
    return true; //success
}

/////////////////////////////////////////////////////////////////////////
static void SimEditorMenu(Document& editedSim)
{
    if (ImGui::BeginMenu("New")) {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Open..."))
    {
        ImGuiFileDialog::Instance()->OpenDialog("OpenDlgKey", "Open Simulation File...", ".json\0.*\0\0", ".");
    }

    // display Save As dialog
    if (ImGuiFileDialog::Instance()->FileDialog("OpenDlgKey"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk == true)
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
            std::string currentPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            OpenSimFile(editedSim, filePathName);
            mostRecentFilepath = filePathName;
        }
        // close
        ImGuiFileDialog::Instance()->CloseDialog("OpenDlgKey");
    }

    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Save", "Ctrl+S")) {
        bool success = SaveSimFile(editedSim);
        if (!success)
        {
            ImGui::OpenPopup("SaveFailurePopup");
        }
    }

    if (ImGui::BeginPopup("SaveFailurePopup"))
    {
        ImGui::Text("The file failed to Save. Would you like to Save As instead?");
        if (ImGui::Button("Save As..."))
        {
            ImGuiFileDialog::Instance()->OpenDialog("SaveAsDlgKey", "Save Simulation As...", ".json\0.*\0\0", ".");
        }
        ImGui::EndPopup();
    }


    if (ImGui::MenuItem("Save As..."))
    {
        ImGuiFileDialog::Instance()->OpenDialog("SaveAsDlgKey", "Save Simulation As...", ".json\0.*\0\0", ".");
    }

    // display Save As dialog
    if (ImGuiFileDialog::Instance()->FileDialog("SaveAsDlgKey"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk == true)
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
            std::string currentPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            SaveSimFile(editedSim, filePathName);
            mostRecentFilepath = filePathName;
        }
        // close
        ImGuiFileDialog::Instance()->CloseDialog("SaveAsDlgKey");
    }

    /*if (ImGui::MenuItem("Save As...")) {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp\0.h\0.hpp\0\0", ".");

    }*/


    if (ImGui::BeginMenu("Options"))
    {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}
