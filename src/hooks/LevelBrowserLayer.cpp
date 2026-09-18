#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include "ui/ui.hpp"
#include "ui/collablayer/CollabLayer.hpp"

class $modify(MyLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* object) {
        if (!LevelBrowserLayer::init(object))
            return false;

        auto myLevelsMenu = this->getChildByID("my-levels-menu");
        if (!myLevelsMenu) {
            log::info("my-levels-menu was not found");
            return false;
        }

        auto btnSprite = CCSprite::create("button2.png"_spr);
        if (!btnSprite) {
            log::info("Could not load button2.png");
            return false;
        }
        btnSprite->setScale(0.4f);

        auto button = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(MyLevelBrowserLayer::onMyButton)
        );
        auto menuPos = myLevelsMenu->getPosition();
        button->setPosition({
            menuPos.x - 5.f,
            menuPos.y + 50.f
        });
        this->addChild(button);

        return true;
        }
        void onMyButton(CCObject* sender) {
        auto collabLayer = CollabLayer::create();
        auto scene = CCScene::create();
        scene->addChild(collabLayer);
        auto transition = Transition::create(0.5f, scene, {0, 0, 0});
        CCDirector::sharedDirector()->replaceScene(transition);
        };
    };