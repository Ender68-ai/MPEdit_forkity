#pragma once

#include <Geode/Geode.hpp>
#include "../core/BasePopup.hpp"

namespace mpedit {

    class ChatPopup : public BasePopup {
    protected:
        geode::ScrollLayer* m_scrollLayer = nullptr;
        geode::TextInput* m_input = nullptr;

        bool setup();
        void populateChat();
        void onSend(cocos2d::CCObject*);
        void onClose(cocos2d::CCObject* obj) override;
        void keyDown(cocos2d::enumKeyCodes key, double p1) override;

    public:
        static ChatPopup* create();
    };

}
