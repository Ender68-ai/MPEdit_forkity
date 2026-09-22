#pragma once

#include <Geode/Geode.hpp>

#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Log.hpp>

namespace UI {

    inline bool useGlobed() {
        return geode::Loader::get()->isModLoaded("dankmeme.globed2");
    }

}
using namespace cocos2d;
class Transition : public cocos2d::CCTransitionScene {
    public:
        static cocos2d::CCTransitionFade* create(float duration, cocos2d::CCScene* scene, cocos2d::ccColor3B const& color) {
            auto ret = new CCTransitionFade();
            if (ret && ret->initWithDuration(duration, scene)) {
                ret->autorelease();
                return ret;
            }
            delete ret;
            return nullptr;
        }
};

class TransitionBack : public cocos2d::CCTransitionScene {
    public:
        static cocos2d::CCTransitionFade* create(float duration, cocos2d::CCScene*, cocos2d::ccColor3B const& color) {
            auto ret = new CCTransitionFade();
            if (ret && ret->initWithDuration(duration, CCDirector::sharedDirector()->getRunningScene())) {
                ret->autorelease();
                return ret;
            }
            delete ret;
            return nullptr;
        }
};

extern bool fromCollab;
