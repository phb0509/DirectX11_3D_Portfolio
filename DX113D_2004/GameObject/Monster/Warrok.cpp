#include "Framework.h"


Warrok::Warrok()
	: 
	mAnimation(eMutantAnimationStates::Idle),
	mFSM(eMutantFSMStates::Patrol),
	mbOnHit(false)
{
	mScale = { 0.05f, 0.05f, 0.05f };

	SetMesh("Warrok", "Warrok.mesh");
	SetMaterial("Warrok", "Warrok.mat");
	SetShader(L"ModelAnimation");

	mRotation.y = XM_PI;
	UpdateWorld();
}

Warrok::~Warrok()
{

}

void Warrok::Update()
{
	mPosition.y = mTerrain->GetTargetPositionY(mPosition);

	UpdateWorld();
	ModelAnimator::Update();
}

void Warrok::InstanceUpdate()
{
}

void Warrok::PreRender()
{
}

void Warrok::Render()
{
	SetWorldBuffer();
	ModelAnimator::Render();
}

void Warrok::PostRender()
{
	
}

void Warrok::OnDamage(AttackInformation attackInformation)
{
	mFSM = eMutantFSMStates::OnDamage;
	mbOnHit = true;
	//GM->SetHitCheckMap(this, true);
	mCurHP -= 10.0f;
}

bool Warrok::CheckIsCollision(Collider* collider)
{
	return false;
}

void Warrok::CheckOnHit()
{
	if (!mbOnHit) return;

	SetAnimation(static_cast<int>(eMutantAnimationStates::OnDamage));
}

Collider* Warrok::GetColliderForAStar()
{
	return mBodyCollider;
}

void Warrok::setOnDamageEnd()
{
	SetAnimation(static_cast<int>(eMutantAnimationStates::Idle));
	mbOnHit = false;
}

void Warrok::SetIdle()
{
	SetAnimation(static_cast<int>(eMutantAnimationStates::Idle));
}

void Warrok::SetAnimation(int animationState, bool isForcingPlay)
{
	eMutantAnimationStates state = static_cast<eMutantAnimationStates>(animationState);

	if (mAnimation != state)
	{
		mAnimation = state;
		PlayClip(static_cast<int>(mAnimation));
	}
}



void Warrok::setAttackEnd()
{
	mPlayer = GM->Get()->GetPlayer();
	RotateToDestinationForModel(this, mPlayer->mPosition);
}

Collider* Warrok::GetHitCollider()
{
	return mBodyCollider;
}









