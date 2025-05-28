#pragma once
#include<vector>
#include"../Header/VertexData.h"
//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>

struct ModelData {
    std::vector<VertexData> vertices;
};

/// @brief ModelDataの読み込み関数/// @param directoryPath ディレクトリパス/// @param filename ファイル名/// @return　モデルデータ 
ModelData LoadObjeFile(const std::string& directoryPath, const std::string& filename);