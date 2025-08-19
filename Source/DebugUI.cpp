#include "DebugUI.h"
#include"math/Normalize.h"
#include"Model.h"
#include"Input.h"
#include"Sprite.h"
#include"SphereMesh.h"
#include<numbers>
#include<algorithm>
#include"Camera.h"

#include "../Game/Mirror.h"
void DebugUI::Update() {


}

void DebugUI::CameraUpdate(Camera& camera) {
    ImGui::Begin("Camera");

    ImGui::SliderFloat3("translate", &camera.GetTranslate().x, -100.0f, 100.0f);
    ImGui::SliderFloat3("rotate", &camera.GetRotate().x, -100.0f, 100.0f);




    ImGui::End();



}

void DebugUI::DebugMirror(std::list<Mirror*>mirrors) {
    ImGui::Begin("Model");

    for (Mirror* mirror : mirrors) {
        if (ImGui::TreeNode("mirrors")) {

            Vector3 pos = mirror->GetWorldPosition();
            ImGui::DragFloat3("worldPos", &pos.x, 0.03f, 0.0f, 1000.0f);
            ImGui::Text("%s", (mirror->IsBroken()) ? "broken" : "safe");
            ImGui::TreePop();
        }
    }


    ImGui::End();


}

void DebugUI::ModelUpdate(Model& model) {

    ImGui::Begin("Model");

    if (ImGui::TreeNode("Wave")) {

        ImGui::DragFloat("time", &model.GetWaveData(0).time, 0.03f);
        ImGui::DragFloat("amplitude", &model.GetWaveData(0).amplitude, 0.03f);
        ImGui::DragFloat3("direction", &model.GetWaveData(0).direction.x, 0.03f, 0.0f, 1.0f);
        ImGui::SliderFloat("frequency", &model.GetWaveData(0).frequency, 1.0f, 10.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Wave2")) {
        ImGui::DragFloat("time", &model.GetWaveData(1).time, 0.03f);
        ImGui::DragFloat("amplitude", &model.GetWaveData(1).amplitude, 0.03f);
        ImGui::DragFloat3("direction", &model.GetWaveData(1).direction.x, 0.03f, 0.0f, 1.0f);
        ImGui::SliderFloat("frequency", &model.GetWaveData(1).frequency, 1.0f, 10.0f);

        ImGui::TreePop();
    }


    Vector3 waveDirection1 = model.GetWaveData(0).direction;
    model.GetWaveData(0).direction = Normalize(waveDirection1);


    Vector3 waveDirection2 = model.GetWaveData(1).direction;
    model.GetWaveData(1).direction = Normalize(waveDirection2);

    if (ImGui::TreeNode("Expansion")) {

        ImGui::DragFloat("expansionData", &model.GetExpansionData().expansion, 0.03f, 0.0f, 10.0f);
        ImGui::DragFloat("sphere", &model.GetExpansionData().sphere, 0.03f, 0.0f, 1.0f);
        ImGui::DragFloat("cube", &model.GetExpansionData().cube, 0.03f, 0.0f, 1.0f);
        ImGui::Checkbox("isSphere", &model.GetExpansionData().isSphere);

        ImGui::TreePop();
    }


    if (ImGui::TreeNode("UV")) {

        ImGui::SliderFloat3("Translate", &model.GetUVTransform().translate.x, -100.0f, 100.0f);
        ImGui::SliderFloat3("Rotation", &model.GetUVTransform().rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
        ImGui::SliderFloat3("Scale", &model.GetUVTransform().scale.x, 0.0f, 100.0f);
        ImGui::ColorEdit4("color", &model.GetColor().x);
        ImGui::TreePop();
    }

    ImGui::End();

}

void DebugUI::InputUpdate(Input& input) {

    ImGui::Begin("Input");
    ImGui::SliderFloat("polar", &input.GetSc().polar, -10.0f, 10.0f);
    ImGui::SliderFloat("azimuthal", &input.GetSc().azimuthal, -10.0f, 10.0f);
    ImGui::SliderFloat("radius", &input.GetSc().radius, -100.0f, 100.0f);
    ImGui::SliderFloat2("startPos", &input.GetOffset().x, -100.0f, 100.0f);
    ImGui::SliderFloat2("currentPos", &input.GetCurrentPos().x, -100.0f, 100.0f);


    Vector2 normL = Normalize(Vector2(static_cast<float>(input.GetJoyState().lX), static_cast<float>(input.GetJoyState().lY)));

    ImGui::Text("normLX: %f %f", normL.x,normL.y);//x軸位置


    ImGui::Text("joyStateLX: %ld", input.GetJoyState().lX);//x軸位置
    ImGui::Text("joyStateLY: %ld", input.GetJoyState().lY);
    ImGui::Text("joyStateLZ: %ld", input.GetJoyState().lZ);
    ImGui::Text("joyStateRX: %ld", input.GetJoyState().lRx);//右スティック
    ImGui::Text("joyStateRY: %ld", input.GetJoyState().lRy);

    if (input.GetJoyState().rgdwPOV[0] != -1) {//十字キー　角度を表す
        ImGui::Text("POV[%d]: %lu", 0, input.GetJoyState().rgdwPOV[0]);
    } else {
        ImGui::Text("POV[%d]: Centered", 0);
    }

    for (int i = 0; i < 12; ++i) {
        ImGui::Text("Button[%d]: %s", i,
            (input.GetJoyState().rgbButtons[i] & 0x80) ? "Pressed" : "Released");
    }

    ImGui::End();

}

void DebugUI::SpriteUpdate(Sprite& sprite) {
    ImGui::Begin("Sprite");
    ImGui::SliderFloat3("translation", &sprite.GetTranslateRef().x, 0.0f, 640.0f);
    ImGui::SliderFloat3("rotation", &sprite.GetRotateRef().x, 0.0f, std::numbers::pi_v<float>*2.0f);
    ImGui::SliderFloat3("scale", &sprite.GetScaleRef().x, 0.0f, 10.0f);
    ImGui::End();
}

void DebugUI::SphereUpdate(SphereMesh& sphere) {
    ImGui::Begin("Sphere");
    ImGui::SliderFloat3("uvTranslate", &sphere.GetUVTransform().translate.x, -100.0f, 100.0f);
    ImGui::SliderFloat3("uvRotation", &sphere.GetUVTransform().rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
    ImGui::SliderFloat3("uvScale", &sphere.GetUVTransform().scale.x, 0.0f, 100.0f);
    ImGui::ColorEdit4("color", &sphere.GetColor().x);
    ImGui::End();

    ImGui::Begin("SphereExpansion");
    ImGui::DragFloat("expansionData", &sphere.GetExpansionData().expansion, 0.03f, 0.0f, 10.0f);
    ImGui::DragFloat("sphere", &sphere.GetExpansionData().sphere, 0.03f, 0.0f, 1.0f);
    ImGui::DragFloat("cube", &sphere.GetExpansionData().cube, 0.03f, 0.0f, 1.0f);
    ImGui::Checkbox("isSphere", &sphere.GetExpansionData().isSphere);

    ImGui::End();
}

void DebugUI::WorldMatrixUpdate(Vector3& scale, Vector3& rotate, Vector3& translate, const char* label) {

    if (ImGui::TreeNode(label)) {
        ImGui::SliderFloat3("translation", &translate.x, -10.0f, 10.0f);
        ImGui::SliderFloat3("rotation", &rotate.x, 0.0f, std::numbers::pi_v<float>*2.0f);
        ImGui::SliderFloat3("scale", &scale.x, 0.0f, 10.0f);
        ImGui::TreePop();
    }

};

void DebugUI::Color( Vector4& color) {
    ImGui::Begin("Color");
    ImGui::ColorEdit4("color", (float*)&color);
    ImGui::End();
}