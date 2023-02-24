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
      harmonicHeadComponent{SectionComponent{"", HEADER_CHECK::Hidden, std::make_unique<HarmonicHeadComponent>()}},
      harmonicBodyComponents{
          SectionComponent{"1", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(0, p.allParams)},
          SectionComponent{"2", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(1, p.allParams)},
          SectionComponent{"3", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(2, p.allParams)},
          SectionComponent{"4", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(3, p.allParams)},
          SectionComponent{"5", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(4, p.allParams)},
          SectionComponent{"6", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(5, p.allParams)},
          SectionComponent{"7", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(6, p.allParams)},
          SectionComponent{"8", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(7, p.allParams)},
          SectionComponent{"9", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(8, p.allParams)},
          SectionComponent{"10", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(9, p.allParams)},
          SectionComponent{"11", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(10, p.allParams)},
          SectionComponent{"12", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(11, p.allParams)},
          SectionComponent{"13", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(12, p.allParams)},
          SectionComponent{"14", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(13, p.allParams)},
          SectionComponent{"15", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(14, p.allParams)},
          SectionComponent{"16..", HEADER_CHECK::Hidden, std::make_unique<HarmonicBodyComponent>(15, p.allParams)},
      },
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
    auto &mainParams = audioProcessor.allParams.getCurrentMainParams();

    addAndMakeVisible(harmonicHeadComponent);
    for (auto i = 0; i < NUM_OSC; i++) {
        auto &component = harmonicBodyComponents[i];
        addAndMakeVisible(component);
    }
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

        auto lowerArea = bounds.reduced(AREA_PADDING_X, AREA_PADDING_Y);

        auto leftWidth = (width - PANEL_MARGIN_X * 2) * 0.5;
        {
            auto area = lowerArea.removeFromLeft(leftWidth);
            harmonicHeadComponent.setBounds(area.removeFromTop(LABEL_HEIGHT));
            area.removeFromTop(PANEL_MARGIN_Y);

            auto harmonicPanelHeight = (area.getHeight() - PANEL_MARGIN_Y * (NUM_OSC - 1)) / NUM_OSC;
            harmonicBodyComponents[0].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[1].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[2].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[3].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[4].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[5].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[6].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[7].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[8].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[9].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[10].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[11].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[12].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[13].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[14].setBounds(area.removeFromTop(harmonicPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            harmonicBodyComponents[15].setBounds(area);
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