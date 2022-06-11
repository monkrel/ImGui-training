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
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.0f, &config);

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
		

		//simple csgo cheat menu
		ImGui::Begin("CSGO Cheat Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("CSGO Cheat Menu");
		ImGui::Separator();
		ImGui::Text("Version: 1.0");
		ImGui::Text("Author: Kuroko");
		ImGui::Text("Discription: Simple CSGO Cheat Menu");
		ImGui::Separator();
		ImGui::Text("Features:");
		ImGui::Text("- ESP");
		ImGui::Text("- Triggerbot");
		ImGui::Text("- Aimbot");
		ImGui::Text("- Bunnyhop");
		ImGui::Text("- Noflash");
		ImGui::Text("- No recoil");
		ImGui::Text("- No spread");
		ImGui::End();

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