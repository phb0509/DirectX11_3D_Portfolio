#pragma once

class Player;
class State;
class PatrolState;
class StalkingState;
class AttackState;
class OnDamageState;

class Monster : public Transform , public ModelAnimator
{
public:

	Monster();
	~Monster();

	virtual void Update() = 0;
	virtual void InstanceUpdate() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual Collider* GetHitCollider() = 0;
	virtual bool CheckOnDamage(const Collider* collider) = 0;
	virtual void OnDamage(float damage) = 0;
	virtual void CheckOnHit() = 0;
	virtual Collider* GetColliderForAStar() = 0;
	virtual void SetAnimation(eAnimationStates value) = 0;

	void MoveToDestUsingAStar(Vector3 dest);
	void ChangeState(State* nextState);

	Terrain* GetTerrain()const { return mTerrain; }
	vector<Vector3>& GetPath() { return mPath; }
	float GetMoveSpeed() const { return mMoveSpeed; }
	bool GetIsStalk()const { return mbIsStalk; }
	float GetDistanceToPlayer();
	float GetDistanceToPlayerForAttack() const { return mDistanceToPlayerForAttack; }
	float GetPlayerDetectRange() const { return mPlayerDetectRange; }
	PatrolState* GetPatrolState() { return mPatrolState; }
	StalkingState* GetStalkingState() { return mStalkingState; }
	AttackState* GetAttackState() { return mAttackState; }
	AStar* GetAStar() { return mAStar; }
	eAnimationStates GetAnimationStates() { return mAnimationState; }
	State* GetCurrentState() { return mCurrentState; }
	int GetCurrentClip() { return mFrameBuffer->data.tweenDesc[0].cur.clip; }
	int GetNextClip() { return mFrameBuffer->data.tweenDesc[0].next.clip; }
	InstanceColliderData GetInstanceColliderData() { return mInstanceColliderData; }

	void SetTerrain(Terrain* value, bool hasTerrainObstacles);
	void SetAStar(AStar* value) { mAStar = value; }
	void SetIsStalk(bool value) { mbIsStalk = value; }
	void SetDistanceToPlayerForAttack(float value) { mDistanceToPlayerForAttack = value; }
	void SetAnimationStates(eAnimationStates animationStates) { mAnimationState = animationStates; }
	void SetRealtimeAStarPath(Vector3 destPos);
	void SetAStarPath(Vector3 destPos);
	void SetInstanceIndex(int index) { mInstanceIndex = index; }
	void SetUpperFrameBuffer(FrameBuffer* frameBuffer) { mUpperFrameBuffer = frameBuffer; }
	void SetInstanceCollider(InstanceColliderData instanceColliderData) { mInstanceColliderData = instanceColliderData; }

private:
	void setObstaclesTerrain(Vector3 destPos);
	void setNoneObstaclesTerrain(Vector3 destPos);

protected:
	float mDamage;
	float mMaxHP;
	float mCurrentHP;

	Terrain* mTerrain;
	Player* mPlayer;
	AStar* mAStar;
	Vector3 mDestPos;
	vector<Vector3> mPath;

	bool mbIsStalk;

	Vector3 mDistanceVector3ToPlayer; // 플레이어까지 거리 x,z만 반영.
	float mDistanceToPlayer;
	float mPlayerDetectRange;
	float mDistanceToPlayerForAttack;
	float mAStarPathUpdatePeriodTime;

	State* mCurrentState;
	PatrolState* mPatrolState;
	StalkingState* mStalkingState;
	AttackState* mAttackState;
	OnDamageState* mOnDamageState;
	ModelAnimator* mModelAnimator;
	eAnimationStates mAnimationState;
	eMutantFSMstates mFSM;
	int mInstanceIndex;
	FrameBuffer* mUpperFrameBuffer;

private:
	function<void(Vector3)> mPathUpdatePeriodFuncPointer;
	function<void(Transform*, Vector3)> mRotationPeriodFuncPointer;

	Vector3 mTargetNodeDirVector3;
	Vector3 mTargetNode;

	Vector3 mBeforeDirVector3;
	Vector3 mBeforeNode;

	vector<bool> mPathNodesCheck;
	int mPathNodesCheckSize;

	Vector3 mBeforeTargetPosition;
	Vector3 mCurrentTargetPosition;

	bool mbPathSizeCheck;
	bool mbHasTerrainObstacles;

	int count = 0;
	InstanceColliderData mInstanceColliderData;
};