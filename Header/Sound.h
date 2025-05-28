#pragma once

#include <xaudio2.h>//wavファイル用
#pragma comment(lib,"xaudio2.lib")//xaudio2.libをリンクする。
//ComPtr(コムポインタ)
#include<wrl.h>
#include<stdint.h>

//チャンクヘッダ
struct ChunkHeader {
    char id[4];//チャンク毎のID
    int32_t size;//チャンクサイズ
};

//以下がWavファイルのデータ構造

//RIFFヘッダチャンク
struct RiffHeader {
    ChunkHeader chunk;//"RIFF"
    char type[4];//"WAVE"
};

//FMTチャンク
struct FormatChunk {
    ChunkHeader chunk;//"fmt"
    WAVEFORMATEX fmt;//波形フォーマット
};

//音声データ
struct SoundData {
    //波形フォーマット
    WAVEFORMATEX wfex;
    //バッファの先頭アドレス
    BYTE* pBuffer;
    //バッファサイズ
    unsigned int bufferSize;
};

class Sound {
public:

    /// @brief 初期化処理
    /// @param xAudio2 XAudioエンジン
    /// @param masterVoice 
    /// @return 
    HRESULT Initialize();

    /// @brief Wave音声読み込み関数
    /// @param filename ファイル名 
    /// @return 音声データ
    SoundData SoundLoadWave(const char* filename);

    /// @brief 音声データの解放関数
    /// @param soundData 音声データ
    void SoundUnload(SoundData* soundData);

    /// @brief Wave音声の再生
    /// @param ixAudio2 XAudioエンジン
    /// @param soundData 音声データ
    void SoundPlayWave(const SoundData& soundData);

    void Reset();

private:

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;//ComオブジェクトなのでComPtrで管理する。
    IXAudio2MasteringVoice* masterVoice_;//ReleaseなしのためComPtrで管理することが出来ない。
};