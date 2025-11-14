#include <iostream>

#include "player.h"

struct StratFinder {
    Player player;
    Vector2<double> size{1.6f, 1.6f};
    float angleMin = 0.0f;
    float angleMax = -45.0f;
    bool canJump = true;
    int airtime = 12;
    float rotation = 0.0f;
    int duration = 0;
    bool left = true;

    void findStrat() {
        player.inputs = "w";
        int tick = 0;
        while (std::fabs(player.position.z) <= size.z) {
            player.sprint(1, Args{rotation});
            tick++;
        }
        float predictedAngle = std::acos(size.z / player.position.z);
        int angleIndex = static_cast<int>(predictedAngle * 10430.378f) & 65535;
        predictedAngle *= 180.0 / PI;
        reset();
        player.sprint(tick, Args{predictedAngle});
        duration = tick;
        if (left) angleIndex = -angleIndex;
        std::cout << angleIndex << std::endl;
        if (std::fabs(player.position.z) < size.z) {
            reset();
            rotation = 360.0 * angleIndex / 65536.0;
            player.sprint(duration, Args{rotation});
        }
        int counter = 0;
        while (std::fabs(player.position.z) >= size.z) {
            if (left) {
                angleIndex--;
            } else {
                angleIndex++;
            }
            reset();
            rotation = 360.0 * angleIndex / 65536.0;
            player.sprint(duration, Args{rotation});
            counter++;
        }
        std::cout << "hit: " << counter << std::endl;
    }

   private:
    void reset() {
        player.velocity.x = 0.0;
        player.velocity.z = 0.0;
        player.position.x = 0.0;
        player.position.z = 0.0;
    }
};
