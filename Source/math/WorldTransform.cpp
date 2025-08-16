#include "WorldTransform.h"  
#include "MakeAffineMatrix.h"  
#include"MakeIdentity4x4.h"

void WorldTransform::Initialize() {

    scale_ = { 1.0f,1.0f,1.0f };
    rotate_ = { 0.0f,0.0f,0.0f };
    translate_ = { 0.0f,0.0f,0.0f };
    matWorld_ = MakeIdentity4x4();
}


void WorldTransformUpdate(WorldTransform& worldTransform) {
    worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotate_, worldTransform.translate_);
}