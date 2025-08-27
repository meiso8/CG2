#pragma once

#include"ImGuiClass.h"
#include"Vector3.h"
#include"Vector4.h"
#include"WorldTransform.h"

class Model;
class Input;
class Sprite;
class SphereMesh;
class Mirror;
class Camera;
class Hammer;
class Player;
class Dove;
class FPSCounter;
struct DirectionalLight;
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
    void HammerUpdate(Hammer& hammer);
    void UpdatePlayer(Player& player);
    void DoveUpdate(Dove& dove);
    void CheckInt(int& value);
    void CheckFPS(FPSCounter& fpsCounter);
    void CheckDirectionalLight(DirectionalLight& directionalLights);
    void WorldTransformUpdate(WorldTransform& worldTransform);

};

