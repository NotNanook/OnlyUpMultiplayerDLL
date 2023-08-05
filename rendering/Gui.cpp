#include "Gui.h"
#include "util.h"
#include "positionManager.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"

ImGuiWindowFlags window_flags = 0;
ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

int fov = 78;

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

    float calculateRadius(float distance, float minRadius, float maxRadius) {
        if (distance <= 100) {
            return maxRadius;
        }
        else if (distance >= 20000) {
            return minRadius;
        }
        else {
            float scaleFactor = (distance - 100) / (20000 - 100);
            float scaledRadius = (1 - scaleFactor) * maxRadius + scaleFactor * minRadius;
            return scaledRadius;
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
                        }
                        else if (PositionManager::setupNetwork() == 3) {
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
                ImGui::SliderInt("Change Proj. M. FOV", &fov, 70, 180);
                ImGui::Separator();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    void drawUi() {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (PositionManager::connected) {
            uintptr_t pitchPointer = util::FindDMAAddy(((uintptr_t)PositionManager::baseModule + 0x07872B00), { 0x30, 0x308 });
            if (pitchPointer == NULL) { return; }
            double pitch = *(double*)pitchPointer;
            double yaw = *(double*)(pitchPointer + 0x8);

            uintptr_t camPosPointer = util::FindDMAAddy(((uintptr_t)PositionManager::baseModule + 0x07442848), { 0x8, 0x10, 0xB0, 0x6B8, 0x68, 0x18, 0x80 });
            DirectX::XMVECTOR cameraPosition = DirectX::XMVectorSet(*(float*)(camPosPointer), *(float*)(camPosPointer + 0x40), *(float*)(camPosPointer + 0x20), 1.0f);
            
            if (pitch > 260) pitch -= 360;
            if (yaw < 360 && yaw > 180) yaw -= 360;

            float yawRad = DirectX::XMConvertToRadians(yaw);
            float pitchRad = DirectX::XMConvertToRadians(pitch);

            float cosYaw = cosf(yawRad);
            float sinYaw = sinf(yawRad);
            float cosPitch = cosf(pitchRad);
            float sinPitch = sinf(pitchRad);

            DirectX::XMVECTOR forward = DirectX::XMVectorSet(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch, 0.0f);

            // Calculate the up vector based on the pitch angle
            DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, cosPitch, 0.0f, 0.0f);

            // Calculate the target position by adding the forward vector to the camera position
            DirectX::XMVECTOR target = DirectX::XMVectorAdd(cameraPosition, forward);

            // Calculate the view matrix
            DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, target, up);

            DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y, 1.0f, 100.0f);

            PositionManager::m.lock();
            DirectX::XMVECTOR otherPlayerPos = DirectX::XMVectorSet(PositionManager::mirrorPlayer.lastPosition.x, PositionManager::mirrorPlayer.lastPosition.y, PositionManager::mirrorPlayer.lastPosition.z, 1.0f);
            DirectX::XMVECTOR otherPlayerPos2 = DirectX::XMVectorSet(PositionManager::mirrorPlayer.lastPosition.x, PositionManager::mirrorPlayer.lastPosition.y+60.0f, PositionManager::mirrorPlayer.lastPosition.z, 1.0f);
            //DirectX::XMVECTOR otherPlayerPos = DirectX::XMVectorSet(4000.0f, -2700.0f, 18000.0f, 1.0f);
            //DirectX::XMVECTOR otherPlayerPos2 = DirectX::XMVectorSet(4000.0f, -2640.0f, 18000.0f, 1.0f); // Name is 60 units above circle
            PositionManager::m.unlock();

            float distance = std::sqrtf((std::pow(DirectX::XMVectorGetX(otherPlayerPos) - PositionManager::localPlayerPos.x, 2) +
                std::pow(DirectX::XMVectorGetY(otherPlayerPos) - PositionManager::localPlayerPos.y, 2) +
                std::pow(DirectX::XMVectorGetZ(otherPlayerPos) - PositionManager::localPlayerPos.z, 2)));

            float scaledRadius = calculateRadius(distance, 2, 8);

            DirectX::XMFLOAT2 screenPos = util::WorldToScreen(otherPlayerPos, viewMatrix, projectionMatrix, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
            DirectX::XMFLOAT2 screenPosText = util::WorldToScreen(otherPlayerPos2, viewMatrix, projectionMatrix, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

            if (screenPos.x == -1 && screenPos.y == -1) return;

            drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), scaledRadius, ImColor(255, 255, 255, 255));
            ImVec2 textSize = ImGui::CalcTextSize(PositionManager::mirrorPlayer.playerName);
            drawList->AddText(ImVec2(screenPosText.x - textSize.x/2, screenPosText.y - textSize.x/2), ImColor(255, 255, 255, 255), PositionManager::mirrorPlayer.playerName);

        }
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