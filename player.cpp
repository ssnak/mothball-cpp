#include "player.h"

#include <algorithm>
#include <cmath>
#include <optional>

void Player::move(int duration, std::optional<float> rotation, float rotationOffset, std::optional<float> slipperiness,
                  bool isSprinting, bool isSneaking, std::optional<int> speed, std::optional<int> slow, State state) {
    slipperiness = slipperiness.value_or(this->defaultGroundSlipperiness);
    speed = speed.value_or(this->speedEffect);
    slow = slow.value_or(this->slowEffect);
    bool overrideRotation = false;
    if (rotation.has_value()) {
        overrideRotation = true;
        rotation = rotation.value() + rotationOffset;
    }

    if (this->modifiers & (int)Modifiers::WATER) {
        slipperiness = 0.8f / 0.91f;
    } else if (this->modifiers & (int)Modifiers::LAVA) {
        slipperiness = 0.5f / 0.91f;
    }

    if (rotationOffset == 45) this->inputs = "wa";
    this->state = state;

    if (((this->sneakDelay && this->previouslySneaking) || (!this->sneakDelay && isSneaking)) &&
        this->modifiers & (int)Modifiers::LAVA)
        this->state = State::AIRBORNE;

    float sprintjumpBoost = this->sprintjumpBoost;
    if (this->reverse) {
        sprintjumpBoost *= -1;
    }

    for (int i = 0; i < duration; i++) {
        if (!overrideRotation) rotation = this->getAngle() + rotationOffset;
        this->position.add(this->velocity);

        if (this->modifiers & (int)Modifiers::SOULSAND) this->velocity.scale(0.4);

        Vector2<float> direction = this->movementValues();

        this->velocity.scale(0.91 * this->previousSlipperiness);

        if (this->inertiaAxis == 1) {
            if (std::fabs(this->velocity.x) < this->inertiaThreshold || this->previouslyInWeb) this->velocity.x = 0.0f;
            if (std::fabs(this->velocity.z) < this->inertiaThreshold || this->previouslyInWeb) this->velocity.z = 0.0f;
        } else if (this->inertiaAxis == 2) {
            // TODO: add multi axis inertia
        }

        if (this->state == State::JUMPING && isSprinting) {
            float facing = rotation.value() * 0.017453292f;
            this->velocity.x -= double(this->mcsin(facing) * sprintjumpBoost);
            this->velocity.x += double(this->mccos(facing) * sprintjumpBoost);
        }

        if (this->modifiers & (int)Modifiers::BLOCK) direction.scale(0.2f);
        if ((this->sneakDelay && this->previouslySneaking) || (!this->sneakDelay && isSneaking)) direction.scale(0.3f);
        direction.scale(0.98f);

        float multiplier = this->getMovementMultiplier(slipperiness.value(), isSprinting, speed.value(), slow.value());
        float distance = direction.sqrMagnitude();
        if (distance > 0.0f) {
            distance = std::sqrtf(distance);
            distance = std::max(distance, 1.0f);
            distance = multiplier / distance;
            direction.scale(distance);
            float sinYaw = this->mcsin(rotation.value() * PI / 180.0f);
            float cosYaw = this->mccos(rotation.value() * PI / 180.0f);
            this->velocity.x += direction.z * cosYaw - direction.x * sinYaw;
            this->velocity.z += direction.x * cosYaw + direction.z * sinYaw;
        }

        if (this->modifiers & (int)Modifiers::WEB) this->velocity.scale(0.25f);
        if (this->modifiers & (int)Modifiers::LADDER) {
            this->velocity.x = std::clamp(this->velocity.x, 0.15, -0.15);
            this->velocity.z = std::clamp(this->velocity.z, 0.15, -0.15);
        }

        this->previousSlipperiness = slipperiness.value();
        this->previouslySprinting = isSprinting;
        this->previouslySneaking = isSneaking;
        this->previouslyInWeb = this->modifiers & (int)Modifiers::WEB;
        this->lastTurn = rotation.value() - this->lastRotation;
        this->lastRotation = rotation.value();
    }
}

float Player::getMovementMultiplier(float slipperiness, bool isSprinting, int16_t speed, int16_t slow) {
    if (this->modifiers & (int)Modifiers::WATER || this->modifiers & (int)Modifiers::LAVA) {
        return 0.02;
    } else if (this->state == State::AIRBORNE) {
        if ((this->airSprintDelay && this->previouslySprinting) || (!this->airSprintDelay && isSprinting)) {
            return 0.02f * 0.3f * 2.0f;

        } else {
            return 0.0f;
        }
    } else {
        float multiplier = 0.1f;

        if (speed > 0) multiplier *= 1.0f + (0.2f * speed);
        if (slow > 0) multiplier *= 1.0f + std::fmax(-0.15f * speed, 0.0f);
        if (isSprinting) multiplier *= 1.3f;
        float drag = 0.91f * slipperiness;
        return multiplier * (0.16277136f / (drag * drag * drag));
    }
}
