#include <Geode/binding/LevelCell.hpp>
#include <Geode/binding/EditLevelLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/loader/Log.hpp>


using namespace geode::prelude;

class $modify(MPLevelCell, LevelCell) {

    void onClick(CCObject* sender) {
        auto* scene = CCDirector::sharedDirector()->getRunningScene();

        bool fromCollab = scene && scene->getChildByID(
            "ender68.multiplayeredit/collab-layer"
        ) != nullptr;

        if (fromCollab) {
            CCDirector::sharedDirector()->replaceScene(
                LevelEditorLayer::scene(m_level, false)
            );
            log::info(
            "BEFORE PUSH: running scene = {}",
            CCDirector::sharedDirector()->getRunningScene()
            );
            return;
        }

        LevelCell::onClick(sender);
    }

    void loadFromLevel(GJGameLevel* level) {

        // Basically load the cells and then schedule upd
        
        LevelCell::loadFromLevel(level);

        this->schedule(
                schedule_selector(MPLevelCell::updateCollabButtons),
                0.5f
            );      
    }

    void updateCollabButtons(float dt) {

        auto scene = CCDirector::sharedDirector()->getRunningScene();

        bool isCollabScene =
            scene->getChildByID("ender68.multiplayeredit/collab-layer") != nullptr;


            if (!isCollabScene) {
                return;
            }

        auto mainLayer = this->getChildByID("main-layer");
            if (!mainLayer) {
                log::debug("NO MAIN LAYER");
                return;
            }

            if (auto infoIcon = mainLayer->getChildByID("info-icon")) {
                infoIcon->setVisible(false);
            }
            if (auto infoLabel = mainLayer->getChildByID("info-label")) {
                infoLabel->setVisible(false);
            }
            if (auto revision = mainLayer->getChildByID("level-revision")) {
                revision->setVisible(false);
            }

        auto mainMenu = mainLayer->getChildByID("main-menu");

            if (!mainMenu) {
                return;
            }            
            if (auto toggler = mainMenu->getChildByID("select-toggler")) {
                toggler->setVisible(false);
            }
            

            auto view = mainMenu->getChildByID("view-button");

            auto viewButton = typeinfo_cast<CCMenuItemSpriteExtra*>(view);
                if (!viewButton) {
                    return;
                }

            auto btnSprite = typeinfo_cast<ButtonSprite*>(viewButton->getNormalImage());
                if(!btnSprite)
                    return;

                btnSprite->m_label->setString("Host");
                btnSprite->m_BGSprite->setColor({30, 128, 255});  
                
    }
};