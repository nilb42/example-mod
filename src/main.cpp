#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelCompleteLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

bool g_isModActive = true;

class $modify(MyUILayer, UILayer) {
    void keyDown(enumKeyCodes key) {
        UILayer::keyDown(key);
        if (key == enumKeyCodes::KEY_LeftBracket) {
            g_isModActive = !g_isModActive;
            if (g_isModActive) {
                Notification::create("Smooth Spam: ON", NotificationIcon::Success)->show();
            } else {
                Notification::create("Smooth Spam: OFF", NotificationIcon::Error)->show();
            }
        }
    }
};

class $modify(SpamWarningLayer, LevelCompleteLayer) {
    bool init() {
        if (!LevelCompleteLayer::init()) return false;
        if (g_isModActive) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto label = CCLabelBMFont::create("smooth spam used", "chatFont.fnt");
            label->setPosition({winSize.width - 5.0f, 5.0f});
            label->setAnchorPoint({1.0f, 0.0f});
            label->setScale(0.5f);
            label->setOpacity(150);
            label->setColor({255, 50, 50});
            this->addChild(label);
        }
        return true;
    }
};

class $modify(MyPlayer, PlayerObject) {
    float m_clickTimer = 0.0f;
    float m_lastY = 0.0f;
    float m_slope = 0.0f;
    bool m_isSpamming = false;

    void pushButton(PlayerButton btn) {
        if (!g_isModActive || !this->m_isDart || !Mod::get()->getSettingValue<bool>("enable-wave")) {
            PlayerObject::pushButton(btn);
            return;
        }
        
        float cps = 0.0f;
        if (m_fields->m_clickTimer > 0.01f) {
            cps = 1.0f / m_fields->m_clickTimer;
        }
        
        if (cps >= 8.0f) {
            float curY = this->getPositionY();
            float newSlope = (curY - m_fields->m_lastY) / m_fields->m_clickTimer;
            if (m_fields->m_isSpamming) {
                m_fields->m_slope = (m_fields->m_slope * 0.5f) + (newSlope * 0.5f);
            } else {
                m_fields->m_slope = newSlope;
            }
            m_fields->m_isSpamming = true;
        }
        
        if (cps <= Mod::get()->getSettingValue<int64_t>("max-cps")) {
            PlayerObject::pushButton(btn);
        }
        
        m_fields->m_clickTimer = 0.0f;
        m_fields->m_lastY = this->getPositionY();
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!g_isModActive) return;
        
        m_fields->m_clickTimer += dt;
        if (m_fields->m_clickTimer > 0.3f) {
            m_fields->m_isSpamming = false;
        }
        
        if (this->m_isDart && m_fields->m_isSpamming && Mod::get()->getSettingValue<bool>("enable-wave")) {
            float strength = Mod::get()->getSettingValue<double>("lock-strength");
            this->m_yVelocity *= 0.7f;
            float newY = this->getPositionY() + (m_fields->m_slope * dt * strength);
            this->setPositionY(newY);
        }
        
        if (this->m_isShip && Mod::get()->getSettingValue<bool>("enable-ship")) {
            if (this->m_yVelocity > -5.0f && this->m_yVelocity < 5.0f) {
                this->m_yVelocity *= 0.93f;
            }
        }
    }
};
