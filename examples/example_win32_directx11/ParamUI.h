#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "imgui.h"


// Only for this: https://github.com/ocornut/imgui/issues/211#issuecomment-339241929
#include "imgui_internal.h"

//#include "ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using std::string;
using std::to_string;
using std::map;
using namespace rapidjson;

#define MAX_NESTING_DEPTH 500
#define MAX_STRING_LENGTH 10000

static const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };

static const char* paramTypes[] =
{ "Array", "Double", "String", "Bool"};

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
static map<ImGuiID, bool*> perID_IsSaved;
//string mostRecentFilename = "";
string mostRecentFilepath = "";

//////////////////////////////////////////

// User must PopStyleColor!
static bool ColorIfUnsaved(ImGuiID currentID)
{
    if (!*perID_IsSaved[currentID])
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(255 / 255.0f, 225 / 255.0f, 25 / 255.0f, 70 / 255.0f));
        return true;
    }
    return false;
}

static void AddParamSection(Value& paramSet, Document& simJSON)
{
    static char newParam_name[254];
    static int newParam_typenum;

    static char newParam_string[MAX_STRING_LENGTH];
    static double newParam_double;
    static bool newParam_bool;

    //if (firstTimeThrough)
    //{
    //    newParam_typenum = 0;
    //    //...
    //}

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.08f);

    bool addThisParam = ImGui::Button("+ Add Param"); ImGui::SameLine();
    ImGui::Text("Type"); ImGui::SameLine();
    ImGui::Combo("##Type", &newParam_typenum, paramTypes, 4); ImGui::SameLine();

    //static const char* paramTypes[] =
    //{ "Array", "Double", "String", "Bool"};
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
    ImGui::Text("Key"); ImGui::SameLine();
    ImGui::InputText("##Key", newParam_name, MAX_STRING_LENGTH); ImGui::SameLine();

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.30f);

    Value newVal;

    ImGui::Text("Value"); ImGui::SameLine();
    switch (newParam_typenum)
    {
    case 0: //Array
    {
        ImGui::Text("["); ImGui::SameLine();
        ImGui::InputText("##Value", newParam_string, MAX_STRING_LENGTH); ImGui::SameLine();
        ImGui::Text("]");

        //Added to this one for performance
        if (addThisParam)
        {
            Document doc;
            string toParse = "[" + string(newParam_string) + "]";
            if (doc.Parse(toParse.c_str()).HasParseError()) {
                std::cout << "ERROR: Invalid Json structure for array" << std::endl;
                //TODO better error
                addThisParam = false;

            }
            else if (!doc.IsArray())
            {
                std::cout << "ERROR: Invalid Json structure for array" << std::endl;
                addThisParam = false;
            }
            else
            {
                newVal = doc.GetArray();
            }
        }
    }
        break;
    case 1: //Double
        ImGui::InputDouble("##Value", &newParam_double);

        newVal.SetDouble(newParam_double);
        break;
    case 2: //String
        ImGui::InputText("##Value", newParam_string, MAX_STRING_LENGTH);

        newVal.SetString(newParam_string, strlen(newParam_string));
        break;
    case 3: //Bool
        ImGui::Checkbox("##Value", &newParam_bool);

        newVal.SetBool(newParam_bool);
        break;
    default:
        throw std::logic_error("Impossible new parameter type number.");
    }

    ImGui::PopItemWidth();

    if (addThisParam)
    {
        //Checks
        assert(paramSet.IsObject());
        if (strcmp("", newParam_name) == 0)
            return;
        for (auto& param : paramSet.GetObject())
        {
            if (strcmp(param.name.GetString(), newParam_name) == 0)
                return;
        }
        string tmpstr = newParam_name;
        const char* tmpcharptr = tmpstr.c_str();
        Value tmpVal(tmpcharptr, strlen(tmpcharptr), simJSON.GetAllocator());
        //paramSet.AddMember("Test", newVal, simJSON.GetAllocator());
        //paramSet.AddMember(Value(newParam_name, strlen(newParam_name)), newVal, simJSON.GetAllocator());
        paramSet.AddMember(tmpVal, newVal, simJSON.GetAllocator());
        //TODO the reallocation in here is resetting the paramSet-based IDs. 
    }

}

void recurseParamTree(const char * nameOfSet, Value& paramSet, Document& simJSON, int depth)
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

    //Do this outside the tree structure so it doesn't get passed over
    if (firstTimeThrough)
    {
        for (auto& param : paramSet.GetObject())
        {
            ImGui::PushID(&param);

            ImGuiID currentID = ImGui::GetID(&param);

            if (perID_IsEditable.find(currentID) == perID_IsEditable.end())
            {
                perID_IsEditable[currentID] = new bool(false);
            }
            if (perID_IsSaved.find(currentID) == perID_IsSaved.end())
            {
                perID_IsSaved[currentID] = new bool(true);   
            }

            Value& val = param.value;
            const char* valTypeName = kTypeNames[val.GetType()];

            // Recurse if object
            if (valTypeName == "Object")
            {
                const char* key = param.name.GetString();
                //Replace label extension (for better ID differences)
                string treeLabel = key;
                string delimiter = "##";
                string token = treeLabel.substr(0, treeLabel.find(delimiter));
                treeLabel = token + "##" + to_string(depth);
                recurseParamTree(treeLabel.c_str(), val, simJSON, depth + 1);
            }

            ImGui::PopID();
        }
        return; //Break out early first time
    }


    if (treeActive && depth < MAX_NESTING_DEPTH)
    {
        ImGuiInputTextFlags inputFieldFlags = NULL;
        ImGui::Indent();

        assert(paramSet.IsObject());
        
        AddParamSection(paramSet, simJSON);

        for (auto& param : paramSet.GetObject())
        {
            ImGui::PushID(&param);
            //ImGui::PushID(&(param.value));
            //ImGui::PushID(param.name.GetString());

            ImGuiID currentID = ImGui::GetID(&param);
            //ImGuiID currentID = ImGui::GetID(&(param.value));
            //ImGuiID currentID = ImGui::GetID(param.name.GetString());

            if (perID_IsEditable.find(currentID) == perID_IsEditable.end())
            {
                perID_IsEditable[currentID] = new bool(false);
            }
            if (perID_IsSaved.find(currentID) == perID_IsSaved.end())
            {
                perID_IsSaved[currentID] = new bool(false);
            }
            // is parameter locked currently
            bool paramIsLocked;
            // Did we push a style color mid-param viewing
            bool pushedStyleColor = false;

            int c = depth % 6;

            const char* key = param.name.GetString();
            Value& val = param.value;
            const char* valTypeName = kTypeNames[val.GetType()];

            if (valTypeName != "Object")
            {
                ImGui::Checkbox(*perID_IsEditable[currentID] ? " " : "X", perID_IsEditable[currentID]);
                ImGui::SameLine();
                paramIsLocked = !*perID_IsEditable[currentID];
                if (paramIsLocked)
                {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.6f);
                }

                ImVec4 textcol = { colors[c][0] / 255.0f, colors[c][1] / 255.0f, colors[c][2] / 255.0f, 1.0f };
                ImGui::TextColored(textcol, key);

                if (valTypeName == "Array")
                {
                    ImGui::SameLine();
                    bool fallbackRendering = false;
                    string arrayValType = "not initialized";


                    if (!val.IsArray()) {fallbackRendering = true;}
                    else if(strcmp(key,"Systems") == 0) { fallbackRendering = true; }
                    else if(val.Size() == 0) { fallbackRendering = true; }
                    else
                    {
                        if(!val.GetArray()[0].IsString()) { fallbackRendering = true; }
                        else
                        {
                            arrayValType = val.GetArray()[0].GetString();
                        }
                    }

                    if (!fallbackRendering)
                    {
                        if (arrayValType == "doubleV2")
                        {
                            //TODO check length of array
                            ImGui::SameLine();
                            double outval[2] = { val.GetArray()[1].GetDouble(),
                                val.GetArray()[2].GetDouble() };

                            pushedStyleColor = ColorIfUnsaved(currentID);
                            ImGui::InputDouble3("(DoubleV2)", outval);

                            if (val.GetArray()[1].GetDouble() != outval[0] ||
                                val.GetArray()[2].GetDouble() != outval[1])
                            {
                                *perID_IsSaved[currentID] = false;
                            }

                            Value outvalJ(kArrayType);
                            outvalJ.PushBack("doubleV2", simJSON.GetAllocator());
                            outvalJ.PushBack(outval[0], simJSON.GetAllocator());
                            outvalJ.PushBack(outval[1], simJSON.GetAllocator());

                            paramSet[key] = outvalJ;
                        }
                        else if (arrayValType == "doubleV3")
                        {
                            //TODO check length of array
                            ImGui::SameLine();
                            double outval[3] = { val.GetArray()[1].GetDouble(),
                                val.GetArray()[2].GetDouble(),
                                val.GetArray()[3].GetDouble() };

                            pushedStyleColor = ColorIfUnsaved(currentID);
                            ImGui::InputDouble3("(DoubleV3)", outval);

                            if (val.GetArray()[1].GetDouble() != outval[0] ||
                                val.GetArray()[2].GetDouble() != outval[1] ||
                                val.GetArray()[3].GetDouble() != outval[2])
                            {
                                *perID_IsSaved[currentID] = false;
                            }

                            Value outvalJ(kArrayType);
                            outvalJ.PushBack("doubleV3", simJSON.GetAllocator());
                            outvalJ.PushBack(outval[0], simJSON.GetAllocator());
                            outvalJ.PushBack(outval[1], simJSON.GetAllocator());
                            outvalJ.PushBack(outval[2], simJSON.GetAllocator());

                            paramSet[key] = outvalJ;
                        }
                        else if (arrayValType == "doubleV4")
                        {
                            //TODO check length of array
                            ImGui::SameLine();
                            double outval[4] = { val.GetArray()[1].GetDouble(),
                                val.GetArray()[2].GetDouble(),
                                val.GetArray()[3].GetDouble(),
                                val.GetArray()[4].GetDouble() };

                            pushedStyleColor = ColorIfUnsaved(currentID);
                            ImGui::InputDouble4("(DoubleV4)", outval);

                            if (val.GetArray()[1].GetDouble() != outval[0] ||
                                val.GetArray()[2].GetDouble() != outval[1] ||
                                val.GetArray()[3].GetDouble() != outval[2] ||
                                val.GetArray()[4].GetDouble() != outval[3])
                            {
                                *perID_IsSaved[currentID] = false;
                            }

                            Value outvalJ(kArrayType);
                            outvalJ.PushBack("doubleV4", simJSON.GetAllocator());
                            outvalJ.PushBack(outval[0], simJSON.GetAllocator());
                            outvalJ.PushBack(outval[1], simJSON.GetAllocator());
                            outvalJ.PushBack(outval[2], simJSON.GetAllocator());
                            outvalJ.PushBack(outval[3], simJSON.GetAllocator());

                            paramSet[key] = outvalJ;
                        }
                        else
                        {
                            fallbackRendering = true;
                        }
                    }

                    if (fallbackRendering)
                    {
                        StringBuffer sb;
                        PrettyWriter<StringBuffer> writer(sb);
                        val.Accept(writer);

                        ImGui::Text(sb.GetString());
                    }
                }
                else if (valTypeName == "String")
                {
                    ImGui::SameLine();
                    const char* currentval = val.GetString();

                    size_t length = strlen(currentval);
                    char* outval = new char[length + 1](); //TODO memory leak here!
                    strncpy(outval, currentval, length);

                    
                    pushedStyleColor = ColorIfUnsaved(currentID);
                    ImGui::InputText("(String)", outval, MAX_STRING_LENGTH, inputFieldFlags);

                    // If it changed
                    if (strcmp(currentval, outval) != 0)
                    {
                        *perID_IsSaved[currentID] = false;
                    }

                    Value outvalJ;
                    outvalJ.SetString(outval, length);
                    paramSet[key] = outvalJ;
                    //delete outval;//TODO ?
                }
                else if (valTypeName == "Number")
                {
                    ImGui::SameLine();
                    double outval = val.GetDouble();

                    pushedStyleColor = ColorIfUnsaved(currentID);
                    ImGui::InputDouble("(Double)", &outval, 1.0, 100.0, "%.6f", inputFieldFlags);

                    if (val.GetDouble() != outval)
                    {
                        *perID_IsSaved[currentID] = false;
                    }

                    Value outvalJ(outval);
                    paramSet[key] = outvalJ;
                }
                else if (valTypeName == "True" || valTypeName == "False")
                {
                    ImGui::SameLine();
                    bool outval = val.GetBool();
                    ImGui::Checkbox("", &outval);

                    pushedStyleColor = ColorIfUnsaved(currentID);
                    if (val.GetBool() != outval)
                    {
                        *perID_IsSaved[currentID] = false;
                    }

                    Value outvalJ(outval);
                    paramSet[key] = outvalJ;

                }
                else // Unimplemented param type (probably impossible due to this being JSON types?)
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
                paramIsLocked = false;
                //Replace label extension (for better ID differences)
                string treeLabel = key;
                string delimiter = "##";
                string token = treeLabel.substr(0, treeLabel.find(delimiter));
                treeLabel = token + "##" + to_string(depth);

                recurseParamTree(treeLabel.c_str(), val, simJSON, depth + 1);
            }

            if (paramIsLocked)
            {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }

            if (pushedStyleColor)
            {
                ImGui::PopStyleColor();
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
    
}
