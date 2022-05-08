#include "Framework.h"

Player::Player(): 
	ModelAnimator(),
	mbIsInitialize(false),
	state(IDLE),
	mbIsNormalAttack(false),
	mbIsNormalAttackCollide(false),
	mNormalAttackDamage(10.0f),
	mbIsTargetMode(false),
	mTargetCameraRotationX(0.0f),
	mTargetCameraRotationY(0.0f),
	mbIsLODTerrain(false)
{
	mScale = { 0.05f, 0.05f, 0.05f };
	mMoveSpeed = 50.0f;
	mRotationSpeed = 5.0f;
	mPreFrameMousePosition = MOUSEPOS;

	SetMesh("Player", "Player.mesh");
	SetMaterial("Player", "Player.mat");

	SetShader(L"ModelAnimation");

	ReadClip("Player", "TPose.clip");
	ReadClip("Player", "Idle.clip");
	ReadClip("Player", "Run.clip");
	ReadClip("Player", "Attack.clip");
	ReadClip("Player", "Die.clip");

	SetEndEvent(RUN, bind(&Player::setIdle, this));
	SetEndEvent(ATTACK, bind(&Player::setAttackEnd, this));

	PlayClip(1);

	mRotation.y = XM_PI; // 포워드랑 반대로되어있어서 180도 돌려줘야됨.

	loadBinaryFile();
	UpdateWorld();
}

Player::~Player()
{

}

void Player::Update()
{
	if (!mbIsInitialize)
	{
		initialize();
		mbIsInitialize = true;
	}

	setColliders();

	for (int i = 0; i < mColliders.size(); i++)
	{
		mColliders[i].collider->Update();
	}

	updateCamera();

	move();
	attack();

	checkNormalAttackCollision(); // 기본공격 몬스터 충돌체크.

	Transform::UpdateWorld();
	ModelAnimator::Update();
}

void Player::Render()
{
	RenderColliders();

	/*if (!mbIsTargetMode)
	{
		mTargetCameraInWorld->RenderFrustumCollider(); 
	}*/

	Transform::UpdateWorld();
	Transform::SetWorldBuffer();
	ModelAnimator::Render();
}

void Player::DeferredRender()
{
	Transform::UpdateWorld();
	Transform::SetWorldBuffer();
	ModelAnimator::DeferredRender();
}

void Player::move()
{
	if (mbIsTargetMode)
	{
		moveInTargetMode();
	}
	else
	{
		moveInWorldMode();
	}
}

void Player::attack()
{
	if (KEY_DOWN(VK_LBUTTON))
	{
		normalAttack();
	}
}

void Player::updateCamera()
{
	if (mbIsTargetMode)
	{
		mTargetCamera->Update();
		moveTargetCamera();
	}
	else
	{
		mTargetCameraInWorld->Update();
		moveTargetCameraInWorld();
	}
}

// Player
void Player::moveInTargetMode() // Player
{
	if (mbIsNormalAttack) return;

	float terrainY = 0.0f;

	if (mbIsLODTerrain)
	{
		//terrainY = mLODTerrain->GetHeight(mPosition);
	}
	else
	{
		terrainY = mTerrain->GetTargetPositionY(mPosition);
	}
	

	mPosition.y = terrainY;

	if (KEY_PRESS('W'))
	{
		rotateInTargetMode();
		mPosition.z += mTargetCameraForward.z * mMoveSpeed * DELTA * 1.0f;
		mPosition.x += mTargetCameraForward.x * mMoveSpeed * DELTA * 1.0f;

		setAnimation(RUN);
	}

	if (KEY_PRESS('S'))
	{
		rotateInTargetMode();
		mPosition.z += mTargetCameraForward.z * -mMoveSpeed * DELTA * 1.0f;
		mPosition.x += mTargetCameraForward.x * -mMoveSpeed * DELTA * 1.0f;

		setAnimation(RUN);
	}

	if (KEY_PRESS('A'))
	{
		rotateInTargetMode();
		mPosition += Right() * mMoveSpeed * DELTA;
	}

	if (KEY_PRESS('D'))
	{
		rotateInTargetMode();
		mPosition += Right() * -mMoveSpeed * DELTA;
	}
} 

void Player::moveInWorldMode() // Player
{
	if (mbIsNormalAttack) return;

	float terrainY = mTerrain->GetTargetPositionY(mPosition);

	mPosition.y = terrainY;

	if (KEY_PRESS('W'))
	{
		mPosition.z += -Forward().z * mMoveSpeed * DELTA;
		mPosition.x += -Forward().x * mMoveSpeed * DELTA;

		setAnimation(RUN);
	}

	if (KEY_PRESS('S'))
	{
		mPosition.z -= Forward().z * -mMoveSpeed * DELTA;
		mPosition.x -= Forward().x * -mMoveSpeed * DELTA;

		setAnimation(RUN);
	}

	if (KEY_PRESS('A'))
	{
		mRotation.y -= mRotationSpeed * DELTA;
	}

	if (KEY_PRESS('D'))
	{
		mRotation.y += mRotationSpeed * DELTA;
	}
}

void Player::rotateInTargetMode()
{
	float rotationSpeed = 10.0f * DELTA;

	Vector3 temp = Vector3::Cross(mTargetCameraForward, -1 * Forward());

	if (temp.y < 0.0f) // 플레이어 포워드벡터가 카메라포워드벡터의 왼쪽에 있다면. // 즉 오른쪽으로 회전해야한다면
	{
		if (CompareFloat(mTargetCameraForward.x, -Forward().x) &&
			CompareFloat(mTargetCameraForward.z, -Forward().z))
		{
		}
		else
		{
			mRotation.y += rotationSpeed;
		}
	}

	else if (temp.y >= 0.0f)// 왼쪽으로 회전해야 한다면.
	{
		if (CompareFloat(mTargetCameraForward.x, -Forward().x) &&
			CompareFloat(mTargetCameraForward.z, -Forward().z))
		{
		}
		else
		{
			mRotation.y -= rotationSpeed;
		}
	}
}

// SettedCamera - TargetCamera
void Player::moveTargetCamera()
{
	mTargetCamera->mRotation = { 0.0f,0.0f,0.0f };
	rotateTargetCamera();

	Matrix targetCameraRotMatrixY = XMMatrixRotationY(mTargetCameraRotationY); // rotY는 FollowControl에서 마우스x좌표값 이동절대값에 따라 크기조절. 걍 rotY만큼 이동량 조절.
	Matrix targetCameraRotMatrixX = XMMatrixRotationX(mTargetCameraRotationX);
	float distance = 13.0f;
	float height = 11.0f;

	Vector3 targetCameraForward = XMVector3TransformNormal(kForward, targetCameraRotMatrixX * targetCameraRotMatrixY); // rotMatrix의 방향만? 따오는듯.
	//forward = XMVector3TransformNormal(forward, mRotMatrixX);

	Vector3 destPos;
	destPos = targetCameraForward * -distance;
	destPos += GetGlobalPosition();
	destPos.y += height;

	mTargetCamera->mPosition = LERP(mTargetCamera->mPosition, destPos, 5.0f * DELTA); // 카메라 위치.
																				      // 현위치에서 desPost까지 moveDamping값만큼.

	Vector3 offset = { 0.0f,5.0f,0.0f };
	Vector3 tempOffset = XMVector3TransformCoord(offset.data, targetCameraRotMatrixY);
	Vector3 targetPosition = GetGlobalPosition() + tempOffset;

	//cameraForward = (mPlayer->GlobalPos() - position).Normal();
	mTargetCameraForward = targetCameraForward.Normal();

	Matrix viewMatrix = XMMatrixLookAtLH(mTargetCamera->mPosition.data, targetPosition.data,
		mTargetCamera->Up().data); // 카메라위치 , 타겟위치 , 카메라의 업벡터

	// 프러스텀에 뷰버퍼 설정.
	mTargetCamera->GetViewBuffer()->SetMatrix(viewMatrix);
	mTargetCamera->SetViewToFrustum(viewMatrix);
}

void Player::rotateTargetCamera()
{
	Vector3 value = MOUSEPOS - mPreFrameMousePosition;

	mTargetCameraRotationY += value.x * mRotationSpeed * DELTA;
	mTargetCameraRotationX += value.y * mRotationSpeed * DELTA;

	mPreFrameMousePosition = MOUSEPOS;
}


// SettedCamera - TargetCameraInWorld
void Player::moveTargetCameraInWorld()
{
	Vector3 targetPosition = mPosition;
	Vector3 beforePosition = mTargetCameraInWorld->mPosition;

	mTargetCameraInWorld->mPosition = mPosition + Forward() * 1.0f; // 타겟오브젝트랑 같은 위치로 세팅.

	Matrix viewMatrix = XMMatrixLookAtLH(mTargetCameraInWorld->mPosition.data, targetPosition.data,
		mTargetCameraInWorld->Up().data); // 카메라위치 , 타겟위치 , 카메라의 업벡터

	// 프러스텀에 뷰버퍼 설정.
	mTargetCameraInWorld->GetViewBuffer()->SetMatrix(viewMatrix);
	mTargetCameraInWorld->SetViewToFrustum(viewMatrix);
}


void Player::checkNormalAttackCollision()
{
	if (mbIsNormalAttack) // 공격도중이면.
	{
		mMutants = GM->GetMonsters();

		int a = 0;
		for (int i = 0; i < mMutants.size(); i++)
		{
			if (!(GM->GetHitCheckMap()[mMutants[i]])) // 공격받을수있는 상황이면(한프레임도 아직 공격받지 않았다면
			{
				if (mCollidersMap["swordCollider"]->Collision(mMutants[i]->GetHitCollider())) // 고놈만 충돌ㅇ체크.
				{
					//mCollidersMap["swordCollider"]->SetColor(Float4(1.0f, 0.0f, 0.0f, 1.0f));
					mMutants[i]->OnDamage(mNormalAttackDamage);
				}
			}
		}
	}

}

void Player::setAttackEnd()
{
	setAnimation(IDLE);
	mbIsNormalAttack = false;
	//mCollidersMap["swordCollider"]->SetColor(Float4(0.0f, 1.0f, 0.0f, 1.0f));
}

void Player::normalAttack()
{
	if (mbIsNormalAttack) return;
	setAnimation(ATTACK);
	mbIsNormalAttack = true;
}

void Player::RenderColliders()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		mColliders[i].collider->Render();
	}
}

void Player::setColliders()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		string nodeName = mColliders[i].nodeName;
		int nodeIndex = GetNodeIndex(nodeName); // 반복문돌려서찾는건데 고정값이니까 룩업테이블 따로. 값있으면 바로 쓰고,없으면 그떄 get하면되니까.
		mColliders[i].matrix = GetTransformByNode(nodeIndex) * this->mWorldMatrix;
		mColliders[i].collider->SetParent(&mColliders[i].matrix);
	}
}

void Player::loadBinaryFile()
{
	BinaryReader binaryReader(L"TextData/Player.map");
	UINT colliderCount = binaryReader.UInt();
	int colliderType;

	mColliderSRTdatas.resize(colliderCount);
	mColliderDatas.resize(colliderCount);

	void* ptr1 = (void*)mColliderSRTdatas.data();

	for (int i = 0; i < colliderCount; i++)
	{
		mColliderDatas[i].colliderName = binaryReader.String();
		mColliderDatas[i].nodeName = binaryReader.String();
		mColliderDatas[i].colliderType = binaryReader.UInt();
	}

	binaryReader.Byte(&ptr1, sizeof(TempCollider) * colliderCount);

	for (int i = 0; i < colliderCount; i++)
	{
		mColliderDatas[i].position = mColliderSRTdatas[i].position;
		mColliderDatas[i].rotation = mColliderSRTdatas[i].rotation;
		mColliderDatas[i].scale = mColliderSRTdatas[i].scale;
	}

	// Create Colliders;
	for (int i = 0; i < mColliderDatas.size(); i++)
	{
		SettedCollider settedCollider;
		Collider* collider = nullptr;

		switch (mColliderDatas[i].colliderType)
		{
		case 0: collider = new BoxCollider();
			break;
		case 1: collider = new SphereCollider();
			break;
		case 2: collider = new CapsuleCollider();
			break;
		default:
			break;
		}
		 
		if (collider != nullptr)
		{
			collider->mTag = mColliderDatas[i].colliderName;
			collider->mPosition = mColliderDatas[i].position;
			collider->mRotation = mColliderDatas[i].rotation;
			collider->mScale = mColliderDatas[i].scale;

			settedCollider.colliderName = mColliderDatas[i].colliderName;
			settedCollider.nodeName = mColliderDatas[i].nodeName;
			settedCollider.collider = collider;

			mColliders.push_back(settedCollider);
			mCollidersMap[mColliderDatas[i].colliderName] = collider;
		}
	}

	binaryReader.CloseReader();
}

void Player::initialize()
{

}

void Player::setIdle()
{
	setAnimation(IDLE);
}

void Player::setAnimation(State value)
{
	if (state != value)
	{
		state = value;
		PlayClip(state);
	}
}

void Player::PostRender()
{
	ImGui::Begin("Player PostRender");

	ImGui::Text("isNormalAttack : %d\n ", mbIsNormalAttack);

	ImGui::InputFloat3("Player Position", (float*)&this->mPosition);
	SpacingRepeatedly(1);

	ImGui::End();
}