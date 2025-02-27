#include "JinglePlayer.h"

const uint16_t NOTIFICATION_WAV[] PROGMEM = {
    #include "jingles/NOTIFICATION_WAV.txt"
};

const uint16_t ERROR_WAV[] PROGMEM = {
    #include "jingles/error_wav.txt"
};

const uint16_t SUCCESS_WAV[] PROGMEM = {
    #include "jingles/success_wav.txt"
};

const uint16_t ALARM_WAV[] PROGMEM = {
    #include "jingles/alarm_wav.txt"
};

const uint16_t CLICK_WAV[] PROGMEM = {
    #include "jingles/CLICK_WAV.txt"
};

const uint16_t PING_WAV[] PROGMEM = {
    #include "jingles/PING_WAV.txt"
};

const uint16_t OPEN_WAV[] PROGMEM = {
    #include "jingles/OPEN_WAV.txt"
};

const uint16_t CLOSE_WAV[] PROGMEM = {
    #include "jingles/CLOSE_WAV.txt"
};


JinglePlayer * JinglePlayer::getJingle(Jingle jingle) {
    switch (jingle)
    {
    case SUCCESS:
        return new JinglePlayer(SUCCESS_WAV, sizeof(SUCCESS_WAV) / audio_b_size);
    case ERROR:
        return new JinglePlayer(ERROR_WAV, sizeof(ERROR_WAV) / audio_b_size);
    case ALARM:
        return new JinglePlayer(ALARM_WAV, sizeof(ALARM_WAV) / audio_b_size);
    case CLICK:
        return new JinglePlayer(CLICK_WAV, sizeof(CLICK_WAV) / audio_b_size);
    case NOTIFICATION:
        return new JinglePlayer(NOTIFICATION_WAV, sizeof(NOTIFICATION_WAV) / audio_b_size);
    case PING:
        return new JinglePlayer(PING_WAV, sizeof(PING_WAV) / audio_b_size);
    case OPEN:
        return new JinglePlayer(OPEN_WAV, sizeof(OPEN_WAV) / audio_b_size);
    case CLOSE:
        return new JinglePlayer(CLOSE_WAV, sizeof(CLOSE_WAV) / audio_b_size);
    default:
        break;
    }

    return NULL;
}

JinglePlayer::JinglePlayer(const uint16_t * jingle, int length) : jingle(jingle), length(length) {
    this->index = 0;
}

JinglePlayer::~JinglePlayer() {
    end();
}

bool JinglePlayer::begin() {
    if (!stream || !(*stream)) return false;
    
    if (_available) return true;

    // to enable preload
    _available = true;

    //preload buffer
    int cont = provide(_preload_blocks);

    Serial.print("preload blocks: ");
    Serial.println(cont);

    (*stream)->open();
    _available = (*stream)->available();
    return _available;
}

void JinglePlayer::end() {
    (*stream)->close();
}

bool JinglePlayer::available() {
    return _available;
}

void JinglePlayer::setStream(BufferedStream ** stream) {
    if (_available) end();
    this->stream = stream;
}

int JinglePlayer::provide(int n) {
    if (!_available) return 0;

    int blocks = (*stream)->get_contiguous_blocks();
    //Serial.print("contiguous: ");
    //Serial.println(blocks);
    int cont = min(length-index, min(blocks, n));

    if (!cont) {
        (*stream)->close();
        return 0;
    }

    memcpy((*stream)->buffer.getWritePointer(), &(jingle[index * (*stream)->buffer.getBlockSize() / sizeof(int16_t)]), cont * (*stream)->buffer.getBlockSize());

    index += cont;

    (*stream)->provide(cont);

    if (cont < n) {
        (*stream)->close();
    }

    return cont;
}

unsigned int JinglePlayer::get_sample_rate() {
    if (!_available) return -1;
    return 44100;
}

unsigned int JinglePlayer::get_size() {
    return length;
}

WAVConfigurationPacket JinglePlayer::get_config() {
    WAVConfigurationPacket wav_packet;

    /*wav_packet.state = 0;
    wav_packet.size = _name.length();

    memcpy(wav_packet.name, _name.c_str(), _name.length());*/

    return wav_packet;
}