#include <cmath>
#include <cstdint>
#include <optional>
#include <string>

#include "vector.h"

#define PI 3.14159265358979323846
class Player {
   public:
    enum class State { JUMPING, GROUNDED, AIRBORNE };
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    float defaultGroundSlipperiness = 0.6f;
    int angles = 65536;
    float rotation = 0.0f;
    float lastRotation = 0.0f;
    float lastTurn = 0.0f;
    // TODO: add angle queue, and turn queue
    bool airSprintDelay = true;
    bool sneakDelay = false;
    int8_t inertiaAxis = 1;
    std::string inputs;
    State state = State::JUMPING;
    // TODO: add record, history, macros, and record inertia
    int16_t speedEffect = 0;
    int16_t slowEffect = 0;
    int modifiers = 0;
    float sprintjumpBoost = 0.2f;
    bool reverse = false;
    float inertiaThreshold = 0.005;
    std::optional<float> previousSlipperiness = std::nullopt;
    bool previouslySneaking = false;
    bool previouslyInWeb = false;
    bool previouslySprinting = false;

    enum class Modifiers : int {
        WATER = 1,
        LAVA = 1 << 1,
        WEB = 1 << 2,
        BLOCK = 1 << 3,
        LADDER = 1 << 4,
        SOULSAND = 1 << 5
    };

    void move(int duration, std::optional<float> rotation, float rotationOffset, std::optional<float> slipperiness,
              bool isSprinting, bool isSneaking, std::optional<int> speed, std::optional<int> slow, State state);

    float getAngle() {
        // TODO: Implement getAngle
        return 0.0f;
    }

    // x: forward, z: strafe
    Vector2 movementValues() { return Vector2{1.0f, 0.0f}; }
    float getMovementMultiplier(float slipperiness, bool isSprinting, int16_t speed, int16_t slow);
    float getOptimalStrafeJumpAngle(std::optional<int> speed, std::optional<int> slow,
                                    std::optional<float> slipperiness, bool dummyForNow) {
        return 0.0f;
    }

    float mcsin(float rad) {
        int index;
        if (angles == -1) {
            return std::sinf(rad);
        } else if (angles == 65536) {
            index = int(rad * 10430.378) & 65535;
        } else {
            index = int(1 / (2 * PI) * angles * rad) & (angles - 1);
        }
        return std::sinf(index * PI * 2.0f / angles);
    }

    float mccos(float rad) {
        int index;
        if (angles == -1) {
            return std::cosf(rad);
        } else if (angles == 65536) {
            index = int(rad * 10430.378 + 16384.0) & 65535;
        } else {
            index = int(1 / (2 * PI) * angles * rad + angles / 4) & (angles - 1);
        }
        return std::sinf(index * PI * 2.0 / angles);
    }

    void walk(int duration, std::optional<float> rotation, std::optional<float> slipperiness, std::optional<int> speed,
              std::optional<int> slow) {
        this->move(duration, rotation, 0.0f, slipperiness, false, false, speed, slow, State::GROUNDED);
    }

    void walk45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                std::optional<int> speed, std::optional<int> slow) {
        this->move(duration, rotation, 45.0f, slipperiness, false, false, speed, slow, State::GROUNDED);
    }

    void sprint(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                std::optional<int> speed, std::optional<int> slow) {
        this->move(duration, rotation, 0.0f, slipperiness, true, false, speed, slow, State::GROUNDED);
    }

    void sprint45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                  std::optional<int> speed, std::optional<int> slow) {
        this->move(duration, rotation, 45.0f, slipperiness, true, false, speed, slow, State::GROUNDED);
    }

    void walkair(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 45.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void walkair45(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 45.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void sprintair(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 45.0f, 1.0, true, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void sprintair45(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 45.0f, 1.0f, true, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }

    void walkjump(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                  std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 45.0f, slipperiness, false, false, speed, slow, State::JUMPING);
            this->walkair(duration - 1, rotation);
        }
    }

    void walkjump45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                    std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 45.0f, slipperiness, false, false, speed, slow, State::JUMPING);
            this->walkair45(duration - 1, rotation);
        }
    }

    void walkpessi(int duration, int delay, std::optional<float> rotation, std::optional<float> slipperiness) {
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

    void walkpessi45(int duration, int delay, std::optional<float> rotation, std::optional<float> slipperiness) {
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

    void sprintjump(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                    std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 0.0f, slipperiness, true, false, speed, slow, State::JUMPING);
            this->sprintair(duration - 1, rotation);
        }
    }

    void sprintjump45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                      std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 0.0f, slipperiness, true, false, speed, slow, State::JUMPING);
            this->sprintair45(duration - 1, rotation);
        }
    }

    void sprintstrafejump(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                          std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, rotation, this->getOptimalStrafeJumpAngle(speed, slow, slipperiness, false), slipperiness,
                       true, false, speed, slow,
                       State::JUMPING);  // TODO: This again
            this->inputs = "w";
            this->sprintair(duration - 1, rotation);
        }
    }
    void sprintstrafejump45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                            std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, rotation, this->getOptimalStrafeJumpAngle(speed, slow, slipperiness, false), slipperiness,
                       true, false, speed, slow,
                       State::JUMPING);  // TODO: Check what boolean is for and
                                         // getOptimalStrafeJumpAngle implementation
            this->sprintair45(duration - 1, rotation);
        }
    }
    void sprintpessi(int duration, int delay, std::optional<float> rotation, std::optional<float> slipperiness) {
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
    void sprintpessi45(int duration, int delay, std::optional<float> rotation, std::optional<float> slipperiness) {
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
    void forcemomentum(int duration, int delay, std::optional<float> rotation, std::optional<float> slipperiness,
                       std::optional<int> speed, std::optional<int> slow) {
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
    void forcemomentum45(int duration, int delay, std::optional<float> rotation, std::optional<float> slipperiness,
                         std::optional<int> speed, std::optional<int> slow) {
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
    void sneak(int duration, std::optional<float> rotation, std::optional<float> slipperiness, std::optional<int> speed,
               std::optional<int> slow) {
        this->move(duration, rotation, 0.0f, slipperiness, false, true, speed, slow, State::GROUNDED);
    }
    void sneak45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                 std::optional<int> speed, std::optional<int> slow) {
        this->move(duration, rotation, 45.0f, slipperiness, false, true, speed, slow, State::GROUNDED);
    }
    void sneakair(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 0.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakair45(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 45.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakjump(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                   std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 0.0f, slipperiness, false, true, speed, slow, State::JUMPING);
            this->sneakair(duration - 1, rotation);
        }
    }
    void sneakjump45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                     std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 45.0f, slipperiness, false, true, speed, slow, State::JUMPING);
            this->sneakair45(duration - 1, rotation);
        }
    }
    void stop(int duration, std::optional<float> slipperiness) {
        this->move(duration, std::nullopt, 0.0f, slipperiness, false, false, std::nullopt, std::nullopt,
                   State::GROUNDED);
    }
    void stopair(int duration) {
        this->move(duration, std::nullopt, 0.0f, 1.0f, false, false, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void stopjump(int duration, std::optional<float> slipperiness) {
        if (duration > 0) {
            this->move(1, std::nullopt, 0.0f, slipperiness, false, false, std::nullopt, std::nullopt, State::JUMPING);
            this->stopair(duration - 1);
        }
    }
    void sneakstop(int duration, std::optional<float> slipperiness) {
        this->move(duration, std::nullopt, 0.0f, slipperiness, false, true, std::nullopt, std::nullopt,
                   State::GROUNDED);
    }
    void sneakstopair(int duration) {
        this->move(duration, std::nullopt, 0.0f, 1.0f, false, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneakstopjump(int duration, std::optional<float> slipperiness) {
        if (duration > 0) {
            this->move(1, std::nullopt, 0.0f, slipperiness, false, true, std::nullopt, std::nullopt, State::JUMPING);
            this->stopair(duration - 1);
        }
    }
    void sneaksprint(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                     std::optional<int> speed, std::optional<int> slow) {
        this->move(duration, rotation, 0.0f, slipperiness, true, true, speed, slow, State::GROUNDED);
    }
    void sneaksprint45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                       std::optional<int> speed, std::optional<int> slow) {
        this->move(duration, rotation, 45.0f, slipperiness, true, true, speed, slow, State::GROUNDED);
    }
    void sneaksprintair(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 0.0f, 1.0f, true, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneaksprintair45(int duration, std::optional<float> rotation) {
        this->move(duration, rotation, 45.0f, 1.0f, true, true, std::nullopt, std::nullopt, State::AIRBORNE);
    }
    void sneaksprintjump(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                         std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->move(1, rotation, 0.0f, slipperiness, true, true, speed, slow, State::JUMPING);
            this->sneaksprintair(duration - 1, rotation);
        }
    }
    void sneaksprintjump45(int duration, std::optional<float> rotation, std::optional<float> slipperiness,
                           std::optional<int> speed, std::optional<int> slow) {
        if (duration > 0) {
            this->inputs = "wa";
            this->move(1, rotation, this->getOptimalStrafeJumpAngle(speed, slow, slipperiness, true), slipperiness,
                       true, true, speed, slow, State::JUMPING);
            this->sneaksprintair45(duration - 1, rotation);
        }
    }
};
