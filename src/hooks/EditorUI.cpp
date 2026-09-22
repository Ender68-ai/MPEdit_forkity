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


namespace {
    void syncTransformedObjects(cocos2d::CCArray* objects,
                                std::function<void()> applyBase) {
        if (hookstate::s_inTransformSync) {
            applyBase();
            return;
        }

        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        struct ObjState {
            std::string uuid;
            GameObject* obj;
            cocos2d::CCPoint oldPos;
        };
        std::vector<ObjState> selected;

        if (session.isInSession() && !handler.isProcessingRemote() && objects) {
            for (auto* obj : CCArrayExt<GameObject*>(objects)) {
                if (!obj) continue;
                if (handler.isObjectPendingPlacement(obj)) {
                    handler.flushPendingPlacements();
                }
                auto uuid = handler.getUUIDForObject(obj);
                if (!uuid.empty()) {
                    selected.push_back({uuid, obj, obj->getPosition()});
                }
            }
        }

        hookstate::s_inTransformSync = true;
        applyBase();
        hookstate::s_inTransformSync = false;

        if (selected.empty()) return;

        std::vector<ActionSerializer::TransformData> transforms;
        std::vector<ActionSerializer::MoveData> moves;

        for (auto& state : selected) {
            ActionSerializer::TransformData td;
            td.uuid = state.uuid;
            td.rotation = state.obj->getRotation();
            td.scaleX = state.obj->getScaleX();
            td.scaleY = state.obj->getScaleY();
            td.flipX = state.obj->isFlipX();
            td.flipY = state.obj->isFlipY();
            transforms.push_back(td);

            cocos2d::CCPoint newPos = state.obj->getPosition();
            float dx = newPos.x - state.oldPos.x;
            float dy = newPos.y - state.oldPos.y;
            if (dx != 0.f || dy != 0.f) {
                ActionSerializer::MoveData md;
                md.uuid = state.uuid;
                md.dx = dx;
                md.dy = dy;
                moves.push_back(md);
            }
        }

        for (auto const& t : transforms) {
            MessageBatcher::get().queueTransform(t.uuid, t);
        }
        for (auto const& m : moves) {
            MessageBatcher::get().queueMove(m.uuid, m.dx, m.dy);
        }

        auto& tracked = handler.getTrackedSelections();
        auto* editor = LevelEditorLayer::get();
        if (editor) {
            for (auto& state : selected) {
                auto tIt = tracked.find(state.obj);
                if (tIt != tracked.end()) {
                    tIt->second = state.obj->getSaveString(editor);
                }
            }
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
            log::info("EditorHooks: Broadcasted granular property updates for {} objects from popup", updates.size());
        }
    }
}


class $modify(MPEditorUI, EditorUI) {
    struct Fields {
        float m_lockRefreshTimer = 0.f;
    };

    void onSettings(cocos2d::CCObject* sender) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        EditorUI::onSettings(sender);
    }

    void undoLastAction(cocos2d::CCObject* sender) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        EditorUI::undoLastAction(sender);
    }

    void redoLastAction(cocos2d::CCObject* sender) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        EditorUI::redoLastAction(sender);
    }

    void onCreateObject(int id) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        EditorUI::onCreateObject(id);
        hookstate::s_selectedObjectID = id;
    }

    void toggleMode(cocos2d::CCObject* btn) {
        if (SessionManager::get().isLocalPlayerViewOnly() && btn != this->m_editModeBtn) {
            EditorUI::toggleMode(this->m_editModeBtn);
            return;
        }
        EditorUI::toggleMode(btn);
    }

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        if (SessionManager::get().isLocalPlayerViewOnly()) {
            if (this->m_selectedMode != 2) {
                this->toggleMode(this->m_editModeBtn);
            }
            return EditorUI::ccTouchBegan(touch, event);
        }
        bool res = EditorUI::ccTouchBegan(touch, event);
        if (touch) {
            hookstate::s_lastTouchPos = touch->getLocation();
            hookstate::s_isTouching = true;
        }
        return res;
    }

    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        EditorUI::ccTouchMoved(touch, event);
        if (touch) {
            hookstate::s_lastTouchPos = touch->getLocation();
            hookstate::s_isTouching = true;
        }
    }

    void keyDown(cocos2d::enumKeyCodes key, double timestamp) {
        if (key == cocos2d::enumKeyCodes::KEY_Slash) {
            auto& session = SessionManager::get();
            if (session.isInSession()) {
                QuickChatPopup::create()->show();
                return;
            }
        }
        EditorUI::keyDown(key, timestamp);
    }

    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        EditorUI::ccTouchEnded(touch, event);
        hookstate::s_isTouching = false;
        if (SessionManager::get().isInSession()) {
            MessageBatcher::get().flush();
        }
    }

    void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        EditorUI::ccTouchCancelled(touch, event);
        hookstate::s_isTouching = false;
        if (SessionManager::get().isInSession()) {
            MessageBatcher::get().flush();
        }
    }

    void selectObject(GameObject* obj, bool filter) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        if (session.isInSession() && !handler.isProcessingRemote() && obj) {
            auto uuid = handler.getUUIDForObject(obj);
            if (!uuid.empty()) {
                auto const& locks = handler.getObjectLocks();
                auto it = locks.find(uuid);
                if (it != locks.end() && it->second.playerId != session.getLocalPlayerId()) {
                    return;
                }
            }
        }

        EditorUI::selectObject(obj, filter);

        if (session.isInSession() && obj) {
            auto uuid = handler.getOrCreateUUID(obj);
            auto& tracked = handler.getTrackedSelections();
            if (tracked.find(obj) == tracked.end()) {
                if (auto* editor = LevelEditorLayer::get()) {
                    tracked[obj] = obj->getSaveString(editor);
                }
                if (!handler.isProcessingRemote()) {
                    sendChunkedLockObjects({uuid}, true);

                    if (handler.isObjectPendingPlacement(obj)) {
                        handler.flushPendingPlacements();
                    }
                }
            }
        }
    }

    void deselectObject(GameObject* obj) {
        EditorUI::deselectObject(obj);
        
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();
        if (session.isInSession() && obj) {
            auto& tracked = handler.getTrackedSelections();
            if (!handler.isProcessingRemote()) {
                if (handler.isObjectPendingPlacement(obj)) {
                    handler.flushPendingPlacements();
                }

                auto uuid = handler.getUUIDForObject(obj);
                if (!uuid.empty()) {
                    auto tIt = tracked.find(obj);
                    if (tIt != tracked.end()) {
                        if (auto* editor = LevelEditorLayer::get()) {
                            std::string currentSave = obj->getSaveString(editor);
                            if (ActionSerializer::hasDeepPropertyChanges(obj, tIt->second, currentSave)) {
                                auto objData = ActionSerializer::extractObjectData(obj, uuid);
                                sendChunkedUpdateObjects({objData});
                            }
                        }
                    }

                    ActionSerializer::ReconcileData rec;
                    rec.uuid = uuid;
                    rec.x = obj->getPositionX();
                    rec.y = obj->getPositionY();
                    rec.rotation = obj->getRotation();
                    rec.scaleX = obj->getScaleX();
                    rec.scaleY = obj->getScaleY();
                    rec.flipX = obj->isFlipX();
                    rec.flipY = obj->isFlipY();
                    
                    sendChunkedReconcileObjects({rec});
                    
                    MessageBatcher::get().removePending(uuid);

                    sendChunkedLockObjects({uuid}, false);
                }
            }
            tracked.erase(obj);
        }
    }

    void deselectAll() {
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();
        if (session.isInSession()) {
            if (!handler.isProcessingRemote()) {
                auto* editor = LevelEditorLayer::get();
                std::vector<std::string> uuids;
                std::vector<ActionSerializer::ReconcileData> reconciles;
                std::vector<ActionSerializer::ObjectData> updates;
                
                auto& tracked = handler.getTrackedSelections();
                
                for (auto& [obj, savedString] : tracked) {
                    if (!editor || !editor->m_objects || !editor->m_objects->containsObject(obj)) {
                        continue;
                    }

                    if (handler.isObjectPendingPlacement(obj)) {
                        handler.flushPendingPlacements();
                    }

                    auto uuid = handler.getUUIDForObject(obj);
                    if (uuid.empty()) continue;
                    
                    uuids.push_back(uuid);
                    
                    std::string currentSave = obj->getSaveString(editor);
                    if (ActionSerializer::hasDeepPropertyChanges(obj, savedString, currentSave)) {
                        updates.push_back(ActionSerializer::extractObjectData(obj, uuid));
                    }

                    ActionSerializer::ReconcileData rec;
                    rec.uuid = uuid;
                    rec.x = obj->getPositionX();
                    rec.y = obj->getPositionY();
                    rec.rotation = obj->getRotation();
                    rec.scaleX = obj->getScaleX();
                    rec.scaleY = obj->getScaleY();
                    rec.flipX = obj->isFlipX();
                    rec.flipY = obj->isFlipY();
                    reconciles.push_back(rec);
                    
                    MessageBatcher::get().removePending(uuid);
                }
                
                if (!updates.empty()) {
                    sendChunkedUpdateObjects(updates);
                }
                if (!reconciles.empty()) {
                    sendChunkedReconcileObjects(reconciles);
                }
                if (!uuids.empty()) {
                    sendChunkedLockObjects(uuids, false);
                }
            }
            handler.getTrackedSelections().clear();
        }
        EditorUI::deselectAll();
    }

    void onDeleteSelected(cocos2d::CCObject* sender) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        if (session.isInSession() && !handler.isProcessingRemote()) {
            std::vector<std::string> uuids;
            
            if (m_selectedObjects && m_selectedObjects->count() > 0) {
                for (auto* obj : CCArrayExt<GameObject*>(m_selectedObjects)) {
                    auto uuid = handler.getUUIDForObject(obj);
                    if (!uuid.empty()) {
                        uuids.push_back(uuid);
                        handler.unregisterObject(uuid);
                    }
                }
            } else if (m_selectedObject) {
                auto uuid = handler.getUUIDForObject(m_selectedObject);
                if (!uuid.empty()) {
                    uuids.push_back(uuid);
                    handler.unregisterObject(uuid);
                }
            }

            if (!uuids.empty()) {
                sendChunkedDeleteObjects(uuids);
            }
        }

        EditorUI::onDeleteSelected(sender);
    }

    bool shouldDeleteObject(GameObject* obj) {
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        if (session.isInSession() && !handler.isProcessingRemote() && obj) {
            auto uuid = handler.getUUIDForObject(obj);
            if (!uuid.empty()) {
                auto const& locks = handler.getObjectLocks();
                auto it = locks.find(uuid);
                if (it != locks.end() && it->second.playerId != session.getLocalPlayerId()) {
                    return false;
                }
            }
        }
        return EditorUI::shouldDeleteObject(obj);
    }

    void selectObjects(cocos2d::CCArray* objects, bool filter) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        cocos2d::CCArray* filteredObjects = objects;
        if (session.isInSession() && !handler.isProcessingRemote() && objects) {
            auto const& locks = handler.getObjectLocks();
            int localId = session.getLocalPlayerId();
            
            bool hasLocked = false;
            for (auto* obj : CCArrayExt<GameObject*>(objects)) {
                auto uuid = handler.getUUIDForObject(obj);
                if (!uuid.empty()) {
                    auto it = locks.find(uuid);
                    if (it != locks.end() && it->second.playerId != localId) {
                        hasLocked = true;
                        break;
                    }
                }
            }

            if (hasLocked) {
                filteredObjects = cocos2d::CCArray::create();
                for (auto* obj : CCArrayExt<GameObject*>(objects)) {
                    auto uuid = handler.getUUIDForObject(obj);
                    bool isLockedByOther = false;
                    if (!uuid.empty()) {
                        auto it = locks.find(uuid);
                        if (it != locks.end() && it->second.playerId != localId) {
                            isLockedByOther = true;
                        }
                    }
                    if (!isLockedByOther) {
                        filteredObjects->addObject(obj);
                    }
                }
            }
        }

        EditorUI::selectObjects(filteredObjects, filter);

        if (session.isInSession() && filteredObjects) {
            std::vector<std::string> uuids;
            auto& tracked = handler.getTrackedSelections();
            auto* editor = LevelEditorLayer::get();
            for (auto* obj : CCArrayExt<GameObject*>(filteredObjects)) {
                auto uuid = handler.getOrCreateUUID(obj);
                if (tracked.find(obj) == tracked.end()) {
                    if (editor) {
                        tracked[obj] = obj->getSaveString(editor);
                    }
                    uuids.push_back(uuid);
                }
            }
            if (!uuids.empty() && !handler.isProcessingRemote()) {
                sendChunkedLockObjects(uuids, true);
            }
        }
    }

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto* helper = UpdateHelperNode::create([this](float dt) {
            this->syncDeselections(dt);
        }, 0.1f);
        if (helper) {
            helper->setID("sync-deselect-helper"_spr);
            this->addChild(helper);
        }
        return true;
    }

    void syncDeselections(float dt) {
        auto* editor = LevelEditorLayer::get();
        if (!editor || !editor->m_objects || editor->m_playbackMode != PlaybackMode::Not) return;

        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();
        if (!session.isInSession() || handler.isProcessingRemote()) return;

        auto& tracked = handler.getTrackedSelections();
        auto const& locks = handler.getObjectLocks();
        int localId = session.getLocalPlayerId();

        std::vector<GameObject*> currentSelection;
        if (m_selectedObject) {
            currentSelection.push_back(m_selectedObject);
        }
        if (m_selectedObjects) {
            for (auto* obj : CCArrayExt<GameObject*>(m_selectedObjects)) {
                if (obj) currentSelection.push_back(obj);
            }
        }

        std::vector<GameObject*> toDeselect;
        std::vector<std::string> toLockUuids;

        for (auto* obj : currentSelection) {
            auto uuid = handler.getUUIDForObject(obj);
            if (uuid.empty()) {
                uuid = RemoteActionHandler::generateUUID();
                handler.registerObject(uuid, obj);
            }

            auto it = locks.find(uuid);
            if (it != locks.end() && it->second.playerId != localId) {
                toDeselect.push_back(obj);
            } else {
                if (tracked.find(obj) == tracked.end()) {
                    tracked[obj] = obj->getSaveString(editor);
                    toLockUuids.push_back(uuid);
                }
            }
        }

        for (auto* obj : toDeselect) {
            this->deselectObject(obj);
            if (m_selectedObject == obj) {
                m_selectedObject = nullptr;
            }
            if (m_selectedObjects && m_selectedObjects->containsObject(obj)) {
                m_selectedObjects->removeObject(obj);
            }
        }

        if (!toLockUuids.empty()) {
            sendChunkedLockObjects(toLockUuids, true);
        }

        m_fields->m_lockRefreshTimer += dt;
        if (m_fields->m_lockRefreshTimer >= 1.0f) {
            m_fields->m_lockRefreshTimer = 0.f;
            std::vector<std::string> refreshUuids;
            for (auto const& [obj, _] : tracked) {
                if (editor->m_objects->containsObject(obj)) {
                    auto uuid = handler.getUUIDForObject(obj);
                    if (!uuid.empty()) {
                        refreshUuids.push_back(uuid);
                    }
                }
            }
            if (!refreshUuids.empty()) {
                sendChunkedLockObjects(refreshUuids, true);
            }
        }

        std::vector<std::string> unlockUuids;
        std::vector<ActionSerializer::ReconcileData> reconciles;
        std::vector<ActionSerializer::ObjectData> updates;

        for (auto it = tracked.begin(); it != tracked.end(); ) {
            GameObject* obj = it->first;

            if (!editor->m_objects->containsObject(obj)) {
                it = tracked.erase(it);
                continue;
            }

            bool isSelected = (std::find(currentSelection.begin(), currentSelection.end(), obj) != currentSelection.end()) &&
                              (std::find(toDeselect.begin(), toDeselect.end(), obj) == toDeselect.end());

            if (handler.isObjectPendingPlacement(obj)) {
                handler.flushPendingPlacements();
            }
            auto uuid = handler.getUUIDForObject(obj);
            if (uuid.empty()) {
                it = tracked.erase(it);
                continue;
            }

            if (!isSelected) {
                unlockUuids.push_back(uuid);

                ActionSerializer::ReconcileData rec;
                rec.uuid = uuid;
                rec.x = obj->getPositionX();
                rec.y = obj->getPositionY();
                rec.rotation = obj->getRotation();
                rec.scaleX = obj->getScaleX();
                rec.scaleY = obj->getScaleY();
                rec.flipX = obj->isFlipX();
                rec.flipY = obj->isFlipY();
                reconciles.push_back(rec);

                std::string currentSave = obj->getSaveString(editor);
                if (ActionSerializer::hasDeepPropertyChanges(obj, it->second, currentSave)) {
                    updates.push_back(ActionSerializer::extractObjectData(obj, uuid));
                }
                
                MessageBatcher::get().removePending(uuid);

                it = tracked.erase(it);
            } else {
                std::string currentSave = obj->getSaveString(editor);
                if (ActionSerializer::hasDeepPropertyChanges(obj, it->second, currentSave)) {
                    updates.push_back(ActionSerializer::extractObjectData(obj, uuid));
                    it->second = currentSave;
                } else if (it->second != currentSave) {
                    it->second = currentSave;
                    
                    MessageBatcher::get().removePending(uuid);
                    
                    ActionSerializer::ReconcileData rec;
                    rec.uuid = uuid;
                    rec.x = obj->getPositionX();
                    rec.y = obj->getPositionY();
                    rec.rotation = obj->getRotation();
                    rec.scaleX = obj->getScaleX();
                    rec.scaleY = obj->getScaleY();
                    rec.flipX = obj->isFlipX();
                    rec.flipY = obj->isFlipY();
                    reconciles.push_back(rec);
                }
                ++it;
            }
        }

        for (auto it = hookstate::s_startPosObjects.begin(); it != hookstate::s_startPosObjects.end(); ) {
            GameObject* obj = *it;
            if (!editor->m_objects->containsObject(obj)) {
                hookstate::s_startPosSaveStrings.erase(obj);
                it = hookstate::s_startPosObjects.erase(it);
                continue;
            }
            auto uuid = handler.getUUIDForObject(obj);
            if (!uuid.empty()) {
                std::string currentSave = obj->getSaveString(editor);
                if (hookstate::s_startPosSaveStrings.count(obj)) {
                    if (ActionSerializer::hasDeepPropertyChanges(obj, hookstate::s_startPosSaveStrings[obj], currentSave)) {
                        updates.push_back(ActionSerializer::extractObjectData(obj, uuid));
                    }
                }
                hookstate::s_startPosSaveStrings[obj] = currentSave;
            }
            ++it;
        }

        if (!unlockUuids.empty()) {
            sendChunkedLockObjects(unlockUuids, false);
        }
        if (!reconciles.empty()) {
            sendChunkedReconcileObjects(reconciles);
        }
        if (!updates.empty()) {
            sendChunkedUpdateObjects(updates);
        }
    }

    void moveObject(GameObject* obj, cocos2d::CCPoint position) {
        if (SessionManager::get().isLocalPlayerViewOnly() && !RemoteActionHandler::get().isProcessingRemote()) return;
        auto& handler = RemoteActionHandler::get();
        auto& session = SessionManager::get();

        if (session.isInSession() && !handler.isProcessingRemote() && obj) {
            auto uuid = handler.getUUIDForObject(obj);
            if (!uuid.empty()) {
                auto const& locks = handler.getObjectLocks();
                auto it = locks.find(uuid);
                if (it != locks.end() && it->second.playerId != session.getLocalPlayerId()) {
                    return;
                }
            }
        }

        CCPoint oldPos = obj->getPosition();

        EditorUI::moveObject(obj, position);

        if (session.isInSession() && !handler.isProcessingRemote()) {
            auto uuid = handler.getUUIDForObject(obj);
            if (handler.isObjectPendingPlacement(obj)) {
                return;
            }
            if (!uuid.empty()) {
                CCPoint newPos = obj->getPosition();
                ActionSerializer::MoveData move;
                move.uuid = uuid;
                move.dx = newPos.x - oldPos.x;
                move.dy = newPos.y - oldPos.y;

                if ((move.dx != 0.f || move.dy != 0.f) && !hookstate::s_inTransformSync) {
                    MessageBatcher::get().queueMove(uuid, move.dx, move.dy);

                    auto& tracked = handler.getTrackedSelections();
                    auto tIt = tracked.find(obj);
                    if (tIt != tracked.end()) {
                        if (auto* editor = LevelEditorLayer::get()) {
                            tIt->second = obj->getSaveString(editor);
                        }
                    }
                }
            }
        }
    }

    void transformObjectCall(EditCommand command) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        syncTransformedObjects(m_selectedObjects, [&]() {
            EditorUI::transformObjectCall(command);
        });
    }

    void rotateObjects(cocos2d::CCArray* objects, float rotation, cocos2d::CCPoint pivotPoint) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        syncTransformedObjects(objects, [&]() {
            EditorUI::rotateObjects(objects, rotation, pivotPoint);
        });
    }

    void scaleObjects(cocos2d::CCArray* objects, float scaleX, float scaleY, cocos2d::CCPoint pivotPoint, ObjectScaleType type, bool lockMove) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        syncTransformedObjects(objects, [&]() {
            EditorUI::scaleObjects(objects, scaleX, scaleY, pivotPoint, type, lockMove);
        });
    }

    void flipObjectsX(cocos2d::CCArray* objects) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        syncTransformedObjects(objects, [&]() {
            EditorUI::flipObjectsX(objects);
        });
    }

    void flipObjectsY(cocos2d::CCArray* objects) {
        if (SessionManager::get().isLocalPlayerViewOnly()) return;
        syncTransformedObjects(objects, [&]() {
            EditorUI::flipObjectsY(objects);
        });
    }
};

