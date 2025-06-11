#include"../Header/Sound.h"
#include <fstream>
#include<assert.h>

#pragma comment(lib, "Mf.lib")  
#pragma comment(lib, "mfplat.lib")  
#pragma comment(lib, "MFreadwrite.lib")  
#pragma comment(lib, "mfuuid.lib")  

#pragma comment(lib, "xaudio2.lib") // xaudio2.libをリンクする。  

HRESULT Sound::Initialize() {
    HRESULT result;
    result = XAudio2Create(xAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(result));

    //マスターボイスの生成
    result = xAudio2_->CreateMasteringVoice(&masterVoice_);//masterVoiceはxAudio2の解放と同時に無効になるため自分でdeleteしない
    assert(SUCCEEDED(result));

    result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(result));

    return result;
};

SoundData Sound::SoundLoad(const std::wstring& path) {

    //ソースリーダーの作成
    IMFSourceReader* pMFSourceReader{ nullptr };
    MFCreateSourceReaderFromURL(path.c_str(), NULL, &pMFSourceReader);

    //メディアタイプの取得
    IMFMediaType* pMFMediaType{ nullptr };
    MFCreateMediaType(&pMFMediaType);
    pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType);

    pMFMediaType->Release();
    pMFMediaType = nullptr;
    pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMFMediaType);

    //オーディオデータ形式の作成　メディアタイプからWaveForMatexを生成
    WAVEFORMATEX* waveFormat{ nullptr };
    MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, nullptr);
    //データの読み込み
    std::vector<BYTE>mediaData;

    while (true) {
        IMFSample* pMFSample{ nullptr };
        DWORD dwStreamFlags{ 0 };
        pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            break;
        }

        IMFMediaBuffer* pMFMediaBuffer{ nullptr };
        pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

        BYTE* pBuffer{ nullptr };
        DWORD cbCurrentLength{ 0 };
        pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

        mediaData.resize(mediaData.size() + cbCurrentLength);
        memcpy(mediaData.data() + mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

        pMFMediaBuffer->Unlock();

        pMFMediaBuffer->Release();
        pMFSample->Release();

    }

    //読み込んだ音声ファイルデータをreturn
    SoundData soundDataMP3 = {};

    soundDataMP3.waveFormat = waveFormat;
    soundDataMP3.mediaData = mediaData;

    return soundDataMP3;
}


void Sound::SoundUnload(SoundData* soundData) {

    // メディアデータの解放
    soundData->mediaData.clear();
    soundData->mediaData.shrink_to_fit();
    // waveFormatの解放
    soundData->waveFormat = {};

};

void Sound::SoundPlay(const SoundData& soundData) {
    HRESULT result;

    IXAudio2SourceVoice* pSourceVoice{ nullptr };
    result = xAudio2_->CreateSourceVoice(&pSourceVoice, soundData.waveFormat);
    assert(SUCCEEDED(result));

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.mediaData.data();
    buf.AudioBytes = sizeof(BYTE) * static_cast<UINT32>(soundData.mediaData.size());
    buf.Flags = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    assert(SUCCEEDED(result));
    result = pSourceVoice->Start();//再生開始
    assert(SUCCEEDED(result));
};
