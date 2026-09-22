#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include "ui/ui.hpp"
#include "ui/collablayer/CollabLayer.hpp"
#include "SessionManager.hpp"
#include "P2PManager.hpp"
#include "RemoteActionHandler.hpp"
#include "ui/menu/MultiplayerMenuPopup.hpp"
#include "ui/menu/CreateRoomPopup.hpp"

using namespace geode::prelude;
using namespace mpedit;

class $modify(MPEditorPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer* editor) {
        if (!EditorPauseLayer::init(editor)) return false;

        CCMenu* targetMenu = typeinfo_cast<CCMenu*>(this->getChildByIDRecursive("resume-menu"));
        if (!targetMenu) {
            targetMenu = typeinfo_cast<CCMenu*>(this->getChildByIDRecursive("center-button-menu"));
        }

        auto& session = SessionManager::get();

        if (session.isInSession() && session.getRole() == SessionManager::Role::Client && P2PManager::get().isDedicatedServer()) {
            auto* saveCopySprite = ButtonSprite::create(
                "Save Copy", 90, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.45f
            );
            auto* saveCopyBtn = CCMenuItemSpriteExtra::create(
                saveCopySprite,
                this,
                menu_selector(MPEditorPauseLayer::onSaveLocal)
            );
            saveCopyBtn->setID("save-copy-button"_spr);
            if (targetMenu) {
                targetMenu->addChild(saveCopyBtn);
            }
        }

        if (session.isInSession()) {
            auto disableBtn = [this](const char* id) {
                if (auto* btn = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive(id))) {
                    btn->setEnabled(false);
                    std::function<void(cocos2d::CCNode*)> grayOut = [&](cocos2d::CCNode* n) {
                        if (!n) return;
                        if (auto* rgba = typeinfo_cast<cocos2d::CCNodeRGBA*>(n)) {
                            rgba->setColor({100, 100, 100});
                        }
                        if (n->getChildren()) {
                            for (auto* c : CCArrayExt<cocos2d::CCNode*>(n->getChildren())) {
                                grayOut(c);
                            }
                        }
                    };
                    grayOut(btn->getNormalImage());
                }
            };

            if (session.getRole() == SessionManager::Role::Host) {
                disableBtn("save-and-play-button");
            } else if (session.getRole() == SessionManager::Role::Client) {
                if (!P2PManager::get().isDedicatedServer()) {
                    disableBtn("save-button");
                    disableBtn("save-and-exit-button");
                }
                disableBtn("save-and-play-button");
            }
        }

        return true;
    }

    void onSaveLocal(CCObject* sender) {
        EditorPauseLayer::saveLevel();
        auto* editor = LevelEditorLayer::get();
        if (editor && editor->m_level) {
            auto* newLevel = GameLevelManager::sharedState()->createNewLevel();
            if (newLevel) {
                newLevel->m_levelName = std::string(editor->m_level->m_levelName) + " local";
                newLevel->m_levelString = editor->m_level->m_levelString;
                newLevel->m_audioTrack = editor->m_level->m_audioTrack;
                newLevel->m_songID = editor->m_level->m_songID;
                newLevel->m_levelLength = editor->m_level->m_levelLength;
                Notification::create("Saved local copy!", cocos2d::CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
            }
        }
    }

    void onSave(CCObject* sender) {
        if (SessionManager::get().isInSession() && SessionManager::get().getRole() == SessionManager::Role::Client) {
            if (!P2PManager::get().isDedicatedServer()) {
                Notification::create("Guests cannot save levels", NotificationIcon::Warning)->show();
                return;
            }
            if (auto* btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
                btn->setEnabled(false);
            }
            auto* loadingCircle = LoadingCircle::create();
            loadingCircle->setParentLayer(this);
            loadingCircle->show();

            this->retain();
            RemoteActionHandler::get().sendSnapshotToServer([this, sender, loadingCircle]() {
                loadingCircle->fadeAndRemove();
                Notification::create("Saved to Server!", cocos2d::CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
                if (auto* btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
                    btn->setEnabled(true);
                }
                this->release();
            });
            return;
        }
        EditorPauseLayer::onSave(sender);
    }

    void onSaveAndPlay(CCObject* sender) {
        if (SessionManager::get().isInSession()) {
            Notification::create("Cannot Save & Play in multiplayer", NotificationIcon::Warning)->show();
            return;
        }
        EditorPauseLayer::onSaveAndPlay(sender);
    }

    void onSaveAndExit(CCObject* sender) {
        auto& session = SessionManager::get();
        if (session.isInSession()) {
            if (session.getRole() == SessionManager::Role::Client) {
                if (!P2PManager::get().isDedicatedServer()) {
                    Notification::create("Guests cannot save levels", NotificationIcon::Warning)->show();
                    return;
                }
                if (auto* btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
                    btn->setEnabled(false);
                }        
                auto* loadingCircle = LoadingCircle::create();
                loadingCircle->setParentLayer(this);
                loadingCircle->show();

                this->retain();
                RemoteActionHandler::get().sendSnapshotToServer([this, sender, loadingCircle]() {
                    loadingCircle->fadeAndRemove();
                    Notification::create("Saved to Server!", cocos2d::CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
                    this->m_saved = true;
                    this->onExitEditor(sender);
                    this->release();
                });
                return;
            }

            EditorPauseLayer::saveLevel();
            this->onExitEditor(sender);
            return;
        }

        EditorPauseLayer::onSaveAndExit(sender);
    }

    void onExitEditor(CCObject* sender) {
        auto& session = SessionManager::get();
        if (session.isInSession()) {
            session.leaveSession();
            auto collabLayer = CollabLayer::create();
            auto scene = CCScene::create();
            scene->addChild(collabLayer);
            auto transition = Transition::create(0.5f, scene, {0, 0, 0});
            CCDirector::sharedDirector()->replaceScene(transition);
        } else {
            EditorPauseLayer::onExitEditor(sender);
        }
    }

    void onMultiplayer(CCObject*) {
        if (SessionManager::get().isInSession()) {
            MultiplayerMenuPopup::create()->show();
        } else {
            CreateRoomPopup::create(nullptr)->show();
        }
    }
};