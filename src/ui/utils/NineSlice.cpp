#include <Geode/loader/Log.hpp>

#include "NineSlice.hpp"

NineSliceBox* NineSliceBox::create(float width, float height) {
    auto ret = new NineSliceBox();

    if (ret && ret->init(width, height)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool NineSliceBox::init(float width, float height) {
    if (!CCNode::init())
        return false;

    m_bg = geode::NineSlice::create(
        "GJ_square01.png",
        {},
        {
            .top = 10.f,
            .right = 10.f,
            .bottom = 10.f,
            .left = 10.f
        }
    );

    m_bg->setContentSize({width, height});
    m_bg->setAnchorPoint({0, 0});
    m_bg->setOpacity(255);

    this->addChild(m_bg);

    return true;
}

void NineSliceBox::setSize(float width, float height) {
    m_bg->setContentSize({width, height});
}

void NineSliceBox::animateResize(
    float targetWidth,
    float targetHeight,
    float duration
) {
    if (!m_bg) {
        geode::log::error("NineSliceBox: m_bg is null!");
        return;
    }

    if (duration <= 0.f) {
        m_bg->setContentSize({targetWidth, targetHeight});
        return;
    }

    m_startSize = m_bg->getContentSize();
    m_targetSize = cocos2d::CCSize(targetWidth, targetHeight);
    m_animationTime = 0.f;
    m_animationDuration = duration;

    this->unschedule(
        schedule_selector(NineSliceBox::updateResize)
    );

    this->schedule(
        schedule_selector(NineSliceBox::updateResize)
    );
}

void NineSliceBox::updateResize(float dt) {
    m_animationTime += dt;

    float progress = m_animationTime / m_animationDuration;

    if (progress >= 1.f) {
        progress = 1.f;
    }

    float width =
        m_startSize.width +
        (m_targetSize.width - m_startSize.width) * progress;

    float height =
        m_startSize.height +
        (m_targetSize.height - m_startSize.height) * progress;

    m_bg->setContentSize({width, height});
    this->setContentSize({width, height});

    if (progress >= 1.f) {
        this->unschedule(
            schedule_selector(NineSliceBox::updateResize)
        );
    }
}