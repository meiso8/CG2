#include"../Header/Texture.h"
#include"../Header/Camera.h"
#include"../Header/ModelData.h"
#include"../Header/MaterialResource.h"
#include"../Header/TransformationMatrix.h"
#include"../Header/Config.h"
#include"../Header/Balloon.h"
#include"../Header/Wave.h"

class Model
{
public:

    Model(ModelConfig mc)
        : modelConfig_(mc)
    {
    }

    void Create(
        const std::string& directoryPath,
        const std::string& filename,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap, uint32_t index);

    void PreDraw();
    void Draw(const Matrix4x4& worldMatrix, Camera& camera);

    Material* GetMaterial() { return materialResource_.GetMaterial(); };

    VertexData* GetVertexData() {
        return vertexData_;
    }

    Balloon& GetExpansionData() {
        return *expansionData_;
    }

    Wave& GetWaveData(size_t index) { return waveData_[index]; };

    void SetColor(const Vector4& color);

    ~Model();

private:
    void CreateWorldVPResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
private:
    ShaderResourceView srv_;

    Camera* camera_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpDate_ = nullptr;

    ModelConfig modelConfig_;

    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };
    MaterialResource materialResource_;
    ModelData modelData_;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    VertexData* vertexData_ = nullptr;

    Texture* texture_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> expansionResource_;
    Balloon* expansionData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> waveResource_;
    Wave* waveData_ = nullptr;


};
