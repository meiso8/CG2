#include"../Header/Sound.h"
#include <fstream>
#include<assert.h>

SoundData Sound::SoundLoadWave(const char* filename) {
    //HRESULT result;
    //1.ファイルオープン

    //ファイル入力ストリームのインスタンス
    std::ifstream file;
    //.wavファイルをバイナリモードで開く
    file.open(filename, std::ios_base::binary);
    //ファイルオープン失敗を検出する
    assert(file.is_open());

    //2. .wavデータ読み込み

    //RIFFヘッダーの読み込み
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    //ファイルがRIFFかチェック
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
        assert(0);
    }
    //タイプがWAVEかチェック
    if (strncmp(riff.type, "WAVE", 4) != 0) {
        assert(0);
    }

    //Fotmatチャンクの読み込み
    FormatChunk format = {};
    //チャンクヘッダーの確認
    file.read((char*)&format, sizeof(ChunkHeader));
    if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
        assert(0);
    }

    //チャンク本体の読み込み
    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);

    //Dataチャンクの読み込み
    ChunkHeader data;
    file.read((char*)&data, sizeof(data));
    //JUNKチャンクを検出した場合 JUNKチャンクというものが間に挟まっている場合があるデータの開始位置を桐の良いところに配置するためのダミーデータ
    if (strncmp(data.id, "JUNK", 4) == 0) {
        //読み取り位置をJUNKチャンクの終わりまで進める　seekg()で読み取り位置を動かすことが出来る
        file.seekg(data.size, std::ios_base::cur);
        //再読み込み　本物のでーたを読み込み
        file.read((char*)&data, sizeof(data));
    }

    if (strncmp(data.id, "data", 4) != 0) {
        assert(0);
    }

    //Dataチャンクのデータ部(波形データ)の読み込み 動的に確保する
    char* pBuffer = new char[data.size];
    file.read(pBuffer, data.size);

    //3.ファイルクローズ

    //Waveファイルを閉じる
    file.close();

    //4.読み込んだ音声ファイルデータをreturn
    //returnするための音声データ
    SoundData soundData = {};

    soundData.wfex = format.fmt;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
    soundData.bufferSize = data.size;

    return soundData;
}

HRESULT Sound::Initialize(/*Microsoft::WRL::ComPtr<IXAudio2>&xAudio2,IXAudio2MasteringVoice* masterVoice*/) {
    HRESULT result;
    result = XAudio2Create(ixAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
    //マスターボイスの生成
    result = ixAudio2_->CreateMasteringVoice(&masterVoice_);//masterVoiceはxAudio2の解放と同時に無効になるため自分でdeleteしない
    return result;
}

void Sound::SoundUnload(SoundData* soundData) {

    //バッファのメモリを解放 配列deleteで波形データのバッファを解放する
    delete[] soundData->pBuffer;

    soundData->pBuffer = 0;
    soundData->bufferSize = 0;
    soundData->wfex = {};

}

void Sound::SoundPlayWave(/*IXAudio2* ixAudio2, */const SoundData& soundData) {
    HRESULT result;

    //波形フォーマットを元にSoundVoiceの生成
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = ixAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
    assert(SUCCEEDED(result));

    //再生する波形データの設定
    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.pBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    //波形データの再生
    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();//再生開始
}

Sound::~Sound() {
    ixAudio2_.Reset();
}