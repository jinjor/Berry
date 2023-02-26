#include "PluginEditor.h"

#include "Params.h"
#include "PluginProcessor.h"
#include "StyleConstants.h"
#include "Voice.h"

using namespace styles;

//==============================================================================
BerryAudioProcessorEditor::BerryAudioProcessorEditor(BerryAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      voiceComponent{SectionComponent{"VOICE", HEADER_CHECK::Hidden, std::make_unique<VoiceComponent>(p.allParams)}},
      analyserToggle(&analyserMode),
      analyserWindow(&analyserMode, &p.latestDataProvider),
      statusComponent(&p.polyphony, &p.timeConsumptionState, &p.latestDataProvider),
      utilComponent{SectionComponent{"UTILITY", HEADER_CHECK::Hidden, std::make_unique<UtilComponent>(p)}},
      timbreComponent{SectionComponent{
          "TIMBRE", HEADER_CHECK::Hidden, std::make_unique<KeyboardComponent>(p.allParams, p.keyboardState)}},
      harmonicHeadComponent{SectionComponent{"", HEADER_CHECK::Hidden, std::make_unique<HarmonicHeadComponent>()}},
      harmonicBodyComponent{
          SectionComponent{"HARMONICS", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(p.allParams)}},
      noiseComponents{
          SectionComponent{"NOISE 1", HEADER_CHECK::Hidden, std::make_unique<NoiseComponent>(0, p.allParams)},
          SectionComponent{"NOISE 2", HEADER_CHECK::Hidden, std::make_unique<NoiseComponent>(1, p.allParams)}},
      delayComponent{SectionComponent{"DELAY", HEADER_CHECK::Enabled, std::make_unique<DelayComponent>(p.allParams)}},
      masterComponent{
          SectionComponent{"MASTER", HEADER_CHECK::Hidden, std::make_unique<MasterComponent>(p.allParams)}} {
    getLookAndFeel().setColour(juce::Label::textColourId, colour::TEXT);

    addAndMakeVisible(voiceComponent);
    addAndMakeVisible(analyserToggle);
    addAndMakeVisible(analyserWindow);
    addAndMakeVisible(statusComponent);
    addAndMakeVisible(utilComponent);
    addAndMakeVisible(timbreComponent);

    addAndMakeVisible(harmonicHeadComponent);
    addAndMakeVisible(harmonicBodyComponent);
    for (auto i = 0; i < NUM_NOISE; i++) {
        auto &component = noiseComponents[i];
        addAndMakeVisible(component);
    }
    {
        auto &params = audioProcessor.allParams.delayParams;
        delayComponent.setEnabled(params.Enabled->get());
        delayComponent.addListener(this);
        addAndMakeVisible(delayComponent);
    }
    addAndMakeVisible(masterComponent);
    setSize(1024, 768);
#if JUCE_DEBUG
    setResizable(true, true);  // for debug
#endif
    startTimerHz(30.0f);
}

BerryAudioProcessorEditor::~BerryAudioProcessorEditor() {}

//==============================================================================
void BerryAudioProcessorEditor::paint(juce::Graphics &g) {
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();
    auto upperArea = bounds.removeFromTop(height * 0.12);
    auto middleArea = bounds.removeFromTop(bounds.getHeight() * 1 / 5);

    g.fillAll(colour::BACKGROUND);

    auto areas = std::array{upperArea, middleArea};
    for (auto &area : areas) {
        {
            juce::Path p;
            p.addLineSegment(juce::Line<float>(0, area.getBottom() - 0.5, area.getWidth(), area.getBottom() - 0.5), 0);
            g.setColour(juce::Colour(10, 10, 10));
            g.strokePath(p, juce::PathStrokeType(1));
        }
        {
            juce::Path p;
            p.addLineSegment(juce::Line<float>(0, area.getBottom() + 0.5, area.getWidth(), area.getBottom() + 0.5), 0);
            g.setColour(juce::Colour(60, 60, 60));
            g.strokePath(p, juce::PathStrokeType(1));
        }
    }
}

void BerryAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    auto upperHeight = height * 0.12;
    auto middleHeight = (height - upperHeight) * 1 / 5;
    {
        auto upperArea = bounds.removeFromTop(upperHeight).reduced(AREA_PADDING_X, AREA_PADDING_Y);
        auto sideWidth = width * 0.36;
        auto centreWidth = width - sideWidth * 2;

        auto voiceWidth = sideWidth * 2 / 3;
        auto analyserToggleWidth = sideWidth * 1 / 3;
        auto statusWidth = sideWidth * 0.5;

        voiceComponent.setBounds(upperArea.removeFromLeft(voiceWidth));
        analyserToggle.setBounds(upperArea.removeFromLeft(analyserToggleWidth).reduced(3));
        analyserWindow.setBounds(upperArea.removeFromLeft(centreWidth).reduced(3));
        statusComponent.setBounds(upperArea.removeFromLeft(statusWidth));
        utilComponent.setBounds(upperArea);
    }
    {
        auto middleArea = bounds.removeFromTop(middleHeight).reduced(AREA_PADDING_X, AREA_PADDING_Y);
        timbreComponent.setBounds(middleArea);
    }
    {
        auto lowerArea = bounds.reduced(AREA_PADDING_X, AREA_PADDING_Y);

        auto leftWidth = (width - PANEL_MARGIN_X * 2) * 0.57;
        {
            auto area = lowerArea.removeFromLeft(leftWidth);
            harmonicHeadComponent.setBounds(area.removeFromTop(LABEL_HEIGHT));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponent.setBounds(area);
        }
        {
            auto &area = lowerArea;
            auto noisePanelHeight = (area.getHeight() - PANEL_MARGIN_Y) / 2;
            noiseComponents[0].setBounds(area.removeFromTop(noisePanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            noiseComponents[1].setBounds(area.removeFromTop(noisePanelHeight));
        }
    }
}
void BerryAudioProcessorEditor::timerCallback() {
    auto &mainParams = audioProcessor.allParams.getCurrentMainParams();
    delayComponent.setEnabled(audioProcessor.allParams.delayParams.Enabled->get());
}
void BerryAudioProcessorEditor::enabledChanged(SectionComponent *section) {
    if (&delayComponent == section) {
        auto &params = audioProcessor.allParams.delayParams;
        *params.Enabled = section->getEnabled();
    }
}