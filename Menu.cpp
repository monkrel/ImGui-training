#define WIN32_LEAN_AND_MEAN  
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <comdef.h>
#include <d3d11.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include <stdio.h>
#include "ImGui/imgui_internal.h"
#include <filesystem>
#include "variables.h"

namespace fs = std::filesystem;

HWND window = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swapchain = nullptr;
ID3D11RenderTargetView* view = nullptr;

inline bool CreateView() {
	ID3D11Texture2D* buffer = nullptr;
	if (FAILED(swapchain->GetBuffer(0, __uuidof(buffer), reinterpret_cast<PVOID*>(&buffer)))) return false;
	if (FAILED(device->CreateRenderTargetView(buffer, nullptr, &view))) return false;
	buffer->Release();
	return true;
}

void ShowErrorMsg(const char* lpszFunction, HRESULT hr)
{
	char* buf = new char[0x200];
	_com_error err(hr);
	sprintf(buf, "%s failed with error 0x%lX: %s", lpszFunction, hr, err.ErrorMessage());
	MessageBoxA(nullptr, buf, "Error", 0);
	delete[] buf;
}

inline bool InitDX()
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	HRESULT hr = 0;

	if (FAILED(hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &context))) 
	{
		ShowErrorMsg("D3D11CreateDeviceAndSwapChain", hr);
		return false;
	}

	if (!CreateView())
	{
		if (swapchain)
		{
			(swapchain)->Release();
			swapchain = nullptr;
		}
		if (device)
		{
			(device)->Release();
			swapchain = nullptr;
		}
		return false;
	}
	return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_SIZE:

		if (device != nullptr && wParam != SIZE_MINIMIZED)
		{
			if (view)
			{
				view->Release();
				view = nullptr;
			}

			swapchain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
			CreateView();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

struct Vec2 : public ImVec2 {
	using ImVec2::ImVec2;
	FORCEINLINE float Size() const { return sqrtf(x * x + y * y); }
	FORCEINLINE Vec2 operator*(float Scale) const { return Vec2(x * Scale, y * Scale); }
	FORCEINLINE Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
	FORCEINLINE Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
};

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "ImGui Example", NULL };
	RegisterClassExA(&wc);
	window = CreateWindowExA(0L, wc.lpszClassName, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, wc.hInstance, NULL);

	if (!window)
	{
		UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	if (!InitDX())
	{
		return 1;
	}

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	////YouGame Start Style
	ImFontConfig config;
	config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
	config.RasterizerMultiply = 1.125f;
	io.Fonts->AddFontFromFileTTF("C:\\Users\\m0nkrel\\AppData\\Local\\Microsoft\\Windows\\Fonts\\HurmeGeometricSans3-Regular.ttf", 16.0f, &config);
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style->WindowBorderSize = 0.0f;
	style->FramePadding = ImVec2(39.f, 6.f);
	style->WindowPadding = ImVec2(10.f, 6.f);
	style->GrabMinSize = 24.f;
	style->WindowBorderSize = 0.f;
	style->WindowMenuButtonPosition = ImGuiDir_None;
	style->ColorButtonPosition = ImGuiDir_Left;
	style->FrameRounding = 3.f;
	style->GrabRounding = 4.f;
	style->ItemSpacing = ImVec2(6.f, 6.0f);


	style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.0f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.153f, 0.157f, 0.227f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.176f, 0.176f, 0.247f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.310f, 0.310f, 0.310f, 0.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.310f, 0.310f, 0.310f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.410f, 0.410f, 0.410f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.510f, 0.510f, 0.510f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.039f, 0.039f, 0.078f, 0.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.039f, 0.039f, 0.078f, 0.00f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.039f, 0.039f, 0.078f, 0.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 0.00f);
	style->Colors[ImGuiCol_Tab] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.054f, 0.054f, 0.104f, 1.00f);
	style->Colors[ImGuiCol_TabActive] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	////YouGame END Style


	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, context);

	MSG msg{};
	float time = 0.f;
	Vec2 TL;
	while (msg.message != WM_QUIT) {

		if (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			continue;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		float xcenter = (io.DisplaySize.x) / 2;
		float ycenter = (io.DisplaySize.y) / 2;

		static float color = 0.0f;
		static bool isOpen = false;
		if (GetAsyncKeyState(VK_INSERT) & 0x1) isOpen = !isOpen;
		
		////YouGame Start MENU
		if (isOpen)
		{
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;
			ImGui::SetNextWindowSize(ImVec2((io.DisplaySize.x) / 1.7f, (io.DisplaySize.y) / 1.2f));
			ImGui::Begin("Unicore", NULL, flags);
			if (ImGui::BeginTabBar("###1", ImGuiTabBarFlags_NoTooltip))
			{
				if (ImGui::BeginTabItem("	Visuals"))
				{
					if (ImGui::BeginTabBar("###2", ImGuiTabBarFlags_NoTooltip))
					{
	
						if (ImGui::BeginTabItem("					Global"))
						{

							//make 2 buttons
							ImGui::Button("Chests", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
							ImGui::SameLine();
							ImGui::Button("Misc", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));	
							
							ImGui::BeginChild("1", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.4f));
								ImGui::Checkbox("Chest (Common)"	, &chest_com);
								ImGui::Checkbox("Chest (Exquisite)" , &chest_exq);
								ImGui::Checkbox("Chest (Luxorius)"	, &chest_lux);
								ImGui::Checkbox("Wind Slime"		, &w_slime);
								ImGui::Checkbox("Ice Bulk"			, &ice_bulk);
								ImGui::Checkbox("Search Point"		, &s_point);
								ImGui::Checkbox("Bloatty Floatty"	, &bloat_float);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("1.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.4f));
								ImGui::Checkbox("Offscreen##1"		, &offscreen1);
								ImGui::Checkbox("Offscreen##2"		, &offscreen2);
								ImGui::Checkbox("Offscreen##3"		, &offscreen3);
								ImGui::Checkbox("Offscreen##4"		, &offscreen4);
								ImGui::Checkbox("Offscreen##5"		, &offscreen5);
								ImGui::Checkbox("Offscreen##6"		, &offscreen6);
								ImGui::Checkbox("Offscreen##7"		, &offscreen7);
							ImGui::EndChild();
							
							ImGui::SameLine();
							ImGui::BeginChild("2", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.4f));
								ImGui::Checkbox("Seelie"			, &seelie);
								ImGui::Checkbox("Challenge"			, &challenge);
								ImGui::Checkbox("Oculus"			, &oculi);
								ImGui::Checkbox("Agate"				, &agate);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("2.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.4f));
								ImGui::Checkbox("Offscreen##8"		, &offscreen8);
								ImGui::Checkbox("Offscreen##9"		, &offscreen9);
								ImGui::Checkbox("Offscreen##10"		, &offscreen10);
								ImGui::Checkbox("Offscreen##11"		, &offscreen11);
							ImGui::EndChild();

							ImGui::Button("Ores", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
							ImGui::SameLine();
							ImGui::Button("Locations", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
							
							ImGui::BeginChild("3", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.32f));
								ImGui::Checkbox("Ore (Metal)"		, &ore_met);
								ImGui::Checkbox("Ore (Crystal)"		, &ore_cryst);
								ImGui::Checkbox("Ore (Stone)"		, &ore_stone);
								ImGui::Checkbox("Ore (Electric)"	, &ore_electr);
								ImGui::Checkbox("Ore (Starsilver)"	, &ore_starsilver);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("3.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.32f));
								ImGui::Checkbox("Offscreen##12"		, &offscreen12);
								ImGui::Checkbox("Offscreen##13"		, &offscreen13);
								ImGui::Checkbox("Offscreen##14"		, &offscreen14);
								ImGui::Checkbox("Offscreen##15"		, &offscreen15);
								ImGui::Checkbox("Offscreen##16"		, &offscreen16);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("4", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.32f));
								ImGui::Checkbox("Archon Towers"		, &towers);
								ImGui::Checkbox("Teleports"			, &teleport);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("4.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 6)) * 0.32f));
								ImGui::Checkbox("Offscreen##17"		, &offscreen17);
								ImGui::Checkbox("Offscreen##18"		, &offscreen18);
							ImGui::EndChild();
							
							ImGui::EndTabItem();
						}



						
						if (ImGui::BeginTabItem("					 Local"))
						{
							ImGui::Button("Mondstadt", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
							ImGui::SameLine();
							ImGui::Button("Liyue", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));

							ImGui::BeginChild("5", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.4f));
								ImGui::Checkbox("Calla Lily"	  , &calla_lily);
								ImGui::Checkbox("Cecilia"         , &cecilia);
								ImGui::Checkbox("Dandelion"       , &dandelion);
								ImGui::Checkbox("Philanemo"       , &philanemo);
								ImGui::Checkbox("Small Lamp Grass", &lamp_grass);
								ImGui::Checkbox("Valberry"        , &vallberry);
								ImGui::Checkbox("Windwhell Aster" , &wind_aster);
								ImGui::Checkbox("Woolfhook"       , &wolfhook);
							ImGui::EndChild();
							
							ImGui::SameLine();
							ImGui::BeginChild("5.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.4f));
								ImGui::Checkbox("Offscreen##19"	  , &offscreen19);
								ImGui::Checkbox("Offscreen##20"   , &offscreen20);
								ImGui::Checkbox("Offscreen##21"   , &offscreen21);
								ImGui::Checkbox("Offscreen##22"   , &offscreen22);
								ImGui::Checkbox("Offscreen##23"   , &offscreen23);
								ImGui::Checkbox("Offscreen##24"   , &offscreen24);
								ImGui::Checkbox("Offscreen##25"   , &offscreen25);
								ImGui::Checkbox("Offscreen##26"	  , &offscreen26);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("6", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.4f));
								ImGui::Checkbox("Glaze Lily"	  , &glaze_lily);
								ImGui::Checkbox("Juyeun Chili"	  , &chili);
								ImGui::Checkbox("Qingxin"		  , &qingxin);
								ImGui::Checkbox("Silk flower"	  , &silk_flow);
								ImGui::Checkbox("Violetgrass"	  , &violetgrass);
								ImGui::Checkbox("Ore (Cor Lapis)" , &ore_lapis);
								ImGui::Checkbox("Ore (Noc. Jade)" , &ore_nocjade);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("6.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.4f));
								ImGui::Checkbox("Offscreen##27"   , &offscreen27);
								ImGui::Checkbox("Offscreen##28"   , &offscreen28);
								ImGui::Checkbox("Offscreen##29"   , &offscreen29);
								ImGui::Checkbox("Offscreen##30"   , &offscreen30);
								ImGui::Checkbox("Offscreen##31"   , &offscreen31);
								ImGui::Checkbox("Offscreen##32"   , &offscreen32);
								ImGui::Checkbox("Offscreen##33"   , &offscreen33);
							ImGui::EndChild();

							ImGui::Button("Inazuma", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
							ImGui::BeginChild("7", ImVec2(ImGui::GetWindowContentRegionWidth() / 3.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.32f));
								ImGui::Checkbox("Sea Ganodema", &sea_ganodema);
								ImGui::Checkbox("Naku Weed", &naku_weed);
								ImGui::Checkbox("Sakura Bloom", &sakura_bloom);
								ImGui::Checkbox("Onikabuto", &onikabuto);
								ImGui::Checkbox("Dendrobium", &dendrobium);
								ImGui::Checkbox("Crystal Marrow", &crystal_marrow);
								ImGui::Checkbox("Amethyst Lump", &amethyst_lump);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("7.5", ImVec2(ImGui::GetWindowContentRegionWidth() / 6.25f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.32f));
								ImGui::Checkbox("Offscreen##34", &offscreen34);
								ImGui::Checkbox("Offscreen##35", &offscreen35);
								ImGui::Checkbox("Offscreen##36", &offscreen36);
								ImGui::Checkbox("Offscreen##37", &offscreen37);
								ImGui::Checkbox("Offscreen##38", &offscreen38);
								ImGui::Checkbox("Offscreen##39", &offscreen39);
								ImGui::Checkbox("Offscreen##40", &offscreen40);
							ImGui::EndChild();

							ImGui::EndTabItem();
						}
							
						
						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
			
				if (ImGui::BeginTabItem("	Player"))
				{
					char noclip_b[128];
					char attack_b[128];
					

					if (noclip_spd > 5.f)
						sprintf(noclip_b, "Noclip (Unsafe)");
					else
						sprintf(noclip_b, "Noclip (Safe)");
					ImGui::Button(noclip_b, ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
					ImGui::SameLine();
					if (attack_spd > 5.f)
						sprintf(attack_b, "Attack (Unsafe)");
					else
						sprintf(attack_b, "Attack (Safe)");
					ImGui::Button(attack_b, ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
					
					ImGui::BeginChild("1", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) / 2.7f));
						ImGui::Checkbox("Enabled##1", &noclip);
						ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth()*0.7f);
						ImGui::SliderFloat("Speed", &noclip_spd,1.00f ,10.00f, "%.3f");
						//I have no idea how to make hotkey input so it will be input text.
						ImGui::InputText("Key##1", hotkey_speed, 128);
					ImGui::EndChild();

					ImGui::SameLine();
					ImGui::BeginChild("2", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) / 2.7f));
						ImGui::Checkbox("Enabled##2", &atkspd);
						ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.7f);
						ImGui::SliderFloat("Attack", &attack_spd, 1.00f, 10.00f, "%.3f");
					ImGui::EndChild();

					ImGui::Button("Skills", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
					ImGui::SameLine();
					ImGui::Button("Rapid Fire", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));

					ImGui::BeginChild("3", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) / 2.7f));
						ImGui::Checkbox("Infinite Ultimate", &infult);
						ImGui::Checkbox("No E/Q Cooldown", &cdreduce);
						ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.7f);
						ImGui::SliderFloat("Power##1", &cdreduse_pwr, 0.00f, 1.00f, "%.3f");
					ImGui::EndChild();

					ImGui::SameLine();
					ImGui::BeginChild("4", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) / 2.7f));
						ImGui::Checkbox("Rapid Fire", &rapfire);
						ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.7f);
						ImGui::SliderFloat("Power##2", &rapfire_pwr, 1.00f, 50.00f, "%.3f");
						//I have no idea how to make hotkey input so it will be input text.
						ImGui::InputText("Key##2", hotkey_rapfire, 128);
					ImGui::EndChild();

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("	Others"))
				{
					
					ImGui::Button("Game proccess", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
					ImGui::SameLine();
					ImGui::Button("Magnetizer", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
					
					ImGui::BeginChild("8", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.4f));
						ImGui::Checkbox("Skip cutscenes", &skip_cutscenes);
						ImGui::Checkbox("Speedup Dialogs", &speed_dial);
						//I have no idea how to make hotkey input so it will be input text.
						ImGui::InputText("Key##3", hotkey_skip_cutscenes, 128);
						ImGui::Separator();
						ImGui::Checkbox("Freeze mobs", &freeze_mobs);
						//I have no idea how to make hotkey input so it will be input text.
						ImGui::InputText("Key##4", hotkey_freeze_mobs, 128);
					ImGui::EndChild();

					ImGui::SameLine();
					ImGui::BeginChild("9", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 2)) * 0.4f));
						ImGui::Checkbox("Magnetize Oculus", &mag_oculus);
						ImGui::Checkbox("Magnetize Agate", &mag_agate);
						ImGui::Checkbox("Magnetize Crystal Ore", &mag_crystal);
						ImGui::Checkbox("Magnetize Metal Ore", &mag_metal);
						ImGui::Separator();
						ImGui::Checkbox("Magnetize Mobs", &mag_mobs);
						ImGui::SliderFloat("Radius", &mag_mobs_radius,0.0, 150.f, "%.3f");
						//I have no idea how to make hotkey input so it will be input text.
						ImGui::InputText("Key##5", hotkey_mag_mobs, 128);
					ImGui::EndChild();
					
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("	Settings"))
				{
					ImGui::Button("Settings", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, 28.f));
					//выпадающий список
					ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() / 4.f);
					ImGui::Combo("Language", &lang, "EN\0RU\0");
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();

			}
			
			ImGui::End();
		}

		////YouGame END MENU

		ImGui::Render();
		context->OMSetRenderTargets(1, &view, NULL);
		const float clearColor[] = { 0.f, 0.f, 0.f, 1.f };
		context->ClearRenderTargetView(view, clearColor);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (view) {
		view->Release();
		view = nullptr;
	}
	if (swapchain)
	{
		swapchain->Release();
		swapchain = nullptr;
	}
	if (context)
	{
		context->Release();
		context = nullptr;
	}
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (DestroyWindow(window))
	{
		UnregisterClassA(wc.lpszClassName, wc.hInstance);
	};

	return 0;
}