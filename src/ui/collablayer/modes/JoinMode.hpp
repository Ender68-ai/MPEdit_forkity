#pragma once

#include <Geode/Geode.hpp>

using namespace cocos2d;
using namespace geode::prelude;

class RoomList {
protected:
    

public:
    static RoomList* create();
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