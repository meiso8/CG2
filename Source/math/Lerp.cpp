#include "Lerp.h"

KamataEngine::Vector3 Lerp(KamataEngine::Vector3 start, KamataEngine::Vector3 end, float t) { 
    
    
    return {start * (1.0f - t) + end * t}; 



};
