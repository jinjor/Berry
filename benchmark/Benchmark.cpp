#include <JuceHeader.h>
#include <benchmark/benchmark.h>

#include "../src/Params.h"
#include "../src/Voice.h"

static void doStepLoop(benchmark::State& state, AllParams& p) {
    CurrentPositionInfo currentPositionInfo{};
    juce::AudioBuffer<float> buf{};

    std::vector<std::unique_ptr<juce::AudioBuffer<float>>> buffers{};
    buffers.reserve(129);
    for (int i = 0; i < 129; i++) {
        buffers.push_back(std::make_unique<juce::AudioBuffer<float>>(2, 0));
    }
    BerryVoice voice{&currentPositionInfo, buffers, p.globalParams, p.voiceParams, p.mainParamList};
    auto numChannels = 2;
    auto sampleRate = 48000;
    double out[2]{0, 0};

    BerrySound sound = BerrySound(p.voiceParams, p.mainParamList);

    p.freeze();
    voice.startNote(60, 1.0, &sound, 8192);
    voice.applyParamsBeforeLoop(sampleRate);
    for (auto _ : state) {
        voice.step(out, sampleRate, numChannels);
    }
}

static void BM_VoiceStep_empty(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_empty);

static void BM_VoiceStep_single_whitenoise(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("White");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_whitenoise);

static void BM_VoiceStep_single_pinknoise(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Pink");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_pinknoise);

static void BM_VoiceStep_single_sine(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine);

static void BM_VoiceStep_single_square(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Square");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_square);

static void BM_VoiceStep_single_sine_with_vibrato(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[0].Unison = 4;
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("OSC");
    *p.mainParamList[128].lfoParams[0].TargetOscParam = LFO_TARGET_OSC_PARAM_NAMES.indexOf("Vibrato");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine_with_vibrato);

static void BM_VoiceStep_single_sine_with_tremolo(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[0].Unison = 4;
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("OSC");
    *p.mainParamList[128].lfoParams[0].TargetOscParam = LFO_TARGET_OSC_PARAM_NAMES.indexOf("Tremolo");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine_with_tremolo);

static void BM_VoiceStep_single_sine_unison(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[0].Unison = 4;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine_unison);

static void BM_VoiceStep_single_sine_unison_with_autopan(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[0].Unison = 4;
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("OSC");
    *p.mainParamList[128].lfoParams[0].TargetOscParam = LFO_TARGET_OSC_PARAM_NAMES.indexOf("Pan");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine_unison_with_autopan);

static void BM_VoiceStep_multiple_sine(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[1].Enabled = true;
    *p.mainParamList[128].oscParams[1].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[2].Enabled = true;
    *p.mainParamList[128].oscParams[2].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_sine);

static void BM_VoiceStep_single_abs_filter(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_abs_filter);

static void BM_VoiceStep_single_abs_filter_with_lfo_filterfreq(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.mainParamList[128].lfoParams[0].TargetFilterParam = LFO_TARGET_FILTER_PARAM_NAMES.indexOf("Freq");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_abs_filter_with_lfo_filterfreq);

static void BM_VoiceStep_single_abs_lowshelf_with_lfo_filterfreq(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[0].Type = FILTER_TYPE_NAMES.indexOf("LowShelf");
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.mainParamList[128].lfoParams[0].TargetFilterParam = LFO_TARGET_FILTER_PARAM_NAMES.indexOf("Freq");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_abs_lowshelf_with_lfo_filterfreq);

static void BM_VoiceStep_single_abs_filter_with_lfo_filterq(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.mainParamList[128].lfoParams[0].TargetFilterParam = LFO_TARGET_FILTER_PARAM_NAMES.indexOf("Q");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_abs_filter_with_lfo_filterq);

static void BM_VoiceStep_single_rel_filter(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[0].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_rel_filter);

static void BM_VoiceStep_single_rel_filter_with_vibrato(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[0].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("OSC");
    *p.mainParamList[128].lfoParams[0].TargetOscParam = LFO_TARGET_OSC_PARAM_NAMES.indexOf("Vibrato");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_rel_filter_with_vibrato);

static void BM_VoiceStep_multiple_abs_filter(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[1].Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_abs_filter);

static void BM_VoiceStep_multiple_rel_filter(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[0].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.mainParamList[128].filterParams[1].Enabled = true;
    *p.mainParamList[128].filterParams[1].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_rel_filter);

static void BM_VoiceStep_multiple_rel_filter_with_vibrato(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[0].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.mainParamList[128].filterParams[1].Enabled = true;
    *p.mainParamList[128].filterParams[1].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[0].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("OSC");
    *p.mainParamList[128].lfoParams[0].TargetOscParam = LFO_TARGET_OSC_PARAM_NAMES.indexOf("Vibrato");
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_rel_filter_with_vibrato);

static void BM_VoiceStep_full(benchmark::State& state) {
    AllParams p{};
    *p.mainParamList[128].oscParams[0].Enabled = true;
    *p.mainParamList[128].oscParams[0].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[1].Enabled = true;
    *p.mainParamList[128].oscParams[1].Waveform = OSC_WAVEFORM_NAMES.indexOf("Square");
    *p.mainParamList[128].oscParams[1].Edge = 0.5;
    *p.mainParamList[128].oscParams[2].Enabled = true;
    *p.mainParamList[128].oscParams[2].Waveform = OSC_WAVEFORM_NAMES.indexOf("Sine");
    *p.mainParamList[128].oscParams[2].Unison = 4;
    *p.mainParamList[128].filterParams[0].Enabled = true;
    *p.mainParamList[128].filterParams[1].Enabled = true;
    *p.mainParamList[128].filterParams[1].FreqType = FILTER_FREQ_TYPE_NAMES.indexOf("Rel");
    *p.mainParamList[128].lfoParams[0].Enabled = true;
    *p.mainParamList[128].lfoParams[1].Enabled = true;
    *p.mainParamList[128].lfoParams[1].TargetOscParam = LFO_TARGET_OSC_PARAM_NAMES.indexOf("FM");
    *p.mainParamList[128].lfoParams[2].Enabled = true;
    *p.mainParamList[128].lfoParams[2].TargetType = LFO_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.mainParamList[128].lfoParams[2].TargetFilterParam = LFO_TARGET_FILTER_PARAM_NAMES.indexOf("Freq");
    *p.mainParamList[128].modEnvParams[0].Enabled = true;
    *p.mainParamList[128].modEnvParams[0].TargetType = MODENV_TARGET_TYPE_NAMES.indexOf("OSC");
    *p.mainParamList[128].modEnvParams[0].TargetOscParam = MODENV_TARGET_OSC_PARAM_NAMES.indexOf("Freq");
    *p.mainParamList[128].modEnvParams[1].Enabled = true;
    *p.mainParamList[128].modEnvParams[1].TargetType = MODENV_TARGET_TYPE_NAMES.indexOf("Filter");
    *p.mainParamList[128].modEnvParams[1].TargetFilterParam = MODENV_TARGET_FILTER_PARAM_NAMES.indexOf("Freq");
    *p.mainParamList[128].modEnvParams[2].Enabled = true;
    *p.mainParamList[128].modEnvParams[2].TargetType = MODENV_TARGET_TYPE_NAMES.indexOf("LFO");
    *p.mainParamList[128].modEnvParams[2].TargetLfoParam = MODENV_TARGET_LFO_PARAM_NAMES.indexOf("Freq");
    *p.mainParamList[128].delayParams.Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_full);

static void BM_DelayStep(benchmark::State& state) {
    auto numChannels = 2;
    auto sampleRate = 48000;
    auto bpm = 120;
    StereoDelay stereoDelay{};

    stereoDelay.setParams(sampleRate,
                          bpm,
                          DELAY_TYPE::Parallel,
                          false,  // Sync
                          0.3,    // TimeL
                          0.3,    // TimeR
                          0.125,  // TimeSyncL
                          0.125,  // TimeSyncR
                          100,    // LowFreq
                          4000,   // HighFreq
                          0.3,    // Feedback
                          0.3);   // Mix
    juce::Random whiteNoise;
    for (auto _ : state) {
        double s = whiteNoise.nextDouble();
        double sample[2]{s, s};
        stereoDelay.step(sample);
    }
}
BENCHMARK(BM_DelayStep);

BENCHMARK_MAIN();