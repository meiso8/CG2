#include"../Header/DrawGrid.h"
#include<numbers>

DrawGrid::DrawGrid(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Camera& camera, ModelConfig& mc) {


    for (int i = 0; i < 102; ++i) {
        line_[i].Create(device, camera, mc);
        line_[i].SetVertexPos(Vector3(-25.0f, -0.005f, 0.0f), Vector3(25.0f, 0.005f, 0.0f));
    }

    for (int i = 0; i < 51; ++i) {
        line_[i].SetTranslate({ 0.0f,0.0f, static_cast<float>(i - 25) });
    }

    for (int i = 0; i < 51; ++i) {
        line_[i + 51].SetRotate(Vector3{ 0.0f,std::numbers::pi_v<float> / 2.0f,0.0f });
        line_[i + 51].SetTranslate({ static_cast<float>(i - 25),0.0f, 0.0f });
    }

    for (int i = 0; i < 51; ++i) {
        if (i % 10 == 0) {
            line_[i].SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

            line_[i + 51].SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        } else {
            line_[i].SetColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
            line_[i + 51].SetColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
        }
    }


    line_[25].SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    line_[25].SetScale(Vector3(1.0f, 10.0f, 10.0f));

    line_[76].SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    line_[76].SetScale(Vector3(1.0f, 10.0f, 10.0f));

}

void DrawGrid::Draw(ShaderResourceView& srv) {

    line_[0].PreDraw();

    for (int i = 0; i < 102; ++i) {
        line_[i].Update();
        line_[i].Draw(srv);
    }


}