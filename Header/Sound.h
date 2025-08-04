#pragma once  

#pragma region //MP3  

#include <vector> 
#include <mfapi.h>  
#include <mfidl.h>  
#include <mfreadwrite.h>  
#include <string>

struct SoundData {
    WAVEFORMATEX* pWaveFormat;
    std::vector<BYTE> mediaData;
};

#pragma endregion  


#include <xaudio2.h> // wavファイル用  
#include <stdint.h> // int32_t  

#include <wrl.h> // ComPtr(コムポインタ)  

class Sound {
public:

    /// @brief 初期化処理  
    /// @param xAudio2 XAudioエンジン  
    /// @param masterVoice  
    /// @return  
   void Initialize();

    SoundData SoundLoad(const std::wstring& path);

    void SoundPlay(const SoundData& soundData, const float& volume);
    /// @brief 音声データの解放関数  
    /// @param soundData 音声データ  
    void SoundUnload(SoundData* soundData);
    ~Sound();
private:

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr; // ComオブジェクトなのでComPtrで管理する。  
    IXAudio2MasteringVoice* masterVoice_ = { nullptr }; // ReleaseなしのためComPtrで管理することが出来ない。  
};