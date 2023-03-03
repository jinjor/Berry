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

    OscParams(int timbreIndex, int index);
    OscParams(const OscParams&) = delete;
    OscParams(OscParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    float gain;

    void freeze() { gain = Gain->get(); }

private:
    int index;
    OscParams(){};
};

//==============================================================================
class NoiseParams : public SynthParametersBase {
public:
    juce::AudioParameterFloat* Gain;

    NoiseParams(int timbreIndex, int index);
    NoiseParams(const NoiseParams&) = delete;
    NoiseParams(NoiseParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    float gain;

    void freeze() { gain = Gain->get(); }

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
    juce::AudioParameterFloat* NoiseGain;
    std::array<NoiseParams, NUM_NOISE> noiseParams;
    std::array<EnvelopeParams, NUM_NOISE> noiseEnvelopeParams;

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
        for (int i = 0; i < NUM_NOISE; ++i) {
            noiseParams[i].freeze();
            noiseEnvelopeParams[i].freeze();
        }
    }
};

//==============================================================================
class NoiseUnitParams : public SynthParametersBase {
public:
    juce::AudioParameterChoice* Waveform;
    std::array<FilterParams, NUM_NOISE_FILTER> filterParams;

    NoiseUnitParams(int index);
    NoiseUnitParams(const NoiseUnitParams&) = delete;
    NoiseUnitParams(NoiseUnitParams&&) noexcept = default;

    virtual void addAllParameters(juce::AudioProcessor& processor) override;
    virtual void saveParameters(juce::XmlElement& xml) override;
    virtual void loadParameters(juce::XmlElement& xml) override;

    WAVEFORM getWaveForm() { return NOISE_WAVEFORM_VALUES[Waveform->getIndex()]; }

    int index;
    WAVEFORM waveform;

    void freeze() {
        waveform = getWaveForm();
        for (auto& params : filterParams) {
            params.freeze();
        }
    }
};

//==============================================================================
class SoloMuteParams {
public:
    SoloMuteParams() {}
    SoloMuteParams(const SoloMuteParams&) = delete;
    SoloMuteParams(SoloMuteParams&&) noexcept = default;

    bool isSolo(bool isNoise, int index) { return isNoise ? noiseSolos[index] : harmonicSolos[index]; }
    bool isMute(bool isNoise, int index) {
        if (soloExists()) {
            return !isSolo(isNoise, index);
        }
        return isNoise ? noiseMutes[index] : harmonicMutes[index];
    }
    void turnOnSolo(bool isNoise, int index) { (isNoise ? noiseSolos[index] : harmonicSolos[index]) = true; }
    void turnOffSolo(bool isNoise, int index) { (isNoise ? noiseSolos[index] : harmonicSolos[index]) = false; }
    void turnOnMute(bool isNoise, int index) {
        (isNoise ? noiseSolos[index] : harmonicSolos[index]) = false;
        (isNoise ? noiseMutes[index] : harmonicMutes[index]) = true;
    }
    void turnOffMute(bool isNoise, int index) {
        if (soloExists()) {
            (isNoise ? noiseSolos[index] : harmonicSolos[index]) = true;
        }
        (isNoise ? noiseMutes[index] : harmonicMutes[index]) = false;
    }
    void toggleSolo(bool isNoise, int index) {
        isSolo(isNoise, index) ? turnOffSolo(isNoise, index) : turnOnSolo(isNoise, index);
    }
    void toggleMute(bool isNoise, int index) {
        isMute(isNoise, index) ? turnOffMute(isNoise, index) : turnOnMute(isNoise, index);
    }

    bool harmonicMute[NUM_OSC]{};
    bool noiseMute[NUM_NOISE]{};
    void freeze() {
        for (int i = 0; i < NUM_OSC; i++) {
            harmonicMute[i] = isMute(false, i);
        }
        for (int i = 0; i < NUM_NOISE; i++) {
            noiseMute[i] = isMute(true, i);
        }
    }

private:
    std::array<bool, NUM_OSC> harmonicSolos{};
    std::array<bool, NUM_OSC> harmonicMutes{};
    std::array<bool, NUM_NOISE> noiseSolos{};
    std::array<bool, NUM_NOISE> noiseMutes{};
    bool soloExists() {
        for (auto solo : harmonicSolos) {
            if (solo) {
                return true;
            }
        }
        for (auto solo : noiseSolos) {
            if (solo) {
                return true;
            }
        }
        return false;
    }
};

//==============================================================================
struct CalculatedParams {
    double gain[NUM_OSC]{};
    double attackCurve[NUM_OSC]{};
    double attack[NUM_OSC]{};
    double decay[NUM_OSC]{};
    double release[NUM_OSC]{};
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
    SoloMuteParams soloMuteParams;
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
        soloMuteParams.freeze();
    }
    MainParams& getCurrentMainParams() { return mainParams[editingTimbreIndex]; }
    void calculateIntermediateParams(CalculatedParams& params, CalculatedParams& noiseParams, int noteNumber);

private:
};