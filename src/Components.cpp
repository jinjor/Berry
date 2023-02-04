#include "Components.h"

#include <JuceHeader.h>

#include <utility>

#include "Params.h"

//==============================================================================
float calcCurrentLevel(int numSamples, float* data) {
    float maxValue = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        maxValue = std::max(maxValue, std::abs(data[i]));
    }
    return juce::Decibels::gainToDecibels(maxValue);
}

//==============================================================================
HeaderComponent::HeaderComponent(std::string name, HEADER_CHECK check)
    : enabledButton("Enabled"), name(std::move(name)), check(check) {
    if (check != HEADER_CHECK::Hidden) {
        addAndMakeVisible(enabledButton);
        enabledButton.setEnabled(check == HEADER_CHECK::Enabled);
    }
}
HeaderComponent::~HeaderComponent() {}
void HeaderComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.setColour(colour::PANEL_NAME_BACKGROUND);
    g.fillRect(bounds);

    juce::GlyphArrangement ga;
    juce::Font font = juce::Font(PANEL_NAME_FONT_SIZE, juce::Font::plain).withTypefaceStyle("Bold");
    ga.addFittedText(font, name, 0, 0, bounds.getHeight(), bounds.getWidth(), juce::Justification::right, 1);
    juce::Path p;
    ga.createPath(p);
    auto pathBounds = ga.getBoundingBox(0, name.length(), true);
    p.applyTransform(
        juce::AffineTransform()
            .rotated(-juce::MathConstants<float>::halfPi, 0, 0)
            .translated(0, bounds.getHeight() + (check == HEADER_CHECK::Hidden ? 8.0 : PANEL_NAME_HEIGHT) + 1.0));
    g.setColour(colour::TEXT);
    g.fillPath(p);
}
void HeaderComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto enabledButtonArea = bounds.removeFromTop(bounds.getWidth());
    enabledButton.setBounds(enabledButtonArea.reduced(6));
}

//==============================================================================
SectionComponent::SectionComponent(std::string name, HEADER_CHECK check, std::unique_ptr<juce::Component> _body)
    : header(std::move(name), check), body(std::move(_body)) {
    header.enabledButton.setLookAndFeel(&berryLookAndFeel);
    header.enabledButton.addListener(this);
    addAndMakeVisible(header);
    addAndMakeVisible(*body);
}
SectionComponent::~SectionComponent() {}
void SectionComponent::paint(juce::Graphics& g) {}
void SectionComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto headerArea = bounds.removeFromLeft(PANEL_NAME_HEIGHT);
    header.setBounds(headerArea);
    body->setBounds(bounds);
}
void SectionComponent::addListener(Listener* newListener) { listeners.add(newListener); }
void SectionComponent::setEnabled(bool enabled) {
    header.enabledButton.setToggleState(enabled, juce::dontSendNotification);
    body->setEnabled(enabled);
}
bool SectionComponent::getEnabled() { return header.enabledButton.getToggleState(); }
void SectionComponent::buttonClicked(juce::Button* button) {
    if (button == &header.enabledButton) {
        body->setEnabled(getEnabled());
        listeners.call([this](Listener& l) { l.enabledChanged(this); });
    }
}

//==============================================================================
ArrowButton2::ArrowButton2(const String& name, float arrowDirectionInRadians, Colour arrowColour)
    : Button(name), colour(arrowColour) {
    path.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform(AffineTransform::rotation(MathConstants<float>::twoPi * arrowDirectionInRadians, 0.5f, 0.5f));
}

ArrowButton2::~ArrowButton2() {}

void ArrowButton2::paintButton(Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown) {
    auto bounds = getLocalBounds();
    auto enabled = this->isEnabled();

    // g.setColour(colour::PIT);
    // g.fillRect(bounds);

    Path p(path);

    const float offset = shouldDrawButtonAsDown ? 1.0f : 0.0f;
    auto width = (float)getWidth();
    auto height = (float)getHeight();
    auto x = width / 2 + offset;
    auto y = height / 2 + offset;
    p.applyTransform(path.getTransformToScaleToFit(x - 4, y - 2, 8, 4, false));

    DropShadow(Colours::black.withAlpha(0.3f), shouldDrawButtonAsDown ? 2 : 4, Point<int>()).drawForPath(g, p);
    g.setColour(colour.withAlpha(enabled ? 1.0f : 0.3f));
    g.fillPath(p);
}

//==============================================================================
IncDecButton::IncDecButton()
    : incButton("Inc Button", 0.75, colour::TEXT),
      decButton("Dec Button", 0.25, colour::TEXT),
      slider(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {
    label.setColour(juce::Label::textColourId, colour::TEXT);
    label.setText(juce::String(value), juce::dontSendNotification);
    label.setJustificationType(Justification::centred);
    this->addAndMakeVisible(label);

    incButton.setEnabled(value < max);
    this->addAndMakeVisible(incButton);
    incButton.addListener(this);

    decButton.setEnabled(min < value);
    this->addAndMakeVisible(decButton);
    decButton.addListener(this);

    slider.setRange(min, max, 1);
    slider.setValue(value);
    this->addAndMakeVisible(slider);
    slider.addListener(this);
}
IncDecButton::~IncDecButton() {}
void IncDecButton::paint(juce::Graphics& g) {}
void IncDecButton::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto incButtonArea = bounds.removeFromTop(12);  // TODO もう少し範囲を広げたい
    incButton.setBounds(incButtonArea);
    auto decButtonArea = bounds.removeFromBottom(12);  // TODO
    decButton.setBounds(decButtonArea);
    label.setBounds(bounds);
    slider.setBounds(bounds);
}
void IncDecButton::setRange(int min, int max) {
    jassert(min < max);
    this->min = min;
    this->max = max;
    incButton.setEnabled(value < max);
    decButton.setEnabled(min < value);
    slider.setRange(min, max, 1);
}
void IncDecButton::setValue(int newValue, NotificationType notification) {
    jassert(min <= newValue);
    jassert(newValue <= max);
    value = newValue;
    label.setText(juce::String(newValue), notification);
    incButton.setEnabled(value < max);
    decButton.setEnabled(min < value);
    slider.setValue(value);
}
int IncDecButton::getValue() { return value; }
void IncDecButton::addListener(IncDecButton::Listener* l) { listeners.add(l); }
void IncDecButton::removeListener(IncDecButton::Listener* l) { listeners.remove(l); }
void IncDecButton::buttonClicked(juce::Button* button) {
    if (button == &incButton) {
        value++;
    } else if (button == &decButton) {
        value--;
    }
    incButton.setEnabled(value < max);
    decButton.setEnabled(min < value);
    slider.setValue(value);
    listeners.call([this](Listener& l) { l.incDecValueChanged(this); });
}
void IncDecButton::sliderValueChanged(juce::Slider* _slider) {
    if (_slider == &slider) {
        value = slider.getValue();
        incButton.setEnabled(value < max);
        decButton.setEnabled(min < value);
        listeners.call([this](Listener& l) { l.incDecValueChanged(this); });
    }
}

//==============================================================================
VoiceComponent::VoiceComponent(AllParams& allParams)
    : params(allParams.voiceParams),
      mainParamList(allParams.mainParamList),
      controlItemParams(allParams.controlItemParams),
      modeSelector("Mode"),
      portamentoTimeSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                           juce::Slider::TextEntryBoxPosition::NoTextBox),
      pitchBendRangeButton() {
    initChoice(modeSelector, params.Mode, this, *this);
    initLinear(portamentoTimeSlider, params.PortamentoTime, 0.001, " sec", nullptr, this, *this);
    initIncDec(pitchBendRangeButton, params.PitchBendRange, this, *this);
    initChoice(targetNoteKindSelector, params.TargetNoteKind, this, drumTargetSelector);
    initChoice(targetNoteOctSelector, params.TargetNoteOct, this, drumTargetSelector);
    initLabel(modeLabel, "Mode", *this);
    initLabel(portamentoTimeLabel, "Glide Time", *this);
    initLabel(pitchBendRangeLabel, "PB Range", *this);
    initLabel(targetNoteLabel, "Target Note", *this);

    addAndMakeVisible(drumTargetSelector);

    startTimerHz(30.0f);
}

VoiceComponent::~VoiceComponent() {}

void VoiceComponent::paint(juce::Graphics& g) {}

void VoiceComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.reduce(0, 10);
    consumeLabeledComboBox(bounds, 70, modeLabel, modeSelector);
    auto drumBounds = bounds;
    consumeLabeledComboBox(drumBounds, 120, targetNoteLabel, drumTargetSelector);
    {
        juce::Rectangle<int> selectorsArea = drumTargetSelector.getLocalBounds();
        targetNoteKindSelector.setBounds(selectorsArea.removeFromLeft(60));
        targetNoteOctSelector.setBounds(selectorsArea.removeFromLeft(60));
    }
    consumeLabeledKnob(bounds, portamentoTimeLabel, portamentoTimeSlider);
    consumeLabeledIncDecButton(bounds, 60, pitchBendRangeLabel, pitchBendRangeButton);
}
void VoiceComponent::comboBoxChanged(juce::ComboBox* comboBox) {
    if (comboBox == &modeSelector) {
        *params.Mode = modeSelector.getSelectedItemIndex();
    } else if (comboBox == &targetNoteKindSelector) {
        *params.TargetNoteKind = targetNoteKindSelector.getSelectedItemIndex();
    } else if (comboBox == &targetNoteOctSelector) {
        *params.TargetNoteOct = targetNoteOctSelector.getSelectedItemIndex();
    }
}
void VoiceComponent::sliderValueChanged(juce::Slider* slider) {
    if (slider == &portamentoTimeSlider) {
        *params.PortamentoTime = portamentoTimeSlider.getValue();
    }
}
void VoiceComponent::incDecValueChanged(IncDecButton* button) {
    if (button == &pitchBendRangeButton) {
        *params.PitchBendRange = pitchBendRangeButton.getValue();
    }
}
void VoiceComponent::timerCallback() {
    modeSelector.setSelectedItemIndex(params.Mode->getIndex(), juce::dontSendNotification);
    portamentoTimeSlider.setValue(params.PortamentoTime->get(), juce::dontSendNotification);
    pitchBendRangeButton.setValue(params.PitchBendRange->get(), juce::dontSendNotification);
    targetNoteKindSelector.setSelectedItemIndex(params.TargetNoteKind->getIndex(), juce::dontSendNotification);
    targetNoteOctSelector.setSelectedItemIndex(params.TargetNoteOct->getIndex(), juce::dontSendNotification);

    for (auto octIndex = 0; octIndex < 7; octIndex++) {
        auto oct = TARGET_NOTE_OCT_VALUES[octIndex];
        auto cNote = (oct + 2) * 12;
        auto isOctSelected = TARGET_NOTE_OCT_VALUES[params.TargetNoteOct->getIndex()] == oct;
        auto isAnyNoteInOctEnabled = false;
        for (auto kindIndex = 0; kindIndex < 12; kindIndex++) {
            auto note = cNote + kindIndex;
            auto isNoteEnabled = mainParamList[note].isEnabled();
            isAnyNoteInOctEnabled |= isNoteEnabled;
            if (isOctSelected) {
                auto kindName = TARGET_NOTE_KINDS[kindIndex];
                kindName = (isNoteEnabled ? "*" : " ") + kindName;
                targetNoteKindSelector.changeItemText(kindIndex + 1, kindName);
            }
        }
        auto octName = juce::String(oct);
        octName = (isAnyNoteInOctEnabled ? "*" : " ") + octName;
        targetNoteOctSelector.changeItemText(octIndex + 1, octName);
    }

    auto isMono = params.isMonoMode();
    portamentoTimeLabel.setEnabled(isMono);
    portamentoTimeSlider.setEnabled(isMono);

    auto isDrum = params.isDrumMode();
    portamentoTimeLabel.setVisible(!isDrum);
    portamentoTimeSlider.setVisible(!isDrum);
    pitchBendRangeLabel.setVisible(!isDrum);
    pitchBendRangeButton.setVisible(!isDrum);
    targetNoteLabel.setVisible(isDrum);
    drumTargetSelector.setVisible(isDrum);

    portamentoTimeSlider.setLookAndFeel(&berryLookAndFeel);
    for (auto& p : controlItemParams) {
        if (p.isControlling(CONTROL_TARGET_MISC_PARAM::PortamentoTime)) {
            portamentoTimeSlider.setLookAndFeel(&berryLookAndFeelControlled);
        }
    }
}

//==============================================================================
UtilComponent::UtilComponent(BerryAudioProcessor& processor)
    : processor(processor), copyToClipboardButton(), pasteFromClipboardButton() {
    copyToClipboardButton.setLookAndFeel(&berryLookAndFeel);
    copyToClipboardButton.setButtonText("Copy");
    copyToClipboardButton.addListener(this);
    copyToClipboardButton.setLookAndFeel(&berryLookAndFeel);
    this->addAndMakeVisible(copyToClipboardButton);

    pasteFromClipboardButton.setLookAndFeel(&berryLookAndFeel);
    pasteFromClipboardButton.setButtonText("Paste");
    pasteFromClipboardButton.addListener(this);
    pasteFromClipboardButton.setLookAndFeel(&berryLookAndFeel);
    this->addAndMakeVisible(pasteFromClipboardButton);

    initLabel(copyToClipboardLabel, "Copy", *this);
    initLabel(pasteFromClipboardLabel, "Paste", *this);
}

UtilComponent::~UtilComponent() {}

void UtilComponent::paint(juce::Graphics& g) {}

void UtilComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.reduce(0, 10);
    consumeLabeledComboBox(bounds, 60, copyToClipboardLabel, copyToClipboardButton);
    consumeLabeledComboBox(bounds, 60, pasteFromClipboardLabel, pasteFromClipboardButton);
}
void UtilComponent::buttonClicked(juce::Button* button) {
    if (button == &copyToClipboardButton) {
        processor.copyToClipboard();
    } else if (button == &pasteFromClipboardButton) {
        processor.pasteFromClipboard();
    }
}

//==============================================================================
StatusComponent::StatusComponent(int* polyphony,
                                 TimeConsumptionState* timeConsumptionState,
                                 LatestDataProvider* latestDataProvider)
    : polyphony(polyphony), timeConsumptionState(timeConsumptionState), latestDataProvider(latestDataProvider) {
    latestDataProvider->addConsumer(&levelConsumer);

    initStatusValue(volumeValueLabel, "0.0dB", *this);
    initStatusValue(polyphonyValueLabel, std::to_string(*polyphony), *this);
    initStatusValue(timeConsumptionValueLabel,
                    std::to_string(juce::roundToInt(timeConsumptionState->currentTimeConsumptionRate * 100)) + "%",
                    *this);

    initStatusKey(volumeLabel, "Peak", *this);
    initStatusKey(polyphonyLabel, "Polyphony", *this);
    initStatusKey(timeConsumptionLabel, "Busyness", *this);

    startTimerHz(4.0f);
}

StatusComponent::~StatusComponent() { latestDataProvider->removeConsumer(&levelConsumer); }

void StatusComponent::paint(juce::Graphics& g) {}

void StatusComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.reduce(0, 10);
    auto boundsHeight = bounds.getHeight();
    auto boundsWidth = bounds.getWidth();
    consumeKeyValueText(bounds, boundsHeight / 3, boundsWidth * 0.4, volumeLabel, volumeValueLabel);
    consumeKeyValueText(bounds, boundsHeight / 3, boundsWidth * 0.4, polyphonyLabel, polyphonyValueLabel);
    consumeKeyValueText(bounds, boundsHeight / 3, boundsWidth * 0.4, timeConsumptionLabel, timeConsumptionValueLabel);
}
void StatusComponent::timerCallback() {
    if (overflowWarning > 0) {
        volumeValueLabel.setColour(juce::Label::textColourId, colour::ERROR);
        auto levelStr = juce::String(overflowedLevel, 1) + " dB";
        volumeValueLabel.setText(levelStr, juce::dontSendNotification);

        overflowWarning--;
    } else {
        volumeValueLabel.removeColour(juce::Label::textColourId);

        if (levelConsumer.ready) {
            float levelLdB = calcCurrentLevel(levelConsumer.numSamples, levelConsumer.destinationL);
            float levelRdB = calcCurrentLevel(levelConsumer.numSamples, levelConsumer.destinationR);
            auto leveldB = std::max(levelLdB, levelRdB);
            auto levelStr = (leveldB <= -100 ? "-Inf" : juce::String(leveldB, 1)) + " dB";
            volumeValueLabel.setText(levelStr, juce::dontSendNotification);
            levelConsumer.ready = false;
            if (leveldB > 0) {
                overflowedLevel = leveldB;
                overflowWarning = 4 * 1.2;
            }
        }
    }
    {
        int numVoices = 64;  // TODO: share this
        int value = *polyphony;
        polyphonyValueLabel.setText(juce::String(value), juce::dontSendNotification);
        if (value >= numVoices) {
            polyphonyValueLabel.setColour(juce::Label::textColourId, colour::ERROR);
        } else if (value > numVoices * 0.8) {
            polyphonyValueLabel.setColour(juce::Label::textColourId, colour::WARNING);
        } else {
            polyphonyValueLabel.setColour(juce::Label::textColourId, colour::TEXT);
        }
    }
    {
        auto value = timeConsumptionState->currentTimeConsumptionRate;
        timeConsumptionValueLabel.setText(juce::String(juce::roundToInt(value * 100)) + "%",
                                          juce::dontSendNotification);
        if (value >= 1.0) {
            timeConsumptionValueLabel.setColour(juce::Label::textColourId, colour::ERROR);
        } else if (value >= 0.8) {
            timeConsumptionValueLabel.setColour(juce::Label::textColourId, colour::WARNING);
        } else {
            timeConsumptionValueLabel.setColour(juce::Label::textColourId, colour::TEXT);
        }
    }
}

//==============================================================================
MasterComponent::MasterComponent(AllParams& allParams)
    : allParams(allParams),
      panSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
      volumeSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedOscParams();
    initLinear(panSlider, params.Pan, 0.01, this, *this);
    initLinear(volumeSlider, params.MasterVolume, 0.01, this, *this);
    initLabel(panLabel, "Pan", *this);
    initLabel(volumeLabel, "Volume", *this);

    startTimerHz(30.0f);
}

MasterComponent::~MasterComponent() {}

void MasterComponent::paint(juce::Graphics& g) {}

void MasterComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds = bounds.removeFromTop(bounds.getHeight() * 3 / 4);

    consumeLabeledKnob(bounds, panLabel, panSlider);
    consumeLabeledKnob(bounds, volumeLabel, volumeSlider);
}
void MasterComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedOscParams();
    if (slider == &panSlider) {
        *params.Pan = (float)panSlider.getValue();
    } else if (slider == &volumeSlider) {
        *params.MasterVolume = (float)volumeSlider.getValue();
    }
}
void MasterComponent::timerCallback() {
    auto& params = getSelectedOscParams();
    panSlider.setValue(params.Pan->get(), juce::dontSendNotification);
    volumeSlider.setValue(params.MasterVolume->get(), juce::dontSendNotification);
}

//==============================================================================
OscComponent::OscComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      envelopeSelector("Envelope"),
      waveformSelector("Waveform"),
      edgeSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
      octaveButton(),
      semitoneButton(),
      unisonButton(),
      detuneSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox),
      spreadSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox),
      gainSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedOscParams();

    initChoice(envelopeSelector, params.Envelope, this, *this);
    initChoice(waveformSelector, params.Waveform, this, *this);
    initLinear(edgeSlider, params.Edge, 0.01, this, *this);
    initIncDec(octaveButton, params.Octave, this, *this);
    initIncDec(semitoneButton, params.Coarse, this, *this);
    initIncDec(unisonButton, params.Unison, this, *this);
    initLinear(detuneSlider, params.Detune, 0.01, this, *this);
    initLinear(spreadSlider, params.Spread, 0.01, this, *this);
    auto formatGain = [](double gain) { return juce::String(juce::Decibels::gainToDecibels(gain), 2) + " dB"; };
    initSkewFromMid(gainSlider, params.Gain, 0.01f, nullptr, std::move(formatGain), this, *this);
    initLabel(envelopeLabel, "Env", *this);
    initLabel(waveformLabel, "Waveform", *this);
    initLabel(edgeLabel, "Edge", *this);
    initLabel(octaveLabel, "Oct", *this);
    initLabel(coarseLabel, "Semi", *this);
    initLabel(unisonLabel, "Unis", *this);
    initLabel(detuneLabel, "Detune", *this);
    initLabel(spreadLabel, "Spread", *this);
    initLabel(gainLabel, "Gain", *this);

    startTimerHz(30.0f);
}

OscComponent::~OscComponent() {}

void OscComponent::paint(juce::Graphics& g) {}

void OscComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto bodyHeight = bounds.getHeight();
    auto upperArea = bounds.removeFromTop(bodyHeight / 2);
    auto& lowerArea = bounds;
    consumeLabeledComboBox(upperArea, 60, envelopeLabel, envelopeSelector);
    consumeLabeledComboBox(upperArea, 105, waveformLabel, waveformSelector);
    consumeLabeledKnob(upperArea, edgeLabel, edgeSlider);
    consumeLabeledKnob(upperArea, gainLabel, gainSlider);
    consumeLabeledIncDecButton(lowerArea, 35, octaveLabel, octaveButton);
    consumeLabeledIncDecButton(lowerArea, 35, coarseLabel, semitoneButton);
    consumeLabeledIncDecButton(lowerArea, 35, unisonLabel, unisonButton);
    consumeLabeledKnob(lowerArea, detuneLabel, detuneSlider);
    consumeLabeledKnob(lowerArea, spreadLabel, spreadSlider);
}
void OscComponent::comboBoxChanged(juce::ComboBox* comboBox) {
    auto& params = getSelectedOscParams();
    if (comboBox == &envelopeSelector) {
        *params.Envelope = envelopeSelector.getSelectedItemIndex();
    } else if (comboBox == &waveformSelector) {
        *params.Waveform = waveformSelector.getSelectedItemIndex();
    }
}
void OscComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedOscParams();
    if (slider == &edgeSlider) {
        *params.Edge = edgeSlider.getValue();
    } else if (slider == &detuneSlider) {
        *params.Detune = (float)detuneSlider.getValue();
    } else if (slider == &spreadSlider) {
        *params.Spread = (float)spreadSlider.getValue();
    } else if (slider == &gainSlider) {
        *params.Gain = (float)gainSlider.getValue();
    }
}
void OscComponent::incDecValueChanged(IncDecButton* button) {
    auto& params = getSelectedOscParams();
    if (button == &octaveButton) {
        *params.Octave = octaveButton.getValue();
    } else if (button == &semitoneButton) {
        *params.Coarse = semitoneButton.getValue();
    } else if (button == &unisonButton) {
        *params.Unison = unisonButton.getValue();
    }
}
void OscComponent::timerCallback() {
    auto& params = getSelectedOscParams();
    envelopeSelector.setSelectedItemIndex(params.Envelope->getIndex(), juce::dontSendNotification);
    waveformSelector.setSelectedItemIndex(params.Waveform->getIndex(), juce::dontSendNotification);
    edgeSlider.setValue(params.Edge->get(), juce::dontSendNotification);
    octaveButton.setValue(params.Octave->get(), juce::dontSendNotification);
    semitoneButton.setValue(params.Coarse->get(), juce::dontSendNotification);
    unisonButton.setValue(params.Unison->get(), juce::dontSendNotification);
    detuneSlider.setValue(params.Detune->get(), juce::dontSendNotification);
    spreadSlider.setValue(params.Spread->get(), juce::dontSendNotification);
    gainSlider.setValue(params.Gain->get(), juce::dontSendNotification);

    auto hasEdge = params.hasEdge();
    edgeLabel.setEnabled(hasEdge);
    edgeSlider.setEnabled(hasEdge);

    auto isNoise = params.isNoise();
    unisonLabel.setEnabled(!isNoise);
    unisonButton.setEnabled(!isNoise);
    detuneLabel.setEnabled(!isNoise);
    detuneSlider.setEnabled(!isNoise);
    spreadLabel.setEnabled(!isNoise);
    spreadSlider.setEnabled(!isNoise);

    edgeSlider.setLookAndFeel(&berryLookAndFeel);
    detuneSlider.setLookAndFeel(&berryLookAndFeel);
    spreadSlider.setLookAndFeel(&berryLookAndFeel);
    gainSlider.setLookAndFeel(&berryLookAndFeel);
    for (auto& p : allParams.controlItemParams) {
        if (p.isControlling(CONTROL_TARGET_OSC_PARAM::Edge, index)) {
            edgeSlider.setLookAndFeel(&berryLookAndFeelControlled);
        } else if (p.isControlling(CONTROL_TARGET_OSC_PARAM::Detune, index)) {
            detuneSlider.setLookAndFeel(&berryLookAndFeelControlled);
        } else if (p.isControlling(CONTROL_TARGET_OSC_PARAM::Spread, index)) {
            spreadSlider.setLookAndFeel(&berryLookAndFeelControlled);
        } else if (p.isControlling(CONTROL_TARGET_OSC_PARAM::Gain, index)) {
            gainSlider.setLookAndFeel(&berryLookAndFeelControlled);
        }
    }
}

//==============================================================================
EnvelopeComponent::EnvelopeComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      attackCurveSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                        juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox),
      decaySlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
      sustainSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox),
      releaseSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedEnvelopeParams();

    initLinear(attackCurveSlider, params.AttackCurve, 0.01, this, *this);
    initSkewFromMid(attackSlider, params.Attack, 0.001, " sec", nullptr, this, *this);
    initSkewFromMid(decaySlider, params.Decay, 0.01, " sec", nullptr, this, *this);
    initLinearPercent(sustainSlider, params.Sustain, 0.01, this, *this);
    initSkewFromMid(releaseSlider, params.Release, 0.01, " sec", nullptr, this, *this);
    initLabel(attackCurveLabel, "A. Curve", *this);
    initLabel(attackLabel, "Attack", *this);
    initLabel(decayLabel, "Decay", *this);
    initLabel(sustainLabel, "Sustain", *this);
    initLabel(releaseLabel, "Release", *this);

    startTimerHz(30.0f);
}

EnvelopeComponent::~EnvelopeComponent() {}

void EnvelopeComponent::paint(juce::Graphics& g) {}

void EnvelopeComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    consumeLabeledKnob(bounds, attackCurveLabel, attackCurveSlider);
    consumeLabeledKnob(bounds, attackLabel, attackSlider);
    consumeLabeledKnob(bounds, decayLabel, decaySlider);
    consumeLabeledKnob(bounds, sustainLabel, sustainSlider);
    consumeLabeledKnob(bounds, releaseLabel, releaseSlider);
}
void EnvelopeComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedEnvelopeParams();
    if (slider == &attackCurveSlider) {
        *params.AttackCurve = (float)attackCurveSlider.getValue();
    } else if (slider == &attackSlider) {
        *params.Attack = (float)attackSlider.getValue();
    } else if (slider == &decaySlider) {
        *params.Decay = (float)decaySlider.getValue();
    } else if (slider == &sustainSlider) {
        *params.Sustain = (float)sustainSlider.getValue();
    } else if (slider == &releaseSlider) {
        *params.Release = (float)releaseSlider.getValue();
    }
}
void EnvelopeComponent::timerCallback() {
    auto& params = getSelectedEnvelopeParams();
    attackCurveSlider.setValue(params.AttackCurve->get(), juce::dontSendNotification);
    attackSlider.setValue(params.Attack->get(), juce::dontSendNotification);
    decaySlider.setValue(params.Decay->get(), juce::dontSendNotification);
    sustainSlider.setValue(params.Sustain->get(), juce::dontSendNotification);
    releaseSlider.setValue(params.Release->get(), juce::dontSendNotification);
}

//==============================================================================
FilterComponent::FilterComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      targetSelector("Target"),
      typeSelector("Type"),
      freqTypeToggle("Freq Type"),
      hzSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
      semitoneSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
      qSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
      gainSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedFilterParams();

    initChoice(targetSelector, params.Target, this, *this);
    initChoice(typeSelector, params.Type, this, *this);
    initChoiceToggle(freqTypeToggle, FILTER_FREQ_TYPE_NAMES.indexOf("Rel"), params.FreqType, this, *this);
    initSkewFromMid(hzSlider, params.Hz, 0.01f, " Hz", nullptr, this, *this);
    auto formatSemitone = [](double value) -> std::string {
        int cent = value;
        int centAbs = std::abs(cent);
        int oct = centAbs / 12;
        int octFrac = centAbs % 12;
        return (cent == 0 ? " " : cent > 0 ? "+" : "-") + std::to_string(oct) + ":" + std::to_string(octFrac) + " oct";
    };
    initLinear(semitoneSlider, params.Semitone, 0.01, nullptr, std::move(formatSemitone), this, *this);
    initSkewFromMid(qSlider, params.Q, 0.01, nullptr, nullptr, this, *this);
    initLinear(gainSlider, params.Gain, 0.01, " dB", nullptr, this, *this);
    initLabel(targetLabel, "OSC", *this);
    initLabel(typeLabel, "Type", *this);
    initLabel(freqTypeLabel, "Rel", *this);
    initLabel(freqLabel, "Freq", *this);
    initLabel(qLabel, "Q", *this);
    initLabel(gainLabel, "Gain", *this);

    startTimerHz(30.0f);
}

FilterComponent::~FilterComponent() {}

void FilterComponent::paint(juce::Graphics& g) {}

void FilterComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto bodyHeight = bounds.getHeight();
    auto upperArea = bounds.removeFromTop(bodyHeight / 2);
    auto& lowerArea = bounds;
    consumeLabeledComboBox(upperArea, 70, targetLabel, targetSelector);
    consumeLabeledComboBox(upperArea, 120, typeLabel, typeSelector);
    consumeLabeledToggle(lowerArea, 35, freqTypeLabel, freqTypeToggle);
    consumeLabeledKnob(lowerArea, freqLabel, hzSlider, semitoneSlider);
    consumeLabeledKnob(lowerArea, qLabel, qSlider);
    consumeLabeledKnob(lowerArea, gainLabel, gainSlider);
}
void FilterComponent::buttonClicked(juce::Button* button) {
    auto& params = getSelectedFilterParams();
    if (button == &freqTypeToggle) {
        *params.FreqType = freqTypeToggle.getToggleState() ? FILTER_FREQ_TYPE_NAMES.indexOf("Rel")
                                                           : FILTER_FREQ_TYPE_NAMES.indexOf("Abs");
    }
}
void FilterComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    auto& params = getSelectedFilterParams();
    if (comboBoxThatHasChanged == &targetSelector) {
        *params.Target = targetSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &typeSelector) {
        *params.Type = typeSelector.getSelectedItemIndex();
    }
}
void FilterComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedFilterParams();
    if (slider == &hzSlider) {
        *params.Hz = (float)hzSlider.getValue();
    } else if (slider == &semitoneSlider) {
        *params.Semitone = semitoneSlider.getValue();
    } else if (slider == &qSlider) {
        *params.Q = (float)qSlider.getValue();
    } else if (slider == &gainSlider) {
        *params.Gain = (float)gainSlider.getValue();
    }
}
void FilterComponent::timerCallback() {
    auto& params = getSelectedFilterParams();

    targetSelector.setSelectedItemIndex(params.Target->getIndex(), juce::dontSendNotification);
    typeSelector.setSelectedItemIndex(params.Type->getIndex(), juce::dontSendNotification);
    freqTypeToggle.setToggleState(params.FreqType->getIndex() == FILTER_FREQ_TYPE_NAMES.indexOf("Rel"),
                                  juce::dontSendNotification);
    hzSlider.setValue(params.Hz->get(), juce::dontSendNotification);
    semitoneSlider.setValue(params.Semitone->get(), juce::dontSendNotification);
    qSlider.setValue(params.Q->get(), juce::dontSendNotification);

    hzSlider.setVisible(params.isFreqAbsolute());
    semitoneSlider.setVisible(!params.isFreqAbsolute());

    auto hasGain = params.hasGain();
    gainLabel.setEnabled(hasGain);
    gainSlider.setEnabled(hasGain);

    hzSlider.setLookAndFeel(&berryLookAndFeel);
    semitoneSlider.setLookAndFeel(&berryLookAndFeel);
    qSlider.setLookAndFeel(&berryLookAndFeel);
    for (auto& p : allParams.controlItemParams) {
        if (p.isControlling(CONTROL_TARGET_FILTER_PARAM::Freq, index)) {
            hzSlider.setLookAndFeel(&berryLookAndFeelControlled);
            semitoneSlider.setLookAndFeel(&berryLookAndFeelControlled);
        } else if (p.isControlling(CONTROL_TARGET_FILTER_PARAM::Q, index)) {
            qSlider.setLookAndFeel(&berryLookAndFeelControlled);
        }
    }
}

//==============================================================================
ModEnvComponent::ModEnvComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      targetTypeSelector("TargetType"),
      targetOscSelector("TargetOsc"),
      targetFilterSelector("TargetFilter"),
      targetOscParamSelector("TargetOscParam"),
      targetFilterParamSelector("TargetFilterParam"),
      peakFreqSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
      waitSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox),
      decaySlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedModEnvParams();

    initChoice(targetTypeSelector, params.TargetType, this, targetSelector);
    initChoice(targetOscSelector, params.TargetOsc, this, targetSelector);
    initChoice(targetFilterSelector, params.TargetFilter, this, targetSelector);
    initChoice(targetOscParamSelector, params.TargetOscParam, this, targetSelector);
    initChoice(targetFilterParamSelector, params.TargetFilterParam, this, targetSelector);
    initChoice(fadeSelector, params.Fade, this, *this);
    auto formatPeakFreq = [](double oct) -> juce::String {
        return (oct == 0 ? " " : oct > 0 ? "+" : "-") + juce::String(std::abs(oct), 2) + " oct";
    };
    initLinear(peakFreqSlider, params.PeakFreq, 0.01, nullptr, std::move(formatPeakFreq), this, *this);
    initSkewFromMid(waitSlider, params.Wait, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(attackSlider, params.Attack, 0.001, " sec", nullptr, this, *this);
    initSkewFromMid(decaySlider, params.Decay, 0.01, " sec", nullptr, this, *this);
    initLabel(targetLabel, "Destination", *this);
    initLabel(typeLabel, "Type", *this);
    initLabel(fadeLabel, "Fade", *this);
    initLabel(peakFreqLabel, "Peak Freq", *this);
    initLabel(waitLabel, "Wait", *this);
    initLabel(attackLabel, "Attack", *this);
    initLabel(decayLabel, "Decay", *this);

    this->addAndMakeVisible(targetSelector);

    startTimerHz(30.0f);
}

ModEnvComponent::~ModEnvComponent() {}

void ModEnvComponent::paint(juce::Graphics& g) {}

void ModEnvComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    auto bodyHeight = bounds.getHeight();
    auto upperArea = bounds.removeFromTop(bodyHeight / 2);
    auto& lowerArea = bounds;
    consumeLabeledComboBox(upperArea, 280, targetLabel, targetSelector);
    {
        auto selectorsArea = targetSelector.getLocalBounds();

        targetTypeSelector.setBounds(selectorsArea.removeFromLeft(90));
        auto indexArea = selectorsArea.removeFromLeft(70);
        targetOscSelector.setBounds(indexArea);
        targetFilterSelector.setBounds(indexArea);
        auto paramArea = selectorsArea.removeFromLeft(110);
        targetOscParamSelector.setBounds(paramArea);
        targetFilterParamSelector.setBounds(paramArea);
    }
    {
        juce::Rectangle<int> area = lowerArea.removeFromLeft(SLIDER_WIDTH);
        auto area2 = area;
        consumeLabeledKnob(area, peakFreqLabel, peakFreqSlider);
        consumeLabeledComboBox(area2, 120, fadeLabel, fadeSelector);
    }
    consumeLabeledKnob(lowerArea, waitLabel, waitSlider, attackLabel, attackSlider);
    consumeLabeledKnob(lowerArea, decayLabel, decaySlider);
}
void ModEnvComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    auto& params = getSelectedModEnvParams();
    if (comboBoxThatHasChanged == &targetTypeSelector) {
        *params.TargetType = targetTypeSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetOscSelector) {
        *params.TargetOsc = targetOscSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetFilterSelector) {
        *params.TargetFilter = targetFilterSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetOscParamSelector) {
        *params.TargetOscParam = targetOscParamSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetFilterParamSelector) {
        *params.TargetFilterParam = targetFilterParamSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &fadeSelector) {
        *params.Fade = fadeSelector.getSelectedItemIndex();
    }
    resized();  // re-render
}
void ModEnvComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedModEnvParams();
    if (slider == &peakFreqSlider) {
        *params.PeakFreq = (float)peakFreqSlider.getValue();
    } else if (slider == &waitSlider) {
        *params.Wait = (float)waitSlider.getValue();
    } else if (slider == &attackSlider) {
        *params.Attack = (float)attackSlider.getValue();
    } else if (slider == &decaySlider) {
        *params.Decay = (float)decaySlider.getValue();
    }
}
void ModEnvComponent::timerCallback() {
    auto& params = getSelectedModEnvParams();

    targetTypeSelector.setSelectedItemIndex(params.TargetType->getIndex(), juce::dontSendNotification);
    targetOscSelector.setSelectedItemIndex(params.TargetOsc->getIndex(), juce::dontSendNotification);
    targetFilterSelector.setSelectedItemIndex(params.TargetFilter->getIndex(), juce::dontSendNotification);
    targetOscParamSelector.setSelectedItemIndex(params.TargetOscParam->getIndex(), juce::dontSendNotification);
    targetFilterParamSelector.setSelectedItemIndex(params.TargetFilterParam->getIndex(), juce::dontSendNotification);
    fadeSelector.setSelectedItemIndex(params.Fade->getIndex(), juce::dontSendNotification);

    peakFreqSlider.setValue(params.PeakFreq->get(), juce::dontSendNotification);
    waitSlider.setValue(params.Wait->get(), juce::dontSendNotification);
    attackSlider.setValue(params.Attack->get(), juce::dontSendNotification);
    decaySlider.setValue(params.Decay->get(), juce::dontSendNotification);

    auto targetType = params.getTargetType();
    targetOscSelector.setVisible(targetType == MODENV_TARGET_TYPE::OSC);
    targetOscParamSelector.setVisible(targetType == MODENV_TARGET_TYPE::OSC);
    targetFilterSelector.setVisible(targetType == MODENV_TARGET_TYPE::Filter);
    targetFilterParamSelector.setVisible(targetType == MODENV_TARGET_TYPE::Filter);

    auto isTargetFreq = params.isTargetFreq();
    auto isFadeIn = params.isFadeIn();
    peakFreqLabel.setVisible(isTargetFreq);
    peakFreqSlider.setVisible(isTargetFreq);
    fadeLabel.setVisible(!isTargetFreq);
    fadeSelector.setVisible(!isTargetFreq);
    waitLabel.setVisible(!isTargetFreq && isFadeIn);
    waitSlider.setVisible(!isTargetFreq && isFadeIn);
    attackLabel.setVisible(isTargetFreq || !isFadeIn);
    attackSlider.setVisible(isTargetFreq || !isFadeIn);
}

//==============================================================================
DelayComponent::DelayComponent(AllParams& allParams)
    : allParams(allParams),
      typeSelector("Type"),
      timeLSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
      timeRSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
      timeSyncLSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                      juce::Slider::TextEntryBoxPosition::NoTextBox),
      timeSyncRSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                      juce::Slider::TextEntryBoxPosition::NoTextBox),
      lowFreqSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox),
      highFreqSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
      feedbackSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
      mixSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedDelayParams();

    initChoice(typeSelector, params.Type, this, *this);
    initChoiceToggle(syncToggle, params.Sync, this, *this);
    initSkewFromMid(timeLSlider, params.TimeL, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(timeRSlider, params.TimeR, 0.01, " sec", nullptr, this, *this);
    initEnum(timeSyncLSlider, params.TimeSyncL, this, *this);
    initEnum(timeSyncRSlider, params.TimeSyncR, this, *this);
    initSkewFromMid(lowFreqSlider, params.LowFreq, 1.0, " Hz", nullptr, this, *this);
    initSkewFromMid(highFreqSlider, params.HighFreq, 1.0, " Hz", nullptr, this, *this);
    initLinearPercent(feedbackSlider, params.Feedback, 0.01, this, *this);
    initLinear(mixSlider, params.Mix, 0.01, this, *this);

    initLabel(typeLabel, "Type", *this);
    initLabel(syncLabel, "Sync", *this);
    initLabel(timeLLabel, "Time L", *this);
    initLabel(timeRLabel, "Time R", *this);
    initLabel(lowFreqLabel, "Lo Cut", *this);
    initLabel(highFreqLabel, "Hi Cut", *this);
    initLabel(feedbackLabel, "Feedback", *this);
    initLabel(mixLabel, "Mix", *this);

    startTimerHz(30.0f);
}

DelayComponent::~DelayComponent() {}

void DelayComponent::paint(juce::Graphics& g) {}

void DelayComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    auto bodyHeight = bounds.getHeight();
    auto upperArea = bounds.removeFromTop(bodyHeight / 2);
    auto& lowerArea = bounds;
    consumeLabeledComboBox(upperArea, 120, typeLabel, typeSelector);
    consumeLabeledToggle(upperArea, 35, syncLabel, syncToggle);
    consumeLabeledKnob(upperArea, lowFreqLabel, lowFreqSlider);
    consumeLabeledKnob(upperArea, highFreqLabel, highFreqSlider);
    consumeLabeledKnob(lowerArea, timeLLabel, timeLSlider, timeSyncLSlider);
    consumeLabeledKnob(lowerArea, timeRLabel, timeRSlider, timeSyncRSlider);
    consumeLabeledKnob(lowerArea, feedbackLabel, feedbackSlider);
    consumeLabeledKnob(lowerArea, mixLabel, mixSlider);
}
void DelayComponent::buttonClicked(juce::Button* button) {
    auto& params = getSelectedDelayParams();
    if (button == &syncToggle) {
        *params.Sync = syncToggle.getToggleState();
    }
}
void DelayComponent::comboBoxChanged(juce::ComboBox* comboBox) {
    auto& params = getSelectedDelayParams();
    if (comboBox == &typeSelector) {
        *params.Type = typeSelector.getSelectedItemIndex();
    }
}
void DelayComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedDelayParams();
    if (slider == &timeLSlider) {
        *params.TimeL = (float)timeLSlider.getValue();
    } else if (slider == &timeRSlider) {
        *params.TimeR = (float)timeRSlider.getValue();
    } else if (slider == &timeSyncLSlider) {
        *params.TimeSyncL = timeSyncLSlider.getValue();
    } else if (slider == &timeSyncRSlider) {
        *params.TimeSyncR = timeSyncRSlider.getValue();
    } else if (slider == &lowFreqSlider) {
        *params.LowFreq = (float)lowFreqSlider.getValue();
    } else if (slider == &highFreqSlider) {
        *params.HighFreq = (float)highFreqSlider.getValue();
    } else if (slider == &feedbackSlider) {
        *params.Feedback = (float)feedbackSlider.getValue();
    } else if (slider == &mixSlider) {
        *params.Mix = (float)mixSlider.getValue();
    }
}
void DelayComponent::timerCallback() {
    auto& params = getSelectedDelayParams();

    typeSelector.setSelectedItemIndex(params.Type->getIndex(), juce::dontSendNotification);
    syncToggle.setToggleState(params.Sync->get(), juce::dontSendNotification);
    timeLSlider.setVisible(!params.Sync->get());
    timeRSlider.setVisible(!params.Sync->get());
    timeSyncLSlider.setVisible(params.Sync->get());
    timeSyncRSlider.setVisible(params.Sync->get());
    timeLSlider.setValue(params.TimeL->get(), juce::dontSendNotification);
    timeRSlider.setValue(params.TimeR->get(), juce::dontSendNotification);
    timeSyncLSlider.setValue(params.TimeSyncL->getIndex(), juce::dontSendNotification);
    timeSyncRSlider.setValue(params.TimeSyncR->getIndex(), juce::dontSendNotification);

    lowFreqSlider.setValue(params.LowFreq->get(), juce::dontSendNotification);
    highFreqSlider.setValue(params.HighFreq->get(), juce::dontSendNotification);
    feedbackSlider.setValue(params.Feedback->get(), juce::dontSendNotification);
    mixSlider.setValue(params.Mix->get(), juce::dontSendNotification);

    mixSlider.setLookAndFeel(&berryLookAndFeel);
    for (auto& p : allParams.controlItemParams) {
        if (p.isControlling(CONTROL_TARGET_MISC_PARAM::DelayMix)) {
            mixSlider.setLookAndFeel(&berryLookAndFeelControlled);
        }
    }
}

//==============================================================================
DrumComponent::DrumComponent(AllParams& allParams)
    : allParams(allParams),
      noteToPlaySlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                       juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedDrumParams();

    initLinear(noteToPlaySlider, params.NoteToPlay, this, *this);
    initChoice(noteToMuteEnabledSelector, params.NoteToMuteEnabled, this, noteToMuteSelector);
    initChoice(noteToMuteKindSelector, params.NoteToMuteKind, this, noteToMuteSelector);
    initChoice(noteToMuteOctSelector, params.NoteToMuteOct, this, noteToMuteSelector);
    initChoice(busSelector, params.Bus, this, *this);

    initLabel(noteToPlayLabel, "Note", *this);
    initLabel(noteToMuteLabel, "Note to Mute", *this);
    initLabel(busLabel, "Bus", *this);

    this->addAndMakeVisible(noteToMuteSelector);

    startTimerHz(30.0f);
}

DrumComponent::~DrumComponent() {}

void DrumComponent::paint(juce::Graphics& g) {}

void DrumComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto height = bounds.getHeight();
    // auto upperArea = bounds.removeFromTop(height / 2);
    auto upperArea = bounds.removeFromTop(height * 3 / 4);
    auto& lowerArea = bounds;

    auto& params = getSelectedDrumParams();

    consumeLabeledKnob(upperArea, noteToPlayLabel, noteToPlaySlider);
    consumeLabeledComboBox(upperArea, 180, noteToMuteLabel, noteToMuteSelector);
    {
        juce::Rectangle<int> selectorsArea = noteToMuteSelector.getLocalBounds();
        noteToMuteEnabledSelector.setBounds(selectorsArea.removeFromLeft(60));
        noteToMuteKindSelector.setBounds(selectorsArea.removeFromLeft(60));
        noteToMuteOctSelector.setBounds(selectorsArea.removeFromLeft(60));
    }
    consumeLabeledComboBox(upperArea, 70, busLabel, busSelector);
}
void DrumComponent::comboBoxChanged(juce::ComboBox* comboBox) {
    auto& params = getSelectedDrumParams();
    if (comboBox == &noteToMuteEnabledSelector) {
        *params.NoteToMuteEnabled = noteToMuteEnabledSelector.getSelectedItemIndex();
    } else if (comboBox == &noteToMuteKindSelector) {
        *params.NoteToMuteKind = noteToMuteKindSelector.getSelectedItemIndex();
    } else if (comboBox == &noteToMuteOctSelector) {
        *params.NoteToMuteOct = noteToMuteOctSelector.getSelectedItemIndex();
    } else if (comboBox == &busSelector) {
        *params.Bus = busSelector.getSelectedItemIndex();
    }
}
void DrumComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedDrumParams();
    if (slider == &noteToPlaySlider) {
        *params.NoteToPlay = noteToPlaySlider.getValue();
    }
}

void DrumComponent::timerCallback() {
    auto& params = getSelectedDrumParams();
    noteToPlaySlider.setValue(params.NoteToPlay->get(), juce::dontSendNotification);
    noteToMuteEnabledSelector.setSelectedItemIndex(params.NoteToMuteEnabled->get(), juce::dontSendNotification);
    noteToMuteKindSelector.setSelectedItemIndex(params.NoteToMuteKind->getIndex(), juce::dontSendNotification);
    noteToMuteOctSelector.setSelectedItemIndex(params.NoteToMuteOct->getIndex(), juce::dontSendNotification);

    auto muteEnabled = params.NoteToMuteEnabled->get();
    noteToMuteKindSelector.setEnabled(muteEnabled);
    noteToMuteOctSelector.setEnabled(muteEnabled);
    busSelector.setSelectedItemIndex(params.Bus->getIndex(), juce::dontSendNotification);
}

//==============================================================================
ControlItemComponent::ControlItemComponent(ControlItemParams& params)
    : params(params),
      numberSelector("Number"),
      targetTypeSelector("TargetType"),
      targetOscSelector("TargetOsc"),
      targetFilterSelector("TargetFilter"),
      targetOscParamSelector("TargetOscParam"),
      targetFilterParamSelector("TargetFilterParam"),
      targetMiscParamSelector("TargetMiscParam") {
    initChoice(numberSelector, params.Number, this, *this);
    initChoice(targetTypeSelector, params.TargetType, this, *this);
    initChoice(targetOscSelector, params.TargetOsc, this, *this);
    initChoice(targetFilterSelector, params.TargetFilter, this, *this);
    initChoice(targetOscParamSelector, params.TargetOscParam, this, *this);
    initChoice(targetFilterParamSelector, params.TargetFilterParam, this, *this);
    initChoice(targetMiscParamSelector, params.TargetMiscParam, this, *this);

    startTimerHz(30.0f);
}

ControlItemComponent::~ControlItemComponent() {}

void ControlItemComponent::paint(juce::Graphics& g) {}

void ControlItemComponent::resized() {
    juce::Rectangle<int> area = getLocalBounds();

    numberSelector.setBounds(area.removeFromLeft(area.getWidth() / 5));

    area.removeFromLeft(PARAM_MARGIN_LEFT);

    targetTypeSelector.setBounds(area.removeFromLeft(90));

    targetMiscParamSelector.setBounds(area);

    auto indexArea = area.removeFromLeft(70);
    targetOscSelector.setBounds(indexArea);
    targetFilterSelector.setBounds(indexArea);

    auto& paramArea = area;
    targetOscParamSelector.setBounds(paramArea);
    targetFilterParamSelector.setBounds(paramArea);
}
void ControlItemComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &numberSelector) {
        *params.Number = numberSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetTypeSelector) {
        *params.TargetType = targetTypeSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetOscSelector) {
        *params.TargetOsc = targetOscSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetFilterSelector) {
        *params.TargetFilter = targetFilterSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetOscParamSelector) {
        *params.TargetOscParam = targetOscParamSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetFilterParamSelector) {
        *params.TargetFilterParam = targetFilterParamSelector.getSelectedItemIndex();
    } else if (comboBoxThatHasChanged == &targetMiscParamSelector) {
        *params.TargetMiscParam = targetMiscParamSelector.getSelectedItemIndex();
    }
    resized();  // re-render
}
void ControlItemComponent::timerCallback() {
    numberSelector.setSelectedItemIndex(params.Number->getIndex(), juce::dontSendNotification);
    targetTypeSelector.setSelectedItemIndex(params.TargetType->getIndex(), juce::dontSendNotification);
    targetOscSelector.setSelectedItemIndex(params.TargetOsc->getIndex(), juce::dontSendNotification);
    targetFilterSelector.setSelectedItemIndex(params.TargetFilter->getIndex(), juce::dontSendNotification);
    targetOscParamSelector.setSelectedItemIndex(params.TargetOscParam->getIndex(), juce::dontSendNotification);
    targetFilterParamSelector.setSelectedItemIndex(params.TargetFilterParam->getIndex(), juce::dontSendNotification);
    targetMiscParamSelector.setSelectedItemIndex(params.TargetMiscParam->getIndex(), juce::dontSendNotification);
    auto enabled = params.Number->getIndex() != 0;
    targetTypeSelector.setEnabled(enabled);
    targetOscSelector.setEnabled(enabled);
    targetFilterSelector.setEnabled(enabled);
    targetOscParamSelector.setEnabled(enabled);
    targetFilterParamSelector.setEnabled(enabled);
    targetMiscParamSelector.setEnabled(enabled);

    auto targetType = params.getTargetType();
    targetOscSelector.setVisible(targetType == CONTROL_TARGET_TYPE::OSC);
    targetOscParamSelector.setVisible(targetType == CONTROL_TARGET_TYPE::OSC);
    targetFilterSelector.setVisible(targetType == CONTROL_TARGET_TYPE::Filter);
    targetFilterParamSelector.setVisible(targetType == CONTROL_TARGET_TYPE::Filter);
    targetMiscParamSelector.setVisible(targetType == CONTROL_TARGET_TYPE::Master);
}

//==============================================================================
ControlComponent::ControlComponent(std::array<ControlItemParams, NUM_CONTROL>& params)
    : controlItemComponents{
          ControlItemComponent(params[0]), ControlItemComponent(params[1]), ControlItemComponent(params[2])} {
    initLabel(numberLabel, "CC", *this);
    initLabel(targetLabel, "Destination", *this);

    for (int i = 0; i < NUM_CONTROL; i++) {
        addAndMakeVisible(controlItemComponents[i]);
    }
}

ControlComponent::~ControlComponent() {}

void ControlComponent::paint(juce::Graphics& g) {}

void ControlComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto width = bounds.getWidth();

    auto labelArea = bounds.removeFromTop(LABEL_HEIGHT);
    numberLabel.setBounds(labelArea.removeFromLeft(width / 5));
    targetLabel.setBounds(labelArea);

    for (int i = 0; i < NUM_CONTROL; i++) {
        bounds.removeFromTop(LABEL_MARGIN_BOTTOM);
        controlItemComponents[i].setBounds(bounds.removeFromTop(COMBO_BOX_HEIGHT));
    }
}

//==============================================================================
AnalyserToggleItem::AnalyserToggleItem(std::string name) {
    initLabel(nameLabel, PARAM_LABEL_FONT_SIZE, "Regular", juce::Justification::right, std::move(name), *this);
}
AnalyserToggleItem::~AnalyserToggleItem() {}
void AnalyserToggleItem::paint(juce::Graphics& g) {
    juce::Rectangle<int> bounds = getLocalBounds().removeFromRight(3).reduced(0, 4);

    auto color = value ? colour::SELECT : colour::PIT;
    g.setColour(color);
    g.fillRect(bounds);
}
void AnalyserToggleItem::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.removeFromRight(5);
    nameLabel.setBounds(bounds);
}
void AnalyserToggleItem::addListener(Listener* l) { listeners.add(l); }
void AnalyserToggleItem::mouseUp(const juce::MouseEvent& e) {
    std::cout << "mouseup:" << nameLabel.getText() << std::endl;
    Component::BailOutChecker checker(this);
    //    if (checker.shouldBailOut()) {
    //        return;
    //    }
    if (e.mouseWasClicked()) {
        if (!value) {
            value = true;
            listeners.callChecked(checker, [this](AnalyserToggleItem::Listener& l) { l.toggleItemSelected(this); });
        }
    }
}

//==============================================================================
AnalyserToggle::AnalyserToggle(ANALYSER_MODE* analyserMode)
    : analyserMode(analyserMode), spectrumToggle("Spectrum"), envelopeToggle("Envelope"), filterToggle("Filter") {
    spectrumToggle.addListener(this);
    addAndMakeVisible(spectrumToggle);

    envelopeToggle.addListener(this);
    addAndMakeVisible(envelopeToggle);

    filterToggle.addListener(this);
    addAndMakeVisible(filterToggle);

    spectrumToggle.setValue(*analyserMode == ANALYSER_MODE::Spectrum);
    envelopeToggle.setValue(*analyserMode == ANALYSER_MODE::Envelope);
    filterToggle.setValue(*analyserMode == ANALYSER_MODE::Filter);
}
AnalyserToggle::~AnalyserToggle() {}
void AnalyserToggle::paint(juce::Graphics& g) {}
void AnalyserToggle::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    spectrumToggle.setBounds(bounds.removeFromTop(25));
    envelopeToggle.setBounds(bounds.removeFromTop(25));
    filterToggle.setBounds(bounds.removeFromTop(25));
}
void AnalyserToggle::toggleItemSelected(AnalyserToggleItem* toggleItem) {
    if (toggleItem == &spectrumToggle) {
        *analyserMode = ANALYSER_MODE::Spectrum;
    } else if (toggleItem == &envelopeToggle) {
        *analyserMode = ANALYSER_MODE::Envelope;
    } else if (toggleItem == &filterToggle) {
        *analyserMode = ANALYSER_MODE::Filter;
    }
    spectrumToggle.setValue(*analyserMode == ANALYSER_MODE::Spectrum);
    envelopeToggle.setValue(*analyserMode == ANALYSER_MODE::Envelope);
    filterToggle.setValue(*analyserMode == ANALYSER_MODE::Filter);
}

//==============================================================================
AnalyserWindow::AnalyserWindow(ANALYSER_MODE* analyserMode,
                               LatestDataProvider* latestDataProvider,
                               MonoStack* monoStack,
                               VoiceParams& voiceParams,
                               std::vector<MainParams>& mainParamList)
    : analyserMode(analyserMode),
      latestDataProvider(latestDataProvider),
      monoStack(monoStack),
      voiceParams(voiceParams),
      mainParamList(mainParamList),
      forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann),
      lastAmpEnvParams{SimpleAmpEnvParams(), SimpleAmpEnvParams()},
      lastModEnvParams{SimpleModEnvParams(), SimpleModEnvParams(), SimpleModEnvParams()} {
    latestDataProvider->addConsumer(&fftConsumer);
    latestDataProvider->addConsumer(&levelConsumer);

    startTimerHz(30.0f);
}
AnalyserWindow::~AnalyserWindow() {
    latestDataProvider->removeConsumer(&fftConsumer);
    latestDataProvider->removeConsumer(&levelConsumer);
}

void AnalyserWindow::resized() {}
void AnalyserWindow::timerCallback() {
    stopTimer();
    bool shouldRepaint = false;

    switch (*analyserMode) {
        case ANALYSER_MODE::Spectrum: {
            lastAnalyserMode = ANALYSER_MODE::Spectrum;
            if (fftConsumer.ready) {
                auto hasData = drawNextFrameOfSpectrum();
                fftConsumer.ready = false;
                readyToDrawFrame = true;
                shouldRepaint = shouldRepaint || hasData;
            }
            if (levelConsumer.ready) {
                auto hasData = drawNextFrameOfLevel();
                levelConsumer.ready = false;
                //        readyToDrawFrame = true;
                shouldRepaint = shouldRepaint || hasData;
            }
            break;
        }
        case ANALYSER_MODE::Envelope: {
            auto& mainParams = mainParamList[voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128];
            auto& envelopeParams = mainParams.envelopeParams;
            auto& modEnvParams = mainParams.modEnvParams;

            if (lastAnalyserMode != ANALYSER_MODE::Envelope) {
                shouldRepaint = true;
            }
            lastAnalyserMode = ANALYSER_MODE::Envelope;
            auto changed = false;
            for (int i = 0; i < NUM_ENVELOPE; i++) {
                auto p = SimpleAmpEnvParams(envelopeParams[i]);
                if (!lastAmpEnvParams[i].equals(p)) {
                    changed = true;
                }
                lastAmpEnvParams[i] = p;
            }
            for (int i = 0; i < NUM_MODENV; i++) {
                auto p = SimpleModEnvParams(modEnvParams[i]);
                if (!lastModEnvParams[i].equals(p)) {
                    changed = true;
                }
                lastModEnvParams[i] = p;
            }
            if (!changed) {
                break;
            }
            auto maxAD = std::max(
                {envelopeParams[0].Attack->get() + envelopeParams[0].Decay->get() * 4,
                 envelopeParams[1].Attack->get() + envelopeParams[1].Decay->get() * 4,
                 (modEnvParams[0].shouldUseHold() ? modEnvParams[0].Wait->get() : modEnvParams[0].Attack->get()) +
                     modEnvParams[0].Decay->get() * 4,
                 (modEnvParams[1].shouldUseHold() ? modEnvParams[1].Wait->get() : modEnvParams[1].Attack->get()) +
                     modEnvParams[1].Decay->get() * 4,
                 (modEnvParams[2].shouldUseHold() ? modEnvParams[2].Wait->get() : modEnvParams[2].Attack->get()) +
                     modEnvParams[2].Decay->get() * 4});
            auto maxR = std::max({envelopeParams[0].Release->get() * 4, envelopeParams[1].Release->get() * 4});
            auto maxSec = maxAD + maxR;
            auto sampleRate = (float)scopeSize / maxSec;
            for (int i = 0; i < NUM_ENVELOPE; i++) {
                ampEnvs[i].setParams(envelopeParams[i].AttackCurve->get(),
                                     envelopeParams[i].Attack->get(),
                                     0,
                                     envelopeParams[i].Decay->get(),
                                     envelopeParams[i].Sustain->get(),
                                     envelopeParams[i].Release->get());
            }
            for (int i = 0; i < NUM_MODENV; i++) {
                if (modEnvParams[i].shouldUseHold()) {
                    modEnvs[i].setParams(0.5, 0.0, modEnvParams[i].Wait->get(), modEnvParams[i].Decay->get(), 0.0, 0.0);
                } else {
                    modEnvs[i].setParams(
                        0.5, modEnvParams[i].Attack->get(), 0.0, modEnvParams[i].Decay->get(), 0.0, 0.0);
                }
            }
            for (int i = 0; i < NUM_ENVELOPE; i++) {
                ampEnvs[i].doAttack(sampleRate);
            }
            for (int i = 0; i < NUM_MODENV; i++) {
                modEnvs[i].doAttack(sampleRate);
            }
            int releasePoint = sampleRate * maxAD;
            for (int pos = 0; pos < scopeSize; pos++) {
                if (pos == releasePoint) {
                    for (int i = 0; i < NUM_ENVELOPE; i++) {
                        ampEnvs[i].doRelease(sampleRate);
                    }
                }
                for (int i = 0; i < NUM_ENVELOPE; i++) {
                    scopeDataForAmpEnv[i][pos] = ampEnvs[i].getValue();
                    ampEnvs[i].step(sampleRate);
                }
                for (int i = 0; i < NUM_MODENV; i++) {
                    auto value = 0.0f;
                    if (modEnvParams[i].Enabled->get()) {
                        value = modEnvs[i].getValue();
                        if (!modEnvParams[i].isTargetFreq() && modEnvParams[i].isFadeIn()) {
                            value = 1 - value;
                        }
                    }
                    scopeDataForModEnv[i][pos] = value;
                    modEnvs[i].step(sampleRate);
                }
            }
            for (int i = 0; i < NUM_ENVELOPE; i++) {
                ampEnvs[i].forceStop();
            }
            for (int i = 0; i < NUM_MODENV; i++) {
                modEnvs[i].forceStop();
            }
            readyToDrawFrame = true;
            shouldRepaint = true;
            break;
        }
        case ANALYSER_MODE::Filter: {
            auto& mainParams = mainParamList[voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128];
            auto& oscParams = mainParams.oscParams;
            auto& filterParams = mainParams.filterParams;

            if (lastAnalyserMode != ANALYSER_MODE::Filter) {
                shouldRepaint = true;
            }
            lastAnalyserMode = ANALYSER_MODE::Filter;

            bool noteChanged = false;
            if (monoStack->latestNoteNumber != 0) {
                auto newNoteNumber = monoStack->latestNoteNumber;
                noteChanged = relNoteNumber != newNoteNumber;
                relNoteNumber = newNoteNumber;
            }
            for (int i = 0; i < NUM_FILTER; ++i) {
                auto filterType = filterParams[i].getType();
                double freq;
                bool isRel = false;
                switch (filterParams[i].getFreqType()) {
                    case FILTER_FREQ_TYPE::Absolute: {
                        freq = filterParams[i].Hz->get();
                        break;
                    }
                    case FILTER_FREQ_TYPE::Relative: {
                        isRel = true;
                        auto target = filterParams[i].Target->getIndex();
                        float lastNote = relNoteNumber;
                        if (target != NUM_OSC) {
                            lastNote += oscParams[target].Octave->get() * 12;
                            lastNote += oscParams[target].Coarse->get();
                        }
                        lastNote += filterParams[i].Semitone->get();
                        freq = 440.0 * std::pow(2.0, (float)(lastNote - 69) / 12);
                        break;
                    }
                }
                auto q = filterParams[i].Q->get();
                auto gain = filterParams[i].Gain->get();

                auto params = SimpleFilterParams();
                params.enabled = filterParams[i].Enabled->get();
                params.type = filterParams[i].Type->getIndex();
                params.freq = freq;
                params.q = q;
                params.gain = gain;
                if (lastFilterParams[i].equals(params) && !(isRel && noteChanged)) {
                    continue;
                }
                lastFilterParams[i] = params;

                readyToDrawFrame = true;
                shouldRepaint = true;

                if (!filterParams[i].Enabled->get()) {
                    continue;
                }
                auto& filter = filters[i];
                auto sampleRate = 48000;  // TODO: ?
                filter.setSampleRate(sampleRate);
                filter.initializePastData();
                std::fill_n(filterSource, fftSize * 2, 0);
                filterSource[0] = 1;
                for (int i = 0; i < fftSize; i++) {
                    filterSource[i] = filter.step(filterType, freq, q, gain, 0, filterSource[i]);
                }
                forwardFFT.performFrequencyOnlyForwardTransform(filterSource);

                auto minFreq = 40.0f;
                auto maxFreq = 20000.0f;
                auto mindB = -50.0f;
                auto maxdB = 50.0f;
                for (int j = 0; j < scopeSize; ++j) {
                    float hz = xToHz(minFreq, maxFreq, (float)j / scopeSize);
                    float gain = getFFTDataByHz(filterSource, fftSize, sampleRate, hz);
                    auto level = juce::jmap(juce::Decibels::gainToDecibels(gain), mindB, maxdB, 0.0f, 1.0f);
                    scopeDataForFilter[i][j] = level;
                }
            }
            break;
        }
    }
    startTimerHz(30.0f);
    if (shouldRepaint) {
        repaint();
    }
}

bool AnalyserWindow::drawNextFrameOfSpectrum() {
    bool hasData = false;
    for (int i = 0; i < fftSize; i++) {
        fftData[i] = (fftData[i] + fftData[i + fftSize]) * 0.5f;
        if (fftData[i] != 0.0f) {
            hasData = true;
        }
        fftData[i + fftSize] = 0;
    }
    if (!hasData) {
        return false;
    }
    window.multiplyWithWindowingTable(fftData, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    auto sampleRate = 48000;  // TODO: ?
    auto minFreq = 40.0f;
    auto maxFreq = 20000.0f;
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    for (int i = 0; i < scopeSize; ++i) {
        float hz = xToHz(minFreq, maxFreq, (float)i / scopeSize);
        float gain = getFFTDataByHz(fftData, fftSize, sampleRate, hz);
        auto level = juce::jmap(juce::Decibels::gainToDecibels(gain) - juce::Decibels::gainToDecibels((float)fftSize),
                                mindB,
                                maxdB,
                                0.0f,
                                1.0f);
        scopeData[i] = level;
    }
    return true;
}
bool AnalyserWindow::drawNextFrameOfLevel() {
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    bool hasData = false;
    for (int i = 0; i < 2; i++) {
        auto* data = i == 0 ? levelConsumer.destinationL : levelConsumer.destinationR;
        auto db = calcCurrentLevel(levelConsumer.numSamples, data);
        currentLevel[i] = juce::jmap(db, mindB, maxdB, 0.0f, 1.0f);
        if (currentLevel[i] > mindB) {
            hasData = true;
        }
        if (db > 0) {
            (i == 0 ? overflowedLevelL : overflowedLevelR) = db;
            (i == 0 ? overflowWarningL : overflowWarningR) = 30 * 1.2;
        }
    }
    return hasData;
}
void AnalyserWindow::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);

    juce::Rectangle<int> bounds = getLocalBounds();

    if (readyToDrawFrame) {
        auto offsetX = 2;
        auto offsetY = 2;
        auto displayBounds = bounds.reduced(offsetX, offsetY);
        auto height = displayBounds.getHeight();

        switch (*analyserMode) {
            case ANALYSER_MODE::Spectrum: {
                auto levelWidth = 8;
                auto spectrumWidth = displayBounds.getWidth() - levelWidth * 2;

                paintSpectrum(g, colour::ANALYSER_LINE, offsetX, offsetY, spectrumWidth, height, scopeData);
                offsetX += spectrumWidth;
                paintLevel(g, offsetX, offsetY, levelWidth, height, currentLevel[0]);
                offsetX += levelWidth;
                paintLevel(g, offsetX, offsetY, levelWidth, height, currentLevel[1]);
                break;
            }
            case ANALYSER_MODE::Envelope: {
                auto& mainParams = mainParamList[voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128];
                auto& modEnvParams = mainParams.modEnvParams;

                auto spectrumWidth = displayBounds.getWidth();
                for (int i = NUM_MODENV - 1; i >= 0; i--) {
                    if (!modEnvParams[i].Enabled->get()) {
                        continue;
                    }
                    juce::Colour colour = colour::ANALYSER_LINE2;
                    paintSpectrum(g, colour, offsetX, offsetY, spectrumWidth, height, &scopeDataForModEnv[i][0]);
                }
                for (int i = NUM_ENVELOPE - 1; i >= 0; i--) {
                    juce::Colour colour = colour::ANALYSER_LINE;
                    paintSpectrum(g, colour, offsetX, offsetY, spectrumWidth, height, &scopeDataForAmpEnv[i][0]);
                }
                break;
            }
            case ANALYSER_MODE::Filter: {
                auto& mainParams = mainParamList[voiceParams.isDrumMode() ? voiceParams.getTargetNote() : 128];
                auto& filterParams = mainParams.filterParams;

                for (int i = 0; i < NUM_FILTER; i++) {
                    if (!filterParams[i].Enabled->get()) {
                        continue;
                    }
                    auto spectrumWidth = displayBounds.getWidth();
                    bool isRel = !filterParams[i].isFreqAbsolute();
                    juce::Colour colour = isRel ? colour::ANALYSER_LINE2 : colour::ANALYSER_LINE;
                    paintSpectrum(g, colour, offsetX, offsetY, spectrumWidth, height, &scopeDataForFilter[i][0]);
                }
                break;
            }
        }
    }
    g.setColour(juce::Colour(30, 30, 30));
    g.drawRect(bounds, 2.0f);
}
void AnalyserWindow::paintSpectrum(
    juce::Graphics& g, juce::Colour colour, int offsetX, int offsetY, int width, int height, float* scopeData) {
    g.setColour(colour);
    for (int i = 1; i < scopeSize; ++i) {
        g.drawLine({offsetX + (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width),
                    offsetY - 0.5f + juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                    offsetX + (float)juce::jmap(i, 0, scopeSize - 1, 0, width),
                    offsetY - 0.5f + juce::jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f)});
    }
}
void AnalyserWindow::paintLevel(juce::Graphics& g, int offsetX, int offsetY, int width, int height, float level) {
    g.setColour(colour::ANALYSER_LINE);
    if (overflowWarningL > 0) {
        g.setColour(colour::ERROR);
        overflowWarningL--;
    }
    int barWidth = width - 1;
    int barHeight = level * height;
    g.fillRect(offsetX + 1, offsetY + height - barHeight, barWidth, barHeight);
}
