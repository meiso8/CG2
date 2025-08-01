#pragma once

#include"../Header/ImGuiClass.h"
#include"../Header/math/Vector3.h"

class Model;
class Input;
class Sprite;
class Sphere;

class DebugUI
{
public:
    void Update();
    void ModelUpdate(Model& model);
    void InputUpdate(Input& input);
    void SpriteUpdate(Sprite& sprite);
    void SphereUpdate(Sphere& sphere);
    void WorldMatrixUpdate(Vector3& scale, Vector3& rotate, Vector3& translate, const char* label);

};

