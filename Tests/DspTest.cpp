// Офлайн-тест DSP: не запускает DAW, проверяет уровни, NaN и тишину.
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cstdio>

static bool g_ok = true;
static void check(bool c, const char* msg) { std::printf("[%s] %s\n", c ? "PASS" : "FAIL", msg); if (!c) g_ok = false; }

static void setParam(PepeLAudioProcessor& p, const char* id, float v)
{
    if (auto* prm = p.apvts.getParameter(id)) prm->setValueNotifyingHost(prm->convertTo0to1(v));
}

struct RunResult { float peak; bool bad; float dc; };

static RunResult run(PepeLAudioProcessor& p, float amp, float freq, int seconds = 2)
{
    const double sr = 48000.0; const int bs = 256;
    p.setPlayConfigDetails(2, 2, sr, bs);
    p.prepareToPlay(sr, bs);
    juce::AudioBuffer<float> buf(2, bs); juce::MidiBuffer midi;
    double ph = 0; float peak = 0; bool bad = false; double sum = 0; long cnt = 0;
    for (int b = 0; b < (int)(sr * seconds / bs); ++b) {
        for (int i = 0; i < bs; ++i) {
            float s = amp * (float)std::sin(ph); ph += 2.0 * juce::MathConstants<double>::pi * freq / sr;
            buf.setSample(0, i, s); buf.setSample(1, i, s);
        }
        p.processBlock(buf, midi);
        for (int c = 0; c < 2; ++c) for (int i = 0; i < bs; ++i) {
            float v = buf.getSample(c, i);
            if (!std::isfinite(v)) bad = true;
            if (b > 20) { peak = std::max(peak, std::abs(v)); sum += v; ++cnt; }
        }
    }
    return { peak, bad, (float)(sum / std::max(1L, cnt)) };
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    const char* names[4] = { "CLEAN", "GLUE", "DIRTY", "AIR" };
    struct P { float thr,ratio,atk,rel,knee,drive,chara,satMix,exciter,excFreq,hpf,makeup; };
    const P table[4] = {
        {-14.f,1.8f,40.f,200.f,18.f,6.f,20.f,35.f,15.f,6000.f,30.f,2.f},
        {-18.f,2.5f,30.f,250.f,12.f,8.f,35.f,55.f,20.f,4500.f,25.f,3.f},
        {-22.f,4.0f,15.f,180.f,6.f,16.f,70.f,80.f,35.f,3000.f,25.f,4.f},
        {-10.f,1.5f,50.f,300.f,20.f,4.f,20.f,30.f,60.f,5000.f,30.f,1.f}};
    for (int k = 0; k < 4; ++k) {
        PepeLAudioProcessor p;
        const P& v = table[k];
        setParam(p,"threshold",v.thr); setParam(p,"ratio",v.ratio); setParam(p,"attack",v.atk); setParam(p,"release",v.rel);
        setParam(p,"knee",v.knee); setParam(p,"drive",v.drive); setParam(p,"char",v.chara); setParam(p,"satMix",v.satMix);
        setParam(p,"exciter",v.exciter); setParam(p,"excFreq",v.excFreq); setParam(p,"hpf",v.hpf); setParam(p,"makeup",v.makeup);
        auto r = run(p, 0.5f, 220.f);
        std::printf("preset %-5s  in peak 0.50 -> out peak %.2f (%.1f dB gain), DC %.4f\n", names[k], r.peak, 20*std::log10(r.peak/0.5f), r.dc);
        check(!r.bad, "no NaN/Inf");
        check(r.peak < 1.5f, "output peak below 1.5 (about +3.5 dB over input)... not exploding");
        check(std::abs(r.dc) < 0.02f, "DC offset small");
    }
    { // тишина -> тишина
        PepeLAudioProcessor p; auto r = run(p, 0.f, 100.f);
        check(r.peak < 1e-4f, "silence in -> silence out");
    }
    { // сатуратор отдельно: компрессор выключен (ratio 1), без эксайтера
        PepeLAudioProcessor p;
        setParam(p,"ratio",1.f); setParam(p,"threshold",0.f); setParam(p,"exciter",0.f);
        setParam(p,"drive",16.f); setParam(p,"char",70.f); setParam(p,"satMix",80.f); setParam(p,"hpf",20.f);
        auto r = run(p, 0.5f, 220.f);
        std::printf("saturator only (drive 16 dB, char 70%%): out peak %.2f (%.1f dB gain), DC %.4f\n", r.peak, 20*std::log10(r.peak/0.5f), r.dc);
        check(r.peak < 1.5f, "saturator alone stays bounded (<1.5)");
        check(std::abs(r.dc) < 0.02f, "saturator alone: DC small");
    }
    { // экстремальные настройки
        PepeLAudioProcessor p;
        setParam(p,"drive",24.f); setParam(p,"char",100.f); setParam(p,"satMix",100.f); setParam(p,"exciter",100.f);
        setParam(p,"inputGain",24.f); setParam(p,"makeup",12.f); setParam(p,"ratio",8.f); setParam(p,"threshold",-60.f);
        auto r = run(p, 1.0f, 1000.f);
        std::printf("extreme: out peak %.2f\n", r.peak);
        check(!r.bad, "extreme: no NaN/Inf");
        check(r.peak < 40.f, "extreme: peak under +32 dB");
    }
    std::printf(g_ok ? "ALL OK\n" : "FAILED\n");
    return g_ok ? 0 : 1;
}
