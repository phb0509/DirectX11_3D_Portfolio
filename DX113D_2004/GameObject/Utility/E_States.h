#pragma once


namespace states
{
	enum class eAnimationStates
	{
		Idle,
		Run,
		SmashAttack,
		OnDamage,
		Die,
	};

	enum class eMutantFSMstates
	{
		Patrol,
		Stalk,
		Attack, // 스매쉬어택이든 뭐든, 공격상태면 일단?
		OnDamage,
		Die,
	};

	enum class eColliderType
	{
		BOX,
		SPHERE,
		CAPSULE,
		NONE
	};
}