#include "ChatPopup.hpp"
#include "../../SessionManager.hpp"
#include "../../utils/ColorPalette.hpp"
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>

using namespace geode::prelude;

namespace mpedit {

    static cocos2d::ccColor3B getColorForIndex(int index) {
        return ColorPalette::getColor(index);
    }

    bool ChatPopup::setup() {
        this->setTitle("Chat");
        
        auto hintLabel = CCLabelBMFont::create("Press / in-game to quick chat", "chatFont.fnt");
        hintLabel->setPosition({m_size.width / 2.f, m_size.height - 32.f});
        hintLabel->setScale(0.4f);
        hintLabel->setColor({200, 200, 200});
        m_mainLayer->addChild(hintLabel);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        float scrollHeight = m_size.height - 100.f;
        m_scrollLayer = ScrollLayer::create({m_size.width - 40.f, scrollHeight});
        m_scrollLayer->setPosition({20.f, 60.f});
        m_scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()->setGap(2.f)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End));
        m_mainLayer->addChild(m_scrollLayer);

        auto borders = ListBorders::create();
        borders->setContentSize(m_scrollLayer->getContentSize());
        borders->setPosition({m_size.width / 2.f, 60.f + scrollHeight / 2.f});
        m_mainLayer->addChild(borders);

        m_input = TextInput::create(m_size.width - 120.f, "Type a message...");
        m_input->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
        m_input->setPosition({m_size.width / 2.f - 30.f, 30.f});
        m_input->setMaxCharCount(100);
        m_mainLayer->addChild(m_input);

        auto sendSpr = ButtonSprite::create("Send");
        sendSpr->setScale(0.7f);
        auto sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(ChatPopup::onSend));
        sendBtn->setPosition({m_size.width - 50.f, 30.f});
        
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        menu->addChild(sendBtn);
        m_mainLayer->addChild(menu);

        populateChat();

        SessionManager::get().onChatMessage(this, [this](SessionManager::ChatMessage const& msg) {
            this->populateChat();
        });

        this->syncTouchPriority();
        return true;
    }

    void ChatPopup::populateChat() {
        m_scrollLayer->m_contentLayer->removeAllChildren();
        
        auto& history = SessionManager::get().getChatHistory();
        float width = m_scrollLayer->getContentSize().width;
        float currentY = 0.f;
        
        std::vector<cocos2d::CCNode*> cells;
        float totalHeight = 0.f;

        for (auto& msg : history) {
            auto cell = cocos2d::CCNode::create();
            auto* pInfo = SessionManager::get().getPlayer(msg.playerId);
            cocos2d::ccColor3B pColor = pInfo ? getPlayerEffectiveColor(pInfo->colorIndex, pInfo->iconStr) : ColorPalette::getColor(0);
            
            int cubeFrame = 1;
            cocos2d::ccColor3B col1 = pColor;
            cocos2d::ccColor3B col2 = {255, 255, 255};
            bool glowEnabled = false;
            cocos2d::ccColor3B glowCol = {0, 0, 0};

            if (pInfo && !pInfo->iconStr.empty()) {
                std::stringstream ss(pInfo->iconStr);
                std::string token;
                std::vector<std::string> tokens;
                while (std::getline(ss, token, ':')) {
                    tokens.push_back(token);
                }
                if (tokens.size() >= 5) {
                    cubeFrame = geode::utils::numFromString<int>(tokens[0]).unwrapOr(1);
                    auto gm = GameManager::sharedState();
                    col1 = gm->colorForIdx(geode::utils::numFromString<int>(tokens[1]).unwrapOr(0));
                    col2 = gm->colorForIdx(geode::utils::numFromString<int>(tokens[2]).unwrapOr(0));
                    glowEnabled = (tokens[3] == "1");
                    glowCol = gm->colorForIdx(geode::utils::numFromString<int>(tokens[4]).unwrapOr(0));
                }
            }

            auto playerIcon = SimplePlayer::create(cubeFrame);
            playerIcon->setColor(col1);
            playerIcon->setSecondColor(col2);
            if (glowEnabled) {
                playerIcon->setGlowOutline(glowCol);
            }
            playerIcon->setScale(0.5f);
            playerIcon->setPosition({15.f, 15.f});
            cell->addChild(playerIcon);
            auto nameLabel = cocos2d::CCLabelBMFont::create(fmt::format("{}:", msg.senderName).c_str(), "chatFont.fnt");
            nameLabel->setScale(0.6f);
            nameLabel->setColor(pColor);
            nameLabel->setAnchorPoint({0.f, 0.5f});
            nameLabel->setPosition({28.f, 16.f});
            cell->addChild(nameLabel);
            auto msgLabel = cocos2d::CCLabelBMFont::create(msg.message.c_str(), "chatFont.fnt");
            msgLabel->limitLabelWidth(width - 35.f - nameLabel->getScaledContentSize().width, 0.6f, 0.1f);
            msgLabel->setAnchorPoint({0.f, 0.5f});
            msgLabel->setPosition({28.f + nameLabel->getScaledContentSize().width + 4.f, 16.f});
            cell->addChild(msgLabel);

            float cellHeight = 30.f;
            cell->setContentSize({width, cellHeight});
            cells.push_back(cell);
            totalHeight += cellHeight + 5.f;
        }

        m_scrollLayer->m_contentLayer->setContentSize({width, std::max(totalHeight, m_scrollLayer->getContentSize().height)});
        
        float startY = m_scrollLayer->m_contentLayer->getContentSize().height;
        for (auto* cell : cells) {
            startY -= cell->getContentSize().height + 5.f;
            cell->setPosition({0.f, startY});
            m_scrollLayer->m_contentLayer->addChild(cell);
        }

        float contentHeight = m_scrollLayer->m_contentLayer->getContentSize().height;
        float viewHeight = m_scrollLayer->getContentSize().height;
        if (contentHeight > viewHeight) {
            m_scrollLayer->m_contentLayer->setPositionY(0.f);
        } else {
            m_scrollLayer->m_contentLayer->setPositionY(viewHeight - contentHeight);
        }
        this->syncTouchPriority();
    }

    void ChatPopup::keyDown(cocos2d::enumKeyCodes key, double p1) {
        if (key == cocos2d::KEY_Enter) {
            onSend(nullptr);
            return;
        }
        BasePopup::keyDown(key, p1);
    }

    void ChatPopup::onSend(CCObject*) {
        auto text = m_input->getString();
        if (!text.empty()) {
            SessionManager::get().sendChatMessage(std::string(text.c_str()));
            m_input->setString("");
        }
    }

    void ChatPopup::onClose(CCObject* obj) {
        SessionManager::get().removeListener(this);
        BasePopup::onClose(obj);
    }

    ChatPopup* ChatPopup::create() {
        auto ret = new ChatPopup();
        if (ret && ret->init(360.f, 260.f) && ret->setup()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

}
