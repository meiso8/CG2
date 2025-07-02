#include"../Header/Texture.h"
#include"../Header/Camera.h"
#include"../Header/CommandList.h"
#include"../Header/ModelData.h"
#include"../Header/PSO.h"
#include"../Header/MaterialResource.h"
#include"../Header/TransformationMatrix.h"

class Model
{
public:

    Model(CommandList& commandList, D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature,
        PSO& pso,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& directionalLightResource,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& waveResource,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& expansionResource)
        : commandList_(&commandList), viewport_(&viewport),
        scissorRect_(&scissorRect), rootSignature_(rootSignature), pso_(&pso),
        directionalLightResource_(directionalLightResource),
        waveResource_(waveResource), expansionResource_(expansionResource)
    {
    }

    void Create(
        const std::string& directoryPath,
        const std::string& filename,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvDescriptorHeap);

    void CreateWorldVPResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    void PreDraw();
    void Draw(const Matrix4x4& worldMatrix, Camera& camera);

    Material* Getmaterial() { return materialResource_.GetMaterial(); };

    VertexData* GetVertexData() {
        return vertexData_;
    }

    void SetColor(const Vector4& color);

    ~Model();

private:
    ShaderResourceView srv_;
    CommandList* commandList_ = nullptr;
    D3D12_VIEWPORT* viewport_ = nullptr;
    D3D12_RECT* scissorRect_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    PSO* pso_ = nullptr;

    Camera* camera_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpDate_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> waveResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> expansionResource_;

    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };

    MaterialResource materialResource_;
    ModelData modelData_;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    VertexData* vertexData_ = nullptr;

    Texture* texture_ = nullptr;
};
