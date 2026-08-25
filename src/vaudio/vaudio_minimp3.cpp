#include <SDL3/SDL_assert.h>

#include <interface/interface.h>
#include <vaudio/ivaudio.h>

#define MINIMP3_ONLY_MP3
#define MINIMP3_NONSTANDARD_BUT_LOGICAL
#define MINIMP3_NO_STDIO
#define MINIMP3_IMPLEMENTATION
#include <vaudio/minimp3/minimp3_ex.h>

class CMiniMP3 final : public IAudioStream {
public:
    CMiniMP3(IAudioStreamEvent *pEventHandler);

    ~CMiniMP3() override = default;

    int Decode(void *pBuffer, unsigned int bufferSize) override;

    int GetOutputBits() override { return 8 * sizeof(mp3d_sample_t); }
    int GetOutputRate() override { return m_Dec.info.hz; }
    int GetOutputChannels() override { return m_Dec.info.channels; }

    unsigned int GetPosition() override { return m_Dec.offset; }

    void SetPosition(unsigned int position) override;

private:
    mp3dec_ex_t m_Dec{};
    IAudioStreamEvent *m_pEventHandler;

    static constexpr int m_dataSize = MINIMP3_IO_SIZE;
    uint8_t m_pData[m_dataSize]{};
    unsigned int m_readed = 0;
};

CMiniMP3::CMiniMP3(IAudioStreamEvent *pEventHandler) {
    m_pEventHandler = pEventHandler;
    const int size = m_pEventHandler->StreamRequestData(m_pData, m_dataSize, 0);
    mp3dec_ex_open_buf(&m_Dec, m_pData, size, MP3D_SEEK_TO_BYTE);
    m_readed += size;
}

int CMiniMP3::Decode(void *pBuffer, unsigned int bufferSize) {
    SDL_assert(bufferSize <= m_dataSize);

    if (m_Dec.offset + bufferSize > m_dataSize) {
        unsigned int offset = m_Dec.offset - m_dataSize;
        // move forward
        m_readed -= offset;
        const int size = m_pEventHandler->StreamRequestData(m_pData, m_dataSize, static_cast<int>(m_readed));
        m_readed += m_dataSize;
        if (size != m_dataSize)
            memset(m_pData + size, 0, m_dataSize - size);

        mp3dec_ex_open_buf(&m_Dec, m_pData, size, MP3D_SEEK_TO_BYTE);
    }

    const size_t samples = mp3dec_ex_read(&m_Dec, static_cast<mp3d_sample_t *>(pBuffer),
                                          bufferSize / sizeof(mp3d_sample_t));
    const int bytes = static_cast<int>(samples * sizeof(mp3d_sample_t));
    return bytes;
}

void CMiniMP3::SetPosition(unsigned int position) {
    const int size = m_pEventHandler->StreamRequestData(m_pData, m_dataSize, static_cast<int>(position));
    if (size < m_dataSize)
        memset(m_pData + size, 0, m_dataSize - size);

    mp3dec_ex_open_buf(&m_Dec, m_pData, size, MP3D_SEEK_TO_BYTE);

    m_readed = position;
}

class CVAudio : public IVAudio {
public:
    ~CVAudio() override = default;

    IAudioStream *CreateMP3StreamDecoder(IAudioStreamEvent *pEventHandler) override {
        return new CMiniMP3(pEventHandler);
    }

    void DestroyMP3StreamDecoder(IAudioStream *pDecoder) override { delete static_cast<CMiniMP3 *>(pDecoder); }

    void *CreateMilesAudioEngine() override { return nullptr; }
    void DestroyMilesAudioEngine(void *) override { return; }
};

EXPOSE_INTERFACE(CVAudio, IVAudio, VAUDIO_INTERFACE_VERSION);
