#pragma once
#include <JuceHeader.h>
#include <array>

class PepeLAudioProcessor : public juce::AudioProcessor
{
public:
    PepeLAudioProcessor();
    ~PepeLAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PEPEL Glue"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inputLevel { 0.0f };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    using SmLin = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>;
    using SmMul = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>;

    void updateTargets (bool snap);

    double sr = 44100.0;
    float env = 0.f, gainEnv = 1.f;

    // указатели на параметры кэшируются один раз (в processBlock нет поиска по строкам)
    std::atomic<float> *pInGain{}, *pHpf{}, *pThr{}, *pRatio{}, *pAtk{}, *pRel{}, *pKnee{}, *pMakeup{},
                       *pDrive{}, *pChar{}, *pSatMix{}, *pExc{}, *pExcFreq{}, *pMix{}, *pInten{};

    SmLin sInGain, sMakeup, sDrive, sChar, sSatMix, sExc, sMix, sThr, sRatio, sKnee;
    SmMul sHpf, sExcFreq;

    juce::dsp::StateVariableTPTFilter<float> hpf, excHpf1, excHpf2;
    float dcX[2] {}, dcY[2] {};
    float dcR = 0.999f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PepeLAudioProcessor)
};
