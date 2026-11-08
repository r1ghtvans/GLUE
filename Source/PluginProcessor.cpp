#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout PepeLAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    auto add = [&](auto p){ params.push_back(std::move(p)); };
    add(std::make_unique<AudioParameterFloat>("inputGain","Input Gain",NormalisableRange<float>(-24.f,24.f,0.1f),0.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>("hpf","HP Filter",NormalisableRange<float>(20.f,300.f,1.f),30.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" Hz"; })));
    add(std::make_unique<AudioParameterFloat>("threshold","Threshold",NormalisableRange<float>(-60.f,0.f,0.1f),-18.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>("ratio","Ratio",NormalisableRange<float>(1.f,8.f,0.1f),2.5f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" :1"; })));
    add(std::make_unique<AudioParameterFloat>("attack","Attack",NormalisableRange<float>(1.f,200.f,1.f),30.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" ms"; })));
    add(std::make_unique<AudioParameterFloat>("release","Release",NormalisableRange<float>(20.f,1000.f,1.f),250.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" ms"; })));
    add(std::make_unique<AudioParameterFloat>("knee","Knee",NormalisableRange<float>(0.f,24.f,0.1f),12.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>("makeup","Makeup",NormalisableRange<float>(-12.f,12.f,0.1f),0.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>("drive","Drive",NormalisableRange<float>(0.f,24.f,0.1f),8.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>("char","Character",NormalisableRange<float>(0.f,100.f,1.f),35.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>("satMix","Sat Mix",NormalisableRange<float>(0.f,100.f,1.f),55.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>("exciter","Exciter",NormalisableRange<float>(0.f,100.f,1.f),20.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>("excFreq","Exc Freq",NormalisableRange<float>(1000.f,10000.f,10.f),4500.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" Hz"; })));
    add(std::make_unique<AudioParameterFloat>("mix","Mix",NormalisableRange<float>(0.f,100.f,1.f),100.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>("intensity","Intensity",NormalisableRange<float>(0.f,100.f,1.f),100.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterChoice>("preset","Preset",StringArray{"CLEAN","GLUE","DIRTY","AIR"},1));
    return { params.begin(), params.end() };
}

PepeLAudioProcessor::PepeLAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

void PepeLAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    sr = sampleRate;
    auto hp = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate,30.f);
    auto exc = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate,4500.f);
    for (int ch=0;ch<2;++ch) { hpf[ch].coefficients = hp; excHpf1[ch].coefficients = exc; excHpf2[ch].coefficients = exc; }
    env=0; gainEnv=1.f; lastHpf=30.f; lastExcFreq=4500.f;
}

void PepeLAudioProcessor::releaseResources(){}

bool PepeLAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo()) return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
    return true;
}

void PepeLAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    int numChannels = getTotalNumInputChannels();
    int n = buffer.getNumSamples();
    if (n==0) return;

    auto* rawInGain = apvts.getRawParameterValue("inputGain");
    auto* rawHpf    = apvts.getRawParameterValue("hpf");
    auto* rawThr    = apvts.getRawParameterValue("threshold");
    auto* rawRatio  = apvts.getRawParameterValue("ratio");
    auto* rawAtk    = apvts.getRawParameterValue("attack");
    auto* rawRel    = apvts.getRawParameterValue("release");
    auto* rawKnee   = apvts.getRawParameterValue("knee");
    auto* rawMakeup = apvts.getRawParameterValue("makeup");
    auto* rawDrive  = apvts.getRawParameterValue("drive");
    auto* rawChar   = apvts.getRawParameterValue("char");
    auto* rawSatMix = apvts.getRawParameterValue("satMix");
    auto* rawExc    = apvts.getRawParameterValue("exciter");
    auto* rawExcF   = apvts.getRawParameterValue("excFreq");
    auto* rawMix    = apvts.getRawParameterValue("mix");
    auto* rawInt    = apvts.getRawParameterValue("intensity");

    float inGain = Decibels::decibelsToGain(rawInGain->load());
    float hpfFreq = rawHpf->load();
    float thrDb = rawThr->load();
    float ratio = rawRatio->load();
    float atkMs = rawAtk->load();
    float relMs = rawRel->load();
    float knee = rawKnee->load();
    float makeup = Decibels::decibelsToGain(rawMakeup->load());
    float driveDb = rawDrive->load();
    float charC = rawChar->load()*0.01f;
    float satMix = rawSatMix->load()*0.01f;
    float excAmt = rawExc->load()*0.01f;
    float excFreq = rawExcF->load();
    float mix = rawMix->load()*0.01f;
    float inten = rawInt->load()*0.01f;

    if (std::abs(hpfFreq-lastHpf)>0.5f){ auto c = dsp::IIR::Coefficients<float>::makeHighPass(sr,hpfFreq); hpf[0].coefficients=c; hpf[1].coefficients=c; lastHpf=hpfFreq; }
    if (std::abs(excFreq-lastExcFreq)>10.f){ auto c = dsp::IIR::Coefficients<float>::makeHighPass(sr,excFreq); for(int ch=0;ch<2;++ch){ excHpf1[ch].coefficients=c; excHpf2[ch].coefficients=c; } lastExcFreq=excFreq; }

    float ratioE = 1.f + (ratio-1.f)*inten;
    float driveLin = Decibels::decibelsToGain(driveDb*inten);
    float satMixE = satMix*inten;
    float excE = excAmt*inten;

    float atkC = std::exp(-1.f/(0.001f*atkMs*sr));
    float relC = std::exp(-1.f/(0.001f*relMs*sr));
    float gAtkC = std::exp(-1.f/(0.005*sr));
    float gRelC = relC;

    float peak = 0.f;
    float* l = buffer.getWritePointer(0);
    float* r = numChannels>1 ? buffer.getWritePointer(1) : l;

    for (int i=0;i<n;++i){
        float dryL = l[i];
        float dryR = numChannels>1 ? r[i] : dryL;
        peak = std::max(peak, std::max(std::abs(dryL), std::abs(dryR)));

        float xL = hpf[0].processSample(dryL*inGain);
        float xR = hpf[1].processSample(dryR*inGain);

        float det = std::max(std::abs(xL), std::abs(xR));
        env = det > env ? det + (env-det)*atkC : det + (env-det)*relC;

        float over = Decibels::gainToDecibels(env + 1e-6f) - thrDb;
        float grDb = 0.f;
        float half = knee*0.5f;
        if (over > half) grDb = over*(1.f - 1.f/ratioE);
        else if (over > -half) { float t=(over+half)/(knee+1e-6f); grDb = (over+half)*t*0.5f*(1.f - 1.f/ratioE); }

        float gTarget = Decibels::decibelsToGain(-grDb);
        gainEnv = gTarget < gainEnv ? gTarget + (gainEnv-gTarget)*gAtkC : gTarget + (gainEnv-gTarget)*gRelC;
        float g = gainEnv*makeup;

        auto saturate = [&](float x){
            float p = x*driveLin;
            float odd = std::tanh(p);
            float even = p*std::abs(p)*0.7f;
            float wet = odd*(1.f-charC) + even*charC;
            return x*(1.f-satMixE) + wet*satMixE;
        };

        float cL = saturate(xL*g);
        float cR = saturate(xR*g);

        float hL = excHpf1[0].processSample(cL); hL = std::tanh(2.f*hL); hL = excHpf2[0].processSample(hL);
        float hR = excHpf1[1].processSample(cR); hR = std::tanh(2.f*hR); hR = excHpf2[1].processSample(hR);
        cL += hL*excE*0.8f;
        cR += hR*excE*0.8f;

        l[i] = dryL*(1.f-mix) + cL*mix;
        if (numChannels>1) r[i] = dryR*(1.f-mix) + cR*mix;
    }
    inputLevel.store(peak);
}

void PepeLAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PepeLAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType())) apvts.replaceState(ValueTree::fromXml(*xml));
}

AudioProcessorEditor* PepeLAudioProcessor::createEditor() { return new PepeLAudioProcessorEditor(*this); }

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PepeLAudioProcessor(); }
