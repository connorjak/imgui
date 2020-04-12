#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
//#include <list>
#include <vector>

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
//using std::list;
using std::vector;
using namespace rapidjson;

#define MAX_NESTING_DEPTH 500
#define MAX_STRING_LENGTH 10000

static const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };

static const char* paramTypes[] =
{ "Bool", "String", "Double", "DoubleV2", "DoubleV3", "DoubleV4", "Int32" };
//Array removed for now

static int colors[6][3] = {
    { 255, 255, 255 },  //White
    { 255, 225, 25 },   //Yellow
    {0, 130, 200},      //Blue
    {245, 130, 48},     //Orange
    {250, 190, 190},    //Pink
    {230, 190, 255}/*,  //Purple
    {128, 128, 128}*/   //Grey
};

static ImVec4 ImVecFromIntColor(int color[3])
{
    return ImVec4(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, 1.0f);
}

//Statics to get around recursion (global)

static bool firstTimeThrough = true;
static map<ImGuiID, bool*> perID_IsEditable;
static map<ImGuiID, bool*> perID_IsSaved;

static vector<string> cachedSysNametags;

//string mostRecentFilename = "";
string mostRecentFilepath = "";

//////////////////////////////////////////

// User must PopStyleColor!
static bool ColorIfUnsaved(ImGuiID currentID)
{
    if (perID_IsSaved.find(currentID) == perID_IsSaved.end())
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(255 / 255.0f, 225 / 255.0f, 25 / 255.0f, 70 / 255.0f));
        return true;
    }
    if (!*perID_IsSaved[currentID])
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(255 / 255.0f, 225 / 255.0f, 25 / 255.0f, 70 / 255.0f));
        return true;
    }
    return false;
}

static bool IsAnythingUnsaved()
{
    bool unsavedFlag = false;
    for (auto item : perID_IsSaved)
    {
        if (*(item.second) == false)
            unsavedFlag = true;
        //TODO count of unsaved items?
    }
    return unsavedFlag;
}

static void AddSystemsSection(Value& systemsArray, Document& simJSON)
{
    static char newSystem_nametag[254];
    static char newSystem_source[254]; //TODO filepath length issues???
    static char newSystem_type[254];
    static double newSystem_freq;

    //if (firstTimeThrough)
    //{
    //    newParam_typenum = 0;
    //    //...
    //}

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.12f);

    bool addThisSystem = ImGui::Button("+ Add System"); ImGui::SameLine();
    ImGui::Text("Nametag"); ImGui::SameLine();
    ImGui::InputText("##SystemNametag", newSystem_nametag, MAX_STRING_LENGTH); ImGui::SameLine();

    ImGui::Text("Source"); ImGui::SameLine();
    ImGui::InputText("##SystemSource", newSystem_source, MAX_STRING_LENGTH); ImGui::SameLine();

    //TODO enumerated types?
    ImGui::Text("Type"); ImGui::SameLine();
    ImGui::InputText("##SystemType", newSystem_type, MAX_STRING_LENGTH); ImGui::SameLine();

    ImGui::Text("Frequency"); ImGui::SameLine();
    ImGui::InputDouble("##SystemFreq", &newSystem_freq,1,2,"%0.15f");

    ImGui::PopItemWidth();

    if (addThisSystem)
    {
        //Checks
        assert(systemsArray.IsArray());
        if (strcmp("", newSystem_nametag) == 0)
            return;
        if (strcmp("", newSystem_source) == 0)
            return;
        for (auto& system : systemsArray.GetArray())
        {
            if (system.HasMember("Nametag"))
            {
                if (strcmp(system["Nametag"].GetString(), newSystem_nametag) == 0)
                    return;
            }
        }
        string nameStr = newSystem_nametag;
        const char* nameCstr = nameStr.c_str();
        Value nameValue(nameCstr, strlen(nameCstr), simJSON.GetAllocator());

        string sourceStr = newSystem_source;
        const char* sourceCstr = sourceStr.c_str();
        Value sourceValue(sourceCstr, strlen(sourceCstr), simJSON.GetAllocator());

        string typeStr = newSystem_type;
        const char* typeCstr = typeStr.c_str();
        Value typeValue(typeCstr, strlen(typeCstr), simJSON.GetAllocator());

        Value freqValue = Value(newSystem_freq);

        Value newSystem = Value(kObjectType);

        newSystem.AddMember("Nametag", nameValue, simJSON.GetAllocator());
        newSystem.AddMember("Source", sourceValue, simJSON.GetAllocator());
        newSystem.AddMember("Type", typeValue, simJSON.GetAllocator());
        newSystem.AddMember("Frequency", freqValue, simJSON.GetAllocator());

        Value instParameters = Value(kObjectType);

        newSystem.AddMember("Inst_Parameters", instParameters, simJSON.GetAllocator());

        systemsArray.PushBack(newSystem, simJSON.GetAllocator());
        //TODO the reallocation in here is resetting the paramSet-based IDs.
    }

}

static void AddEntitySection(Value& entitiesArray, Document& simJSON)
{
    static char newEntity_name[254];
    static char newEntity_type[254];

    //if (firstTimeThrough)
    //{
    //    newParam_typenum = 0;
    //    //...
    //}

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);

    bool addThisEntity = ImGui::Button("+ Add Entity"); ImGui::SameLine();
    ImGui::Text("Name"); ImGui::SameLine();
    ImGui::InputText("##EntityName", newEntity_name, MAX_STRING_LENGTH); ImGui::SameLine();

    //TODO enumerated types?
    /*ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);*/
    ImGui::Text("Type"); ImGui::SameLine();
    ImGui::InputText("##EntityType", newEntity_type, MAX_STRING_LENGTH);

    ImGui::PopItemWidth();

    if (addThisEntity)
    {
        //Checks
        assert(entitiesArray.IsArray());
        if (strcmp("", newEntity_name) == 0)
            return;
        for (auto& entity : entitiesArray.GetArray())
        {
            if (entity.HasMember("Name"))
            {
                if (strcmp(entity["Name"].GetString(), newEntity_name) == 0)
                    return;
            }
        }
        string nameStr = newEntity_name;
        const char* nameCstr = nameStr.c_str();
        Value nameValue(nameCstr, strlen(nameCstr), simJSON.GetAllocator());

        string typeStr = newEntity_type;
        const char* typeCstr = typeStr.c_str();
        Value typeValue(typeCstr, strlen(typeCstr), simJSON.GetAllocator());

        Value newEntity = Value(kObjectType);

        newEntity.AddMember("Name", nameValue, simJSON.GetAllocator());
        newEntity.AddMember("Type", typeValue, simJSON.GetAllocator());

        Value parametersSection = Value(kObjectType);

        //TODO remove later?
        newEntity.AddMember("Parameters", parametersSection, simJSON.GetAllocator());

        entitiesArray.PushBack(newEntity, simJSON.GetAllocator());
        //TODO the reallocation in here is resetting the paramSet-based IDs. 
    }

}

static void AddParamSection(Value& paramSet, Document& simJSON)
{
    static char newParam_name[254];
    static int newParam_typenum;

    static char newParam_string[MAX_STRING_LENGTH];
    static double newParam_double;
    static double newParam_double2[2];
    static double newParam_double3[3];
    static double newParam_double4[4];
    static bool newParam_bool;
    static int32_t newParam_int32;

    //if (firstTimeThrough)
    //{
    //    newParam_typenum = 0;
    //    //...
    //}
    ImGui::PushID(&paramSet);

    bool addThisParam = ImGui::Button("+ Add Param"); ImGui::SameLine();

    
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
    ImGui::Text("Key"); ImGui::SameLine();
    ImGui::InputText("##ParamKey", newParam_name, MAX_STRING_LENGTH); ImGui::SameLine();

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.08f);
    ImGui::Text("Type"); ImGui::SameLine();
    ImGui::Combo("##ParamType", &newParam_typenum, paramTypes, 7); ImGui::SameLine();


    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.30f);

    Value newVal;

    ImGui::Text("Value"); ImGui::SameLine();

    //static const char* paramTypes[] =
    //{ "Bool", "String", "Double", "DoubleV2", "DoubleV3", "DoubleV4", "Int32" };
    switch (newParam_typenum)
    {
    //case 0: //Array
    //{
    //    ImGui::Text("["); ImGui::SameLine();
    //    ImGui::InputText("##ParamValue", newParam_string, MAX_STRING_LENGTH); ImGui::SameLine();
    //    ImGui::Text("]");
    //
    //    //Added to this one for performance
    //    if (addThisParam)
    //    {
    //        Document doc;
    //        string toParse = "[" + string(newParam_string) + "]";
    //        if (doc.Parse(toParse.c_str()).HasParseError()) {
    //            std::cout << "ERROR: Invalid Json structure for array" << std::endl;
    //            //TODO better error
    //            addThisParam = false;
    //        }
    //        else if (!doc.IsArray())
    //        {
    //            std::cout << "ERROR: Invalid Json structure for array" << std::endl;
    //            addThisParam = false;
    //        }
    //        else if (doc.GetArray().Size() == 0)
    //        {
    //            //TODO do we need this case available? it might be problematic elsewhere in the software.
    //            std::cout << "ERROR: Invalid Json structure for param array" << std::endl;
    //            addThisParam = false;
    //        }
    //        else if (!doc.GetArray()[0].IsString())
    //        {
    //            //TODO do we need this case available? it's problematic elsewhere in the software.
    //            std::cout << "ERROR: Invalid Json structure for param array" << std::endl;
    //            addThisParam = false;
    //        }
    //        else
    //        {
    //            newVal = doc.GetArray();
    //        }
    //    }
    //}
    //    break;
    case 0: //Bool
        ImGui::Checkbox("##ParamValue", &newParam_bool);

        newVal.SetBool(newParam_bool);
        break;
    case 1: //String
        ImGui::InputText("##ParamValue", newParam_string, MAX_STRING_LENGTH);

        newVal.SetString(newParam_string, strlen(newParam_string));
        break;
    case 2: //Double
        ImGui::InputDouble("##ParamValue", &newParam_double,1,2, "%0.15f", ImGuiInputTextFlags_CharsScientific);

        newVal.SetDouble(newParam_double);
        break;
    case 3: //DoubleV2
    {
        ImGui::InputDouble2("##ParamValue", newParam_double2,"%0.15f", ImGuiInputTextFlags_CharsScientific);

        Value outvalJ(kArrayType);
        outvalJ.PushBack("doubleV2", simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double2[0], simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double2[1], simJSON.GetAllocator());

        newVal.SetArray();
    }
        break;
    case 4: //DoubleV3
    {
        ImGui::InputDouble3("##ParamValue", newParam_double3, "%0.15f", ImGuiInputTextFlags_CharsScientific);

        Value outvalJ(kArrayType);
        outvalJ.PushBack("doubleV3", simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double3[0], simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double3[1], simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double3[2], simJSON.GetAllocator());

        newVal = outvalJ;
    }
        break;
    case 5: //DoubleV4
    {
        ImGui::InputDouble4("##ParamValue", newParam_double4, "%0.15f", ImGuiInputTextFlags_CharsScientific);

        Value outvalJ(kArrayType);
        outvalJ.PushBack("doubleV4", simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double4[0], simJSON.GetAllocator());
        outvalJ.PushBack(newParam_double4[1], simJSON.GetAllocator());

        newVal = outvalJ;
    }
        break;
    case 6: //Int32
    {
        ImGui::InputInt("##ParamValue", &newParam_int32, ImGuiInputTextFlags_CharsScientific);

        Value outvalJ(kArrayType);
        outvalJ.PushBack("int32", simJSON.GetAllocator());
        outvalJ.PushBack(newParam_int32, simJSON.GetAllocator());

        newVal = outvalJ;
    }
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

    ImGui::PopID();
}

// returns true when the param was renamed/deleted
static bool paramContextMenu(ImGuiID currentID, const char* key, Value& paramSet, Document& simJSON)
{
    static char renamingStr[254] = "new name";
    bool didRenameOrDelete = false;

    if (ImGui::BeginPopupContextItem("ParamTreeContextMenu"))
    {
        if (ImGui::Button("Rename"))
        {
            ImGui::OpenPopup("RenameParam");
            /*strcpy(renamingStr, nameOfSet);*/
            //TODO strcpy_s, or std::string stuff?
            //TODO start out with the same name?
        }

        if (ImGui::BeginPopup("RenameParam"))
        {
            ImGui::Text("Rename");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4( 250/255, 190/255, 190/255 ,1),key);
            ImGui::SameLine();
            ImGui::Text(":");


            ImGui::InputText("##renameField", renamingStr, 254);

            if (ImGui::Button("Confirm"))
            {
                //TODO consolidate name checking
                if (strlen(renamingStr) != 0 &&
                    strcmp(renamingStr, key) != 0)
                {
                    bool improperNameFlag = false;
                    for (auto& param : paramSet.GetObject())
                    {
                        if (strcmp(renamingStr, param.name.GetString()) == 0)
                        {
                            improperNameFlag = true;
                        }
                    }
                    if (!improperNameFlag)
                    { // Yes, can rename with this name
                        string tempstr = renamingStr;
                        const char* tempCstr = tempstr.c_str();

                        Value newName(tempCstr, strlen(tempCstr), simJSON.GetAllocator());

                        Value newVal;
                        newVal.CopyFrom(paramSet[key], simJSON.GetAllocator());
                        //newVal.Swap(paramSet[key]);

                        paramSet.AddMember(newName, newVal, simJSON.GetAllocator());
                        //paramSet.RemoveMember(paramSet.FindMember(key));
                        paramSet.EraseMember(paramSet.FindMember(key));

                        perID_IsEditable.erase(currentID);
                        perID_IsSaved.erase(currentID);


                        ImGui::CloseCurrentPopup();
                        didRenameOrDelete = true;
                    }
                }
            }
            ImGui::EndPopup();
        }


        if (ImGui::Button("Delete"))
        {
            paramSet.EraseMember(paramSet.FindMember(key));

            perID_IsEditable.erase(currentID);
            perID_IsSaved.erase(currentID);
            ImGui::CloseCurrentPopup();
            didRenameOrDelete = true;
        }

        ImGui::EndPopup();
    }
    return didRenameOrDelete;
}

// Display JSON array of strings (System Nametags)
static void displaySystems(Value& paramSet)
{
    //StringBuffer sb;
    //PrettyWriter<StringBuffer> writer(sb);
    //val.Accept(writer);

    //ImGui::Text(sb.GetString());


    if (!paramSet.IsArray())
        return;

    //ImGui::NewLine();
    ImGui::Indent();

    for (auto& sysNameTag : paramSet.GetArray())
    {
        //TODO notify when nametag does not match any known Systems
        //TODO click on System to navigate to it in the config
        bool sysClicked = false;

        if (sysNameTag.IsString())
        {
            const char* nametagStr = sysNameTag.GetString();
            bool matchesKnownSystem = false;

            for (string nametag : cachedSysNametags)
            {
                if (nametag == string(nametagStr))
                {
                    matchesKnownSystem = true;
                }
            }

            if (matchesKnownSystem)
            {
                ImVec4 newCol = ImVecFromIntColor(colors[2]);
                //newCol.w = 90 / 255.0f;
                ImGui::PushStyleColor(ImGuiCol_Button, newCol);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, newCol);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255 / 255.0f, 50 / 255.0f, 50 / 255.0f, 70 / 255.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(255 / 255.0f, 50 / 255.0f, 50 / 255.0f, 70 / 255.0f));
            }

            sysClicked = ImGui::Button(sysNameTag.GetString());

            //if (!matchesKnownSystem)
            ImGui::PopStyleColor(2);
        }
        else
        {
            ImGui::Button("NON-STRING NAMETAG");
        }
        ImGui::SameLine();
        //TODO screen wrapping when too long
    }

    ImGui::Unindent();
    ImGui::NewLine();

}



void recurseParamTree(const char* nameOfSet, Value& paramSet, Document& simJSON, int depth)
{
    

    bool treeActive;
    //if (depth == 0)
    //{
    //    treeActive = ImGui::TreeNodeEx(nameOfSet, ImGuiTreeNodeFlags_CollapsingHeader);
    //    if (ImGui::BeginPopupContextItem("ParamTreeContextMenu"))
    //    {
    //        if (ImGui::Button("Rename"))
    //        {
    //            ImGui::BeginPopupContextWindow("RenameParamTree");
    //        }
    //
    //        ImGui::EndPopup();
    //    }
    //}
    //else
    //{
    //    /*treeActive = ImGui::TreeNode(nameOfSet);*/
    //    treeActive = ImGui::TreeNodeEx(nameOfSet, ImGuiTreeNodeFlags_CollapsingHeader);
    //    if (ImGui::BeginPopupContextItem("ParamContextMenu"))
    //    {
    //        ImGui::Text("Hello There");
    //        ImGui::EndPopup();
    //    }
    //}

    // root tree for this set
    treeActive = ImGui::TreeNodeEx(nameOfSet, ImGuiTreeNodeFlags_CollapsingHeader);
    

    ///////////////////////
    // FIRST TIME STUFF

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

    ///////////////////////
    // IF INSIDE TREE

    if (treeActive && depth < MAX_NESTING_DEPTH)
    {
        ImGuiInputTextFlags inputFieldFlags = NULL;
        ImGui::Indent();
        
        assert(paramSet.IsObject());

        //int numRowsForTreeSection = paramSet.GetObject().MemberCount();
        //ImGui::BeginChild("TreeBorderChild", ImVec2(0, numRowsForTreeSection*ImGui::GetTextLineHeightWithSpacing()), true);
        
        AddParamSection(paramSet, simJSON);
        ImGui::Spacing();

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

            // IF THIS VALUE IS *NOT* A SUBDIRECTORY OF PARAMETERS (JSON Object) //
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


                ///////////////////////
                // PRINTING OF KEY

                ImGui::TextColored(textcol, key);
                // If we don't rename/delete this param, it's ok to go on
                if (!paramContextMenu(currentID, key, paramSet, simJSON))
                {

                    ///////////////////////
                    // VALUE STUFF

                    // IF THIS VALUE IS AN ARRAY-STYLE PARAMETER (JSON Array) //
                    if (valTypeName == "Array")
                    {
                        //ImGui::SameLine();
                        bool renderingIsDone = false;
                        bool fallbackRendering = false;
                        bool isEmpty = false;
                        string arrayValType = "not initialized";


                        if (!val.IsArray()) { fallbackRendering = true; }
                        else if (val.Size() == 0) { isEmpty = true; fallbackRendering = true; }
                        else if (strcmp(key, "Systems") == 0)
                        {
                            displaySystems(param.value);
                            renderingIsDone = true;
                        }
                        else
                        {
                            if (!val.GetArray()[0].IsString())
                            { // Not a valid array-defined param
                                fallbackRendering = true;
                                arrayValType = "not initialized";
                            }
                            else
                            {
                                //TODO bandaid fix
                                try
                                {
                                    if (!val.GetArray()[0].IsNull())
                                        arrayValType = val.GetArray()[0].GetString();
                                }
                                //catch (const std::exception& e)
                                catch (...)
                                {
                                    arrayValType = "not initialized";
                                    fallbackRendering = true;
                                }
                            }
                        }

                        if (!fallbackRendering && arrayValType != "not initialized")
                        {
                            if (arrayValType == "int32")
                            {
                                //TODO check length of array
                                ImGui::SameLine();
                                int32_t outval = val.GetArray()[1].GetInt();

                                pushedStyleColor = ColorIfUnsaved(currentID);
                                ImGui::InputInt("(Int32)",&outval, ImGuiInputTextFlags_CharsScientific);

                                if (val.GetArray()[1].GetInt() != outval)
                                {
                                    *perID_IsSaved[currentID] = false;
                                }

                                Value outvalJ(kArrayType);
                                outvalJ.PushBack("int32", simJSON.GetAllocator());
                                outvalJ.PushBack(outval, simJSON.GetAllocator());

                                paramSet[key] = outvalJ;

                            }
                            else if (arrayValType == "doubleV2")
                            {
                                //TODO check length of array
                                ImGui::SameLine();
                                double outval[2] = { val.GetArray()[1].GetDouble(),
                                    val.GetArray()[2].GetDouble() };

                                pushedStyleColor = ColorIfUnsaved(currentID);
                                ImGui::InputDouble3("(DoubleV2)", outval, "%0.15f", ImGuiInputTextFlags_CharsScientific);

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
                                ImGui::InputDouble3("(DoubleV3)", outval, "%0.15f", ImGuiInputTextFlags_CharsScientific);

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
                                ImGui::InputDouble4("(DoubleV4)", outval, "%0.15f", ImGuiInputTextFlags_CharsScientific);

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


                        if (isEmpty)
                        {
                            ImGui::Text("EMPTY");
                        }
                        else if (fallbackRendering)
                        {
                            //TODO fix!
                            /*StringBuffer sb;
                            PrettyWriter<StringBuffer> writer(sb);
                            val.Accept(writer);

                            ImGui::Text(sb.GetString());*/
                            ImGui::Text("NON-PARAMETER ARRAY");
                        }
                    }
                    // IF THIS VALUE IS A STRING PARAMETER (JSON String) //
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
                    // IF THIS VALUE IS A DOUBLE PARAMETER (JSON Number) //
                    else if (valTypeName == "Number")
                    {
                        ImGui::SameLine();
                        double outval = val.GetDouble();

                        pushedStyleColor = ColorIfUnsaved(currentID);
                        ImGui::InputDouble("(Double)", &outval, 1.0, 100.0, "%0.15f", inputFieldFlags | ImGuiInputTextFlags_CharsScientific);

                        if (val.GetDouble() != outval)
                        {
                            *perID_IsSaved[currentID] = false;
                        }

                        Value outvalJ(outval);
                        paramSet[key] = outvalJ;
                    }
                    // IF THIS VALUE IS A BOOL PARAMETER (JSON True/False) //
                    else if (valTypeName == "True" || valTypeName == "False")
                    {
                        ImGui::SameLine();
                        bool outval = val.GetBool();

                        pushedStyleColor = ColorIfUnsaved(currentID);
                        ImGui::Checkbox("", &outval);

                        if (val.GetBool() != outval)
                        {
                            *perID_IsSaved[currentID] = false;
                        }

                        Value outvalJ(outval);
                        paramSet[key] = outvalJ;

                    }
                    // IF THIS VALUE IS A JSON Null? //
                    else // Unimplemented param type (probably impossible due to this being JSON types?)
                    {
                        ImGui::SameLine();

                        //TODO better handling of this than just printing JSON

                        StringBuffer sb;
                        PrettyWriter<StringBuffer> writer(sb);
                        val.Accept(writer);

                        ImGui::Text(sb.GetString());
                    }
                } // end if context menu renamed/deleted
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
        } // End loop through params in paramSet

        //ImGui::EndChild();

        // End of tree area stuff
        if (depth != 0)
        {
            //ImGui::TreePop();
            /*ImGui::Unindent();*/
        }
        ImGui::Unindent();
    }
    
}
