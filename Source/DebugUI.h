#pragma once

#include"ImGuiClass.h"
#include"math/Vector3.h"
#include"math/Vector4.h"

class Model;
class Input;
class Sprite;
class SphereMesh;
class Mirror;
class Camera;

#include<list>

class DebugUI
{
public:
    void Update();
    void ModelUpdate(Model& model);
    void InputUpdate(Input& input);
    void SpriteUpdate(Sprite& sprite);
    void SphereUpdate(SphereMesh& sphere);
    void WorldMatrixUpdate(Vector3& scale, Vector3& rotate, Vector3& translate, const char* label);
    void DebugMirror(std::list<Mirror*>mirrors);
    void Color( Vector4& color);
    void CameraUpdate(Camera& camera);
};

