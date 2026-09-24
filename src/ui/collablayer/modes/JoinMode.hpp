#pragma once

#include <Geode/Geode.hpp>
#include "../../../P2PManager.hpp"
#include <functional>

using namespace cocos2d;
using namespace geode::prelude;
using namespace mpedit;

class RoomList : public cocos2d::CCNode {
public:
    using JoinCallback = std::function<void(P2PManager::RoomInfo const&, std::string const&)>;

protected:
    JoinCallback m_onJoin;
    geode::ScrollLayer* m_scroll = nullptr;

    bool init(std::function<void(P2PManager::RoomInfo const&, std::string const&)> onJoin);
    void fetchRooms();
    void populateRooms(std::vector<P2PManager::RoomInfo> const& rooms);
    void onRefresh(cocos2d::CCObject*);

public:
    static RoomList* create(JoinCallback onJoin);
    void refresh();
};



// JoinMode: Public rooms, and current room. add animation later.   
/* if (auto layer = CollabLayer::get()) {
            log::info("COLLAB LOCAL LEVEL CLICK!");

            if (m_level) {
                log::info(
                    "Level: {} ID: {}",
                    m_level->m_levelName,
                    m_level->m_levelID
                );

                // TODO: move the level to the game tab
            }

            // Decide whether to let GD open the level:
            // LevelCell::onClick(sender);
            return;
        }

*/