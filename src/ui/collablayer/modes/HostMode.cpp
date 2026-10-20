#include "HostMode.hpp"
#include "hostmode/RoomCreateLayer.hpp"
#include "SessionManager.hpp"
#include "ui/collablayer/CollabLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/LocalLevelManager.hpp>
#include <Geode/binding/CustomListView.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>

using namespace geode::prelude;

// Loading overlay helper
static CCNode* createLoadingOverlay(const char* message, CCLabelBMFont*& loadingLabel) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Background dimmer
    auto background = CCScale9Sprite::create("black.png");
    background->setContentSize({winSize.width, winSize.height});
    background->setPosition({winSize.width * 0.5f, winSize.height * 0.5f});
    background->setColor({0, 0, 0});
    background->setOpacity(150);
    background->setID("mpedit-loading-bg"_spr);
    
    // Loading panel
    auto panel = CCScale9Sprite::create("GJ_square02.png");
    panel->setContentSize({280.f, 120.f});
    panel->setPosition({winSize.width * 0.5f, winSize.height * 0.5f});
    panel->setID("mpedit-loading-panel"_spr);
    
    // Loading label
    auto label = CCLabelBMFont::create(message, "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition({0.f, 25.f});
    label->setID("mpedit-loading-label"_spr);
    panel->addChild(label);
    loadingLabel = label;
    
    // Spinning indicator (simple circle)
    auto spinner = CCSprite::createWithSpriteFrameName("GJSpinner_001.png");
    if (!spinner) {
        // Fallback: create a simple circle using draw primitives
        spinner = CCSprite::createWithSpriteFrameName("GJ_chatIcon_001.png");
    }
    spinner->setScale(0.8f);
    spinner->setPosition({0.f, -20.f});
    spinner->setID("mpedit-loading-spinner"_spr);
    
    // Animate spinning
    auto spinAction = CCRepeatForever::create(CCSequence::create(
        CCRotateBy::create(0.5f, 360.f),
        nullptr
    ));
    spinner->runAction(spinAction);
    
    panel->addChild(spinner);
    
    // Container
    auto container = CCNode::create();
    container->setContentSize({winSize.width, winSize.height});
    container->addChild(background);
    container->addChild(panel);
    container->setID("mpedit-loading-container"_spr);
    
    return container;
}

HostMode* HostMode::create() {
    auto ret = new HostMode();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

HostMode::~HostMode() {
    mpedit::SessionManager::get().removeListener(this);
    hideLoadingOverlay();
}

HostMode* HostMode::get() {
    auto* runningScene = CCDirector::sharedDirector()->getRunningScene();
    if (!runningScene) return nullptr;
    auto findHostMode = [](auto&& findHostMode, CCNode* node) -> HostMode* {
        if (auto* hostMode = typeinfo_cast<HostMode*>(node)) {
            return hostMode;
        }
        for (auto* child : CCArrayExt<CCNode*>(node->getChildren())) {
            if (auto* hostMode = findHostMode(findHostMode, child)) {
                return hostMode;
            }
        }
        return nullptr;
    };
    return findHostMode(findHostMode, runningScene);
}


void HostMode::showLoadingOverlay(const char* message) {
    if (m_loadingOverlay) {
        return; // Already showing
    }
    
    m_loadingOverlay = createLoadingOverlay(message, m_loadingLabel);
    this->addChild(m_loadingOverlay, 1000); // High z-order to be on top
}

void HostMode::hideLoadingOverlay() {
    if (m_loadingOverlay) {
        m_loadingOverlay->removeFromParentAndCleanup(true);
        m_loadingOverlay = nullptr;
        m_loadingLabel = nullptr;
    }
}


void HostMode::showListAfterDelay(float) {
        m_listLayer->setVisible(true);
}

void HostMode::onLevelSelected(GJGameLevel* level) {
    if (!level) {
        log::warn("HostMode: Cannot load null level");
        hideLoadingOverlay();
        return;
    }
    
    log::info("HostMode: Loading level '{}' (ID: {}) for multiplayer session", 
              level->m_levelName, level->m_levelID);
    
    // Set the fromCollab flag so EditLevelLayer hooks know we're coming from multiplayer
    fromCollab = true;
    
    // Load the level into the editor
    auto* scene = LevelEditorLayer::scene(level, false);
    if (!scene) {
        log::error("HostMode: Failed to create editor scene for level");
        fromCollab = false;
        hideLoadingOverlay();
        geode::queueInMainThread([this]() {
            geode::Notification::create("Failed to load level.", geode::NotificationIcon::Error)->show();
        });
        return;
    }
    
    // Hide the loading overlay before scene transition
    hideLoadingOverlay();
    
    // Transition to the editor scene
    auto* director = CCDirector::sharedDirector();
    auto* transition = CCTransitionFade::create(0.5f, scene);
    director->replaceScene(transition);
    
    log::info("HostMode: Transited to editor for level '{}'", level->m_levelName);
}

bool HostMode::init() {
    if (!CCNode::init())
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Track this pointer for callbacks (HostMode lives as long as CollabLayer)
    auto* self = this;

    auto roomSetup = RoomCreateLayer::create([this, self]() {
        if (m_listLayer) {
            this->scheduleOnce(schedule_selector(HostMode::showListAfterDelay), 0.5f);
        }
    });

    if (!roomSetup) {
        return false;
    }

    this->addChild(roomSetup);

    // Register callback for when session ends
    mpedit::SessionManager::get().onSessionEnded(this, [this, self]() {
        hideLoadingOverlay();
        if (m_listLayer) {
            m_listLayer->setVisible(false);
        }
    });

    // Error handling — show notification and reset UI state
    mpedit::SessionManager::get().onError(this, [self](std::string const& error) {
        geode::Notification::create(error.c_str(), geode::NotificationIcon::Error)->show();
        geode::queueInMainThread([self]() {
            self->hideLoadingOverlay();
        });
    });

    // Status updates — update loading overlay text if showing
    mpedit::SessionManager::get().onStatus(this, [self](std::string const& status) {
        geode::queueInMainThread([self, status]() {
            if (self->m_loadingLabel) {
                self->m_loadingLabel->setString(status.c_str());
            }
        });
    });

    auto delegate = HostLocalLevelList::create();

    auto listView = CustomListView::create(
        LocalLevelManager::sharedState()->m_localLevels,
        delegate,
        200.f,
        200.f,
        0,
        BoomListType::Level,
        0.f
    );

    if (!listView)
        return false;

    listView->setID("local-levels-list"_spr);

    auto levelListLayer = GJListLayer::create(
        listView,
        "Local",
        {255, 255, 255, 255},
        200.f,
        200.f,
        0
    );

    if (!levelListLayer)
        return false;

    auto top = levelListLayer->getChildByID("top-border");
    auto bottom = levelListLayer->getChildByID("bottom-border");

    if (top) {
        top->setScaleX(0.6f);
    }

    if (bottom) {
        bottom->setScaleX(0.6f);
    }

    levelListLayer->setPosition({
        winSize.width * 0.55f,
        winSize.height * 0.12f
    });

    levelListLayer->setVisible(false);
    levelListLayer->setScale(0.9f);

    m_listLayer = levelListLayer;
    this->addChild(m_listLayer);

    return true;
}

// HostLocalLevelList

// Helper function to get level from cell (must be declared before cellPerformedAction usage)
static GJGameLevel* getLevelFromCell(TableViewCell* cell) {
    if (!cell) return nullptr;
    
    auto* lm = LocalLevelManager::sharedState();
    if (!lm) return nullptr;
    
    // Get the level index from the cell's tag (set by TableView delegate)
    int levelIdx = static_cast<int>(cell->getTag());
    
    if (levelIdx >= 0 && levelIdx < static_cast<int>(lm->m_localLevels->count())) {
        auto* levelObj = static_cast<CCObject*>(lm->m_localLevels->objectAtIndex(levelIdx));
        return typeinfo_cast<GJGameLevel*>(levelObj);
    }
    
    return nullptr;
}

HostLocalLevelList* HostLocalLevelList::create() {
    auto ret = new HostLocalLevelList();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    

    delete ret;
    return nullptr;
}

bool HostLocalLevelList::cellPerformedAction(
    TableViewCell* cell,
    int listType,
    CellAction action,
    cocos2d::CCNode* parent
) {
    if (action == CellAction::Click) {
        auto* level = getLevelFromCell(cell);
        if (!level) {
            log::warn("HostLocalLevelList: Could not get level from cell");
            return true;
        }
        
        // Get the HostMode parent to trigger the transition
        auto* hostMode = HostMode::get();
        if (!hostMode) {
            log::warn("HostLocalLevelList: HostMode not found");
            return true;
        }
        
        // Trigger level selection with loading feedback
        hostMode->showLoadingOverlay("Loading level...");
        hostMode->onLevelSelected(level);
        
        return true;
    }

    return false;
}

int HostLocalLevelList::getSelectedCellIdx() {
    return -1;
}

bool HostLocalLevelList::shouldSnapToSelected() {
    return false;
}

int HostLocalLevelList::getCellDelegateType() {
    return 0;
}

bool HostLocalLevelList::init() {
    if (!CCNode::init())
        return false; 

    return true;
}
