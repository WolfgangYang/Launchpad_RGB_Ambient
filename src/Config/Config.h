// Configuration Module
#pragma once

#include <string>

class Config {
public:
    enum class Language { Chinese, English };
    
    static void Initialize();
    static void LoadFromFile(const std::wstring& path);
    static void SaveFromFile(const std::wstring& path);
    
    static Language GetLanguage();
    static int GetBrightness();
    static int GetSpeed();
    
    static void SetLanguage(Language lang);
    static void SetBrightness(int value);
    static void SetSpeed(int value);
    
private:
    static Language language;
    static int brightness;
    static int speed;
};