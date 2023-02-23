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

    VoiceParams();
    VoiceParams(const VoiceParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    int pitchBendRange;
    void freeze() { pitchBendRange = PitchBendRange->get(); }

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
    juce::AudioParameterFloat* Gain;
    juce::AudioParameterBool* NewEnvelope;

    OscParams(int timbreIndex, int index);
    OscParams(const OscParams&) = delete;
    OscParams(OscParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    float gain;
    bool newEnvelope;

    void freeze() {
        gain = Gain->get();
        newEnvelope = NewEnvelope->get();
    }

private:
    int index;
    OscParams(){};
};

//==============================================================================
class NoiseParams : public SynthParametersBase {
public:
    juce::AudioParameterFloat* Gain;
    juce::AudioParameterChoice* Waveform;

    NoiseParams(int index);
    NoiseParams(const NoiseParams&) = delete;
    NoiseParams(NoiseParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    WAVEFORM getWaveForm() { return NOISE_WAVEFORM_VALUES[Waveform->getIndex()]; }

    float gain;
    WAVEFORM waveform;

    void freeze() {
        gain = Gain->get();
        waveform = getWaveForm();
    }

private:
    NoiseParams(){};
};

//==============================================================================
class EnvelopeParams : public SynthParametersBase {
public:
    juce::AudioParameterFloat* AttackCurve;
    juce::AudioParameterFloat* Attack;
    juce::AudioParameterFloat* Decay;
    juce::AudioParameterFloat* Release;

    EnvelopeParams(int timbreIndex, int index);
    EnvelopeParams(const EnvelopeParams&) = delete;
    EnvelopeParams(EnvelopeParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    float attackCurve;
    float attack;
    float decay;
    float release;
    void freeze() {
        attackCurve = AttackCurve->get();
        attack = Attack->get();
        decay = Decay->get();
        release = Release->get();
    }

private:
    EnvelopeParams(){};
};

//==============================================================================
class FilterParams : public SynthParametersBase {
public:
    juce::AudioParameterBool* Enabled;
    juce::AudioParameterChoice* Type;
    juce::AudioParameterChoice* FreqType;
    juce::AudioParameterFloat* Hz;
    juce::AudioParameterInt* Semitone;
    juce::AudioParameterFloat* Q;
    juce::AudioParameterFloat* Gain;

    FilterParams(int noiseIndex, int filterIndex);
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
    FILTER_TYPE type;
    bool isFreqAbsoluteFreezed;
    float hz;
    int semitone;
    float q;
    float gain;
    void freeze() {
        enabled = Enabled->get();
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
class DelayParams : public SynthParametersBase {
public:
    juce::AudioParameterBool* Enabled;
    juce::AudioParameterChoice* Type;
    juce::AudioParameterFloat* TimeL;
    juce::AudioParameterFloat* TimeR;
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

    bool enabled;
    DELAY_TYPE type;
    float timeL;
    float timeR;
    float lowFreq;
    float highFreq;
    float feedback;
    float mix;
    void freeze() {
        enabled = Enabled->get();
        type = getType();
        timeL = TimeL->get();
        timeR = TimeR->get();
        lowFreq = LowFreq->get();
        highFreq = HighFreq->get();
        feedback = Feedback->get();
        mix = Mix->get();
    }

private:
};

//==============================================================================
class MainParams : public SynthParametersBase {
public:
    juce::AudioParameterInt* NoteNumber;
    std::array<OscParams, NUM_OSC> oscParams;
    std::array<EnvelopeParams, NUM_OSC> envelopeParams;

    MainParams(int index);
    MainParams(const MainParams&) = delete;
    MainParams(MainParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    int index;
    int noteNumber;
    void freeze() {
        noteNumber = NoteNumber->get();
        for (int i = 0; i < NUM_OSC; ++i) {
            oscParams[i].freeze();
            envelopeParams[i].freeze();
        }
    }
};

//==============================================================================
class NoiseUnitParams : public SynthParametersBase {
public:
    NoiseParams noiseParams;
    EnvelopeParams envelopeParams;
    std::array<FilterParams, NUM_NOISE_FILTER> filterParams;

    NoiseUnitParams(int index);
    NoiseUnitParams(const NoiseUnitParams&) = delete;
    NoiseUnitParams(NoiseUnitParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    int index;
    void freeze() {
        noiseParams.freeze();
        envelopeParams.freeze();
        for (auto& params : filterParams) {
            params.freeze();
        }
    }
};

//==============================================================================
class AllParams : public SynthParametersBase {
public:
    GlobalParams globalParams;
    VoiceParams voiceParams;
    std::array<MainParams, NUM_TIMBRES> mainParams;
    std::array<NoiseUnitParams, NUM_NOISE> noiseUnitParams;
    DelayParams delayParams;
    MasterParams masterParams;
    // UI の状態
    int editingTimbreIndex = 0;

    AllParams();
    AllParams(const AllParams&) = delete;
    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    void freeze() {
        globalParams.freeze();
        voiceParams.freeze();
        for (auto& params : mainParams) {
            params.freeze();
        }
        for (auto& params : noiseUnitParams) {
            params.freeze();
        }
        delayParams.freeze();
        masterParams.freeze();
    }
    MainParams& getCurrentMainParams() { return mainParams[editingTimbreIndex]; }

private:
};