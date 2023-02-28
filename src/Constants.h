#pragma once

namespace {
const int NUM_TIMBRES = 4;
const int NUM_OSC = 16;
const int NUM_NOISE = 2;
const int NUM_NOISE_FILTER = 2;
const int MIN_OF_88_NOTES = 21;   // A0
const int MAX_OF_88_NOTES = 108;  // C8
const int DEFAULT_TIMBRE_NOTES[NUM_TIMBRES] = {MIN_OF_88_NOTES, 48, 72, MAX_OF_88_NOTES};
}  // namespace