#include "ui.hh"
#include "../globals.hh"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include <windows.h>
#include <string>
#include <iostream>
#include "../PH.hpp"
#include <thread>
#include "../imgui/spinner.h"
using namespace ph;

static int selectedItem = 0;
login_request_t login_resp = {};
inject_request_t inject_resp = {};

bool open_popup = false;
bool nonClosablePopup = false;
bool open_popup_next_frame = false;
bool exit_after_popup = false;
std::string popup_text = "";
std::string popup_title = "";

void ui::render() {
    if (!globals.active) return;

    ImGui::SetNextWindowPos(ImVec2(window_pos.x, window_pos.y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::Begin(window_title, &globals.active, window_flags);
    {
		if (ImGui::BeginPopupModal((popup_title + ("##popup")).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::Text(popup_text.c_str());

			if (!nonClosablePopup && ImGui::Button("Ok", ImVec2(-1.0f, 0.0f))) {
				ImGui::CloseCurrentPopup();

				if (exit_after_popup)
					exit(0);
			}

			ImGui::EndPopup();
		}
        switch (globals.menuNumber)
        {
			case 0:
				if (!globals.loginSuccess) {
					if (globals.runningLogin) {
						const auto r = 8.0f;
						ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionMax().x * 0.5f - r, ImGui::GetWindowContentRegionMax().y * 0.5f - r));
						ImGui::Spinner(("##Update Spinner"), r, 3.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0f, 1.0f, (sin(ImGui::GetTime() * IM_PI * 2.0f) * 0.5f + 0.5f) * 0.5f + 0.5f));
						if (globals.loginSuccess) {
							const auto text_size = ImGui::CalcTextSize(("checking for updates"));
							ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionMax().x * 0.5f - text_size.x * 0.5f, ImGui::GetWindowContentRegionMax().y * 0.5f + r + 8.0f));
							ImGui::Text(("checking for updates"));
							ImGui::PopStyleColor();
						}
						else {
							const auto text_size = ImGui::CalcTextSize(("authenticating"));
							ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionMax().x * 0.5f - text_size.x * 0.5f, ImGui::GetWindowContentRegionMax().y * 0.5f + r + 8.0f));
							ImGui::Text(("authenticating"));
							ImGui::PopStyleColor();
						}
					}
					else {
						ImGui::Text("welcome to nor.cat");
						if (ImGui::Button("login") && !globals.runningLogin) {
							std::thread([]() {
								globals.runningLogin = true;

								dbg_print("Attempting Login...\n");

								login(login_resp, modules, true);

								dbg_print("[Response] %s\n", login_resp.message);

								std::chrono::seconds timespan(2);
								std::this_thread::sleep_for(timespan); //Sleep for error visibility
								if (!login_resp.success) { // Failed to Login
									globals.runningLogin = false;
									std::thread([]() { //Error Popup
										popup_text = login_resp.message;
										popup_title = ("error");
										open_popup = true;
										exit_after_popup = true;
										nonClosablePopup = false;
										}).detach();
								}
								else
								{

										globals.loginSuccess = true;
										globals.runningLogin = false;
										moduleItems.push_back("None"); //Adding modules to Combo List
										for (auto& cur_mod : modules) {
											moduleItems.push_back(cur_mod.name);
										}
										dbg_print("Iterating through modules...");

										globals.menuNumber = 1;
								}
							}).detach();
						}

					}
				}



				break;
            case 1:
				if (globals.runningInjection) {
					const auto r = 8.0f;
					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionMax().x * 0.5f - r, ImGui::GetWindowContentRegionMax().y * 0.5f - r));
					ImGui::Spinner(("##Update Spinner"), r, 3.0f, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)));
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0f, 1.0f, (sin(ImGui::GetTime() * IM_PI * 2.0f) * 0.5f + 0.5f) * 0.5f + 0.5f));
					const auto text_size = ImGui::CalcTextSize(("injecting module"));
					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionMax().x * 0.5f - text_size.x * 0.5f, ImGui::GetWindowContentRegionMax().y * 0.5f + r + 8.0f));
					ImGui::Text(("injecting module"));
					ImGui::PopStyleColor();
				}

				else if (globals.loginSuccess) {

					ImGui::Text("welcome to nor.cat, %s", std::string(get_username()).data());
					ImGui::NewLine();

					if (!selectedItem) {
						ImGui::NewLine();
						ImGui::NewLine();
						ImGui::NewLine();
					}

					else {
						ImGui::Text("status:\t%s", modules[selectedItem - 1].status);
						ImGui::Text("target:\t%s"  , modules[selectedItem - 1 ].target );
						ImGui::Text("version:\t%s", modules[selectedItem - 1].version);
					}
					ImGui::NewLine();
					ImGui::NewLine();
					ImGui::Separator();


					ImGui::Combo("module", &selectedItem, moduleItems.data(), moduleItems.size());


					if (ImGui::Button("inject", ImVec2(-1.0f, 0.0f)) && !globals.runningInjection && !globals.injectSuccess) {
						if (!selectedItem) {
							std::thread([]() { //Invalid Module Popup
								popup_text = "error";
								popup_title = ("please select a valid module");
								open_popup = true;
								nonClosablePopup = false;
								exit_after_popup = false;
							}).detach();
						}
						else {
							globals.runningInjection = true;
							std::thread([]() {
								if (inject(inject_resp, modules[selectedItem - 1].id)) {


									if (inject_resp.success) {

										globals.runningInjection = false;
										globals.injectSuccess = true;


										dbg_print("Successfully Injected Binary!\nResponse: %s\n", inject_resp.message);
										std::thread([]() { //Success popup
											popup_text = inject_resp.message;
											popup_title = ("successfully injected");
											open_popup = true;
											exit_after_popup = true;
											nonClosablePopup = false;
										}).detach();
									}
									else {
										dbg_print("Error Injecting Binary!\nResponse: %s\n", inject_resp.message);
										std::thread([]() { //Error popup
											popup_text = inject_resp.message;
											popup_title = ("injection failed");
											open_popup = true;
											exit_after_popup = true;
											nonClosablePopup = false;
										}).detach();
									}
								}
							}).detach();
						}
					}
				}
				else {
					std::thread([]() { //Error popup
						popup_text = "error";
						popup_title = ("invalid user");
						nonClosablePopup = false;
						exit_after_popup = false;
					}).detach();
					globals.menuNumber = 0;
				}
				break;
        }

    }
	if (open_popup_next_frame) {
		ImGui::OpenPopup((popup_title + ("##popup")).c_str());
		open_popup_next_frame = false;
	}

	if (open_popup) {
		open_popup_next_frame = true;
		open_popup = false;
	}
    ImGui::End();
}

void autoClose() {
	std::chrono::seconds timespan(5);
	std::this_thread::sleep_for(timespan);
	exit(2);
}

void ui::init(LPDIRECT3DDEVICE9 device) {
    dev = device;
	
    // colors
    //ImGui::StyleColorsDark();
    auto style = &ImGui::GetStyle();
	style->WindowRounding = 0.0f;
	style->FrameRounding = 4.0f;
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	//style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_Text] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
	//style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	//style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.00f);
	//style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_Border]   = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	//style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	//style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	//style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	//style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	//style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
	//style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

	style->Colors[ImGuiCol_Separator] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);

	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);

	if (window_pos.x == 0) {
		RECT screen_rect{};
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		screen_res = ImVec2(float(screen_rect.right), float(screen_rect.bottom));
		window_pos = (screen_res - window_size) * 0.5f;

		// init images here
	}
}