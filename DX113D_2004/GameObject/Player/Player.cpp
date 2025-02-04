#include "Framework.h"

Player::Player(): 
	PlayerModelAnimator(),
	mAnimationStates(Idle),
	mbIsNormalAttack(false),
	mNormalAttackDamage(34.0f),
	mKickAttackDamage(20.0f),
	mbIsTargetMode(false),
	mTargetCameraRotationX(0.0f),
	mTargetCameraRotationY(0.0f),
	mbIsLODTerrain(false),
	mbIsCheckNormalAttackCollision(false),
	mMaxHP(100.0f),
	mCurHP(100.0f),
	mHPRate(1.0f),
	mMaxMP(100.0f),
	mCurMP(100.0f),
	mMPRate(1.0f),
	mbIsDie(false)
{
	SetTag("Player");
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
	ReadClip("Player", "LeftWalk.clip");
	ReadClip("Player", "RightWalk.clip");
	ReadClip("Player", "BackWalk.clip");
	ReadClip("Player", "Attack.clip");
	ReadClip("Player", "Kick.clip");
	ReadClip("Player", "Die.clip");

	SetEndEvent(Run, bind(&Player::setIdle, this));
	SetEndEvent(LeftWalk, bind(&Player::setIdle, this));
	SetEndEvent(RightWalk, bind(&Player::setIdle, this));
	SetEndEvent(BackWalk, bind(&Player::setIdle, this));
	SetEndEvent(NormalAttack, bind(&Player::setNormalAttackEnd, this));
	SetEndEvent(KickAttack, bind(&Player::setKickAttackEnd, this));

	PlayClip(1);

	mRotation.y = XM_PI; // 포워드랑 반대로되어있어서 180도 돌려줘야됨.

	loadCollidersBinaryFile(L"Player.map");
	setAttackInformations();
	UpdateWorld();

	mStatusBar = new PlayerStatusBar();
	mStatusBar->SetTag("PlayerStatusBar");
	mStatusBar->SetMaxHP(mMaxHP);
	mStatusBar->SetCurHP(mCurHP);
}

Player::~Player()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		GM->SafeDelete(mColliders[i].collider);
	}

	GM->SafeDelete(mStatusBar);
}

void Player::Update()
{
	mStatusBar->Update();

	setColliders();
	updateCamera();

	move();
	attack();

	Transform::UpdateWorld();
	PlayerModelAnimator::Update();

	if (KEY_DOWN('U'))
	{
		mbIsNormalAttack = false;
		mbIsKickAttack = false;
	}
}

void Player::Render()
{
	//renderColliders();
	Transform::UpdateWorld();
	Transform::SetWorldBuffer();
	PlayerModelAnimator::Render();
}

void Player::DeferredRender()
{
	//renderDeferredColliders();
	Transform::UpdateWorld();
	Transform::SetWorldBuffer();
	PlayerModelAnimator::DeferredRender();
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

	if (KEY_DOWN(VK_RBUTTON))
	{
		kickAttack();
	}

	checkNormalAttackCollision(); // 기본공격 몬스터 충돌체크.
	checkKickAttackCollision();
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

void Player::moveInTargetMode() // Player
{
	if (mbIsNormalAttack) return;
	if (mbIsKickAttack) return;

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

		setAnimation(Run);
	}

	if (KEY_PRESS('S'))
	{
		rotateInTargetMode();
		mPosition.z += mTargetCameraForward.z * -mMoveSpeed * DELTA * 1.0f;
		mPosition.x += mTargetCameraForward.x * -mMoveSpeed * DELTA * 1.0f;

		setAnimation(BackWalk);
	}

	if (KEY_PRESS('A'))
	{
		setAnimation(LeftWalk,1.0f,0.05f);
		rotateInTargetMode();
		mPosition += GetRightVector() * mMoveSpeed * DELTA;
	}

	if (KEY_PRESS('D'))
	{
		setAnimation(RightWalk,1.0f,0.05f);
		rotateInTargetMode();
		mPosition += GetRightVector() * -mMoveSpeed * DELTA;
	}
} 

void Player::moveInWorldMode() // Player
{
	if (mbIsNormalAttack) return;
	if (mbIsKickAttack) return;

	float terrainY = mTerrain->GetTargetPositionY(mPosition);

	mPosition.y = terrainY;

	if (KEY_PRESS('W'))
	{
		mPosition.z += -GetForwardVector().z * mMoveSpeed * DELTA;
		mPosition.x += -GetForwardVector().x * mMoveSpeed * DELTA;

		setAnimation(Run);
	}

	if (KEY_PRESS('S'))
	{
		mPosition.z -= GetForwardVector().z * -mMoveSpeed * DELTA;
		mPosition.x -= GetForwardVector().x * -mMoveSpeed * DELTA;

		setAnimation(Run);
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
	float rotationSpeed = 20.0f * DELTA;

	Vector3 temp = Vector3::Cross(mTargetCameraForward, -1 * GetForwardVector());

	if (temp.y < 0.0f) // 플레이어 포워드벡터가 카메라포워드벡터의 왼쪽에 있다면. // 즉 오른쪽으로 회전해야한다면
	{
		if (CompareFloat(mTargetCameraForward.x, -GetForwardVector().x) &&
			CompareFloat(mTargetCameraForward.z, -GetForwardVector().z))
		{}
		else
		{
			mRotation.y += rotationSpeed;
		}
	}

	else if (temp.y >= 0.0f)// 왼쪽으로 회전해야 한다면.
	{
		if (CompareFloat(mTargetCameraForward.x, -GetForwardVector().x) &&
			CompareFloat(mTargetCameraForward.z, -GetForwardVector().z))
		{}
		else
		{
			mRotation.y -= rotationSpeed;
		}
	}
}

// SettedCamera - TargetCamera
void Player::moveTargetCamera()
{
	//m TargetCamera->mRotation = { 0.25f,0.25f,0.0f };
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
		mTargetCamera->GetUpVector().data); // 카메라위치 , 타겟위치 , 카메라의 업벡터

	// 프러스텀에 뷰버퍼 설정.
	mTargetCamera->SetViewMatrix(viewMatrix);
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

	mTargetCameraInWorld->mPosition = mPosition + GetForwardVector() * 1.0f; // 타겟오브젝트랑 같은 위치로 세팅.

	Matrix viewMatrix = XMMatrixLookAtLH(mTargetCameraInWorld->mPosition.data, targetPosition.data,
		mTargetCameraInWorld->GetUpVector().data); // 카메라위치 , 타겟위치 , 카메라의 업벡터

	// 프러스텀에 뷰버퍼 설정.
	mTargetCameraInWorld->SetViewMatrix(viewMatrix);
	mTargetCameraInWorld->GetViewBuffer()->SetMatrix(viewMatrix);
	mTargetCameraInWorld->SetViewToFrustum(viewMatrix);
}

void Player::checkNormalAttackCollision()
{
	if (!mbIsNormalAttack) return;

	for (auto monster : mMonsters)
	{
		string monsterName = monster.first;

		for (int i = 0; i < mMonsters[monsterName].size(); i++) // 전부 검사.
		{
			Monster* monster = mMonsters[monsterName][i].monster;
			bool bWasNormalAttackedPrev = mMonsters[monsterName][i].isCheckAttackMap["NormalAttack"];

			if (!bWasNormalAttackedPrev && !(monster->GetIsDie())) // 아직 공격받지 않은 상태면,, 
			{
				if (monster->CheckIsCollision(mAttackInformations["NormalAttack"].attackColliders[0])) // 충돌됐다면
				{
					monster->OnDamage(mAttackInformations["NormalAttack"]);
					mMonsters[monsterName][i].isCheckAttackMap["NormalAttack"] = true;
				}
			}
		}
	}
}

void Player::checkKickAttackCollision()
{
	if (!mbIsKickAttack) return;

	for (auto monster : mMonsters)
	{
		string monsterName = monster.first;

		for (int i = 0; i < mMonsters[monsterName].size(); i++) // 전부 검사.
		{
			Monster* monster = mMonsters[monsterName][i].monster;
			bool bWasKickAttackedPrev = mMonsters[monsterName][i].isCheckAttackMap["KickAttack"];

			if (!bWasKickAttackedPrev && !(monster->GetIsDie())) // 아직 공격받지 않은 상태면,, 
			{
				if (monster->CheckIsCollision(mAttackInformations["KickAttack"].attackColliders[0])) // 충돌됐다면
				{
					monster->OnDamage(mAttackInformations["KickAttack"]);
					mMonsters[monsterName][i].isCheckAttackMap["KickAttack"] = true;
				}
			}
		}
	}
}

void Player::setNormalAttackEnd()
{
	setAnimation(Idle);
	mbIsNormalAttack = false;

	for (auto monster : mMonsters)
	{
		string monsterName = monster.first;

		for (int i = 0; i < mMonsters[monsterName].size(); i++) // 전부 검사.
		{
			mMonsters[monsterName][i].isCheckAttackMap["NormalAttack"] = false;
		}
	}
}

void Player::setKickAttackEnd()
{
	setAnimation(Idle);
	mbIsKickAttack = false;

	for (auto monster : mMonsters)
	{
		string monsterName = monster.first;

		for (int i = 0; i < mMonsters[monsterName].size(); i++) // 전부 검사.
		{
			mMonsters[monsterName][i].isCheckAttackMap["KickAttack"] = false;
		}
	}
}

void Player::normalAttack()
{
	if (mbIsNormalAttack) return;
	if (mbIsKickAttack) return;
	setAnimation(NormalAttack);
	mbIsNormalAttack = true;
}

void Player::kickAttack()
{
	if (mbIsKickAttack) return;
	if (mbIsNormalAttack) return;
	setAnimation(KickAttack);
	mbIsKickAttack = true;
}

void Player::renderColliders()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		mColliders[i].collider->Render();
	}
}

void Player::renderDeferredColliders()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		mColliders[i].collider->DeferredRender();
	}
}

void Player::setColliders()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		string nodeName = mColliders[i].nodeName;
		int nodeIndex = mNodeLookupTable[nodeName]; // 반복문돌려서찾는건데 고정값이니까 룩업테이블 따로. 값있으면 바로 쓰고,없으면 그때 get하면되니까.
		mColliders[i].matrix = GetTransformByNode(nodeIndex) * this->GetWorldMatrixValue(); // 해당 노드의 월드매트릭스.
		mColliders[i].collider->SetParent(&mColliders[i].matrix); // 컬라이더의 부모를 노드의 월드매트릭스로 지정.
		mColliders[i].collider->Update();
	}
}

void Player::setAttackInformations()
{
	// 지금은 AttackInformation에 컬라이더 1개만 넣긴 하지만, 공격에따라 여러 컬라이더를 넣어야할 수도 있긴함.
	vector<Collider*> temp;

	// NormalAttack
	temp.push_back(mCollidersMap["SwordCollider"]);
	AttackInformation normalAttackInformation("NormalAttack", temp, eAttackType::Normal, mNormalAttackDamage);
	mAttackInformations[normalAttackInformation.attackName] = normalAttackInformation;
	temp.clear();
	mbIsCheckAttack["NormalAttack"] = false;

	// KickAttack
	temp.push_back(mCollidersMap["KickAttackCollider"]);
	AttackInformation kickAttackInformation("KickAttack", temp, eAttackType::KnockBack, mKickAttackDamage);
	mAttackInformations[kickAttackInformation.attackName] = kickAttackInformation;
	temp.clear();
	mbIsCheckAttack["KickAttack"] = false;
}

void Player::loadCollidersBinaryFile(wstring fileName)
{
	bool isSuccessedLoadFile = false;
	wstring temp = L"TextData/" + fileName;
	BinaryReader binaryReader(temp, isSuccessedLoadFile);

	UINT colliderCount = binaryReader.ReadUInt();

	mTempColliderSRTdatas.resize(colliderCount);
	mTempColliderDatas.resize(colliderCount);

	void* ptr1 = (void*)mTempColliderSRTdatas.data();

	for (int i = 0; i < colliderCount; i++)
	{
		mTempColliderDatas[i].colliderName = binaryReader.ReadString();
		mTempColliderDatas[i].nodeName = binaryReader.ReadString();
		mTempColliderDatas[i].colliderType = binaryReader.ReadUInt();
	}

	binaryReader.Byte(&ptr1, sizeof(TempCollider) * colliderCount);

	for (int i = 0; i < colliderCount; i++)
	{
		mTempColliderDatas[i].position = mTempColliderSRTdatas[i].position;
		mTempColliderDatas[i].rotation = mTempColliderSRTdatas[i].rotation;
		mTempColliderDatas[i].scale = mTempColliderSRTdatas[i].scale;
	}

	// Create Colliders;
	for (int i = 0; i < mTempColliderDatas.size(); i++)
	{
		SettedCollider settedCollider;
		Collider* collider = nullptr;

		switch (mTempColliderDatas[i].colliderType)
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
			collider->SetTag(mTempColliderDatas[i].colliderName);
			collider->mPosition = mTempColliderDatas[i].position;
			collider->mRotation = mTempColliderDatas[i].rotation;
			collider->mScale = mTempColliderDatas[i].scale;

			settedCollider.colliderName = mTempColliderDatas[i].colliderName;
			settedCollider.nodeName = mTempColliderDatas[i].nodeName;
			settedCollider.collider = collider;

			mColliders.push_back(settedCollider);
			mCollidersMap[mTempColliderDatas[i].colliderName] = collider;
		}
	}

	binaryReader.CloseReader();
}

void Player::SetMonsters(string name, vector<Monster*> monsters)
{
	for (int i = 0; i < monsters.size(); i++)
	{
		MonsterForAttackCheck temp;
		temp.monster = monsters[i];
		temp.isCheckAttackMap = mbIsCheckAttack; // setAttackInformations()에서 미리 설정해놓은 값.
		mMonsters[name].push_back(temp);
	}
}

void Player::OnDamage(AttackInformation attackInformation)
{
	mCurHP -= attackInformation.damage;

	if (mCurHP <= 0.0f) mCurHP = 0.0f;

	mHPRate = mCurHP / mMaxHP;
	mStatusBar->SetCurHP(mCurHP);
	mStatusBar->SetHPRate(mHPRate);

	if (mCurHP == 0.0f)
	{
		mbIsDie = true;
	}

	if (attackInformation.attackType == eAttackType::Normal)
	{
		/*GetFSMState(static_cast<int>(eMutantFSMStates::AttackedNormal))->SetIsDie(mbIsDie);
		GetFSMState(static_cast<int>(eMutantFSMStates::AttackedNormal))->SetAttackInformation(attackInformation);
		ChangeState(GetFSMState(static_cast<int>(eMutantFSMStates::AttackedNormal)));*/
	}
	else if (attackInformation.attackType == eAttackType::KnockBack)
	{
	/*	GetFSMState(static_cast<int>(eMutantFSMStates::AttackedKnockBack))->SetIsDie(mbIsDie);
		GetFSMState(static_cast<int>(eMutantFSMStates::AttackedKnockBack))->SetAttackInformation(attackInformation);
		ChangeState(GetFSMState(static_cast<int>(eMutantFSMStates::AttackedKnockBack)));*/
	}
}

bool Player::CheckIsCollision(Collider* collider)
{
	Collider* bodyCollider = mCollidersMap["BodyCollider"];

	if (bodyCollider->Collision(collider))
	{
		return true;
	}

	return false;
}

void Player::setIdle()
{
	setAnimation(Idle);
}

void Player::setAnimation(eAnimationStates value, float speed, float takeTime, bool isForcingPlay)
{
	if (!isForcingPlay)
	{
		if (mAnimationStates != value)
		{
			mAnimationStates = value;
			PlayClip(mAnimationStates, speed, takeTime);
		}
	}
	else
	{
		mAnimationStates = value;
		PlayClip(mAnimationStates, speed, takeTime);
	}
}

void Player::PostRender()
{
	ImGui::Begin("Player PostRender");

	ImGui::InputFloat3("Player Position", (float*)&this->mPosition);
	SpacingRepeatedly(3);

	ImGui::Text("isNormalAttack : %d\n ", mbIsNormalAttack);
	ImGui::Text("isKickAttack : %d\n ", mbIsKickAttack);

	SpacingRepeatedly(2);

	//for (auto monster : mMonsters)
	//{
	//	string monsterName = monster.first;

	//	for (int i = 0; i < mMonsters[monsterName].size(); i++) // 전부 검사.
	//	{
	//		Monster* monster = mMonsters[monsterName][i].monster;

	//		string tag = monster->GetTag();
	//		string temp = tag + " is NormalAttacked?  :  " + to_string((int)mMonsters[monsterName][i].isCheckAttackMap["NormalAttack"]);

	//		ImGui::Text(temp.c_str());
	//		SpacingRepeatedly(2);
	//	}
	//}

	//mMonsters[monsterName][i].isCheckAttackMap["NormalAttack"]

	ImGui::End();

	//mStatusBar->PostTransformRender();
	//mStatusBar->PostRender();
}

void Player::UIRender()
{
	mStatusBar->Render();
}
