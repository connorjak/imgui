#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "imgui.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

// Only for this: https://github.com/ocornut/imgui/issues/211#issuecomment-339241929
#include "imgui_internal.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using std::string;
using std::to_string;
using std::map;
using namespace rapidjson;

#define MAXIMUM_NESTING_DEPTH 500
#define MAXIMUM_STRING_LENGTH 10000

static const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };

static int colors[6][3] = {
    { 255, 255, 255 },  //White
    { 255, 225, 25 },   //Yellow
    {0, 130, 200},      //Blue
    {245, 130, 48},     //Orange
    {250, 190, 190},    //
    {230, 190, 255}/*,  //
    {128, 128, 128}*/   //Grey
};

//Statics to get around recursion (global)

static bool firstTimeThrough = true;
static map<ImGuiID, bool*> perID_IsEditable;

void recurseParamTree(const char * nameOfSet, Value& paramSet, int depth)
{
    bool treeActive;
    if (depth == 0)
    {
        treeActive = ImGui::TreeNodeEx(nameOfSet, ImGuiTreeNodeFlags_CollapsingHeader);
    }
    else
    {
        /*treeActive = ImGui::TreeNode(nameOfSet);*/
        treeActive = ImGui::TreeNodeEx(nameOfSet, ImGuiTreeNodeFlags_CollapsingHeader);
    }

    if (treeActive && depth < MAXIMUM_NESTING_DEPTH)
    {
        ImGuiInputTextFlags inputFieldFlags = NULL;
        ImGui::Indent();

        assert(paramSet.IsObject());
        
        
        for (auto& param : paramSet.GetObject())
        {
            ImGui::PushID(&param);

            ImGuiID currentID = ImGui::GetID(&param);

            if (perID_IsEditable.find(currentID) == perID_IsEditable.end())
            {
                perID_IsEditable[currentID] = new bool(false);
            }
            bool disabled;

            

            int c = depth % 6;

            const char* key = param.name.GetString();
            Value& val = param.value;
            const char* valTypeName = kTypeNames[val.GetType()];

            if (valTypeName != "Object")
            {
                ImGui::Checkbox(*perID_IsEditable[currentID] ? " " : "X", perID_IsEditable[currentID]);
                ImGui::SameLine();
                disabled = !*perID_IsEditable[currentID];
                if (disabled)
                {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.6f);
                }

                ImVec4 textcol = { colors[c][0] / 255.0f, colors[c][1] / 255.0f, colors[c][2] / 255.0f, 1.0f };
                ImGui::TextColored(textcol, key);

                if (valTypeName == "Array")
                {
                    ImGui::SameLine();

                    //TODO better handling of this than just printing JSON

                    StringBuffer sb;
                    PrettyWriter<StringBuffer> writer(sb);
                    val.Accept(writer);

                    ImGui::Text(sb.GetString());
                }
                if (valTypeName == "String")
                {
                    ImGui::SameLine();
                    const char* currentval = val.GetString();

                    size_t length = strlen(currentval);
                    char* outval = new char[length + 1](); //TODO memory leak here!
                    strncpy(outval, currentval, length);

                    ImGui::InputText("(String)", outval, MAXIMUM_STRING_LENGTH, inputFieldFlags);
                    Value outvalJ;
                    outvalJ.SetString(outval, length);
                    paramSet[key] = outvalJ;
                    //delete outval;//TODO ?
                }
                if (valTypeName == "Number")
                {
                    ImGui::SameLine();
                    double outval = val.GetDouble();
                    ImGui::InputDouble("(Double)", &outval, 1.0, 100.0, "%.6f", inputFieldFlags);
                    Value outvalJ(outval);
                    paramSet[key] = outvalJ;
                }
                if (valTypeName == "True" || valTypeName == "False")
                {
                    ImGui::SameLine();
                    bool outval = val.GetBool();
                    ImGui::Checkbox("", &outval);
                    Value outvalJ(outval);
                    paramSet[key] = outvalJ;

                }
                else // Unimplemented param type
                {
                    ImGui::SameLine();

                    //TODO better handling of this than just printing JSON

                    StringBuffer sb;
                    PrettyWriter<StringBuffer> writer(sb);
                    val.Accept(writer);

                    ImGui::Text(sb.GetString());
                }
            }
            else // is JSON Object type
            {
                disabled = false;
                //Replace label extension (for better ID differences)
                string treeLabel = key;
                string delimiter = "##";
                string token = treeLabel.substr(0, treeLabel.find(delimiter));
                treeLabel = token + "##" + to_string(depth);

                recurseParamTree(treeLabel.c_str(), val, depth + 1);
            }

            if (disabled)
            {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }

            ImGui::PopID();
        } // End loop through params in set

        // End of tree area stuff
        if (depth != 0)
        {
            //ImGui::TreePop();
            /*ImGui::Unindent();*/
        }
        ImGui::Unindent();
    }
    firstTimeThrough = false;
}


static void SimEditorMenu_File(const Document& editedSim)
{
    if (ImGui::BeginMenu("New")) {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Open", "Ctrl+O")) {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        editedSim.Accept(writer);

        std::ofstream of("editedSimulation.json"); //TODO
        of << sb.GetString();
        if (!of.good()) throw std::runtime_error("Can't write the JSON string to the file!");
    }
    if (ImGui::MenuItem("Save As...")) {
        // open Dialog Simple
        if (ImGui::Button("Open File Dialog"))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp\0.h\0.hpp\0\0", ".");

        // display
        if (ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                // action
            }
            // close
            ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
        }

        ImGui::EndMenu();
    }

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        ImGui::MenuItem("(unimplemented)", NULL, false, false);
        
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}










////(Copy pasted)
//// Note that shortcuts are currently provided for display only (future version will add flags to BeginMenu to process shortcuts)
//static void ShowExampleMenuFile()
//{
//    ImGui::MenuItem("(dummy menu)", NULL, false, false);
//    if (ImGui::MenuItem("New")) {}
//    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
//    if (ImGui::BeginMenu("Open Recent"))
//    {
//        ImGui::MenuItem("fish_hat.c");
//        ImGui::MenuItem("fish_hat.inl");
//        ImGui::MenuItem("fish_hat.h");
//        if (ImGui::BeginMenu("More.."))
//        {
//            ImGui::MenuItem("Hello");
//            ImGui::MenuItem("Sailor");
//            if (ImGui::BeginMenu("Recurse.."))
//            {
//                ShowExampleMenuFile();
//                ImGui::EndMenu();
//            }
//            ImGui::EndMenu();
//        }
//        ImGui::EndMenu();
//    }
//    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
//    if (ImGui::MenuItem("Save As..")) {}
//
//    ImGui::Separator();
//    if (ImGui::BeginMenu("Options"))
//    {
//        static bool enabled = true;
//        ImGui::MenuItem("Enabled", "", &enabled);
//        ImGui::BeginChild("child", ImVec2(0, 60), true);
//        for (int i = 0; i < 10; i++)
//            ImGui::Text("Scrolling Text %d", i);
//        ImGui::EndChild();
//        static float f = 0.5f;
//        static int n = 0;
//        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
//        ImGui::InputFloat("Input", &f, 0.1f);
//        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
//        ImGui::EndMenu();
//    }
//
//    if (ImGui::BeginMenu("Colors"))
//    {
//        float sz = ImGui::GetTextLineHeight();
//        for (int i = 0; i < ImGuiCol_COUNT; i++)
//        {
//            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
//            ImVec2 p = ImGui::GetCursorScreenPos();
//            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
//            ImGui::Dummy(ImVec2(sz, sz));
//            ImGui::SameLine();
//            ImGui::MenuItem(name);
//        }
//        ImGui::EndMenu();
//    }
//
//    // Here we demonstrate appending again to the "Options" menu (which we already created above)
//    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
//    // In a real code-base using it would make senses to use this feature from very different code locations.
//    if (ImGui::BeginMenu("Options")) // <-- Append!
//    {
//        static bool b = true;
//        ImGui::Checkbox("SomeOption", &b);
//        ImGui::EndMenu();
//    }
//
//    if (ImGui::BeginMenu("Disabled", false)) // Disabled
//    {
//        IM_ASSERT(0);
//    }
//    if (ImGui::MenuItem("Checked", NULL, true)) {}
//    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
//}
