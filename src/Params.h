#pragma once

#include <JuceHeader.h>

#include "Constants.h"
#include "DSP.h"

//==============================================================================
class SynthParametersBase {
public:
    virtual ~SynthParametersBase() {}
    virtual void addAllParameters(juce::AudioProcessor& processor) = 0;
    virtual void saveParameters(juce::XmlElement& xml) = 0;
    virtual void loadParameters(juce::XmlElement& xml) = 0;
};

//==============================================================================
class VoiceParams : public SynthParametersBase {
public:
    juce::AudioParameterChoice* Mode;
    juce::AudioParameterFloat* PortamentoTime;
    juce::AudioParameterInt* PitchBendRange;
    juce::AudioParameterChoice* TargetNoteKind;
    juce::AudioParameterChoice* TargetNoteOct;

    VoiceParams();
    VoiceParams(const VoiceParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    VOICE_MODE getMode() { return static_cast<VOICE_MODE>(Mode->getIndex()); }
    void setPortamentoTimeFromControl(double normalizedValue) {
        *PortamentoTime = PortamentoTime->range.convertFrom0to1(normalizedValue);
    }

    bool isMonoMode() { return getMode() == VOICE_MODE::Mono; }
    bool isDrumMode() { return getMode() == VOICE_MODE::Drum; }
    int getTargetNote() {
        return (TARGET_NOTE_OCT_VALUES[TargetNoteOct->getIndex()] + 2) * 12 + TargetNoteKind->getIndex();
    }

    bool isMonoModeFreezed;
    bool isDrumModeFreezed;
    float portamentoTime;
    int pitchBendRange;
    int targetNote;
    void freeze() {
        isMonoModeFreezed = isMonoMode();
        isDrumModeFreezed = isDrumMode();
        portamentoTime = PortamentoTime->get();
        pitchBendRange = PitchBendRange->get();
        targetNote = getTargetNote();
    }

private:
};

//==============================================================================
class GlobalParams : public SynthParametersBase {
public:
    //    TODO: portamento time?, pitch bend range?, velocity sense
    juce::AudioParameterFloat* Pitch;
    juce::AudioParameterFloat* Pan;
    juce::AudioParameterFloat* Expression;
    juce::AudioParameterFloat* MidiVolume;

    GlobalParams();
    GlobalParams(const GlobalParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    void setMidiVolumeFromControl(double normalizedValue) { *MidiVolume = normalizedValue; }
    void setPanFromControl(double normalizedValue) { *Pan = Pan->range.convertFrom0to1(normalizedValue); }
    void setExpressionFromControl(double normalizedValue) { *Expression = normalizedValue; }

    float pitch;
    float pan;
    float expression;
    float midiVolume;
    void freeze() {
        pitch = Pitch->get();
        pan = Pan->get();
        expression = Expression->get();
        midiVolume = MidiVolume->get();
    }

private:
};

//==============================================================================
class MasterParams : public SynthParametersBase {
public:
    //    TODO: portamento time?, pitch bend range?, velocity sense

    juce::AudioParameterFloat* Pan;
    juce::AudioParameterFloat* MasterVolume;

    MasterParams(std::string idPrefix, std::string namePrefix);
    MasterParams(const MasterParams&) = delete;
    MasterParams(MasterParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    float pan;
    float masterVolume;
    void freeze() {
        pan = Pan->get();
        masterVolume = MasterVolume->get();
    }

private:
};

//==============================================================================
class OscParams : public SynthParametersBase {
public:
    juce::AudioParameterBool* Enabled;
    juce::AudioParameterChoice* Waveform;
    juce::AudioParameterFloat* Edge;
    juce::AudioParameterInt* Octave;
    juce::AudioParameterInt* Coarse;
    juce::AudioParameterInt* Unison;
    juce::AudioParameterFloat* Detune;
    juce::AudioParameterFloat* Spread;
    juce::AudioParameterFloat* Gain;
    juce::AudioParameterChoice* Envelope;

    OscParams(std::string idPrefix, std::string namePrefix, int index);
    OscParams(const OscParams&) = delete;
    OscParams(OscParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    void setEdgeFromControl(double normalizedValue) { *Edge = normalizedValue; }
    void setDetuneFromControl(double normalizedValue) { *Detune = normalizedValue; }
    void setSpreadFromControl(double normalizedValue) { *Spread = normalizedValue; }
    void setGainFromControl(double normalizedValue) { *Gain = Gain->range.convertFrom0to1(normalizedValue); }

    WAVEFORM getWaveForm() { return OSC_WAVEFORM_VALUES[Waveform->getIndex()]; }
    bool hasEdge() { return false; }
    bool isNoise() {
        switch (getWaveForm()) {
            case WAVEFORM::White:
            case WAVEFORM::Pink:
                return true;
            default:
                return false;
        }
    }

    bool enabled;
    WAVEFORM waveform;
    float edge;
    int octave;
    int coarse;
    int unison;
    float detune;
    float spread;
    float gain;
    int envelope;
    void freeze() {
        enabled = Enabled->get();
        waveform = getWaveForm();
        edge = Edge->get();
        octave = Octave->get();
        coarse = Coarse->get();
        unison = Unison->get();
        detune = Detune->get();
        spread = Spread->get();
        gain = Gain->get();
        envelope = Envelope->getIndex();
    }

private:
    OscParams(){};
};

//==============================================================================
class EnvelopeParams : public SynthParametersBase {
public:
    juce::AudioParameterFloat* AttackCurve;
    juce::AudioParameterFloat* Attack;
    juce::AudioParameterFloat* Decay;
    juce::AudioParameterFloat* Sustain;
    juce::AudioParameterFloat* Release;

    EnvelopeParams(std::string idPrefix, std::string namePrefix, int index);
    EnvelopeParams(const EnvelopeParams&) = delete;
    EnvelopeParams(EnvelopeParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    float attackCurve;
    float attack;
    float decay;
    float sustain;
    float release;
    void freeze() {
        attackCurve = AttackCurve->get();
        attack = Attack->get();
        decay = Decay->get();
        sustain = Sustain->get();
        release = Release->get();
    }

private:
    EnvelopeParams(){};
};

//==============================================================================
class FilterParams : public SynthParametersBase {
public:
    juce::AudioParameterBool* Enabled;
    juce::AudioParameterChoice* Target;
    juce::AudioParameterChoice* Type;
    juce::AudioParameterChoice* FreqType;
    juce::AudioParameterFloat* Hz;
    juce::AudioParameterInt* Semitone;
    juce::AudioParameterFloat* Q;
    juce::AudioParameterFloat* Gain;

    FilterParams(std::string idPrefix, std::string namePrefix, int index);
    FilterParams(const FilterParams&) = delete;
    FilterParams(FilterParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    FILTER_TYPE getType() { return static_cast<FILTER_TYPE>(Type->getIndex()); }
    FILTER_FREQ_TYPE getFreqType() { return static_cast<FILTER_FREQ_TYPE>(FreqType->getIndex()); }

    bool hasGain() {
        switch (getType()) {
            case FILTER_TYPE::Peaking:
            case FILTER_TYPE::LowShelf:
            case FILTER_TYPE::HighShelf:
                return true;
            default:
                return false;
        }
    }
    bool isFreqAbsolute() { return getFreqType() == FILTER_FREQ_TYPE::Absolute; }

    void setHzFromControl(double normalizedValue) { *Hz = Hz->range.convertFrom0to1(normalizedValue); }
    void setSemitoneFromControl(double normalizedValue) {
        *Semitone = Semitone->getNormalisableRange().convertFrom0to1(normalizedValue);
    }
    void setQFromControl(double normalizedValue) { *Q = Q->range.convertFrom0to1(normalizedValue); }

    bool enabled;
    int target;
    FILTER_TYPE type;
    bool isFreqAbsoluteFreezed;
    float hz;
    int semitone;
    float q;
    float gain;
    void freeze() {
        enabled = Enabled->get();
        target = Target->getIndex();
        type = getType();
        isFreqAbsoluteFreezed = isFreqAbsolute();
        hz = Hz->get();
        semitone = Semitone->get();
        q = Q->get();
        gain = Gain->get();
    }

private:
    FilterParams(){};
};

//==============================================================================
class ModEnvParams : public SynthParametersBase {
public:
    juce::AudioParameterBool* Enabled;
    juce::AudioParameterChoice* TargetType;
    juce::AudioParameterChoice* TargetOsc;
    juce::AudioParameterChoice* TargetFilter;
    juce::AudioParameterChoice* TargetOscParam;
    juce::AudioParameterChoice* TargetFilterParam;
    juce::AudioParameterChoice* Fade;
    juce::AudioParameterFloat* PeakFreq;
    juce::AudioParameterFloat* Wait;
    juce::AudioParameterFloat* Attack;
    juce::AudioParameterFloat* Decay;

    ModEnvParams(std::string idPrefix, std::string namePrefix, int index);
    ModEnvParams(const ModEnvParams&) = delete;
    ModEnvParams(ModEnvParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    MODENV_TARGET_TYPE getTargetType() { return static_cast<MODENV_TARGET_TYPE>(TargetType->getIndex()); }
    MODENV_TARGET_OSC_PARAM getTargetOscParam() {
        return static_cast<MODENV_TARGET_OSC_PARAM>(TargetOscParam->getIndex());
    }
    MODENV_TARGET_FILTER_PARAM getTargetFilterParam() {
        return static_cast<MODENV_TARGET_FILTER_PARAM>(TargetFilterParam->getIndex());
    }
    bool isTargetFreq() {
        auto t = getTargetType();
        return (t == MODENV_TARGET_TYPE::OSC && getTargetOscParam() == MODENV_TARGET_OSC_PARAM::Freq) ||
               (t == MODENV_TARGET_TYPE::Filter && getTargetFilterParam() == MODENV_TARGET_FILTER_PARAM::Freq);
    }
    bool shouldUseHold() { return !isTargetFreq() && isFadeIn(); }
    bool isFadeIn() { return static_cast<MODENV_FADE>(Fade->getIndex()) == MODENV_FADE::In; }

    bool enabled;
    MODENV_TARGET_TYPE targetType;
    int targetOsc;
    int targetFilter;
    MODENV_TARGET_OSC_PARAM targetOscParam;
    MODENV_TARGET_FILTER_PARAM targetFilterParam;
    bool fadeIn;
    float peakFreq;
    float wait;
    float attack;
    float decay;
    void freeze() {
        enabled = Enabled->get();
        targetType = getTargetType();
        targetOsc = TargetOsc->getIndex();
        targetFilter = TargetFilter->getIndex();
        targetOscParam = getTargetOscParam();
        targetFilterParam = getTargetFilterParam();
        fadeIn = isFadeIn();
        peakFreq = PeakFreq->get();
        wait = Wait->get();
        attack = Attack->get();
        decay = Decay->get();
    }

private:
    ModEnvParams(){};
};

//==============================================================================
class DelayParams : public SynthParametersBase {
public:
    juce::AudioParameterBool* Enabled;
    juce::AudioParameterChoice* Type;
    juce::AudioParameterBool* Sync;
    juce::AudioParameterFloat* TimeL;
    juce::AudioParameterFloat* TimeR;
    juce::AudioParameterChoice* TimeSyncL;
    juce::AudioParameterChoice* TimeSyncR;
    juce::AudioParameterFloat* LowFreq;
    juce::AudioParameterFloat* HighFreq;
    juce::AudioParameterFloat* Feedback;
    juce::AudioParameterFloat* Mix;
    DelayParams(std::string idPrefix, std::string namePrefix);
    DelayParams(const DelayParams&) = delete;
    DelayParams(DelayParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    void setMixFromControl(double normalizedValue) { *Mix = normalizedValue; }

    DELAY_TYPE getType() { return static_cast<DELAY_TYPE>(Type->getIndex()); }
    double getTimeSyncL() { return DELAY_TIME_SYNC_VALUES[TimeSyncL->getIndex()]; }
    double getTimeSyncR() { return DELAY_TIME_SYNC_VALUES[TimeSyncR->getIndex()]; }

    bool enabled;
    DELAY_TYPE type;
    bool sync;
    float timeL;
    float timeR;
    double timeSyncL;
    double timeSyncR;
    float lowFreq;
    float highFreq;
    float feedback;
    float mix;
    void freeze() {
        enabled = Enabled->get();
        type = getType();
        sync = Sync->get();
        timeL = TimeL->get();
        timeR = TimeR->get();
        timeSyncL = getTimeSyncL();
        timeSyncR = getTimeSyncR();
        lowFreq = LowFreq->get();
        highFreq = HighFreq->get();
        feedback = Feedback->get();
        mix = Mix->get();
    }

private:
};

//==============================================================================
class DrumParams : public SynthParametersBase {
public:
    juce::AudioParameterInt* NoteToPlay;
    juce::AudioParameterBool* NoteToMuteEnabled;
    juce::AudioParameterChoice* NoteToMuteKind;
    juce::AudioParameterChoice* NoteToMuteOct;
    juce::AudioParameterChoice* Bus;

    DrumParams(std::string idPrefix, std::string namePrefix);
    DrumParams(const DrumParams&) = delete;
    DrumParams(DrumParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    int noteToPlay;
    bool noteToMuteEnabled;
    int noteToMute;
    int busIndex;
    int getNoteToMute() {
        return (TARGET_NOTE_OCT_VALUES[NoteToMuteOct->getIndex()] + 2) * 12 + NoteToMuteKind->getIndex();
    }
    void freeze() {
        noteToPlay = NoteToPlay->get();
        noteToMuteEnabled = NoteToMuteEnabled->get();
        noteToMute = getNoteToMute();
        busIndex = Bus->getIndex();
    }

private:
};

class MainParams : public SynthParametersBase {
public:
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;
    MainParams(int groupIndex);
    MainParams(const MainParams&) = delete;
    MainParams(MainParams&&) noexcept = default;

    std::array<OscParams, NUM_OSC> oscParams;
    std::array<EnvelopeParams, NUM_ENVELOPE> envelopeParams;
    std::array<FilterParams, NUM_FILTER> filterParams;
    std::array<ModEnvParams, NUM_MODENV> modEnvParams;
    DelayParams delayParams;
    DrumParams drumParams;
    MasterParams masterParams;
    bool isEnabled() {
        for (int i = 0; i < NUM_OSC; ++i) {
            if (oscParams[i].Enabled->get()) {
                return true;
            }
        }
        return false;
    }
    void freeze() {
        for (int i = 0; i < NUM_OSC; ++i) {
            oscParams[i].freeze();
        }
        for (int i = 0; i < NUM_ENVELOPE; ++i) {
            envelopeParams[i].freeze();
        }
        for (int i = 0; i < NUM_FILTER; ++i) {
            filterParams[i].freeze();
        }
        for (int i = 0; i < NUM_MODENV; ++i) {
            modEnvParams[i].freeze();
        }
        delayParams.freeze();
        drumParams.freeze();
        masterParams.freeze();
    }

private:
    static std::string idPrefix(int groupIndex) {
        return groupIndex == 128 ? "" : "G" + std::to_string(groupIndex) + "_";
    }
    static std::string namePrefix(int groupIndex) {
        return groupIndex == 128 ? "" : "G" + std::to_string(groupIndex) + " ";
    }
};

//==============================================================================
class ControlItemParams : public SynthParametersBase {
public:
    juce::AudioParameterChoice* Number;
    juce::AudioParameterChoice* TargetType;
    juce::AudioParameterChoice* TargetOsc;
    juce::AudioParameterChoice* TargetFilter;
    juce::AudioParameterChoice* TargetOscParam;
    juce::AudioParameterChoice* TargetFilterParam;
    juce::AudioParameterChoice* TargetMiscParam;
    ControlItemParams(int index);
    ControlItemParams(const ControlItemParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    int getCcNumber() { return CONTROL_NUMBER_VALUES[Number->getIndex()]; }
    CONTROL_TARGET_TYPE getTargetType() { return static_cast<CONTROL_TARGET_TYPE>(TargetType->getIndex()); }
    CONTROL_TARGET_OSC_PARAM getTargetOscParam() {
        return static_cast<CONTROL_TARGET_OSC_PARAM>(TargetOscParam->getIndex());
    }
    CONTROL_TARGET_FILTER_PARAM getTargetFilterParam() {
        return static_cast<CONTROL_TARGET_FILTER_PARAM>(TargetFilterParam->getIndex());
    }
    CONTROL_TARGET_MISC_PARAM getTargetMiscParam() {
        return static_cast<CONTROL_TARGET_MISC_PARAM>(TargetMiscParam->getIndex());
    }
    bool isControlling(CONTROL_TARGET_OSC_PARAM param, int index) {
        return getCcNumber() >= 0 && getTargetType() == CONTROL_TARGET_TYPE::OSC && TargetOsc->getIndex() == index &&
               getTargetOscParam() == param;
    }
    bool isControlling(CONTROL_TARGET_FILTER_PARAM param, int index) {
        return getCcNumber() >= 0 && getTargetType() == CONTROL_TARGET_TYPE::Filter &&
               TargetFilter->getIndex() == index && getTargetFilterParam() == param;
    }
    bool isControlling(CONTROL_TARGET_MISC_PARAM param) {
        return getCcNumber() >= 0 && getTargetType() == CONTROL_TARGET_TYPE::Master && getTargetMiscParam() == param;
    }

    int number;
    CONTROL_TARGET_TYPE targetType;
    int targetOsc;
    int targetFilter;
    CONTROL_TARGET_OSC_PARAM targetOscParam;
    CONTROL_TARGET_FILTER_PARAM targetFilterParam;
    CONTROL_TARGET_MISC_PARAM targetMiscParam;
    void freeze() {
        number = getCcNumber();
        targetType = getTargetType();
        targetOsc = TargetOsc->getIndex();
        targetFilter = TargetFilter->getIndex();
        targetOscParam = getTargetOscParam();
        targetFilterParam = getTargetFilterParam();
        targetMiscParam = getTargetMiscParam();
    }

private:
    ControlItemParams(){};
};

//==============================================================================
class AllParams : public SynthParametersBase {
public:
    GlobalParams globalParams;
    VoiceParams voiceParams;
    std::vector<MainParams> mainParamList{};
    std::array<ControlItemParams, NUM_CONTROL> controlItemParams;

    AllParams();
    AllParams(const AllParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    void saveParametersToClipboard(juce::XmlElement& xml);
    void loadParametersFromClipboard(juce::XmlElement& xml);
    void freeze() {
        globalParams.freeze();
        voiceParams.freeze();
        auto& mainParams = mainParamList[voiceParams.isDrumModeFreezed ? voiceParams.targetNote : 128];
        for (auto& mainParams : mainParamList) {
            if (mainParams.isEnabled()) {
                mainParams.freeze();
            }
        }
        for (int i = 0; i < NUM_CONTROL; ++i) {
            controlItemParams[i].freeze();
        }
    }
    MainParams& getCurrentMainParams() {
        return mainParamList[voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128];
    }

private:
};