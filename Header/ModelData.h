#pragma once
#include<vector>
#include"../Header/VertexData.h"
#include"../Header/MaterialData.h"

struct ModelData {
    std::vector<VertexData> vertices;
    MaterialData material;
};

/// @brief ModelDataの読み込み関数/// @param directoryPath ディレクトリパス/// @param filename ファイル名/// @return　モデルデータ 
ModelData LoadObjeFile(const std::string& directoryPath, const std::string& filename);