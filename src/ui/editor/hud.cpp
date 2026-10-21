#include <Geode/Geode.hpp>

#include "hud.hpp"
#include "SessionManager.hpp"
#include "P2PManager.hpp"

#include <algorithm>

#include "ui/utils/Panel.hpp"

using namespace geode::prelude;

namespace mpedit {

    SessionStatusNode* SessionStatusNode::create() {
        auto* ret = new SessionStatusNode();
        if (ret && ret->init()) {
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

        auto statusPosition = CCPoint{winSize.width * 0.37f, winSize.height * 0.96f};
        m_statusBackground = Panel::create(nullptr, {160.f, 34.f});
        if (!m_statusBackground) {
            return false;
        }
        m_statusBackground->setPosition(statusPosition);
        this->addChild(m_statusBackground, 0);

        m_statusText = CCLabelBMFont::create("", "goldFont.fnt");
        if (!m_statusText) {
            log::error("SessionStatusNode: Failed to create status label");
            return false;
        }
        m_statusText->setScale(0.4f);
        m_statusText->setPosition({0.f, 0.f});
        m_statusBackground->getContent()->addChild(m_statusText);

        this->update(0.f);
        this->schedule(schedule_selector(SessionStatusNode::update), 0.25f);

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
            setVisible(false);
            return;
        }

        setVisible(true);

        std::string statusText;
        ccColor3B color{255, 255, 255};

        switch (state) {
            case P2PManager::State::Connected:
                statusText = "Connected";
                break;

            case P2PManager::State::Connecting:
                statusText = "Connecting...";
                color = {255, 255, 100};
                break;

            case P2PManager::State::Reconnecting:
                statusText = "Reconnecting...";
                color = {255, 255, 100};
                break;

            case P2PManager::State::Disconnected:
                statusText = "Disconnected";
                color = {255, 100, 100};
                break;

            case P2PManager::State::Error:
                statusText = "Error";
                color = {255, 100, 100};
                break;

            default:
                statusText = "Unknown";
                color = {255, 255, 255};
                break;
        }

        if (state == P2PManager::State::Error && !errStr.empty()) {
            constexpr size_t maxErrorLength = 36;
            statusText += fmt::format(": {}", errStr.substr(0, maxErrorLength));
            if (errStr.size() > maxErrorLength) {
                statusText += "...";
            }
        }
        if (!roomCode.empty()) {
            statusText += fmt::format(" | Room {}", roomCode);
        }
        statusText += fmt::format(" | Players {}", playerCount);

        m_statusText->setString(statusText.c_str());
        m_statusText->setColor(color);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto availableWidth = 2.f * std::min(
            m_statusBackground->getPositionX(),
            winSize.width - m_statusBackground->getPositionX()
        ) - 16.f;
        auto maxWidth = std::max(0.f, availableWidth);
        auto unscaledTextWidth = m_statusText->getContentSize().width;
        if (unscaledTextWidth > 0.f) {
            m_statusText->setScale(std::min(
                0.4f,
                std::max(0.f, maxWidth - 24.f) / unscaledTextWidth
            ));
        }
        auto textWidth = unscaledTextWidth * m_statusText->getScale();
        auto panelWidth = std::min(
            std::max(160.f, textWidth + 24.f),
            maxWidth
        );
        m_statusBackground->setPanelSize({panelWidth, 34.f});
    }

}
