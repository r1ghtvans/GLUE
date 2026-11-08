#include "PluginEditor.h"

using namespace juce;

// ---- Спрайты (X=волосы/основа, k=глаза/чернила, m=рот розовый, F=лицо, .=пусто) ----
static const char* spriteHeart[] = {
    ".XX.XX.","XXXXXXX","XXXXXXX",".XXXXX.","..XXX..","...X...",nullptr
};
static const char* spriteStar[] = {
    "..X..",".XXX.","XXXXX",".XXX.","..X..",nullptr
};
static const char* spriteCloud[] = {
    "..XXXXXX....",".XXXXXXXXXX.","XXXXXXXXXXXX","XXXXXXXXXXXX",".XXXXXXXXXX.","..XXXXXX....",nullptr
};
static const char* spriteGirlClosed[] = {
    "....XXXX....","..XXXXXXXX..",".XXXXXXXXXX.",".XFkkFFkkFX.",".XFFFFFFFFX.",".XFFkkkkFFX.",
    ".XFFFFFFFFX.","..XFkkkkFX..","..XFFFFFFX..",".XFFXXXXFFX.",".XFXXXXXXFX.","..XFXXXXFX..",
    "...XFFFFX...","....XXXX....",nullptr
};
static const char* spriteGirlOpen[] = {
    "....XXXX....","..XXXXXXXX..",".XXXXXXXXXX.",".XFkkFFkkFX.",".XFFFFFFFFX.",".XFmmmmmmFX.",
    ".XFFFFFFFFX.","..XFkkkkFX..","..XFFFFFFX..",".XFFXXXXFFX.",".XFXXXXXXFX.","..XFXXXXFX..",
    "...XFFFFX...","....XXXX....",nullptr
};

static void drawSprite(Graphics& g, const char** rows, int x, int y, int px, std::function<Colour(char)> col)
{
    for (int r=0; rows[r]; ++r){
        const char* row = rows[r];
        for (int c=0; row[c]; ++c){
            char ch = row[c];
            if (ch=='.') continue;
            g.setColour(col(ch));
            g.fillRect(x+c*px, y+r*px, px, px);
        }
    }
}

static void fillStepped(Graphics& g, Rectangle<int> b)
{
    Path p;
    int s=3;
    p.startNewSubPath((float)(b.getX()+s), (float)b.getY());
    p.lineTo((float)(b.getRight()-s), (float)b.getY());
    p.lineTo((float)b.getRight(), (float)(b.getY()+s));
    p.lineTo((float)b.getRight(), (float)(b.getBottom()-s));
    p.lineTo((float)(b.getRight()-s), (float)b.getBottom());
    p.lineTo((float)(b.getX()+s), (float)b.getBottom());
    p.lineTo((float)b.getX(), (float)(b.getBottom()-s));
    p.lineTo((float)b.getX(), (float)(b.getY()+s));
    p.closeSubPath();
    g.fillPath(p);
}

static void strokeStepped(Graphics& g, Rectangle<int> b, int sw)
{
    Path p;
    int s=3;
    p.startNewSubPath((float)(b.getX()+s), (float)b.getY());
    p.lineTo((float)(b.getRight()-s), (float)b.getY());
    p.lineTo((float)b.getRight(), (float)(b.getY()+s));
    p.lineTo((float)b.getRight(), (float)(b.getBottom()-s));
    p.lineTo((float)(b.getRight()-s), (float)b.getBottom());
    p.lineTo((float)(b.getX()+s), (float)b.getBottom());
    p.lineTo((float)b.getX(), (float)(b.getBottom()-s));
    p.lineTo((float)b.getX(), (float)(b.getY()+s));
    p.closeSubPath();
    g.strokePath(p, PathStrokeType((float)sw));
}

// ---- LookAndFeel ----
void PixelLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int w, int h,
                                        float pos, float min, float max,
                                        Slider::SliderStyle style, Slider& s)
{
    ignoreUnused(pos,min,max,style);
    auto bounds = Rectangle<int>(x,y,w,h).withSizeKeepingCentre(w, 14);
    const int segW=6, gap=2, segH=14;
    int n = std::max(1, (bounds.getWidth()+gap)/(segW+gap));
    double v = s.getValue();
    double lo=s.getMinimum(), hi=s.getMaximum();
    float f = (float)((v-lo)/(hi-lo));
    int filled = juce::roundToInt(f*n);
    ColourGradient grad(pal::pink, (float)bounds.getX(), 0.f, pal::sky, (float)bounds.getRight(), 0.f, false);
    grad.addColour(0.5f, pal::lavender);
    for (int i=0;i<n;++i){
        auto r = Rectangle<int>(bounds.getX()+i*(segW+gap), bounds.getY()+(bounds.getHeight()-segH)/2, segW, segH);
        if (i<filled){
            g.setGradientFill(grad);
            g.fillRect(r);
            g.setColour(Colours::white.withAlpha(0.5f));
            g.fillRect(r.getX(), r.getY(), segW, 2);
        } else {
            g.setColour(pal::track);
            g.fillRect(r);
        }
    }
    if (filled>0 && filled<=n){
        auto r = Rectangle<int>(bounds.getX()+(filled-1)*(segW+gap)-1, bounds.getY()+(bounds.getHeight()-16)/2-1, segW+2, 16);
        g.setColour(Colours::white);
        g.fillRect(r);
        g.setColour(pal::ink);
        g.drawRect(r, 1);
    }
}

void PixelLookAndFeel::drawComboBox(Graphics& g, int w, int h, bool, int, int, int, int, ComboBox&)
{
    auto r = Rectangle<int>(0,0,w,h).reduced(1);
    g.setColour(pal::paper); g.fillRect(r);
    g.setColour(pal::ink); g.drawRect(r, 1);
    // стрелка
    Path p;
    int cx=w/2, cy=h/2+2;
    p.addTriangle((float)(cx-4),(float)(cy-3),(float)(cx+4),(float)(cy-3),(float)cx,(float)(cy+3));
    g.fillPath(p);
}

Font PixelLookAndFeel::getComboBoxFont(ComboBox&) { return Font(FontOptions().withTypeface(font)).withHeight(13); }
void PixelLookAndFeel::positionComboBoxText(ComboBox& box, Label& label)
{
    label.setBounds(4,0,box.getWidth()-20,box.getHeight());
    label.setFont(getComboBoxFont(box));
    label.setColour(Label::textColourId, pal::ink);
    label.setJustificationType(Justification::centredLeft);
}

// ---- Panel ----
void Panel::paint(Graphics& g)
{
    auto b = getLocalBounds();
    // тень
    g.setColour(pal::ink.withAlpha(0.18f));
    fillStepped(g, b.translated(4,4));
    // заливка
    g.setColour(pal::paper);
    fillStepped(g, b);
    // рамки
    g.setColour(color);
    strokeStepped(g, b, 3);
    strokeStepped(g, b.reduced(4), 1);
    // шапка
    auto h = b.removeFromTop(36);
    g.setColour(color.withAlpha(0.45f));
    g.fillRect(h.reduced(3,3).withTrimmedBottom(0));
    // сердечко
    drawSprite(g, spriteHeart, h.getX()+12, h.getY()+12, 2, [](char ch)->Colour{
        if (ch=='X') return pal::pink; return Colours::transparentBlack;
    });
    g.setColour(pal::ink);
    g.setFont(Font(FontOptions().withTypeface(titleFont)).withHeight(13));
    g.drawText(title, h.translated(34,0), Justification::centredLeft);
}

// ---- ParamRow ----
ParamRow::ParamRow(AudioProcessorValueTreeState& st, const String& pid,
                   const String& label, const String& suffix, double def,
                   Typeface::Ptr f)
{
    nameLabel.setText(label, dontSendNotification);
    nameLabel.setFont(Font(FontOptions().withTypeface(f)).withHeight(24));
    nameLabel.setColour(Label::textColourId, pal::ink);
    nameLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(nameLabel);

    slider.setSliderStyle(Slider::LinearHorizontal);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    slider.setDoubleClickReturnValue(true, def);
    slider.setTextValueSuffix(suffix);
    slider.setLookAndFeel(nullptr); // будет через родителя
    addAndMakeVisible(slider);
    attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(st, pid, slider);

    valueLabel.setText(slider.getTextFromValue(slider.getValue()), dontSendNotification);
    valueLabel.setFont(Font(FontOptions().withTypeface(f)).withHeight(24));
    valueLabel.setColour(Label::textColourId, pal::inkSoft);
    valueLabel.setJustificationType(Justification::centredRight);
    addAndMakeVisible(valueLabel);

    slider.onValueChange = [this]{ valueLabel.setText(slider.getTextFromValue(slider.getValue()), dontSendNotification); };
}

void ParamRow::resized()
{
    auto b = getLocalBounds();
    nameLabel.setBounds(b.removeFromLeft(112));
    valueLabel.setBounds(b.removeFromRight(84));
    slider.setBounds(b.reduced(4,0));
}

// ---- Editor ----
PepeLAudioProcessorEditor::PepeLAudioProcessorEditor(PepeLAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p),
      compPanel(pal::pink, "COMPRESSOR", nullptr),
      satPanel(pal::mint, "SATURATOR", nullptr),
      excPanel(pal::lavender, "EXCITER", nullptr),
      outPanel(pal::sky, "OUTPUT", nullptr)
{
    fVt = Typeface::createSystemTypefaceFor(Fonts::VT323Regular_ttf, Fonts::VT323Regular_ttfSize);
    fPs = Typeface::createSystemTypefaceFor(Fonts::PressStart2PRegular_ttf, Fonts::PressStart2PRegular_ttfSize);

    compPanel.titleFont = fPs; satPanel.titleFont = fPs; excPanel.titleFont = fPs; outPanel.titleFont = fPs;
    laf.font = fPs;

    auto add = [&](Panel& panel, const String& pid, const String& nm, const String& sfx, double def){
        auto r = std::make_unique<ParamRow>(proc.apvts, pid, nm, sfx, def, fVt);
        r->slider.setLookAndFeel(&laf);
        panel.addAndMakeVisible(r.get());
        rows.push_back(std::move(r));
    };

    add(compPanel, "threshold","Threshold"," dB", -18.0);
    add(compPanel, "ratio","Ratio"," :1", 2.5);
    add(compPanel, "attack","Attack"," ms", 30.0);
    add(compPanel, "release","Release"," ms", 250.0);
    add(compPanel, "knee","Knee"," dB", 12.0);

    add(satPanel, "drive","Drive"," dB", 8.0);
    add(satPanel, "char","Character"," %", 35.0);
    add(satPanel, "satMix","Sat Mix"," %", 55.0);

    add(excPanel, "exciter","Exciter"," %", 20.0);
    add(excPanel, "excFreq","Exc Freq"," Hz", 4500.0);
    add(excPanel, "hpf","HP Filter"," Hz", 30.0);

    add(outPanel, "mix","Mix"," %", 100.0);
    add(outPanel, "intensity","Intensity"," %", 100.0);
    add(outPanel, "inputGain","Input Gain"," dB", 0.0);
    add(outPanel, "makeup","Makeup"," dB", 0.0);

    addAndMakeVisible(compPanel);
    addAndMakeVisible(satPanel);
    addAndMakeVisible(excPanel);
    addAndMakeVisible(outPanel);

    presetBox.setLookAndFeel(&laf);
    presetBox.addItemList({"CLEAN","GLUE","DIRTY","AIR"}, 1);
    presetAtt = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, "preset", presetBox);
    presetBox.onChange = [this]{
        if (applyingPreset) return;
        applyPreset(presetBox.getSelectedItemIndex());
    };
    outPanel.addAndMakeVisible(&presetBox);

    setSize(800, 548);
    setResizable(false, false);

    // сканлайны
    scanImg = Image(Image::ARGB, 800, 548, true);
    Graphics g(scanImg);
    for (int y=0; y<548; y+=3){
        g.setColour(pal::ink.withAlpha(0.09f));
        g.fillRect(0,y,800,1);
    }

    // скриншот
    if (JUCEApplicationBase::isStandaloneApp()){
        if (auto* app = dynamic_cast<JUCEApplication*>(JUCEApplicationBase::getInstance())){
            auto cl = app->getCommandLineParameters();
            if (cl.contains("--screenshot=")){
                shotPath = cl.fromFirstOccurrenceOf("--screenshot=", false, false).upToFirstOccurrenceOf(" ", false, false);
                shotMode = true;
            }
        }
    }

    startTimerHz(15);
}

PepeLAudioProcessorEditor::~PepeLAudioProcessorEditor()
{
    presetBox.setLookAndFeel(nullptr);
    for (auto& r: rows) r->slider.setLookAndFeel(nullptr);
}

void PepeLAudioProcessorEditor::applyPreset(int idx)
{
    if (idx<0 || idx>3) return;
    applyingPreset = true;
    struct P { float thr,ratio,atk,rel,knee,drive,chara,satMix,exciter,excFreq,hpf,makeup; };
    static const P table[4] = {
        {-14.f, 1.8f, 40.f, 200.f, 18.f,  6.f, 20.f, 35.f, 15.f, 6000.f, 30.f,  2.f},
        {-18.f, 2.5f, 30.f, 250.f, 12.f,  8.f, 35.f, 55.f, 20.f, 4500.f, 25.f,  3.f},
        {-22.f, 4.0f, 15.f, 180.f,  6.f, 16.f, 70.f, 80.f, 35.f, 3000.f, 25.f,  4.f},
        {-10.f, 1.5f, 50.f, 300.f, 20.f,  4.f, 20.f, 30.f, 60.f, 5000.f, 30.f,  1.f}
    };
    const P& v = table[idx];
    auto set = [&](const String& id, float val){
        if (auto* p = proc.apvts.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(val));
    };
    set("threshold", v.thr); set("ratio", v.ratio); set("attack", v.atk); set("release", v.rel);
    set("knee", v.knee); set("drive", v.drive); set("char", v.chara); set("satMix", v.satMix);
    set("exciter", v.exciter); set("excFreq", v.excFreq); set("hpf", v.hpf); set("makeup", v.makeup);
    applyingPreset = false;
}

void PepeLAudioProcessorEditor::resized()
{
    compPanel.setBounds(20,128,370,192);
    satPanel.setBounds(20,336,370,132);
    excPanel.setBounds(410,128,370,132);
    outPanel.setBounds(410,276,370,192);
    layoutPanelRows(compPanel);
    layoutPanelRows(satPanel);
    layoutPanelRows(excPanel);
    layoutPanelRows(outPanel);
    // пресет — последняя строка в OUTPUT
    auto ob = outPanel.getLocalBounds();
    ob.removeFromTop(36+4);
    for (int i=0;i<4;++i) ob.removeFromTop(30);
    presetBox.setBounds(ob.removeFromTop(30).reduced(4,0));
}

void PepeLAudioProcessorEditor::layoutPanelRows(Panel& p)
{
    auto b = p.getLocalBounds();
    b.removeFromTop(36+4);
    for (auto* c : p.getChildren()){
        if (auto* row = dynamic_cast<ParamRow*>(c)){
            row->setBounds(b.removeFromTop(30));
        }
    }
}

void PepeLAudioProcessorEditor::paint(Graphics& g)
{
    // фон: полосы 6px, 14 ступеней градиента
    for (int y=0; y<getHeight(); y+=6){
        float t = juce::jlimit(0.f,1.f, (float)y/(float)getHeight());
        int step = juce::jlimit(0,13, (int)(t*14.f));
        float s = step/13.f;
        Colour c;
        if (s<0.5f) c = pal::pink.interpolatedWith(pal::lavender, s*2.f);
        else c = pal::lavender.interpolatedWith(pal::mint, (s-0.5f)*2.f);
        g.setColour(c);
        g.fillRect(0,y,getWidth(),6);
    }

    // облака
    for (int i=0;i<3;++i){
        int cx = ((int)cloudX[i]) % (getWidth()+200) - 100;
        drawSprite(g, spriteCloud, cx, 8+i*18, 3, [](char ch)->Colour{
            if (ch=='X') return Colours::white.withAlpha(0.65f); return Colours::transparentBlack;
        });
    }

    // звёзды
    for (int i=0;i<12;++i){
        int sx = (i*73 + 30) % getWidth();
        int sy = (i*37 + 10) % 90;
        float a = ((tick + i*3) % 8 < 4) ? 0.8f : 0.3f;
        drawSprite(g, spriteStar, sx, sy, 2, [a](char ch)->Colour{
            if (ch=='X') return Colours::white.withAlpha(a); return Colours::transparentBlack;
        });
    }

    // заголовок с хроматической аберрацией
    int off = glitchOn ? 4 : 2;
    g.setFont(Font(FontOptions().withTypeface(fPs)).withHeight(30));
    g.setColour(pal::glitchCyan);
    g.drawText("PEPEL GLUE", 24+off, 28, 400, 40, Justification::centredLeft);
    g.setColour(pal::glitchPink);
    g.drawText("PEPEL GLUE", 24-off, 28, 400, 40, Justification::centredLeft);
    g.setColour(pal::ink);
    g.drawText("PEPEL GLUE", 24, 28, 400, 40, Justification::centredLeft);

    g.setFont(Font(FontOptions().withTypeface(fVt)).withHeight(24));
    g.setColour(pal::inkSoft);
    g.drawText("glue compressor // saturator // exciter", 24, 66, 500, 30, Justification::centredLeft);

    // чиби
    float lvl = proc.inputLevel.load();
    bool sing = lvl > 0.01f;
    int hop = sing ? ((tick%2)*2) : 0;
    auto hair = [&](int idx)->Colour{ return idx==0 ? pal::pink : pal::lavender; };
    for (int i=0;i<2;++i){
        int gx = 640 + i*80;
        int gy = 30 - hop;
        auto* spr = sing ? spriteGirlOpen : spriteGirlClosed;
        drawSprite(g, spr, gx, gy, 4, [&](char ch)->Colour{
            if (ch=='X') return hair(i);
            if (ch=='F') return pal::peach;
            if (ch=='k') return pal::ink;
            if (ch=='m') return pal::pink;
            return Colours::transparentBlack;
        });
    }

    // эквалайзер внизу
    int eqY = 478;
    int eqH = 48;
    int barW = 6, gap = 2;
    int startX = 20;
    for (int i=0;i<48;++i){
        int bh = juce::jlimit(2, eqH, (int)(eqBars[i]));
        auto r = Rectangle<int>(startX + i*(barW+gap), eqY + (eqH-bh), barW, bh);
        g.setColour(pal::lavender);
        g.fillRect(r);
        g.setColour(pal::inkSoft);
        g.drawRect(r, 1);
    }

    // подпись
    g.setFont(Font(FontOptions().withTypeface(fPs)).withHeight(10));
    g.setColour(pal::inkSoft);
    g.drawText("PEPEL", 740, 530, 50, 14, Justification::centredRight);
}

void PepeLAudioProcessorEditor::paintOverChildren(Graphics& g)
{
    // сканлайны
    g.drawImageAt(scanImg, 0, 0);

    // зерно
    for (int i=0;i<24;++i){
        int x = rng.nextInt(getWidth()-2);
        int y = rng.nextInt(getHeight());
        g.setColour(pal::ink.withAlpha(0.12f));
        g.fillRect(x,y,2,1);
    }

    // сбой
    if (glitchOn){
        int gy = 100 + (tick*37 % 300);
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillRect(0, gy, getWidth(), 8);
        g.setColour(pal::glitchCyan.withAlpha(0.35f));
        g.fillRect(20, gy-2, getWidth()-40, 2);
        g.setColour(pal::glitchPink.withAlpha(0.35f));
        g.fillRect(20, gy+8, getWidth()-40, 2);
    }
}

void PepeLAudioProcessorEditor::timerCallback()
{
    ++tick;
    // облака
    cloudX[0] += 0.4f; cloudX[1] += 0.25f; cloudX[2] += 0.55f;
    // глитч
    if (tick % 60 == 0) glitchLeft = 2 + rng.nextInt(3);
    glitchOn = glitchLeft > 0;
    if (glitchLeft > 0) --glitchLeft;

    // эквалайзер
    float lvl = proc.inputLevel.load();
    for (int i=0;i<48;++i){
        float target = lvl * (0.3f + 0.7f*rng.nextFloat()) * 46.f;
        eqBars[i] += (target - eqBars[i]) * 0.5f;
    }

    if (shotMode){
        if (++shotTicks == 4){
            auto img = createComponentSnapshot(getLocalBounds());
            PNGImageFormat png;
            MemoryOutputStream mos;
            if (png.writeImageToStream(img, mos)){
                File(shotPath).replaceWithData(mos.getData(), mos.getDataSize());
            }
            JUCEApplicationBase::quit();
        }
    }
    repaint();
}
