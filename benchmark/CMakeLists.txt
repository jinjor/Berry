juce_add_console_app(BerryBenchmarks)

target_compile_features(BerryBenchmarks PUBLIC cxx_std_17)

juce_generate_juce_header(BerryBenchmarks)

target_link_libraries(BerryBenchmarks
  PUBLIC
    benchmark::benchmark
    gtest_main
    BerryPluginBinaries
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_cryptography
    juce::juce_data_structures
    juce::juce_dsp
)

target_sources(BerryBenchmarks
  PRIVATE
    Benchmark.cpp
    ../src/Params.cpp
    ../src/Voice.cpp
)