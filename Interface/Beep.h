#ifndef RADIATIONINTERFACE_BEEP_H
#define RADIATIONINTERFACE_BEEP_H

#include <RtAudio.h>
#include <Tonic/Synth.h>
#include <condition_variable>
#include <thread>

class Beep {
    inline static const unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 512;

    RtAudio dac;
    RtAudio::StreamParameters rtParams;

    std::mutex audioPlaying;
    std::condition_variable audioPlay;

    std::optional<std::thread> thread;
public:
    inline static const unsigned int nChannels = 2;

    Beep();
    Beep(const Beep&) = delete;

    virtual ~Beep();

    void ominousBeep();

    void streamThread();
};

#endif //RADIATIONINTERFACE_BEEP_H
