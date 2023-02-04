#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>


namespace {
const int NUM_DIVISIONS = 4095;
const int NUM_SAMPLES = NUM_DIVISIONS + 1;
const float PI = 3.141592f;
const std::string SINE_FILE = "sine";
}  // namespace

int main() {
    {
        std::cout << "generating sine table..." << std::endl;
        float* sineTable = new float[NUM_SAMPLES]();

        for (int i = 0; i < NUM_SAMPLES; i++) {
            float angle = 2.0 * PI / NUM_DIVISIONS * i;
            float value = std::sin(angle);
            sineTable[i] = value;
        }

        std::ofstream ofs(SINE_FILE, std::ios::out | std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(sineTable), NUM_SAMPLES * sizeof(float));
        delete[] sineTable;
    }
    return 0;
}

// ref: https://www.musicdsp.org/en/latest/Synthesis/17-bandlimited-waveform-generation.html
