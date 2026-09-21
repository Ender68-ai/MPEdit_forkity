#include "QuickChatPopup.hpp"
#include "../SessionManager.hpp"

using namespace geode::prelude;

namespace mpedit {

    bool QuickChatPopup::setup() {
        m_noElasticity = true;
        
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto size = CCSize(340.f, 50.f);
        this->setTitle("");
        m_mainLayer->setContentSize(size);
        m_mainLayer->setPosition({winSize.width / 2, 40.f});
        
        m_input = TextInput::create(260.f, "Type message...");
        m_input->setPosition({140.f, 25.f});
        m_input->setMaxCharCount(100);
        m_input->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
        m_input->getInputNode()->setLabelPlaceholderColor({200, 200, 200});
        m_mainLayer->addChild(m_input);
        
        auto sendSpr = ButtonSprite::create("Send");
        sendSpr->setScale(0.6f);
        auto sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(QuickChatPopup::onSend));
        sendBtn->setPosition({300.f, 25.f});
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        menu->addChild(sendBtn);
        m_mainLayer->addChild(menu);
        
        m_input->getInputNode()->onClickTrackNode(true);
        this->syncTouchPriority();

        return true;
    }

    void QuickChatPopup::onClose(cocos2d::CCObject* sender) {
        m_isClosing = true;
        BasePopup::onClose(sender);
    }

    void QuickChatPopup::keyDown(cocos2d::enumKeyCodes key, double p1) {
        if (key == cocos2d::KEY_Enter) {
            onSend(nullptr);
            return;
        }
        BasePopup::keyDown(key, p1);
    }

    void QuickChatPopup::onSend(CCObject*) {
        auto text = m_input->getString();
        if (!text.empty()) {
            SessionManager::get().sendChatMessage(std::string(text.c_str()));
        }
        this->onClose(nullptr);
    }

    QuickChatPopup* QuickChatPopup::create() {
        auto ret = new QuickChatPopup();
        if (ret && ret->init(340.f, 50.f, "GJ_square01.png") && ret->setup()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
}
