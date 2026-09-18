#include <Geode/Geode.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelCell.hpp>

#include "settings/settings.hpp"
#include "CollabLayer.hpp"
#include "SessionManager.hpp"
#include "ui/utils/Panel.hpp"
#include "ui/ui.hpp"
#include "ui/menu/MultiplayerMenuPopup.hpp"


using namespace geode::prelude;
using namespace mpedit;


// Create


CollabLayer* CollabLayer::create() {
    auto ret = new CollabLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
// initialize fromcollab so hooks work

bool fromCollab = false;

// init
bool CollabLayer::init() {
    if (!CCLayer::init())
        return false;

    
    this->setID("collab-layer"_spr);

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto background = CCSprite::create("GJ_gradientBG.png");    
    background->setScaleX(winSize.width / background->getContentSize().width);
    background->setScaleY(winSize.height / background->getContentSize().height);
    background->setPosition({winSize.width / 2, winSize.height / 2});
    background->setColor({ 120, 161, 255 });
    this->addChild(background, -10);


    auto backSprite = CCSprite::create("../resources/backbtn.png"_spr);
    backSprite->setAnchorPoint({0.5f, 0.5f});
    backSprite->setScale(0.2f);
    backSprite->setRotation(270.0f);
    backSprite->setPosition({winSize.width * 0.02f, winSize.height * 0.95f});

    auto backButton = CCMenuItemSpriteExtra::create(
        backSprite,
        this,
        menu_selector(CollabLayer::onBack)
    );

    auto settingsSprite = CCSprite::create("../resources/settingsbtn.png"_spr);
    settingsSprite->setAnchorPoint({0.5f, 0.5f});
    settingsSprite->setScale(0.25f);
    settingsSprite->setColor({ 250, 243, 243 });
    settingsSprite->setOpacity(255);
    settingsSprite->setCascadeColorEnabled(true);
    settingsSprite->setCascadeOpacityEnabled(true); 
    
    auto &session = SessionManager::get();
    auto playerCount = session.getPlayers().size();

    auto settingsButton = CCMenuItemSpriteExtra::create(
        settingsSprite,
        this,
        menu_selector(CollabLayer::onSettings)
    );


    // mode tag and buttons

    auto modeTag = Panel::create("", {winSize.width * 0.4f, 50.f});
    modeTag->setPosition({winSize.width * 0.5f, winSize.height * 0.92f});
    modeTag->setScale(0.8f);


    auto jointxt = CCLabelBMFont::create("JOIN", "goldFont.fnt");

    auto joinmodebtn = CCMenuItemSpriteExtra::create(
        jointxt,
        this,
        menu_selector(CollabLayer::onJoinMode)
    );

    joinmodebtn->setScale(0.6f);

    joinmodebtn->setPosition({
        winSize.width * 0.4f,
        winSize.height * 0.92f
    });

    auto hosttxt = CCLabelBMFont::create("HOST", "goldFont.fnt");

    auto hostmodebtn = CCMenuItemSpriteExtra::create(
        hosttxt,
        this,
        menu_selector(CollabLayer::onHostMode)
    );

    hostmodebtn->setScale(0.6f);

    hostmodebtn->setPosition({
        winSize.width * 0.6f,
        winSize.height * 0.92f
    });

    auto modeMenu = CCMenu::create();
    modeMenu->setID("CollabStateMenu"_spr);
    modeMenu->setPosition(0, 0);
    modeMenu->addChild(modeTag);
    modeMenu->addChild(joinmodebtn);
    modeMenu->addChild(hostmodebtn);
    addChild(modeMenu);

    // BackMenu
    
    backButton->setPosition({
        winSize.width * 0.02f,
        winSize.height * 0.92f
    });

    settingsButton->setPosition({
        winSize.width * 0.02f,
        winSize.height * 0.80f
    });

    auto backMenu = CCMenu::create();
    backMenu->setID("BackMenu"_spr);
    backMenu->setPosition(CCPoint(winSize.width * 0.04f, (float)(0)));
    backMenu->addChild(backButton);
    backMenu->addChild(settingsButton);
    addChild(backMenu);

    // SessionMenu

    bool isInSession = session.isInSession();

    m_onlineSprite = CCSprite::create("../resources/online.png"_spr);
    m_onlineSprite->setScale(0.5f);

    m_offlineSprite = CCSprite::create("../resources/offline.png"_spr);
    m_offlineSprite->setScale(0.5f);

    
    bool online = SessionManager::get().isInSession();

    m_onlineSprite->setVisible(online);
    m_offlineSprite->setVisible(!online);

    

    m_offlineSprite->setPosition({
        winSize.width * 0.8f,
        winSize.height * 0.9f
    });
    m_onlineSprite->setPosition({
        winSize.width * 0.8f,
        winSize.height * 0.9f
    });
    m_onlineSprite->setScale(0.2f);
    m_offlineSprite->setScale(0.22f);
    m_playerCountLabel = CCLabelBMFont::create(
    fmt::format("{}", playerCount).c_str(),
    "bigFont.fnt"
    );
    m_playerCountLabel->setScale(0.5f);
    m_playerCountLabel->setPosition({
    winSize.width * 0.75f,
    winSize.height * 0.9f
    });

    auto discordSpr = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    discordSpr->setScale(1.0f);
    auto discordBtn = CCMenuItemSpriteExtra::create(discordSpr, this, menu_selector(CollabLayer::onDiscord));
    discordBtn->setPosition({
        winSize.width * 0.95f,
        winSize.height * 0.9f
    });

    auto patreonIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    auto patreonSpr = CircleButtonSprite::create(
        patreonIcon, CircleBaseColor::Pink, CircleBaseSize::Small
    );
    patreonSpr->setScale(0.85f);
    auto patreonBtn = CCMenuItemSpriteExtra::create(patreonSpr, this, menu_selector(CollabLayer::onPatreon));
    patreonBtn->setPosition({
        winSize.width * 0.95f,
        winSize.height * 0.8f
    });

    auto webSpr = CCSprite::create("../resources/webspr.png"_spr);
    auto webBtn = CCMenuItemSpriteExtra::create(webSpr, this, menu_selector(CollabLayer::onWeb));
    webBtn->setPosition({
        winSize.width * 0.95f,
        winSize.height * 0.7f
    });
    webBtn->setScale(0.3f);
    m_webBtn = webBtn;
        
    auto SessionMenu = CCMenu::create();
    SessionMenu->setID("SessionMenu"_spr);
    SessionMenu->addChild(m_onlineSprite);
    SessionMenu->addChild(m_offlineSprite);
    SessionMenu->addChild(m_playerCountLabel);
    SessionMenu->addChild(patreonBtn);
    SessionMenu->addChild(discordBtn);
    SessionMenu->addChild(webBtn);
    SessionMenu->setPosition(0, 0);
    addChild(SessionMenu);

    this->schedule(
            schedule_selector(CollabLayer::updateExtMenu),
            0.1f
    );
    this->schedule(
            schedule_selector(CollabLayer::updateStatus),
            0.25f
    );      

    // JoinModes implementatiom
    // PublicRoomList
    
    auto panel = NineSliceBox::create(winSize.width * 0.8f, winSize.height * 0.7f);
    panel->setPosition({
        winSize.width * 0.1f,
        winSize.height * 0.1f
    });
    m_publicRoomList = panel;

    auto boxTitle = CCLabelBMFont::create("Public Rooms", "goldFont.fnt");
    boxTitle->setScale(0.6f);
    boxTitle->setPosition({
        winSize.width * 0.4f,
        winSize.height * 0.65f
    });



    panel->addChild(boxTitle);
    this->addChild(panel);
    


    /// Placeholder button (relocate for now)

    auto* roomListBtnSpr = ButtonSprite::create(
            "Multiplayer Edit", 90, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.45f);

    auto* roomListButton = CCMenuItemSpriteExtra::create(
        roomListBtnSpr,
        this,
        menu_selector(CollabLayer::onMultiplayer)
    );
    m_roomListButton = roomListButton;

    auto roomListMenu = CCMenu::create();
    roomListMenu->setPosition({
        winSize.width * 0.7f,
        winSize.height * 0.15f
    });
    m_roomListMenu = roomListMenu;
    roomListMenu->addChild(roomListButton);
    this->addChild(roomListMenu);


    // HostModes implementation
    auto hostMode = HostMode::create();

    if (!hostMode)
        return false;

    m_hostMode = hostMode;
    this->addChild(m_hostMode);
    m_hostMode->setVisible(false);
    // end of HostMode


    if(m_collabState) {
        CollabLayer::onHostMode(nullptr);
    } else {
        CollabLayer::onJoinMode(nullptr);
    }


    return true;
};


void CollabLayer::onBack(CCObject* sender) {
    auto& session = SessionManager::get();

    if (session.isInSession()) {
        session.leaveSession();
    }
        auto levelBrowserLayer = LevelBrowserLayer::create(GJSearchObject::create(SearchType::MyLevels));
        auto scene = CCScene::create();
        scene->addChild(levelBrowserLayer);
        auto transition = Transition::create(0.5f, scene, {0, 0, 0});
        CCDirector::sharedDirector()->replaceScene(transition);
    };



void CollabLayer::onSettings(CCObject*) {
    auto SettingsLayer = SettingsLayer::create();
    auto scene = CCScene::create();
    scene->addChild(SettingsLayer);
    auto transition = Transition::create(0.5f, scene, {0, 0, 0});
    CCDirector::sharedDirector()->pushScene(transition);
};

void CollabLayer::updateStatus(float) {
        auto &session = SessionManager::get();

        bool online = SessionManager::get().isInSession();
        size_t playerCount = session.getPlayers().size();


        m_onlineSprite->setVisible(online);
        m_offlineSprite->setVisible(!online);
        m_playerCountLabel->setString(
            fmt::format("{}", playerCount).c_str()
        );

};

void CollabLayer::onDiscord(CCObject*) {
        createQuickPopup(
            "Discord",
            "Join the <cy>Multiplayer Edit</c> Discord server?",
            "Cancel", "Join",
            [](auto, bool btn2) {
                if (btn2) geode::utils::web::openLinkInBrowser("https://discord.gg/mdsuxYu2YP");
            }
        );
};

void CollabLayer::onPatreon(CCObject*) {
        createQuickPopup(
            "Patreon",
            "Support me on <cy>Patreon</c>?",
            "Cancel", "Open",
            [](auto, bool btn2) {
                if (btn2) geode::utils::web::openLinkInBrowser("https://www.patreon.com/cw/d050/membership");
            }
        );
};

void CollabLayer::onWeb(CCObject*) {
    createQuickPopup(
        "Web Page",
        "Visit our web page?",
        "Cancel", "Visit",
        [](auto, bool btn2) {
            if (btn2) geode::utils::web::openLinkInBrowser("http://mpedit-web.duckdns.org:8001");
        }
    );
};

void CollabLayer::updateExtMenu(float dt) {
    m_webBtn->setScale(0.3f);
}


void CollabLayer::onJoinMode(CCObject*) {

    m_roomListMenu->setVisible(true);

    m_publicRoomList->setVisible(true);

    m_roomListButton->setVisible(true);

    m_hostMode->setVisible(false);

}

void CollabLayer::onHostMode(CCObject*) {
    m_roomListMenu->setVisible(false);

    m_publicRoomList->setVisible(false);

    m_roomListButton->setVisible(false);

    m_hostMode->setVisible(true);

}


void CollabLayer::onMultiplayer(CCObject*) {
        MultiplayerMenuPopup::create()->show();
    }



