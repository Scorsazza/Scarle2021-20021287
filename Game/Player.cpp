#include "pch.h"
#include "Player.h"
#include <dinput.h>
#include "GameData.h"

Player::Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF)
    : CMOGO(_fileName, _pd3dDevice, _EF), m_isImmune(true), m_immuneTime(0.0f), m_immuneDuration(10.0f)
{
    m_fudge = Matrix::CreateRotationY(XM_PI);
    m_pos.y = 10.0f;
    SetDrag(0.7);
    SetPhysicsOn(true);
}

Player::~Player()
{
}

void Player::Tick(GameData* _GD)
{

    float mouseSensitivity = -0.015f;

    m_yaw += _GD->m_MS.x * mouseSensitivity;

    if (m_isImmune) {
        m_immuneTime += _GD->m_dt;
        if (m_immuneTime >= m_immuneDuration) {
            m_isImmune = false;
        }
    }

    switch (_GD->m_GS) {
    case GS_PLAY_MAIN_CAM:
    {
        float speed = 10.0f;
        m_acc.x += speed * _GD->m_MS.x;
        m_acc.z += speed * _GD->m_MS.y;
        break;
    }
    case GS_PLAY_TPS_CAM:
    {
        Vector3 forwardMove = 40.0f * Vector3::Forward;
        Matrix rotMove = Matrix::CreateRotationY(m_yaw);
        Vector3 rightMove = 40.0f * Vector3::Right;
        forwardMove = Vector3::Transform(forwardMove, rotMove);
        rightMove = Vector3::Transform(rightMove, rotMove);

        if (_GD->m_KBS.W) {
            m_acc += forwardMove;
        }
        if (_GD->m_KBS.S) {
            m_acc -= forwardMove;
        }
        if (_GD->m_KBS.A) {
            m_acc -= rightMove;
        }
        if (_GD->m_KBS.D) {
            m_acc += rightMove;
        }

        break;
    }
    }

    if (m_yaw > XM_PI) {
        m_yaw -= XM_2PI;
    }
    else if (m_yaw < -XM_PI) {
        m_yaw += XM_2PI;
    }

    CMOGO::Tick(_GD);
}

bool Player::IsImmune() const {
    return m_isImmune;
}

void Player::Reset() {
    m_pos = Vector3(0.0f, 10.0f, 0.0f);

    m_vel = Vector3::Zero;
    m_acc = Vector3::Zero;

    m_isImmune = true;
    m_immuneTime = 0.0f;

    m_yaw = 0.0f;
}