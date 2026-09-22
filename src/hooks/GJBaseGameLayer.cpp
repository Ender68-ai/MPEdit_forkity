#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/binding/TeleportPortalObject.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/loader/Dirs.hpp>

#include "SessionManager.hpp"
#include "ActionSerializer.hpp"
#include "RemoteActionHandler.hpp"

#include "EditorHookState.hpp"

using namespace geode::prelude;
using namespace mpedit;


using namespace geode::prelude;
using namespace mpedit;

class $modify(MPBaseGameLayer, GJBaseGameLayer) {
    void addToSection(GameObject* obj) {
        GJBaseGameLayer::addToSection(obj);

        if (obj && obj->m_objectID == 31) {
            auto* editor = LevelEditorLayer::get();
            if (editor && static_cast<GJBaseGameLayer*>(editor) == this) {
                hookstate::s_startPosObjects.insert(obj);
                hookstate::s_startPosSaveStrings[obj] = obj->getSaveString(editor);
            }
        }

        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        if (!session.isInSession() || handler.isProcessingRemote() || !obj) {
            return;
        }

        if (!handler.isInitialSyncCompleted()) {
            return;
        }

        auto* editor = LevelEditorLayer::get();
        if (!editor || static_cast<GJBaseGameLayer*>(editor) != this) {
            return;
        }

        
        if (!session.isInSession()) {
            return;
        }

        if (!handler.getUUIDForObject(obj).empty()) {
            return;
        }

        if (auto* tpPortal = typeinfo_cast<TeleportPortalObject*>(obj)) {
            if (tpPortal->m_isYellowPortal) {
                return;
            }
        }

        auto uuid = RemoteActionHandler::generateUUID();
        handler.registerObject(uuid, obj);
        handler.queueObjectForPlacement(uuid, obj);
    }
};

