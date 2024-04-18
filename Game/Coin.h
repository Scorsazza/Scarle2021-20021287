#pragma once
#include "GameData.h"
#include "CMOGO.h"

class Coin : public CMOGO {
public:
	Coin(const string& modelFileName, ID3D11Device* device, IEffectFactory* fxFactory)
		: CMOGO(modelFileName, device, fxFactory) {
		SetScale(Vector3(1.0f, 1.0f, 1.0f));
	}

	virtual void Tick(GameData* GD) override {
		SetYaw(GetYaw() + GD->m_dt * 360.0f * 0.5f);
		CMOGO::Tick(GD);
	}
};