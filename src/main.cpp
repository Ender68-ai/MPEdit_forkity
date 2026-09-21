#include <Geode/Geode.hpp>
#include "SessionManager.hpp"
#include "P2PManager.hpp"
#include "RemoteActionHandler.hpp"
#include "ui/settings/AppearanceSetting.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("Multiplayer Edit v{} loaded!", Mod::get()->getVersion().toNonVString());

    (void)Mod::get()->registerCustomSettingType("appearance", &mpedit::AppearanceSettingV3::parse);
}

$on_mod(DataSaved) {
    auto& session = mpedit::SessionManager::get();
    if (session.isInSession()) {
        session.leaveSession();
    }
}

// scientists reveal the hi