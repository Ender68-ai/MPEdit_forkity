#include <Geode/binding/EditLevelLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/Geode.hpp>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h>

#include "ui/collablayer/CollabLayer.hpp"

class $modify(MyEditLevelLayer, EditLevelLayer) {
    void onBack(CCObject* sender) {
        if(fromCollab) {   
        fromCollab = false;
        
        auto scene = CCScene::create();
        scene->addChild(CollabLayer::create());

        auto transition = CCTransitionFade::create(0.5f, scene);
        CCDirector::sharedDirector()->replaceScene(transition);
        }
        else {
                EditLevelLayer::onBack(sender);
}
}
};