#include <Geode/binding/EditLevelLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/Geode.hpp>

#include <Geode/cocos/layers_scenes_transitions_nodes/CCTransition.h>
#include "ui/ui.hpp"
#include "ui/collablayer/CollabLayer.hpp"

class $modify(MyEditLevelLayer, EditLevelLayer) {
    void onBack(CCObject* sender) {
        if(fromCollab) {   
        fromCollab = false;
        
        auto scene = CCScene::create();
        scene->addChild(CollabLayer::create());

        auto transition = Transition::create(0.5f, scene, {0, 0, 0});
        CCDirector::sharedDirector()->replaceScene(transition);
        }
        else {
                EditLevelLayer::onBack(sender);
}
}
};