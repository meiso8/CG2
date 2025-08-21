#include "SpriteFont.h"


#include <SpriteBatch.h>
#include <SpriteFont.h>

// 初期化部分
void SpriteFont::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    std::unique_ptr<SpriteBatch> spriteBatch;
    std::unique_ptr<SpriteFont> spriteFont;

    spriteBatch = std::make_unique<SpriteBatch>(deviceContext);
    spriteFont = std::make_unique<SpriteFont>(device, L"myfile.spritefont");
}

// 描画部分
void SpriteFont::Render() {
    spriteBatch->Begin();

    DirectX::XMFLOAT2 position(100, 100);
    const wchar_t* text = L"こんにちは、SpriteFont！";

    spriteFont->DrawString(spriteBatch.get(), text, position);

    spriteBatch->End();
}
