#include "settings.hpp"
#include "../../utils/Panel.hpp"

// SettingsLayer

SettingsLayer* SettingsLayer::create() {
    auto ret = new SettingsLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}


bool SettingsLayer::init() {
    if (!CCLayer::init())
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto background = CCSprite::create("GJ_gradientBG.png");    
    background->setScaleX(winSize.width / background->getContentSize().width);
    background->setScaleY(winSize.height / background->getContentSize().height);
    background->setPosition({winSize.width / 2, winSize.height / 2});
    background->setColor({ 120, 161, 255 });
    this->addChild(background, -10);

    // Backbutton :P
    auto backSprite = CCSprite::create("backbtn.png"_spr);
    backSprite->setAnchorPoint({0.5f, 0.5f});
    backSprite->setScale(0.2f);
    backSprite->setRotation(270.0f);
    backSprite->setColor({ 255, 255, 255 });
    backSprite->setOpacity(255);
    backSprite->setCascadeColorEnabled(true);
    backSprite->setCascadeOpacityEnabled(true);
    backSprite->setPosition({winSize.width * 0.02f, winSize.height * 0.95f});

    auto backButton = CCMenuItemSpriteExtra::create(
        backSprite,
        this,
        menu_selector(SettingsLayer::onBack)
    );
    backButton->setScale(0.8f);
    
    backButton->setPosition({
        winSize.width * 0.02f,
        winSize.height * 0.92f
    });


    auto backMenu = CCMenu::create();
        backMenu->setID("BackMenu"_spr);
        backMenu->setPosition(CCPoint(winSize.width * 0.04f, (float)(0)));
        backMenu->addChild(backButton);
        addChild(backMenu);

    auto title = Panel::create("Settings", {500.f, 50.f});
    title->setPosition({winSize.width / 2 + 20.f, winSize.height * 0.9f});
    addChild(title);

    auto list = Panel::create("Tab List", {400.f, 500.f});
    list->setPosition({winSize.width, winSize.height});
    addChild(list);


    auto Home = Panel::create("Home", {260.f, 320.f});
    Home->setPosition({400.f, 180.f});
    addChild(Home);

    
    return true;

}

void SettingsLayer::onBack(CCObject* sender) {
    CCDirector::sharedDirector()->popSceneWithTransition(
        0.5f, 
        cocos2d::PopTransition()
    );
}
