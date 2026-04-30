// main.cpp
// Dear ImGui - Two Button Example (DirectX 11 + Win32)
// Buttons rendered directly in the main window (no separate ImGui window)

#pragma once

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <vector>
#include <list>
#include <string>
#include <fstream>

#include "ParseFromInput.h"
#include "WriteToFiles.h"
#include "CudaynamicsDirectory.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void MakeTextFile(std::string systemName, std::vector<std::string> varNames, std::vector<std::string> varEqs, int varAmount);
bool checkNameCorrectness(std::string line);
bool checkEqsCorrectness(std::string line);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::filesystem::path getExecutableDirectory();

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    //WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr) };
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"CUDAynamicsEditor", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"CUDAynamics System Editor", WS_OVERLAPPEDWINDOW, 100, 100, 800, 500, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();     // Or ImGui::StyleColorsClassic()

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Main window variables
    bool createButton_clicked = false;
    bool button2_clicked = false;
    bool exitButton_clicked = false;

    // Create window variables
    std::string systemName;
    std::vector<std::string> varNames;
    std::vector<std::string> varEqs;
    std::vector<bool> isDerivative;
    int varAmount = 0;
    bool codeC = true;
    bool valuesNotCorrect = false;
    bool valuesNotMatching = false;
    bool showHelpWindow = false;

    // Main loop
    bool done = false;
    
    //fonts
    ImFont* font_default = io.Fonts->AddFontDefault();
    ImFont* font_title = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 40.0f);
    ImFont* font_large = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 32.0f);

    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ============================================
        // UI Elements - No separate window, just floating elements
        // ============================================

        // Get window size to center the UI
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;

        // Set next window position to center of screen
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Fullscreen", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);



        if (!createButton_clicked) {
            ImGui::PushFont(font_title);
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("CUDAynamics System Editor").x) * 0.5f);
            ImGui::Text("CUDAynamics System Editor");
            ImGui::PopFont();
            ImGui::Separator();

            // Center the buttons horizontally
            float buttonWidth = 400.0f;
            float windowWidth = ImGui::GetWindowWidth();


            ImGui::PushFont(font_large);
            ImGui::SetCursorPosX((windowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
            if (ImGui::Button("Create System", ImVec2(buttonWidth, 0)))
            {
                createButton_clicked = true;
            }
            ImGui::Text(" ");
            ImGui::Text(" ");
            ImGui::SetCursorPosX((windowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
            if (ImGui::Button("Delete System", ImVec2(buttonWidth, 0)))
            {
                button2_clicked = true;
            }
            ImGui::Text(" ");
            ImGui::Text(" ");
            ImGui::SetCursorPosX((windowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
            if (ImGui::Button("Exit", ImVec2(buttonWidth, 0)))
            {
                exitButton_clicked = true;
            }
            ImGui::PopFont();


            if (button2_clicked)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Button 2 was clicked!");
                button2_clicked = false; 
            }

            if (exitButton_clicked)
            {
                done = true;
            }
        }
        else if (createButton_clicked) {

            if (ImGui::Button("Back to menu")) {
                createButton_clicked = false;
                for (int i = 0; i < varAmount; i++) {
                    varNames.pop_back();
                    varEqs.pop_back();
                    isDerivative.pop_back();
                }
                varAmount = 0;
                systemName = "";
                valuesNotCorrect = false;
            }
            ImGui::SameLine();
            ImGui::PushFont(font_large);
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Create system").x) * 0.5f);
            ImGui::Text("Create system");
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::SetCursorPosX((ImGui::GetWindowWidth()) * 0.92f);

            if (ImGui::Button("?", ImVec2(30, 0))) {
                showHelpWindow = !showHelpWindow; 
            }
            if (showHelpWindow)
            {
                ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
                if (ImGui::Begin("System Editor Help", &showHelpWindow, ImGuiWindowFlags_NoCollapse))
                {
                    ImGui::Text("Help Information");
                    ImGui::Separator();
                    ImGui::Text("How to create a system:");
                    ImGui::BulletText("Enter a system name");
                    ImGui::BulletText("Click '+' to add variables");
                    ImGui::BulletText("Enter variable names and equations");
                    ImGui::BulletText("Use the checkboxes to toggle between equation types");
                    ImGui::BulletText("Click 'X' to remove a variable");
                    ImGui::BulletText("Variable names must be unique and contain no special characters,\n   except \"_\" and spaces");
                    ImGui::BulletText("Equations must contain only allowed mathematical operations");
                    ImGui::BulletText("Click 'Create' to generate the system");
                    ImGui::Separator();
                    ImGui::Text("Equations are expected to be written in C++ expressions (without semicolo \";\").\nAllowed functions are as follows: abs, fabs, exp, sqrt,\n   pow, sin, cos, log, min, max, fmin, fmax, fmod.\nTernary conditional statements are also allowed\n   condition ? value_if_true : value_if_false\n   Ex: var1 < par ? par : var1 ");
                }
                ImGui::End();
            }

            ImGui::Separator();

            ImGui::Checkbox("C++ code", &codeC);
            ImGui::Separator();

            ImGui::Text("System Name: "); ImGui::SameLine();
            ImGui::InputText("##SystemNameTextID", &systemName); ImGui::SameLine(); ImGui::Text("System");
            ImGui::Text(" ");
            
            for (int i = 0; i < varAmount; i++) {
                bool isSignal;
                if (isDerivative[i] == true) {
                    std::string labelText = "Diff. Eq-n of Variable " + std::to_string(i + 1) + ":";
                    ImGui::Text("%s", labelText.c_str());
                }
                else {
                    std::string labelText = "Eq-n of Variable " + std::to_string(i + 1) + ":";
                    ImGui::Text("%s", labelText.c_str());
                }
                
                ImGui::SameLine();

                std::string inputID = "##Variable" + std::to_string(i);

                if (isDerivative[i] == true) ImGui::PushItemWidth((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Diff. Eq-n of Variable ").x)* 0.2f);
                else ImGui::PushItemWidth((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Eq-n of Variable ").x) * 0.2f);
                ImGui::InputText(inputID.c_str(), &varNames[i]);
                ImGui::PopItemWidth();

                ImGui::SameLine();

                ImGui::Text(" = ");

                ImGui::SameLine();

                if (varEqs[i] == "signal") { isSignal = true; ImGui::Text("signal"); }
                else {
                    isSignal = false;
                    inputID = "##Equation" + std::to_string(i);
                    if (isDerivative[i] == true) ImGui::PushItemWidth((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Diff. Eq-n of Variable ").x)* 0.55f);
                    else ImGui::PushItemWidth((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Eq-n of Variable ").x) * 0.55f);
                    ImGui::InputText(inputID.c_str(), &varEqs[i]);
                    ImGui::PopItemWidth();
                }
                ImGui::SameLine();

                ImGui::PushID(i);
                if (ImGui::Button("X")) {
                    varNames.erase(varNames.begin() + i);
                    varEqs.erase(varEqs.begin() + i);
                    isDerivative.erase(isDerivative.begin() + i);
                    varAmount--;
                    i--;
                }
                ImGui::PopID();

                
                

                ImGui::Text("       ");
                ImGui::SameLine();
                inputID = "Is Signal##" + std::to_string(i);
                ImGui::Checkbox(inputID.c_str(), &isSignal);
                if (isSignal) { varEqs[i] = "signal"; isDerivative[i] = false; }
                else if (varEqs[i] == "signal") { varEqs[i] = ""; isDerivative[i] = true; }

                ImGui::SameLine();
                ImGui::Text("       ");
                ImGui::SameLine();
                inputID = "Is Diff Eq-n##" + std::to_string(i);
                bool tmpbool = isDerivative[i];
                ImGui::Checkbox(inputID.c_str(), &tmpbool);
                isDerivative[i] = tmpbool;

                ImGui::Text(" ");

            }
            if (ImGui::Button("+", ImVec2(60, 0))) {
                varAmount++;
                varNames.push_back("");
                varEqs.push_back("");
                isDerivative.push_back(true);
                
            }

            ImGui::Text(" ");
            if (valuesNotCorrect) { ImGui::Text("Input text is incorrect. Check text input boxes for dissalowed symbols and check for repeating variable names."); }
            float buttonWidth = 80.0f;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth - ImGui::GetStyle().ItemSpacing.x) * 0.9f);
            if (ImGui::Button("Create", ImVec2(buttonWidth, 50.0f))) {
                valuesNotCorrect = false;
                if (varAmount < 1)valuesNotCorrect = true;
                if (!valuesNotCorrect) valuesNotCorrect = checkNameCorrectness(systemName);
                if(!valuesNotCorrect)
                    for (int count = 0; count < varAmount; count++) {
                        for (int j = count+1; j < varAmount; j++) {
                            if(j<varAmount)
                                if (varNames[j] == varNames[count]) { valuesNotCorrect = true; break; }
                        }
                        if (valuesNotCorrect)break;
                        valuesNotCorrect = checkNameCorrectness(varNames[count]);
                        if (valuesNotCorrect)break;
                        valuesNotCorrect = checkEqsCorrectness(varEqs[count]);
                        if (valuesNotCorrect)break;
                    }

                

                
                if (!valuesNotCorrect) {
                    valuesNotCorrect = false;

                    systemStruct systemData;
                    systemData.systemNameTXT = systemName;
                    systemData.varNames = varNames;
                    systemData.varEqs = varEqs;
                    systemData.isDerivative = isDerivative;
                    
                    mainDataProcess(&systemData);

                    


                    std::filesystem::path exeDir = getExecutableDirectory();
                    std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
                    std::filesystem::path cudaynamicsPath = projectRoot / ReturnDirectory();

                    WriteMain(systemData, cudaynamicsPath);

                    createButton_clicked = false;
                    for (int i = 0; i < varAmount; i++) {
                        varNames.pop_back();
                        varEqs.pop_back();
                        isDerivative.pop_back();
                    }
                    varAmount = 0;
                    systemName = "";
                }
            }
        }

       

        ImGui::End();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool checkNameCorrectness(std::string line) {
    bool notBlank = false;

    if (line.size() == 0) return 1;

    if (line[0] < 65 || line[0]>122 || (line[0] > 90 && line[0] < 97)) {
        return 1;
    }

    for (int i = 0; i < line.size(); i++) {
        if (line[i] != 32) notBlank = true;
        if (line[i] < 32 || (line[i] > 32 && line[i] < 45 )|| (line[i] > 45 && line[i] < 48) || (line[i] > 57 && line[i] < 65) || (line[i] > 90 && line[i] < 97) || line[i] > 122) return 1;
        
    }
    if (!notBlank)return 1;
    return 0;
}

bool checkEqsCorrectness(std::string line) {
    if (line.size() == 0) return 1;
    for (int i = 0; i < line.size(); i++) {
        if (line[i]<32 || (line[i] >32 && line[i] < 37) || line[i] == 39 || (line[i]>57 && line[i] < 60) || line[i] == 64 || (line[i]>90 && line[i] < 97) || line[i] > 122) return 1;
    }
    return 0;
}

void MakeTextFile(std::string systemName, std::vector<std::string> varNames, std::vector<std::string> varEqs, int varAmount) {
    std::ofstream outputFile("Output.txt");
    outputFile << systemName << '\n';
    for (int count = 0; count < varAmount; count++) {
        outputFile << varNames[count] << " = " << varEqs[count]<<'\n';
    }
    outputFile.close();
}

std::filesystem::path getExecutableDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    // Uncomment to enable debug layer
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware doesn't support
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}