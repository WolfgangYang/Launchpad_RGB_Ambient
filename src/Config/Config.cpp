#include "Config.h"
#include <algorithm>

Config::Language Config::language = Config::Language::English;
int Config::brightness = 80;
int Config::speed = 50;

void Config::Initialize() {
    // 这里以后可以添加从文件读取配置的逻辑
}

Config::Language Config::GetLanguage() { return language; }
int Config::GetBrightness() { return brightness; }
int Config::GetSpeed() { return speed; }
void Config::SetBrightness(int value) { brightness = std::clamp(value, 0, 100); }
void Config::SetSpeed(int value) { speed = std::clamp(value, 0, 100); }
