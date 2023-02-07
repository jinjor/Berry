#include "Voice.h"

#include "Params.h"

//==============================================================================
BerryVoice::BerryVoice(CurrentPositionInfo *currentPositionInfo,
                       juce::AudioBuffer<float> &buffer,
                       GlobalParams &globalParams,
                       VoiceParams &voiceParams,
                       MainParams &mainParams)
    : perf(juce::PerformanceCounter("voice cycle", 100000)),
      currentPositionInfo(currentPositionInfo),
      buffer(buffer),
      globalParams(globalParams),
      voiceParams(voiceParams),
      mainParams(mainParams),
      oscs{MultiOsc(), MultiOsc(), MultiOsc(), MultiOsc(), MultiOsc(), MultiOsc(), MultiOsc(), MultiOsc(), MultiOsc()},
      adsr{Adsr(), Adsr()},
      filters{Filter(), Filter()},
      modEnvs{Adsr(), Adsr(), Adsr()} {}
BerryVoice::~BerryVoice() { DBG("BerryVoice's destructor called."); }
bool BerryVoice::canPlaySound(juce::SynthesiserSound *sound) {
    if (dynamic_cast<BerrySound *>(sound) != nullptr) {
        return true;
    }
    return false;
}
void BerryVoice::startNote(int midiNoteNumber,
                           float velocity,
                           juce::SynthesiserSound *sound,
                           int currentPitchWheelPosition) {
    DBG("startNote() midiNoteNumber:" << midiNoteNumber);
    noteNumberAtStart = midiNoteNumber;
    if (BerrySound *playingSound = dynamic_cast<BerrySound *>(sound)) {
        auto sampleRate = getSampleRate();
        auto &mainParams = getMainParams();
        smoothNote.init(midiNoteNumber);
        if (stolen) {
            smoothVelocity.exponentialInfinite(0.01, velocity, sampleRate);
        } else {
            smoothVelocity.init(velocity);
        }
        stolen = false;

        auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control

        for (int i = 0; i < NUM_OSC; ++i) {
            if (!stolen) {
                //                oscs[i].setAngle(0.0);
                oscs[i].initializePastData();
            }
            auto &params = mainParams.envelopeParams[i];
            adsr[i].setParams(params.attackCurve, params.attack, 0.0, params.decay, 0.0, params.release);
            adsr[i].doAttack(fixedSampleRate);
        }
        for (int i = 0; i < NUM_FILTER; ++i) {
            filters[i].initializePastData();
        }
        for (int i = 0; i < NUM_MODENV; ++i) {
            auto &params = mainParams.modEnvParams[i];
            if (params.shouldUseHold()) {
                modEnvs[i].setParams(0.5, 0.0, params.wait, params.decay, 0.0, 0.0);
            } else {
                modEnvs[i].setParams(0.5, params.attack, 0.0, params.decay, 0.0, 0.0);
            }
            modEnvs[i].doAttack(fixedSampleRate);
        }
        stepCounter = 0;
    }
}
void BerryVoice::stopNote(float velocity, bool allowTailOff) {
    DBG("stopNote() allowTailOff:" << std::to_string(allowTailOff));
    if (BerrySound *playingSound = dynamic_cast<BerrySound *>(getCurrentlyPlayingSound().get())) {
        if (allowTailOff) {
            auto sampleRate = getSampleRate();
            auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control
            for (int i = 0; i < NUM_OSC; ++i) {
                if (adsr[i].isReleasing()) {
                    continue;
                }
                adsr[i].doRelease(fixedSampleRate);
            }
        } else {
            stolen = true;
            for (int i = 0; i < NUM_OSC; ++i) {
                oscs[i].setSampleRate(0.0);  // stop
            }
            for (int i = 0; i < NUM_OSC; ++i) {
                adsr[i].forceStop();
            }
            clearCurrentNote();
        }
    }
}
void BerryVoice::mute(double duration) {
    DBG("mute()");
    auto sampleRate = getSampleRate();
    auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control
    for (int i = 0; i < NUM_OSC; ++i) {
        adsr[i].doRelease(fixedSampleRate, duration);
    }
}
void BerryVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) {
    if (BerrySound *playingSound = dynamic_cast<BerrySound *>(getCurrentlyPlayingSound().get())) {
        // DBG("startSample: " + std::to_string(startSample));
        // DBG("numSamples: " + std::to_string(numSamples));
        if (getCurrentlyPlayingNote() == 0) {
            return;
        }
        auto sampleRate = getSampleRate();

        applyParamsBeforeLoop(sampleRate);

        int numChannels = outputBuffer.getNumChannels();
        jassert(numChannels <= 2);
        auto &buffer = getBuffer();
        while (--numSamples >= 0) {
            double out[2]{0, 0};
            auto active = step(out, sampleRate, numChannels);
            for (auto ch = 0; ch < numChannels; ++ch) {
                buffer.addSample(ch, startSample, out[ch]);
            }
            ++startSample;
            if (!active) {
                clearCurrentNote();
                break;
            }
        }
    }
}
void BerryVoice::applyParamsBeforeLoop(double sampleRate) {
    auto &mainParams = getMainParams();
    for (int i = 0; i < NUM_OSC; ++i) {
        oscs[i].setSampleRate(sampleRate);
        auto &params = mainParams.envelopeParams[i];
        adsr[i].setParams(params.attackCurve, params.attack, 0.0, params.decay, 0.0, params.release);
    }
    for (int i = 0; i < NUM_FILTER; ++i) {
        filters[i].setSampleRate(sampleRate);
    }
    for (int i = 0; i < NUM_MODENV; ++i) {
        auto &params = mainParams.modEnvParams[i];
        if (params.shouldUseHold()) {
            modEnvs[i].setParams(0.5, 0.0, params.wait, params.decay, 0.0, 0.0);
        } else {
            modEnvs[i].setParams(0.5, params.attack, 0.0, params.decay, 0.0, 0.0);
        }
    }
}
bool BerryVoice::step(double *out, double sampleRate, int numChannels) {
    auto &mainParams = getMainParams();
    smoothNote.step();
    smoothVelocity.step();

    double midiNoteNumber = smoothNote.value + globalParams.pitch * voiceParams.pitchBendRange;
    auto baseFreq = getMidiNoteInHertzDouble(midiNoteNumber);

    if (stepCounter == 0) {
        auto fixedSampleRate = sampleRate * CONTROL_RATE;
        for (int i = 0; i < NUM_OSC; ++i) {
            adsr[i].step(fixedSampleRate);
        }
        controlModifiers = Modifiers{};
        updateModifiersByModEnv(controlModifiers, fixedSampleRate);
    }
    stepCounter++;
    if (stepCounter >= CONTROL_INTERVAL) {
        stepCounter = 0;
    }

    auto modifiers = controlModifiers;  // copy;

    bool active = false;
    auto panBase = mainParams.masterParams.pan;
    if (globalParams.pan >= 0) {
        panBase = (1 - panBase) * globalParams.pan + panBase;
    } else {
        panBase = (1 + panBase) * globalParams.pan + panBase;
    }

    auto panModAmp = std::min(1.0 - panBase, 1.0 + panBase);
    // ---------------- OSC with Envelope and Filter ----------------
    for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
        auto &p = mainParams.oscParams[oscIndex];
        if (!p.enabled) {
            continue;
        }
        int envelopeIndex = 0;
        for (int i = oscIndex; i >= 1; i--) {
            auto &p = mainParams.oscParams[i];
            if (!p.syncEnvelope) {
                envelopeIndex = i;
                break;
            }
        }
        if (!adsr[envelopeIndex].isActive()) {
            continue;
        }
        active = true;
        auto freq = baseFreq * (oscIndex + 1);
        auto pan = panBase + panModAmp * modifiers.panMod[oscIndex];
        jassert(pan >= -1);
        jassert(pan <= 1);

        double o[2]{0, 0};
        auto sineGain = adsr[envelopeIndex].getValue() * p.gain;

        oscs[oscIndex].step(pan, freq, 0.0, sineGain, o);

        out[0] += o[0];
        out[1] += o[1];
    }
    for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
        auto &fp = mainParams.filterParams[filterIndex];
        if (!fp.enabled) {
            continue;
        }
        if (true) {
            auto filterType = fp.type;
            double freq;
            if (fp.isFreqAbsoluteFreezed) {
                double noteShift = modifiers.filterOctShift[filterIndex] * 12;
                freq = shiftHertsByNotes(fp.hz, noteShift);
            } else {
                double shiftedNoteNumber = midiNoteNumber + fp.semitone + modifiers.filterOctShift[filterIndex] * 12;
                freq = getMidiNoteInHertzDouble(shiftedNoteNumber);
            }
            auto q = fp.q;
            if (modifiers.filterQExp[filterIndex] != 1.0) {
                q = std::pow(q, modifiers.filterQExp[filterIndex]);
            }
            auto gain = fp.gain;
            for (auto ch = 0; ch < numChannels; ++ch) {
                out[ch] = filters[filterIndex].step(filterType, freq, q, gain, ch, out[ch]);
            }
        }
    }
    auto finalGain = 0.3 * smoothVelocity.value;
    for (auto ch = 0; ch < numChannels; ++ch) {
        out[ch] *= finalGain;
    }
    return active;
    // for (auto ch = 0; ch < numChannels; ++ch) {
    //     outputBuffer.addSample (ch, startSample, out[ch]);
    // }
    // ++startSample;
    // if(!active) {
    //     clearCurrentNote();
    //     break;
    // }
}

void BerryVoice::updateModifiersByModEnv(Modifiers &modifiers, double sampleRate) {
    auto &mainParams = getMainParams();
    for (int i = 0; i < NUM_MODENV; ++i) {
        auto &params = mainParams.modEnvParams[i];
        if (!params.enabled) {
            continue;
        }
        modEnvs[i].step(sampleRate);
        auto modEnvValue = modEnvs[i].getValue();
        switch (params.targetType) {
            case MODENV_TARGET_TYPE::Filter: {
                int targetIndex = params.targetFilter;
                switch (params.targetFilterParam) {
                    case MODENV_TARGET_FILTER_PARAM::Freq: {
                        auto v = params.peakFreq * modEnvValue;
                        if (targetIndex == NUM_FILTER) {
                            for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
                                modifiers.filterOctShift[filterIndex] += v;
                            }
                        } else {
                            modifiers.filterOctShift[targetIndex] += v;
                        }
                        break;
                    }
                    case MODENV_TARGET_FILTER_PARAM::Q: {
                        auto v = params.fadeIn ? 1 - modEnvValue : modEnvValue;
                        if (targetIndex == NUM_FILTER) {
                            for (int filterIndex = 0; filterIndex < NUM_FILTER; ++filterIndex) {
                                modifiers.filterQExp[filterIndex] *= v;
                            }
                        } else {
                            modifiers.filterQExp[targetIndex] *= v;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}
