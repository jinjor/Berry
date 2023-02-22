#pragma once

#include <JuceHeader.h>

#include "Components.h"
#include "PluginProcessor.h"
#include "Voice.h"

//==============================================================================
class BerryAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer, SectionComponent::Listener {
public:
    BerryAudioProcessorEditor(BerryAudioProcessor &);
    ~BerryAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    BerryAudioProcessor &audioProcessor;
    ANALYSER_MODE analyserMode = ANALYSER_MODE::Spectrum;

    SectionComponent voiceComponent;
    AnalyserToggle analyserToggle;
    AnalyserWindow analyserWindow;
    StatusComponent statusComponent;
    SectionComponent utilComponent;
    SectionComponent oscComponents[NUM_OSC];
    SectionComponent noiseComponents[NUM_NOISE];
    SectionComponent delayComponent;
    SectionComponent masterComponent;

    virtual void timerCallback() override;
    virtual void enabledChanged(SectionComponent *section) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BerryAudioProcessorEditor)
};
