#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "imgui.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using std::string;
using namespace rapidjson;

#define MAXIMUM_NESTING_DEPTH 500
#define MAXIMUM_STRING_LENGTH 10000

static const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };

static int colors[6][3] = { { 255, 225, 25 }, {0, 130, 200},
{245, 130, 48}, {250, 190, 190}, {230, 190, 255}, {128, 128, 128} };


void recurseParamTree(const char * nameOfSet, Value& paramSet, int depth)
{
    bool treeActive;
    if (depth == 0)
        treeActive = ImGui::TreeNodeEx(nameOfSet, ImGuiTreeNodeFlags_CollapsingHeader);
    else
        treeActive = ImGui::TreeNode(nameOfSet);

    if (treeActive && depth < MAXIMUM_NESTING_DEPTH)
    {

        assert(paramSet.IsObject());

        for (auto& param : paramSet.GetObject())
        {
            ImGui::PushID(&param);

            int c = depth % 6;

            const char* key = param.name.GetString();
            Value& val = param.value;
            const char* valTypeName = kTypeNames[val.GetType()];

            if (valTypeName != "Object")
            {
                ImVec4 textcol = { colors[c][0] / 255.0f, colors[c][1] / 255.0f, colors[c][2] / 255.0f, 1.0f };
                ImGui::TextColored(textcol, key);

                if (valTypeName == "Array")
                {
                    ImGui::SameLine();

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

                    ImGui::InputText("(String)", outval, MAXIMUM_STRING_LENGTH);
                    Value outvalJ;
                    outvalJ.SetString(outval, length);
                    paramSet[key] = outvalJ;
                    //delete outval;//TODO ?
                }
                if (valTypeName == "Number")
                {
                    ImGui::SameLine();
                    double outval = val.GetDouble();
                    ImGui::InputDouble("(Double)", &outval, 1.0, 100.0);
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
            }
            else
            {
                recurseParamTree(key, val, depth + 1);
            }
            ImGui::PopID();
        }
        if(depth != 0)
            ImGui::TreePop();
    }
}
