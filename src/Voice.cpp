#include "Voice.h"

#include "Params.h"

//==============================================================================
BerryVoice::BerryVoice(CurrentPositionInfo *currentPositionInfo, juce::AudioBuffer<float> &buffer, AllParams &allParams)
    : perf(juce::PerformanceCounter("voice cycle", 100000)),
      currentPositionInfo(currentPositionInfo),
      buffer(buffer),
      allParams(allParams),
      oscs{MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(false),
           MultiOsc(true)},
      adsr{Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr(),
           Adsr()},
      noises{Osc{}, Osc{}},
      noiseAdsr{Adsr(), Adsr()},
      noiseFilters{Filter{}, Filter{}, Filter{}, Filter{}} {}
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
        smoothNote.init(midiNoteNumber);
        if (stolen) {
            smoothVelocity.exponentialInfinite(0.01, velocity, sampleRate);
        } else {
            smoothVelocity.init(velocity);
        }
        stolen = false;

        auto fixedSampleRate = sampleRate * CONTROL_RATE;  // for control
        auto calculatedParams = CalculatedParams{};
        auto calculatedNoiseParams = CalculatedParams{};
        calculateParamsBeforeLoop(calculatedParams, calculatedNoiseParams);

        for (int i = 0; i < NUM_OSC; ++i) {
            if (!stolen) {
                //                oscs[i].setAngle(0.0);
            }
            oscs[i].setSampleRate(sampleRate);
            adsr[i].setParams(calculatedParams.attackCurve[i],
                              calculatedParams.attack[i],
                              0.0,
                              calculatedParams.decay[i],
                              0.0,
                              calculatedParams.release[i]);
            adsr[i].doAttack(fixedSampleRate);
        }
        for (int i = 0; i < NUM_NOISE; ++i) {
            noises[i].setSampleRate(sampleRate);
            noises[i].setWaveform(allParams.noiseUnitParams[i].waveform, true);
            noiseAdsr[i].setParams(calculatedNoiseParams.attackCurve[i],
                                   calculatedNoiseParams.attack[i],
                                   0.0,
                                   calculatedNoiseParams.decay[i],
                                   0.0,
                                   calculatedNoiseParams.release[i]);
            noiseAdsr[i].doAttack(fixedSampleRate);
            for (int j = 0; j < NUM_NOISE_FILTER; ++j) {
                noiseFilters[i][j].initializePastData();
                noiseFilters[i][j].setSampleRate(sampleRate);
            }
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
            for (int i = 0; i < NUM_NOISE; ++i) {
                if (noiseAdsr[i].isReleasing()) {
                    continue;
                }
                noiseAdsr[i].doRelease(fixedSampleRate);
            }
        } else {
            stolen = true;
            for (int i = 0; i < NUM_OSC; ++i) {
                oscs[i].setSampleRate(0.0);  // stop
            }
            for (int i = 0; i < NUM_OSC; ++i) {
                adsr[i].forceStop();
            }
            for (int i = 0; i < NUM_NOISE; ++i) {
                noiseAdsr[i].forceStop();
            }
            clearCurrentNote();
        }
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

        auto calculatedParams = CalculatedParams{};
        auto calculatedNoiseParams = CalculatedParams{};
        calculateParamsBeforeLoop(calculatedParams, calculatedNoiseParams);
        applyParamsBeforeLoop(sampleRate, calculatedParams, calculatedNoiseParams);

        int numChannels = outputBuffer.getNumChannels();
        jassert(numChannels <= 2);
        while (--numSamples >= 0) {
            double out[2]{0, 0};
            auto active = step(out, sampleRate, numChannels, calculatedParams, calculatedNoiseParams);
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
void BerryVoice::applyParamsBeforeLoop(double sampleRate, CalculatedParams &params, CalculatedParams &noiseParams) {
    for (int i = 0; i < NUM_OSC; ++i) {
        oscs[i].setSampleRate(sampleRate);
        adsr[i].setParams(params.attackCurve[i], params.attack[i], 0.0, params.decay[i], 0.0, params.release[i]);
    }
    for (int i = 0; i < NUM_NOISE; ++i) {
        noises[i].setSampleRate(sampleRate);
        noises[i].setWaveform(allParams.noiseUnitParams[i].waveform, true);
        noiseAdsr[i].setParams(
            noiseParams.attackCurve[i], noiseParams.attack[i], 0.0, noiseParams.decay[i], 0.0, noiseParams.release[i]);
    }
}
void BerryVoice::calculateParamsBeforeLoop(CalculatedParams &params, CalculatedParams &noiseParams) {
    auto leftIndex = 0;
    auto rightIndex = NUM_TIMBRES - 1;
    auto leftNote = 0;
    auto rightNote = 127;
    for (int i = 0; i < NUM_TIMBRES; i++) {
        if (noteNumberAtStart <= allParams.mainParams[i].noteNumber) {
            rightIndex = i;
            rightNote = allParams.mainParams[i].noteNumber;
            break;
        }
    }
    for (int i = NUM_TIMBRES - 1; i >= 0; i--) {
        if (allParams.mainParams[i].noteNumber < noteNumberAtStart) {
            leftIndex = i;
            leftNote = allParams.mainParams[i].noteNumber;
            break;
        }
    }
    jassert(rightNote - leftNote > 0);
    auto leftRatio = (double)(rightNote - noteNumberAtStart) / (double)(rightNote - leftNote);
    auto rightRatio = 1.0 - leftRatio;
    auto &leftParams = allParams.mainParams[leftIndex];
    auto &rightParams = allParams.mainParams[rightIndex];
    for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
        auto &leftOsc = leftParams.oscParams[oscIndex];
        auto &rightOsc = rightParams.oscParams[oscIndex];
        auto &leftEnv = leftParams.envelopeParams[oscIndex];
        auto &rightEnv = rightParams.envelopeParams[oscIndex];
        params.gain[oscIndex] = leftOsc.gain * leftRatio + rightOsc.gain * rightRatio;
        params.attackCurve[oscIndex] = leftEnv.attackCurve * leftRatio + rightEnv.attackCurve * rightRatio;
        params.attack[oscIndex] = leftEnv.attack * leftRatio + rightEnv.attack * rightRatio;
        params.decay[oscIndex] = leftEnv.decay * leftRatio + rightEnv.decay * rightRatio;
        params.release[oscIndex] = leftEnv.release * leftRatio + rightEnv.release * rightRatio;
    }
    for (int noiseIndex = 0; noiseIndex < NUM_NOISE; ++noiseIndex) {
        auto &leftOsc = leftParams.noiseParams[noiseIndex];
        auto &rightOsc = rightParams.noiseParams[noiseIndex];
        auto &leftEnv = leftParams.noiseEnvelopeParams[noiseIndex];
        auto &rightEnv = rightParams.noiseEnvelopeParams[noiseIndex];
        noiseParams.gain[noiseIndex] = leftOsc.gain * leftRatio + rightOsc.gain * rightRatio;
        noiseParams.attackCurve[noiseIndex] = leftEnv.attackCurve * leftRatio + rightEnv.attackCurve * rightRatio;
        noiseParams.attack[noiseIndex] = leftEnv.attack * leftRatio + rightEnv.attack * rightRatio;
        noiseParams.decay[noiseIndex] = leftEnv.decay * leftRatio + rightEnv.decay * rightRatio;
        noiseParams.release[noiseIndex] = leftEnv.release * leftRatio + rightEnv.release * rightRatio;
    }
}
bool BerryVoice::step(
    double *out, double sampleRate, int numChannels, CalculatedParams &params, CalculatedParams &noiseParams) {
    smoothNote.step();
    smoothVelocity.step();

    double midiNoteNumber = smoothNote.value + allParams.globalParams.pitch * allParams.voiceParams.pitchBendRange;
    auto baseFreq = getMidiNoteInHertzDouble(midiNoteNumber);

    if (stepCounter == 0) {
        auto fixedSampleRate = sampleRate * CONTROL_RATE;
        for (int i = 0; i < NUM_OSC; ++i) {
            adsr[i].step(fixedSampleRate);
        }
        for (int i = 0; i < NUM_NOISE; ++i) {
            noiseAdsr[i].step(fixedSampleRate);
        }
    }
    stepCounter++;
    if (stepCounter >= CONTROL_INTERVAL) {
        stepCounter = 0;
    }

    bool active = false;
    auto panBase = allParams.masterParams.pan;
    if (allParams.globalParams.pan >= 0) {
        panBase = (1 - panBase) * allParams.globalParams.pan + panBase;
    } else {
        panBase = (1 + panBase) * allParams.globalParams.pan + panBase;
    }

    // ---------------- OSC with Envelope and Filter ----------------
    for (int oscIndex = 0; oscIndex < NUM_OSC; ++oscIndex) {
        if (!adsr[oscIndex].isActive()) {
            continue;
        }
        active = true;
        auto freq = oscIndex == NUM_OSC - 1 ? baseFreq : baseFreq * (oscIndex + 1);
        auto pan = panBase;
        jassert(pan >= -1);
        jassert(pan <= 1);

        double o[2]{0, 0};
        auto sineGain = adsr[oscIndex].getValue() * params.gain[oscIndex];

        oscs[oscIndex].step(pan, freq, 0.0, sineGain, o);

        out[0] += o[0];
        out[1] += o[1];
    }
    for (int noiseIndex = 0; noiseIndex < NUM_NOISE; ++noiseIndex) {
        if (!noiseAdsr[noiseIndex].isActive()) {
            continue;
        }
        active = true;
        auto &noiseUnitParams = allParams.noiseUnitParams[noiseIndex];

        auto gain = noiseAdsr[noiseIndex].getValue() * noiseParams.gain[noiseIndex];
        auto value = noises[noiseIndex].step(440, 0.0) * gain;
        double o[2]{value, value};

        for (int filterIndex = 0; filterIndex < NUM_NOISE_FILTER; ++filterIndex) {
            auto &fp = noiseUnitParams.filterParams[filterIndex];
            if (!fp.enabled) {
                continue;
            }
            auto freq = fp.isFreqAbsoluteFreezed ? fp.hz : getMidiNoteInHertzDouble(noteNumberAtStart + fp.semitone);
            for (auto ch = 0; ch < numChannels; ++ch) {
                o[ch] = noiseFilters[noiseIndex][filterIndex].step(fp.type, freq, fp.q, fp.gain, ch, o[ch]);
            }
        }
        out[0] += o[0];
        out[1] += o[1];
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
