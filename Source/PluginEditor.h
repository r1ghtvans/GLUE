#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace pal {
    inline const juce::Colour ink(0xff5b4b8a), inkSoft(0xff8c7bb5), pink(0xffff9ec7),
        lavender(0xffc9b3ff), mint(0xff9ee8d4), sky(0xff9fd0ff), lemon(0xffffe69a),
        peach(0xffffc4a8), paper(0xfffff7fb), track(0xffeadcf5);
    inline const juce::Colour glitchCyan(0xff7fe7ff), glitchPink(0xffff7fbf);
}

class PixelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Typeface::Ptr font;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                          float pos, float min, float max,
                          juce::Slider::SliderStyle style, juce::Slider& s) override;
    void drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int,
                      juce::ComboBox& box) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
};

class Panel : public juce::Component
{
public:
    Panel(juce::Colour c, juce::String t, juce::Typeface::Ptr f) : color(c), title(t), titleFont(f) {}
    void paint(juce::Graphics& g) override;
    juce::Colour color;
    juce::String title;
    juce::Typeface::Ptr titleFont;
};

class ParamRow : public juce::Component
{
public:
    ParamRow(juce::AudioProcessorValueTreeState& st, const juce::String& pid,
             const juce::String& label, const juce::String& suffix, double def,
             juce::Typeface::Ptr f);
    void resized() override;
    juce::Slider slider;
    juce::Label nameLabel, valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class PepeLAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PepeLAudioProcessorEditor(PepeLAudioProcessor&);
    ~PepeLAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void applyPreset(int idx);
    void layoutPanelRows(Panel& p);

    PepeLAudioProcessor& proc;
    juce::Typeface::Ptr fVt, fPs;
    PixelLookAndFeel laf;

    Panel compPanel, satPanel, excPanel, outPanel;
    std::vector<std::unique_ptr<ParamRow>> rows;
    juce::ComboBox presetBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAtt;
    bool applyingPreset = false;

    juce::Image scanImg;
    int tick = 0;
    int glitchLeft = 0;
    bool glitchOn = false;
    juce::Random rng;
    float eqBars[48] = {};
    float cloudX[3] = {0, 220, 520};
    int chibiHop = 0;

    juce::String shotPath;
    int shotTicks = 0;
    bool shotMode = false;
};
