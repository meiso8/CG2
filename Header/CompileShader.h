#pragma once

#include <dxcapi.h>
#include<string>
#include<wrl.h>


//CompileShader関数
IDxcBlob* CompileShader(
    //CompilerするShaderファイルへのパス
    const std::wstring& filePath,
    //Compilerに使用するProfile
    const wchar_t* profile,
    //初期化で生成されたものを3つ
    IDxcUtils* dxcUtils,
    IDxcCompiler3* dxcCompiler,
    IDxcIncludeHandler* includeHandler);

class DxcCompiler
{
public:
    void Initialize();
    void ShaderSeting();
    Microsoft::WRL::ComPtr <IDxcBlob>& GetVertexShaderBlob() { return vertexShaderBlob_; };
    Microsoft::WRL::ComPtr <IDxcBlob>& GetPixelShaderBlob() { return pixelShaderBlob_; };
private:
    IDxcUtils* dxcUtils_ = nullptr;
    IDxcCompiler3* dxcCompiler_ = nullptr;
    IDxcIncludeHandler* includeHandler_ = nullptr;
    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob_;
    Microsoft::WRL::ComPtr <IDxcBlob>pixelShaderBlob_;
};