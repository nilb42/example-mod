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
                Notification::create("Smooth Spam: ON")->show();
            } else {
                Notification::create("Smooth Spam: OFF")->show();
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
    // Наша личная память для каждого игрока, которую игра не сможет спрятать
    struct Fields {
        bool isDart = false;
        bool isShip = false;
        float clickTimer = 0.0f;
        float lastY = 0.0f;
        float slope = 0.0f;
        bool isSpamming = false;
    };

    // --- НАЧАЛО ХИТРОСТИ ---
    // Слушаем портал волны
    void toggleDartMode(bool p0, bool p1) {
        PlayerObject::toggleDartMode(p0, p1);
        m_fields->isDart = p0;
        if (p0) m_fields->isShip = false;
    }

    // Слушаем портал корабля
    void toggleFlyMode(bool p0, bool p1) {
        PlayerObject::toggleFlyMode(p0, p1);
        m_fields->isShip = p0;
        if (p0) m_fields->isDart = false;
    }

    // Если коснулись любого другого портала — отключаем всё
    void toggleBirdMode(bool p0, bool p1) { PlayerObject::toggleBirdMode(p0, p1); if(p0) { m_fields->isDart = false; m_fields->isShip = false; } }
    void toggleRollMode(bool p0, bool p1) { PlayerObject::toggleRollMode(p0, p1); if(p0) { m_fields->isDart = false; m_fields->isShip = false; } }
    void toggleRobotMode(bool p0, bool p1) { PlayerObject::toggleRobotMode(p0, p1); if(p0) { m_fields->isDart = false; m_fields->isShip = false; } }
    void toggleSpiderMode(bool p0, bool p1) { PlayerObject::toggleSpiderMode(p0, p1); if(p0) { m_fields->isDart = false; m_fields->isShip = false; } }
    void toggleSwingMode(bool p0, bool p1) { PlayerObject::toggleSwingMode(p0, p1); if(p0) { m_fields->isDart = false; m_fields->isShip = false; } }
    // --- КОНЕЦ ХИТРОСТИ ---

    void pushButton(PlayerButton btn) {
        if (!g_isModActive || !m_fields->isDart || !Mod::get()->getSettingValue<bool>("enable-wave")) {
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
        
        if (m_fields->isDart && m_fields->isSpamming && Mod::get()->getSettingValue<bool>("enable-wave")) {
            float strength = Mod::get()->getSettingValue<double>("lock-strength");
            this->m_yVelocity *= 0.7f;
            float newY = this->getPositionY() + (m_fields->slope * dt * strength);
            this->setPositionY(newY);
        }
        
        if (m_fields->isShip && Mod::get()->getSettingValue<bool>("enable-ship")) {
            if (this->m_yVelocity > -5.0f && this->m_yVelocity < 5.0f) {
                this->m_yVelocity *= 0.93f;
            }
        }
    }
};
