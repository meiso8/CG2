#pragma once

#include"../Header/ImGuiClass.h"

class Model;
class Input;
class Sprite;

class DebugUI
{
public:
    void Update();
    void ModelUpdate(Model& model);
    void InputUpdate(Input& input);
    void SpriteUpdate(Sprite& sprite);
};

