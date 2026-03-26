#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// Global "brain" memory for the invisible line
float g_timeSinceClick = 0.0f;
float g_brainCenterY = 0.0f;
bool g_isSpamming = false;

class $modify(MyPlayer, PlayerObject) {
    
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
        
        // The "brain" analyzes CPS
        if (g_timeSinceClick < 0.2f) { 
            if (!g_isSpamming) {
                g_brainCenterY = this->getPositionY();
                g_isSpamming = true;
            } else {
                g_brainCenterY = (g_brainCenterY * 0.7f) + (this->getPositionY() * 0.3f);
            }
        }
        
        g_timeSinceClick = 0.0f; 
    }

    void update(float dt) {
        PlayerObject::update(dt);
        
        g_timeSinceClick += dt;

        if (g_timeSinceClick > 0.15f) {
            g_isSpamming = false; 
        }

        if (g_isSpamming) {
            float currentY = this->getPositionY();
            float diff = g_brainCenterY - currentY;
            
            // ПРОСТАЯ МАТЕМАТИКА (никаких abs)
            if (diff > 2.0f || diff < -2.0f) {
                this->setPositionY(currentY + diff * 0.1f);
            }
        }
    }
};
