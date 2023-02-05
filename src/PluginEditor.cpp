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
      analyserWindow(&analyserMode, &p.latestDataProvider, p.allParams.voiceParams, p.allParams.mainParams),
      statusComponent(&p.polyphony, &p.timeConsumptionState, &p.latestDataProvider),
      utilComponent{SectionComponent{"UTILITY", HEADER_CHECK::Hidden, std::make_unique<UtilComponent>(p)}},
      oscComponents{
          SectionComponent{"OSC 1", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(0, p.allParams)},
          SectionComponent{"OSC 2", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(1, p.allParams)},
          SectionComponent{"OSC 3", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(2, p.allParams)},
          SectionComponent{"OSC 4", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(3, p.allParams)},
          SectionComponent{"OSC 5", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(4, p.allParams)},
          SectionComponent{"OSC 6", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(5, p.allParams)},
          SectionComponent{"OSC 7", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(6, p.allParams)},
          SectionComponent{"OSC 8", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(7, p.allParams)},
          SectionComponent{"OSC 9", HEADER_CHECK::Enabled, std::make_unique<OscComponent>(8, p.allParams)},
      },
      envelopeComponents{
          SectionComponent{"ENV 1", HEADER_CHECK::Hidden, std::make_unique<EnvelopeComponent>(0, p.allParams)},
          SectionComponent{"ENV 2", HEADER_CHECK::Hidden, std::make_unique<EnvelopeComponent>(1, p.allParams)},
          SectionComponent{"ENV 3", HEADER_CHECK::Hidden, std::make_unique<EnvelopeComponent>(2, p.allParams)}},
      filterComponents{
          SectionComponent{"FILTER 1", HEADER_CHECK::Enabled, std::make_unique<FilterComponent>(0, p.allParams)},
          SectionComponent{"FILTER 2", HEADER_CHECK::Enabled, std::make_unique<FilterComponent>(1, p.allParams)}},
      modEnvComponents{
          SectionComponent{"MOD ENV 1", HEADER_CHECK::Enabled, std::make_unique<ModEnvComponent>(0, p.allParams)},
          SectionComponent{"MOD ENV 2", HEADER_CHECK::Enabled, std::make_unique<ModEnvComponent>(1, p.allParams)},
          SectionComponent{"MOD ENV 3", HEADER_CHECK::Enabled, std::make_unique<ModEnvComponent>(2, p.allParams)}},
      delayComponent{SectionComponent{"DELAY", HEADER_CHECK::Enabled, std::make_unique<DelayComponent>(p.allParams)}},
      masterComponent{
          SectionComponent{"MASTER", HEADER_CHECK::Hidden, std::make_unique<MasterComponent>(p.allParams)}} {
    getLookAndFeel().setColour(juce::Label::textColourId, colour::TEXT);

    addAndMakeVisible(voiceComponent);
    addAndMakeVisible(analyserToggle);
    addAndMakeVisible(analyserWindow);
    addAndMakeVisible(statusComponent);
    addAndMakeVisible(utilComponent);
    for (auto i = 0; i < NUM_OSC; i++) {
        auto &params = audioProcessor.allParams.mainParams.oscParams[i];
        auto &component = oscComponents[i];
        component.setEnabled(params.Enabled->get());
        component.addListener(this);
        addAndMakeVisible(component);
    }
    for (auto i = 0; i < NUM_ENVELOPE; i++) {
        auto &component = envelopeComponents[i];
        component.addListener(this);
        addAndMakeVisible(component);
    }
    for (auto i = 0; i < NUM_FILTER; i++) {
        auto &params = audioProcessor.allParams.mainParams.filterParams[i];
        auto &component = filterComponents[i];
        component.setEnabled(params.Enabled->get());
        component.addListener(this);
        addAndMakeVisible(component);
    }
    for (auto i = 0; i < NUM_MODENV; i++) {
        auto &params = audioProcessor.allParams.mainParams.modEnvParams[i];
        auto &component = modEnvComponents[i];
        component.setEnabled(params.Enabled->get());
        component.addListener(this);
        addAndMakeVisible(component);
    }
    {
        auto &params = audioProcessor.allParams.mainParams.delayParams;
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
    auto middleArea = bounds.removeFromTop(bounds.getHeight() * 2 / 5);

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
    auto middleHeight = (height - upperHeight) * 2 / 5;
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
        auto leftWidth = (width - PANEL_MARGIN_X * 2) * 0.35;
        auto centreWidth = (width - PANEL_MARGIN_X * 2) * 0.35;

        auto middleHeight = middleArea.getHeight();
        auto halfPanelHeight = (middleHeight - PANEL_MARGIN_Y) * 0.5;
        auto quarterPanelHeight = (halfPanelHeight - PANEL_MARGIN_Y) * 0.5;
        {
            auto area = middleArea.removeFromLeft(leftWidth);
            envelopeComponents[0].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            envelopeComponents[1].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            envelopeComponents[2].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[0].setBounds(area);
        }
        middleArea.removeFromLeft(PANEL_MARGIN_X);
        {
            auto area = middleArea.removeFromLeft(centreWidth);
            oscComponents[1].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[2].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[3].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[4].setBounds(area);
        }
        middleArea.removeFromLeft(PANEL_MARGIN_X);
        {
            auto &area = middleArea;
            oscComponents[5].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[6].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[7].setBounds(area.removeFromTop(quarterPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            oscComponents[8].setBounds(area);

            // filterComponents[0].setBounds(area.removeFromTop(halfPanelHeight));
            // area.removeFromTop(PANEL_MARGIN_Y);
            // filterComponents[1].setBounds(area);
        }
    }
    {
        auto lowerArea = bounds.reduced(AREA_PADDING_X, AREA_PADDING_Y);
        auto leftWidth = (width - PANEL_MARGIN_X * 2) * 0.31;
        auto centreWidth = (width - PANEL_MARGIN_X * 2) * 0.31;

        auto lowerHeight = lowerArea.getHeight();
        auto modEnvPanelHeight = (lowerHeight - PANEL_MARGIN_Y * 2) / 3;
        auto delayPanelHeight = (lowerHeight - PANEL_MARGIN_Y * 2) / 3;
        auto masterPanelHeight = (lowerHeight - delayPanelHeight - PANEL_MARGIN_Y) * 1 / 3;
        lowerArea.removeFromLeft(PANEL_MARGIN_X);
        {
            auto area = lowerArea.removeFromLeft(leftWidth);
            modEnvComponents[0].setBounds(area.removeFromTop(modEnvPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            modEnvComponents[1].setBounds(area.removeFromTop(modEnvPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            modEnvComponents[2].setBounds(area.removeFromTop(modEnvPanelHeight));
        }
        lowerArea.removeFromLeft(PANEL_MARGIN_X);
        {
            auto &area = lowerArea;
            delayComponent.setBounds(area.removeFromTop(delayPanelHeight));
            area.removeFromTop(PANEL_MARGIN_Y);
            masterComponent.setBounds(area.removeFromTop(masterPanelHeight));
        }
    }
}
void BerryAudioProcessorEditor::timerCallback() {
    auto &mainParams = audioProcessor.allParams.mainParams;
    for (auto i = 0; i < NUM_OSC; i++) {
        oscComponents[i].setEnabled(mainParams.oscParams[i].Enabled->get());
    }
    for (auto i = 0; i < NUM_FILTER; i++) {
        filterComponents[i].setEnabled(mainParams.filterParams[i].Enabled->get());
    }
    for (auto i = 0; i < NUM_MODENV; i++) {
        modEnvComponents[i].setEnabled(mainParams.modEnvParams[i].Enabled->get());
    }
    delayComponent.setEnabled(mainParams.delayParams.Enabled->get());
}
void BerryAudioProcessorEditor::enabledChanged(SectionComponent *section) {
    for (auto i = 0; i < NUM_OSC; i++) {
        if (&oscComponents[i] == section) {
            auto &params = audioProcessor.allParams.mainParams.oscParams[i];
            *params.Enabled = section->getEnabled();
            return;
        }
    }
    for (auto i = 0; i < NUM_FILTER; i++) {
        if (&filterComponents[i] == section) {
            auto &params = audioProcessor.allParams.mainParams.filterParams[i];
            *params.Enabled = section->getEnabled();
            return;
        }
    }
    for (auto i = 0; i < NUM_MODENV; i++) {
        if (&modEnvComponents[i] == section) {
            auto &params = audioProcessor.allParams.mainParams.modEnvParams[i];
            *params.Enabled = section->getEnabled();
            return;
        }
    }
    if (&delayComponent == section) {
        auto &params = audioProcessor.allParams.mainParams.delayParams;
        *params.Enabled = section->getEnabled();
    }
}