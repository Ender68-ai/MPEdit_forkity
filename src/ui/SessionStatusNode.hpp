#pragma once

#include <cocos2d.h>
#include "../P2PManager.hpp"

namespace mpedit {

    class SessionStatusNode : public cocos2d::CCNode {
    protected:
        cocos2d::CCLabelBMFont* m_statusText = nullptr;
        P2PManager::State m_cachedState = P2PManager::State::Disconnected;
        size_t m_cachedPlayerCount = 0;
        std::string m_cachedRoomCode;
        std::string m_cachedError;
        bool m_cachedInSession = false;

        bool init() override;
        void update(float dt) override;

    public:
        static SessionStatusNode* create();
    };

}
