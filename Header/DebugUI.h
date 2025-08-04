#pragma once

#include"../Header/ImGuiClass.h"
#include"../Header/math/Vector3.h"
#include"../Header/math/Vector4.h"

class Model;
class Input;
class Sprite;
class Sphere;
class Mirror;

#include<list>

class DebugUI
{
public:
    void Update();
    void ModelUpdate(Model& model);
    void InputUpdate(Input& input);
    void SpriteUpdate(Sprite& sprite);
    void SphereUpdate(Sphere& sphere);
    void WorldMatrixUpdate(Vector3& scale, Vector3& rotate, Vector3& translate, const char* label);
    void DebugMirror(std::list<Mirror*>mirrors);
    void Color( Vector4& color);
};

