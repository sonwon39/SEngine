#include "SceneComponent.h"
#include "directxtk12/SimpleMath.h"
#include "StaticMeshComponent.h"
#include "CollisionComponent.h"
#include "CameraComponent.h"

using DirectX::SimpleMath::Vector3;

SceneComponent::SceneComponent(Actor* owner)
	:ActorComponent(owner),
	m_frontDirection(Vector3(0, 0, 1)),
	m_baseFrontDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_baseUpDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
{
}

SceneComponent::~SceneComponent()
{
}

void SceneComponent::Attach(std::shared_ptr<SceneComponent> sceneComp)
{
	sceneComp->m_parent = this;
	m_children.push_back(sceneComp);
	//sceneComp->UpdateWorldTransform(localTransform);
	UpdateConstantTransform();
}

void SceneComponent::SetSpeed(const float& newSpeed)
{
	m_speed = newSpeed;
}
void SceneComponent::SetRotateSpeed(const float& newSpeed)
{
	m_rotateSpeed = newSpeed;
}

void SceneComponent::UpdateWorldTransform(const Transform& tr)
{
	worldTransform = tr;
	UpdateConstantTransform();
}

void SceneComponent::SetLocalConstant(const LocalConstant& newConstant)
{
	localConstant = newConstant;
	DirectX::SimpleMath::Matrix model = newConstant.model.Transpose();

	XMVECTOR s, q, t;
	DirectX::XMMatrixDecompose(&s, &q, &t, model);
	localTransform.location = t;
	localTransform.quat = q;
	localTransform.scale = s;

	UpdateConstantTransform();

	/*for (const auto& c : m_children)
	{
		c->UpdateWorldTransform(localTransform);
	}*/
}

void SceneComponent::UpdateRotation(const int& mouseDeltaX, const int& mouseDeltaY, const float& deltaTime)
{
	float delX = mouseDeltaX * m_rotateSpeed * deltaTime;
	float delY = mouseDeltaY * m_rotateSpeed * deltaTime;
	xAngle += delX;
	if (xAngle >= 360) {
		xAngle -= 360;
	}
	if (xAngle <= -360) {
		xAngle += 360;
	}

	float xRadian = DirectX::XMConvertToRadians(xAngle);

	DirectX::SimpleMath::Matrix m_rotation = DirectX::XMMatrixRotationY(xRadian);
	DirectX::SimpleMath::Quaternion yRotQ = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(m_rotation);

	DirectX::SimpleMath::Vector3 m_frontDir = DirectX::SimpleMath::Vector3::Transform(GetBaseFrontDirection(), m_rotation);
	DirectX::SimpleMath::Vector3 m_rightDir = GetBaseUpDirection().Cross(m_frontDir);;

	if (yAngle + delY >= maxYAngle)
		delY = maxYAngle - yAngle;
	else if (yAngle + delY <= minYAngle)
		delY = minYAngle - yAngle;

	yAngle += delY;
	//std::cout << yAngle << '\n';
	float yRadian = DirectX::XMConvertToRadians(yAngle);


	m_rotation = DirectX::XMMatrixRotationAxis(m_rightDir, yRadian);
	m_frontDir = DirectX::SimpleMath::Vector3::Transform(m_frontDir, m_rotation);

	SetFrontDirection(m_frontDir);
	SetRightDirection(m_rightDir);
	SetRotation(yRotQ);
}

void SceneComponent::SetLocation(const DirectX::SimpleMath::Vector3& newLocation)
{
	localTransform.location = newLocation;
	//UpdateConstantLocation();
	UpdateConstantTransform();


}

void SceneComponent::SetRotation(const DirectX::SimpleMath::Quaternion& newQuat)
{
	//TODO front direction도 회전 시켜야 함
	localTransform.quat = newQuat;
	UpdateConstantTransform();

	//for (const auto& c : m_children)
	//{
	//	c->UpdateWorldTransform(localTransform);
	//}
}
void SceneComponent::SetLocalTransform(const Matrix& newMatrix)
{
	//TODO front direction도 회전 시켜야 함
	Transform t(newMatrix);
	localTransform = t;
	UpdateConstantTransform();
}
void SceneComponent::SetLocalTransform(const Transform& newTransform)
{
	//TODO front direction도 회전 시켜야 함
	localTransform = newTransform;
	UpdateConstantTransform();
}
void SceneComponent::SetLocalTransformByLdotW(const Transform& LoW)
{
	Matrix m = LoW.ToMatrix() * worldTransform.ToMatrix().Invert();
	Transform t(m);

	localTransform = m;
	UpdateConstantTransform();
}
void SceneComponent::SetCubeMapMipLevel(const int& newCubeMapMipLevel)
{
	localConstant.cubeMapMipLevel = newCubeMapMipLevel;
}

void SceneComponent::SetHeightScale(const float& newHeightScale)
{
	localConstant.heightScale = newHeightScale;
}

void SceneComponent::SetRoughness(const float& newRoughness)
{
	localConstant.roughness = newRoughness;
}
void SceneComponent::SetMetallic(const float& newMetallic)
{
	localConstant.metallic = newMetallic;
}
void SceneComponent::SetCollisionScale(const Vector3& newScale)
{
	localConstant.collisionScale = newScale;
}
void SceneComponent::SetCollisionShape(const PhysXShape& newShape)
{
	localConstant.collisionShape = newShape;
}
void SceneComponent::AddLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	localTransform.location += delLocation;
	//UpdateConstantLocation();
	UpdateConstantTransform();

	/*for (const auto& c : m_children)
	{
		c->UpdateWorldTransform(localTransform);
	}*/
}

void SceneComponent::AddRotation(const DirectX::SimpleMath::Quaternion& delQ)
{
	localTransform.quat *= delQ;
	//UpdateConstantRotation();
	UpdateConstantTransform();

	/*for (const auto& c : m_children)
	{
		c->UpdateWorldTransform(localTransform);
	}*/
}

DirectX::SimpleMath::Vector3 SceneComponent::GetCollisionScale() const
{
	for (const auto& c : m_children)
	{
		if (CollisionComponent* collisionCmp = dynamic_cast<CollisionComponent*>(c.get()))
			return collisionCmp->GetCollisionScale();
	}
	return localConstant.collisionScale;
}

DirectX::SimpleMath::Quaternion SceneComponent::GetCollisionRotation() const
{
	for (const auto& c : m_children)
	{
		if (CollisionComponent* collisionCmp = dynamic_cast<CollisionComponent*>(c.get()))
			return collisionCmp->GetRotation();
	}
	return GetRotation();
}

DirectX::SimpleMath::Vector3 SceneComponent::GetCollisionLocation() const
{
	for (const auto& c : m_children)
	{
		if (CollisionComponent* collisionCmp = dynamic_cast<CollisionComponent*>(c.get()))
			return collisionCmp->GetLocation();
	}
	return GetLocation();
}

DirectX::SimpleMath::Vector3 SceneComponent::GetCollisionOffsetLocation() const
{
	for (const auto& c : m_children)
	{
		if (CollisionComponent* collisionCmp = dynamic_cast<CollisionComponent*>(c.get()))
			return collisionCmp->GetLocalLocation();
	}
	return GetLocalLocation();
}
DirectX::SimpleMath::Quaternion SceneComponent::GetCollisionOffsetRotation() const
{
	for (const auto& c : m_children)
	{
		if (CollisionComponent* collisionCmp = dynamic_cast<CollisionComponent*>(c.get()))
			return collisionCmp->GetLocalRotation();
	}
	return GetLocalRotation();
}
DirectX::SimpleMath::Quaternion SceneComponent::GetRotation() const
{
	Matrix M = localConstant.model.Transpose();
	Vector3 scale;
	Quaternion rot;
	Vector3 translation;

	bool ok = M.Decompose(scale, rot, translation);

	return rot;
}


DirectX::SimpleMath::Matrix SceneComponent::GetViewMatrix() const
{
	if (m_parent)
	{
		return XMMatrixLookToLH(GetLocation(), m_parent->m_frontDirection, m_parent->m_upDirection);
	}
	return XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);
}

DirectX::SimpleMath::Matrix SceneComponent::GetCameraViewMatrix() const
{
	for (const auto& c : m_children)
	{
		return c->GetViewMatrix();
	}
	return XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);
}

void SceneComponent::GetChildrenComponents(std::vector<std::shared_ptr<SceneComponent>>& children) const
{
	children = m_children;
}

void SceneComponent::OnRegister()
{
	for (const auto& c : m_children)
	{
		c->OnRegister();
	}
}

DirectX::SimpleMath::Vector3 SceneComponent::GetCameraLocation() const
{
	for (const auto& c : m_children)
	{
		if (CameraComponent* cameraComp = dynamic_cast<CameraComponent*>(c.get()))
		{
			return c->GetLocation();
		}
	}
	return DirectX::SimpleMath::Vector3::Zero;
}

DirectX::SimpleMath::Matrix SceneComponent::GetProjMatrix() const
{
	for (const auto& c : m_children)
	{
		if (CameraComponent* cameraComp = dynamic_cast<CameraComponent*>(c.get()))
		{
			return c->GetProjMatrix();
		}
	}
	return DirectX::SimpleMath::Matrix();
}

void SceneComponent::UpdateConstantTransform()
{
	auto mat = localTransform.ToMatrix() * worldTransform.ToMatrix();

	localConstant.model = mat;

	localConstant.modelInvTranspose = localConstant.model.Invert();
	localConstant.model = localConstant.model.Transpose();

	for (const auto& c : m_children)
	{
		Transform t(mat);
		c->UpdateWorldTransform(t);
	}
}

void SceneComponent::UpdateConstantLocation()
{
	auto loc = GetLocation();
	localConstant.model = localConstant.model.Transpose();

	localConstant.model.m[3][0] = loc.x;
	localConstant.model.m[3][1] = loc.y;
	localConstant.model.m[3][2] = loc.z;

	localConstant.modelInvTranspose = localConstant.model.Invert();
	localConstant.model = localConstant.model.Transpose();
}

void SceneComponent::UpdateConstantRotation()
{
	auto q = GetRotation();
	localConstant.model = localConstant.model.Transpose();
	DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixRotationQuaternion(q);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			localConstant.model.m[i][j] = mat.m[i][j];
		}
	}
	localConstant.modelInvTranspose = localConstant.model.Invert();
	localConstant.model = localConstant.model.Transpose();

}
void SceneComponent::UpdateMipState(int newForceMip0)
{
	localConstant.forceMip0 = newForceMip0;
	for (const auto& c : m_children)
	{
		c->UpdateMipState(newForceMip0);
	}
}
void SceneComponent::UpdateUseReflect(int newUseReflect)
{
	localConstant.useReflect = newUseReflect;

	for (const auto& c : m_children)
	{
		c->UpdateUseReflect(newUseReflect);
	}
}
void SceneComponent::UpdateTexTransform(const DirectX::SimpleMath::Matrix& texTransform)
{
	localConstant.texTransform = texTransform;
	for (const auto& c : m_children)
	{
		c->UpdateTexTransform(texTransform);
	}
}
void  SceneComponent::UpdateCameraInfo(const int& width, const int& height)
{
	for (const auto& c : m_children)
	{
		if (CameraComponent* cameraComp = dynamic_cast<CameraComponent*>(c.get()))
		{
			return c->UpdateCameraInfo(width, height);
		}
	}
}
