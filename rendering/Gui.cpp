#include "Gui.h"
#include "util.h"
#include "positionManager.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"

ImGuiWindowFlags window_flags = 0;
ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

namespace Gui {
    bool isEnabled = false;

    bool validUsername = true;
    bool validIP = true;

    void setDisabledStyle(bool flag) {
        if (flag) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
    }

    void releaseDisabledStyle(bool flag) {
        if (flag) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
    }

    void drawGui() {
        ImGui::Begin("OnlyUp Multiplayer", 0, window_flags);
        if (ImGui::BeginTabBar("Action Selection", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Join")) {
                ImGui::Text("This tab is for joining the multiplayer lobby");
                ImGui::Separator();

                ImGui::Text("Please enter your username"); ImGui::SameLine(); 
                ImGui::PushItemWidth(-1); setDisabledStyle(PositionManager::connected);
                ImGui::InputText("##Username", PositionManager::username, IM_ARRAYSIZE(PositionManager::username));
                ImGui::PopItemWidth(); releaseDisabledStyle(PositionManager::connected);

                ImGui::Text("Please enter the server ip address"); ImGui::SameLine(); 
                ImGui::PushItemWidth(-1); setDisabledStyle(PositionManager::connected);
                ImGui::InputText("##IP", PositionManager::ipAddress, IM_ARRAYSIZE(PositionManager::ipAddress));
                ImGui::PopItemWidth(); releaseDisabledStyle(PositionManager::connected);

                setDisabledStyle(PositionManager::connected);
                if (ImGui::Button("Join")) {
                    if (strlen(PositionManager::username) != 0) {
                        if (PositionManager::setupNetwork() == 0) {
                            validIP = true;
                        } else if (PositionManager::setupNetwork() == 3) {
                            validIP = false;
                        }
                        validUsername = true;
                    }
                    else {
                        validUsername = false;
                    }
                }
                releaseDisabledStyle(PositionManager::connected);

                if (!validUsername) { ImGui::SameLine(); ImGui::Text("Please enter a valid username"); }
                if (!validIP) { ImGui::SameLine(); ImGui::Text("Please enter a valid address"); }
                if (PositionManager::lobbyFull) { ImGui::SameLine(); ImGui::Text("Lobby is full"); }

                if (ImGui::Button("Disconnect")) {
                    PositionManager::disconnect();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug")) {
                ImGui::Text("X: %f | Y:%f | Z: %f", PositionManager::localPlayerPos.x, PositionManager::localPlayerPos.y, PositionManager::localPlayerPos.z);
                ImGui::Separator();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    void drawUi() {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    }

    void setStyle() {
        window_flags |= ImGuiWindowFlags_NoCollapse;;

        ImGuiStyle* style = &ImGui::GetStyle();

        style->WindowPadding = ImVec2(15, 15);
        style->WindowRounding = 5.0f;
        style->FramePadding = ImVec2(5, 5);
        style->FrameRounding = 4.0f;
        style->ItemSpacing = ImVec2(12, 8);
        style->ItemInnerSpacing = ImVec2(8, 6);
        style->IndentSpacing = 25.0f;
        style->ScrollbarSize = 15.0f;
        style->ScrollbarRounding = 9.0f;
        style->GrabMinSize = 5.0f;
        style->GrabRounding = 3.0f;

        style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.96f);
        style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.80f);
        style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
        style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);      
        style->Colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f); 
        style->Colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    }
}