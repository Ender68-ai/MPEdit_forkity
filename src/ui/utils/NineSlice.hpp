#pragma once

#include <Geode/Geode.hpp>

class NineSliceBox : public cocos2d::CCNode {
public:
    static NineSliceBox* create(float width, float height);

    void setSize(float width, float height);
    void animateResize(float targetWidth, float targetHeight, float duration);

private:
    bool init(float width, float height);
    void updateResize(float dt);

    geode::NineSlice* m_bg;

    cocos2d::CCSize m_startSize;
    cocos2d::CCSize m_targetSize;

    float m_animationTime = 0.f;
    float m_animationDuration = 0.f;
};

