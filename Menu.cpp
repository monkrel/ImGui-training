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

	ImFontConfig config;
	config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
	config.RasterizerMultiply = 1.125f;
	io.Fonts->AddFontFromFileTTF("C:\\Users\\m0nkrel\\AppData\\Local\\Microsoft\\Windows\\Fonts\\HurmeGeometricSans3-Regular.ttf", 17.0f, &config);
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style->WindowBorderSize = 0.0f;
	style->FramePadding = ImVec2(8.f, 8.f);
	style->WindowPadding = ImVec2(10.f, 6.f);
	style->GrabMinSize = 24.f;
	style->WindowBorderSize = 0.f;
	style->WindowMenuButtonPosition = ImGuiDir_None;
	style->ColorButtonPosition = ImGuiDir_Left;
	style->FrameRounding = 3.f;
	style->GrabRounding = 4.f;
	style->ItemSpacing = ImVec2(8.f, 4.0f);


	style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.f);
	//style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	/*style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);*/
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.153f, 0.157f, 0.227f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.176f, 0.176f, 0.247f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	/*style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);*/
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.310f, 0.310f, 0.310f, 0.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.310f, 0.310f, 0.310f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.410f, 0.410f, 0.410f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.510f, 0.510f, 0.510f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.07f, 0.07f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.176f, 0.176f, 0.247f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.800f, 0.557f, 0.00f, 1.00f);
	/*style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);*/
	style->Colors[ImGuiCol_Separator] = ImVec4(0.039f, 0.039f, 0.078f, 0.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.039f, 0.039f, 0.078f, 0.00f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.039f, 0.039f, 0.078f, 0.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 0.00f);
	style->Colors[ImGuiCol_Tab] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.070f, 0.070f, 0.130f, 1.00f);
	style->Colors[ImGuiCol_TabActive] = ImVec4(0.039f, 0.039f, 0.078f, 1.00f);
	


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
		
		static bool checkbox = false;
		static bool offscreen = true;

		static bool isOpen = false;
		if (GetAsyncKeyState(VK_INSERT) & 0x1) isOpen = !isOpen;
		
		if (isOpen)
		{
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::SetNextWindowSize(ImVec2((io.DisplaySize.x) / 2.f, (io.DisplaySize.y) / 1.45f));
			ImGui::Begin("Unicore", NULL, flags);
			//style->FramePadding.y = 4.f;
			if (ImGui::BeginTabBar("###Main Tab Bar", ImGuiTabBarFlags_NoTooltip))
			{
				if (ImGui::BeginTabItem("    Visuals    "))
				{
					if (ImGui::BeginTabBar("Lol", ImGuiTabBarFlags_NoTooltip))
					{
						if (ImGui::BeginTabItem("Global"))
						{
							ImGui::BeginChild("1", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 3)) / 2.5f));
							ImGui::Checkbox("Chest (Common)", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::Checkbox("Chest (Exquisite)", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::Checkbox("Chest (Luxorius)", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::Checkbox("Wind Slime", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::Checkbox("Ice Bulk", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::Checkbox("Search Point", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::Checkbox("Bloatty Floatty", &checkbox);
							//ImGui::SameLine();
							//ImGui::Checkbox("Offscreen", &offscreen);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("2", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 3)) / 2.5f));
							ImGui::Checkbox("     Chest2", &checkbox);
							ImGui::EndChild();

							ImGui::BeginChild("3", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 3)) / 2.5f));
							ImGui::Checkbox("     Chest3", &checkbox);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("4", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 3)) / 2.5f));
							ImGui::Checkbox("     Chest4", &checkbox);
							ImGui::EndChild();

							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Local"))
						{
							ImGui::BeginChild("1", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 3)) / 2.5f));
							ImGui::Checkbox("     Chest2", &checkbox);
							ImGui::EndChild();

							ImGui::SameLine();
							ImGui::BeginChild("2", ImVec2(ImGui::GetWindowContentRegionWidth() / 2.f, (ImGui::GetWindowSize().y - (style->FramePadding.y * 3)) / 2.5f));
							ImGui::Checkbox("     Chest3", &checkbox);
							ImGui::EndChild();

							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
			
				if (ImGui::BeginTabItem("Player"))
				{
					ImGui::Text("Player tab");
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Others"))
				{
					ImGui::Text("Others tab");
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Unsafe"))
				{
					ImGui::Text("Unsafe tab");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Settings"))
				{
					ImGui::Text("Settings tab");
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();

			}

			ImGui::End();
			//ImGui::ShowStyleEditor();
		}

		//end
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