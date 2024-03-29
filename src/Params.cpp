#include "Params.h"

#include "Voice.h"

namespace {
juce::NormalisableRange<float> rangeWithSkewForCentre(float rangeStart, float rangeEnd, float centrePointValue) {
    auto range = juce::NormalisableRange(rangeStart, rangeEnd);
    range.setSkewForCentre(centrePointValue);
    return range;
}
}  // namespace
//==============================================================================
GlobalParams::GlobalParams() {
    std::string idPrefix = "GLOBAL_";
    std::string namePrefix = "Global ";
    Pitch = new juce::AudioParameterFloat(idPrefix + "PITCH", namePrefix + "Pitch", -1.0f, 1.0f, 0.0f);
    Pan = new juce::AudioParameterFloat(idPrefix + "PAN", namePrefix + "Pan", -1.0f, 1.0f, 0.0f);
    Expression = new juce::AudioParameterFloat(idPrefix + "EXPRESSION", namePrefix + "Expression", 0.0f, 1.0f, 1.0f);
    MidiVolume = new juce::AudioParameterFloat(idPrefix + "MIDI_VOLUME", namePrefix + "Midi Volume", 0.0f, 1.0f, 1.0f);
}
void GlobalParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Pitch);
    processor.addParameter(Pan);
    processor.addParameter(Expression);
    processor.addParameter(MidiVolume);
}
void GlobalParams::saveParameters(juce::XmlElement& xml) {}
void GlobalParams::loadParameters(juce::XmlElement& xml) {}

//==============================================================================
MasterParams::MasterParams() {
    juce::String idPrefix = "MASTER_";
    juce::String namePrefix = "Master ";
    Pan = new juce::AudioParameterFloat(idPrefix + "PAN", namePrefix + "Pan", -1.0f, 1.0f, 0.0f);
    MasterVolume = new juce::AudioParameterFloat(idPrefix + "VOLUME", namePrefix + "Volume", 0.0f, 1.0f, 1.0f);
}
void MasterParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Pan);
    processor.addParameter(MasterVolume);
}
void MasterParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Pan->paramID, (double)Pan->get());
    xml.setAttribute(MasterVolume->paramID, (double)MasterVolume->get());
}
void MasterParams::loadParameters(juce::XmlElement& xml) {
    *Pan = (float)xml.getDoubleAttribute(Pan->paramID, 0);
    *MasterVolume = (float)xml.getDoubleAttribute(MasterVolume->paramID, 1.0);
}

//==============================================================================
VoiceParams::VoiceParams() {
    std::string idPrefix = "VOICE_";
    std::string namePrefix = "Voice ";
    PitchBendRange =
        new juce::AudioParameterInt(idPrefix + "PITCH_BEND_RANGE", namePrefix + "Pitch-Bend Range", 1, 12, 2);
}
void VoiceParams::addAllParameters(juce::AudioProcessor& processor) { processor.addParameter(PitchBendRange); }
void VoiceParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(PitchBendRange->paramID, PitchBendRange->get());
}
void VoiceParams::loadParameters(juce::XmlElement& xml) {
    *PitchBendRange = xml.getIntAttribute(PitchBendRange->paramID, 2);
}

//==============================================================================
OscParams::OscParams(int timbreIndex, int index) : index(index) {
    auto idPrefix = "T" + std::to_string(timbreIndex) + "_OSC" + std::to_string(index) + "_";
    auto namePrefix = "T" + std::to_string(timbreIndex) + " OSC" + std::to_string(index) + " ";
    Gain = new juce::AudioParameterFloat(
        idPrefix + "GAIN", namePrefix + "Gain", rangeWithSkewForCentre(0.0f, 4.0f, 1.0f), 1.0f);
}
void OscParams::addAllParameters(juce::AudioProcessor& processor) { processor.addParameter(Gain); }
void OscParams::saveParameters(juce::XmlElement& xml) { xml.setAttribute(Gain->paramID, (double)Gain->get()); }
void OscParams::loadParameters(juce::XmlElement& xml) { *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0); }

//==============================================================================
NoiseParams::NoiseParams(int timbreIndex, int index) {
    auto idPrefix = "T" + std::to_string(timbreIndex) + "_NOISE" + std::to_string(index) + "_";
    auto namePrefix = "T" + std::to_string(timbreIndex) + " Noise" + std::to_string(index) + " ";
    Gain = new juce::AudioParameterFloat(
        idPrefix + "GAIN", namePrefix + "Gain", rangeWithSkewForCentre(0.0f, 4.0f, 1.0f), 1.0f);
}
void NoiseParams::addAllParameters(juce::AudioProcessor& processor) { processor.addParameter(Gain); }
void NoiseParams::saveParameters(juce::XmlElement& xml) { xml.setAttribute(Gain->paramID, (double)Gain->get()); }
void NoiseParams::loadParameters(juce::XmlElement& xml) { *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0); }

//==============================================================================
EnvelopeParams::EnvelopeParams(int timbreIndex, int index) {
    auto idPrefix = "T" + std::to_string(timbreIndex) + "_ENV" + std::to_string(index) + "_";
    auto namePrefix = "T" + std::to_string(timbreIndex) + " Env" + std::to_string(index) + " ";
    AttackCurve = new juce::AudioParameterFloat(idPrefix + "ATTACK_CURVE", "Attack Curve", 0.01, 0.99, 0.5f);
    Attack = new juce::AudioParameterFloat(
        idPrefix + "ATTACK", "Attack", rangeWithSkewForCentre(0.001f, 0.2f, 0.05f), 0.05f);
    Decay = new juce::AudioParameterFloat(idPrefix + "DECAY", "Decay", rangeWithSkewForCentre(0.01f, 2.0f, 0.3f), 0.1f);
    Release =
        new juce::AudioParameterFloat(idPrefix + "RELEASE", "Release", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.1f);
}
void EnvelopeParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(AttackCurve);
    processor.addParameter(Attack);
    processor.addParameter(Decay);
    processor.addParameter(Release);
}
void EnvelopeParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(AttackCurve->paramID, (double)AttackCurve->get());
    xml.setAttribute(Attack->paramID, (double)Attack->get());
    xml.setAttribute(Decay->paramID, (double)Decay->get());
    xml.setAttribute(Release->paramID, (double)Release->get());
}
void EnvelopeParams::loadParameters(juce::XmlElement& xml) {
    *AttackCurve = (float)xml.getDoubleAttribute(AttackCurve->paramID, 0.5);
    *Attack = (float)xml.getDoubleAttribute(Attack->paramID, 0.01);
    *Decay = (float)xml.getDoubleAttribute(Decay->paramID, 0.01);
    *Release = (float)xml.getDoubleAttribute(Release->paramID, 0.01);
}

//==============================================================================
FilterParams::FilterParams(int noiseIndex, int filterIndex) {
    auto idPrefix = "N" + juce::String(noiseIndex) + "_FILTER" + juce::String(filterIndex) + "_";
    auto namePrefix = "N" + juce::String(noiseIndex) + " Filter" + juce::String(filterIndex) + " ";

    // Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", true);  // TODO
    Type = new juce::AudioParameterChoice(
        idPrefix + "TYPE", namePrefix + "Type", FILTER_TYPE_NAMES, FILTER_TYPE_NAMES.indexOf("Lowpass"));
    FreqType = new juce::AudioParameterChoice(idPrefix + "FREQ_TYPE",
                                              namePrefix + "Freq Type",
                                              FILTER_FREQ_TYPE_NAMES,
                                              FILTER_FREQ_TYPE_NAMES.indexOf("Abs"));
    Hz = new juce::AudioParameterFloat(
        idPrefix + "HZ", namePrefix + "Hz", rangeWithSkewForCentre(30.0f, 20000.0f, 2000.0f), 4000.0f);
    Semitone = new juce::AudioParameterInt(idPrefix + "SEMITONE", namePrefix + "Semitone", -48, 48, 0);
    Q = new juce::AudioParameterFloat(
        idPrefix + "Q", namePrefix + "Q", rangeWithSkewForCentre(0.01f, 100.0f, 1.0f), 1.0f);
    Gain = new juce::AudioParameterFloat(idPrefix + "GAIN", namePrefix + "Gain", -20.0f, 20.0f, 0.0f);
}
void FilterParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Type);
    processor.addParameter(FreqType);
    processor.addParameter(Hz);
    processor.addParameter(Semitone);
    processor.addParameter(Q);
    processor.addParameter(Gain);
}
void FilterParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Type->paramID, Type->getIndex());
    xml.setAttribute(FreqType->paramID, FreqType->getIndex());
    xml.setAttribute(Hz->paramID, (double)Hz->get());
    xml.setAttribute(Semitone->paramID, Semitone->get());
    xml.setAttribute(Q->paramID, (double)Q->get());
    xml.setAttribute(Gain->paramID, (double)Gain->get());
}
void FilterParams::loadParameters(juce::XmlElement& xml) {
    // *Enabled = xml.getBoolAttribute(Enabled->paramID, false);
    *Enabled = true;  // TODO
    *Type = xml.getIntAttribute(Type->paramID, 0);
    *FreqType = xml.getIntAttribute(FreqType->paramID, 0);
    *Hz = (float)xml.getDoubleAttribute(Hz->paramID, 0);
    *Semitone = xml.getDoubleAttribute(Semitone->paramID, 0);
    *Q = (float)xml.getDoubleAttribute(Q->paramID, 1.0);
    *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0);
}

//==============================================================================
DelayParams::DelayParams() {
    juce::String idPrefix = "DELAY_";
    juce::String namePrefix = "Delay ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Type = new juce::AudioParameterChoice(
        idPrefix + "TYPE", namePrefix + "Type", DELAY_TYPE_NAMES, DELAY_TYPE_NAMES.indexOf("Parallel"));
    TimeL = new juce::AudioParameterFloat(
        idPrefix + "TIME_L", namePrefix + "TimeL", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.3f);
    TimeR = new juce::AudioParameterFloat(
        idPrefix + "TIME_R", namePrefix + "TimeR", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.4f);
    LowFreq = new juce::AudioParameterFloat(
        idPrefix + "LOW_FREQ", namePrefix + "LowFfreq", rangeWithSkewForCentre(10.0f, 20000.0f, 2000.0f), 10.0f);
    HighFreq = new juce::AudioParameterFloat(
        idPrefix + "HIGH_FREQ", namePrefix + "HighFreq", rangeWithSkewForCentre(10.0f, 20000.0f, 2000.0f), 20000.0f);
    Feedback = new juce::AudioParameterFloat(idPrefix + "FEEDBACK", namePrefix + "Feedback", 0.0f, 1.0f, 0.3f);
    Mix = new juce::AudioParameterFloat(idPrefix + "MIX", namePrefix + "Mix", 0.0f, 1.0f, 0.3f);
}
void DelayParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Type);
    processor.addParameter(TimeL);
    processor.addParameter(TimeR);
    processor.addParameter(LowFreq);
    processor.addParameter(HighFreq);
    processor.addParameter(Feedback);
    processor.addParameter(Mix);
}
void DelayParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Type->paramID, Type->getIndex());
    xml.setAttribute(TimeL->paramID, (double)TimeL->get());
    xml.setAttribute(TimeR->paramID, (double)TimeR->get());
    xml.setAttribute(LowFreq->paramID, (double)LowFreq->get());
    xml.setAttribute(HighFreq->paramID, (double)HighFreq->get());
    xml.setAttribute(Feedback->paramID, (double)Feedback->get());
    xml.setAttribute(Mix->paramID, (double)Mix->get());
}
void DelayParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getBoolAttribute(Enabled->paramID, false);
    *Type = xml.getIntAttribute(Type->paramID, 0);
    *TimeL = (float)xml.getDoubleAttribute(TimeL->paramID, 0.01);
    *TimeR = (float)xml.getDoubleAttribute(TimeR->paramID, 0.01);
    *LowFreq = (float)xml.getDoubleAttribute(LowFreq->paramID, 10);
    *HighFreq = (float)xml.getDoubleAttribute(HighFreq->paramID, 20000);
    *Feedback = (float)xml.getDoubleAttribute(Feedback->paramID, 0);
    *Mix = (float)xml.getDoubleAttribute(Mix->paramID, 0);
}

//==============================================================================
MainParams::MainParams(int index)
    : index(index),
      oscParams{OscParams{index, 0},
                OscParams{index, 1},
                OscParams{index, 2},
                OscParams{index, 3},
                OscParams{index, 4},
                OscParams{index, 5},
                OscParams{index, 6},
                OscParams{index, 7},
                OscParams{index, 8},
                OscParams{index, 9},
                OscParams{index, 10},
                OscParams{index, 11},
                OscParams{index, 12},
                OscParams{index, 13},
                OscParams{index, 14},
                OscParams{index, 15}},
      envelopeParams{EnvelopeParams{index, 0},
                     EnvelopeParams{index, 1},
                     EnvelopeParams{index, 2},
                     EnvelopeParams{index, 3},
                     EnvelopeParams{index, 4},
                     EnvelopeParams{index, 5},
                     EnvelopeParams{index, 6},
                     EnvelopeParams{index, 7},
                     EnvelopeParams{index, 8},
                     EnvelopeParams{index, 9},
                     EnvelopeParams{index, 10},
                     EnvelopeParams{index, 11},
                     EnvelopeParams{index, 12},
                     EnvelopeParams{index, 13},
                     EnvelopeParams{index, 14},
                     EnvelopeParams{index, 15}},
      noiseParams{NoiseParams{index, 0}, NoiseParams{index, 1}},
      noiseEnvelopeParams{EnvelopeParams{index, NUM_OSC + 0}, EnvelopeParams{index, NUM_OSC + 1}} {  // TODO
    auto idPrefix = "T" + std::to_string(index) + "_";
    auto namePrefix = "T" + std::to_string(index) + " ";
    NoteNumber = new juce::AudioParameterInt(idPrefix + "NOTE_NUMBER",
                                             namePrefix + "Note Number",
                                             MIN_OF_88_NOTES,
                                             MAX_OF_88_NOTES,
                                             DEFAULT_TIMBRE_NOTES[index]);
}
void MainParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(NoteNumber);
    for (auto& params : oscParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : envelopeParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : noiseParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : noiseEnvelopeParams) {
        params.addAllParameters(processor);
    }
}
void MainParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(NoteNumber->paramID, NoteNumber->get());
    for (auto& param : oscParams) {
        param.saveParameters(xml);
    }
    for (auto& param : envelopeParams) {
        param.saveParameters(xml);
    }
    for (auto& param : noiseParams) {
        param.saveParameters(xml);
    }
    for (auto& param : noiseEnvelopeParams) {
        param.saveParameters(xml);
    }
}
void MainParams::loadParameters(juce::XmlElement& xml) {
    *NoteNumber = xml.getIntAttribute(NoteNumber->paramID, DEFAULT_TIMBRE_NOTES[index]);
    for (auto& param : oscParams) {
        param.loadParameters(xml);
    }
    for (auto& param : envelopeParams) {
        param.loadParameters(xml);
    }
    for (auto& param : noiseParams) {
        param.loadParameters(xml);
    }
    for (auto& param : noiseEnvelopeParams) {
        param.loadParameters(xml);
    }
}

//==============================================================================
NoiseUnitParams::NoiseUnitParams(int index)
    : index(index), filterParams{FilterParams{index, 0}, FilterParams{index, 1}} {
    auto idPrefix = "NOISE" + std::to_string(index) + "_";
    auto namePrefix = "Noise" + std::to_string(index) + " ";
    Waveform = new juce::AudioParameterChoice(
        idPrefix + "WAVEFORM", namePrefix + "Waveform", NOISE_WAVEFORM_NAMES, NOISE_WAVEFORM_NAMES.indexOf("White"));
}
void NoiseUnitParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Waveform);
    for (auto& param : filterParams) {
        param.addAllParameters(processor);
    }
}
void NoiseUnitParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Waveform->paramID, Waveform->getIndex());
    for (auto& param : filterParams) {
        param.saveParameters(xml);
    }
}
void NoiseUnitParams::loadParameters(juce::XmlElement& xml) {
    *Waveform = xml.getIntAttribute(Waveform->paramID, NOISE_WAVEFORM_NAMES.indexOf("White"));
    for (auto& param : filterParams) {
        param.loadParameters(xml);
    }
}

//==============================================================================
AllParams::AllParams()
    : globalParams{},
      voiceParams{},
      mainParams{MainParams{0}, MainParams{1}, MainParams{2}, MainParams{3}},
      noiseUnitParams{NoiseUnitParams{0}, NoiseUnitParams{1}},
      delayParams{},
      masterParams{},
      soloMuteParams{} {
    freeze();
}
void AllParams::addAllParameters(juce::AudioProcessor& processor) {
    globalParams.addAllParameters(processor);
    voiceParams.addAllParameters(processor);
    for (auto& params : mainParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : noiseUnitParams) {
        params.addAllParameters(processor);
    }
    delayParams.addAllParameters(processor);
    masterParams.addAllParameters(processor);
}
void AllParams::saveParameters(juce::XmlElement& xml) {
    globalParams.saveParameters(xml);
    voiceParams.saveParameters(xml);
    for (auto& param : mainParams) {
        param.saveParameters(xml);
    }
    for (auto& param : noiseUnitParams) {
        param.saveParameters(xml);
    }
    delayParams.saveParameters(xml);
    masterParams.saveParameters(xml);
}
void AllParams::loadParameters(juce::XmlElement& xml) {
    globalParams.loadParameters(xml);
    voiceParams.loadParameters(xml);
    for (auto& param : mainParams) {
        param.loadParameters(xml);
    }
    for (auto& param : noiseUnitParams) {
        param.loadParameters(xml);
    }
    delayParams.loadParameters(xml);
    masterParams.loadParameters(xml);
    // UI で防ぐようになっているが、互換性のない変更をした時のために修正できるようにしておく
    for (int i = 0; i < NUM_TIMBRES; i++) {
        int min = i == 0 ? MIN_OF_88_NOTES : mainParams[i - 1].NoteNumber->get() + 1;
        if (mainParams[i].NoteNumber->get() < min) {
            *mainParams[i].NoteNumber = min;
        }
    }
    for (int i = NUM_TIMBRES - 1; i >= 0; i--) {
        int max = i == NUM_TIMBRES - 1 ? MAX_OF_88_NOTES : mainParams[i + 1].NoteNumber->get() - 1;
        if (mainParams[i].NoteNumber->get() > max) {
            *mainParams[i].NoteNumber = max;
        }
    }
    freeze();
}
void AllParams::calculateIntermediateParams(CalculatedParams& params, CalculatedParams& noiseParams, int noteNumber) {
    auto leftIndex = 0;
    auto rightIndex = NUM_TIMBRES - 1;
    auto leftNote = 0;
    auto rightNote = 127;
    for (int i = 0; i < NUM_TIMBRES; i++) {
        if (noteNumber <= mainParams[i].noteNumber) {
            rightIndex = i;
            rightNote = mainParams[i].noteNumber;
            break;
        }
    }
    for (int i = NUM_TIMBRES - 1; i >= 0; i--) {
        if (mainParams[i].noteNumber < noteNumber) {
            leftIndex = i;
            leftNote = mainParams[i].noteNumber;
            break;
        }
    }
    jassert(rightNote - leftNote > 0);
    auto leftRatio = (double)(rightNote - noteNumber) / (double)(rightNote - leftNote);
    auto rightRatio = 1.0 - leftRatio;
    auto& leftParams = mainParams[leftIndex];
    auto& rightParams = mainParams[rightIndex];
    for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
        auto& leftOsc = leftParams.oscParams[oscIndex];
        auto& rightOsc = rightParams.oscParams[oscIndex];
        auto& leftEnv = leftParams.envelopeParams[oscIndex];
        auto& rightEnv = rightParams.envelopeParams[oscIndex];
        params.gain[oscIndex] = leftOsc.gain * leftRatio + rightOsc.gain * rightRatio;
        params.attackCurve[oscIndex] = leftEnv.attackCurve * leftRatio + rightEnv.attackCurve * rightRatio;
        params.attack[oscIndex] = leftEnv.attack * leftRatio + rightEnv.attack * rightRatio;
        params.decay[oscIndex] = leftEnv.decay * leftRatio + rightEnv.decay * rightRatio;
        params.release[oscIndex] = leftEnv.release * leftRatio + rightEnv.release * rightRatio;
    }
    for (int noiseIndex = 0; noiseIndex < NUM_NOISE; ++noiseIndex) {
        auto& leftOsc = leftParams.noiseParams[noiseIndex];
        auto& rightOsc = rightParams.noiseParams[noiseIndex];
        auto& leftEnv = leftParams.noiseEnvelopeParams[noiseIndex];
        auto& rightEnv = rightParams.noiseEnvelopeParams[noiseIndex];
        noiseParams.gain[noiseIndex] = leftOsc.gain * leftRatio + rightOsc.gain * rightRatio;
        noiseParams.attackCurve[noiseIndex] = leftEnv.attackCurve * leftRatio + rightEnv.attackCurve * rightRatio;
        noiseParams.attack[noiseIndex] = leftEnv.attack * leftRatio + rightEnv.attack * rightRatio;
        noiseParams.decay[noiseIndex] = leftEnv.decay * leftRatio + rightEnv.decay * rightRatio;
        noiseParams.release[noiseIndex] = leftEnv.release * leftRatio + rightEnv.release * rightRatio;
    }
}