#pragma once

#include <bits/ostream.h>
#include <sys/types.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <ostream>
#include <queue>
#include <string>

#include "vector.h"

constexpr double PI = 3.14159265358979323846;
inline std::array<float, 65536> makeSinTable() {
    std::array<float, 65536> table{};

    for (size_t i = 0; i < table.size(); i++) {
        table[i] = static_cast<float>(std::sin(PI * 2.0 * i / 65536.0));
    }
    return table;
}
enum class State { JUMPING, GROUNDED, AIRBORNE };

class Player {
   private:
    inline static const std::array<float, 65536> SIN_TABLE = makeSinTable();
    State m_state = State::JUMPING;
    enum class Modifiers : u_int32_t {
        NONE = 0,
        WATER = 1,
        LAVA = 1 << 1,
        WEB = 1 << 2,
        BLOCK = 1 << 3,
        LADDER = 1 << 4,
        SOULSAND = 1 << 5
    };
    const float m_sprintjumpBoost = 0.2f;
    const float m_inertiaThreshold = 0.005;
    float m_defaultGroundSlipperiness = 0.6f;
    float m_rotation = 0.0f;
    float m_lastRotation = 0.0f;
    float m_lastTurn = 0.0f;
    // TODO: add angle queue, and turn queue
    std::queue<float> m_angles;
    bool m_airSprintDelay = true;
    bool m_sneakDelay = false;
    int8_t m_inertiaAxis = 1;
    // TODO: add record, history, macros, and record inertia
    int16_t m_speedEffect = 0;
    int16_t m_slowEffect = 0;
    Modifiers m_modifiers = Modifiers::NONE;
    bool m_reverse = false;
    float m_previousSlipperiness = 0.6f;
    bool m_previouslySneaking = false;
    bool m_previouslyInWeb = false;
    bool m_previouslySprinting = false;

   private:
    void update(bool overrideRotation, float rotationOffset, bool isSprinting, bool isSneaking, float& slipperiness,
                float rotation, int speed, int slow, float sprintjumpBoost);
    bool hasModifier(Modifiers modifier) {
        return static_cast<u_int32_t>(m_modifiers) & static_cast<u_int32_t>(modifier);
    }

    // x: forward, z: strafe
    Vector2<float> movementValues() {
        Vector2<float> movement{0.0f, 0.0f};
        if (inputs.find("w") != inputs.npos) {
            movement.x = 1.0f;
        } else if (inputs.find("s") != inputs.npos) {
            movement.x = -1.0f;
        }
        if (inputs.find("a") != inputs.npos) {
            movement.z = 1.0f;
        } else if (inputs.find("d") != inputs.npos) {
            movement.z = -1.0f;
        }
        if (this->m_reverse) {
            movement.scale(-1.0f);
        }
        return movement;
    }
    float getMovementMultiplier(float slipperiness, bool isSprinting, int16_t speed, int16_t slow);
    float getOptimalStrafeJumpAngle(std::optional<int> speed, std::optional<int> slow,
                                    std::optional<float> slipperiness, bool isSneaking) {
        Player playerCopy = *this;
        playerCopy.position.x = 0.0;
        playerCopy.position.z = 0.0;
        playerCopy.velocity.x = 0.0;
        playerCopy.velocity.z = 0.0;
        playerCopy.m_rotation = 0.0f;
        if (speed.has_value()) {
            playerCopy.m_speedEffect = speed.value();
        }
        if (slow.has_value()) {
            playerCopy.m_slowEffect = slow.value();
        }
        if (slipperiness.has_value()) {
            playerCopy.m_defaultGroundSlipperiness = slipperiness.value();
        }
        playerCopy.inputs = "";
        if (isSneaking) {
            playerCopy.sneaksprintjump(1, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        } else {
            playerCopy.sprintjump(1, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        }
        return std::fabs(180.0 * std::atan2(playerCopy.velocity.x, playerCopy.velocity.z) / PI);
    }

    float mcsin(float radians) {
        // 10430.378f comes from 65536 / (2.0 * PI)
        return SIN_TABLE[static_cast<int>(radians * 10430.378f) & 0xffff];
    }

    float mccos(float radians) { return SIN_TABLE[static_cast<int>(radians * 10430.378f + 16384.0f) & 0xffff]; }

   public:
    Vector2<double> position = {0.0, 0.0};
    Vector2<double> velocity = {0.0, 0.0};
    std::string inputs;
    bool stepExecution = false;
    int precision = 7;

   public:
    void move(int duration, std::optional<float> rotation, float rotationOffset, std::optional<float> slipperiness,
              bool isSprinting, bool isSneaking, std::optional<int> speed, std::optional<int> slow, State state);

    float getAngle() {
        // TODO: Implement getAngle
        if (!m_angles.empty()) {
            m_rotation = m_angles.front();
            m_angles.pop();
        }
        return m_rotation;
    }
    void face(float angle) { m_rotation = angle; }

    void walk(int duration = 1, std::optional<float> rotation = std::nullopt,
              std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
              std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, false, false, speed, slow, State::GROUNDED);
    }

    void walk45(int duration = 1, std::optional<float> rotation = std::nullopt,
                std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 45.0f, slipperiness, false, false, speed, slow, State::GROUNDED);
    }

    void sprint(int duration = 1, std::optional<float> rotation = std::nullopt,
                std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, true, false, speed, slow, State::GROUNDED);
    }

    void sprint45(int duration = 1, std::optional<float> rotation = std::nullopt,
                  std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                  std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 45.0f, slipperiness, true, false, speed, slow, State::GROUNDED);
    }

    void walkair(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 0.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void walkair45(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 45.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void sprintair(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 0.0f, 1.0, true, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void sprintair45(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 45.0f, 1.0f, true, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void walkjump(int duration = 1, std::optional<float> rotation = std::nullopt,
                  std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                  std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, false, false, speed, slow, State::JUMPING);
    }

    void walkjump45(int duration = 1, std::optional<float> rotation = std::nullopt,
                    std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                    std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 45.0f, slipperiness, false, false, speed, slow, State::JUMPING);
    }

    void walkpessi(int duration = 1, int delay = 1, std::optional<float> rotation = std::nullopt,
                   std::optional<float> slipperiness = std::nullopt) {
        if (delay == 0) {
            this->walkjump(duration, rotation, slipperiness, std::nullopt, std::nullopt);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, slipperiness);
            this->inputs = input;
            this->walkair(duration - delay, rotation);
        }
    }

    void walkpessi45(int duration = 1, int delay = 1, std::optional<float> rotation = std::nullopt,
                     std::optional<float> slipperiness = std::nullopt) {
        if (delay == 0) {
            this->walkjump45(duration, rotation, slipperiness, std::nullopt, std::nullopt);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, slipperiness);
            this->inputs = input;
            this->walkair45(duration - delay, rotation);
        }
    }

    void sprintjump(int duration = 1, std::optional<float> rotation = std::nullopt,
                    std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                    std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, true, false, speed, slow, State::JUMPING);
    }

    void sprintjump45(int duration = 1, std::optional<float> rotation = std::nullopt,
                      std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                      std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, true, false, speed, slow, State::JUMPING);
    }

    void sprintstrafejump(int duration = 1, std::optional<float> rotation = std::nullopt,
                          std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                          std::optional<int> slow = std::nullopt) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, rotation, this->getOptimalStrafeJumpAngle(speed, slow, slipperiness, false), slipperiness,
                       true, false, speed, slow,
                       State::JUMPING);  // TODO: This again
            this->inputs = "w";
            this->sprintair(duration - 1, rotation);
        }
    }
    void sprintstrafejump45(int duration = 1, std::optional<float> rotation = std::nullopt,
                            std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                            std::optional<int> slow = std::nullopt) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, rotation, this->getOptimalStrafeJumpAngle(speed, slow, slipperiness, false), slipperiness,
                       true, false, speed, slow,
                       State::JUMPING);  // TODO: Check what boolean is for and
                                         // getOptimalStrafeJumpAngle implementation
            this->sprintair45(duration - 1, rotation);
        }
    }
    void sprintpessi(int duration = 1, int delay = 1, std::optional<float> rotation = std::nullopt,
                     std::optional<float> slipperiness = std::nullopt) {
        if (delay == 0) {
            this->sprintjump(duration, rotation, slipperiness, std::nullopt, std::nullopt);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, slipperiness);
            this->inputs = input;
            this->sprintair(duration - delay, rotation);
        }
    }
    void sprintpessi45(int duration = 1, int delay = 1, std::optional<float> rotation = std::nullopt,
                       std::optional<float> slipperiness = std::nullopt) {
        if (delay == 0) {
            this->sprintjump(duration, rotation, slipperiness, std::nullopt, std::nullopt);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, slipperiness);
            this->inputs = input;
            this->sprintair45(duration - delay, rotation);
        }
    }
    void forcemomentum(int duration = 1, int delay = 1, std::optional<float> rotation = std::nullopt,
                       std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                       std::optional<int> slow = std::nullopt) {
        if (delay < 0) {
            return;  // raise error;
        } else if (duration > 0) {
            if (delay == 0) this->sprintjump(duration, rotation, slipperiness, speed, slow);
        } else {
            if (delay > duration)  // example: fmm(12,15) is just wj(12);
                delay = duration;
        }
        this->walkjump(delay, rotation, slipperiness, speed, slow);
        this->sprintair(duration - delay, rotation);
    }
    void forcemomentum45(int duration = 1, int delay = 1, std::optional<float> rotation = std::nullopt,
                         std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                         std::optional<int> slow = std::nullopt) {
        if (delay < 0) {
            return;  // raise error;
        } else if (duration > 0) {
            if (delay == 0) this->sprintjump45(duration, rotation, slipperiness, speed, slow);
        } else {
            if (delay > duration)  // example: fmm(12,15) is just wj(12);
                delay = duration;
        }
        this->walkjump45(delay, rotation, slipperiness, speed, slow);
        this->sprintair45(duration - delay, rotation);
    }
    void sneak(int duration = 1, std::optional<float> rotation = std::nullopt,
               std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
               std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, false, true, speed, slow, State::GROUNDED);
    }
    void sneak45(int duration = 1, std::optional<float> rotation = std::nullopt,
                 std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                 std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 45.0f, slipperiness, false, true, speed, slow, State::GROUNDED);
    }
    void sneakair(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 0.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakair45(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 45.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakjump(int duration = 1, std::optional<float> rotation = std::nullopt,
                   std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                   std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, false, true, speed, slow, State::JUMPING);
    }
    void sneakjump45(int duration = 1, std::optional<float> rotation = std::nullopt,
                     std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                     std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 45.0f, slipperiness, false, true, speed, slow, State::JUMPING);
    }
    void stop(int duration = 1, std::optional<float> slipperiness = std::nullopt) {
        this->move(duration, std::nullopt, 0.0f, slipperiness, false, false, std::nullopt, std::nullopt,
                   State::GROUNDED);
    }
    void stopair(int duration = 1) {
        this->move(duration, std::nullopt, 0.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void stopjump(int duration = 1, std::optional<float> slipperiness = std::nullopt) {
        this->move(duration, std::nullopt, 0.0f, slipperiness, false, false, std::nullopt, std::nullopt,
                   State::JUMPING);
    }
    void sneakstop(int duration = 1, std::optional<float> slipperiness = std::nullopt) {
        this->move(duration, std::nullopt, 0.0f, slipperiness, false, true, std::nullopt, std::nullopt,
                   State::GROUNDED);
    }
    void sneakstopair(int duration = 1) {
        this->move(duration, std::nullopt, 0.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakstopjump(int duration = 1, std::optional<float> slipperiness = std::nullopt) {
        this->move(duration, std::nullopt, 0.0f, slipperiness, false, true, std::nullopt, std::nullopt, State::JUMPING);
    }
    void sneaksprint(int duration = 1, std::optional<float> rotation = std::nullopt,
                     std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                     std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, true, true, speed, slow, State::GROUNDED);
    }
    void sneaksprint45(int duration = 1, std::optional<float> rotation = std::nullopt,
                       std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                       std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 45.0f, slipperiness, true, true, speed, slow, State::GROUNDED);
    }
    void sneaksprintair(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 0.0f, 1.0f, true, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneaksprintair45(int duration = 1, std::optional<float> rotation = std::nullopt) {
        this->move(duration, rotation, 45.0f, 1.0f, true, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneaksprintjump(int duration = 1, std::optional<float> rotation = std::nullopt,
                         std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                         std::optional<int> slow = std::nullopt) {
        this->move(duration, rotation, 0.0f, slipperiness, true, true, speed, slow, State::JUMPING);
    }
    void sneaksprintjump45(int duration = 1, std::optional<float> rotation = std::nullopt,
                           std::optional<float> slipperiness = std::nullopt, std::optional<int> speed = std::nullopt,
                           std::optional<int> slow = std::nullopt) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, rotation, this->getOptimalStrafeJumpAngle(speed, slow, slipperiness, true), slipperiness,
                       true, true, speed, slow, State::JUMPING);
            this->sneaksprintair45(duration - 1, rotation);
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Player& p);
