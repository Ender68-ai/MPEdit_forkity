#pragma once

#include <cocos2d.h>
#include <array>
#include <string>

namespace mpedit::ColorPalette {

    inline const std::array<cocos2d::ccColor3B, 16> colors = {
        cocos2d::ccColor3B{80, 200, 255},
        cocos2d::ccColor3B{255, 85, 85},
        cocos2d::ccColor3B{80, 240, 165},
        cocos2d::ccColor3B{255, 135, 35},
        cocos2d::ccColor3B{175, 95, 255},
        cocos2d::ccColor3B{255, 95, 195},
        cocos2d::ccColor3B{210, 160, 255},
        cocos2d::ccColor3B{55, 130, 255},
        cocos2d::ccColor3B{135, 245, 65},
        cocos2d::ccColor3B{255, 235, 50},
        cocos2d::ccColor3B{255, 190, 40},
        cocos2d::ccColor3B{40, 250, 240},
        cocos2d::ccColor3B{35, 205, 110},
        cocos2d::ccColor3B{255, 130, 150},
        cocos2d::ccColor3B{255, 255, 255},
        cocos2d::ccColor3B{180, 195, 215}
    };

    inline const std::array<const char*, 16> names = {
        "Light Blue",
        "Red",
        "Mint",
        "Orange",
        "Purple",
        "Pink",
        "Lavender",
        "Dark Blue",
        "Lime",
        "Yellow",
        "Gold",
        "Cyan",
        "Dark Green",
        "Rose",
        "White",
        "Gray"
    };

    inline cocos2d::ccColor3B getColor(int index) {
        if (index < 0) index = 0;
        return colors[static_cast<size_t>(index) % colors.size()];
    }

    inline const char* getName(int index) {
        if (index < 0) index = 0;
        return names[static_cast<size_t>(index) % names.size()];
    }

}
