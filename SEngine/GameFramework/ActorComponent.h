#pragma once

class Actor;

class ActorComponent
{
public:
	ActorComponent(Actor* owner);
	virtual ~ActorComponent();

protected:
	virtual void Initialize();

public:
	Actor* GetOwner() { return m_owner; }

public:
	virtual void Tick(const float& deltaTime);

protected:
	Actor* m_owner;
};
