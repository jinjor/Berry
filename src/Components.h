#pragma once

#include <JuceHeader.h>

#include "LookAndFeel.h"
#include "Params.h"
#include "PluginProcessor.h"
#include "StyleConstants.h"

using namespace styles;

enum class ANALYSER_MODE { Spectrum };

//==============================================================================

class ArrowButton2 : public Button {
public:
    ArrowButton2(const String& buttonName, float arrowDirection, Colour arrowColour);
    ~ArrowButton2() override;
    void paintButton(Graphics&, bool, bool) override;

private:
    Colour colour;
    Path path;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArrowButton2)
};

//==============================================================================

class IncDecButton : public juce::Component, juce::Button::Listener, juce::Slider::Listener {
public:
    IncDecButton();
    virtual ~IncDecButton();
    IncDecButton(const IncDecButton&) = delete;
    ArrowButton2 incButton;
    ArrowButton2 decButton;
    juce::Label label;
    juce::Slider slider;
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;
    void setRange(int min, int max);
    void setValue(int newValue, NotificationType notification);
    int getValue();

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void incDecValueChanged(IncDecButton*) = 0;
    };
    void addListener(Listener* newListener);
    void removeListener(Listener* listener);
    virtual void buttonClicked(juce::Button* button) override;

private:
    int min = 0;
    int max = 1;
    int value = 0;
    std::string name;
    ListenerList<Listener> listeners;
    virtual void sliderValueChanged(juce::Slider* slider) override;
};

//==============================================================================
class ComponentHelper {
protected:
    BerryLookAndFeel berryLookAndFeel;
    BerryLookAndFeel berryLookAndFeelControlled = BerryLookAndFeel(true);
    juce::Font paramLabelFont = juce::Font(PARAM_LABEL_FONT_SIZE, juce::Font::plain).withTypefaceStyle("Regular");
    juce::Font paramValueLabelFont =
        juce::Font(PARAM_VALUE_LABEL_FONT_SIZE, juce::Font::plain).withTypefaceStyle("Regular");

    void initLabel(juce::Label& label,
                   int fontSize,
                   std::string&& typeFaceStyle,
                   juce::Justification justification,
                   std::string&& text,
                   juce::Component& parent) {
        juce::Font paramLabelFont = juce::Font(fontSize, juce::Font::plain).withTypefaceStyle(typeFaceStyle);

        label.setFont(paramLabelFont);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(justification);
        label.setInterceptsMouseClicks(false, false);
        parent.addAndMakeVisible(label);
    }
    void initLabel(juce::Label& label, std::string&& text, juce::Component& parent) {
        label.setFont(paramLabelFont);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setEditable(false, false, false);
        parent.addAndMakeVisible(label);
    }
    void initStatusValue(juce::Label& label, std::string&& text, juce::Component& parent) {
        label.setFont(paramValueLabelFont);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::left);
        label.setEditable(false, false, false);
        parent.addAndMakeVisible(label);
    }
    void initStatusKey(juce::Label& label, std::string&& text, juce::Component& parent) {
        label.setFont(paramLabelFont);
        label.setText(text + ":", juce::dontSendNotification);
        label.setJustificationType(juce::Justification::right);
        label.setEditable(false, false, false);
        parent.addAndMakeVisible(label);
    }
    void initChoice(juce::ComboBox& box,
                    const juce::StringArray& allValueStrings,
                    int selectedIndex,
                    juce::ComboBox::Listener* listener,
                    juce::Component& parent) {
        box.setLookAndFeel(&berryLookAndFeel);
        box.addItemList(allValueStrings, 1);
        box.setSelectedItemIndex(selectedIndex, juce::dontSendNotification);
        box.setJustificationType(juce::Justification::centred);
        box.addListener(listener);
        parent.addAndMakeVisible(box);
    }
    void initChoice(juce::ComboBox& box,
                    juce::AudioParameterChoice* param,
                    juce::ComboBox::Listener* listener,
                    juce::Component& parent) {
        initChoice(box, param->getAllValueStrings(), param->getIndex(), listener, parent);
    }
    void initChoice(juce::ComboBox& box,
                    juce::AudioParameterBool* param,
                    juce::ComboBox::Listener* listener,
                    juce::Component& parent) {
        initChoice(box, param->getAllValueStrings(), param->get(), listener, parent);
    }
    void initChoiceToggle(juce::ToggleButton& toggle,
                          int checkIndex,
                          juce::AudioParameterChoice* param,
                          juce::ToggleButton::Listener* listener,
                          juce::Component& parent) {
        toggle.setLookAndFeel(&berryLookAndFeel);
        toggle.addListener(listener);
        toggle.setButtonText("");
        toggle.setToggleState(param->getIndex() == checkIndex, juce::dontSendNotification);
        parent.addAndMakeVisible(toggle);
    }
    void initChoiceToggle(juce::ToggleButton& toggle,
                          juce::AudioParameterBool* param,
                          juce::ToggleButton::Listener* listener,
                          juce::Component& parent) {
        toggle.setLookAndFeel(&berryLookAndFeel);
        toggle.addListener(listener);
        toggle.setButtonText("");
        toggle.setToggleState(param->get(), juce::dontSendNotification);
        parent.addAndMakeVisible(toggle);
    }
    void initSkewFromMid(juce::Slider& slider,
                         juce::AudioParameterFloat* param,
                         float step,
                         const char* unit,
                         std::function<juce::String(double)>&& format,
                         juce::Slider::Listener* listener,
                         juce::Component& parent) {
        slider.setLookAndFeel(&berryLookAndFeel);
        auto nrange = NormalisableRange<double>{
            param->range.start, param->range.end, step, param->range.skew, param->range.symmetricSkew};
        slider.setNormalisableRange(nrange);
        slider.setValue(param->get(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        if (unit != nullptr) {
            slider.setTextValueSuffix(unit);
        }
        if (format != nullptr) {
            slider.textFromValueFunction = format;
        }
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterFloat* param,
                    float step,
                    const char* unit,
                    std::function<juce::String(double)>&& format,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        slider.setLookAndFeel(&berryLookAndFeel);
        slider.setRange(param->range.start, param->range.end, step);
        slider.setValue(param->get(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        if (unit != nullptr) {
            slider.setTextValueSuffix(unit);
        }
        if (format != nullptr) {
            slider.textFromValueFunction = format;
        }
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterInt* param,
                    float step,
                    const char* unit,
                    std::function<juce::String(double)>&& format,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        slider.setLookAndFeel(&berryLookAndFeel);
        slider.setRange(param->getRange().getStart(), param->getRange().getEnd(), step);
        slider.setValue(param->get(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        if (unit != nullptr) {
            slider.setTextValueSuffix(unit);
        }
        if (format != nullptr) {
            slider.textFromValueFunction = format;
        }
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterFloat* param,
                    float step,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        initLinear(slider, param, step, nullptr, nullptr, listener, parent);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterInt* param,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        initLinear(slider, param, 1, nullptr, nullptr, listener, parent);
    }
    void initLinearPercent(juce::Slider& slider,
                           juce::AudioParameterFloat* param,
                           float step,
                           juce::Slider::Listener* listener,
                           juce::Component& parent) {
        auto f = [](double gain) { return juce::String(gain * 100, 0) + " %"; };
        initLinear(slider, param, step, nullptr, std::move(f), listener, parent);
    }
    void initIncDec(IncDecButton& incDec,
                    juce::AudioParameterInt* param,
                    IncDecButton::Listener* listener,
                    juce::Component& parent) {
        incDec.setLookAndFeel(&berryLookAndFeel);
        incDec.setRange(param->getRange().getStart(), param->getRange().getEnd());
        incDec.setValue(param->get(), juce::dontSendNotification);
        incDec.addListener(listener);
        parent.addAndMakeVisible(incDec);
    }
    void initEnum(juce::Slider& slider,
                  juce::AudioParameterChoice* param,
                  juce::Slider::Listener* listener,
                  juce::Component& parent) {
        const juce::StringArray& values = param->getAllValueStrings();
        slider.setLookAndFeel(&berryLookAndFeel);
        slider.setRange(0, values.size() - 1, 1);
        slider.setValue(param->getIndex(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        slider.textFromValueFunction = [values](double index) { return values[index]; };
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void consumeLabel(juce::Rectangle<int>& parentArea, int width, juce::Label& label) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
    }
    void consumeLabeledKnob(juce::Rectangle<int>& parentArea, juce::Label& label, juce::Slider& knob) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(SLIDER_WIDTH);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        knob.setBounds(area.removeFromTop(KNOB_HEIGHT));
    }
    void consumeLabeledKnob(juce::Rectangle<int>& parentArea,
                            juce::Label& label,
                            juce::Slider& knob1,
                            juce::Slider& knob2) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(SLIDER_WIDTH);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        auto knobBounds = area.removeFromTop(KNOB_HEIGHT);
        knob1.setBounds(knobBounds);
        knob2.setBounds(knobBounds);
    }
    void consumeLabeledKnob(juce::Rectangle<int>& parentArea,
                            juce::Label& label1,
                            juce::Slider& knob1,
                            juce::Label& label2,
                            juce::Slider& knob2) {
        auto copied = parentArea;
        consumeLabeledKnob(parentArea, label1, knob1);
        consumeLabeledKnob(copied, label2, knob2);
    }
    void consumeHorizontalSlider(juce::Rectangle<int>& parentArea, juce::Slider& slider) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        slider.setBounds(parentArea.removeFromLeft(HORIZONTAL_SLIDER_WIDTH).removeFromTop(HORIZONTAL_SLIDER_HEIGHT));
    }
    void consumeLabeledComboBox(juce::Rectangle<int>& parentArea, int width, juce::Label& label, juce::Component& box) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        box.setBounds(area.removeFromTop(COMBO_BOX_HEIGHT));
    }
    void consumeLabeledIncDecButton(juce::Rectangle<int>& parentArea,
                                    int width,
                                    juce::Label& label,
                                    juce::Component& button) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        button.setBounds(area.removeFromTop(KNOB_HEIGHT));  // TODO
    }
    void consumeLabeledToggle(juce::Rectangle<int>& parentArea,
                              int width,
                              juce::Label& label,
                              juce::ToggleButton& toggle) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        auto padding = (width - CHECKBOX_SIZE) / 2.0f;
        toggle.setBounds(
            area.removeFromTop(CHECKBOX_SIZE).removeFromLeft(padding + CHECKBOX_SIZE).removeFromRight(CHECKBOX_SIZE));
    }
    void consumeToggle(juce::Rectangle<int>& parentArea, int width, juce::ToggleButton& toggle) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        auto padding = (width - CHECKBOX_SIZE) / 2.0f;
        toggle.setBounds(
            area.removeFromTop(CHECKBOX_SIZE).removeFromLeft(padding + CHECKBOX_SIZE).removeFromRight(CHECKBOX_SIZE));
    }
    void consumeKeyValueText(
        juce::Rectangle<int>& parentArea, int height, int width, juce::Label& keyLabel, juce::Label& valueLabel) {
        auto area = parentArea.removeFromTop(height);
        keyLabel.setBounds(area.removeFromLeft(width));
        area.removeFromLeft(3);
        valueLabel.setBounds(area);
    }
};

//==============================================================================

enum class HEADER_CHECK { Hidden, Disabled, Enabled };

class HeaderComponent : public juce::Component {
public:
    HeaderComponent(std::string name, HEADER_CHECK check);
    virtual ~HeaderComponent();
    HeaderComponent(const HeaderComponent&) = delete;
    juce::ToggleButton enabledButton;
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    std::string name;
    HEADER_CHECK check;
};

//==============================================================================

class SectionComponent : public juce::Component, juce::Button::Listener {
public:
    SectionComponent(std::string name, HEADER_CHECK check, std::unique_ptr<juce::Component> body);
    virtual ~SectionComponent();
    SectionComponent(const SectionComponent&) = delete;
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void enabledChanged(SectionComponent*) = 0;
    };
    void addListener(Listener* newListener);
    void setEnabled(bool enabled);
    bool getEnabled();

private:
    BerryLookAndFeel berryLookAndFeel;
    HeaderComponent header;
    std::unique_ptr<juce::Component> body;
    ListenerList<Listener> listeners;
    virtual void buttonClicked(juce::Button* button) override;
};

//==============================================================================
class VoiceComponent : public juce::Component, IncDecButton::Listener, private juce::Timer, ComponentHelper {
public:
    VoiceComponent(AllParams& allParams);
    virtual ~VoiceComponent();
    VoiceComponent(const VoiceComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    virtual void incDecValueChanged(IncDecButton* button) override;
    virtual void timerCallback() override;

    AllParams& allParams;

    IncDecButton pitchBendRangeButton;

    juce::Label pitchBendRangeLabel;
};

//==============================================================================
class UtilComponent : public juce::Component, juce::Button::Listener, private ComponentHelper {
public:
    UtilComponent(BerryAudioProcessor& processor);
    virtual ~UtilComponent();
    UtilComponent(const UtilComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;

    virtual void resized() override;

private:
    BerryAudioProcessor& processor;

    virtual void buttonClicked(juce::Button* button) override;

    juce::TextButton copyToClipboardButton;
    juce::TextButton pasteFromClipboardButton;

    juce::Label copyToClipboardLabel;
    juce::Label pasteFromClipboardLabel;
};

//==============================================================================
class StatusComponent : public juce::Component, private juce::Timer, ComponentHelper {
public:
    StatusComponent(int* polyphony, TimeConsumptionState* timeConsumptionState, LatestDataProvider* latestDataProvider);
    virtual ~StatusComponent();
    StatusComponent(const StatusComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    virtual void timerCallback() override;
    int* polyphony;
    TimeConsumptionState* timeConsumptionState;
    LatestDataProvider* latestDataProvider;

    juce::Label volumeValueLabel;
    juce::Label polyphonyValueLabel;
    juce::Label timeConsumptionValueLabel;

    juce::Label volumeLabel;
    juce::Label polyphonyLabel;
    juce::Label timeConsumptionLabel;

    float levelDataL[2048];
    float levelDataR[2048];
    LatestDataProvider::Consumer levelConsumer{levelDataL, levelDataR, 2048, false};
    float overflowedLevel = 0;
    int overflowWarning = 0;
};

//==============================================================================
class MasterComponent : public juce::Component, juce::Slider::Listener, private juce::Timer, ComponentHelper {
public:
    MasterComponent(AllParams& allParams);
    virtual ~MasterComponent();
    MasterComponent(const MasterComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;

    virtual void resized() override;

private:
    virtual void sliderValueChanged(juce::Slider* slider) override;
    virtual void timerCallback() override;

    AllParams& allParams;

    juce::Slider panSlider;
    juce::Slider volumeSlider;

    juce::Label panLabel;
    juce::Label volumeLabel;
};

//==============================================================================
namespace {
constexpr int NUM_KEYS = MAX_OF_88_NOTES - MIN_OF_88_NOTES + 1;
constexpr float KEY_POSITIONS[12] = {0,
                                     -1.0f / 12,
                                     1.0f / 7,
                                     -3.0f / 12,
                                     2.0f / 7,
                                     3.0f / 7,
                                     -6.0f / 12,
                                     4.0f / 7,
                                     -8.0f / 12,
                                     5.0f / 7,
                                     -10.0f / 12,
                                     6.0f / 7};
}  // namespace

class FocusedNote : private juce::Timer {
public:
    FocusedNote(AllParams& allParams, MonoStack& monoStack);
    virtual ~FocusedNote();
    FocusedNote(const FocusedNote&) = delete;

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void focusedNoteChanged(FocusedNote*) = 0;
    };
    void addListener(Listener* newListener);
    void removeListener(Listener* listener);

    int getFocusedNote() { return focusedNote; }

private:
    ListenerList<Listener> listeners;
    AllParams& allParams;
    MonoStack& monoStack;
    std::array<int, NUM_TIMBRES> timbreNoteNumbers{};
    int focusedNote = 0;
    virtual void timerCallback() override;
};

class KeyComponent : public juce::Component {
public:
    KeyComponent();
    virtual ~KeyComponent();
    KeyComponent(const KeyComponent&) = delete;

    void update(bool isBlack, bool isOn);
    void setFocused(bool isFocused);

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    bool isBlack = false;
    bool isOn = false;
    bool isFocused = false;
};

class TimbreNote : public juce::Component {
public:
    TimbreNote(int index, AllParams& allParams);
    virtual ~TimbreNote();
    TimbreNote(const TimbreNote&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    int index;
    AllParams& allParams;
};

//==============================================================================
class KeyboardComponent : public juce::Component, private FocusedNote::Listener, juce::Timer, ComponentHelper {
public:
    KeyboardComponent(AllParams& allParams, juce::MidiKeyboardState& keyboardState, FocusedNote& focusedNote);
    virtual ~KeyboardComponent();
    KeyboardComponent(const KeyboardComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    virtual void focusedNoteChanged(FocusedNote* focusedNote) override;
    virtual void timerCallback() override;

    AllParams& allParams;
    juce::MidiKeyboardState& keyboardState;
    FocusedNote& focusedNote;

    std::array<juce::Label, NUM_TIMBRES> timbreLabels;
    std::array<TimbreNote, NUM_TIMBRES> timbreNotes;
    std::array<KeyComponent, NUM_KEYS> keys;

    virtual void mouseDown(const juce::MouseEvent& e) override;
    virtual void mouseDrag(const juce::MouseEvent& e) override;
};

//==============================================================================
class TimbreHeadComponent : public juce::Component, private juce::Timer, ComponentHelper {
public:
    TimbreHeadComponent(AllParams& allParams);
    virtual ~TimbreHeadComponent();
    TimbreHeadComponent(const TimbreHeadComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    AllParams& allParams;
    juce::Label nameLabel;
    virtual void timerCallback() override;
};

//==============================================================================
class HarmonicHeadComponent : public juce::Component, private ComponentHelper {
public:
    HarmonicHeadComponent();
    virtual ~HarmonicHeadComponent();
    HarmonicHeadComponent(const HarmonicHeadComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    juce::Label nameLabel;
    juce::Label muteLabel;
    juce::Label soloLabel;
    juce::Label gainLabel;
    juce::Label attackCurveLabel;
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label releaseLabel;
};

//==============================================================================
class HarmonicComponent : public juce::Component, juce::Slider::Listener, private juce::Timer, ComponentHelper {
public:
    HarmonicComponent(bool isNoise, int index, AllParams& allParams, FocusedNote& focusedNote);
    virtual ~HarmonicComponent();
    HarmonicComponent(const HarmonicComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;
    void setFocusedParams(std::shared_ptr<CalculatedParams> params);

private:
    FocusedNote& focusedNote;
    virtual void sliderValueChanged(juce::Slider* slider) override;
    virtual void timerCallback() override;
    bool isNoise;
    int index;
    std::shared_ptr<CalculatedParams> focusedParams;

    AllParams& allParams;

    juce::Label nameLabel;
    juce::Label soloToggle;
    juce::Label muteToggle;
    juce::Slider gainSlider;
    juce::Slider attackCurveSlider;
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider releaseSlider;

    AudioParameterFloat* getSelectedGainParam() {
        return isNoise ? allParams.getCurrentMainParams().noiseParams[index].Gain
                       : allParams.getCurrentMainParams().oscParams[index].Gain;
    }
    AudioParameterFloat* getSelectedAttackCurveParam() {
        return isNoise ? allParams.getCurrentMainParams().noiseEnvelopeParams[index].AttackCurve
                       : allParams.getCurrentMainParams().envelopeParams[index].AttackCurve;
    }
    AudioParameterFloat* getSelectedAttackParam() {
        return isNoise ? allParams.getCurrentMainParams().noiseEnvelopeParams[index].Attack
                       : allParams.getCurrentMainParams().envelopeParams[index].Attack;
    }
    AudioParameterFloat* getSelectedDecayParam() {
        return isNoise ? allParams.getCurrentMainParams().noiseEnvelopeParams[index].Decay
                       : allParams.getCurrentMainParams().envelopeParams[index].Decay;
    }
    AudioParameterFloat* getSelectedReleaseParam() {
        return isNoise ? allParams.getCurrentMainParams().noiseEnvelopeParams[index].Release
                       : allParams.getCurrentMainParams().envelopeParams[index].Release;
    }
    virtual void mouseDown(const juce::MouseEvent& e) override;
    void paintFocusedParam(juce::Graphics& g, juce::Slider& bar, double value);
};

//==============================================================================
class HarmonicsComponent : public juce::Component, private ComponentHelper, FocusedNote::Listener {
public:
    HarmonicsComponent(AllParams& allParams, FocusedNote& focusedNote);
    virtual ~HarmonicsComponent();
    HarmonicsComponent(const HarmonicsComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    AllParams& allParams;
    HarmonicHeadComponent head;
    std::array<HarmonicComponent, NUM_OSC> harmonics;
    virtual void focusedNoteChanged(FocusedNote* focusedNote) override;
};

//==============================================================================
class NoisesComponent : public juce::Component, private ComponentHelper, FocusedNote::Listener {
public:
    NoisesComponent(AllParams& allParams, FocusedNote& focusedNote);
    virtual ~NoisesComponent();
    NoisesComponent(const NoisesComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    AllParams& allParams;
    HarmonicHeadComponent head;
    std::array<HarmonicComponent, NUM_NOISE> noises;
    virtual void focusedNoteChanged(FocusedNote* focusedNote) override;
};

//==============================================================================
class FilterComponent : public juce::Component,
                        juce::ToggleButton::Listener,
                        juce::ComboBox::Listener,
                        juce::Slider::Listener,
                        private juce::Timer,
                        ComponentHelper {
public:
    FilterComponent(int noiseIndex, int filterIndex, AllParams& allParams);
    virtual ~FilterComponent();
    FilterComponent(const FilterComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;

    virtual void resized() override;

private:
    virtual void buttonClicked(juce::Button* button) override;
    virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    virtual void sliderValueChanged(juce::Slider* slider) override;
    virtual void timerCallback() override;
    int noiseIndex;
    int filterIndex;

    AllParams& allParams;

    juce::ComboBox typeSelector;
    juce::ToggleButton freqTypeToggle;
    juce::Slider hzSlider;
    juce::Slider semitoneSlider;
    juce::Slider qSlider;
    juce::Slider gainSlider;

    juce::Label targetLabel;
    juce::Label typeLabel;
    juce::Label freqTypeLabel;
    juce::Label freqLabel;
    juce::Label qLabel;
    juce::Label gainLabel;

    FilterParams& getSelectedFilterParams() { return allParams.noiseUnitParams[noiseIndex].filterParams[filterIndex]; }
};

//==============================================================================
class NoiseComponent : public juce::Component, juce::ComboBox::Listener, private juce::Timer, ComponentHelper {
public:
    NoiseComponent(int index, AllParams& allParams);
    virtual ~NoiseComponent();
    NoiseComponent(const NoiseComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    virtual void comboBoxChanged(juce::ComboBox* comboBox) override;
    virtual void timerCallback() override;
    int index;

    AllParams& allParams;

    juce::ComboBox typeSelector;

    juce::Label typeLabel;

    std::array<FilterComponent, NUM_NOISE_FILTER> filters;

    NoiseUnitParams& getNoiseUnitParams() { return allParams.noiseUnitParams[index]; }
};

//==============================================================================
class DelayComponent : public juce::Component,
                       juce::ComboBox::Listener,
                       juce::Slider::Listener,
                       private juce::Timer,
                       ComponentHelper {
public:
    DelayComponent(AllParams& allParams);
    virtual ~DelayComponent();
    DelayComponent(const DelayComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    virtual void comboBoxChanged(juce::ComboBox* comboBox) override;
    virtual void sliderValueChanged(juce::Slider* slider) override;
    virtual void timerCallback() override;

    AllParams& allParams;

    juce::ComboBox typeSelector;
    juce::Slider timeLSlider;
    juce::Slider timeRSlider;
    juce::Slider lowFreqSlider;
    juce::Slider highFreqSlider;
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;

    juce::Label typeLabel;
    juce::Label timeLLabel;
    juce::Label timeRLabel;
    juce::Label lowFreqLabel;
    juce::Label highFreqLabel;
    juce::Label feedbackLabel;
    juce::Label mixLabel;

    DelayParams& getSelectedDelayParams() { return allParams.delayParams; }
};

//==============================================================================
class AnalyserToggleItem : public juce::Component, private ComponentHelper {
public:
    AnalyserToggleItem(std::string name);
    virtual ~AnalyserToggleItem();
    AnalyserToggleItem(const AnalyserToggleItem&) = delete;

    void setValue(bool value) {
        this->value = value;
        repaint();
    };
    bool getValue() { return value; };

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void toggleItemSelected(AnalyserToggleItem*) = 0;
    };
    void addListener(Listener* e);

private:
    virtual void mouseUp(const juce::MouseEvent& e) override;

    juce::ListenerList<Listener> listeners;
    ANALYSER_MODE* analyserMode;
    juce::Label nameLabel;
    bool value;
};

//==============================================================================
class AnalyserToggle : public juce::Component, private AnalyserToggleItem::Listener {
public:
    AnalyserToggle(ANALYSER_MODE* analyserMode);
    virtual ~AnalyserToggle();
    AnalyserToggle(const AnalyserToggle&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    ANALYSER_MODE* analyserMode;
    AnalyserToggleItem spectrumToggle;

    virtual void toggleItemSelected(AnalyserToggleItem* toggleItem) override;
};

//==============================================================================
class AnalyserWindow : public juce::Component, private juce::Timer {
public:
    AnalyserWindow(ANALYSER_MODE* analyserMode, LatestDataProvider* latestDataProvider);
    virtual ~AnalyserWindow();
    AnalyserWindow(const AnalyserWindow&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    enum { scopeSize = 512 };
    ANALYSER_MODE* analyserMode;
    LatestDataProvider* latestDataProvider;
    ANALYSER_MODE lastAnalyserMode = ANALYSER_MODE::Spectrum;

    // FFT
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    static const int fftOrder = 11;
    static const int fftSize = 2048;
    float fftData[fftSize * 2];
    LatestDataProvider::Consumer fftConsumer{fftData, fftData + fftSize, fftSize, false};
    float scopeData[scopeSize]{};
    bool readyToDrawFrame = false;

    // Level
    float levelDataL[2048];
    float levelDataR[2048];
    LatestDataProvider::Consumer levelConsumer{levelDataL, levelDataR, 2048, false};
    float currentLevel[2]{};
    float overflowedLevelL = 0;
    float overflowedLevelR = 0;
    int overflowWarningL = 0;
    int overflowWarningR = 0;

    // methods
    virtual void timerCallback() override;
    bool drawNextFrameOfSpectrum();
    bool drawNextFrameOfLevel();
    void paintSpectrum(
        juce::Graphics& g, juce::Colour colour, int offsetX, int offsetY, int width, int height, float* scopeData);
    void paintLevel(juce::Graphics& g, int offsetX, int offsetY, int width, int height, float level);
    static float xToHz(float minFreq, float maxFreq, float notmalizedX) {
        return minFreq * std::pow(maxFreq / minFreq, notmalizedX);
    }
    static float getFFTDataByHz(float* processedFFTData, float fftSize, float sampleRate, float hz) {
        float indexFloat = hz * ((fftSize * 0.5) / (sampleRate * 0.5));
        int index = indexFloat;
        float frac = indexFloat - index;
        return processedFFTData[index] * (1 - frac) + processedFFTData[index + 1] * frac;
    }
};
