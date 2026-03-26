#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

bool g_isModActive = true;

class $modify(MyUILayer, UILayer) {
    void keyDown(cocos2d::enumKeyCodes key) {
        UILayer::keyDown(key);
        if (key == cocos2d::enumKeyCodes::KEY_LeftBracket) {
            g_isModActive = !g_isModActive;
            if (g_isModActive) {
                Notification::create("Smooth Spam: ON")->show();
            } else {
                Notification::create("Smooth Spam: OFF")->show();
            }
        }
    }
};

class $modify(MyPlayer, PlayerObject) {
    struct Fields {
        float clickTimer = 0.0f;
        float lastY = 0.0f;
        float slope = 0.0f;
        bool isSpamming = false;
    };

    void pushButton(PlayerButton btn) {
        if (!g_isModActive || !this->m_isDart || !Mod::get()->getSettingValue<bool>("enable-wave")) {
            PlayerObject::pushButton(btn);
            return;
        }
        
        float cps = 0.0f;
        if (m_fields->clickTimer > 0.01f) {
            cps = 1.0f / m_fields->clickTimer;
        }
        
        if (cps >= 8.0f) {
            float curY = this->getPositionY();
            float newSlope = (curY - m_fields->lastY) / m_fields->clickTimer;
            if (m_fields->isSpamming) {
                m_fields->slope = (m_fields->slope * 0.5f) + (newSlope * 0.5f);
            } else {
                m_fields->slope = newSlope;
            }
            m_fields->isSpamming = true;
        }
        
        if (cps <= Mod::get()->getSettingValue<int64_t>("max-cps")) {
            PlayerObject::pushButton(btn);
        }
        
        m_fields->clickTimer = 0.0f;
        m_fields->lastY = this->getPositionY();
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!g_isModActive) return;
        
        m_fields->clickTimer += dt;
        if (m_fields->clickTimer > 0.3f) {
            m_fields->isSpamming = false;
        }
        
        if (this->m_isDart && m_fields->isSpamming && Mod::get()->getSettingValue<bool>("enable-wave")) {
            float strength = Mod::get()->getSettingValue<double>("lock-strength");
            this->m_yVelocity *= 0.7f;
            float newY = this->getPositionY() + (m_fields->slope * dt * strength);
            this->setPositionY(newY);
        }
        
        if (this->m_isShip && Mod::get()->getSettingValue<bool>("enable-ship")) {
            if (this->m_yVelocity > -5.0f && this->m_yVelocity < 5.0f) {
                this->m_yVelocity *= 0.93f;
            }
        }
    }
};
