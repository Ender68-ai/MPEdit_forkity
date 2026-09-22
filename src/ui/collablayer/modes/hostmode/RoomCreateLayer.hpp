#pragma once

#include <functional>

#include <Geode/Geode.hpp>

#include "ui/utils/NineSlice.hpp"

using namespace geode::prelude;

class RoomCreateLayer : public CCNode {
private:
    void onSessionStarted(std::string const& roomCode, int localPlayerId);

protected:
    TextInput* m_nameInput;
    TextInput* m_passInput;
    TextInput* m_limitInput;

    CCMenuItemToggler* m_privateToggle;
    CCMenuItemToggler* m_viewOnlyToggle;

    bool init();
    void refreshPlayerList();
    void showPlayerListDelayed(float);
    void updateRoomPassword(std::string const& password);
    void onToggleShowPassword(CCObject*);
    void onToggleHidePassword(CCObject*);
    void onCopyRoomCode(CCObject*);
    void onLeaveSession(CCObject*);

    CCLabelBMFont* m_boxTitle;
    CCNode* m_layoutNode;
    CCMenu* m_createMenu;
    ScrollLayer* m_scrollLayer;

    CCLabelBMFont* m_hostingTitle;
    CCMenuItemSpriteExtra* m_leaveSessionBtn;
    CCMenu* m_leaveSessionMenu;
    CCScale9Sprite* m_topLine;
    CCScale9Sprite* m_bottomLine;
    CCLabelBMFont* m_roomCodeLabel;
    CCMenuItemSpriteExtra* m_copyCodeBtn;
    CCMenu* m_codeButtonMenu;
    CCLabelBMFont* m_roomPwdLabel;
    CCScale9Sprite* m_passwordRect;
    CCMenuItemSpriteExtra* m_showPasswordBtn;
    CCMenuItemSpriteExtra* m_hidePasswordBtn;
    CCMenu* m_passwordToggleMenu;
    std::string m_currentPassword;
    std::string m_currentRoomCode;
    std::function<void()> m_onRoomCreated;

public:
    ~RoomCreateLayer();
    static RoomCreateLayer* create(std::function<void()> onRoomCreated = {});

    void onCreate(CCObject*);

    NineSliceBox* m_createRoomLayer;
};