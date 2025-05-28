#include"../Header/ModelData.h"

#include <fstream>//ファイルの書いたり読んだりするライブラリ
#include<cassert> //assertも利用するため

ModelData LoadObjeFile(const std::string& directoryPath, const std::string& filename) {

    //1.中で必要となる変数の宣言
    ModelData modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;//ファイルから読み込んだ1行を格納するもの

    //2.ファイルを開く
    std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
    assert(file.is_open());//開けなかったら止める

    //3.実際にファイルを読み、ModelDataを構成していく
    //std::getlineはstreamから1行読んでstd::stringに格納する関数
    while (std::getline(file, line)) {
        std::string identifier;
        //文字列を分解しながら読むためのクラス（空白を区切り文字として読むことが可能）
        std::istringstream s(line); //これに対して>>すると空文字区切りで1ずつ値を読むことが出来る
        s >> identifier;//先頭の識別子を読む　IDを読む

        //IDに応じた処理
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.y *= -1.0f;//座標系の統一のため
            position.w = 1.0f;//同次座標のため
            positions.push_back(position);

        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoords.push_back(texcoord);

        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.y *= -1.0f;//座標系の統一のため
            normals.push_back(normal);
        } else if (identifier == "f") {
            //三角形を作る

            //面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                //頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');//区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }

                //要素へのIndexから、実際の要素の値を取得して、頂点を構成する
                Vector4 position = positions[elementIndices[0] - 1];//1始まりなので-1する
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];

                //まずobj通りに保存、格納する際に逆にする　座標系の統一のため
                VertexData triangle = { position,texcoord,normal };
                modelData.vertices.push_back(triangle);
            }

            //////頂点を逆順で登録することで、回り順を逆順にする
            //modelData.vertices.push_back(triangle[0]);
            //modelData.vertices.push_back(triangle[1]);
            //modelData.vertices.push_back(triangle[2]);

        }

    }

    //4.ModelDataを返す
    return modelData;

}
