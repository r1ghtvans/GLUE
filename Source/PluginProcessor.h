#pragma once
#include <JuceHeader.h>

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
    double sr = 44100.0;
    float env = 0.f, gainEnv = 1.f;
    float lastHpf = 30.f, lastExcFreq = 4500.f;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> hpf;
    std::array<juce::dsp::IIR::Filter<float>, 2> excHpf1, excHpf2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PepeLAudioProcessor)
};
