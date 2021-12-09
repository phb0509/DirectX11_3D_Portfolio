#include "Framework.h"


Mutant::Mutant()
	: ModelAnimator(),
	mAnimation(eAnimationStates::Idle),
	mFSM(eFSMstates::Patrol),
	mbOnHit(false)
{
	mScale = { 0.05f, 0.05f, 0.05f };

	SetMesh("Mutant", "Mutant.mesh");
	SetMaterial("Mutant", "Mutant.mat");

	SetShader(L"ModelAnimation");

	ReadClip("Mutant","Idle.clip");
	ReadClip("Mutant","Run.clip");
	ReadClip("Mutant","SmashAttack.clip");
	ReadClip("Mutant","OnDamage.clip");
	ReadClip("Mutant","Die.clip");

	SetEndEvent(static_cast<int>(eAnimationStates::Run), bind(&Mutant::SetIdle, this));
	SetEndEvent(static_cast<int>(eAnimationStates::OnDamage), bind(&Mutant::setOnDamageEnd, this));
	SetEndEvent(static_cast<int>(eAnimationStates::SmashAttack), bind(&Mutant::setAttackEnd, this));
	
	PlayClip(0);

	loadBinaryFile(); // 툴에서 셋팅한 컬라이더 불러오기.

	mRotation.y = XM_PI;
	UpdateWorld();

	mPlayerDetectRange = 15.0f;
	mDistanceToPlayerForAttack = 7.0f;
    mCurrentState = GetPatrolState();
}

Mutant::~Mutant()
{

}

void Mutant::Update()
{
	setColliders();

	for (int i = 0; i < mColliders.size(); i++)
	{
		mColliders[i].collider->Update();
	}

	mPosition.y = mTerrain->GetHeight(mPosition);

	mCurrentState->Execute(this);

	UpdateWorld();
	ModelAnimator::Update();
}

void Mutant::Render()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		mColliders[i].collider->Render();
	}

	SetWorldBuffer();
	ModelAnimator::Render();
}

void Mutant::PostRender()
{

}

void Mutant::OnDamage(float damage)
{
	mFSM = eFSMstates::OnDamage;
	mbOnHit = true;
	GM->SetHitCheckMap(this, true);
	mCurrentHP -= 10.0f;
}

void Mutant::CheckOnHit()
{
	if (!mbOnHit) return;

	SetAnimation(eAnimationStates::OnDamage);
}

Collider* Mutant::GetColliderForAStar() // 몸쪽 컬라이더 넘겨주자.
{
	return mCollidersMap["bodyCollider"];
}

void Mutant::setOnDamageEnd()
{
	SetAnimation(eAnimationStates::Idle);
	GM->SetHitCheckMap(this, false);
	mbOnHit = false;
}


void Mutant::SetIdle()
{
	SetAnimation(eAnimationStates::Idle);
}

void Mutant::SetAnimation(eAnimationStates value)
{
	if (mAnimation != value)
	{
		mAnimation = value;
		PlayClip(static_cast<int>(mAnimation));
	}
}



void Mutant::setAttackEnd()
{
	mPlayer = GM->Get()->GetPlayer();
	RotateToDestinationForModel(this, mPlayer->mPosition);
}

Collider* Mutant::GetHitCollider() // 히트체크용 컬라이더
{
	return mCollidersMap["bodyCollider"];
}


void Mutant::loadBinaryFile()
{
	BinaryReader binaryReader(L"TextData/Mutant.map");
	UINT colliderCount = binaryReader.UInt();

	mColliderSRTdatas.resize(colliderCount);
	mColliderDatas.resize(colliderCount);

	void* ptr1 = (void*)mColliderSRTdatas.data();

	for (int i = 0; i < colliderCount; i++)
	{
		mColliderDatas[i].colliderName = binaryReader.String();
		mColliderDatas[i].nodeName = binaryReader.String();
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
		Collider* collider = new BoxCollider(); // 일단 박스

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

	binaryReader.CloseReader();
}

void Mutant::setColliders()
{
	for (int i = 0; i < mColliders.size(); i++)
	{
		string nodeName = mColliders[i].nodeName;
		int nodeIndex = GetNodeIndex(nodeName); // 반복문돌려서찾는건데 고정값이니까 룩업테이블 따로. 값있으면 바로 쓰고,없으면 그떄 get하면되니까.
		mColliders[i].matrix = GetTransformByNode(nodeIndex) * this->mWorldMatrix;
		mColliders[i].collider->SetParent(&mColliders[i].matrix);
	}
}




