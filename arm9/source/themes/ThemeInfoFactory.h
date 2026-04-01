#pragma once
#include <memory>
#include "ThemeInfo.h"

class ThemeInfoFactory
{
public:
    static constexpr u32 MAX_THEMES = 8;

    std::unique_ptr<ThemeInfo> CreateFromThemeFolder(const TCHAR* folderName) const;

    u32 EnumerateThemes(ThemeInfo* outThemes[], u32 maxThemes) const;

    std::unique_ptr<ThemeInfo> CreateFallbackTheme() const
    {
        return std::make_unique<ThemeInfo>("", ThemeType::Material, "Fallback", "", "", Rgb<8,8,8>(138, 217, 255), false);
    }
};
