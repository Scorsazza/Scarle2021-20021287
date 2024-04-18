#ifndef _TPS_CAMERA_
#define _TPS_CAMERA_
#include "camera.h"

class TPSCamera : public Camera {
public:
	TPSCamera(float _fieldOfView, float _aspectRatio, float _nearPlaneDistance, float _farPlaneDistance, GameObject* _target, DirectX::SimpleMath::Vector3 _up, DirectX::SimpleMath::Vector3 _dpos);
	virtual ~TPSCamera();

	void Tick(GameData* _GD) override;

private:
	GameObject* m_targetObject;
	DirectX::SimpleMath::Vector3 m_up;
	DirectX::SimpleMath::Vector3 m_dpos;
	DirectX::SimpleMath::Vector3 m_offset;
};

#endif
