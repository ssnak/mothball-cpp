#include <bits/ostream.h>
#include <sys/types.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>

#include "vector.h"

#pragma once

struct Args {
    std::optional<float> rotation;
    std::optional<float> slipperiness;
    std::optional<int> speed;
    std::optional<int> slow;
    Args() = default;
    Args(float rotation) : rotation(rotation) {}
};

constexpr double PI = 3.14159265358979323846;
inline std::array<float, 65536> makeSinTable() {
    std::array<float, 65536> table{};

    for (size_t i = 0; i < table.size(); i++) {
        table[i] = static_cast<float>(std::sin(PI * 2.0 * i / 65536.0));
    }
    return table;
}
class Player {
   private:
    inline static const std::array<float, 65536> SIN_TABLE = makeSinTable();
    enum class State { JUMPING, GROUNDED, AIRBORNE };
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
    void move(int duration, std::optional<float> rotation, float rotationOffset, std::optional<float> slipperiness,
              bool isSprinting, bool isSneaking, std::optional<int> speed, std::optional<int> slow, State state);
    void update(bool overrideRotation, float rotationOffset, bool isSprinting, bool isSneaking, float slipperiness,
                float rotation, int speed, int slow, float sprintjumpBoost);
    bool hasModifier(Modifiers modifier) {
        return static_cast<u_int32_t>(m_modifiers) & static_cast<u_int32_t>(modifier);
    }

    float getAngle() {
        // TODO: Implement getAngle
        return m_rotation;
    }

    // x: forward, z: strafe
    Vector2<float> movementValues() {
        if (this->m_reverse) {
            return Vector2<float>{-1.0f, 0.0f};
        }
        return Vector2<float>{1.0f, 0.0f};
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
            playerCopy.sneaksprintjump(1, Args{});
        } else {
            playerCopy.sprintjump(1, Args{});
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
    void walk(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, args.slipperiness, false, false, args.speed, args.slow,
                   State::GROUNDED);
    }

    void walk45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, args.slipperiness, false, false, args.speed, args.slow,
                   State::GROUNDED);
    }

    void sprint(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, args.slipperiness, true, false, args.speed, args.slow,
                   State::GROUNDED);
    }

    void sprint45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, args.slipperiness, true, false, args.speed, args.slow,
                   State::GROUNDED);
    }

    void walkair(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void walkair45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void sprintair(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, 1.0, true, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void sprintair45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, 1.0f, true, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void walkjump(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 0.0f, args.slipperiness, false, false, args.speed, args.slow, State::JUMPING);
            this->walkair(duration - 1, args);
        }
    }

    void walkjump45(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 45.0f, args.slipperiness, false, false, args.speed, args.slow, State::JUMPING);
            this->walkair45(duration - 1, args);
        }
    }

    void walkpessi(int duration, int delay, Args args) {
        if (delay == 0) {
            this->walkjump(duration, args);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, args);
            this->inputs = input;
            this->walkair(duration - delay, args);
        }
    }

    void walkpessi45(int duration, int delay, Args args) {
        if (delay == 0) {
            this->walkjump45(duration, args);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, args);
            this->inputs = input;
            this->walkair45(duration - delay, args);
        }
    }

    void sprintjump(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 0.0f, args.slipperiness, true, false, args.speed, args.slow, State::JUMPING);
            this->sprintair(duration - 1, args);
        }
    }

    void sprintjump45(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 0.0f, args.slipperiness, true, false, args.speed, args.slow, State::JUMPING);
            this->sprintair45(duration - 1, args);
        }
    }

    void sprintstrafejump(int duration, Args args) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, args.rotation,
                       this->getOptimalStrafeJumpAngle(args.speed, args.slow, args.slipperiness, false),
                       args.slipperiness, true, false, args.speed, args.slow, State::JUMPING);
            this->inputs = "w";
            this->sprintair(duration - 1, args);
        }
    }
    void sprintstrafejump45(int duration, Args args) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, args.rotation,
                       this->getOptimalStrafeJumpAngle(args.speed, args.slow, args.slipperiness, false),
                       args.slipperiness, true, false, args.speed, args.slow, State::JUMPING);
            this->sprintair45(duration - 1, args);
        }
    }
    void sprintpessi(int duration, int delay, Args args) {
        if (delay == 0) {
            this->sprintjump(duration, args);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, args);
            this->inputs = input;
            this->sprintair(duration - delay, args);
        }
    }
    void sprintpessi45(int duration, int delay, Args args) {
        if (delay == 0) {
            this->sprintjump(duration, args);
        } else if (duration > 0) {
            if (delay > duration) {
                delay = duration;
            }
            std::string input = this->inputs;
            this->inputs = "";
            this->stopjump(delay, args);
            this->inputs = input;
            this->sprintair45(duration - delay, args);
        }
    }
    void forcemomentum(int duration, int delay, Args args) {
        if (delay < 0) {
            return;  // raise error;
        } else if (duration > 0) {
            if (delay == 0) this->sprintjump(duration, args);
        } else {
            if (delay > duration)  // example: fmm(12,15) is just wj(12);
                delay = duration;
        }
        this->walkjump(delay, args);
        this->sprintair(duration - delay, args);
    }
    void forcemomentum45(int duration, int delay, Args args) {
        if (delay < 0) {
            return;  // raise error;
        } else if (duration > 0) {
            if (delay == 0) this->sprintjump45(duration, args);
        } else {
            if (delay > duration)  // example: fmm(12,15) is just wj(12);
                delay = duration;
        }
        this->walkjump45(delay, args);
        this->sprintair45(duration - delay, args);
    }
    void sneak(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, args.slipperiness, false, true, args.speed, args.slow,
                   State::GROUNDED);
    }
    void sneak45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, args.slipperiness, false, true, args.speed, args.slow,
                   State::GROUNDED);
    }
    void sneakair(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakair45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakjump(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 0.0f, args.slipperiness, false, true, args.speed, args.slow, State::JUMPING);
            this->sneakair(duration - 1, args);
        }
    }
    void sneakjump45(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 45.0f, args.slipperiness, false, true, args.speed, args.slow, State::JUMPING);
            this->sneakair45(duration - 1, args);
        }
    }
    void stop(int duration, Args args) {
        this->move(duration, std::nullopt, 0.0f, args.slipperiness, false, false, std::nullopt, std::nullopt,
                   State::GROUNDED);
    }
    void stopair(int duration, Args _) {
        this->move(duration, std::nullopt, 0.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void stopjump(int duration, Args args) {
        if (duration > 0) {
            this->move(1, std::nullopt, 0.0f, args.slipperiness, false, false, std::nullopt, std::nullopt,
                       State::JUMPING);
            this->stopair(duration - 1, args);
        }
    }
    void sneakstop(int duration, Args args) {
        this->move(duration, std::nullopt, 0.0f, args.slipperiness, false, true, std::nullopt, std::nullopt,
                   State::GROUNDED);
    }
    void sneakstopair(int duration, Args _) {
        this->move(duration, std::nullopt, 0.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakstopjump(int duration, Args args) {
        if (duration > 0) {
            this->move(1, std::nullopt, 0.0f, args.slipperiness, false, true, std::nullopt, std::nullopt,
                       State::JUMPING);
            this->stopair(duration - 1, args);
        }
    }
    void sneaksprint(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, args.slipperiness, true, true, args.speed, args.slow,
                   State::GROUNDED);
    }
    void sneaksprint45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, args.slipperiness, true, true, args.speed, args.slow,
                   State::GROUNDED);
    }
    void sneaksprintair(int duration, Args args) {
        this->move(duration, args.rotation, 0.0f, 1.0f, true, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneaksprintair45(int duration, Args args) {
        this->move(duration, args.rotation, 45.0f, 1.0f, true, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneaksprintjump(int duration, Args args) {
        if (duration > 0) {
            this->move(1, args.rotation, 0.0f, args.slipperiness, true, true, args.speed, args.slow, State::JUMPING);
            this->sneaksprintair(duration - 1, args);
        }
    }
    void sneaksprintjump45(int duration, Args args) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, args.rotation,
                       this->getOptimalStrafeJumpAngle(args.speed, args.slow, args.slipperiness, true),
                       args.slipperiness, true, true, args.speed, args.slow, State::JUMPING);
            this->sneaksprintair45(duration - 1, args);
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Player& p);
