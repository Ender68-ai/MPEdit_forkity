#pragma once

#include <Geode/Geode.hpp>
#include <cocos-ext.h>
#include "../core/BasePopup.hpp"
#include "../../SessionManager.hpp"
#include "../../utils/ColorPalette.hpp"
#include <vector>

namespace mpedit {

    class AppearancePopup : public BasePopup {
    protected:
        int m_selectedType = 0;
        int m_selectedColor = 0;

        cocos2d::CCNode* m_previewBox = nullptr;
        cocos2d::CCDrawNode* m_previewArrow = nullptr;
        SimplePlayer* m_previewWave = nullptr;
        cocos2d::CCLabelBMFont* m_previewName = nullptr;

        cocos2d::CCNode* m_paletteContainer = nullptr;
        cocos2d::CCNode* m_waveInfoContainer = nullptr;

        std::vector<CCMenuItemSpriteExtra*> m_typeButtons;
        std::vector<cocos2d::CCDrawNode*> m_typeDrawNodes;
        cocos2d::CCSprite* m_cursorTypeSelect = nullptr;

        std::vector<CCMenuItemSpriteExtra*> m_colorButtons;
        cocos2d::CCSprite* m_cursorSelect = nullptr;

        bool init() override;
        void updateTypeSelection();
        void updateCursorListColors();
        void updatePreview();
        void updateColorSelection();

        void onSelectType(cocos2d::CCObject* sender);
        void onSelectColor(cocos2d::CCObject* sender);
        void onSave(cocos2d::CCObject* sender);

    public:
        static AppearancePopup* create();
    };

}
