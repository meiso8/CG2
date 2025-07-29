#pragma once

#include"../Header/ImGuiClass.h"

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
};

