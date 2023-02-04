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
    BerrySound(VoiceParams &voiceParams, std::vector<MainParams> &mainParamList)
        : voiceParams(voiceParams), mainParamList(mainParamList) {}
    ~BerrySound(){};
    bool appliesToNote(int noteNumber) override { return true; };
    bool appliesToChannel(int) override { return true; };

private:
    VoiceParams &voiceParams;
    std::vector<MainParams> &mainParamList;
};

//==============================================================================
class CurrentPositionInfo {
public:
    CurrentPositionInfo() {}
    ~CurrentPositionInfo(){};
    std::optional<double> bpm{};
};

//==============================================================================
class MonoStack {
public:
    int latestNoteNumber = 0;
    int firstNoteNumber = 0;
    MonoStack(){};
    ~MonoStack(){};
    bool push(int noteNumber, float velocity) {
        if (noteNumber == 0 || velocity == 0.0f) {
            return false;
        }
        if (firstNoteNumber == 0) {
            firstNoteNumber = noteNumber;
        }
        remove(noteNumber);
        notes[noteNumber].velocity = velocity;
        notes[noteNumber].next = latestNoteNumber;
        latestNoteNumber = noteNumber;
        return true;
    }
    bool remove(int noteNumber) {
        if (latestNoteNumber == 0) {
            return false;
        }
        auto next = notes[noteNumber].next;
        notes[noteNumber].velocity = 0.0f;
        notes[noteNumber].next = 0;
        if (latestNoteNumber == noteNumber) {
            latestNoteNumber = next;
            if (latestNoteNumber == 0) {
                firstNoteNumber = 0;
            }
            return true;
        }
        auto currentNoteNumber = latestNoteNumber;
        while (notes[currentNoteNumber].next != 0) {
            if (notes[currentNoteNumber].next == noteNumber) {
                notes[currentNoteNumber].next = next;
                return true;
            }
            currentNoteNumber = notes[currentNoteNumber].next;
        }
        return false;
    }
    float getVelocity(int noteNumber) { return notes[noteNumber].velocity; }
    void reset() { std::fill_n(notes, 128, NoteInfo{}); }

private:
    struct NoteInfo {
        float velocity = 0.0f;
        int next = 0;
    };
    NoteInfo notes[128]{};
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
               std::vector<std::unique_ptr<juce::AudioBuffer<float>>> &buffers,
               GlobalParams &globalParams,
               VoiceParams &voiceParams,
               std::vector<MainParams> &mainParamList);
    ~BerryVoice();
    bool canPlaySound(juce::SynthesiserSound *sound) override;
    void startNote(int midiNoteNumber,
                   float velocity,
                   juce::SynthesiserSound *,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void glide(int midiNoteNumber, float velocity);
    void mute(double duration);
    virtual void pitchWheelMoved(int) override{};
    virtual void controllerMoved(int, int) override{};
    void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample, int numSamples) override;
    void applyParamsBeforeLoop(double sampleRate);
    bool step(double *out, double sampleRate, int numChannels);
    bool isDrumAtStart = false;
    int noteNumberAtStart = -1;

private:
    juce::PerformanceCounter perf;
    CurrentPositionInfo *currentPositionInfo;

    GlobalParams &globalParams;
    VoiceParams &voiceParams;
    std::vector<MainParams> &mainParamList;
    std::vector<std::unique_ptr<juce::AudioBuffer<float>>> &buffers;

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
        return mainParamList[isDrumAtStart ? noteNumberAtStart : 128];
    }
    std::unique_ptr<juce::AudioBuffer<float>> &getBuffer() {
        jassert(noteNumberAtStart >= 0);
        return buffers[isDrumAtStart ? noteNumberAtStart : 128];
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
    BerrySynthesiser(CurrentPositionInfo *currentPositionInfo,
                     MonoStack *monoStack,
                     std::vector<std::unique_ptr<juce::AudioBuffer<float>>> &buffers,
                     std::vector<std::optional<juce::AudioBuffer<float>>> &busBuffers,
                     AllParams &allParams)
        : currentPositionInfo(currentPositionInfo),
          monoStack(monoStack),
          buffers(buffers),
          busBuffers(busBuffers),
          allParams(allParams) {
        addSound(new BerrySound(allParams.voiceParams, allParams.mainParamList));
        for (auto i = 0; i < 129; i++) {
            stereoDelays.push_back(std::unique_ptr<StereoDelay>(nullptr));
        }
    }
    ~BerrySynthesiser() {}
    virtual void renderNextBlock(AudioBuffer<float> &outputAudio,
                                 const MidiBuffer &inputMidi,
                                 int startSample,
                                 int numSamples) {
        allParams.freeze();
        for (auto i = 0; i < 129; i++) {
            if (allParams.mainParamList[i].isEnabled()) {
                buffers[i]->setSize(2, startSample + numSamples, false, false, true);
                buffers[i]->clear();
            } else {
                buffers[i]->setSize(2, 0);
            }
        }
        juce::Synthesiser::renderNextBlock(outputAudio, inputMidi, startSample, numSamples);
    }
    virtual void handleMidiEvent(const juce::MidiMessage &m) override {
        const int channel = m.getChannel();
        if (m.isNoteOn()) {
            auto midiNoteNumber = m.getNoteNumber();
            auto velocity = m.getFloatVelocity();
            for (auto *sound : sounds) {
                if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(channel)) {
                    if (BerryVoice *voice = dynamic_cast<BerryVoice *>(voices[0])) {
                        bool playingNotesExist = monoStack->latestNoteNumber != 0;
                        monoStack->push(midiNoteNumber, velocity);
                        if (allParams.voiceParams.isMonoModeFreezed) {
                            jassert(getNumVoices() == 1);
                            if (voice->isPlayingChannel(channel) && playingNotesExist) {
                                voice->glide(midiNoteNumber, velocity);
                                return;
                            }
                        }
                    }
                    if (allParams.voiceParams.isDrumModeFreezed) {
                        auto &drumParams = allParams.mainParamList[midiNoteNumber].drumParams;
                        if (drumParams.noteToMuteEnabled) {
                            for (auto *voice_ : voices) {
                                if (BerryVoice *voice = dynamic_cast<BerryVoice *>(voice_)) {
                                    if (voice->isPlayingChannel(channel) &&
                                        voice->noteNumberAtStart == drumParams.noteToMute) {
                                        voice->mute(0.001);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (m.isNoteOff()) {
            auto midiNoteNumber = m.getNoteNumber();
            for (auto *sound : sounds) {
                if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(channel)) {
                    if (BerryVoice *voice = dynamic_cast<BerryVoice *>(voices[0])) {
                        int firstNoteNumber = monoStack->firstNoteNumber;
                        bool removed = monoStack->remove(midiNoteNumber);
                        if (!removed) {
                            return;
                        }
                        bool playingNotesExist = monoStack->latestNoteNumber != 0;
                        if (allParams.voiceParams.isMonoModeFreezed) {
                            jassert(getNumVoices() == 1);
                            if (voice->isPlayingChannel(channel) && playingNotesExist) {
                                auto velocity = monoStack->getVelocity(monoStack->latestNoteNumber);
                                jassert(velocity != 0);
                                voice->glide(monoStack->latestNoteNumber, velocity);
                                return;
                            }
                            noteOff(channel, firstNoteNumber, m.getFloatVelocity(), true);
                            return;
                        }
                    }
                }
            }
        } else if (m.isAllNotesOff()) {
            monoStack->reset();
        } else if (m.isAllSoundOff()) {
            monoStack->reset();
        }
        jassert(getNumVoices() > 0);
        Synthesiser::handleMidiEvent(m);
    }
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
    void renderVoices(juce::AudioBuffer<float> &_buffer, int startSample, int numSamples) override {
        juce::Synthesiser::renderVoices(_buffer, startSample, numSamples);

        for (auto n = 0; n < 129; n++) {
            if (!allParams.mainParamList[n].isEnabled()) {
                continue;
            }
            auto &buffer = buffers[n];
            auto &mainParams = allParams.mainParamList[n];
            auto &delayParams = mainParams.delayParams;
            auto &stereoDelay = stereoDelays[n];

            auto busIndex = allParams.voiceParams.isDrumModeFreezed ? mainParams.drumParams.busIndex : 0;
            auto &outBuffer = busBuffers[busIndex];
            if (delayParams.enabled) {
                if (!stereoDelay) {
                    stereoDelay.reset(new StereoDelay());
                }
                stereoDelay->setParams(getSampleRate(),
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
            } else {
                stereoDelay.reset();
            }
            auto *leftIn = buffer->getReadPointer(0, startSample);
            auto *rightIn = buffer->getReadPointer(1, startSample);

            auto delayEnabled = delayParams.enabled;
            auto expression = allParams.globalParams.expression;
            auto masterVolume = mainParams.masterParams.masterVolume * allParams.globalParams.midiVolume;
            for (int i = 0; i < numSamples; ++i) {
                double sample[2]{leftIn[i] * expression, rightIn[i] * expression};

                // Delay
                if (delayEnabled) {
                    stereoDelay->step(sample);
                }

                // Master Volume
                sample[0] *= masterVolume;
                sample[1] *= masterVolume;
                if (outBuffer) {
                    outBuffer->addSample(0, startSample + i, sample[0]);
                    outBuffer->addSample(1, startSample + i, sample[1]);
                }
            }
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
            default: {
                if (allParams.voiceParams.isDrumMode()) {
                    break;
                }
                auto &mainParams = allParams.mainParamList[128];
                // custom
                for (int i = 0; i < NUM_CONTROL; ++i) {
                    auto &params = allParams.controlItemParams[i];
                    if (number != params.number) {
                        continue;
                    }
                    switch (params.targetType) {
                        case CONTROL_TARGET_TYPE::OSC: {
                            int targetIndex = params.targetOsc;
                            auto targetParam = params.targetOscParam;
                            for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
                                if (targetIndex == oscIndex) {
                                    auto &params = mainParams.oscParams[oscIndex];
                                    switch (targetParam) {
                                        case CONTROL_TARGET_OSC_PARAM::Edge: {
                                            params.setEdgeFromControl(normalizedValue);
                                            params.freeze();
                                            break;
                                        }
                                        case CONTROL_TARGET_OSC_PARAM::Detune: {
                                            params.setDetuneFromControl(normalizedValue);
                                            params.freeze();
                                            break;
                                        }
                                        case CONTROL_TARGET_OSC_PARAM::Spread: {
                                            params.setSpreadFromControl(normalizedValue);
                                            params.freeze();
                                            break;
                                        }
                                        case CONTROL_TARGET_OSC_PARAM::Gain: {
                                            params.setGainFromControl(normalizedValue);
                                            params.freeze();
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        case CONTROL_TARGET_TYPE::Filter: {
                            int targetIndex = params.targetFilter;
                            auto targetParam = params.targetFilterParam;
                            for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
                                if (targetIndex == filterIndex) {
                                    auto &params = mainParams.filterParams[filterIndex];
                                    switch (targetParam) {
                                        case CONTROL_TARGET_FILTER_PARAM::Freq: {
                                            if (params.isFreqAbsoluteFreezed) {
                                                params.setHzFromControl(normalizedValue);
                                                params.freeze();
                                            } else {
                                                params.setSemitoneFromControl(normalizedValue);
                                                params.freeze();
                                            }
                                            break;
                                        }
                                        case CONTROL_TARGET_FILTER_PARAM::Q: {
                                            params.setQFromControl(normalizedValue);
                                            params.freeze();
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        case CONTROL_TARGET_TYPE::Master: {
                            switch (params.targetMiscParam) {
                                case CONTROL_TARGET_MISC_PARAM::PortamentoTime: {
                                    allParams.voiceParams.setPortamentoTimeFromControl(normalizedValue);
                                    allParams.voiceParams.freeze();
                                    break;
                                }
                                case CONTROL_TARGET_MISC_PARAM::DelayMix: {
                                    mainParams.delayParams.setMixFromControl(normalizedValue);
                                    mainParams.delayParams.freeze();
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    void pitchWheelMoved(int value) {
        value -= 8192;
        *allParams.globalParams.Pitch = value >= 0 ? value / 8191.0 : value / 8192.0;
        allParams.globalParams.freeze();
    }

private:
    CurrentPositionInfo *currentPositionInfo;

    MonoStack *monoStack;
    std::vector<std::unique_ptr<juce::AudioBuffer<float>>> &buffers;
    std::vector<std::optional<juce::AudioBuffer<float>>> &busBuffers;
    AllParams &allParams;

    std::vector<std::unique_ptr<StereoDelay>> stereoDelays{};
};
