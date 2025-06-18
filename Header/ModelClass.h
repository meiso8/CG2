#pragma once  
#include"../Header/CommandList.h"  
#include"../Header/ModelData.h"  
#include"../Header/PSO.h"  
#include"../Header/ShaderResourceView.h"  

class ModelClass  
{  
public:  
  
   void Draw(  
       CommandList& commandList,
       D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
       const Microsoft::WRL::ComPtr <ID3D12Resource>& materialResource,
       const Microsoft::WRL::ComPtr <ID3D12Resource>& wvpResource,
       ShaderResourceView& srv
   );  

   void DrawCall(CommandList& commandList, ModelData& modelData);

private:  
};
