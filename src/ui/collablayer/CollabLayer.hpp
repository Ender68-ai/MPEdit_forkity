#pragma once

#include <Geode/Geode.hpp>

#include "../utils/NineSlice.hpp"
#include "modes/HostMode.hpp"


using namespace geode::prelude;

class CollabLayer : public CCLayer, public TableViewCellDelegate {
protected:
    bool init() override;
    void onBack(CCObject*);
    void onSettings(CCObject*);
    void updateStatus(float dt);

    bool fromCollab = false;

    bool m_collabState = false; // false = join, true = host

    void onDiscord(CCObject*);
    void onPatreon(CCObject*);
    void onWeb(CCObject*);

    void updateExtMenu(float dt);

    CCNode* m_webBtn;

    HostMode* m_hostMode = nullptr;

public:
    static CollabLayer* create();
    static CollabLayer *get() {
        auto* runningScene = cocos2d::CCDirector::sharedDirector()->getRunningScene();
        if (!runningScene) return nullptr;
        for (auto* child : CCArrayExt<CCNode*>(runningScene->getChildren())) {
            if (auto* layer = typeinfo_cast<CollabLayer*>(child)) {
                return layer;
            }
        }
        return nullptr;
    };
    CCSprite* m_onlineSprite = nullptr;
    CCSprite* m_offlineSprite = nullptr;
    CCLabelBMFont* m_playerCountLabel = nullptr;

    void onHostMode(CCObject*);
    void onJoinMode(CCObject*);

    void onMultiplayer(CCObject*);

   
    CCMenu* m_roomListMenu;
    NineSliceBox* m_publicRoomList;
    CCMenuItemSpriteExtra* m_roomListButton;

};

