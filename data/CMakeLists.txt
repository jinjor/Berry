add_executable(wavetable-gen WavetableGen.cpp)
target_compile_features(wavetable-gen PUBLIC cxx_std_17)

add_custom_command(
    OUTPUT
        lookup
        parabola
        saw
        sine
    COMMAND wavetable-gen
    DEPENDS wavetable-gen
    VERBATIM)

add_custom_target(wavetables
    DEPENDS
        lookup
        parabola
        saw
        sine
    VERBATIM)

juce_add_binary_data(BerryPluginBinaries SOURCES 
    ${CMAKE_BUILD_DIR}/${CMAKE_CURRENT_BINARY_DIR}/lookup
    ${CMAKE_BUILD_DIR}/${CMAKE_CURRENT_BINARY_DIR}/parabola
    ${CMAKE_BUILD_DIR}/${CMAKE_CURRENT_BINARY_DIR}/saw
    ${CMAKE_BUILD_DIR}/${CMAKE_CURRENT_BINARY_DIR}/sine
)
add_dependencies(BerryPluginBinaries wavetables)