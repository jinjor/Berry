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
    freeze();
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
MasterParams::MasterParams(std::string idPrefix, std::string namePrefix) {
    idPrefix += "MASTER_";
    namePrefix += "Master ";
    Pan = new juce::AudioParameterFloat(idPrefix + "PAN", namePrefix + "Pan", -1.0f, 1.0f, 0.0f);
    MasterVolume = new juce::AudioParameterFloat(idPrefix + "VOLUME", namePrefix + "Volume", 0.0f, 1.0f, 1.0f);
    freeze();
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
    Mode = new juce::AudioParameterChoice(
        idPrefix + "MODE", namePrefix + "Mode", VOICE_MODE_NAMES, VOICE_MODE_NAMES.indexOf("Poly"));
    PitchBendRange =
        new juce::AudioParameterInt(idPrefix + "PITCH_BEND_RANGE", namePrefix + "Pitch-Bend Range", 1, 12, 2);
    TargetNoteKind = new juce::AudioParameterChoice(idPrefix + "TARGET_NOTE_KIND",
                                                    namePrefix + "Target Note Kind",
                                                    TARGET_NOTE_KINDS,
                                                    TARGET_NOTE_KINDS.indexOf("C"));
    TargetNoteOct = new juce::AudioParameterChoice(idPrefix + "TARGET_NOTE_OCT",
                                                   namePrefix + "Target Note Oct",
                                                   TARGET_NOTE_OCT_NAMES,
                                                   TARGET_NOTE_OCT_NAMES.indexOf("1"));
    freeze();
}
void VoiceParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Mode);
    processor.addParameter(PitchBendRange);
    processor.addParameter(TargetNoteKind);
    processor.addParameter(TargetNoteOct);
}
void VoiceParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Mode->paramID, Mode->getIndex());
    xml.setAttribute(PitchBendRange->paramID, PitchBendRange->get());
    xml.setAttribute(TargetNoteKind->paramID, TargetNoteKind->getIndex());
    xml.setAttribute(TargetNoteOct->paramID, TargetNoteOct->getIndex());
}
void VoiceParams::loadParameters(juce::XmlElement& xml) {
    *Mode = xml.getIntAttribute(Mode->paramID, 0);
    *PitchBendRange = xml.getIntAttribute(PitchBendRange->paramID, 2);
    *TargetNoteKind = xml.getIntAttribute(TargetNoteKind->paramID, TARGET_NOTE_KINDS.indexOf("C"));
    *TargetNoteOct = xml.getIntAttribute(TargetNoteOct->paramID, TARGET_NOTE_OCT_NAMES.indexOf("1"));
}

//==============================================================================
OscParams::OscParams(std::string idPrefix, std::string namePrefix, int index) {
    idPrefix += "OSC" + std::to_string(index) + "_";
    namePrefix += "OSC" + std::to_string(index) + " ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Waveform = new juce::AudioParameterChoice(
        idPrefix + "WAVEFORM", namePrefix + "Waveform", OSC_WAVEFORM_NAMES, OSC_WAVEFORM_NAMES.indexOf("Sine"));
    Edge = new juce::AudioParameterFloat(idPrefix + "EDGE", namePrefix + "Edge", 0.0f, 1.0f, 0.0f);
    Octave = new juce::AudioParameterInt(idPrefix + "OCTAVE", namePrefix + "Octave", -2, 2, 0);
    Coarse = new juce::AudioParameterInt(idPrefix + "COARSE", namePrefix + "Coarse", -12, 12, 0);
    Unison = new juce::AudioParameterInt(idPrefix + "UNISON", namePrefix + "Unison", 1, 4, 1);
    Detune = new juce::AudioParameterFloat(idPrefix + "DETUNE", namePrefix + "Detune", 0.0f, 1.0f, 0.0f);
    Spread = new juce::AudioParameterFloat(idPrefix + "SPREAD", namePrefix + "Spread", 0.0f, 1.0f, 0.0f);
    Gain = new juce::AudioParameterFloat(
        idPrefix + "GAIN", namePrefix + "Gain", rangeWithSkewForCentre(0.0f, 4.0f, 1.0f), 1.0f);
    Envelope = new juce::AudioParameterChoice(
        idPrefix + "ENVELOPE", namePrefix + "Envelope", OSC_ENV_NAMES, OSC_ENV_NAMES.indexOf("1"));
    freeze();
}
void OscParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Waveform);
    processor.addParameter(Edge);
    processor.addParameter(Octave);
    processor.addParameter(Coarse);
    processor.addParameter(Unison);
    processor.addParameter(Detune);
    processor.addParameter(Spread);
    processor.addParameter(Gain);
    processor.addParameter(Envelope);
}
void OscParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Waveform->paramID, Waveform->getIndex());
    xml.setAttribute(Edge->paramID, Edge->get());
    xml.setAttribute(Octave->paramID, Octave->get());
    xml.setAttribute(Coarse->paramID, Coarse->get());
    xml.setAttribute(Unison->paramID, Unison->get());
    xml.setAttribute(Detune->paramID, (double)Detune->get());
    xml.setAttribute(Spread->paramID, (double)Spread->get());
    xml.setAttribute(Gain->paramID, (double)Gain->get());
    xml.setAttribute(Envelope->paramID, Envelope->getIndex());
}
void OscParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getIntAttribute(Enabled->paramID, 0);
    *Waveform = xml.getIntAttribute(Waveform->paramID, 0);
    *Edge = (float)xml.getDoubleAttribute(Edge->paramID, 0.0);
    *Octave = xml.getIntAttribute(Octave->paramID, 0);
    *Coarse = xml.getIntAttribute(Coarse->paramID, 0);
    *Unison = xml.getIntAttribute(Unison->paramID, 1);
    *Detune = (float)xml.getDoubleAttribute(Detune->paramID, 0);
    *Spread = (float)xml.getDoubleAttribute(Spread->paramID, 0);
    *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0);
    *Envelope = xml.getIntAttribute(Envelope->paramID, 0);
}

//==============================================================================
EnvelopeParams::EnvelopeParams(std::string idPrefix, std::string namePrefix, int index) {
    idPrefix += "ENV" + std::to_string(index) + "_";
    namePrefix += "Env" + std::to_string(index) + " ";
    AttackCurve = new juce::AudioParameterFloat(idPrefix + "ATTACK_CURVE", "Attack Curve", 0.01, 0.99, 0.5f);
    Attack =
        new juce::AudioParameterFloat(idPrefix + "ATTACK", "Attack", rangeWithSkewForCentre(0.001f, 1.0f, 0.2f), 0.05f);
    Decay = new juce::AudioParameterFloat(idPrefix + "DECAY", "Decay", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.1f);
    Sustain = new juce::AudioParameterFloat(idPrefix + "SUSTAIN", "Sustain", 0.0f, 1.0f, 0.7f);
    Release =
        new juce::AudioParameterFloat(idPrefix + "RELEASE", "Release", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.1f);
    freeze();
}
void EnvelopeParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(AttackCurve);
    processor.addParameter(Attack);
    processor.addParameter(Decay);
    processor.addParameter(Sustain);
    processor.addParameter(Release);
}
void EnvelopeParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(AttackCurve->paramID, (double)AttackCurve->get());
    xml.setAttribute(Attack->paramID, (double)Attack->get());
    xml.setAttribute(Decay->paramID, (double)Decay->get());
    xml.setAttribute(Sustain->paramID, (double)Sustain->get());
    xml.setAttribute(Release->paramID, (double)Release->get());
}
void EnvelopeParams::loadParameters(juce::XmlElement& xml) {
    *AttackCurve = (float)xml.getDoubleAttribute(AttackCurve->paramID, 0.5);
    *Attack = (float)xml.getDoubleAttribute(Attack->paramID, 0.01);
    *Decay = (float)xml.getDoubleAttribute(Decay->paramID, 0.01);
    *Sustain = (float)xml.getDoubleAttribute(Sustain->paramID, 1.0);
    *Release = (float)xml.getDoubleAttribute(Release->paramID, 0.01);
}

//==============================================================================
FilterParams::FilterParams(std::string idPrefix, std::string namePrefix, int index) {
    idPrefix += "FILTER" + std::to_string(index) + "_";
    namePrefix += "Filter" + std::to_string(index) + " ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Target = new juce::AudioParameterChoice(
        idPrefix + "TARGET", namePrefix + "Target", FILTER_TARGET_NAMES, FILTER_TARGET_NAMES.indexOf("All"));
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
    freeze();
}
void FilterParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Target);
    processor.addParameter(Type);
    processor.addParameter(FreqType);
    processor.addParameter(Hz);
    processor.addParameter(Semitone);
    processor.addParameter(Q);
    processor.addParameter(Gain);
}
void FilterParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Target->paramID, Target->getIndex());
    xml.setAttribute(Type->paramID, Type->getIndex());
    xml.setAttribute(FreqType->paramID, FreqType->getIndex());
    xml.setAttribute(Hz->paramID, (double)Hz->get());
    xml.setAttribute(Semitone->paramID, Semitone->get());
    xml.setAttribute(Q->paramID, (double)Q->get());
    xml.setAttribute(Gain->paramID, (double)Gain->get());
}
void FilterParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getIntAttribute(Enabled->paramID, 0);
    *Target = xml.getIntAttribute(Target->paramID, NUM_OSC);
    *Type = xml.getIntAttribute(Type->paramID, 0);
    *FreqType = xml.getIntAttribute(FreqType->paramID, 0);
    *Hz = (float)xml.getDoubleAttribute(Hz->paramID, 0);
    *Semitone = xml.getDoubleAttribute(Semitone->paramID, 0);
    *Q = (float)xml.getDoubleAttribute(Q->paramID, 1.0);
    *Gain = (float)xml.getDoubleAttribute(Gain->paramID, 0);
}

//==============================================================================
ModEnvParams::ModEnvParams(std::string idPrefix, std::string namePrefix, int index) {
    idPrefix += "MODENV" + std::to_string(index) + "_";
    namePrefix += "ModEnv" + std::to_string(index) + " ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    TargetType = new juce::AudioParameterChoice(idPrefix + "TARGET_TYPE",
                                                namePrefix + "Target Type",
                                                MODENV_TARGET_TYPE_NAMES,
                                                MODENV_TARGET_TYPE_NAMES.indexOf("LFO"));
    TargetOsc = new juce::AudioParameterChoice(idPrefix + "TARGET_OSC",
                                               namePrefix + "Target OSC",
                                               MODENV_TARGET_OSC_NAMES,
                                               MODENV_TARGET_OSC_NAMES.indexOf("All"));
    TargetFilter = new juce::AudioParameterChoice(idPrefix + "TARGET_FILTER",
                                                  namePrefix + "Target Filter",
                                                  MODENV_TARGET_FILTER_NAMES,
                                                  MODENV_TARGET_FILTER_NAMES.indexOf("All"));
    TargetOscParam = new juce::AudioParameterChoice(idPrefix + "TARGET_OSC_PARAM",
                                                    namePrefix + "Target OSC Param",
                                                    MODENV_TARGET_OSC_PARAM_NAMES,
                                                    MODENV_TARGET_OSC_PARAM_NAMES.indexOf("Freq"));
    TargetFilterParam = new juce::AudioParameterChoice(idPrefix + "TARGET_FILTER_PARAM",
                                                       namePrefix + "Target Filter Param",
                                                       MODENV_TARGET_FILTER_PARAM_NAMES,
                                                       MODENV_TARGET_FILTER_PARAM_NAMES.indexOf("Freq"));
    Fade = new juce::AudioParameterChoice(
        idPrefix + "FADE", namePrefix + "Fade", MODENV_FADE_NAMES, MODENV_FADE_NAMES.indexOf("In"));
    PeakFreq = new juce::AudioParameterFloat(idPrefix + "PEAK_FREQ", namePrefix + "Peak Freq", -8.0f, 8.0, 2.0f);
    Wait = new juce::AudioParameterFloat(
        idPrefix + "WAIT", namePrefix + "Wait", rangeWithSkewForCentre(0.0f, 1.0f, 0.2f), 0.5f);
    Attack = new juce::AudioParameterFloat(
        idPrefix + "ATTACK", namePrefix + "Attack", rangeWithSkewForCentre(0.0f, 1.0f, 0.2f), 0.0f);
    Decay = new juce::AudioParameterFloat(
        idPrefix + "DECAY", namePrefix + "Decay", rangeWithSkewForCentre(0.0f, 1.0f, 0.4f), 0.2f);
    freeze();
}
void ModEnvParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(TargetType);
    processor.addParameter(TargetOsc);
    processor.addParameter(TargetFilter);
    processor.addParameter(TargetOscParam);
    processor.addParameter(TargetFilterParam);
    processor.addParameter(Fade);
    processor.addParameter(PeakFreq);
    processor.addParameter(Wait);
    processor.addParameter(Attack);
    processor.addParameter(Decay);
}
void ModEnvParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(TargetType->paramID, TargetType->getIndex());
    xml.setAttribute(TargetOsc->paramID, TargetOsc->getIndex());
    xml.setAttribute(TargetFilter->paramID, TargetFilter->getIndex());
    xml.setAttribute(TargetOscParam->paramID, TargetOscParam->getIndex());
    xml.setAttribute(TargetFilterParam->paramID, TargetFilterParam->getIndex());
    xml.setAttribute(Fade->paramID, Fade->getIndex());
    xml.setAttribute(PeakFreq->paramID, (double)PeakFreq->get());
    xml.setAttribute(Wait->paramID, (double)Wait->get());
    xml.setAttribute(Attack->paramID, (double)Attack->get());
    xml.setAttribute(Decay->paramID, (double)Decay->get());
}
void ModEnvParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getIntAttribute(Enabled->paramID, 0);
    *TargetType = xml.getIntAttribute(TargetType->paramID, 0);
    *TargetOsc = xml.getIntAttribute(TargetOsc->paramID, NUM_OSC);
    *TargetFilter = xml.getIntAttribute(TargetOsc->paramID, NUM_FILTER);
    *TargetOscParam = xml.getIntAttribute(TargetOscParam->paramID, 0);
    *TargetFilterParam = xml.getIntAttribute(TargetFilterParam->paramID, 0);
    *Fade = xml.getIntAttribute(Fade->paramID, 0);
    *PeakFreq = (float)xml.getDoubleAttribute(PeakFreq->paramID, 0);
    *Wait = (float)xml.getDoubleAttribute(Wait->paramID, 0);
    *Attack = (float)xml.getDoubleAttribute(Attack->paramID, 0.01);
    *Decay = (float)xml.getDoubleAttribute(Decay->paramID, 0.1);
}

//==============================================================================
DelayParams::DelayParams(std::string idPrefix, std::string namePrefix) {
    idPrefix += "DELAY_";
    namePrefix += "Delay ";
    Enabled = new juce::AudioParameterBool(idPrefix + "ENABLED", namePrefix + "Enabled", false);
    Type = new juce::AudioParameterChoice(
        idPrefix + "TYPE", namePrefix + "Type", DELAY_TYPE_NAMES, DELAY_TYPE_NAMES.indexOf("Parallel"));
    Sync = new juce::AudioParameterBool(idPrefix + "SYNC", namePrefix + "Sync", false);
    TimeL = new juce::AudioParameterFloat(
        idPrefix + "TIME_L", namePrefix + "TimeL", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.3f);
    TimeR = new juce::AudioParameterFloat(
        idPrefix + "TIME_R", namePrefix + "TimeR", rangeWithSkewForCentre(0.01f, 1.0f, 0.4f), 0.4f);
    TimeSyncL = new juce::AudioParameterChoice(idPrefix + "TIME_SYNC_L",
                                               namePrefix + "TimeSyncL",
                                               DELAY_TIME_SYNC_NAMES,
                                               DELAY_TIME_SYNC_NAMES.indexOf("1/8"));
    TimeSyncR = new juce::AudioParameterChoice(idPrefix + "TIME_SYNC_R",
                                               namePrefix + "TimeSyncR",
                                               DELAY_TIME_SYNC_NAMES,
                                               DELAY_TIME_SYNC_NAMES.indexOf("1/8"));
    LowFreq = new juce::AudioParameterFloat(
        idPrefix + "LOW_FREQ", namePrefix + "LowFfreq", rangeWithSkewForCentre(10.0f, 20000.0f, 2000.0f), 10.0f);
    HighFreq = new juce::AudioParameterFloat(
        idPrefix + "HIGH_FREQ", namePrefix + "HighFreq", rangeWithSkewForCentre(10.0f, 20000.0f, 2000.0f), 20000.0f);
    Feedback = new juce::AudioParameterFloat(idPrefix + "FEEDBACK", namePrefix + "Feedback", 0.0f, 1.0f, 0.3f);
    Mix = new juce::AudioParameterFloat(idPrefix + "MIX", namePrefix + "Mix", 0.0f, 1.0f, 0.3f);
    freeze();
}
void DelayParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(Enabled);
    processor.addParameter(Type);
    processor.addParameter(Sync);
    processor.addParameter(TimeL);
    processor.addParameter(TimeR);
    processor.addParameter(TimeSyncL);
    processor.addParameter(TimeSyncR);
    processor.addParameter(LowFreq);
    processor.addParameter(HighFreq);
    processor.addParameter(Feedback);
    processor.addParameter(Mix);
}
void DelayParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(Enabled->paramID, Enabled->get());
    xml.setAttribute(Type->paramID, Type->getIndex());
    xml.setAttribute(Sync->paramID, Sync->get());
    xml.setAttribute(TimeL->paramID, (double)TimeL->get());
    xml.setAttribute(TimeR->paramID, (double)TimeR->get());
    xml.setAttribute(TimeSyncL->paramID, TimeSyncL->getIndex());
    xml.setAttribute(TimeSyncR->paramID, TimeSyncR->getIndex());
    xml.setAttribute(LowFreq->paramID, (double)LowFreq->get());
    xml.setAttribute(HighFreq->paramID, (double)HighFreq->get());
    xml.setAttribute(Feedback->paramID, (double)Feedback->get());
    xml.setAttribute(Mix->paramID, (double)Mix->get());
}
void DelayParams::loadParameters(juce::XmlElement& xml) {
    *Enabled = xml.getBoolAttribute(Enabled->paramID, false);
    *Type = xml.getIntAttribute(Type->paramID, 0);
    *Sync = xml.getBoolAttribute(Sync->paramID, false);
    *TimeL = (float)xml.getDoubleAttribute(TimeL->paramID, 0.01);
    *TimeR = (float)xml.getDoubleAttribute(TimeR->paramID, 0.01);
    *TimeSyncL = xml.getIntAttribute(TimeSyncL->paramID, 0);
    *TimeSyncR = xml.getIntAttribute(TimeSyncR->paramID, 0);
    *LowFreq = (float)xml.getDoubleAttribute(LowFreq->paramID, 10);
    *HighFreq = (float)xml.getDoubleAttribute(HighFreq->paramID, 20000);
    *Feedback = (float)xml.getDoubleAttribute(Feedback->paramID, 0);
    *Mix = (float)xml.getDoubleAttribute(Mix->paramID, 0);
}

//==============================================================================
DrumParams::DrumParams(std::string idPrefix, std::string namePrefix) {
    idPrefix += "DELAY_";
    namePrefix += "Delay ";
    NoteToPlay = new juce::AudioParameterInt(idPrefix + "NOTE_TO_PLAY", namePrefix + "Note to play", 0, 127, 60);
    NoteToMuteEnabled =
        new juce::AudioParameterBool(idPrefix + "NOTE_TO_MUTE_ENABLED", namePrefix + "Note to Mute Enabled", false);
    NoteToMuteKind = new juce::AudioParameterChoice(idPrefix + "NOTE_TO_MUTE_KIND",
                                                    namePrefix + "Note to Mute Kind",
                                                    TARGET_NOTE_KINDS,
                                                    TARGET_NOTE_KINDS.indexOf("C"));
    NoteToMuteOct = new juce::AudioParameterChoice(idPrefix + "NOTE_TO_MUTE_OCT",
                                                   namePrefix + "Note to Mute Oct",
                                                   TARGET_NOTE_OCT_NAMES,
                                                   TARGET_NOTE_OCT_NAMES.indexOf("1"));
    Bus = new juce::AudioParameterChoice(idPrefix + "Bus", namePrefix + "Bus", BUS_NAMES, BUS_NAMES.indexOf("Main"));
    freeze();
}
void DrumParams::addAllParameters(juce::AudioProcessor& processor) {
    processor.addParameter(NoteToPlay);
    processor.addParameter(NoteToMuteEnabled);
    processor.addParameter(NoteToMuteKind);
    processor.addParameter(NoteToMuteOct);
    processor.addParameter(Bus);
}
void DrumParams::saveParameters(juce::XmlElement& xml) {
    xml.setAttribute(NoteToPlay->paramID, NoteToPlay->get());
    xml.setAttribute(NoteToMuteEnabled->paramID, NoteToMuteEnabled->get());
    xml.setAttribute(NoteToMuteKind->paramID, NoteToMuteKind->getIndex());
    xml.setAttribute(NoteToMuteOct->paramID, NoteToMuteOct->getIndex());
    xml.setAttribute(Bus->paramID, Bus->getIndex());
}
void DrumParams::loadParameters(juce::XmlElement& xml) {
    *NoteToPlay = xml.getIntAttribute(NoteToPlay->paramID, 60);
    *NoteToMuteEnabled = xml.getIntAttribute(NoteToMuteEnabled->paramID, false);
    *NoteToMuteKind = xml.getIntAttribute(NoteToMuteKind->paramID, TARGET_NOTE_KINDS.indexOf("C"));
    *NoteToMuteOct = xml.getIntAttribute(NoteToMuteOct->paramID, TARGET_NOTE_OCT_NAMES.indexOf("1"));
    *Bus = xml.getIntAttribute(Bus->paramID, BUS_NAMES.indexOf("Bus"));
}

//==============================================================================
MainParams::MainParams(int groupIndex)
    : oscParams{OscParams{idPrefix(groupIndex), namePrefix(groupIndex), 0},
                OscParams{idPrefix(groupIndex), namePrefix(groupIndex), 1},
                OscParams{idPrefix(groupIndex), namePrefix(groupIndex), 2}},
      envelopeParams{EnvelopeParams{idPrefix(groupIndex), namePrefix(groupIndex), 0},
                     EnvelopeParams{idPrefix(groupIndex), namePrefix(groupIndex), 1}},
      filterParams{FilterParams{idPrefix(groupIndex), namePrefix(groupIndex), 0},
                   FilterParams{idPrefix(groupIndex), namePrefix(groupIndex), 1}},
      modEnvParams{ModEnvParams{idPrefix(groupIndex), namePrefix(groupIndex), 0},
                   ModEnvParams{idPrefix(groupIndex), namePrefix(groupIndex), 1},
                   ModEnvParams{idPrefix(groupIndex), namePrefix(groupIndex), 2}},
      delayParams{idPrefix(groupIndex), namePrefix(groupIndex)},
      drumParams{idPrefix(groupIndex), namePrefix(groupIndex)},
      masterParams{idPrefix(groupIndex), namePrefix(groupIndex)} {}
void MainParams::addAllParameters(juce::AudioProcessor& processor) {
    for (auto& params : envelopeParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : oscParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : filterParams) {
        params.addAllParameters(processor);
    }
    for (auto& params : modEnvParams) {
        params.addAllParameters(processor);
    }
    delayParams.addAllParameters(processor);
    drumParams.addAllParameters(processor);
    masterParams.addAllParameters(processor);
}
void MainParams::saveParameters(juce::XmlElement& xml) {
    for (auto& param : envelopeParams) {
        param.saveParameters(xml);
    }
    for (auto& param : oscParams) {
        param.saveParameters(xml);
    }
    for (auto& param : filterParams) {
        param.saveParameters(xml);
    }
    for (auto& param : modEnvParams) {
        param.saveParameters(xml);
    }
    delayParams.saveParameters(xml);
    drumParams.saveParameters(xml);
    masterParams.saveParameters(xml);
}
void MainParams::loadParameters(juce::XmlElement& xml) {
    for (auto& param : envelopeParams) {
        param.loadParameters(xml);
    }
    for (auto& param : oscParams) {
        param.loadParameters(xml);
    }
    for (auto& param : filterParams) {
        param.loadParameters(xml);
    }
    for (auto& param : modEnvParams) {
        param.loadParameters(xml);
    }
    delayParams.loadParameters(xml);
    drumParams.loadParameters(xml);
    masterParams.loadParameters(xml);
}

//==============================================================================
AllParams::AllParams() : globalParams{}, voiceParams{} {
    mainParamList.reserve(129);
    for (int i = 0; i < 129; i++) {
        mainParamList.push_back(MainParams{i});
    }
}
void AllParams::addAllParameters(juce::AudioProcessor& processor) {
    globalParams.addAllParameters(processor);
    voiceParams.addAllParameters(processor);
    for (int i = 0; i < 129; i++) {
        mainParamList[i].addAllParameters(processor);
    }
}
void AllParams::saveParameters(juce::XmlElement& xml) {
    globalParams.saveParameters(xml);
    voiceParams.saveParameters(xml);
    for (int i = 0; i < 129; i++) {
        auto enabled = mainParamList[i].isEnabled();
        xml.setAttribute(juce::String("MAIN_PARAMS_" + std::to_string(i) + "_ENABLED"), enabled);
        if (enabled) {
            mainParamList[i].saveParameters(xml);
        }
    }
}
void AllParams::loadParameters(juce::XmlElement& xml) {
    globalParams.loadParameters(xml);
    voiceParams.loadParameters(xml);
    for (int i = 0; i < 129; i++) {
        auto enabled = xml.getBoolAttribute(juce::String("MAIN_PARAMS_" + std::to_string(i) + "_ENABLED"), i == 128);
        if (enabled) {
            mainParamList[i].loadParameters(xml);
        }
    }
}
void replaceAttributeNames(juce::XmlElement& xml, juce::StringRef before, juce::StringRef after) {
    auto newAttrs = std::map<juce::String, juce::String>{};
    for (auto i = 0; i < xml.getNumAttributes(); i++) {
        auto attrName = xml.getAttributeName(i);
        attrName = attrName.replace(before, after);
        newAttrs[attrName] = xml.getAttributeValue(i);
    }
    xml.removeAllAttributes();
    for (auto const& [attrName, value] : newAttrs) {
        xml.setAttribute(attrName, value);
    }
}
void AllParams::saveParametersToClipboard(juce::XmlElement& xml) {
    auto index = voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128;
    xml.setAttribute("DRUM_MODE", voiceParams.isDrumMode());
    mainParamList[index].saveParameters(xml);

    replaceAttributeNames(xml, "G" + std::to_string(index) + "_", "GROUP_");
}
void AllParams::loadParametersFromClipboard(juce::XmlElement& xml) {
    auto index = voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128;

    replaceAttributeNames(xml, "GROUP_", "G" + std::to_string(index) + "_");

    auto wasDrumMode = xml.getBoolAttribute("DRUM_MODE", false);
    mainParamList[index].loadParameters(xml);
}