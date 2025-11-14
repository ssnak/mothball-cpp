#include "player.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <optional>

void Player::move(int duration, std::optional<float> rotation, float rotationOffset, std::optional<float> slipperiness,
                  bool isSprinting, bool isSneaking, std::optional<int> speedEffect, std::optional<int> slowEffect,
                  State state) {
    slipperiness = slipperiness.value_or(m_defaultGroundSlipperiness);
    speedEffect = speedEffect.value_or(m_speedEffect);
    slowEffect = slowEffect.value_or(m_slowEffect);
    bool overrideRotation = false;
    if (rotation.has_value()) {
        overrideRotation = true;
        rotation = rotation.value() + rotationOffset;
    } else {
        rotation = 0.0f;
    }

    if (this->hasModifier(Modifiers::WATER)) {
        slipperiness = 0.8f / 0.91f;
    } else if (this->hasModifier(Modifiers::LAVA)) {
        slipperiness = 0.5f / 0.91f;
    }

    if (rotationOffset == 45) this->inputs = "wa";
    m_state = state;

    if (((m_sneakDelay && m_previouslySneaking) || (!m_sneakDelay && isSneaking)) && this->hasModifier(Modifiers::LAVA))
        m_state = State::AIRBORNE;

    float sprintjumpBoost = m_sprintjumpBoost;
    if (m_reverse) sprintjumpBoost *= -1;

    for (int i = 0; i < duration; i++) {
        this->update(overrideRotation, rotationOffset, isSprinting, isSneaking, slipperiness.value(), rotation.value(),
                     speedEffect.value(), slowEffect.value(), sprintjumpBoost);
    }
}

float Player::getMovementMultiplier(float slipperiness, bool isSprinting, int16_t speed, int16_t slow) {
    if (this->hasModifier(Modifiers::WATER) || this->hasModifier(Modifiers::LAVA)) {
        return 0.02f;
    } else if (m_state == State::AIRBORNE) {
        if ((m_airSprintDelay && m_previouslySprinting) || (!m_airSprintDelay && isSprinting)) {
            return 0.02f + 0.02f * 0.3f;
        } else {
            return 0.02f;
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

void Player::update(bool overrideRotation, float rotationOffset, bool isSprinting, bool isSneaking, float slipperiness,
                    float rotation, int speed, int slow, float sprintjumpBoost) {
    if (!overrideRotation) rotation = this->getAngle() + rotationOffset;
    this->position.add(this->velocity);

    if (this->hasModifier(Modifiers::SOULSAND)) this->velocity.scale(0.4);

    Vector2<float> direction = this->movementValues();

    this->velocity.scale(0.91 * m_previousSlipperiness);

    if (m_inertiaAxis == 1) {
        if (std::fabs(this->velocity.x) < m_inertiaThreshold || m_previouslyInWeb) this->velocity.x = 0.0f;
        if (std::fabs(this->velocity.z) < m_inertiaThreshold || m_previouslyInWeb) this->velocity.z = 0.0f;
    }

    if (m_state == State::JUMPING && isSprinting) {
        float facing = rotation * 0.017453292f;
        this->velocity.x -= double(this->mcsin(facing) * sprintjumpBoost);
        this->velocity.z += double(this->mccos(facing) * sprintjumpBoost);
    }

    if (this->hasModifier(Modifiers::BLOCK)) direction.scale(0.2f);
    if ((m_sneakDelay && m_previouslySneaking) || (!m_sneakDelay && isSneaking)) direction.scale(0.3f);
    direction.scale(0.98f);

    float multiplier = this->getMovementMultiplier(slipperiness, isSprinting, speed, slow);
    float distance = direction.sqrMagnitude();
    if (distance > 0.0f) {
        distance = std::sqrtf(distance);
        distance = std::max(distance, 1.0f);
        distance = multiplier / distance;
        direction.scale(distance);
        float sinYaw = this->mcsin(rotation * PI / 180.0f);
        float cosYaw = this->mccos(rotation * PI / 180.0f);
        this->velocity.x += direction.z * cosYaw - direction.x * sinYaw;
        this->velocity.z += direction.x * cosYaw + direction.z * sinYaw;
    }

    if (this->hasModifier(Modifiers::WEB)) this->velocity.scale(0.25f);
    if (this->hasModifier(Modifiers::LADDER)) {
        this->velocity.x = std::clamp(this->velocity.x, 0.15, -0.15);
        this->velocity.z = std::clamp(this->velocity.z, 0.15, -0.15);
    }

    m_previousSlipperiness = slipperiness;
    m_previouslySprinting = isSprinting;
    m_previouslySneaking = isSneaking;
    m_previouslyInWeb = this->hasModifier(Modifiers::WEB);
    m_lastTurn = rotation - m_lastRotation;
    m_lastRotation = rotation;
}

std::ostream& operator<<(std::ostream& os, const Player& p) {
    os << "Velocity: (" << std::setprecision(p.precision) << p.velocity.x << ", " << std::setprecision(p.precision)
       << p.velocity.z << ")" << std::endl
       << "Position: (" << std::setprecision(p.precision) << p.position.x << ", " << std::setprecision(p.precision)
       << p.position.z << ")" << std::endl;
    return os;
}
