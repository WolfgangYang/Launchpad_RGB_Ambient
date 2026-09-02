// Configuration Implementation
#include "Config.h"
#include <fstream>
#include <filesystem>

Config::Language Config::language = Config::English;
int Config::brightness = 80;
int Config::speed = 50;

void Config::Initialize() {
    // Load default configuration
}

void Config::LoadFromFile(const std::wstring& path) {
    // Placeholder - implement JSON parsing
}

void Config::SaveFromFile(const std::wstring& path) {
    // Placeholder - implement JSON serialization
}

Config::Language Config::GetLanguage() {
    return language;
}

int Config::GetBrightness() {
    return brightness;
}

int Config::GetSpeed() {
    return speed;
}

void Config::SetLanguage(Language lang) {
    language = lang;
}

void Config::SetBrightness(int value) {
    brightness = std::clamp(value, 0, 100);
}

void Config::SetSpeed(int value) {
    speed = std::clamp(value, 0, 100);
}