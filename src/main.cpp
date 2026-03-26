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
        
        // The "brain" analyzes CPS (Clicks Per Second)
        if (g_timeSinceClick < 0.2f) { // If clicking fast
            if (!g_isSpamming) {
                // Player started spamming. Memorize the starting height (invisible line)
                g_brainCenterY = this->getPositionY();
                g_isSpamming = true;
            } else {
                // Player continues spamming. Smoothly follow the player's center
                g_brainCenterY = (g_brainCenterY * 0.7f) + (this->getPositionY() * 0.3f);
            }
        }
        
        // Reset the timer on every click
        g_timeSinceClick = 0.0f; 
    }

    void update(float dt) {
        PlayerObject::update(dt);
        
        // Track time since the last click
        g_timeSinceClick += dt;

        // If the player intentionally stops clicking (waits more than 0.15 seconds)
        if (g_timeSinceClick > 0.15f) {
            g_isSpamming = false; // Disable the lock, allow natural falling
        }

        // Apply the invisible line correction ONLY if actively spamming
        if (g_isSpamming) {
            float currentY = this->getPositionY();
            float diff = g_brainCenterY - currentY;
            
            // If the player drifts more than 2 pixels from the invisible line
            if (std::abs(diff) > 2.0f) {
                // Take control and gently pull the player back to the invisible line
                this->setPositionY(currentY + diff * 0.1f);
            }
        }
    }
};
