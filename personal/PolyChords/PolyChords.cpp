#include "daisy_patch.h"
#include "daisysp.h"

#include <stdio.h>

using namespace daisy;
using namespace daisysp;

DaisyPatch hw_;

Oscillator osc_[3];

void UpdateControls();
void UpdateOled();

void AudioCallback(float **in, float **out, size_t size)
{
    UpdateControls();
    for(size_t i = 0; i < size; i++)
    {
        float mix = 0;
        //Process and output the three oscillators
        for(size_t chn = 0; chn < 3; chn++)
        {
            float sig = osc_[chn].Process();
            mix += sig * .25f;
            out[chn][i] = sig;
        }

        //output the mixed oscillators
        out[3][i] = mix;
    }
}

void SetupOsc(float samplerate)
{
    for(int i = 0; i < 3; i++)
    {
        osc_[i].Init(samplerate);
        osc_[i].SetAmp(.7);
        osc_[i].SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    }
}

int main(void)
{
    hw_.Init();

    SetupOsc(hw_.AudioSampleRate());

    hw_.StartAdc();
    hw_.StartAudio(AudioCallback);
    while(1)
    {
        UpdateOled();
    }
}

float ctrl_value = 0.f;

void UpdateControls()
{
    hw_.ProcessDigitalControls();
    hw_.ProcessAnalogControls();

    //knobs
    float ctrl[4];
    for(int i = 0; i < 4; i++)
    {
        ctrl[i] = hw_.GetKnobValue((DaisyPatch::Ctrl)i);
    }

    ctrl_value = ctrl[0];
    // ctrl[0] set the root note (really root frequency in this case)
    float root = ctrl[0];
    root = root * 5.f;           //voltage
    root = powf(2.f, root) * 55; //Hz

    osc_[0].SetFreq(root);

    // Ratio for the major 3rd: 5:4
    constexpr float THIRD_RATIO = 5.f / 4.f;
    osc_[1].SetFreq(root * THIRD_RATIO);

    // Ratio for the perfect 5th: 3:2
    constexpr float FIFTH_RATIO = 3.f / 2.f;
    osc_[2].SetFreq(root * FIFTH_RATIO);
}

void UpdateOled()
{
    hw_.display.Fill(false);

    hw_.display.SetCursor(0, 0);
    std::string str  = "PolyChord";
    char *      cstr = &str[0];
    hw_.display.WriteString(cstr, Font_7x10, true);

    hw_.display.Update();
}
