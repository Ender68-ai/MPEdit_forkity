#include "Panel.hpp"

#include <Geode/Geode.hpp>

Panel* Panel::create(const char* title, CCSize size) {
    auto ret = new Panel();

    if (ret && ret->init(title, size)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool Panel::init(const char* title, CCSize size) {
    if (!CCNode::init())
        return false;

    setAnchorPoint({0.5f, 0.5f});
    setContentSize(size);

    m_background = extension::CCScale9Sprite::create("square02b_001.png");
    if (!m_background) {
        log::error("Panel: Failed to create background sprite");
        return false;
    }
    m_background->setAnchorPoint({0.5f, 0.5f});
    m_background->setPosition({0.f, 0.f});
    m_background->setContentSize(size);
    m_background->setColor({139, 69, 19});
    addChild(m_background, 0);

    m_content = CCNode::create();
    if (!m_content) {
        log::error("Panel: Failed to create content node");
        return false;
    }
    m_content->setAnchorPoint({0.5f, 0.5f});
    m_content->setPosition({0.f, 0.f});
    m_content->setContentSize(size);
    addChild(m_content, 1);

    if (title && title[0] != '\0') {
        m_title = CCLabelBMFont::create(title, "bigFont.fnt");
        if (!m_title) {
            log::error("Panel: Failed to create title label");
            return false;
        }
        m_title->setPosition({0.f, size.height * 0.5f - 20.f});
        addChild(m_title, 2);
    }

    return true;
}

void Panel::setPanelSize(CCSize size) {
    setContentSize(size);
    if (m_background) {
        m_background->setContentSize(size);
    }
    if (m_content) {
        m_content->setContentSize(size);
    }
    if (m_title) {
        m_title->setPosition({0.f, size.height * 0.5f - 20.f});
    }
}
