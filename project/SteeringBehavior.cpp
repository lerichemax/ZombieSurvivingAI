#include "stdafx.h"
#include "SteeringBehavior.h"

#include <cmath>
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering;

	steering.LinearVelocity = m_Target - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	steering.AutoOrient = true;

	return steering;
}

SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	float distanceToTarget{};

	steering.LinearVelocity = m_Target - pAgent->Position;
	distanceToTarget = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();

	if (distanceToTarget < m_SlowRadius)
	{
		steering.LinearVelocity *= (pAgent->MaxLinearSpeed * (distanceToTarget / m_SlowRadius));
	}
	else
	{
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	}

	return steering;
}

Wander::Wander(const vector<EnemyInfo*>& pEnemies)
	:CollisionAvoidance(pEnemies)
{

}

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	//Define the circle
	Elite::Vector2 center{};
	if (m_Offset == 0)
	{
		center = pAgent->Position;
	}
	else
	{
		center = pAgent->LinearVelocity;
		center.Normalize();
		center *= m_Offset;
		center += pAgent->Position;
	}

	//get a random angle offset
	std::default_random_engine generator(unsigned int(time(NULL)));
	std::uniform_real_distribution<float> distribution(-m_AngleChange, m_AngleChange);

	m_WanderAngle = atan2(pAgent->LinearVelocity.y, pAgent->LinearVelocity.x) + distribution(generator);
	m_Target = { cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius };
	m_Target += center;

	return CollisionAvoidance::CalculateSteering(deltaT, pAgent);
}

SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	float distance{ Elite::Distance(pAgent->Position, m_Target) };
	SteeringPlugin_Output steering{};
	if (distance > m_FleeRadius)
	{
		steering.LinearVelocity = Elite::Vector2{};
		return steering;
	}

	steering = Seek::CalculateSteering(deltaT, pAgent);
	steering.LinearVelocity = -steering.LinearVelocity;

	return steering;
}

CollisionAvoidance::CollisionAvoidance(const vector<EnemyInfo*>& pEnemies)
	:Seek{}, m_pEnemies { pEnemies }
{
}

CollisionAvoidance::~CollisionAvoidance()
{
	for (auto pEn : m_pEnemies)
	{
		delete pEn;
	}
}

SteeringPlugin_Output CollisionAvoidance::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	InitRect(pAgent);

	if (m_pEnemies.empty())
	{
		return Seek::CalculateSteering(deltaT, pAgent);
	}
	Elite::Vector2 seeAhead{ pAgent->Position + pAgent->LinearVelocity.GetNormalized() * m_MaxSeeAhead };

	auto closestEnemy = std::min_element(m_pEnemies.begin(), m_pEnemies.end(), [pAgent](EnemyInfo* pEn1, EnemyInfo* pEn2)
		{
			float distObs1{ (pEn1->Location - pAgent->Position).Magnitude() };
			float distObs2{ (pEn2->Location - pAgent->Position).Magnitude() };

			return distObs1 < distObs2;
		});

	if (CircleRectOverlap((*closestEnemy)->Location, (*closestEnemy)->Size))
	{
		Elite::Vector2 avoidanceForce{ seeAhead - (*closestEnemy)->Location };
		SteeringPlugin_Output steering{ Seek::CalculateSteering(deltaT, pAgent) };
		steering.LinearVelocity += avoidanceForce.GetNormalized() * m_MaxAvoidanceForce;
		return steering;
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

void CollisionAvoidance::Render(IExamInterface* pInterface)
{
	Elite::Vector2* points{ new Elite::Vector2[4]{
	{m_Rect.bottomLeft},
	{m_Rect.topLeft},
	{m_Rect.topRight},
	{m_Rect.bottomRight}}
	};
	pInterface->Draw_Polygon(points, 4, Elite::Vector3{ 1,0,0 });
}

void CollisionAvoidance::InitRect(AgentInfo* pAgent)
{
	float angle{ atan2(pAgent->LinearVelocity.y, pAgent->LinearVelocity.x) };
	m_Rect.bottomLeft = Elite::Vector2{ pAgent->Position.x + cos(angle + Elite::ToRadians(90)) * (pAgent->AgentSize/2 + m_RectAgentSideOffset),
		pAgent->Position.y + sin(angle + Elite::ToRadians(90)) * (pAgent->AgentSize / 2 + m_RectAgentSideOffset) };

	m_Rect.bottomRight = pAgent->Position + (pAgent->Position - m_Rect.bottomLeft);

	m_Rect.topLeft = m_Rect.bottomLeft + pAgent->LinearVelocity.GetNormalized() * m_MaxSeeAhead;
	m_Rect.topRight = m_Rect.bottomRight + pAgent->LinearVelocity.GetNormalized() * m_MaxSeeAhead;
}

bool CollisionAvoidance::CircleRectOverlap(const Elite::Vector2& center, float radius)
{
	if (IsPointInRect(center))
	{
		return true;
	}
	// Check line segments
	if (PointSegDist(center, m_Rect.bottomLeft, m_Rect.topLeft) <= radius)
	{
		return true;
	}
	if (PointSegDist(center, m_Rect.bottomLeft, m_Rect.bottomRight) <= radius)
	{
		return true;
	}
	if (PointSegDist(center, m_Rect.topRight, m_Rect.topLeft) <= radius)
	{
		return true;
	}
	if (PointSegDist(center, m_Rect.topRight, m_Rect.bottomRight) <= radius)
	{
		return true;
	}
	return false;
}

float CollisionAvoidance::PointSegDist(const Elite::Vector2& p, const Elite::Vector2& a, const Elite::Vector2& b)
{
	Elite::Vector2 ab{ b - a };
	Elite::Vector2 ap{ p - a };
	Elite::Vector2 abNorm{ ab.GetNormalized() };
	float distToA{ Elite::Dot(abNorm, ap) };


	// If distToA is negative, then the closest point is A
	// return the distance a, p
	if (distToA < 0)
	{
		return ap.Magnitude();
	}
	// If distToA is > than dist(a,b) then the closest point is B
	// return the distance b, p
	float distAB{ ab.Magnitude() };
	if (distToA > distAB)
	{
		return Elite::Vector2{ p - b }.Magnitude();
	}

	// Closest point is between A and B, calc intersection point
	Elite::Vector2 intersection{ Elite::Dot(abNorm, ap) * abNorm + Elite::Vector2{ a } };
	return Elite::Vector2{ p - intersection }.Magnitude();
}

bool CollisionAvoidance::IsPointInRect(const Elite::Vector2& point)
{
	if ((point.x >= m_Rect.bottomLeft.x && point.x <= m_Rect.bottomRight.x)
		&& (point.y >= m_Rect.bottomLeft.y && point.y <= m_Rect.topLeft.y))
	{
		return true;
	}
	return false;
}

SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	steering.AutoOrient = false;

	Elite::Vector2 targetVector{ m_Target - pAgent->Position };

	float targetAngle{ atan2(targetVector.y, targetVector.x) - pAgent->Orientation  };

	targetAngle += (targetAngle == 0 ? 0 : (float)M_PI / 2);
	if (targetAngle > M_PI) {
		targetAngle -= 2*(float)M_PI;
	}

	steering.AngularVelocity = targetAngle * pAgent->MaxAngularSpeed;

	return steering;
}