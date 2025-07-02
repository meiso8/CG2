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

    Model(Camera& camera, CommandList& commandList, D3D12_VIEWPORT& viewport,
        D3D12_RECT& scissorRect,
        const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature,
        PSO& pso,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& directionalLightResource,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& waveResource,
        const Microsoft::WRL::ComPtr<ID3D12Resource>& expansionResource)
        : camera_(&camera), commandList_(&commandList), viewport_(&viewport),
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

    void CreateWorldVP(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

    void Update();

    void InitTraslate();

    void PreDraw();
    void Draw();

    Material* Getmaterial() { return materialResource_.GetMaterial(); };

    Transform& GetTransformRef() {
        return transform_;
    };

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

    Transform transform_ = { 0.0f };
    Matrix4x4 worldMatrix_ = { 0.0f };
    Matrix4x4 worldViewProjectionMatrix_ = { 0.0f };

    MaterialResource materialResource_;
    ModelData modelData_;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    VertexData* vertexData_ = nullptr;

    Texture* texture_ = nullptr;
};
