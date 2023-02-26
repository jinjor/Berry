#include <JuceHeader.h>
#include <benchmark/benchmark.h>

#include "../src/Params.h"
#include "../src/Voice.h"

static void doStepLoop(benchmark::State& state, AllParams& p) {
    CurrentPositionInfo currentPositionInfo{};
    juce::AudioBuffer<float> buffer{2, 0};

    BerryVoice voice{&currentPositionInfo, buffer, p};
    auto numChannels = 2;
    auto sampleRate = 48000;
    double out[2]{0, 0};

    BerrySound sound = BerrySound();

    p.freeze();
    voice.startNote(60, 1.0, &sound, 8192);
    auto calculatedParams = CalculatedParams{};
    auto noiseCalculatedParams = CalculatedParams{};
    voice.calculateParamsBeforeLoop(calculatedParams, noiseCalculatedParams);
    voice.applyParamsBeforeLoop(sampleRate, calculatedParams, noiseCalculatedParams);
    for (auto _ : state) {
        voice.step(out, sampleRate, numChannels, calculatedParams, noiseCalculatedParams);
    }
}

static void BM_VoiceStep_empty(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_empty);

static void BM_VoiceStep_single_sine(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_sine);

static void BM_VoiceStep_multiple_sine(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_sine);

static void BM_VoiceStep_single_abs_filter(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_single_abs_filter);

static void BM_VoiceStep_multiple_abs_filter(benchmark::State& state) {
    AllParams p{};
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_multiple_abs_filter);

static void BM_VoiceStep_full(benchmark::State& state) {
    AllParams p{};
    *p.delayParams.Enabled = true;
    doStepLoop(state, p);
}
BENCHMARK(BM_VoiceStep_full);

static void BM_DelayStep(benchmark::State& state) {
    auto numChannels = 2;
    auto sampleRate = 48000;
    StereoDelay stereoDelay{};

    stereoDelay.setParams(sampleRate,
                          DELAY_TYPE::Parallel,
                          0.3,   // TimeL
                          0.3,   // TimeR
                          100,   // LowFreq
                          4000,  // HighFreq
                          0.3,   // Feedback
                          0.3);  // Mix
    juce::Random whiteNoise;
    for (auto _ : state) {
        double s = whiteNoise.nextDouble();
        double sample[2]{s, s};
        stereoDelay.step(sample);
    }
}
BENCHMARK(BM_DelayStep);

BENCHMARK_MAIN();