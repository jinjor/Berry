#pragma once

#include <JuceHeader.h>

#include "Constants.h"
#include "DSP.h"
#include "Params.h"

namespace {
const double A = 1.0 / 12.0;
const double X = std::pow(2.0, 1.0 / 12.0);
const double Y = 440.0 / std::pow(X, 69);
const int CONTROL_INTERVAL = 16;
const double CONTROL_RATE = 1.0 / CONTROL_INTERVAL;
}  // namespace

//==============================================================================
class SparseLog {
public:
    SparseLog(int count) : count(count) {}
    ~SparseLog() {}
    void log(std::string &key, std::string &message) {
        if (map.find(key) == map.end()) {
            map[key] = 0;
        }
        if (map[key] == count) {
            DBG(key << ": " << message);
            map[key] = 0;
        } else {
            map[key]++;
        }
    }
    void log(std::string &key, double message) {
        auto s = std::to_string(message);
        log(key, s);
    }

private:
    int count;
    std::map<std::string, int> map;
};

//==============================================================================
class BerrySound : public juce::SynthesiserSound {
public:
    BerrySound(VoiceParams &voiceParams, MainParams &mainParams) : voiceParams(voiceParams), mainParams(mainParams) {}
    ~BerrySound(){};
    bool appliesToNote(int noteNumber) override { return true; };
    bool appliesToChannel(int) override { return true; };

private:
    VoiceParams &voiceParams;
    MainParams &mainParams;
};

//==============================================================================
class CurrentPositionInfo {
public:
    CurrentPositionInfo() {}
    ~CurrentPositionInfo(){};
    std::optional<double> bpm{};
};

//==============================================================================
struct Modifiers {
    double normalizedAngleShift[NUM_OSC]{0.0, 0.0, 0.0};
    double octShift[NUM_OSC]{0.0, 0.0, 0.0};
    double edgeRatio[NUM_OSC]{1.0, 1.0, 1.0};
    double panMod[NUM_OSC]{0.0, 0.0, 0.0};
    double detuneRatio[NUM_OSC]{1.0, 1.0, 1.0};
    double spreadRatio[NUM_OSC]{1.0, 1.0, 1.0};
    double gain[NUM_OSC]{1.0, 1.0, 1.0};
    double filterOctShift[NUM_FILTER]{0.0, 0.0};
    double filterQExp[NUM_FILTER]{1.0, 1.0};
};

//==============================================================================
class BerryVoice : public juce::SynthesiserVoice {
public:
    BerryVoice(CurrentPositionInfo *currentPositionInfo,
               juce::AudioBuffer<float> &buffer,
               GlobalParams &globalParams,
               VoiceParams &voiceParams,
               MainParams &mainParamList);
    ~BerryVoice();
    bool canPlaySound(juce::SynthesiserSound *sound) override;
    void startNote(int midiNoteNumber,
                   float velocity,
                   juce::SynthesiserSound *,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void mute(double duration);
    virtual void pitchWheelMoved(int) override{};
    virtual void controllerMoved(int, int) override{};
    void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample, int numSamples) override;
    void applyParamsBeforeLoop(double sampleRate);
    bool step(double *out, double sampleRate, int numChannels);
    int noteNumberAtStart = -1;

private:
    juce::PerformanceCounter perf;
    CurrentPositionInfo *currentPositionInfo;

    GlobalParams &globalParams;
    VoiceParams &voiceParams;
    MainParams &mainParams;
    juce::AudioBuffer<float> &buffer;

    MultiOsc oscs[NUM_OSC];
    Adsr adsr[NUM_ENVELOPE];
    Filter filters[NUM_FILTER];
    Adsr modEnvs[NUM_MODENV];

    TransitiveValue smoothNote;
    TransitiveValue smoothVelocity;
    bool stolen = false;
    int stepCounter = 0;

    Modifiers controlModifiers = Modifiers{};
    SparseLog sparseLog = SparseLog(10000);
    MainParams &getMainParams() {
        jassert(noteNumberAtStart >= 0);
        return mainParams;
    }
    juce::AudioBuffer<float> &getBuffer() {
        jassert(noteNumberAtStart >= 0);
        return buffer;
    }
    double getMidiNoteInHertzDouble(double noteNumber) {
        return 440.0 * std::pow(2.0, (noteNumber - 69) * A);
        //        return Y * std::pow(X, noteNumber);// こっちの方がパフォーマンス悪かった
    }
    double shiftHertsByNotes(double herts, double notes) { return herts * std::pow(2.0, notes * A); }
    void updateModifiersByModEnv(Modifiers &modifiers, double sampleRate);
};

//==============================================================================
class BerrySynthesiser : public juce::Synthesiser {
public:
    BerrySynthesiser(CurrentPositionInfo *currentPositionInfo, juce::AudioBuffer<float> &buffer, AllParams &allParams)
        : currentPositionInfo(currentPositionInfo), buffer(buffer), allParams(allParams) {
        addSound(new BerrySound(allParams.voiceParams, allParams.mainParams));
    }
    ~BerrySynthesiser() {}
    virtual void renderNextBlock(AudioBuffer<float> &outputAudio,
                                 const MidiBuffer &inputMidi,
                                 int startSample,
                                 int numSamples) {
        allParams.freeze();
        buffer.setSize(2, startSample + numSamples, false, false, true);
        buffer.clear();

        juce::Synthesiser::renderNextBlock(outputAudio, inputMidi, startSample, numSamples);
    }
    virtual void handleMidiEvent(const juce::MidiMessage &m) override { Synthesiser::handleMidiEvent(m); }
    void handleController(const int midiChannel, const int controllerNumber, const int controllerValue) override {
        DBG("handleController: " << midiChannel << ", " << controllerNumber << ", " << controllerValue);
        juce::Synthesiser::handleController(midiChannel, controllerNumber, controllerValue);
        if (getSound(0)->appliesToChannel(midiChannel)) {
            controllerMoved(controllerNumber, controllerValue);
        }
    }
    void handlePitchWheel(const int midiChannel, const int wheelValue) override {
        DBG("handlePitchWheel: " << midiChannel << ", " << wheelValue);
        juce::Synthesiser::handlePitchWheel(midiChannel, wheelValue);
        if (getSound(0)->appliesToChannel(midiChannel)) {
            pitchWheelMoved(wheelValue);
        }
    }
    void renderVoices(juce::AudioBuffer<float> &outBuffer, int startSample, int numSamples) override {
        juce::Synthesiser::renderVoices(outBuffer, startSample, numSamples);

        auto &mainParams = allParams.mainParams;
        auto &delayParams = mainParams.delayParams;

        auto busIndex = 0;
        if (delayParams.enabled) {
            stereoDelay.setParams(getSampleRate(),
                                  currentPositionInfo->bpm,
                                  delayParams.type,
                                  delayParams.sync,
                                  delayParams.timeL,
                                  delayParams.timeR,
                                  delayParams.timeSyncL,
                                  delayParams.timeSyncR,
                                  delayParams.lowFreq,
                                  delayParams.highFreq,
                                  delayParams.feedback,
                                  delayParams.mix);
        }
        auto *leftIn = buffer.getReadPointer(0, startSample);
        auto *rightIn = buffer.getReadPointer(1, startSample);

        auto delayEnabled = delayParams.enabled;
        auto expression = allParams.globalParams.expression;
        auto masterVolume = mainParams.masterParams.masterVolume * allParams.globalParams.midiVolume;
        for (int i = 0; i < numSamples; ++i) {
            double sample[2]{leftIn[i] * expression, rightIn[i] * expression};

            // Delay
            if (delayEnabled) {
                stereoDelay.step(sample);
            }

            // Master Volume
            sample[0] *= masterVolume;
            sample[1] *= masterVolume;
            outBuffer.addSample(0, startSample + i, sample[0]);
            outBuffer.addSample(1, startSample + i, sample[1]);
        }
    }
    void controllerMoved(int number, int value) {
        auto normalizedValue = value / 127.0;

        // predefined
        switch (number) {
            case 7:
                allParams.globalParams.setMidiVolumeFromControl(normalizedValue);
                allParams.globalParams.freeze();
                break;
            case 10:
                allParams.globalParams.setPanFromControl(normalizedValue);
                allParams.globalParams.freeze();
                break;
            case 11:
                allParams.globalParams.setExpressionFromControl(normalizedValue);
                allParams.globalParams.freeze();
                break;
        }
    }
    void pitchWheelMoved(int value) {
        value -= 8192;
        *allParams.globalParams.Pitch = value >= 0 ? value / 8191.0 : value / 8192.0;
        allParams.globalParams.freeze();
    }

private:
    CurrentPositionInfo *currentPositionInfo;
    juce::AudioBuffer<float> &buffer;
    AllParams &allParams;

    StereoDelay stereoDelay{};
};
