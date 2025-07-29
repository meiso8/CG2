#include "../Header/DebugUI.h"
#include"../Header/math/Normalize.h"
#include"../Header/Model.h"
#include"../Header/Input.h"
#include"../Header/Sprite.h"
#include"../Header/Sphere.h"
#include<numbers>

void DebugUI::Update() {


}

void DebugUI::ModelUpdate(Model& model) {

    ImGui::Begin("Wave");
    ImGui::DragFloat("time1", &model.GetWaveData(0).time, 0.03f);
    ImGui::DragFloat("amplitude1", &model.GetWaveData(0).amplitude, 0.03f);
    ImGui::DragFloat3("direction1", &model.GetWaveData(0).direction.x, 0.03f, 0.0f, 1.0f);
    ImGui::SliderFloat("frequency1", &model.GetWaveData(0).frequency, 1.0f, 10.0f);

    ImGui::DragFloat("time2", &model.GetWaveData(1).time, 0.03f);
    ImGui::DragFloat("amplitude2", &model.GetWaveData(1).amplitude, 0.03f);
    ImGui::DragFloat3("direction2", &model.GetWaveData(1).direction.x, 0.03f, 0.0f, 1.0f);
    ImGui::SliderFloat("frequency2", &model.GetWaveData(1).frequency, 1.0f, 10.0f);
    ImGui::End();

    Vector3 waveDirection1 = model.GetWaveData(0).direction;
    model.GetWaveData(0).direction = Normalize(waveDirection1);

    Vector3 waveDirection2 = model.GetWaveData(1).direction;
    model.GetWaveData(1).direction = Normalize(waveDirection2);

    ImGui::Begin("expansion");
    ImGui::DragFloat("expansionData", &model.GetExpansionData().expansion, 0.03f, 0.0f, 10.0f);
    ImGui::DragFloat("sphere", &model.GetExpansionData().sphere, 0.03f, 0.0f, 1.0f);
    ImGui::DragFloat("cube", &model.GetExpansionData().cube, 0.03f, 0.0f, 1.0f);
    ImGui::Checkbox("isSphere", &model.GetExpansionData().isSphere);

    ImGui::End();


}

void DebugUI::InputUpdate(Input& input) {

    ImGui::Begin("Input");
    ImGui::SliderFloat("polar", &input.GetSc().polar, -10.0f, 10.0f);
    ImGui::SliderFloat("azimuthal", &input.GetSc().azimuthal, -10.0f, 10.0f);
    ImGui::SliderFloat("radius", &input.GetSc().radius, -100.0f, 100.0f);
    ImGui::SliderFloat2("startPos", &input.GetOffset().x, -100.0f, 100.0f);
    ImGui::SliderFloat2("currentPos", &input.GetCurrentPos().x, -100.0f, 100.0f);

    ImGui::End();

}

void DebugUI::SpriteUpdate(Sprite& sprite) {
    ImGui::Begin("Sprite");
    ImGui::SliderFloat3("translation", &sprite.GetTranslateRef().x, 0.0f, 640.0f);
    ImGui::SliderFloat3("rotation", &sprite.GetRotateRef().x, 0.0f, std::numbers::pi_v<float>*2.0f);
    ImGui::SliderFloat3("scale", &sprite.GetScaleRef().x, 0.0f, 10.0f);
    ImGui::End();
}

void DebugUI::SphereUpdate(Sphere& sphere) {
    ImGui::Begin("Sphere");
    ImGui::SliderFloat3("uvTranslate", &sphere.GetUVTransform().translate.x, -100.0f, 100.0f);
    ImGui::SliderFloat3("uvRotation", &sphere.GetUVTransform().rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
    ImGui::SliderFloat3("uvScale", &sphere.GetUVTransform().scale.x, 0.0f, 10.0f);
    ImGui::End();
}