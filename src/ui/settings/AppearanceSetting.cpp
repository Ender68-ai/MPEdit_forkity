#include "AppearanceSetting.hpp"
#include "../menu/AppearancePopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

namespace mpedit {

    bool AppearanceSettingNodeV3::init(std::shared_ptr<AppearanceSettingV3> setting, float width) {
        if (!SettingNodeV3::init(setting, width)) return false;

        auto spr = ButtonSprite::create("Customize...", "goldFont.fnt", "GJ_button_04.png", 0.6f);
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(AppearanceSettingNodeV3::onCustomize));
        this->getButtonMenu()->addChildAtPosition(btn, Anchor::Right, ccp(-5, 0), ccp(1.f, 0.5f));

        this->updateState(nullptr);
        return true;
    }

    void AppearanceSettingNodeV3::updateState(CCNode* invoker) {
        SettingNodeV3::updateState(invoker);
    }

    void AppearanceSettingNodeV3::onCommit() {}
    void AppearanceSettingNodeV3::onResetToDefault() {}
    bool AppearanceSettingNodeV3::hasUncommittedChanges() const { return false; }
    bool AppearanceSettingNodeV3::hasNonDefaultValue() const { return false; }

    void AppearanceSettingNodeV3::onCustomize(CCObject*) {
        AppearancePopup::create()->show();
    }

    AppearanceSettingNodeV3* AppearanceSettingNodeV3::create(std::shared_ptr<AppearanceSettingV3> setting, float width) {
        auto ret = new AppearanceSettingNodeV3();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    Result<std::shared_ptr<SettingV3>> AppearanceSettingV3::parse(std::string key, std::string modID, matjson::Value const& json) {
        auto ret = std::make_shared<AppearanceSettingV3>();
        auto root = checkJson(json, "AppearanceSettingV3");
        ret->parseBaseProperties(key, modID, root);
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(ret));
    }

    bool AppearanceSettingV3::load(matjson::Value const&) { return true; }
    bool AppearanceSettingV3::save(matjson::Value&) const { return true; }
    bool AppearanceSettingV3::isDefaultValue() const { return true; }
    void AppearanceSettingV3::reset() {}

    SettingNodeV3* AppearanceSettingV3::createNode(float width) {
        return AppearanceSettingNodeV3::create(
            std::static_pointer_cast<AppearanceSettingV3>(shared_from_this()), width
        );
    }

}
