#include "Beep.h"
#include "Tonic.h"
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

    dac.openStream( &rtParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, renderCallback, NULL, NULL );

    thread.emplace(&Beep::streamThread, this);
}

void Beep::ominousBeep() {
//    TriangleWave tone1 = TriangleWave();
//    SineWave tone2 = SineWave();
//    SineWave vibrato = SineWave().freq(10);
//    SineWave tremolo = SineWave().freq(1);
//
//    //that you can combine using intuitive operators
//    Generator combinedSignal = (tone1 + tone2) * tremolo;
//
//    //and plug in to one another
//    float baseFreq = 200;
//    tone1.freq(baseFreq + vibrato * 10);
//    tone2.freq(baseFreq * 2 + vibrato * 20);

    ControlMetro metro = ControlMetro().bpm(100);
    ControlGenerator freq = ControlRandom().trigger(metro).min(0).max(1);
//    ControlGenerator step = ControlStepper().end(NUM_STEPS).trigger(metro);

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


    LPF24 filter = LPF24().Q(2).cutoff( 700 );
    Generator output = (( tone * env ) >> filter) * 0.3;

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


