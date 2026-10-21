#include <Geode/Geode.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Loader.hpp>
#include <alphalaneous.alphas_geode_utils/include/ObjectModify.hpp>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h>

#include <include/uibuilder/UIBuilder.hpp>
#include <include/globed/GlobedMenuLayer.hpp>
#include "ui/collablayer/CollabLayer.hpp"

using namespace cocos2d;

class $classModify(MyGlobedMenuLayer, globed::GlobedMenuLayer) {
    struct Fields {
        CCMenu* menu = nullptr;
        CCMenuItemSpriteExtra* button = nullptr;
    };

    void onGlobedButton(CCObject*) {
        auto* collabLayer = CollabLayer::create();
        if (!collabLayer) {
            geode::log::error("Globed integration: Failed to create MPEdit layer");
            geode::Notification::create("Could not open MPEdit.", geode::NotificationIcon::Error)->show();
            return;
        }

        auto* scene = CCScene::create();
        if (!scene) {
            geode::log::error("Globed integration: Failed to create MPEdit scene");
            geode::Notification::create("Could not open MPEdit.", geode::NotificationIcon::Error)->show();
            return;
        }
        scene->addChild(collabLayer);
        auto* transition = CCTransitionFade::create(0.5f, scene);
        if (!transition) {
            geode::log::error("Globed integration: Failed to create scene transition");
            geode::Notification::create("Could not open MPEdit.", geode::NotificationIcon::Error)->show();
            return;
        }
        CCDirector::sharedDirector()->pushScene(transition);
    }

    void refreshButton(float) {
        auto menu = dynamic_cast<CCMenu*>(this->getChildByID("far-left-menu"));
        if (!menu)
            return;

        if (menu->getChildByID("collab-button"))
            return;

        auto spr = CCSprite::create("button2.png"_spr);
        if (!spr) {
            geode::log::error("Globed integration: Failed to create MPEdit button sprite");
            return;
        }

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(MyGlobedMenuLayer::onGlobedButton)
        );
        if (!btn) {
            geode::log::error("Globed integration: Failed to create MPEdit menu item");
            return;
        }

        btn->setID("collab-button");
        btn->setScale(0.4f);

        menu->addChild(btn);
        menu->updateLayout();

        geode::log::info("Added MPEdit button to Globed menu");
    }

    void modify() {
        if (geode::Loader::get()->isModLoaded("dankmeme.globed2")) {
            this->schedule(schedule_selector(MyGlobedMenuLayer::refreshButton), 0.5f);
        }
    }
};