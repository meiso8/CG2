#include "../../Header/math/WorldTransform.h"  
#include "../../Header/math/MakeAffineMatrix.h"  
#include"../../Header/math/MakeIdentity4x4.h"

void WorldTransform::Initialize() {

    scale_ = { 1.0f,1.0f,1.0f };
    rotate_ = { 0.0f,0.0f,0.0f };
    translate_ = { 0.0f,0.0f,0.0f };
    matWorld_ = MakeIdentity4x4();
}


void WorldTransformUpdate(WorldTransform& worldTransform) {
    worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotate_, worldTransform.translate_);
}