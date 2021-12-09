#pragma once

class AttackState : public State
{
public:

	AttackState();
	~AttackState();

	// State을(를) 통해 상속됨
	virtual void Enter(Monster* mMonster) override;
	virtual void Execute(Monster* mMonster) override;
	virtual void Exit(Monster* mMonster) override;

private:

};