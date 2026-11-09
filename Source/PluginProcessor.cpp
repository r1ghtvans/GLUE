#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout PepeLAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    auto add = [&](auto p){ params.push_back(std::move(p)); };
    add(std::make_unique<AudioParameterFloat>(ParameterID{"inputGain",1},"Input Gain",NormalisableRange<float>(-24.f,24.f,0.1f),0.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"hpf",1},"HP Filter",NormalisableRange<float>(20.f,300.f,1.f),30.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" Hz"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"threshold",1},"Threshold",NormalisableRange<float>(-60.f,0.f,0.1f),-18.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"ratio",1},"Ratio",NormalisableRange<float>(1.f,8.f,0.1f),2.5f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" :1"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"attack",1},"Attack",NormalisableRange<float>(1.f,200.f,1.f),30.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" ms"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"release",1},"Release",NormalisableRange<float>(20.f,1000.f,1.f),250.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" ms"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"knee",1},"Knee",NormalisableRange<float>(0.f,24.f,0.1f),12.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"makeup",1},"Makeup",NormalisableRange<float>(-12.f,12.f,0.1f),0.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"drive",1},"Drive",NormalisableRange<float>(0.f,24.f,0.1f),8.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String(v,1)+" dB"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"char",1},"Character",NormalisableRange<float>(0.f,100.f,1.f),35.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"satMix",1},"Sat Mix",NormalisableRange<float>(0.f,100.f,1.f),55.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"exciter",1},"Exciter",NormalisableRange<float>(0.f,100.f,1.f),20.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"excFreq",1},"Exc Freq",NormalisableRange<float>(1000.f,10000.f,10.f),4500.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" Hz"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"mix",1},"Mix",NormalisableRange<float>(0.f,100.f,1.f),100.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterFloat>(ParameterID{"intensity",1},"Intensity",NormalisableRange<float>(0.f,100.f,1.f),100.f,AudioParameterFloatAttributes().withStringFromValueFunction([](auto v,int){ return String((int)v)+" %"; })));
    add(std::make_unique<AudioParameterChoice>(ParameterID{"preset",1},"Preset",StringArray{"CLEAN","GLUE","DIRTY","AIR"},1));
    return { params.begin(), params.end() };
}

PepeLAudioProcessor::PepeLAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    auto P = [this](const char* id){ return apvts.getRawParameterValue(id); };
    pInGain = P("inputGain"); pHpf = P("hpf");       pThr = P("threshold"); pRatio = P("ratio");
    pAtk = P("attack");       pRel = P("release");   pKnee = P("knee");     pMakeup = P("makeup");
    pDrive = P("drive");      pChar = P("char");     pSatMix = P("satMix"); pExc = P("exciter");
    pExcFreq = P("excFreq");  pMix = P("mix");       pInten = P("intensity");
}

// Пересчитывает цели сглаживателей из параметров. Без аллокаций.
void PepeLAudioProcessor::updateTargets(bool snap)
{
    const float inten = pInten->load() * 0.01f;
    const float ratio = pRatio->load();

    auto set = [snap](auto& sm, float v){ if (snap) sm.setCurrentAndTargetValue(v); else sm.setTargetValue(v); };
    set(sInGain, Decibels::decibelsToGain(pInGain->load()));
    set(sMakeup, Decibels::decibelsToGain(pMakeup->load()));
    set(sThr,    pThr->load());
    set(sRatio,  1.f + (ratio - 1.f) * inten);
    set(sKnee,   pKnee->load());
    set(sDrive,  Decibels::decibelsToGain(pDrive->load() * inten));
    set(sChar,   pChar->load() * 0.01f);
    set(sSatMix, pSatMix->load() * 0.01f * inten);
    set(sExc,    pExc->load() * 0.01f * inten);
    set(sMix,    pMix->load() * 0.01f);
    set(sHpf,    pHpf->load());
    set(sExcFreq, jlimit(200.f, (float) (sr * 0.45), pExcFreq->load()));
}

void PepeLAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    dsp::ProcessSpec spec { sampleRate, (uint32) jmax(1, samplesPerBlock), 2 };
    for (auto* f : { &hpf, &excHpf1, &excHpf2 }) { f->prepare(spec); f->setType(dsp::StateVariableTPTFilterType::highpass); f->reset(); }

    for (auto* s : { &sInGain, &sMakeup, &sDrive, &sChar, &sSatMix, &sExc, &sMix, &sThr, &sRatio, &sKnee }) s->reset(sampleRate, 0.03);
    for (auto* s : { &sHpf, &sExcFreq }) s->reset(sampleRate, 0.03);
    updateTargets(true);

    env = 0.f; gainEnv = 1.f;
    dcR = 1.f - (2.f * MathConstants<float>::pi * 10.f / (float) sampleRate);
    dcX[0] = dcX[1] = dcY[0] = dcY[1] = 0.f;
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
    const int numChannels = jmin(2, getTotalNumInputChannels());
    const int n = buffer.getNumSamples();
    if (n == 0 || numChannels == 0) return;

    updateTargets(false);

    const float atkC = std::exp(-1.f / jmax(1.f, 0.001f * pAtk->load() * (float) sr));
    const float relC = std::exp(-1.f / jmax(1.f, 0.001f * pRel->load() * (float) sr));
    const float gAtkC = std::exp(-1.f / (0.005f * (float) sr));

    float* l = buffer.getWritePointer(0);
    float* r = numChannels > 1 ? buffer.getWritePointer(1) : l;
    const bool stereo = numChannels > 1;

    float peak = 0.f;

    // Сатуратор: асимметричный tanh (даёт чётные гармоники), ограничен по уровню, с авто-компенсацией drive
    auto saturate = [this](float x, float drive, float ch, float mix, int c)
    {
        const float bias = ch * 0.6f;
        float wet = std::tanh(x * drive + bias) - std::tanh(bias);
        wet *= 1.f / std::sqrt(jmax(1.f, drive));
        // DC-блокировка (около 10 Гц), т.к. асимметрия смещает постоянную составляющую
        const float y = wet - dcX[c] + dcR * dcY[c];
        dcX[c] = wet; dcY[c] = y;
        return x * (1.f - mix) + y * mix;
    };

    for (int i = 0; i < n; ++i)
    {
        const float inG = sInGain.getNextValue(), mk = sMakeup.getNextValue();
        const float thr = sThr.getNextValue(), rat = sRatio.getNextValue(), knee = sKnee.getNextValue();
        const float drv = sDrive.getNextValue(), chr = sChar.getNextValue(), sMx = sSatMix.getNextValue();
        const float exc = sExc.getNextValue(), mix = sMix.getNextValue();
        const float hpFreq = sHpf.getNextValue(), exFreq = sExcFreq.getNextValue();

        if (sHpf.isSmoothing())     hpf.setCutoffFrequency(hpFreq);
        else if (i == 0)            hpf.setCutoffFrequency(hpFreq);
        if (sExcFreq.isSmoothing()) { excHpf1.setCutoffFrequency(exFreq); excHpf2.setCutoffFrequency(exFreq); }
        else if (i == 0)            { excHpf1.setCutoffFrequency(exFreq); excHpf2.setCutoffFrequency(exFreq); }

        const float dryL = l[i];
        const float dryR = stereo ? r[i] : dryL;
        peak = jmax(peak, jmax(std::abs(dryL), std::abs(dryR)));

        const float xL = hpf.processSample(0, dryL * inG);
        const float xR = stereo ? hpf.processSample(1, dryR * inG) : xL;

        // детектор + компрессор
        const float det = jmax(std::abs(xL), std::abs(xR));
        env = det > env ? det + (env - det) * atkC : det + (env - det) * relC;

        const float over = Decibels::gainToDecibels(env + 1e-6f) - thr;
        const float half = knee * 0.5f;
        float grDb = 0.f;
        if (over > half)        grDb = over * (1.f - 1.f / rat);
        else if (over > -half)  { const float t = (over + half) / (knee + 1e-6f); grDb = (over + half) * t * 0.5f * (1.f - 1.f / rat); }

        const float gTarget = Decibels::decibelsToGain(-grDb);
        gainEnv = gTarget < gainEnv ? gTarget + (gainEnv - gTarget) * gAtkC
                                    : gTarget + (gainEnv - gTarget) * relC;
        const float g = gainEnv * mk;

        float cL = saturate(xL * g, drv, chr, sMx, 0);
        float cR = stereo ? saturate(xR * g, drv, chr, sMx, 1) : cL;

        // эксайтер
        float hL = excHpf1.processSample(0, cL); hL = std::tanh(2.f * hL); hL = excHpf2.processSample(0, hL);
        cL += hL * exc * 0.8f;
        if (stereo)
        {
            float hR = excHpf1.processSample(1, cR); hR = std::tanh(2.f * hR); hR = excHpf2.processSample(1, hR);
            cR += hR * exc * 0.8f;
        }

        l[i] = dryL * (1.f - mix) + cL * mix;
        if (stereo) r[i] = dryR * (1.f - mix) + cR * mix;
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
