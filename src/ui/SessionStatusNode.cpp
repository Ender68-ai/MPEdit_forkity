#include <Geode/Geode.hpp>

#include "SessionStatusNode.hpp"
#include "../SessionManager.hpp"
#include "../P2PManager.hpp"


#include "ui/utils/Panel.hpp"

using namespace geode::prelude;

namespace mpedit {

    SessionStatusNode* SessionStatusNode::create() {
        auto* ret = new SessionStatusNode();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool SessionStatusNode::init() {
        if (!CCNode::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        this->setPosition({0, 0});
        this->setAnchorPoint({0, 0});

        m_statusText = CCLabelBMFont::create("", "goldFont.fnt");
        m_statusText->setScale(0.45f);
        m_statusText->setPosition({winSize.width * 0.37f, winSize.height * 0.96f});
        this->addChild(m_statusText);

        auto statusBg = Panel::create("", {winSize.width * 0.08f, winSize.height * 0.03f});
        statusBg->setPosition({winSize.width * 0.37f, winSize.height * 0.96f});
        this->addChild(statusBg);

        

        this->scheduleUpdate();

        return true;
    }

    void SessionStatusNode::update(float dt) {
        auto& session = SessionManager::get();
        auto& net = P2PManager::get();

        bool inSession = session.isInSession();
        auto state = net.getState();
        size_t playerCount = session.getPlayers().size();
        std::string roomCode = session.getRoomCode();
        std::string errStr = net.getError();

        if (inSession == m_cachedInSession &&
            state == m_cachedState &&
            playerCount == m_cachedPlayerCount &&
            roomCode == m_cachedRoomCode &&
            errStr == m_cachedError) {
            return;
        }

        m_cachedInSession = inSession;
        m_cachedState = state;
        m_cachedPlayerCount = playerCount;
        m_cachedRoomCode = roomCode;
        m_cachedError = errStr;

        if (!inSession) {
            m_statusText->setString("");
            return;
        }

        std::string statusText;
        ccColor3B color;

        switch (state) {
            case P2PManager::State::Connected:
                statusText = fmt::format(
                    "Connected"
                );
                color = { 255, 255, 255};
                break;

            case P2PManager::State::Connecting:
            case P2PManager::State::Reconnecting:
                statusText = "Connecting...";
                color = {255, 255, 100};
                break;

            case P2PManager::State::Disconnected:
                statusText = "Disconnected";
                color = {255, 100, 100};
                break;

            case P2PManager::State::Error:
                statusText = fmt::format("Error - {}", errStr);
                color = {255, 100, 100};
                break;
        }

        m_statusText->setString(statusText.c_str());
        m_statusText->setColor(color);
    }

}
