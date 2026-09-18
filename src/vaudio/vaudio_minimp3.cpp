#include <interface/interface.h>
#include <vaudio/ivaudio.h>

#define MINIMP3_ONLY_MP3
#define MINIMP3_NONSTANDARD_BUT_LOGICAL
#define MINIMP3_NO_STDIO
#define MINIMP3_IMPLEMENTATION
#include <vaudio/minimp3/minimp3_ex.h>

class CMiniMP3 final : public IAudioStream {
 public:
  CMiniMP3(IAudioStreamEvent* pEventHandler);

  ~CMiniMP3() override = default;

  int Decode(void* pBuffer, unsigned int bufferSize) override;

  int GetOutputBits() override { return 8 * sizeof(mp3d_sample_t); }
  int GetOutputRate() override { return m_Dec.info.hz; }
  int GetOutputChannels() override { return m_Dec.info.channels; }

  unsigned int GetPosition() override { return m_Dec.offset; }

  void SetPosition(unsigned int position) override;

 private:
  int SampleToByte(int sample) { return sample * sizeof(mp3d_sample_t); }
  int ByteToSample(int byte) { return byte / sizeof(mp3d_sample_t); }

  mp3dec_ex_t m_Dec{};
  IAudioStreamEvent* m_pEventHandler;

  static constexpr int m_dataSize = MINIMP3_IO_SIZE;
  uint8_t m_pData[m_dataSize];
  unsigned int m_offset = 0;
};

CMiniMP3::CMiniMP3(IAudioStreamEvent* pEventHandler) {
  m_pEventHandler = pEventHandler;
  const int size = m_pEventHandler->StreamRequestData(m_pData, m_dataSize, 0);
  m_offset = size;
  mp3dec_ex_open_buf(&m_Dec, m_pData, size, MP3D_SEEK_TO_BYTE);
}

int CMiniMP3::Decode(void* pBuffer, unsigned int bufferSize) {
  int samples = mp3dec_ex_read(&m_Dec, static_cast<mp3d_sample_t*>(pBuffer),
                               ByteToSample(bufferSize));

  while (SampleToByte(samples) < bufferSize) {
    const int size =
        m_pEventHandler->StreamRequestData(m_pData, m_dataSize, m_offset);
    m_offset += size;
    mp3dec_ex_open_buf(&m_Dec, m_pData, size, MP3D_SEEK_TO_BYTE);
    if (size == 0) return SampleToByte(samples);
    samples +=
        mp3dec_ex_read(&m_Dec, static_cast<mp3d_sample_t*>(pBuffer) + samples,
                       ByteToSample(bufferSize - SampleToByte(samples)));
  }

  return SampleToByte(samples);
}

void CMiniMP3::SetPosition(unsigned int position) {
  if (m_offset > position && m_offset - m_Dec.file.size < position) {
    mp3dec_ex_seek(&m_Dec, position - (m_offset - m_Dec.file.size));
  } else {
    const int size =
        m_pEventHandler->StreamRequestData(m_pData, m_dataSize, position);
    mp3dec_ex_open_buf(&m_Dec, m_pData, size, MP3D_SEEK_TO_BYTE);

    m_offset = position + size;
  }
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
