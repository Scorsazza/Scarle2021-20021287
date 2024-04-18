#pragma once
#include "Player.h"
#include "GameData.h"
class Enemy : public CMOGO {
public:

	Enemy(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Player* _player)
		: CMOGO(_fileName, _pd3dDevice, _EF), m_player(_player)
	{
		SetPhysicsOn(true);
		m_pos.y = 0.0f;
		m_pos.x = 200.0f;
	}
	void Activate(bool state) { isActive = state; }
	bool IsActive() const { return isActive; }

	virtual void Tick(GameData* _GD) override {
		if (!isActive) {
			return;
		}

		m_pos.y = m_player->GetPos().y;

		Vector3 dir = m_player->GetPos() - m_pos;
		float distance = dir.Length();
		dir.Normalize();

		float speed = std::min(10.0f, 5.0f + distance / 10.0f);

		if (distance < 20.0f) {
			speed *= 0.5f;
		}
		else if (distance > 100.0f) {
			speed *= 3.5f;
		}

		m_acc = dir * speed;

		m_yaw = atan2(dir.x, dir.z);

		CMOGO::Tick(_GD);
	}
	void Reset() {
		m_pos = Vector3(m_player->GetPos().x + 150, m_player->GetPos().y, 0.0f);

		m_vel = Vector3::Zero;
		m_acc = Vector3::Zero;
		m_yaw = 0;
	}

private:
	Player* m_player;
	GameData* m_GD = NULL;
	bool isActive;
};