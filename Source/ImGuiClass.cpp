#include "../Header/ImGuiClass.h"
#include"../Header/CommandList.h"


#ifdef _DEBUG

void ImGuiClass::Initialize(HWND hWnd,
    const Microsoft::WRL::ComPtr<ID3D12Device>& device,
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc,
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc,
    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap) {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX12_Init(device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        srvDescriptorHeap.Get(),
        srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiClass::FrameStart() {


    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

}

void ImGuiClass::Render() {

    //ImGuiの内部コマンドを生成する
    ImGui::Render();

}

void ImGuiClass::DrawImGui(CommandList& commandList) {

    //諸々の描画処理が終了下タイミングでImGuiの描画コマンドを積む
//実際のcommandListのImGuiの描画コマンドを積む
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.GetComandList().Get());
}

void ImGuiClass::ShutDown() {

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

#endif // _DEBUG
