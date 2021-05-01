#pragma once

class Mutant : public Monster, public ModelAnimator
{
public:
	friend class Player;

	struct colliderData
	{
		string name;
		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
	};

	struct temp_colliderData
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
	};


	Mutant();
	~Mutant();

	virtual void Update() override;
	virtual void Render() override;
	void PostRender();

	virtual Collider* GetHitCollider() override;
	virtual void OnDamage(float damage) override;
	virtual void CheckOnHit() override;
	virtual Collider* GetColliderForAStar() override;

	void SetIdle();
	void SetAnimation(eAnimationStates value) override;
	

private:

	void setOnDamageEnd();
	void setColliders();
	void setAttackEnd();
	void loadCollider();
	void findCollider(string name, Collider* collider);



private:

	Collider* mBodyCollider;
	Matrix mBodyMatrix;
	Collider* mSmashAttackCollider;
	Matrix mSmashAttackMatrix;

	vector<colliderData> mColliderDatas;
	vector<temp_colliderData> mTempColliderDatas;

	bool mbOnHit;

	eAnimationStates mAnimation;
	eStates mFSM;
};