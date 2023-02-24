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
    : allParams(allParams),
      pitchBendRangeButton(),
      timbreNoteNumberSliders{juce::Slider{juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                           juce::Slider::TextEntryBoxPosition::NoTextBox},
                              juce::Slider{juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                           juce::Slider::TextEntryBoxPosition::NoTextBox},
                              juce::Slider{juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                           juce::Slider::TextEntryBoxPosition::NoTextBox}} {
    initIncDec(pitchBendRangeButton, allParams.voiceParams.PitchBendRange, this, *this);
    initChoice(timbreSelector, TIMBER_NAMES, allParams.editingTimbreIndex, this, *this);
    for (int i = 0; i < NUM_TIMBRES; i++) {
        auto& slider = timbreNoteNumberSliders[i];
        initLinear(slider, allParams.mainParams[i].NoteNumber, this, *this);
    }
    initLabel(pitchBendRangeLabel, "PB Range", *this);
    initLabel(timbreLabel, "Timbre", *this);
    initLabel(timbreNoteNumberLabel, "Timbre Note", *this);

    startTimerHz(30.0f);
}

VoiceComponent::~VoiceComponent() {}

void VoiceComponent::paint(juce::Graphics& g) {}

void VoiceComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.reduce(0, 10);
    consumeLabeledIncDecButton(bounds, 60, pitchBendRangeLabel, pitchBendRangeButton);
    consumeLabeledComboBox(bounds, 60, timbreLabel, timbreSelector);
    {
        bounds.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = bounds.removeFromLeft(SLIDER_WIDTH);
        timbreNoteNumberLabel.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        auto knobArea = area.removeFromTop(KNOB_HEIGHT);
        for (int i = 0; i < NUM_TIMBRES; i++) {
            auto& slider = timbreNoteNumberSliders[i];
            slider.setBounds(knobArea);
        }
    }
}
void VoiceComponent::incDecValueChanged(IncDecButton* button) {
    if (button == &pitchBendRangeButton) {
        *allParams.voiceParams.PitchBendRange = pitchBendRangeButton.getValue();
    }
}
void VoiceComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &timbreSelector) {
        allParams.editingTimbreIndex = timbreSelector.getSelectedItemIndex();
    }
}
void VoiceComponent::sliderValueChanged(juce::Slider* slider) {
    for (int i = 0; i < NUM_TIMBRES; i++) {
        auto& timbreNoteNumberSlider = timbreNoteNumberSliders[i];
        if (slider == &timbreNoteNumberSlider) {
            *allParams.mainParams[i].NoteNumber = timbreNoteNumberSlider.getValue();
        }
    }
}
void VoiceComponent::timerCallback() {
    pitchBendRangeButton.setValue(allParams.voiceParams.PitchBendRange->get(), juce::dontSendNotification);
    for (int i = 0; i < NUM_TIMBRES; i++) {
        auto& slider = timbreNoteNumberSliders[i];
        slider.setValue(allParams.mainParams[i].NoteNumber->get(), juce::dontSendNotification);
        slider.setVisible(allParams.editingTimbreIndex == i);
    }
}

//==============================================================================
UtilComponent::UtilComponent(BerryAudioProcessor& processor)
    : processor(processor), copyToClipboardButton(), pasteFromClipboardButton() {
    copyToClipboardButton.setLookAndFeel(&berryLookAndFeel);
    copyToClipboardButton.setButtonText("Copy");
    copyToClipboardButton.addListener(this);
    this->addAndMakeVisible(copyToClipboardButton);

    pasteFromClipboardButton.setLookAndFeel(&berryLookAndFeel);
    pasteFromClipboardButton.setButtonText("Paste");
    pasteFromClipboardButton.addListener(this);
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
    auto& params = allParams.masterParams;
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
    auto& params = allParams.masterParams;
    if (slider == &panSlider) {
        *params.Pan = (float)panSlider.getValue();
    } else if (slider == &volumeSlider) {
        *params.MasterVolume = (float)volumeSlider.getValue();
    }
}
void MasterComponent::timerCallback() {
    auto& params = allParams.masterParams;
    panSlider.setValue(params.Pan->get(), juce::dontSendNotification);
    volumeSlider.setValue(params.MasterVolume->get(), juce::dontSendNotification);
}

//==============================================================================
OscComponent::OscComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      newEnvelopeToggle(),
      gainSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackCurveSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                        juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox),
      decaySlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
      releaseSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedOscParams();
    auto& envParams = getSelectedEnvelopeParams();

    auto formatGain = [](double gain) { return juce::String(juce::Decibels::gainToDecibels(gain), 2) + " dB"; };
    initSkewFromMid(gainSlider, params.Gain, 0.01f, nullptr, std::move(formatGain), this, *this);
    auto formatGain2 = [](double gain) { return juce::String(juce::Decibels::gainToDecibels(gain), 2) + " dB"; };
    initChoiceToggle(newEnvelopeToggle, params.NewEnvelope, this, *this);
    initLinear(attackCurveSlider, envParams.AttackCurve, 0.01, this, *this);
    initSkewFromMid(attackSlider, envParams.Attack, 0.001, " sec", nullptr, this, *this);
    initSkewFromMid(decaySlider, envParams.Decay, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(releaseSlider, envParams.Release, 0.01, " sec", nullptr, this, *this);

    initLabel(gainLabel, "Gain", *this);
    initLabel(newEnvelopeLabel, "New Env", *this);
    initLabel(attackCurveLabel, "A. Curve", *this);
    initLabel(attackLabel, "Attack", *this);
    initLabel(decayLabel, "Decay", *this);
    initLabel(releaseLabel, "Release", *this);

    if (index == 0) {
        newEnvelopeToggle.setEnabled(false);
        newEnvelopeLabel.setEnabled(false);
    }

    startTimerHz(30.0f);
}

OscComponent::~OscComponent() {}

void OscComponent::paint(juce::Graphics& g) {}

void OscComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    consumeLabeledKnob(bounds, gainLabel, gainSlider);
    consumeLabeledToggle(bounds, 45, newEnvelopeLabel, newEnvelopeToggle);
    consumeLabeledKnob(bounds, attackCurveLabel, attackCurveSlider);
    consumeLabeledKnob(bounds, attackLabel, attackSlider);
    consumeLabeledKnob(bounds, decayLabel, decaySlider);
    consumeLabeledKnob(bounds, releaseLabel, releaseSlider);
}
void OscComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedOscParams();
    auto& envParams = getSelectedEnvelopeParams();
    if (slider == &gainSlider) {
        *params.Gain = (float)gainSlider.getValue();
    } else if (slider == &attackCurveSlider) {
        *envParams.AttackCurve = (float)attackCurveSlider.getValue();
    } else if (slider == &attackSlider) {
        *envParams.Attack = (float)attackSlider.getValue();
    } else if (slider == &decaySlider) {
        *envParams.Decay = (float)decaySlider.getValue();
    } else if (slider == &releaseSlider) {
        *envParams.Release = (float)releaseSlider.getValue();
    }
}
void OscComponent::buttonClicked(juce::Button* button) {
    auto& params = getSelectedOscParams();
    if (button == &newEnvelopeToggle) {
        *params.NewEnvelope = newEnvelopeToggle.getToggleState();
    }
}
void OscComponent::timerCallback() {
    auto& params = getSelectedOscParams();
    gainSlider.setValue(params.Gain->get(), juce::dontSendNotification);

    auto newEnvelope = params.NewEnvelope->get();
    newEnvelopeToggle.setToggleState(newEnvelope, juce::dontSendNotification);
    attackCurveSlider.setEnabled(newEnvelope);
    attackSlider.setEnabled(newEnvelope);
    decaySlider.setEnabled(newEnvelope);
    releaseSlider.setEnabled(newEnvelope);
    attackCurveLabel.setEnabled(newEnvelope);
    attackLabel.setEnabled(newEnvelope);
    decayLabel.setEnabled(newEnvelope);
    releaseLabel.setEnabled(newEnvelope);

    auto& envParams = getSelectedEnvelopeParams();
    attackCurveSlider.setValue(envParams.AttackCurve->get(), juce::dontSendNotification);
    attackSlider.setValue(envParams.Attack->get(), juce::dontSendNotification);
    decaySlider.setValue(envParams.Decay->get(), juce::dontSendNotification);
    releaseSlider.setValue(envParams.Release->get(), juce::dontSendNotification);

    gainSlider.setLookAndFeel(&berryLookAndFeel);
}

//==============================================================================
HarmonicHeadComponent::HarmonicHeadComponent() {
    initLabel(gainLabel, "Gain", *this);
    initLabel(newEnvelopeLabel, "New Env", *this);
    initLabel(attackCurveLabel, "A. Curve", *this);
    initLabel(attackLabel, "Attack", *this);
    initLabel(decayLabel, "Decay", *this);
    initLabel(releaseLabel, "Release", *this);
}

HarmonicHeadComponent::~HarmonicHeadComponent() {}

void HarmonicHeadComponent::paint(juce::Graphics& g) {}

void HarmonicHeadComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    consumeLabel(bounds, HORIZONTAL_SLIDER_WIDTH, gainLabel);
    consumeLabel(bounds, 45, newEnvelopeLabel);
    consumeLabel(bounds, HORIZONTAL_SLIDER_WIDTH, attackCurveLabel);
    consumeLabel(bounds, HORIZONTAL_SLIDER_WIDTH, attackLabel);
    consumeLabel(bounds, HORIZONTAL_SLIDER_WIDTH, decayLabel);
    consumeLabel(bounds, HORIZONTAL_SLIDER_WIDTH, releaseLabel);
}

//==============================================================================
HarmonicBodyComponent::HarmonicBodyComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      newEnvelopeToggle(),
      gainSlider(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackCurveSlider(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackSlider(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextEntryBoxPosition::NoTextBox),
      decaySlider(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextEntryBoxPosition::NoTextBox),
      releaseSlider(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedOscParams();
    auto& envParams = getSelectedEnvelopeParams();

    auto formatGain = [](double gain) { return juce::String(juce::Decibels::gainToDecibels(gain), 2) + " dB"; };
    initSkewFromMid(gainSlider, params.Gain, 0.01f, nullptr, std::move(formatGain), this, *this);
    initChoiceToggle(newEnvelopeToggle, params.NewEnvelope, this, *this);
    initLinear(attackCurveSlider, envParams.AttackCurve, 0.01, this, *this);
    initSkewFromMid(attackSlider, envParams.Attack, 0.001, " sec", nullptr, this, *this);
    initSkewFromMid(decaySlider, envParams.Decay, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(releaseSlider, envParams.Release, 0.01, " sec", nullptr, this, *this);

    if (index == 0) {
        newEnvelopeToggle.setEnabled(false);
    }

    startTimerHz(30.0f);
}

HarmonicBodyComponent::~HarmonicBodyComponent() {}

void HarmonicBodyComponent::paint(juce::Graphics& g) {}

void HarmonicBodyComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    consumeHorizontalSlider(bounds, gainSlider);
    consumeToggle(bounds, 45, newEnvelopeToggle);
    consumeHorizontalSlider(bounds, attackCurveSlider);
    consumeHorizontalSlider(bounds, attackSlider);
    consumeHorizontalSlider(bounds, decaySlider);
    consumeHorizontalSlider(bounds, releaseSlider);
}
void HarmonicBodyComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getSelectedOscParams();
    auto& envParams = getSelectedEnvelopeParams();
    if (slider == &gainSlider) {
        *params.Gain = (float)gainSlider.getValue();
    } else if (slider == &attackCurveSlider) {
        *envParams.AttackCurve = (float)attackCurveSlider.getValue();
    } else if (slider == &attackSlider) {
        *envParams.Attack = (float)attackSlider.getValue();
    } else if (slider == &decaySlider) {
        *envParams.Decay = (float)decaySlider.getValue();
    } else if (slider == &releaseSlider) {
        *envParams.Release = (float)releaseSlider.getValue();
    }
}
void HarmonicBodyComponent::buttonClicked(juce::Button* button) {
    auto& params = getSelectedOscParams();
    if (button == &newEnvelopeToggle) {
        *params.NewEnvelope = newEnvelopeToggle.getToggleState();
    }
}
void HarmonicBodyComponent::timerCallback() {
    auto& params = getSelectedOscParams();
    auto& envParams = getSelectedEnvelopeParams();

    if (this->isMouseButtonDownAnywhere()) {
        auto xy = this->getMouseXYRelative();
        if (gainSlider.getBoundsInParent().contains(xy)) {
            double width = gainSlider.getWidth();
            double left = xy.x - gainSlider.getX();
            *params.Gain = (float)gainSlider.proportionOfLengthToValue(left / width);
        } else if (attackCurveSlider.getBoundsInParent().contains(xy)) {
            double width = attackCurveSlider.getWidth();
            double left = xy.x - attackCurveSlider.getX();
            *envParams.AttackCurve = (float)attackCurveSlider.proportionOfLengthToValue(left / width);
        } else if (attackSlider.getBoundsInParent().contains(xy)) {
            double width = attackSlider.getWidth();
            double left = xy.x - attackSlider.getX();
            *envParams.Attack = (float)attackSlider.proportionOfLengthToValue(left / width);
        } else if (decaySlider.getBoundsInParent().contains(xy)) {
            double width = decaySlider.getWidth();
            double left = xy.x - decaySlider.getX();
            *envParams.Decay = (float)decaySlider.proportionOfLengthToValue(left / width);
        } else if (releaseSlider.getBoundsInParent().contains(xy)) {
            double width = releaseSlider.getWidth();
            double left = xy.x - releaseSlider.getX();
            *envParams.Release = (float)releaseSlider.proportionOfLengthToValue(left / width);
        }
    }
    gainSlider.setValue(params.Gain->get(), juce::dontSendNotification);

    auto newEnvelope = params.NewEnvelope->get();
    newEnvelopeToggle.setToggleState(newEnvelope, juce::dontSendNotification);
    attackCurveSlider.setEnabled(newEnvelope);
    attackSlider.setEnabled(newEnvelope);
    decaySlider.setEnabled(newEnvelope);
    releaseSlider.setEnabled(newEnvelope);

    attackCurveSlider.setValue(envParams.AttackCurve->get(), juce::dontSendNotification);
    attackSlider.setValue(envParams.Attack->get(), juce::dontSendNotification);
    decaySlider.setValue(envParams.Decay->get(), juce::dontSendNotification);
    releaseSlider.setValue(envParams.Release->get(), juce::dontSendNotification);
}

//==============================================================================
FilterComponent::FilterComponent(int noiseIndex, int filterIndex, AllParams& allParams)
    : noiseIndex(noiseIndex),
      filterIndex(filterIndex),
      allParams(allParams),
      typeSelector("Type"),
      freqTypeToggle("Freq Type"),
      hzSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
      semitoneSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
      qSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
      gainSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox) {
    auto& params = getSelectedFilterParams();

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
    consumeLabeledComboBox(bounds, 120, typeLabel, typeSelector);
    consumeLabeledToggle(bounds, 35, freqTypeLabel, freqTypeToggle);
    consumeLabeledKnob(bounds, freqLabel, hzSlider, semitoneSlider);
    consumeLabeledKnob(bounds, qLabel, qSlider);
    consumeLabeledKnob(bounds, gainLabel, gainSlider);
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
    if (comboBoxThatHasChanged == &typeSelector) {
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
}

//==============================================================================
NoiseComponent::NoiseComponent(int index, AllParams& allParams)
    : index(index),
      allParams(allParams),
      gainSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
      typeSelector("Type"),
      attackCurveSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                        juce::Slider::TextEntryBoxPosition::NoTextBox),
      attackSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                   juce::Slider::TextEntryBoxPosition::NoTextBox),
      decaySlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
      releaseSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox),
      filters{FilterComponent{index, 0, allParams}, FilterComponent{index, 1, allParams}} {
    auto& params = getNoiseParams();
    auto& envParams = getEnvelopeParams();

    auto formatGain = [](double gain) { return juce::String(juce::Decibels::gainToDecibels(gain), 2) + " dB"; };
    initSkewFromMid(gainSlider, params.Gain, 0.01f, nullptr, std::move(formatGain), this, *this);
    initChoice(typeSelector, NOISE_WAVEFORM_NAMES, params.Waveform->getIndex(), this, *this);
    initLinear(attackCurveSlider, envParams.AttackCurve, 0.01, this, *this);
    initSkewFromMid(attackSlider, envParams.Attack, 0.001, " sec", nullptr, this, *this);
    initSkewFromMid(decaySlider, envParams.Decay, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(releaseSlider, envParams.Release, 0.01, " sec", nullptr, this, *this);

    initLabel(gainLabel, "Gain", *this);
    initLabel(typeLabel, "Type", *this);
    initLabel(attackCurveLabel, "A. Curve", *this);
    initLabel(attackLabel, "Attack", *this);
    initLabel(decayLabel, "Decay", *this);
    initLabel(releaseLabel, "Release", *this);

    for (auto& filter : filters) {
        addAndMakeVisible(filter);
    }

    startTimerHz(30.0f);
}

NoiseComponent::~NoiseComponent() {}

void NoiseComponent::paint(juce::Graphics& g) {}

void NoiseComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto height = bounds.getHeight();
    auto noiseArea = bounds.removeFromTop(height / 3);
    consumeLabeledKnob(noiseArea, gainLabel, gainSlider);
    consumeLabeledComboBox(noiseArea, 70, typeLabel, typeSelector);
    consumeLabeledKnob(noiseArea, attackCurveLabel, attackCurveSlider);
    consumeLabeledKnob(noiseArea, attackLabel, attackSlider);
    consumeLabeledKnob(noiseArea, decayLabel, decaySlider);
    consumeLabeledKnob(noiseArea, releaseLabel, releaseSlider);

    auto remainingHeight = bounds.getHeight();
    for (auto& filter : filters) {
        filter.setBounds(bounds.removeFromTop(remainingHeight / 2));
    }
}
void NoiseComponent::comboBoxChanged(juce::ComboBox* comboBox) {
    auto& params = getNoiseParams();
    if (comboBox == &typeSelector) {
        *params.Waveform = typeSelector.getSelectedItemIndex();
    }
}
void NoiseComponent::sliderValueChanged(juce::Slider* slider) {
    auto& params = getNoiseParams();
    auto& envParams = getEnvelopeParams();
    if (slider == &gainSlider) {
        *params.Gain = (float)gainSlider.getValue();
    } else if (slider == &attackCurveSlider) {
        *envParams.AttackCurve = (float)attackCurveSlider.getValue();
    } else if (slider == &attackSlider) {
        *envParams.Attack = (float)attackSlider.getValue();
    } else if (slider == &decaySlider) {
        *envParams.Decay = (float)decaySlider.getValue();
    } else if (slider == &releaseSlider) {
        *envParams.Release = (float)releaseSlider.getValue();
    }
}
void NoiseComponent::timerCallback() {
    auto& params = getNoiseParams();
    gainSlider.setValue(params.Gain->get(), juce::dontSendNotification);

    auto& envParams = getEnvelopeParams();
    attackCurveSlider.setValue(envParams.AttackCurve->get(), juce::dontSendNotification);
    attackSlider.setValue(envParams.Attack->get(), juce::dontSendNotification);
    decaySlider.setValue(envParams.Decay->get(), juce::dontSendNotification);
    releaseSlider.setValue(envParams.Release->get(), juce::dontSendNotification);

    gainSlider.setLookAndFeel(&berryLookAndFeel);
}

//==============================================================================
DelayComponent::DelayComponent(AllParams& allParams)
    : allParams(allParams),
      typeSelector("Type"),
      timeLSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
      timeRSlider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
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
    initSkewFromMid(timeLSlider, params.TimeL, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(timeRSlider, params.TimeR, 0.01, " sec", nullptr, this, *this);
    initSkewFromMid(lowFreqSlider, params.LowFreq, 1.0, " Hz", nullptr, this, *this);
    initSkewFromMid(highFreqSlider, params.HighFreq, 1.0, " Hz", nullptr, this, *this);
    initLinearPercent(feedbackSlider, params.Feedback, 0.01, this, *this);
    initLinear(mixSlider, params.Mix, 0.01, this, *this);

    initLabel(typeLabel, "Type", *this);
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
    consumeLabeledKnob(upperArea, lowFreqLabel, lowFreqSlider);
    consumeLabeledKnob(upperArea, highFreqLabel, highFreqSlider);
    consumeLabeledKnob(lowerArea, feedbackLabel, feedbackSlider);
    consumeLabeledKnob(lowerArea, mixLabel, mixSlider);
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
    timeLSlider.setValue(params.TimeL->get(), juce::dontSendNotification);
    timeRSlider.setValue(params.TimeR->get(), juce::dontSendNotification);

    lowFreqSlider.setValue(params.LowFreq->get(), juce::dontSendNotification);
    highFreqSlider.setValue(params.HighFreq->get(), juce::dontSendNotification);
    feedbackSlider.setValue(params.Feedback->get(), juce::dontSendNotification);
    mixSlider.setValue(params.Mix->get(), juce::dontSendNotification);

    mixSlider.setLookAndFeel(&berryLookAndFeel);
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
AnalyserToggle::AnalyserToggle(ANALYSER_MODE* analyserMode) : analyserMode(analyserMode), spectrumToggle("Spectrum") {
    spectrumToggle.addListener(this);
    addAndMakeVisible(spectrumToggle);

    spectrumToggle.setValue(*analyserMode == ANALYSER_MODE::Spectrum);
}
AnalyserToggle::~AnalyserToggle() {}
void AnalyserToggle::paint(juce::Graphics& g) {}
void AnalyserToggle::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    spectrumToggle.setBounds(bounds.removeFromTop(25));
}
void AnalyserToggle::toggleItemSelected(AnalyserToggleItem* toggleItem) {
    if (toggleItem == &spectrumToggle) {
        *analyserMode = ANALYSER_MODE::Spectrum;
    }
    spectrumToggle.setValue(*analyserMode == ANALYSER_MODE::Spectrum);
}

//==============================================================================
AnalyserWindow::AnalyserWindow(ANALYSER_MODE* analyserMode, LatestDataProvider* latestDataProvider)
    : analyserMode(analyserMode),
      latestDataProvider(latestDataProvider),
      forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann) {
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
    g.fillAll(colour::ANALYSER_BACKGROUND);

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
        }
    }
    g.setColour(colour::ANALYSER_BORDER);
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
