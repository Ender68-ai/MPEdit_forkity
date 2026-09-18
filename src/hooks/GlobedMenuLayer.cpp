#include <Geode/Geode.hpp>
#include <Geode/loader/Log.hpp>
#include <alphalaneous.alphas_geode_utils/include/ObjectModify.hpp>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h>

#include <include/uibuilder/UIBuilder.hpp>
#include <include/globed/GlobedMenuLayer.hpp>


using namespace cocos2d;

class $classModify(MyGlobedMenuLayer, globed::GlobedMenuLayer) {
    struct Fields {
        CCMenu* menu = nullptr;
        CCMenuItemSpriteExtra* button = nullptr;
    };

    void onGlobedButton(CCObject*) {
        auto collabLayer = CollabLayer::create();
        auto scene = CCScene::create();
        scene->addChild(collabLayer);
        auto transition = Transition::create(0.5f, scene, {0,0,0});
        CCDirector::sharedDirector()->pushScene(transition);
    }
    void refreshButton(float) {
    auto menu = dynamic_cast<CCMenu*>(this->getChildByID("far-left-menu"));
    if (!menu)
        return;

    if (menu->getChildByID("collab-button"))
        return;

    auto spr = CCSprite::create("../resources/button2.png"_spr);

    auto btn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(MyGlobedMenuLayer::onGlobedButton)
    );

    btn->setID("collab-button");
    btn->setScale(0.4f);

    menu->addChild(btn);
    menu->updateLayout();

    geode::log::info("Re-added button");
    }

    void modify() {
        if (UI::useGlobed()) {
            this->schedule(schedule_selector(MyGlobedMenuLayer::refreshButton), 0.5f);
        }
    }
};