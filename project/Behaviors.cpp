#include "stdafx.h"
#include "Behaviors.h"
#include "SteeringBehavior.h"
#include "VisitedHouse.h"
#include "World.h"

#include <vector>
#include <algorithm>
#include <iostream>

//CONDITIONS
bool AreEnemiesVisible(Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* pEntities = nullptr;

	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return false;
	}

	for (const EntityInfo& ent : *pEntities)
	{
		if (ent.Type == eEntityType::ENEMY)
		{
			return true;
		}
	}
	return false;
}

bool GetClosestVisibleEnemyPosition(Blackboard* pBlackboard)
{
	if (!AreEnemiesVisible(pBlackboard))
	{
		return false;
	}

	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	std::vector<EntityInfo>* pEntities = nullptr;
	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return false;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}

	EnemyInfo* pClosestInfo{};
	if (!pBlackboard->GetData("ClosestEnemy", pClosestInfo))
	{
		return false;
	}

	Elite::Vector2* pTarget{};
	if (!pBlackboard->GetData("Target", pTarget))
	{
		return false;
	}

	float shortestDistance{ FLT_MAX };
	EnemyInfo closest{};

	std::for_each((*pEntities).begin(), (*pEntities).end(), 
		[pAgent, &shortestDistance, &pInterface, &closest, &pTarget](EntityInfo entity)
	{
			if (entity.Type == eEntityType::ENEMY)
			{
				if (float currentDistance = (pAgent->Position - entity.Location).Magnitude() < shortestDistance)
				{
					shortestDistance = currentDistance;
					*pTarget = entity.Location;
					pInterface->Enemy_GetInfo(entity, closest);
				}
			}
	});

	delete pClosestInfo;
	pClosestInfo = new EnemyInfo{ closest };
	pBlackboard->ChangeData("ClosestEnemy", pClosestInfo);
	return true;
}

bool IsHouseVisible(Blackboard* pBlackboard)
{
	std::vector<HouseInfo>* pHouses = nullptr;
	if (!pBlackboard->GetData("VisibleHouses", pHouses) || (*pHouses).size() == 0)
	{
		return false;
	}

	HouseInfo* pHouse = nullptr;
	if (!pBlackboard->GetData("SeekedHouse", pHouse))
	{
		return false;
	}

	Elite::Vector2* pTarget{};
	pBlackboard->GetData("Target", pTarget);

	WorldState* pWorld = nullptr;
	pBlackboard->GetData("WorldState", pWorld);

	if (pHouse == nullptr)
	{
		for (HouseInfo& info : (*pHouses))
		{
			if (!IsHouseVisited(pBlackboard, info.Center))
			{
				*pTarget = info.Center;
				pHouse = new HouseInfo{ info };
				pBlackboard->ChangeData("SeekedHouse", pHouse);
				if (pWorld)
				{
					pWorld->AddKnownHouse(pHouse);
				}
				return true;
			}
		}
		*pTarget = (*pHouses)[0].Center;
		return true;
	}

	*pTarget = pHouse->Center;
	return true;
}

bool AreItemsVisible(Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* pEntities = nullptr;

	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return false;
	}

	WorldState* pWorld = nullptr;
	if (!pBlackboard->GetData("WorldState", pWorld))
	{
		return false;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}

	bool toReturn{};
	for (const EntityInfo& ent : *pEntities)
	{
		if (ent.Type == eEntityType::ITEM)
		{
			ItemInfo item;
			Elite::Vector2* pTarget{};
			pBlackboard->GetData("Target", pTarget);
			*pTarget = ent.Location;
			toReturn = true;
			pInterface->Item_GetInfo(ent, item);
			pWorld->AddKnownItem(item);
		}
	}

	return toReturn;
}

bool AreKnownItems(Blackboard* pBlackboard)
{
	WorldState* pWorld = nullptr;
	if (!pBlackboard->GetData("WorldState", pWorld))
	{
		return false;
	}

	if (pWorld->AreKnownItems())
	{
		Elite::Vector2* pTarget{};
		pBlackboard->GetData("Target", pTarget);
		*pTarget = pWorld->GetKnownItems()[0].Location;
		return true;
	}

	return false;
}

bool HasItemInGrabRange(Blackboard* pBlackboard)
{
	if (!AreItemsVisible(pBlackboard))
	{
		return false;
	}

	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	std::vector<EntityInfo>* pEntities = nullptr;
	pBlackboard->GetData("VisibleEntities", pEntities);

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	float grabRange = pAgent->GrabRange;
	Elite::Vector2 agentPos = pAgent->Position;
	for (const EntityInfo& entity : *pEntities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			if ((agentPos - entity.Location).Magnitude() <= grabRange)
			{
				EntityInfo* pEntity{};
				pBlackboard->GetData("EntityToGrab", pEntity);
				delete pEntity;
				pEntity = new EntityInfo{ entity };
				pBlackboard->ChangeData("EntityToGrab", pEntity);
				return true;
			}
		}
	}

	return false;
}

bool HasGun(Blackboard* pBlackboard)
{
	if (HasItem(pBlackboard, eItemType::PISTOL))
	{
		return HasAmmo(pBlackboard, eItemType::PISTOL);
	}

	return false;
}

bool HasNoGun(Blackboard* pBlackboard)
{
	return !HasItem(pBlackboard, eItemType::PISTOL);
}

bool HasAmmo(Blackboard* pBlackboard)
{
	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}

	ItemInfo info{};
	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (!pInterface->Inventory_GetItem(i, info))
		{
			continue;
		}
		if (info.Type != eItemType::PISTOL)
		{
			continue;
		}
		if (pInterface->Weapon_GetAmmo(info) > 0)
		{
			return true;
		}
	}
	return false;
}

bool WillHit(Blackboard* pBlackboard)
{
	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	EnemyInfo* pClosestInfo{};
	if (!pBlackboard->GetData("ClosestEnemy", pClosestInfo))
	{
		return false;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}
	
	//Ray
	Elite::Vector2 origin{ pAgent->Position };
	Elite::Vector2 dir{ Elite::GetNormalized(Elite::Vector2{
		cos(pAgent->Orientation-(float)M_PI/2), sin(pAgent->Orientation - (float)M_PI / 2)}) };

	dir *= pAgent->FOV_Range;
	pInterface->Draw_Segment(origin, origin + dir, { 0,1,0 });

	//circle
	Elite::Vector2 center{ pClosestInfo->Location };
	float radius{ pClosestInfo->Size };

	//Ax^2 + Bx + C = 0
	float A = dir.SqrtMagnitude();
	float B = 2 * Dot(dir, origin - center);
	float C = (origin - center).SqrtMagnitude() - radius * radius;

	float discr = B * B - 4 * A * C;
	if (discr < 0)
	{
		return false;
	}
	
	return true;
}

bool HasMedKit(Blackboard* pBlackboard)
{
	return HasItem(pBlackboard, eItemType::MEDKIT);
}

bool WasBitten(Blackboard* pBlackboard)
{
	AgentInfo* pAgent;
	if (!pBlackboard->GetData("Agent",pAgent))
	{
		return false;
	}

	return pAgent->WasBitten;
}

bool NeedsHealing(Blackboard* pBlackboard)
{
	AgentInfo* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	return pAgent->Health < 10;
}

bool HasFood(Blackboard* pBlackboard)
{
	return HasItem(pBlackboard, eItemType::FOOD);
}

bool NeedsEnergy(Blackboard* pBlackboard)
{
	AgentInfo* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	return pAgent->Energy < 10;
}

bool HasInventoryFreeSpace(Blackboard* pBlackboard)
{
	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	ItemInfo item{};

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (!pInterface->Inventory_GetItem(i, item))
		{
			return true;
		}
	}
	return false;
}

bool IsHouseNotVisited(Blackboard* pBlackboard)
{
	Elite::Vector2* pTarget = nullptr;
	if (!pBlackboard->GetData("Target", pTarget))
	{
		return false;
	}

	HouseInfo* pHouse = nullptr;
	if (!pBlackboard->GetData("SeekedHouse", pHouse))
	{
		return false;
	}

	return !IsHouseVisited(pBlackboard, *pTarget);
}

bool IsInHouse(Blackboard* pBlackboard)
{
	auto pAgent = new AgentInfo{};
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	return pAgent->IsInHouse;
}

bool IsCloseToHouseCenter(Blackboard* pBlackboard)
{
	float distance{ 5.f };

	auto pAgent = new AgentInfo{};
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	HouseInfo* pHouse;
	if (!pBlackboard->GetData("SeekedHouse", pHouse))
	{
		return false;
	}
	if (pHouse == nullptr)
	{
		return false;
	}

	if ((pAgent->Position - pHouse->Center).Magnitude() <= distance)
	{
		return true;
	}
	return false;
}

bool HasHouseTarget(Blackboard* pBlackboard)
{
	HouseInfo* pHouse;
	if (!pBlackboard->GetData("SeekedHouse", pHouse))
	{
		return false;
	}

	if (pHouse == nullptr)
	{
		return false;
	}
	return true;
}

bool HasStamina(Blackboard* pBlackboard)
{
	auto pAgent = new AgentInfo{};
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	if (pAgent->Stamina > 1)
	{
		return true;
	}
	return false;
}

bool IsPurgeZoneVisible(Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* pEntities = nullptr;
	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return false;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}

	for (const EntityInfo& ent : *pEntities)
	{
		if (ent.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo purge{};

			PurgeZoneInfo* pPurge;
			pBlackboard->GetData("PurgeZone", pPurge);

			Elite::Vector2* pTarget{};
			pBlackboard->GetData("Target", pTarget);
			*pTarget = ent.Location;
			pInterface->PurgeZone_GetInfo(ent, purge);

			delete pPurge;
			pPurge = new PurgeZoneInfo{ purge };
			pBlackboard->ChangeData("PurgeZone", pPurge);
			return true;
		}
	}

	if (IsInPurgeZone(pBlackboard))
	{
		return true;
	}

	pBlackboard->ChangeData("PurgeZone", static_cast<PurgeZoneInfo*>(nullptr));
	return false;
}

bool IsInPurgeZone(Blackboard* pBlackboard)
{
	PurgeZoneInfo* pPurge = nullptr;
	pBlackboard->GetData("PurgeZone", pPurge);
	if (pPurge == nullptr)
	{
		return false;
	}

	AgentInfo* pAgent;
	pBlackboard->GetData("Agent", pAgent);

	if ((pAgent->Position - pPurge->Center).Magnitude() >= pPurge->Radius + 30)//hardcoded security distance;
	{
		return false;
	}
	return true;
}

bool IsCloseToCellCenter(Blackboard* pBlackboard)
{
	WorldState* pWorld = nullptr;
	if (!pBlackboard->GetData("WorldState", pWorld))
	{
		return false;
	}

	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return false;
	}

	int currentCellIdx{ pWorld->PositionToIndex(pAgent->Position) };
	if (pWorld->GetCell(currentCellIdx)->IsExplored)
	{
		return false;
	}
	return (pAgent->Position - pWorld->GetCellCenter(currentCellIdx)).Magnitude() < 20.f;
}

//BEHAVIORS
BehaviorState ChangeToRunFrom(Blackboard* pBlackboard)
{
	SteeringBehavior** pSeek = nullptr;

	if (!pBlackboard->GetData("SteeringBehavior", pSeek))
	{
		return Failure;
	}
	delete * pSeek;
	std::vector<EntityInfo>* pEntities = nullptr;
	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return Failure;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	PurgeZoneInfo* pPurge = nullptr;
	if (!pBlackboard->GetData("PurgeZone", pPurge))
	{
		return Failure;
	}

	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return Failure;
	}

	*pSeek = new CollisionAvoidance{ GetVisibleEnemiesInfo(pInterface, pEntities) };

	Elite::Vector2 fleeDir{ (pAgent->Position - pPurge->Center).GetNormalized() * (pPurge->Radius + 30) };//hardcoded security distance;
	Elite::Vector2 fleeTarget = pPurge->Center + fleeDir;

	(*pSeek)->SetTarget(pInterface->NavMesh_GetClosestPathPoint(fleeTarget));
	pBlackboard->ChangeData("SteeringBehavior", pSeek);
	return Success;
}

BehaviorState ChangeToSeek(Blackboard* pBlackboard)
{
	SteeringBehavior** pSeek = nullptr;
	if (!pBlackboard->GetData("SteeringBehavior", pSeek))
	{
		return Failure;
	}
	delete *pSeek;

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	std::vector<EntityInfo>* pEntities = nullptr;
	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return Failure;
	}

	*pSeek = new CollisionAvoidance{ GetVisibleEnemiesInfo(pInterface, pEntities) };

	Elite::Vector2* pTarget{};
	if (!pBlackboard->GetData("Target", pTarget))
	{
		return Failure;
	};

	(*pSeek)->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));
	pBlackboard->ChangeData("SteeringBehavior", pSeek);

	return Success;
}

BehaviorState ChangeToWander(Blackboard* pBlackboard)
{
	SteeringBehavior** pWander = nullptr;

	if (!pBlackboard->GetData("SteeringBehavior", pWander))
	{
		return Failure;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	std::vector<EntityInfo>* pEntities = nullptr;
	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return Failure;
	}

	delete* pWander;
	*pWander = new Wander{ GetVisibleEnemiesInfo(pInterface, pEntities) };

	pBlackboard->ChangeData("SteeringBehavior", pWander);

	WorldState* pWorld = nullptr;
	pBlackboard->GetData("WorldState", pWorld);
	if (pWorld)
	{
		pWorld->SetIsAgentExploring(false);
	}

	return Success;
}

BehaviorState SeekHouseCenter(Blackboard* pBlackboard)
{
	SteeringBehavior** pColl = nullptr;
	!pBlackboard->GetData("SteeringBehavior", pColl);
	delete *pColl;

	IExamInterface* pInterface = nullptr;
	if (!!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	WorldState* pWorld = nullptr;
	pBlackboard->GetData("WorldState", pWorld);
	if (pWorld)
	{
		pWorld->SetIsAgentExploring(false);
	}

	return ChangeToSeek(pBlackboard);
}

BehaviorState ChangeToSeekItem(Blackboard* pBlackboard)
{
	SteeringBehavior* pSeek = nullptr;
	pBlackboard->GetData("SteeringBehavior", pSeek);
	delete pSeek;

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	std::vector<EntityInfo>* pEntities = nullptr;
	if (!pBlackboard->GetData("VisibleEntities", pEntities))
	{
		return Failure;
	}

	pSeek = new Seek{};

	pBlackboard->ChangeData("SteeringBehavior", pSeek);

	WorldState* pWorld = nullptr;
	pBlackboard->GetData("WorldState", pWorld);
	if (pWorld)
	{
		pWorld->SetIsAgentExploring(false);
	}

	return Success;
}

BehaviorState GrabAndAddItem(Blackboard* pBlackboard)
{
	unsigned int freeSlotIdx{};
	if (!HasInventoryFreeSlots(pBlackboard, freeSlotIdx))
	{
		return Failure;
	}

	EntityInfo* pEntity{};
	if (!pBlackboard->GetData("EntityToGrab", pEntity))
	{
		return Failure;
	}

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	WorldState* pWorld = nullptr;
	if (!pBlackboard->GetData("WorldState", pWorld))
	{
		return Failure;
	}

	ItemInfo info{};
	
	pInterface->Item_GetInfo(*pEntity, info);
	pWorld->RemoveKnownItem(info);
	if (info.Type == eItemType::GARBAGE)
	{
		if (!pInterface->Item_Destroy(*pEntity))
		{
			return Failure;
		}
		return Success;
	}

	pInterface->Item_Grab(*pEntity, info);
	pInterface->Inventory_AddItem(freeSlotIdx, info);
	
	return Success;
}

BehaviorState Shoot(Blackboard* pBlackboard)
{
	if (!GetClosestVisibleEnemyPosition(pBlackboard))
	{
		return Failure;
	}
	return UseItem(pBlackboard, eItemType::PISTOL);
}

BehaviorState UseMedKit(Blackboard* pBlackboard)
{
	if (!(HasMedKit(pBlackboard) && NeedsHealing(pBlackboard)))
	{
		return Failure;
	}
	return UseItem(pBlackboard, eItemType::MEDKIT);
}

BehaviorState EatFood(Blackboard* pBlackboard)
{
	if (!HasFood(pBlackboard))
	{
		return Failure;
	}
	return UseItem(pBlackboard, eItemType::FOOD);
}

BehaviorState ChangeToFace(Blackboard* pBlackboard)
{
	SteeringBehavior** pFace = nullptr;
	if (!pBlackboard->GetData("SteeringBehavior", pFace))
	{
		return Failure;
	}

	delete *pFace;
	*pFace = new Face{};

	Elite::Vector2* pTarget{};
	if (!pBlackboard->GetData("Target", pTarget))
	{
		return Failure;
	}
	(*pFace)->SetTarget(*pTarget);

	WorldState* pWorld = nullptr;
	pBlackboard->GetData("WorldState", pWorld);
	if (pWorld)
	{
		pWorld->SetIsAgentExploring(false);
	}

	return Success;
}

BehaviorState SetHouseVisited(Blackboard* pBlackboard)
{
	if (!IsInHouse(pBlackboard))
	{
		return Failure;
	}

	std::vector<VisitedHouse>* pVisitedHouses;
	if (!pBlackboard->GetData("VisitedHouses", pVisitedHouses))
	{
		return Failure;
	}

	HouseInfo* pHouse;
	if (!pBlackboard->GetData("SeekedHouse", pHouse))
	{
		return Failure;
	}

	if (std::find_if(pVisitedHouses->begin(), pVisitedHouses->end(), [&pHouse](const VisitedHouse& visitedHouse)
		{
			return visitedHouse.houseCenter == pHouse->Center;
		}) == pVisitedHouses->end())
	{
		pVisitedHouses->push_back(VisitedHouse{ pHouse });
	}
	return Success;
}

BehaviorState LeaveHouse(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("SeekedHouse", static_cast<HouseInfo*>(nullptr));
	return Explore(pBlackboard);
}

BehaviorState Sprint(Blackboard* pBlackboard)
{
	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return Failure;
	}

	pAgent->RunMode = true;
	pBlackboard->ChangeData("Agent", pAgent);
	return Success;
}

BehaviorState StopSprint(Blackboard* pBlackboard)
{
	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return Failure;
	}

	pAgent->RunMode = false;
	pBlackboard->ChangeData("Agent", pAgent);
	return Success;
}

BehaviorState ReturnFailure(Blackboard* pBlackboard)
{
	return Failure;
}

BehaviorState Explore(Blackboard* pBlackboard)
{
	WorldState* pWorld = nullptr;
	if (!pBlackboard->GetData("WorldState", pWorld))
	{
		return Failure;
	}

	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return Failure;
	}

	Elite::Vector2* pTarget;
	if (!pBlackboard->GetData("Target", pTarget))
	{
		return Failure;
	}

	int currentCellIdx{ pWorld->PositionToIndex(pAgent->Position) };

	if (pWorld->IsCellExplored(currentCellIdx))
	{
		std::vector<VisitedHouse>* pVisitedHouses;
		if (!pBlackboard->GetData("VisitedHouses", pVisitedHouses))
		{
			return Failure;
		}
		
		Cell* pCell = pWorld->GetClosestUnexploredCell(pAgent->Position);
		if (pCell != nullptr)
		{
			*pTarget = pCell->Center;
		}
		else
		{
			*pTarget = pWorld->GetClosestUnvisitedHouse(pAgent->Position, pVisitedHouses)->Center;
		}

		pWorld->SetIsAgentExploring(true);
		return ChangeToSeek(pBlackboard);
	}
	else
	{
		*pTarget = pWorld->GetCell(currentCellIdx)->Center;
		pWorld->SetIsAgentExploring(true);
		return ChangeToSeek(pBlackboard);
	}
}

BehaviorState SetCellExplored(Blackboard* pBlackboard)
{
	WorldState* pWorld = nullptr;
	if (!pBlackboard->GetData("WorldState", pWorld))
	{
		return Failure;
	}

	AgentInfo* pAgent = nullptr;
	if (!pBlackboard->GetData("Agent", pAgent))
	{
		return Failure;
	}

	int currentCellIdx{ pWorld->PositionToIndex(pAgent->Position) };
	pWorld->SetExplored(currentCellIdx);
	return Success;
}

//HELPERS

std::vector<EnemyInfo*> GetVisibleEnemiesInfo(IExamInterface* pInterface, std::vector<EntityInfo>* pEntities)
{
	std::vector<EnemyInfo*> pEnemies{};

	EnemyInfo enemy;
	for (const auto& entity : *pEntities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			if (pInterface->Enemy_GetInfo(entity, enemy))
			{
				pEnemies.push_back(new EnemyInfo{ enemy });
			}
		}
	}
	return pEnemies;
}

bool GetVisibleItemTypePosition(Blackboard* pBlackboard, eItemType type)
{
	if (!AreItemsVisible(pBlackboard))
	{
		return false;
	}

	std::vector<EntityInfo>* pEntities = nullptr;
	pBlackboard->GetData("VisibleEntities", pEntities);

	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	ItemInfo itemInfo{};
	for (const EntityInfo& ent : *pEntities)
	{
		if (ent.Type == eEntityType::ITEM)
		{
			if (pInterface->Item_GetInfo(ent, itemInfo))
			{
				if (itemInfo.Type == type)
				{
					Elite::Vector2* pTarget{};
					pBlackboard->GetData("Target", pTarget);
					*pTarget = itemInfo.Location;
					return true;
				}
			}
		}
	}
	return false;
}

bool HasInventoryFreeSlots(Blackboard* pBlackboard, unsigned int& firstFreeSlot)
{
	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	ItemInfo item{};

	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (!pInterface->Inventory_GetItem(i, item))
		{
			firstFreeSlot = i;
			return true;
		}
	}
	return false;
}

bool HasItem(Blackboard* pBlackboard, eItemType type)
{
	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}
	bool hasItem{};
	ItemInfo info{};
	for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
	{
		if (pInterface->Inventory_GetItem(i, info))
		{
			if (info.Type == type)
			{
				hasItem = true;
			}
		}
	}
	if (hasItem)
	{
		return HasAmmo(pBlackboard, type);
	}

	return false;
}

bool IsHouseVisited(Blackboard* pBlackboard, const Elite::Vector2& houseCenter)
{
	vector<VisitedHouse>* pVisitedHouse;
	if (!pBlackboard->GetData("VisitedHouses", pVisitedHouse))
	{
		return false;
	}

	return std::find_if(pVisitedHouse->begin(), pVisitedHouse->end(), [&houseCenter](const VisitedHouse& house)
		{
			return house.houseCenter == houseCenter;
		}) != pVisitedHouse->end();

}

BehaviorState UseItem(Blackboard* pBlackboard, eItemType type)
{
	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return Failure;
	}

	ItemInfo info{};
	unsigned int itemSlot;
	for (itemSlot = 0; itemSlot < pInterface->Inventory_GetCapacity(); itemSlot++)
	{
		if (pInterface->Inventory_GetItem(itemSlot, info))
		{
			if (info.Type == type)
			{
				break;
			}
		}
	}

	if (!pInterface->Inventory_UseItem(itemSlot))
	{
		pInterface->Inventory_RemoveItem(itemSlot);
	}

	return Running;
}

bool HasAmmo(Blackboard* pBlackboard, eItemType type)
{
	IExamInterface* pInterface = nullptr;
	if (!pBlackboard->GetData("Interface", pInterface))
	{
		return false;
	}
	ItemInfo info;
	switch (type)
	{
	case eItemType::PISTOL:
		for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			if (!pInterface->Inventory_GetItem(i, info))
			{
				continue;
			}
			if (info.Type != eItemType::PISTOL)
			{
				continue;
			}
			if (pInterface->Weapon_GetAmmo(info) > 0)
			{
				return true;
			}
			else
			{
				pInterface->Inventory_RemoveItem(i);
				return false;
			}
		}
		break;
	case eItemType::MEDKIT:
		for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			if (!pInterface->Inventory_GetItem(i, info))
			{
				continue;
			}
			if (info.Type != eItemType::MEDKIT)
			{
				continue;
			}
			if (pInterface->Medkit_GetHealth(info) > 0)
			{
				return true;
			}
			else
			{
				pInterface->Inventory_RemoveItem(i);
				return false;
			}
		}
		break;
	case eItemType::FOOD:
		for (size_t i = 0; i < pInterface->Inventory_GetCapacity(); i++)
		{
			if (!pInterface->Inventory_GetItem(i, info))
			{
				continue;
			}
			if (info.Type != eItemType::FOOD)
			{
				continue;
			}
			if (pInterface->Food_GetEnergy(info) > 0)
			{
				return true;
			}
			else
			{
				pInterface->Inventory_RemoveItem(i);
				return false;
			}
		}
		break;
	default:
		return false;
		break;
	}
	return false;
}

bool operator==(const HouseInfo& h1, const HouseInfo& h2)
{
	if (h1.Center != h2.Center)
	{
		return false;
	}

	if (h1.Size != h2.Size)
	{
		return false;
	}

	return true;
}

bool operator==(const ItemInfo& i1, const ItemInfo& i2)
{
	return i1.Location ==i2.Location;
}