#pragma once
#include "Exam_HelperStructs.h"
#include "../inc/EliteMath/EMath.h"
#include "../inc/IExamInterface.h"

class SteeringBehavior
{
public:
	SteeringBehavior() = default;
	virtual ~SteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	virtual void SetTarget(const Elite::Vector2& target) { m_Target = target; }
	Elite::Vector2 GetTarget() const { return m_Target; }
protected:
	Elite::Vector2 m_Target;
};

class Seek : public SteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

class Arrive final : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	void SetSlowRadius(float radius) { m_SlowRadius = radius; }

private:
	float m_SlowRadius{ 15.f };
};

class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	void SetFleeRadius(float radius) { m_FleeRadius = radius; }
protected:
	float m_FleeRadius{ 12.f };
};

class CollisionAvoidance : public Seek
{
	struct CollisionRect
	{
		Elite::Vector2 bottomLeft;
		Elite::Vector2 topLeft;
		Elite::Vector2 bottomRight;
		Elite::Vector2 topRight;
	};
public:
	CollisionAvoidance(const vector<EnemyInfo*>& pEnemies);
	~CollisionAvoidance();

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	void SetMaxSeeAhead(float dist) { m_MaxSeeAhead = dist; }
	void SetMaxAvoidanceForce(float force) { m_MaxAvoidanceForce = force; }
	void Render(IExamInterface* pInterface);

private:
	CollisionRect m_Rect;
	vector<EnemyInfo*> m_pEnemies;

	float m_MaxSeeAhead{ 8.f };
	float m_MaxAvoidanceForce{ 20.f };
	float m_RectAgentSideOffset{ 2.f };//how far the sides of the rectangle are from the border of the agent circle

	void InitRect(AgentInfo* pAgent);
	bool CircleRectOverlap(const Elite::Vector2 & center, float radius);
	float PointSegDist(const Elite::Vector2 & p, const Elite::Vector2 & a, const Elite::Vector2 & b);
	bool IsPointInRect(const Elite::Vector2 & point);
};

class Wander final : public CollisionAvoidance
{
public:
	Wander(const vector<EnemyInfo*>& pEnemies);
	virtual ~Wander() = default;

	//Wander Behavior
	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

protected:
	float m_Offset{ 6.f };//Offset (agent direction)
	float m_Radius{ 4.f };
	float m_AngleChange{ Elite::ToRadians(45) };//Max wander angle change per frame
	float m_WanderAngle{};//Internal

private:
	virtual void SetTarget(const Elite::Vector2& target) override {};
};

class Face final : public SteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};