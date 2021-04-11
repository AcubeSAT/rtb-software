#include "Beep.h"
#include "Tonic.h"
#include "main.h"
#include <plog/Log.h>
#include <RtAudio.h>

#include <chrono>
#include <thread>
#include <future>

using namespace Tonic;

static Tonic::Synth synth;
static int renderCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                          double streamTime, RtAudioStreamStatus status, void *userData ) {
    synth.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, Beep::nChannels);

    return 0;
};

Beep::Beep() {
    Tonic::setSampleRate(sampleRate);

    rtParams.deviceId = dac.getDefaultOutputDevice();
    rtParams.nChannels = nChannels;

    dac.openStream( &rtParams, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, renderCallback, nullptr, nullptr);

    thread.emplace(&Beep::streamThread, this);
}

void Beep::beep(BeepType type) {
    if (settings.volume <= minVolume + 0.1f) {
        return;
    }

    ControlMetro metro = ControlMetro().bpm(100);
    ControlGenerator freq = ControlRandom().trigger(metro).min(0).max(1);

    Generator tone = SquareWaveBL().freq(
            freq * 0.25 + 100
            + 200
    ); //* SineWave().freq(50);

    ADSR env = ADSR()
            .attack(0.01)
            .decay( 0.4 )
            .sustain(0)
            .release(0)
            .doesSustain(false)
            .trigger(1);


    LPF24 filter = LPF24().Q(2).cutoff( 1200 );
    Generator output = (( tone * env ) >> filter) * 0.5 * getVolume();

    synth.setOutputGen(output);

    audioPlay.notify_one();
}

Beep::~Beep() {
    dac.closeStream();
    audioPlay.notify_one();
    thread->join();
}

void Beep::streamThread() {
    try {
        while(dac.isStreamOpen()) {
            std::unique_lock<std::mutex> lock(audioPlaying);
            audioPlay.wait(lock);

            if (dac.isStreamOpen()) {
                dac.startStream();
                std::this_thread::sleep_for(500ms);
                dac.stopStream();
            }
        }
    } catch ( RtError& e ) {
        LOG_ERROR << e.getMessage();
    }
}

float Beep::getVolume() {
    double dB = settings.volume;
    return std::pow(10, dB / 20.0);
}

