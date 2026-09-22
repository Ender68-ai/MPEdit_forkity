#include <Geode/Geode.hpp>

#include <algorithm>
#include <fmt/format.h>

#include "RoomCreateLayer.hpp"
#include "SessionManager.hpp"

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace geode::prelude;
using namespace mpedit;


RoomCreateLayer::~RoomCreateLayer() {
    mpedit::SessionManager::get().removeListener(this);
}


RoomCreateLayer* RoomCreateLayer::create(std::function<void()> onRoomCreated) {
    auto ret = new RoomCreateLayer();
    ret->m_onRoomCreated = std::move(onRoomCreated);

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}


bool RoomCreateLayer::init() {
    if (!CCNode::init())
        return false;

    auto& session = mpedit::SessionManager::get();

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto panelWidth = winSize.width * 0.8f;
    auto panelHeight = winSize.height * 0.7f;

    auto panel = NineSliceBox::create(panelWidth, panelHeight);
    panel->setPosition({winSize.width * 0.1f, winSize.height * 0.1f});
    m_createRoomLayer = panel;

    auto boxTitle = CCLabelBMFont::create("Create A Room", "goldFont.fnt");
    boxTitle->setScale(0.6f);
    boxTitle->setPosition({panelWidth * 0.5f, panelHeight - 10.f});
    panel->addChild(boxTitle);
    m_boxTitle = boxTitle;


    auto layoutNode = CCNode::create();
    layoutNode->setContentSize({260.f, 190.f});
    layoutNode->setPosition({panelWidth * 0.5f, panelHeight * 0.52f});
    layoutNode->setAnchorPoint({0.5f, 0.5f});
    layoutNode->setLayout(ColumnLayout::create()->setGap(12.f)->setAxisReverse(true));
    m_layoutNode = layoutNode;

    auto createLabeledInput = [](CCNode* parent, const char* labelStr, float width, const char* placeholder, int maxLen, geode::CommonFilter filter, geode::TextInput*& outInput) {
        auto wrapper = CCNode::create();
        wrapper->setContentSize({width, 45.f});
        wrapper->setLayout(ColumnLayout::create()->setAxisReverse(true)->setGap(4.f));

        auto label = CCLabelBMFont::create(labelStr, "goldFont.fnt");
        label->setScale(0.5f);
        wrapper->addChild(label);

        outInput = geode::TextInput::create(width, placeholder, "chatFont.fnt");
        outInput->setMaxCharCount(maxLen);
        outInput->setCommonFilter(filter);
        wrapper->addChild(outInput);
        wrapper->updateLayout();
        parent->addChild(wrapper);
    };

    createLabeledInput(layoutNode, "Room Name", 240.f, "Room Name", 32, geode::CommonFilter::Any, m_nameInput);
    m_nameInput->setString(fmt::format("{}'s room", GJAccountManager::sharedState()->m_username));

    auto inputRow = CCNode::create();
    inputRow->setContentSize({240.f, 45.f});
    inputRow->setLayout(RowLayout::create()->setGap(15.f));
    createLabeledInput(inputRow, "Passcode", 112.5f, "(none)", 11, geode::CommonFilter::Uint, m_passInput);
    createLabeledInput(inputRow, "Player limit", 112.5f, "Unlimited", 6, geode::CommonFilter::Uint, m_limitInput);
    inputRow->updateLayout();
    layoutNode->addChild(inputRow);

    auto createToggleRow = [](CCNode* parent, const char* labelStr, CCMenuItemToggler*& outToggle) {
        auto row = CCNode::create();
        row->setContentSize({240.f, 30.f});
        row->setLayout(RowLayout::create()->setGap(8.f)->setAxisAlignment(AxisAlignment::Center));

        auto label = CCLabelBMFont::create(labelStr, "goldFont.fnt");
        label->setScale(0.4f);
        row->addChild(label);

        auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        onSpr->setScale(0.6f);
        offSpr->setScale(0.6f);
        outToggle = CCMenuItemToggler::create(offSpr, onSpr, parent, nullptr);

        auto menu = CCMenu::create();
        menu->setContentSize({30.f, 30.f});
        outToggle->setPosition({15.f, 15.f});
        menu->addChild(outToggle);
        row->addChild(menu);
        row->updateLayout();
        parent->addChild(row);
    };

    createToggleRow(layoutNode, "Private Room", m_privateToggle);
    createToggleRow(layoutNode, "Default View-Only", m_viewOnlyToggle);
    layoutNode->updateLayout();
    m_layoutNode = layoutNode;
    panel->addChild(layoutNode);

    auto createButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Create", "goldFont.fnt", "GJ_button_01.png", 0.8f),
        this,
        menu_selector(RoomCreateLayer::onCreate)
    );
    auto createMenu = CCMenu::create();
    createMenu->setPosition({panelWidth * 0.5f, 10.f});
    createMenu->addChild(createButton);
    panel->addChild(createMenu);
    m_createMenu = createMenu;

    this->addChild(panel);



    // wHEN ROOM IS CREATED

    auto* hostingTitle = CCLabelBMFont::create("", "goldFont.fnt");
    hostingTitle->setScale(0.6f);
    hostingTitle->setPosition({panelWidth * 0.2f, panelHeight * 0.95f});
    hostingTitle->setAnchorPoint({0.5f, 1.f});
    hostingTitle->setVisible(false);
    panel->addChild(hostingTitle);
    m_hostingTitle = hostingTitle;
    // Leave session button
    auto leaveSprite = ButtonSprite::create("Leave", "goldFont.fnt", "GJ_button_06.png", 0.7f);
    leaveSprite->setScale(0.7f);
    auto leaveSessionBtn = CCMenuItemSpriteExtra::create(
        leaveSprite,
        this,
        menu_selector(RoomCreateLayer::onLeaveSession)
    );
    auto leaveSessionMenu = CCMenu::create();
    leaveSessionMenu->setPosition({panelWidth * 0.4f, panelHeight * 0.92f});
    leaveSessionMenu->addChild(leaveSessionBtn);
    leaveSessionMenu->setVisible(false);
    panel->addChild(leaveSessionMenu);
    m_leaveSessionBtn = leaveSessionBtn;
    m_leaveSessionMenu = leaveSessionMenu;
    // Line below title
    auto topLine = CCScale9Sprite::create("square02_small.png");
    topLine->setContentSize({panelWidth * 0.48f, 2.f});
    topLine->setScaleY(0.1f);
    topLine->setPosition({panelWidth * 0.25f, panelHeight - 32.f});
    topLine->setColor({100, 100, 100});
    topLine->setOpacity(200);
    topLine->setVisible(false);
    panel->addChild(topLine);
    m_topLine = topLine;

    // ScrollLayer with padding
    auto scrollLayer = ScrollLayer::create(
        {panelWidth * 0.5f - 10.f, panelHeight - 84.f},
        true,
        true
    );
    scrollLayer->setPosition({5.f, 47.f});
    scrollLayer->setAnchorPoint({0, 0});
    scrollLayer->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(2.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
    );
    scrollLayer->setVisible(false);
    m_scrollLayer = scrollLayer;
    panel->addChild(scrollLayer);

    // Line above bottom info
    auto bottomLine = CCScale9Sprite::create("square02_small.png");
    bottomLine->setContentSize({panelWidth * 0.48f, 2.f});
    bottomLine->setScaleY(0.1f);
    bottomLine->setPosition({panelWidth * 0.25f, 45.f});
    bottomLine->setColor({100, 100, 100});
    bottomLine->setOpacity(200);
    bottomLine->setVisible(false);
    panel->addChild(bottomLine);
    m_bottomLine = bottomLine;

    // Room code display
    auto roomCodeLabel = CCLabelBMFont::create("", "goldFont.fnt");
    roomCodeLabel->setScale(1.f);
    roomCodeLabel->setPosition({10.f, 20.f});
    roomCodeLabel->setAnchorPoint({0.f, 0.5f});
    roomCodeLabel->setVisible(false);
    panel->addChild(roomCodeLabel);
    m_roomCodeLabel = roomCodeLabel;

    // Copy room code button
    auto copyCodeSprite = ButtonSprite::create("Copy", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    copyCodeSprite->setScale(0.5f);
    auto copyCodeBtn = CCMenuItemSpriteExtra::create(
        copyCodeSprite,
        this,
        menu_selector(RoomCreateLayer::onCopyRoomCode)
    );
    auto codeButtonMenu = CCMenu::create();
    codeButtonMenu->setPosition({panelWidth * 0.45f, panelHeight * 0.08f});
    codeButtonMenu->addChild(copyCodeBtn);
    codeButtonMenu->setVisible(false);
    panel->addChild(codeButtonMenu);
    m_copyCodeBtn = copyCodeBtn;
    m_codeButtonMenu = codeButtonMenu;


    auto roomPwdLabel = CCLabelBMFont::create("", "goldFont.fnt");
    roomPwdLabel->setScale(0.5f);
    roomPwdLabel->setPosition({10.f, 35.f});
    roomPwdLabel->setAnchorPoint({0.f, 0.5f});
    roomPwdLabel->setVisible(false);
    panel->addChild(roomPwdLabel);
    m_roomPwdLabel = roomPwdLabel;

    // Password rectangle overlay
    auto passwordRect = CCScale9Sprite::create("square02_small.png");
    passwordRect->setContentSize({50.f, 15.f});
    passwordRect->setPosition({100.f, 35.f});
    passwordRect->setAnchorPoint({0.f, 0.5f});
    passwordRect->setColor({0, 0, 0});
    passwordRect->setOpacity(255);
    passwordRect->setVisible(false);
    panel->addChild(passwordRect);
    m_passwordRect = passwordRect;

    // Show password button
    auto showButtonSprite = ButtonSprite::create("Show", "goldFont.fnt", "GJ_button_01.png", 0.6f);
    showButtonSprite->setScale(0.6f);
    auto showPasswordBtn = CCMenuItemSpriteExtra::create(
        showButtonSprite,
        this,
        menu_selector(RoomCreateLayer::onToggleShowPassword)
    );
    m_showPasswordBtn = showPasswordBtn;

    // Hide password button
    auto hideButtonSprite = ButtonSprite::create("Hide", "goldFont.fnt", "GJ_button_06.png", 0.6f);
    hideButtonSprite->setScale(0.6f);
    auto hidePasswordBtn = CCMenuItemSpriteExtra::create(
        hideButtonSprite,
        this,
        menu_selector(RoomCreateLayer::onToggleHidePassword)
    );
    m_hidePasswordBtn = hidePasswordBtn;
    hidePasswordBtn->setVisible(false);

    // Password toggle menu
    auto passwordToggleMenu = CCMenu::create();
    passwordToggleMenu->setPosition({180.f, 35.f});
    passwordToggleMenu->addChild(showPasswordBtn);
    passwordToggleMenu->addChild(hidePasswordBtn);
    passwordToggleMenu->setVisible(false);
    panel->addChild(passwordToggleMenu);
    m_passwordToggleMenu = passwordToggleMenu;
    
    mpedit::SessionManager::get().onPlayerJoined(this, [this](mpedit::PlayerInfo const&) {
        if (this->m_scrollLayer) {
            this->refreshPlayerList();
        }
    });

    mpedit::SessionManager::get().onPlayerLeft(this, [this](mpedit::PlayerInfo const&) {
        if (this->m_scrollLayer) {
            this->refreshPlayerList();
        }
    });

   mpedit::SessionManager::get().onSessionStarted(this, [this]() {
    if (!this->m_createRoomLayer)
        return;

    this->onSessionStarted(
        mpedit::SessionManager::get().getRoomCode(),
        mpedit::SessionManager::get().getLocalPlayerId()
    );
    });

    if (session.isInSession()) {
        this->onSessionStarted(
            session.getRoomCode(),
            session.getLocalPlayerId()
        );
    }

    return true;
}

void RoomCreateLayer::refreshPlayerList() {
    if (!m_scrollLayer || !m_scrollLayer->m_contentLayer) {
        return;
    }

    m_scrollLayer->m_contentLayer->removeAllChildren();

    auto const& players = mpedit::SessionManager::get().getPlayers();
    
    if (players.empty()) {
        m_scrollLayer->m_contentLayer->setContentHeight(std::max(m_scrollLayer->getContentSize().height, 10.f));
        m_scrollLayer->m_contentLayer->updateLayout();
        return;
    }

    static const std::array<ccColor3B, 6> playerColors = {
        ccColor3B{100, 200, 255},
        ccColor3B{255, 120, 100},
        ccColor3B{100, 255, 150},
        ccColor3B{255, 200, 100},
        ccColor3B{200, 150, 255},
        ccColor3B{255, 150, 200},
    };

    for (auto const& player : players) {
        auto row = CCNode::create();
        row->setContentSize({m_scrollLayer->getContentSize().width, 30.f});
        row->setLayout(RowLayout::create()->setGap(6.f)->setAxisAlignment(AxisAlignment::Center));

        // Background
        auto bg = CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({m_scrollLayer->getContentSize().width, 30.f});
        bg->setAnchorPoint({0, 0});
        bg->setOpacity(90);
        bg->setColor({0, 0, 0});
        row->addChild(bg, 0);

        // Player icon
        auto icon = SimplePlayer::create(1);
        icon->setScale(0.5f);
        icon->setPosition({15.f, 15.f});
        row->addChild(icon, 1);

        // Player name
        auto nameColor = playerColors[player.colorIndex % playerColors.size()];
        auto name = CCLabelBMFont::create(player.name.c_str(), "goldFont.fnt");
        name->setAnchorPoint({0, 0.5f});
        name->setPosition({35.f, 15.f});
        name->setScale(0.4f);
        name->setColor(nameColor);
        row->addChild(name, 1);

        m_scrollLayer->m_contentLayer->addChild(row);
    }

    float totalPlayersHeight = players.size() * 30.f + std::max(0, static_cast<int>(players.size()) - 1) * 2.f;
    m_scrollLayer->m_contentLayer->setContentHeight(std::max(m_scrollLayer->getContentSize().height, totalPlayersHeight));
    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->m_contentLayer->setPositionY(
        std::max(0.f, m_scrollLayer->getContentSize().height - m_scrollLayer->m_contentLayer->getContentSize().height)
    );
}

void RoomCreateLayer::onCreate(CCObject*) {
    mpedit::RoomSettings settings;
    settings.roomName = m_nameInput->getString();
    if (settings.roomName.empty())
        settings.roomName = fmt::format("{}'s room", GJAccountManager::sharedState()->m_username);

    std::string limit = m_limitInput->getString();
    if (!limit.empty()) {
        // @geode-ignore(geode-alternative)
        settings.playerLimit = std::stoi(limit);
    } else {
        settings.playerLimit = 0;
    }

    settings.password = m_passInput->getString();
    settings.isPrivate = m_privateToggle->isToggled();
    settings.defaultViewOnly = m_viewOnlyToggle->isToggled();
    mpedit::SessionManager::get().hostSession(
        Mod::get()->getSettingValue<std::string>("player-name"),
        settings
    );
}

void RoomCreateLayer::onSessionStarted(std::string const& roomCode, int playerId) {
    if (!m_createRoomLayer || !m_boxTitle || !m_layoutNode || !m_createMenu || !m_hostingTitle) {
        return;
    }
    log::info("ROOM CREATE UI RECEIVED SESSION START");

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_hostingTitle->setString(
    m_nameInput->getString().c_str()
    );

    m_createRoomLayer->animateResize( winSize.width * 0.4f, winSize.height * 0.7f, 0.5f);

    m_boxTitle->setVisible(false);
    m_layoutNode->setVisible(false);
    m_createMenu->setVisible(false);
    m_hostingTitle->setVisible(true);
    m_leaveSessionMenu->setVisible(true);
    m_topLine->setVisible(true);
    m_bottomLine->setVisible(true);
    m_roomCodeLabel->setString(fmt::format("Code: {}", roomCode).c_str());
    m_roomCodeLabel->setVisible(true);
    m_currentRoomCode = roomCode;
    m_codeButtonMenu->setVisible(true);
    std::string password = m_passInput->getString();
    if (!password.empty()) {
        updateRoomPassword(password);
    }
    
    // Defer refreshPlayerList to next frame to ensure SessionManager has populated players
    this->scheduleOnce(schedule_selector(RoomCreateLayer::showPlayerListDelayed), 0.01f);

    if (m_onRoomCreated) {
        m_onRoomCreated();
    }
}

void RoomCreateLayer::showPlayerListDelayed(float) {
    if (m_scrollLayer) {
        refreshPlayerList();
        m_scrollLayer->setVisible(true);
    }
}

void RoomCreateLayer::updateRoomPassword(std::string const& password) {
    m_currentPassword = password;
    if (m_roomPwdLabel) {
        m_roomPwdLabel->setString(fmt::format("Password: {}", password).c_str());
        m_roomPwdLabel->setVisible(true);
    }
    if (m_passwordRect) {
        m_passwordRect->setVisible(true);
    }
    if (m_passwordToggleMenu) {
        m_passwordToggleMenu->setVisible(true);
    }
    if (m_showPasswordBtn) {
        m_showPasswordBtn->setVisible(true);
    }
    if (m_hidePasswordBtn) {
        m_hidePasswordBtn->setVisible(false);
    }
}

void RoomCreateLayer::onToggleShowPassword(CCObject*) {
    if (m_passwordRect) {
        m_passwordRect->setVisible(false);
    }
    if (m_showPasswordBtn) {
        m_showPasswordBtn->setVisible(false);
    }
    if (m_hidePasswordBtn) {
        m_hidePasswordBtn->setVisible(true);
    }
}

void RoomCreateLayer::onToggleHidePassword(CCObject*) {
    if (m_passwordRect) {
        m_passwordRect->setVisible(true);
    }
    if (m_showPasswordBtn) {
        m_showPasswordBtn->setVisible(true);
    }
    if (m_hidePasswordBtn) {
        m_hidePasswordBtn->setVisible(false);
    }
}

void RoomCreateLayer::onCopyRoomCode(CCObject*) {
    #ifdef _WIN32
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, m_currentRoomCode.size() + 1);
        if (hGlobal) {
            memcpy(GlobalLock(hGlobal), m_currentRoomCode.c_str(), m_currentRoomCode.size() + 1);
            GlobalUnlock(hGlobal);
            if (OpenClipboard(nullptr)) {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hGlobal);
                CloseClipboard();
            }
        }
    #endif
}

void RoomCreateLayer::onLeaveSession(CCObject*) {
    mpedit::SessionManager::get().leaveSession();

    log::info(
        "after leave: isInSession = {}",
        mpedit::SessionManager::get().isInSession()
    );

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    m_createRoomLayer->animateResize( winSize.width * 0.8f, winSize.height * 0.7f, 0.5f);

    m_boxTitle->setVisible(true);
    m_layoutNode->setVisible(true);
    m_createMenu->setVisible(true);
    m_hostingTitle->setVisible(false);
    m_leaveSessionMenu->setVisible(false);
    m_topLine->setVisible(false);
    m_bottomLine->setVisible(false);
    m_roomCodeLabel->setVisible(false);
    m_codeButtonMenu->setVisible(false);
    m_scrollLayer->setVisible(false);


}



