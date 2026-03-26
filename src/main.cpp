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
        
        bool waveEnabled = Mod::get()->getSettingValue<bool>("enable-wave");
        bool shipEnabled = Mod::get()->getSettingValue<bool>("enable-ship");
        
        if ((!waveEnabled && !shipEnabled) || !g_isModActive) {
            return true;
        }

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
        if (!g_isModActive) {
            PlayerObject::pushButton(p0);
            return;
        }

        float tempCPS = 0.0f;
        float currentY = this->getPositionY();

        if (m_fields->m_timeSinceLastClick > 0.005f) {
            tempCPS = 1.0f / m_fields->m_timeSinceLastClick;
            float deltaY = currentY - m_fields->m_lastClickY;
            float newSlope = deltaY / m_fields->m_timeSinceLastClick;

            if (this->m_isDart && Mod::get()->getSettingValue<bool>("enable-wave")) {
                if (tempCPS >= 8.0f) {
                    if (!m_fields->m_isSpamming) {
                        m_fields->m_targetY = currentY; 
                        m_fields->m_spamSlope = newSlope; 
                    } else {
                        m_fields->m_spamSlope = (m_fields->m_spamSlope * 0.6f) + (newSlope * 0.4f);
                    }
                    m_fields->m_isSpamming = true;
                }
            }
        }

        int maxCpsLimit = Mod::get()->getSettingValue<int64_t>("max-cps");
        if (tempCPS > maxCpsLimit && this->m_isDart && Mod::get()->getSettingValue<bool>("enable-wave")) {
            return; 
        }

        PlayerObject::pushButton(p0);
        
        m_fields->m_currentCPS = tempCPS;
        m_fields->m_timeSinceLastClick = 0.0f; 
        m_fields->m_lastClickY = currentY; 
    }

    void update(float dt) {
        PlayerObject::update(dt);

        if (!g_isModActive) {
            if (m_fields->m_debugLine) m_fields->m_debugLine->clear();
            return;
        }

        m_fields->m_timeSinceLastClick += dt;

        if (m_fields->m_timeSinceLastClick > 0.25f) {
            m_fields->m_isSpamming = false;
            m_fields->m_currentCPS = 0.0f;
            m_fields->m_spamSlope = 0.0f;
        }

        if (this->m_isDart && Mod::get()->getSettingValue<bool>("enable-wave")) {
            bool showLines = Mod::get()->getSettingValue<bool>("show-lines");
            
            if (!m_fields->m_debugLine && showLines) {
                m_fields->m_debugLine = CCDrawNode::create();
                m_fields->m_debugLine->setZOrder(999); 
                this->addChild(m_fields->m_debugLine);
            }
            if (m_fields->m_debugLine) m_fields->m_debugLine->clear(); 

            if (m_fields->m_isSpamming) {
                m_fields->m_targetY += m_fields->m_spamSlope * dt;
                float currentY = this->getPositionY();
                float distanceToLine = std::abs(currentY - m_fields->m_targetY);
                
                float userLockStrength = Mod::get()->getSettingValue<double>("lock-strength");
                float dynamicCorridor = 60.0f - (m_fields->m_currentCPS * 2.0f);
                if (dynamicCorridor < 20.0f) dynamicCorridor = 20.0f; 

                float magnetStrength = (0.1f + (m_fields->m_currentCPS * 0.005f)) * userLockStrength;
                if (magnetStrength > 0.5f) magnetStrength = 0.5f; 
                
                if (distanceToLine < dynamicCorridor) {
                    this->m_yVelocity *= 0.6f; 
                    float smoothY = currentY + (m_fields->m_targetY - currentY) * magnetStrength;
                    this->setPositionY(smoothY);
                } else {
                    float curveAdaptation = 0.3f + (m_fields->m_currentCPS * 0.01f);
                    if (curveAdaptation > 0.8f) curveAdaptation = 0.8f;
                    m_fields->m_targetY = m_fields->m_targetY + (currentY - m_fields->m_targetY) * curveAdaptation;
                    m_fields->m_spamSlope *= 0.8f;
                }

                if (showLines && m_fields->m_debugLine) {
                    float relativeY = m_fields->m_targetY - currentY;
                    m_fields->m_debugLine->setRotation(-this->getRotation());
                    m_fields->m_debugLine->drawLine({-1000.0f, relativeY}, {1000.0f, relativeY}, {0.0f, 1.0f, 0.0f, 1.0f});
                }
            }
        } 
        else if (m_fields->m_debugLine) {
            m_fields->m_debugLine->clear();
        }

        if (this->m_isShip && Mod::get()->getSettingValue<bool>("enable-ship")) {
            if (m_fields->m_currentCPS >= 5.0f) {
                if (std::abs(this->m_yVelocity) < 4.0f) {
                    this->m_yVelocity *= 0.90f; 
                }
            }
        }
    }
};
