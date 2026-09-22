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
#include "P2PManager.hpp"
#include "BinaryProtocol.hpp"
#include "MessageBatcher.hpp"
#include "ActionSerializer.hpp"
#include "RemoteActionHandler.hpp"
#include "RevertManager.hpp"
#include "ui/menu/MultiplayerMenuPopup.hpp"
#include "ui/menu/CreateRoomPopup.hpp"
#include "ui/QuickChatPopup.hpp"

#include "ui/SessionStatusNode.hpp"
#include "ui/CursorNode.hpp"
#include "ui/CursorOverlayNode.hpp"
#include "ui/UpdateHelperNode.hpp"

using namespace geode::prelude;
using namespace mpedit;
#include "EditorHookState.hpp"

using namespace geode::prelude;
using namespace mpedit;

namespace {
    void sendChunkedUpdateObjects(std::vector<ActionSerializer::ObjectData> const& updates) {
        RevertManager::get().onObjectsUpdated(SessionManager::get().getLocalPlayerId(), updates);
        constexpr size_t MAX_UPDATES_PER_MESSAGE = 100;
        for (size_t i = 0; i < updates.size(); i += MAX_UPDATES_PER_MESSAGE) {
            size_t count = std::min(MAX_UPDATES_PER_MESSAGE, updates.size() - i);
            std::vector<ActionSerializer::ObjectData> chunk(updates.begin() + i, updates.begin() + i + count);
            auto data = proto::serializeUpdateObjects(chunk);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
        }
    }

    void syncObjectProperties(cocos2d::CCArray* objects) {
        if (!objects || objects->count() == 0) return;
        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote()) return;

        auto* editor = LevelEditorLayer::get();
        if (!editor) return;

        auto& tracked = handler.getTrackedSelections();
        std::vector<ActionSerializer::ObjectData> updates;
        for (auto* obj : CCArrayExt<GameObject*>(objects)) {
            if (!obj) continue;
            auto uuid = handler.getUUIDForObject(obj);
            if (uuid.empty()) continue;

            auto tIt = tracked.find(obj);
            if (tIt != tracked.end()) {
                std::string currentSave = obj->getSaveString(editor);
                if (tIt->second != currentSave) {
                    updates.push_back(ActionSerializer::extractObjectData(obj, uuid));
                    tIt->second = currentSave;
                }
            }
        }

        if (!updates.empty()) {
            sendChunkedUpdateObjects(updates);
        }
    }
}

namespace {
    void forceSyncColorsToDict(GJEffectManager* effectMgr) {
        if (!effectMgr || !effectMgr->m_colorActionDict) return;
        for (size_t i = 0; i < effectMgr->m_colorActionVector.size(); i++) {
            auto* vecAction = effectMgr->m_colorActionVector[i];
            if (vecAction) {
                int channelID = vecAction->m_colorID;
                if (channelID == 0) channelID = static_cast<int>(i);
        auto* dictAction = static_cast<ColorAction*>(effectMgr->m_colorActionDict->objectForKey(channelID));
        if (!dictAction) {
            effectMgr->m_colorActionDict->setObject(vecAction, channelID);
        } else if (dictAction != vecAction) {
            dictAction->m_color = vecAction->m_color;
            dictAction->m_fromColor = vecAction->m_fromColor;
            dictAction->m_toColor = vecAction->m_toColor;
            dictAction->m_duration = vecAction->m_duration;
            dictAction->m_blending = vecAction->m_blending;
            dictAction->m_playerColor = vecAction->m_playerColor;
            dictAction->m_fromOpacity = vecAction->m_fromOpacity;
            dictAction->m_toOpacity = vecAction->m_toOpacity;
            dictAction->m_copyHSV = vecAction->m_copyHSV;
            dictAction->m_copyID = vecAction->m_copyID;
            dictAction->m_copyOpacity = vecAction->m_copyOpacity;
            dictAction->m_copyColorCalculated = vecAction->m_copyColorCalculated;
            dictAction->m_colorID = vecAction->m_colorID;
            dictAction->m_copyColorLoop = vecAction->m_copyColorLoop;
            dictAction->m_legacyHSV = vecAction->m_legacyHSV;
        }
    }
}
}
}

namespace mpedit {
ActionSerializer::ColorChannelData colorActionToData(ColorAction* action, int channelID) {
    ActionSerializer::ColorChannelData data;
    data.channelID = channelID;
    if (action) {
        data.color = action->m_color;
        data.fromColor = action->m_fromColor;
        data.toColor = action->m_toColor;
        data.duration = action->m_duration;
        data.blending = action->m_blending;
        data.playerColor = action->m_playerColor;
        data.fromOpacity = action->m_fromOpacity;
        data.toOpacity = action->m_toOpacity;
        data.copyHSV = action->m_copyHSV;
        data.copyID = action->m_copyID;
        data.copyOpacity = action->m_copyOpacity;
        data.copyColorCalculated = action->m_copyColorCalculated;
        data.colorID = action->m_colorID;
        data.copyColorLoop = action->m_copyColorLoop;
        data.legacyHSV = action->m_legacyHSV;
    }
    return data;
}
}


#include <Geode/modify/GJColorSetupLayer.hpp>
class $modify(MPGJColorSetupLayer, GJColorSetupLayer) {
    struct Fields {
        std::unordered_map<int, ActionSerializer::ColorChannelData> m_cachedColors;
    };

    bool init(LevelSettingsObject* p0) {
        if (!GJColorSetupLayer::init(p0)) return false;

        auto* effectMgr = m_settingsObject ? m_settingsObject->m_effectManager : nullptr;
        if (!effectMgr) {
            auto editor = LevelEditorLayer::get();
            if (editor && editor->m_levelSettings) {
                effectMgr = editor->m_levelSettings->m_effectManager;
            }
        }

        if (effectMgr) {
            for (size_t i = 0; i < effectMgr->m_colorActionVector.size(); i++) {
                if (auto* action = effectMgr->m_colorActionVector[i]) {
                    int channelID = action->m_colorID;
                    if (channelID == 0) channelID = static_cast<int>(i);
                    m_fields->m_cachedColors[channelID] = colorActionToData(action, channelID);
                }
            }
        }

        return true;
    }

    void syncColors() {
        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;

        auto& session = SessionManager::get();
        if (!session.isInSession()) return;
        
        auto editor = LevelEditorLayer::get();
        auto* effectMgr = m_settingsObject ? m_settingsObject->m_effectManager : nullptr;
        if (!effectMgr && editor && editor->m_levelSettings) {
            effectMgr = editor->m_levelSettings->m_effectManager;
        }

        if (effectMgr) {
            forceSyncColorsToDict(effectMgr);

            for (size_t i = 0; i < effectMgr->m_colorActionVector.size(); i++) {
                if (auto* action = effectMgr->m_colorActionVector[i]) {
                    int channelID = action->m_colorID;
                    if (channelID == 0) channelID = static_cast<int>(i);
                    auto currentData = colorActionToData(action, channelID);
                    
                    if (m_fields->m_cachedColors.find(channelID) == m_fields->m_cachedColors.end() || m_fields->m_cachedColors[channelID] != currentData) {
                        m_fields->m_cachedColors[channelID] = currentData;
                        RevertManager::get().onColorChannelUpdated(session.getLocalPlayerId(), currentData);
                        auto packet = proto::serializeUpdateColorChannel(currentData);
                        P2PManager::get().send(std::move(packet), ChannelType::Reliable);
                        log::info("Broadcasting granular UpdateColorChannel for channel {}", channelID);
                    }
                }
            }
        }
    }

    void onClose(cocos2d::CCObject* sender) {
        GJColorSetupLayer::onClose(sender);
        syncColors();
    }

    void keyBackClicked() {
        GJColorSetupLayer::keyBackClicked();
        syncColors();
    }
};



#include <Geode/modify/LevelSettingsLayer.hpp>
class $modify(MPLevelSettingsLayer, LevelSettingsLayer) {
    struct Fields {
        std::map<int, ActionSerializer::ColorChannelData> m_cachedColors;
    };

    bool init(LevelSettingsObject* object, LevelEditorLayer* layer) {
        if (!LevelSettingsLayer::init(object, layer)) return false;

        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_levelSettings && editor->m_levelSettings->m_effectManager) {
            auto effectMgr = editor->m_levelSettings->m_effectManager;
            for (size_t i = 0; i < effectMgr->m_colorActionVector.size(); i++) {
                if (auto* action = effectMgr->m_colorActionVector[i]) {
                    int channelID = action->m_colorID;
                    if (channelID == 0) channelID = static_cast<int>(i);
                    m_fields->m_cachedColors[channelID] = colorActionToData(action, channelID);
                }
            }
        }
        return true;
    }

    void syncColors() {
        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;
        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_levelSettings && editor->m_levelSettings->m_effectManager) {
            auto effectMgr = editor->m_levelSettings->m_effectManager;
            forceSyncColorsToDict(effectMgr);

            for (size_t i = 0; i < effectMgr->m_colorActionVector.size(); i++) {
                if (auto* action = effectMgr->m_colorActionVector[i]) {
                    int channelID = action->m_colorID;
                    if (channelID == 0) channelID = static_cast<int>(i);
                    auto currentData = colorActionToData(action, channelID);
                    
                    if (m_fields->m_cachedColors.find(channelID) == m_fields->m_cachedColors.end() || m_fields->m_cachedColors[channelID] != currentData) {
                        m_fields->m_cachedColors[channelID] = currentData;
                        RevertManager::get().onColorChannelUpdated(session.getLocalPlayerId(), currentData);
                        auto packet = proto::serializeUpdateColorChannel(currentData);
                        P2PManager::get().send(std::move(packet), ChannelType::Reliable);
                        log::info("Broadcasting UpdateColorChannel for channel {} from LevelSettingsLayer", channelID);
                    }
                }
            }
        }
    }


    void onClose(cocos2d::CCObject* sender) {
        LevelSettingsLayer::onClose(sender);

        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;

        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_levelSettings) {
            syncColors();
            ActionSerializer::LevelSettingsData settings;
            settings.saveString = editor->m_levelSettings->getSaveString();
            settings.audioTrack = editor->m_level->m_audioTrack;
            settings.songID = editor->m_level->m_songID;
            settings.levelLength = editor->m_level->m_levelLength;
            
            RevertManager::get().onSettingsUpdated(session.getLocalPlayerId(), settings);
            auto packet = proto::serializeUpdateSettings(settings);
            P2PManager::get().send(std::move(packet), ChannelType::Reliable);
        }
    }

    void keyBackClicked() {
        LevelSettingsLayer::keyBackClicked();

        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;

        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        auto editor = LevelEditorLayer::get();
        if (editor && editor->m_levelSettings) {
            syncColors();
            ActionSerializer::LevelSettingsData settings;
            settings.saveString = editor->m_levelSettings->getSaveString();
            settings.audioTrack = editor->m_level->m_audioTrack;
            settings.songID = editor->m_level->m_songID;
            settings.levelLength = editor->m_level->m_levelLength;
            
            RevertManager::get().onSettingsUpdated(session.getLocalPlayerId(), settings);
            auto packet = proto::serializeUpdateSettings(settings);
            P2PManager::get().send(std::move(packet), ChannelType::Reliable);
        }
    }
};

#include <Geode/modify/CustomizeObjectLayer.hpp>
class $modify(MPCustomizeObjectLayer, CustomizeObjectLayer) {
    void syncSelected() {
        if (auto editor = LevelEditorLayer::get()) {
            if (auto ui = editor->m_editorUI) {
                syncObjectProperties(ui->m_selectedObjects);
            }
        }
    }

    void onClose(cocos2d::CCObject* sender) {
        CustomizeObjectLayer::onClose(sender);
        syncSelected();
    }
    void keyBackClicked() {
        CustomizeObjectLayer::keyBackClicked();
        syncSelected();
    }
};

#include <Geode/modify/SetGroupIDLayer.hpp>
class $modify(MPSetGroupIDLayer, SetGroupIDLayer) {
    void onClose(cocos2d::CCObject* sender) {
        SetGroupIDLayer::onClose(sender);
        if (auto editor = LevelEditorLayer::get()) {
            if (auto ui = editor->m_editorUI) {
                syncObjectProperties(ui->m_selectedObjects);
            }
        }
    }
    void keyBackClicked() {
        SetGroupIDLayer::keyBackClicked();
        if (auto editor = LevelEditorLayer::get()) {
            if (auto ui = editor->m_editorUI) {
                syncObjectProperties(ui->m_selectedObjects);
            }
        }
    }
};

#include <Geode/modify/SetupTriggerPopup.hpp>
class $modify(MPSetupTriggerPopup, SetupTriggerPopup) {
    void onClose(cocos2d::CCObject* sender) {
        SetupTriggerPopup::onClose(sender);
        if (auto editor = LevelEditorLayer::get()) {
            if (auto ui = editor->m_editorUI) {
                syncObjectProperties(ui->m_selectedObjects);
            }
        }
    }
    void keyBackClicked() {
        SetupTriggerPopup::keyBackClicked();
        if (auto editor = LevelEditorLayer::get()) {
            if (auto ui = editor->m_editorUI) {
                syncObjectProperties(ui->m_selectedObjects);
            }
        }
    }
};

#include <Geode/modify/ColorSelectPopup.hpp>
class $modify(MPColorSelectPopup, ColorSelectPopup) {
    void closeColorSelect(cocos2d::CCObject* sender) {
        ColorSelectPopup::closeColorSelect(sender);
        syncColor();
    }

    void keyBackClicked() {
        ColorSelectPopup::keyBackClicked();
        syncColor();
    }

    void syncColor() {
        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;

        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        if (m_colorAction) {
            int channelID = m_colorAction->m_colorID;
            auto data = colorActionToData(m_colorAction, channelID);
            RevertManager::get().onColorChannelUpdated(session.getLocalPlayerId(), data);
            auto packet = proto::serializeUpdateColorChannel(data);
            P2PManager::get().send(std::move(packet), ChannelType::Reliable);
            log::info("Broadcasting granular UpdateColorChannel for channel {} from ColorSelectPopup", channelID);
        }
    }
};

#include <Geode/modify/ColorSelectLiveOverlay.hpp>
class $modify(MPColorSelectLiveOverlay, ColorSelectLiveOverlay) {
    void closeColorSelect(cocos2d::CCObject* sender) {
        ColorSelectLiveOverlay::closeColorSelect(sender);
        syncColor();
    }

    void keyBackClicked() {
        ColorSelectLiveOverlay::keyBackClicked();
        syncColor();
    }

    void syncColor() {
        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;

        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        if (m_baseColorAction) {
            int channelID = m_baseColorAction->m_colorID;
            auto data = colorActionToData(m_baseColorAction, channelID);
            RevertManager::get().onColorChannelUpdated(session.getLocalPlayerId(), data);
            auto packet = proto::serializeUpdateColorChannel(data);
            P2PManager::get().send(std::move(packet), ChannelType::Reliable);
        }
        
        if (m_detailColorAction) {
            int channelID = m_detailColorAction->m_colorID;
            auto data = colorActionToData(m_detailColorAction, channelID);
            RevertManager::get().onColorChannelUpdated(session.getLocalPlayerId(), data);
            auto packet = proto::serializeUpdateColorChannel(data);
            P2PManager::get().send(std::move(packet), ChannelType::Reliable);
        }
    }
};

