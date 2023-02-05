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
    juce::AudioParameterInt* PitchBendRange;
    juce::AudioParameterChoice* TargetNoteKind;
    juce::AudioParameterChoice* TargetNoteOct;

    VoiceParams();
    VoiceParams(const VoiceParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    int getTargetNote() {
        return (TARGET_NOTE_OCT_VALUES[TargetNoteOct->getIndex()] + 2) * 12 + TargetNoteKind->getIndex();
    }

    int pitchBendRange;
    int targetNote;
    void freeze() {
        pitchBendRange = PitchBendRange->get();
        targetNote = getTargetNote();
    }

private:
};

//==============================================================================
class GlobalParams : public SynthParametersBase {
public:
    //    TODO: velocity sense
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
    //    TODO: velocity sense
    juce::AudioParameterFloat* Pan;
    juce::AudioParameterFloat* MasterVolume;

    MasterParams();
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
    juce::AudioParameterFloat* Gain;
    juce::AudioParameterChoice* Envelope;

    OscParams(int index);
    OscParams(const OscParams&) = delete;
    OscParams(OscParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    bool enabled;
    float gain;
    int envelope;
    void freeze() {
        enabled = Enabled->get();
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

    EnvelopeParams(int index);
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

    FilterParams(int index);
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

    ModEnvParams(int index);
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
    DelayParams();
    DelayParams(const DelayParams&) = delete;
    DelayParams(DelayParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

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

class MainParams : public SynthParametersBase {
public:
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;
    MainParams();
    MainParams(const MainParams&) = delete;
    MainParams(MainParams&&) noexcept = default;

    std::array<OscParams, NUM_OSC> oscParams;
    std::array<EnvelopeParams, NUM_ENVELOPE> envelopeParams;
    std::array<FilterParams, NUM_FILTER> filterParams;
    std::array<ModEnvParams, NUM_MODENV> modEnvParams;
    DelayParams delayParams;
    MasterParams masterParams;

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
        masterParams.freeze();
    }
};

//==============================================================================
class AllParams : public SynthParametersBase {
public:
    GlobalParams globalParams;
    VoiceParams voiceParams;
    MainParams mainParams;

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
        mainParams.freeze();
    }

private:
};