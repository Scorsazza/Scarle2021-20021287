#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "CMOGO.h"
#include <string> // Ensure string is included if it's not in CMOGO
using namespace std;

//=================================================================
// Base Player Class (i.e., a GameObject the player controls)
//=================================================================

class Player : public CMOGO {
public:
	// Constructor
	Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);

	// Destructor
	virtual ~Player();

	// Overrides the virtual Tick method from CMOGO
	virtual void Tick(GameData* _GD) override;

	// Checks if the player is currently immune (Redundant, this system did not like working, opted for another.
	bool IsImmune() const;

	// Resets the player
	void Reset();

protected:
	bool m_isImmune;
	float m_immuneTime;
	float m_immuneDuration;
};

#endif // _PLAYER_H_
