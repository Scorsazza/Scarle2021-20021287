#include "pch.h"
#include "TPSCamera.h"
#include "GameData.h"

TPSCamera::TPSCamera(float _fieldOfView, float _aspectRatio, float _nearPlaneDistance, float _farPlaneDistance, GameObject* _target, DirectX::SimpleMath::Vector3 _up, DirectX::SimpleMath::Vector3 _dpos)
	: Camera(_fieldOfView, _aspectRatio, _nearPlaneDistance, _farPlaneDistance), m_targetObject(_target), m_up(_up), m_dpos(_dpos)
{
}

TPSCamera::~TPSCamera()
{
	// Destructor implementation
}

void TPSCamera::Tick(GameData* _GD) {
	Matrix rotMatrix = Matrix::CreateFromYawPitchRoll(m_targetObject->GetYaw(), m_targetObject->GetPitch(), 0.0f);

	// Adjust the camera's position offset
	m_pos = m_targetObject->GetPos() + DirectX::SimpleMath::Vector3::Transform(m_offset, rotMatrix);

	// Adjust the forward direction if it's currently looking backward
	m_target = m_pos - DirectX::SimpleMath::Vector3::Transform(Vector3::UnitZ, rotMatrix);  

	Camera::Tick(_GD);
}