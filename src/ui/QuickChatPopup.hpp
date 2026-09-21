#pragma once
#include <Geode/Geode.hpp>
#include "core/BasePopup.hpp"

namespace mpedit {

    class QuickChatPopup : public BasePopup {
    protected:
        bool m_isClosing = false;
        void onClose(cocos2d::CCObject*) override;
    protected:
        geode::TextInput* m_input = nullptr;

        bool setup();
        void keyDown(cocos2d::enumKeyCodes key, double p1) override;
        void onSend(cocos2d::CCObject*);

    public:
        static QuickChatPopup* create();
    };

}
