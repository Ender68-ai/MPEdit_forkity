#pragma once

#include <cocos2d.h>
#include <cocos-ext.h>
#include <unordered_map>
#include "../SessionManager.hpp"

class SimplePlayer;

namespace mpedit {

    class CursorNode : public cocos2d::CCNode {
    protected:
        struct PlayerCursor {
            cocos2d::CCDrawNode* drawNode = nullptr;
            cocos2d::CCLabelBMFont* label = nullptr;
            cocos2d::CCNode* toolIndicator = nullptr;
            SimplePlayer* playtestIcon = nullptr;
            SimplePlayer* playtestInnerCube = nullptr;
            SimplePlayer* playtestIcon2 = nullptr;
            SimplePlayer* playtestInnerCube2 = nullptr;
            SimplePlayer* waveCursor = nullptr;
            cocos2d::CCSprite* lockIcon = nullptr;
            std::string lastStatus;
            float targetX = 0.f;
            float targetY = 0.f;
            float target2X = 0.f;
            float target2Y = 0.f;
        };

        std::unordered_map<int, PlayerCursor> m_cursors;
        cocos2d::CCDrawNode* m_selectionDrawNode = nullptr;

        bool init() override;
        void update(float dt) override;

    public:
        ~CursorNode();
        static CursorNode* create();
        
        static cocos2d::ccColor3B getColorForIndex(int index);
        static void drawCursor(cocos2d::CCDrawNode* node, int cursorType, cocos2d::ccColor3B color);
    };

} 

