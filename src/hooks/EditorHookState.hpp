#pragma once

#include <Geode/Geode.hpp>
#include <set>
#include <unordered_map>

namespace mpedit::hookstate {
    inline int s_selectedObjectID = 1;
    inline bool s_inTransformSync = false;
    inline cocos2d::CCPoint s_lastTouchPos = {0.f, 0.f};
    inline bool s_isTouching = false;
    inline std::set<GameObject*> s_startPosObjects;
    inline std::unordered_map<GameObject*, std::string> s_startPosSaveStrings;
}

namespace mpedit {
    void updateStartPosCache(GameObject* obj);
}
