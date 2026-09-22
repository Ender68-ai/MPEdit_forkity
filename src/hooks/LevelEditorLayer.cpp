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
    void sendChunkedLockObjects(std::vector<std::string> const& uuids, bool locked) {
        constexpr size_t MAX_UUIDS_PER_MESSAGE = 300;
        for (size_t i = 0; i < uuids.size(); i += MAX_UUIDS_PER_MESSAGE) {
            size_t count = std::min(MAX_UUIDS_PER_MESSAGE, uuids.size() - i);
            std::vector<std::string> chunk(uuids.begin() + i, uuids.begin() + i + count);
            auto data = proto::serializeLockObjects(chunk, locked);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
        }
    }

    void sendChunkedDeleteObjects(std::vector<std::string> const& uuids) {
        RevertManager::get().onObjectsDeleted(SessionManager::get().getLocalPlayerId(), uuids);
        constexpr size_t MAX_UUIDS_PER_MESSAGE = 300;
        for (size_t i = 0; i < uuids.size(); i += MAX_UUIDS_PER_MESSAGE) {
            size_t count = std::min(MAX_UUIDS_PER_MESSAGE, uuids.size() - i);
            std::vector<std::string> chunk(uuids.begin() + i, uuids.begin() + i + count);
            auto data = proto::serializeDeleteObjects(chunk);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
        }
    }

    void sendChunkedMoveObjects(std::vector<ActionSerializer::MoveData> const& moves) {
        RevertManager::get().onObjectsMoved(SessionManager::get().getLocalPlayerId(), moves);
        constexpr size_t MAX_MOVES_PER_MESSAGE = 300;
        for (size_t i = 0; i < moves.size(); i += MAX_MOVES_PER_MESSAGE) {
            size_t count = std::min(MAX_MOVES_PER_MESSAGE, moves.size() - i);
            std::vector<ActionSerializer::MoveData> chunk(moves.begin() + i, moves.begin() + i + count);
            auto data = proto::serializeMoveObjects(chunk);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
        }
    }

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

    void sendChunkedReconcileObjects(std::vector<ActionSerializer::ReconcileData> const& reconciles) {
        RevertManager::get().onObjectsReconciled(SessionManager::get().getLocalPlayerId(), reconciles);
        constexpr size_t MAX_RECONCILES_PER_MESSAGE = 1000;
        for (size_t i = 0; i < reconciles.size(); i += MAX_RECONCILES_PER_MESSAGE) {
            size_t count = std::min(MAX_RECONCILES_PER_MESSAGE, reconciles.size() - i);
            std::vector<ActionSerializer::ReconcileData> chunk(reconciles.begin() + i, reconciles.begin() + i + count);
            auto data = proto::serializeReconcileObjects(chunk);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
        }
    }

    void sendChunkedSync(LevelEditorLayer* editor, int targetPlayerId, std::function<void()> onComplete = nullptr) {
        auto& handler = RemoteActionHandler::get();

        std::string fullObjectsString;
        std::vector<std::string> allUuids;

        if (editor->m_objects) {
            int index = 0;
            for (auto* obj : CCArrayExt<GameObject*>(editor->m_objects)) {
                if (!obj) continue;
                auto uuid = handler.getUUIDForObject(obj);
                if (uuid.empty()) {
                    uuid = RemoteActionHandler::generateUUID();
                    handler.registerObject(uuid, obj);
                }
                allUuids.push_back(uuid);
                fullObjectsString += std::string(obj->getSaveString(editor)) + ";";
                index++;
            }
        }

        std::string compressedBytes = "";
        if (!fullObjectsString.empty()) {
            auto tempPath = geode::dirs::getTempDir() / "sync_level.zip";
            {
                if (auto zipRes = geode::utils::file::Zip::create(tempPath)) {
                    (void)zipRes.unwrap().add("level.txt", fullObjectsString);
                } else {
                    log::error("Failed to create temp zip for sync payload");
                }
            }

            if (auto dataRes = geode::utils::file::readBinary(tempPath)) {
                auto bytes = dataRes.unwrap();
                compressedBytes = std::string(bytes.begin(), bytes.end());
            } else {
                log::error("Failed to read compressed sync payload from temp file");
            }

            std::error_code ec;
            std::filesystem::remove(tempPath, ec);
        }

        constexpr size_t MAX_CHUNK_BYTES = 10000;
        constexpr size_t MAX_UUIDS_PER_CHUNK = 150;

        struct ChunkData {
            std::string objectsString;
            std::vector<std::string> uuids;
        };
        std::vector<ChunkData> chunks;

        size_t byteOffset = 0;
        size_t uuidOffset = 0;

        while (byteOffset < compressedBytes.size() || uuidOffset < allUuids.size()) {
            ChunkData chunk;
            
            size_t bytesToTake = std::min(MAX_CHUNK_BYTES, compressedBytes.size() - byteOffset);
            if (bytesToTake > 0) {
                chunk.objectsString = compressedBytes.substr(byteOffset, bytesToTake);
                byteOffset += bytesToTake;
            }

            size_t uuidsToTake = std::min(MAX_UUIDS_PER_CHUNK, allUuids.size() - uuidOffset);
            if (uuidsToTake > 0) {
                chunk.uuids.insert(chunk.uuids.end(), allUuids.begin() + uuidOffset, allUuids.begin() + uuidOffset + uuidsToTake);
                uuidOffset += uuidsToTake;
            }

            chunks.push_back(std::move(chunk));
        }

        if (chunks.empty()) {
            chunks.push_back(ChunkData());
        }

        ActionSerializer::LevelSettingsData settings;
        if (editor->m_levelSettings) {
            settings.saveString = editor->m_levelSettings->getSaveString();
        }
        if (editor->m_level) {
            settings.audioTrack = editor->m_level->m_audioTrack;
            settings.songID = editor->m_level->m_songID;
            settings.levelLength = editor->m_level->m_levelLength;
            settings.levelName = editor->m_level->m_levelName;
        }

        uint32_t totalChunks = static_cast<uint32_t>(chunks.size());
        uint32_t totalObjects = static_cast<uint32_t>(allUuids.size());

        auto startMsg = proto::serializeSyncLevelStart(totalChunks, totalObjects, settings);
        P2PManager::get().sendTo(targetPlayerId, startMsg, ChannelType::Reliable);

        std::vector<ActionSerializer::LockData> locks;
        for (auto const& [uuid, lockInfo] : handler.getObjectLocks()) {
            locks.push_back({uuid, lockInfo.playerId, lockInfo.timeLeft});
        }

        auto serializedChunks = std::make_shared<std::vector<std::vector<uint8_t>>>();
        for (uint32_t i = 0; i < totalChunks; ++i) {
            serializedChunks->push_back(
                proto::serializeSyncLevelChunk(
                    i,
                    reinterpret_cast<const uint8_t*>(chunks[i].objectsString.data()),
                    chunks[i].objectsString.size(),
                    chunks[i].uuids
                )
            );
        }

        auto nextChunkIndex = std::make_shared<uint32_t>(0);
        auto sharedLocks = std::make_shared<std::vector<ActionSerializer::LockData>>(std::move(locks));

        auto* senderNode = UpdateHelperNode::create(
            [targetPlayerId, totalChunks, serializedChunks, nextChunkIndex, sharedLocks, onComplete](float dt) {
                auto& net = P2PManager::get();

                constexpr size_t BUFFER_THRESHOLD = 256 * 1024;
                if (net.getReliableBufferedAmount(targetPlayerId) > BUFFER_THRESHOLD) {
                    return;
                }

                size_t lockChunkCount = (sharedLocks->size() + 999) / 1000;
                if (lockChunkCount == 0) lockChunkCount = 1;
                if (*nextChunkIndex < totalChunks) {
                    net.sendTo(targetPlayerId, (*serializedChunks)[*nextChunkIndex], ChannelType::Reliable);
                    (*nextChunkIndex)++;
                } else if (*nextChunkIndex < totalChunks + lockChunkCount) {
                    if (!sharedLocks->empty()) {
                        size_t lockChunkIdx = *nextChunkIndex - totalChunks;
                        size_t startIdx = lockChunkIdx * 1000;
                        size_t count = std::min((size_t)1000, sharedLocks->size() - startIdx);
                        std::vector<ActionSerializer::LockData> chunk(sharedLocks->begin() + startIdx, sharedLocks->begin() + startIdx + count);
                        net.sendTo(targetPlayerId, proto::serializeSyncLocksChunk(chunk), ChannelType::Reliable);
                    }
                    (*nextChunkIndex)++;
                } else {
                    auto endMsg = proto::serializeSyncLevelEnd();
                    net.sendTo(targetPlayerId, endMsg, ChannelType::Reliable);

                    if (auto* notifNode = cocos2d::CCDirector::sharedDirector()->getNotificationNode()) {
                        if (auto* node = notifNode->getChildByTag(9991 + targetPlayerId)) {
                            node->removeFromParentAndCleanup(true);
                        }
                    }
                    if (onComplete) onComplete();
                }
            },
            0.01f
        );
        senderNode->setTag(9991 + targetPlayerId);
        
        if (auto* notifNode = cocos2d::CCDirector::sharedDirector()->getNotificationNode()) {
            notifNode->addChild(senderNode);
        } else {
            editor->addChild(senderNode);
        }
    }

    void registerObjectsWithUuids(LevelEditorLayer* editor,
                                  std::vector<std::string> const& uuids) {
        if (!editor || !editor->m_objects) return;
        auto& handler = RemoteActionHandler::get();
        int index = 0;
        for (auto* obj : CCArrayExt<GameObject*>(editor->m_objects)) {
            if (!obj) continue;
            if (index < static_cast<int>(uuids.size()) && !uuids[index].empty()) {
                handler.registerObject(uuids[index], obj);
            } else {
                if (handler.getUUIDForObject(obj).empty()) {
                    handler.registerObject(RemoteActionHandler::generateUUID(), obj);
                }
            }
            index++;
        }
        if (index != static_cast<int>(uuids.size())) {
            log::warn("EditorHooks: object/uuid count mismatch on sync "
                      "(objects={}, uuids={})", index, uuids.size());
        }
    }
}


class $modify(MPLevelEditorLayer, LevelEditorLayer) {
    struct Fields {
        float m_cursorSendTimer = 0.f;
        bool m_sessionActive = false;
        bool m_inUndoRedo = false;
        bool m_initializing = true;
        bool m_settingsBroadcastInProgress = false;
        cocos2d::CCPoint m_lastSentLevelPos = {0.f, 0.f};
        bool m_wasPlaytesting = false;

        ~Fields() {
            auto& session = SessionManager::get();
            if (session.isInSession()) {
                session.leaveSession();
                log::info("EditorHooks: Left session automatically on editor destructor (Fields)");
            }
            session.clearCallbacks();
        }
    };

    void levelSettingsUpdated() {
        LevelEditorLayer::levelSettingsUpdated();

        if (m_fields->m_initializing) return;
        if (m_fields->m_settingsBroadcastInProgress) return;

        auto& handler = RemoteActionHandler::get();
        if (handler.isProcessingRemote() || !handler.isInitialSyncCompleted()) return;

        auto& session = SessionManager::get();
        if (session.isInSession()) {
            m_fields->m_settingsBroadcastInProgress = true;
            ActionSerializer::LevelSettingsData settings;
            if (this->m_levelSettings) {
                settings.saveString = this->m_levelSettings->getSaveString();
            }
            if (this->m_level) {
                settings.audioTrack = this->m_level->m_audioTrack;
                settings.songID = this->m_level->m_songID;
                settings.levelLength = this->m_level->m_levelLength;
            }
            RevertManager::get().onSettingsUpdated(session.getLocalPlayerId(), settings);
            auto data = proto::serializeUpdateSettings(settings);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
            m_fields->m_settingsBroadcastInProgress = false;
            log::info("EditorHooks: Broadcasted update_settings");
        }
    }

    bool init(GJGameLevel* level, bool unk) {
        if (!LevelEditorLayer::init(level, unk)) {
            m_fields->m_initializing = false;
            return false;
        }

        hookstate::s_startPosObjects.clear();
        hookstate::s_startPosSaveStrings.clear();
        if (this->m_objects) {
            for (auto* obj : CCArrayExt<GameObject*>(this->m_objects)) {
                if (obj->m_objectID == 31) {
                    hookstate::s_startPosObjects.insert(obj);
                    hookstate::s_startPosSaveStrings[obj] = obj->getSaveString(this);
                }
            }
        }

        m_fields->m_sessionActive = SessionManager::get().isInSession();

        auto& handler = RemoteActionHandler::get();
        handler.clearMappings();
        RevertManager::get().captureBaseline();

        SessionManager::get().onSessionStarted(this, [this]() {
            auto& session = SessionManager::get();
            if (this->m_objects) {
                auto& handler = RemoteActionHandler::get();
                int index = 0;
                for (auto* obj : CCArrayExt<GameObject*>(this->m_objects)) {
                    if (obj && handler.getUUIDForObject(obj).empty()) {
                        handler.registerObject(RemoteActionHandler::generateUUID(), obj);
                    }
                    index++;
                }
            }
        });

        auto& session = SessionManager::get();
        if (session.isInSession()) {
            bool hasPending = handler.hasPendingSync();

            if (!hasPending) {
                auto const& expected = handler.getExpectedUuids();
                if (!expected.empty()) {
                    if (this->m_objects) {
                        registerObjectsWithUuids(this, expected);
                    }
                    handler.clearExpectedUuids();
                } else if (this->m_objects) {
                    int index = 0;
                    for (auto* obj : CCArrayExt<GameObject*>(this->m_objects)) {
                        if (obj && handler.getUUIDForObject(obj).empty()) {
                            handler.registerObject(RemoteActionHandler::generateUUID(), obj);
                        }
                        index++;
                    }
                }
            } else {
                handler.clearExpectedUuids();
            }

            handler.setInitialSyncCompleted(true);

            if (session.getRole() == SessionManager::Role::Host) {
                for (auto const& player : session.getPlayers()) {
                    if (player.id != session.getLocalPlayerId()) {
                        sendChunkedSync(this, player.id);
                        log::info("EditorHooks: Sent chunked sync_level to existing player {}", player.id);
                    }
                }
            }
        }

        SessionManager::get().onPlayerJoined(this, [this](PlayerInfo const& info) {
            auto& session = SessionManager::get();
            if (session.getRole() == SessionManager::Role::Host && info.id != session.getLocalPlayerId()) {
                sendChunkedSync(this, info.id);
                log::info("EditorHooks: Sent chunked sync_level to new player {}", info.id);
            }
        });

        if (handler.hasPendingSync()) {
            handler.setEditorForInit(this);
            handler.applyPendingSync();
            handler.setEditorForInit(nullptr);
        }

        auto* helper = UpdateHelperNode::create([this](float dt) {
            this->networkUpdate(dt);
        }, 0.05f);
        if (helper) {
            helper->setID("network-update-helper"_spr);
            this->addChild(helper);
        }

        auto* status = SessionStatusNode::create();
        status->setID("session-status"_spr);
        this->addChild(status, 1000);

        auto* cursorOverlay = CursorOverlayNode::create();
        cursorOverlay->setID("cursor-overlay-node"_spr);
        int uiZ = this->m_editorUI ? this->m_editorUI->getZOrder() : 100;
        this->addChild(cursorOverlay, uiZ > 2 ? uiZ - 1 : 10);

        auto* cursorNode = CursorNode::create();
        cursorNode->setID("cursor-node"_spr);
        this->m_objectLayer->addChild(cursorNode, 999);

        m_fields->m_initializing = false;
        return true;
    }

    void onExit() {
        LevelEditorLayer::onExit();
        
        auto& session = SessionManager::get();
        if (session.isInSession()) {
            session.leaveSession();
            log::info("EditorHooks: Left session automatically on editor exit");
        }
        session.removeListener(this);
        
        hookstate::s_startPosObjects.clear();
        hookstate::s_startPosSaveStrings.clear();
    }


    GameObject* createObject(int objectID, cocos2d::CCPoint position, bool noUndo) {
        auto* obj = LevelEditorLayer::createObject(objectID, position, noUndo);
        return obj;
    }

    void removeObject(GameObject* obj, bool undo) {
        if (!obj) {
            LevelEditorLayer::removeObject(obj, undo);
            return;
        }

        if (obj->m_objectID == 31) {
            hookstate::s_startPosObjects.erase(obj);
            hookstate::s_startPosSaveStrings.erase(obj);
        }

        obj->retain();

        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

            if (m_gameState.m_lastActivatedPortal1 == obj) {
                m_gameState.m_lastActivatedPortal1 = nullptr;
            }
            if (m_gameState.m_lastActivatedPortal2 == obj) {
                m_gameState.m_lastActivatedPortal2 = nullptr;
            }
            if (this->m_player1) {
                if (this->m_player1->m_lastActivatedPortal == obj) {
                    this->m_player1->m_lastActivatedPortal = nullptr;
                }
                if (this->m_player1->m_touchingRings && this->m_player1->m_touchingRings->containsObject(obj)) {
                    this->m_player1->m_touchingRings->removeObject(obj);
                }
            }
            if (this->m_player2) {
                if (this->m_player2->m_lastActivatedPortal == obj) {
                    this->m_player2->m_lastActivatedPortal = nullptr;
                }
                if (this->m_player2->m_touchingRings && this->m_player2->m_touchingRings->containsObject(obj)) {
                    this->m_player2->m_touchingRings->removeObject(obj);
                }
            }
            if (this->m_endPortal == obj) {
                this->m_endPortal = nullptr;
            }
            if (this->m_player1CollisionBlock == obj) {
                this->m_player1CollisionBlock = nullptr;
            }
            if (this->m_player2CollisionBlock == obj) {
                this->m_player2CollisionBlock = nullptr;
            }
            if (this->m_startPosObject == obj) {
                this->m_startPosObject = nullptr;
            }
            if (this->m_copyStateObject == obj) {
                this->m_copyStateObject = nullptr;
            }
            if (this->m_editorUI) {
                if (this->m_editorUI->m_selectedObject == obj) {
                    this->m_editorUI->m_selectedObject = nullptr;
                }
                if (this->m_editorUI->m_snapObject == obj) {
                    this->m_editorUI->m_snapObject = nullptr;
                }
                if (this->m_editorUI->m_selectedObjects && this->m_editorUI->m_selectedObjects->containsObject(obj)) {
                    this->m_editorUI->m_selectedObjects->removeObject(obj);
                }
            }

        bool inUndoRedo = m_fields->m_inUndoRedo;
        bool shouldBroadcastDelete = session.isInSession()
            && !handler.isProcessingRemote() && !inUndoRedo && !session.isLocalPlayerViewOnly() && obj;

        if (shouldBroadcastDelete) {
            auto uuid = handler.getUUIDForObject(obj);
            if (!uuid.empty()) {
                auto const& locks = handler.getObjectLocks();
                auto it = locks.find(uuid);
                if (it != locks.end() && it->second.playerId != session.getLocalPlayerId()) {
                    log::info("EditorHooks: Blocked removal of locked object (uuid={})", uuid);
                    obj->release();
                    return;
                }
                std::vector<std::string> uuids = {uuid};

                sendChunkedDeleteObjects(uuids);
                handler.unregisterObject(uuid);
                log::debug("EditorHooks: Deleted object(s) (uuid={})", uuid);
            }
        }

        if (obj) {
            auto uuid = handler.getUUIDForObject(obj);
            if (!uuid.empty()) {
                handler.unregisterObject(uuid);
            }
            handler.getTrackedSelections().erase(obj);
        }

        LevelEditorLayer::removeObject(obj, undo);

        obj->release();
    }

    void handleAction(bool undo, cocos2d::CCArray* undoObjects) {
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        if (session.isLocalPlayerViewOnly() && !handler.isProcessingRemote()) {
            LevelEditorLayer::handleAction(undo, undoObjects);
            return;
        }

        if (!session.isInSession() || handler.isProcessingRemote() || !undoObjects || undoObjects->count() == 0) {
            LevelEditorLayer::handleAction(undo, undoObjects);
            return;
        }

        std::unordered_set<GameObject*> affectedObjects;
        auto* lastItem = static_cast<UndoObject*>(undoObjects->lastObject());
        if (lastItem) {
            if (lastItem->m_objects) {
                for (auto* innerObj : geode::cocos::CCArrayExt<cocos2d::CCObject*>(lastItem->m_objects)) {
                    if (auto* gObj = geode::cast::typeinfo_cast<GameObject*>(innerObj)) {
                        affectedObjects.insert(gObj);
                    } else if (auto* copy = geode::cast::typeinfo_cast<GameObjectCopy*>(innerObj)) {
                        if (copy->m_object) {
                            affectedObjects.insert(copy->m_object);
                        }
                    }
                }
            }
            if (lastItem->m_objectCopy && lastItem->m_objectCopy->m_object) {
                affectedObjects.insert(lastItem->m_objectCopy->m_object);
            }
        }

        for (auto* gObj : affectedObjects) {
            if (!gObj) continue;
            auto uuid = handler.getUUIDForObject(gObj);
            if (!uuid.empty()) {
                auto const& locks = handler.getObjectLocks();
                auto it = locks.find(uuid);
                if (it != locks.end() && it->second.playerId != session.getLocalPlayerId()) {
                    log::info("EditorHooks: Blocked undo/redo of locked object");
                    return;
                }
            }
        }

        std::unordered_map<GameObject*, cocos2d::CCPoint> positionsBefore;
        std::unordered_map<GameObject*, std::string> saveStringsBefore;
        std::unordered_set<GameObject*> existedBefore;

        for (auto* obj : affectedObjects) {
            if (!obj) continue;
            if (this->m_objects && this->m_objects->containsObject(obj)) {
                existedBefore.insert(obj);
                positionsBefore[obj] = obj->getPosition();
                saveStringsBefore[obj] = obj->getSaveString(this);
            }
        }

        m_fields->m_inUndoRedo = true;
        LevelEditorLayer::handleAction(undo, undoObjects);

        std::vector<ActionSerializer::ObjectData> placedObjects;
        std::vector<std::string> deletedUuids;
        std::vector<ActionSerializer::MoveData> movedObjects;
        std::vector<ActionSerializer::ObjectData> updatedObjects;

        for (auto* obj : affectedObjects) {
            if (!obj) continue;
            
            bool existed_before = existedBefore.find(obj) != existedBefore.end();
            bool existed_after = this->m_objects && this->m_objects->containsObject(obj);
            
            if (existed_before && !existed_after) {
                std::string uuid = handler.getUUIDForObject(obj);
                if (!uuid.empty()) {
                    deletedUuids.push_back(uuid);
                    handler.unregisterObject(uuid);
                }
            } 
            else if (!existed_before && existed_after) {
                std::string uuid = handler.getUUIDForObject(obj);
                if (uuid.empty()) {
                    uuid = RemoteActionHandler::generateUUID();
                    handler.registerObject(uuid, obj);
                }
                placedObjects.push_back(ActionSerializer::extractObjectData(obj, uuid));
            } 
            else if (existed_before && existed_after) {
                std::string uuid = handler.getUUIDForObject(obj);
                if (uuid.empty()) {
                    uuid = RemoteActionHandler::generateUUID();
                    handler.registerObject(uuid, obj);
                }
                
                std::string currentSave = obj->getSaveString(this);
                if (saveStringsBefore[obj] != currentSave) {
                    updatedObjects.push_back(ActionSerializer::extractObjectData(obj, uuid));
                } else {
                    cocos2d::CCPoint oldPos = positionsBefore[obj];
                    float dx = obj->getPositionX() - oldPos.x;
                    float dy = obj->getPositionY() - oldPos.y;
                    if (dx != 0.f || dy != 0.f) {
                        ActionSerializer::MoveData md;
                        md.uuid = uuid;
                        md.dx = dx;
                        md.dy = dy;
                        movedObjects.push_back(md);
                    }
                }
            }
        }
        if (!placedObjects.empty()) {
            RevertManager::get().onObjectsPlaced(SessionManager::get().getLocalPlayerId(), placedObjects);
            auto data = proto::serializePlaceObjects(placedObjects);
            P2PManager::get().send(std::move(data), ChannelType::Reliable);
            log::info("EditorHooks: Synced redo placement of {} objects", placedObjects.size());
        }
        if (!deletedUuids.empty()) {
            sendChunkedDeleteObjects(deletedUuids);
            log::info("EditorHooks: Synced undo deletion of {} objects", deletedUuids.size());
        }
        if (!movedObjects.empty()) {
            sendChunkedMoveObjects(movedObjects);
        }
        if (!updatedObjects.empty()) {
            sendChunkedUpdateObjects(updatedObjects);
        }
        
        m_fields->m_inUndoRedo = false;
    }

    void networkUpdate(float dt) {
        auto& session = SessionManager::get();
        if (!session.isInSession()) return;

        auto& handler = RemoteActionHandler::get();
        bool isPlaytesting = this->m_playbackMode != PlaybackMode::Not;
        
        if (isPlaytesting && !m_fields->m_wasPlaytesting) {
            MessageBatcher::get().flush();
            handler.flushPendingPlacements();
            if (auto* ui = this->m_editorUI) {
                ui->deselectAll();
            }
        } else if (!isPlaytesting && m_fields->m_wasPlaytesting) {
            handler.flushDeferredDeletions();
        }
        m_fields->m_wasPlaytesting = isPlaytesting;

        P2PManager::get().dispatchMessages();

        handler.updateLocks(dt);
        MessageBatcher::get().update(dt);

        handler.flushPendingPlacements();

        m_fields->m_cursorSendTimer += dt;
        if (m_fields->m_cursorSendTimer >= 0.033f) {
            m_fields->m_cursorSendTimer = 0.f;
            
            if (this->m_objectLayer) {
                cocos2d::CCPoint levelPos;
                std::string statusStr = "";

                if (this->m_playbackMode != PlaybackMode::Not && this->m_player1) {
                    levelPos = this->m_player1->getPosition();
                    
                    auto* gm = GameManager::get();
                    int iconType = 0;
                    if (this->m_player1->m_isShip) {
                        iconType = this->m_player1->m_isPlatformer ? 8 : 1;
                    } else if (this->m_player1->m_isBall) {
                        iconType = 2;
                    } else if (this->m_player1->m_isBird) {
                        iconType = 3;
                    } else if (this->m_player1->m_isDart) {
                        iconType = 4;
                    } else if (this->m_player1->m_isRobot) {
                        iconType = 5;
                    } else if (this->m_player1->m_isSpider) {
                        iconType = 6;
                    } else if (this->m_player1->m_isSwing) {
                        iconType = 7;
                    }

                    auto col1 = gm->colorForIdx(gm->getPlayerColor());
                    auto col2 = gm->colorForIdx(gm->getPlayerColor2());
                    bool glowEnabled = gm->getPlayerGlow();
                    auto colGlow = gm->colorForIdx(gm->getPlayerGlowColor());

                    std::stringstream ss;
                    ss << "pt:1:" 
                       << iconType << ":" 
                       << this->m_player1->getRotation() << ":" 
                       << (this->m_player1->m_isUpsideDown ? 1 : 0) << ":"
                       << gm->getPlayerFrame() << ":"
                       << gm->getPlayerShip() << ":"
                       << gm->getPlayerBall() << ":"
                       << gm->getPlayerBird() << ":"
                       << gm->getPlayerDart() << ":"
                       << gm->getPlayerRobot() << ":"
                       << gm->getPlayerSpider() << ":"
                       << gm->getPlayerSwing() << ":"
                       << static_cast<int>(col1.r) << ":" << static_cast<int>(col1.g) << ":" << static_cast<int>(col1.b) << ":"
                       << static_cast<int>(col2.r) << ":" << static_cast<int>(col2.g) << ":" << static_cast<int>(col2.b) << ":"
                       << (glowEnabled ? 1 : 0) << ":"
                       << static_cast<int>(colGlow.r) << ":" << static_cast<int>(colGlow.g) << ":" << static_cast<int>(colGlow.b) << ":"
                       << (this->m_player1->m_vehicleSize < 1.0f ? 1 : 0);
                       
                    if (this->m_player2 && this->m_gameState.m_isDualMode) {
                        int p2IconType = 0;
                        if (this->m_player2->m_isShip) p2IconType = this->m_player2->m_isPlatformer ? 8 : 1;
                        else if (this->m_player2->m_isBall) p2IconType = 2;
                        else if (this->m_player2->m_isBird) p2IconType = 3;
                        else if (this->m_player2->m_isDart) p2IconType = 4;
                        else if (this->m_player2->m_isRobot) p2IconType = 5;
                        else if (this->m_player2->m_isSpider) p2IconType = 6;
                        else if (this->m_player2->m_isSwing) p2IconType = 7;
                        
                        auto p2Pos = this->m_player2->getPosition();
                        ss << ":1:"
                           << p2Pos.x << ":" << p2Pos.y << ":"
                           << this->m_player2->getRotation() << ":"
                           << (this->m_player2->m_isUpsideDown ? 1 : 0) << ":"
                           << (this->m_player2->m_vehicleSize < 1.0f ? 1 : 0) << ":"
                           << p2IconType;
                    } else {
                        ss << ":0:0:0:0:0:0:0";
                    }
                    
                    ss << ":" << (this->m_player1->m_isGoingLeft ? 1 : 0);
                    ss << ":" << (this->m_player2 && this->m_player2->m_isGoingLeft ? 1 : 0);
                    
                    statusStr = ss.str();
                } else {
#ifdef GEODE_IS_MOBILE
                    if (hookstate::s_isTouching) {
                        levelPos = this->m_objectLayer->convertToNodeSpace(hookstate::s_lastTouchPos);
                        m_fields->m_lastSentLevelPos = levelPos;
                    } else {
                        levelPos = m_fields->m_lastSentLevelPos;
                    }
#else
                    auto mousePos = geode::cocos::getMousePos();
                    levelPos = this->m_objectLayer->convertToNodeSpace(mousePos);
#endif
                    
                    if (auto* ui = this->m_editorUI) {
                        int mode = ui->m_selectedMode;
                        int swipe = ui->m_swipeEnabled ? 1 : 0;
                        int objectId = 0;
                        if (mode == 2) {
                            objectId = hookstate::s_selectedObjectID;
                        } else if (mode == 3) {
                            if (ui->m_selectedObject) {
                                objectId = ui->m_selectedObject->m_objectID;
                            } else if (ui->m_selectedObjects && ui->m_selectedObjects->count() > 0) {
                                if (auto* first = typeinfo_cast<GameObject*>(ui->m_selectedObjects->objectAtIndex(0))) {
                                    objectId = first->m_objectID;
                                }
                            }
                        }
                        statusStr = std::to_string(mode) + ":" + std::to_string(swipe) + ":" + std::to_string(objectId);
                    }
                }
                
                auto data = proto::serializeCursorUpdate(levelPos.x, levelPos.y, statusStr);
                P2PManager::get().send(std::move(data), ChannelType::Unreliable);
                
                auto& session = SessionManager::get();
                if (session.isInSession()) {
                    session.updatePlayerCursor(session.getLocalPlayerId(), levelPos.x, levelPos.y, statusStr);
                }
            }
        }
    }
};



namespace mpedit {
    void RemoteActionHandler::sendSnapshotToServer(std::function<void()> onComplete) {
        if (auto* layer = LevelEditorLayer::get()) {
            ::sendChunkedSync(layer, 0, onComplete);
        } else if (onComplete) {
            onComplete();
        }
    }
}

