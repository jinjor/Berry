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
      oscComponents{
          SectionComponent{"1st", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(0, p.allParams)},
          SectionComponent{"2nd", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(1, p.allParams)},
          SectionComponent{"3rd", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(2, p.allParams)},
          SectionComponent{"4th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(3, p.allParams)},
          SectionComponent{"5th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(4, p.allParams)},
          SectionComponent{"6th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(5, p.allParams)},
          SectionComponent{"7th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(6, p.allParams)},
          SectionComponent{"8th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(7, p.allParams)},
          SectionComponent{"9th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(8, p.allParams)},
          SectionComponent{"10th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(9, p.allParams)},
          SectionComponent{"11th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(10, p.allParams)},
          SectionComponent{"12th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(11, p.allParams)},
          SectionComponent{"13th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(12, p.allParams)},
          SectionComponent{"14th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(13, p.allParams)},
          SectionComponent{"15th", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(14, p.allParams)},
          SectionComponent{"16th...", HEADER_CHECK::Hidden, std::make_unique<OscComponent>(15, p.allParams)},
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

    for (auto i = 0; i < NUM_OSC; i++) {
        auto &component = oscComponents[i];
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
    auto middleArea = bounds.removeFromTop(bounds.getHeight() * 4 / 5);

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
    auto middleHeight = (height - upperHeight) * 4 / 5;
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
        auto leftWidth = (width - PANEL_MARGIN_X * 2) * 0.5;

        auto middleHeight = middleArea.getHeight();
        auto halfPanelHeight = (middleHeight - PANEL_MARGIN_Y) * 0.5;
        auto quarterPanelHeight = (halfPanelHeight - PANEL_MARGIN_Y) * 0.5;
        auto oscPanelHeight = (quarterPanelHeight - PANEL_MARGIN_Y) * 0.5;
        {
            auto area = middleArea.removeFromLeft(leftWidth);
            oscComponents[0].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[1].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[2].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[3].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[4].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[5].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[6].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[7].setBounds(area);
        }
        middleArea.removeFromLeft(PANEL_MARGIN_X);
        {
            auto &area = middleArea;
            oscComponents[8].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[9].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[10].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[11].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[12].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[13].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[14].setBounds(area.removeFromTop(oscPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[15].setBounds(area);
        }
    }
    {
        auto lowerArea = bounds.reduced(AREA_PADDING_X, AREA_PADDING_Y);
        auto leftWidth = (width - PANEL_MARGIN_X * 2) * 0.6;

        auto lowerHeight = lowerArea.getHeight();
        auto filterPanelHeight = (lowerHeight - PANEL_MARGIN_Y * 2) / 2;
        auto delayPanelHeight = (lowerHeight - PANEL_MARGIN_Y * 2) / 2;
        auto masterPanelHeight = (lowerHeight - delayPanelHeight - PANEL_MARGIN_Y) * 1 / 3;
        lowerArea.removeFromLeft(PANEL_MARGIN_X);
        {
            // auto area = lowerArea.removeFromLeft(leftWidth);
            auto &area = lowerArea;
            noiseComponents[0].setBounds(area.removeFromTop(filterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            noiseComponents[1].setBounds(area.removeFromTop(filterPanelHeight));
        }
        // lowerArea.removeFromLeft(PANEL_MARGIN_X);
        // {
        //     auto &area = lowerArea;
        //     delayComponent.setBounds(area.removeFromTop(delayPanelHeight));
        //     area.removeFromTop(PANEL_MARGIN_Y);
        //     masterComponent.setBounds(area.removeFromTop(masterPanelHeight));
        // }
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