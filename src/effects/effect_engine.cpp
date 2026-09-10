#include "effect_engine.h"
#include "../core/color.h"

#include <cmath>
#include <algorithm>

namespace lra {

namespace {
struct Glyph5x7 { std::uint8_t col[5]; };
static const Glyph5x7 kUpper[26] = {
{{0x7E,0x09,0x09,0x09,0x7E}},{{0x7F,0x49,0x49,0x49,0x36}},{{0x3E,0x41,0x41,0x41,0x22}},{{0x7F,0x41,0x41,0x22,0x1C}},{{0x7F,0x49,0x49,0x49,0x41}},{{0x7F,0x09,0x09,0x09,0x01}},
{{0x3E,0x41,0x49,0x49,0x7A}},{{0x7F,0x08,0x08,0x08,0x7F}},{{0x41,0x41,0x7F,0x41,0x41}},{{0x20,0x40,0x40,0x40,0x3F}},{{0x7F,0x08,0x14,0x22,0x41}},{{0x7F,0x40,0x40,0x40,0x40}},
{{0x7F,0x02,0x04,0x02,0x7F}},{{0x7F,0x04,0x08,0x10,0x7F}},{{0x3E,0x41,0x41,0x41,0x3E}},{{0x7F,0x09,0x09,0x09,0x06}},{{0x3E,0x41,0x51,0x21,0x5E}},{{0x7F,0x09,0x19,0x29,0x46}},
{{0x46,0x49,0x49,0x49,0x31}},{{0x01,0x01,0x7F,0x01,0x01}},{{0x3F,0x40,0x40,0x40,0x3F}},{{0x1F,0x20,0x40,0x20,0x1F}},{{0x7F,0x20,0x18,0x20,0x7F}},{{0x63,0x14,0x08,0x14,0x63}},{{0x07,0x08,0x70,0x08,0x07}},{{0x61,0x51,0x49,0x45,0x43}}};
static const Glyph5x7 kLower[26] = {
{{0x20,0x54,0x54,0x54,0x78}},{{0x7F,0x48,0x44,0x44,0x38}},{{0x38,0x44,0x44,0x44,0x20}},{{0x38,0x44,0x44,0x48,0x7F}},{{0x38,0x54,0x54,0x54,0x18}},{{0x08,0x7E,0x09,0x01,0x02}},{{0x0C,0x52,0x52,0x52,0x3E}},{{0x7F,0x08,0x04,0x04,0x78}},{{0x00,0x44,0x7D,0x40,0x00}},{{0x20,0x40,0x44,0x3D,0x00}},{{0x7F,0x10,0x28,0x44,0x00}},{{0x00,0x41,0x7F,0x40,0x00}},{{0x7C,0x04,0x18,0x04,0x78}},{{0x7C,0x08,0x04,0x04,0x78}},{{0x38,0x44,0x44,0x44,0x38}},{{0x7C,0x14,0x14,0x14,0x08}},{{0x08,0x14,0x14,0x18,0x7C}},{{0x7C,0x08,0x04,0x04,0x08}},{{0x48,0x54,0x54,0x54,0x20}},{{0x04,0x3F,0x44,0x40,0x20}},{{0x3C,0x40,0x40,0x20,0x7C}},{{0x1C,0x20,0x40,0x20,0x1C}},{{0x3C,0x40,0x30,0x40,0x3C}},{{0x44,0x28,0x10,0x28,0x44}},{{0x0C,0x50,0x50,0x50,0x3C}},{{0x44,0x64,0x54,0x4C,0x44}}};
const Glyph5x7& glyph(wchar_t ch){ static const Glyph5x7 blank{{0,0,0,0,0}}; if(ch>=L'A'&&ch<=L'Z')return kUpper[ch-L'A']; if(ch>=L'a'&&ch<=L'z')return kLower[ch-L'a']; return blank; }
Rgb textColor(const AppState& s,double f){const double b=s.brightness/100.0;return {(s.textRed/63.0)*b*f,(s.textGreen/63.0)*b*f,(s.textBlue/63.0)*b*f};}
double textAnimationFactor(const AppState& s){const double p=std::fmod(s.animationPhase*0.35,2.0);switch(s.textAnimation){case 2:return std::clamp(p,0.0,1.0);case 3:return 1.0-std::clamp(p,0.0,1.0);case 4:return p<=1.0?p:2.0-p;case 5:return p<1.0?1.0:0.05;default:return 1.0;}}
bool textPixel(const AppState& s,int sx,int y){const int gw=6; if(sx<0||y<0)return false;const int ci=sx/gw;const int lx=sx%gw;if(ci<0||ci>=static_cast<int>(s.textContent.size()))return false;const auto& g=glyph(s.textContent[ci]);if(lx>=5)return false;return y<7&&(g.col[lx]&(1u<<y));}
void renderText(const AppState& s,LedFrame& frame,FunctionKeyFrame& keys){
    for(auto& row:frame)for(auto& p:row)p={};
    for(auto& k:keys)k={};

    // Text size is intentionally fixed at the native 5x7 font.
    const int gw=6;
    const int total=static_cast<int>(s.textContent.size())*gw;
    const double f=textAnimationFactor(s);
    const Rgb c=textColor(s,f);

    if(s.textAnimation==1){
        // Scroll remains smooth; only its speed is adjusted.
        const int scroll=static_cast<int>(std::floor(s.textOffset));
        for(int y=0;y<8;++y)for(int x=0;x<9;++x){
            const int sx=x-s.textX+scroll;
            const int sy=y-s.textY;
            if(sx<0||sx>=total||sy<0||sy>=8||!textPixel(s,sx,sy))continue;
            if(x<8)frame[y][x]=c;else keys[y]=c;
        }
        return;
    }

    // Non-scroll animations reveal the message one complete character at a time.
    // Once a character is revealed it stays visible, so no partial glyphs appear.
    const double charInterval=0.42/(0.2+s.textSpeed/10.0);
    const int visible=std::clamp(static_cast<int>(std::floor(s.animationPhase/charInterval)),0,static_cast<int>(s.textContent.size()));
    const int shownWidth=visible*gw;
    for(int y=0;y<8;++y)for(int x=0;x<9;++x){
        const int sx=x-s.textX;
        const int sy=y-s.textY;
        if(sx<0||sx>=shownWidth||sy<0||sy>=7||!textPixel(s,sx,sy))continue;
        if(x<8)frame[y][x]=c;else keys[y]=c;
    }
}
}

void EffectEngine::render(AppState& state,LedFrame& frame,FunctionKeyFrame& functionKeys) const
{
    if(!state.effectRunning)return;
    renderBase(state,frame);
    renderIndicators(state,frame);
    if(state.effect==Effect::Text){renderText(state,frame,functionKeys);}
    else if(state.effect!=Effect::Breathe){for(auto& row:frame)for(auto& pixel:row)pixel=snapToPalette(pixel);renderFunctionKeys(state,frame,functionKeys);}
    else{renderFunctionKeys(state,frame,functionKeys);}
    const double speed=state.speed/20.0;
    state.animationPhase+=0.035*(0.2+speed);
    if(state.effect==Effect::Text&&state.textAnimation==1){const double step=0.045*(0.2+state.textSpeed/10.0);state.textOffset+=step;const int gw=6;const double total=static_cast<double>(state.textContent.size()*gw+9);if(state.textOffset>total)state.textOffset=-9.0;}
}

void EffectEngine::renderBase(const AppState& state,LedFrame& frame) const
{
    const double brightness=state.brightness/100.0;
    if(state.effect==Effect::Text){for(auto& row:frame)for(auto& pixel:row)pixel={};return;}
    for(int y=0;y<8;++y){for(int x=0;x<8;++x){Rgb rgb{};switch(state.effect){
    case Effect::Rainbow: rgb=hsvToRgb(x/8.0+y/16.0+state.animationPhase*0.11,0.9,brightness);break;
    case Effect::Breathe:{const double value=(0.5+0.5*std::sin(state.animationPhase*2.0))*brightness;const Rgb base=selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));rgb={base.r*value,base.g*value,base.b*value};break;}
    case Effect::Wave:{const double dx=x-3.5,dy=y-3.5,distance=std::sqrt(dx*dx+dy*dy);const double value=(0.5+0.5*std::sin(distance*2.0-state.animationPhase*5.0))*brightness;const Rgb base=selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));rgb={base.r*value,base.g*value,base.b*value};break;}
    case Effect::Stars:{const double n=std::sin(x*12.9898+y*78.233+std::floor(state.animationPhase*1.5)*37.7);const double value=(n-std::floor(n))*0.85*brightness;const bool star=((x*17+y*31+static_cast<int>(state.animationPhase))%19)==0;const Rgb base=selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));if(star)rgb={base.r*value,base.g*value,base.b*value};else rgb={base.r*value*0.02,base.g*value*0.02,base.b*value*0.02};break;}
    case Effect::Solid:{const Rgb base=selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));rgb={base.r*brightness,base.g*brightness,base.b*brightness};break;}
    case Effect::Text:break;}frame[y][x]=rgb;}}
}

void EffectEngine::renderIndicators(const AppState& state,LedFrame& frame) const
{
    const auto barLength=[](double usage){const double value=std::clamp(usage/100.0,0.0,1.0);return static_cast<int>(std::round(value*8.0));};
    if(state.cpuIndicator){const int length=barLength(state.monitoring.cpuUsage);for(int x=0;x<8;++x){if(state.gpuIndicator&&x==7)continue;if(x<length){const double value=std::clamp(state.monitoring.cpuUsage/100.0,0.0,1.0);frame[0][x]={0.08,1.0-value,0.05};}else frame[0][x]={frame[0][x].r*0.15,frame[0][x].g*0.15,frame[0][x].b*0.15};}}
    if(state.ramIndicator){const int length=barLength(state.monitoring.ramUsage);for(int x=0;x<8;++x){if(state.temperatureIndicator&&x==7)continue;if(x<length){const double value=std::clamp(state.monitoring.ramUsage/100.0,0.0,1.0);frame[7][x]={0.10,1.0-value,0.05};}else frame[7][x]={frame[7][x].r*0.15,frame[7][x].g*0.15,frame[7][x].b*0.15};}}
    if(state.gpuIndicator)frame[0][7]={0.15,0.25,1.0};
    if(state.temperatureIndicator)frame[7][7]={1.0,0.1,0.0};
}

void EffectEngine::renderFunctionKeys(const AppState& state,const LedFrame& frame,FunctionKeyFrame& functionKeys) const
{
    if(state.effect==Effect::Text)return;
    for(int i=0;i<8;++i)functionKeys[i]=frame[i][7];
    for(int i=0;i<8;++i)functionKeys[8+i]=frame[0][i];
}

} // namespace lra
