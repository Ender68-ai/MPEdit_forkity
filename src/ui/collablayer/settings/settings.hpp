#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SettingsLayer : public CCLayer {
protected:
    bool init() override;
    void onBack(CCObject*);
    void onAppearance(CCObject*);
    void onDedicatedServers(CCObject*);
public:
    static SettingsLayer* create();
};