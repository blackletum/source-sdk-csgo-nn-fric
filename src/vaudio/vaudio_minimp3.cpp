#include <interface/interface.h>
#include <vaudio/ivaudio.h>

#define MINIMP3_ONLY_MP3
#define MINIMP3_NONSTANDARD_BUT_LOGICAL
#define MINIMP3_IMPLEMENTATION
#include <vaudio/minimp3/minimp3.h>

class CMiniMP3 final : public IAudioStream {
 public:
  CMiniMP3(IAudioStreamEvent* pEventHandler);
  ~CMiniMP3() override = default;

  int Decode(void* pBuffer, unsigned int bufferSize) override;

  int GetOutputBits() override { return 8 * sizeof(mp3d_sample_t); }
  int GetOutputRate() override { return m_Info.hz; }
  int GetOutputChannels() override { return m_Info.channels; }

  unsigned int GetPosition() override {
    return m_nOffset + m_nDataOffset - m_nDataSize;
  }
  void SetPosition(unsigned int position) override;

 private:
  int SampleToByte(int sample) {
    return sample * sizeof(mp3d_sample_t) * GetOutputChannels();
  }

  mp3dec_t m_Dec{};
  mp3dec_frame_info_t m_Info;

  IAudioStreamEvent* m_pEventHandler;

  static constexpr int m_nMaxHalfDataSize = 8 * 1024;  // 8kb
  uint8_t m_pData[2][m_nMaxHalfDataSize];
  int m_nDataSize;

  unsigned int m_nOffset;
  int m_nDataOffset = 0;

  mp3d_sample_t m_pPCM[MINIMP3_MAX_SAMPLES_PER_FRAME];
};

CMiniMP3::CMiniMP3(IAudioStreamEvent* pEventHandler) {
  mp3dec_init(&m_Dec);
  m_pEventHandler = pEventHandler;
  m_nOffset = m_nDataSize =
      m_pEventHandler->StreamRequestData(m_pData[0], m_nMaxHalfDataSize * 2, 0);
  mp3dec_decode_frame(&m_Dec, m_pData[0], m_nDataSize, m_pPCM, &m_Info);
}

int CMiniMP3::Decode(void* pBuffer, unsigned int bufferSize) {
  int bytes = 0;

  while (bufferSize >
         bytes + MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(mp3d_sample_t)) {
    int cur_bytes;

    cur_bytes = SampleToByte(mp3dec_decode_frame(
        &m_Dec, m_pData[0] + m_nDataOffset, m_nDataSize - m_nDataOffset,
        static_cast<mp3d_sample_t*>(pBuffer) + bytes / sizeof(mp3d_sample_t),
        &m_Info));

    bytes += cur_bytes;
    m_nDataOffset += m_Info.frame_bytes;

    // update data if need
    if (m_nDataOffset > m_nMaxHalfDataSize &&
        m_nDataSize == m_nMaxHalfDataSize * 2) {
      m_nDataSize -= m_nMaxHalfDataSize;
      m_nDataOffset -= m_nMaxHalfDataSize;
      memcpy(m_pData[0], m_pData[1], m_nMaxHalfDataSize);
      const int nRequestedSize = m_pEventHandler->StreamRequestData(
          m_pData[1], m_nMaxHalfDataSize, m_nOffset);
      m_nDataSize += nRequestedSize;
      m_nOffset += nRequestedSize;
    }
  }

  return bytes;
}

void CMiniMP3::SetPosition(unsigned int position) {
  m_nDataSize = m_pEventHandler->StreamRequestData(
      m_pData, m_nMaxHalfDataSize * 2, position);
  m_nOffset = position + m_nDataSize;
  m_nDataOffset = 0;
}

class CVAudio : public IVAudio {
 public:
  ~CVAudio() override = default;

  IAudioStream* CreateMP3StreamDecoder(
      IAudioStreamEvent* pEventHandler) override {
    return new CMiniMP3(pEventHandler);
  }

  void DestroyMP3StreamDecoder(IAudioStream* pDecoder) override {
    delete static_cast<CMiniMP3*>(pDecoder);
  }

  void* CreateMilesAudioEngine() override { return nullptr; }
  void DestroyMilesAudioEngine(void*) override { return; }
};

EXPOSE_INTERFACE(CVAudio, IVAudio, VAUDIO_INTERFACE_VERSION);
