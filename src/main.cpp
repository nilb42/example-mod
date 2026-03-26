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
        if ((!Mod::get()->getSettingValue<bool>("enable-wave") && !Mod::get()->getSettingValue<bool>("enable-ship")) || !g_isModActive) return true;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto warningText = CCLabelBMFont::create("smooth spam used", "chatFont.fnt");
        warningText->setAnchorPoint({1.0f, 0.0f}); 
        warningText->setPosition({winSize.width - 5.0f, 5.0f});
        warningText->setScale(0.5f);
        warningText->setOpacity(180);
        warningText->setColor({255, 50, 50});
        this->addChild(warningText);
        return true;
    }
};

class $modify(SmartSpamPlayer, PlayerObject) {
    float m_timeSinceLastClick = 0.0f;
    float m_currentCPS = 0.0f;
    float m_targetY = 0.0f;
    float m_lastClickY = 0.0f; 
    float m_spamSlope = 0.0f;  
    bool m_isSpamming = false;
    CCDrawNode* m_debugLine = nullptr; 

    void pushButton(PlayerButton p0) {
        if (!g_isModActive) { PlayerObject::pushButton(p0); return; }
        float currentY = this->getPositionY();
        float tempCPS = (m_fields->m_timeSinceLastClick > 0.005f) ? 1.0f / m_fields->m_timeSinceLastClick : 0.0f;
        if (this->m_isDart && Mod::get()->getSettingValue<bool>("enable-wave")) {
            if (tempCPS >= 8.0f) {
                if (!m_fields->m_isSpamming) { m_fields->m_targetY = currentY; m_fields->m_spamSlope = (currentY - m_fields->m_lastClickY) / m_fields->m_timeSinceLastClick; }
                else { m_fields->m_spamSlope = (m_fields->m_spamSlope * 0.6f) + ((currentY - m_fields->m_lastClickY) / m_fields->m_timeSinceLastClick * 0.4f); }
                m_fields->m_isSpamming = true;
            }
            if (tempCPS > Mod::get()->getSettingValue<int64_t>("max-cps")) return;
        }
        PlayerObject::pushButton(p0);
        m_fields->m_currentCPS = tempCPS;
        m_fields->m_timeSinceLastClick = 0.0f; 
        m_fields->m_lastClickY = currentY; 
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!g_isModActive) return;
        m_fields->m_timeSinceLastClick += dt;
        if (m_fields->m_timeSinceLastClick > 0.25f) { m_fields->m_isSpamming = false; m_fields->m_currentCPS = 0.0f; }
        if (this->m_isDart && Mod::get()->getSettingValue<bool>("enable-wave")) {
            if (m_fields->m_isSpamming) {
                m_fields->m_targetY += m_fields->m_spamSlope * dt;
                float currentY = this->getPositionY();
                float magnet = (0.1f + (m_fields->m_currentCPS * 0.005f)) * Mod::get()->getSettingValue<double>("lock-strength");
                if (std::abs(currentY - m_fields->m_targetY) < 60.0f) { this->m_yVelocity *= 0.6f; this->setPositionY(currentY + (m_fields->m_targetY - currentY) * std::min(magnet, 0.5f)); }
            }
        }
        if (this->m_isShip && Mod::get()->getSettingValue<bool>("enable-ship") && m_fields->m_currentCPS >= 5.0f && std::abs(this->m_yVelocity) < 4.0f) this->m_yVelocity *= 0.90f;
    }
};
