#pragma once
#include "BehaviorTree.h"
#include "Blackboard.h"
#include "../inc/Exam_HelperStructs.h"
#include "../inc/IExamInterface.h"

//CONDITIONS
bool AreEnemiesVisible(Blackboard* pBlackboard);

bool GetClosestVisibleEnemyPosition(Blackboard* pBlackboard); //if enemies are visible, update the blackboard with the closest one and return true

bool IsHouseVisible(Blackboard* pBlackboard);

bool AreItemsVisible(Blackboard* pBlackboard);

bool AreKnownItems(Blackboard* pBlackboard);

bool HasItemInGrabRange(Blackboard* pBlackboard);

bool HasGun(Blackboard* pBlackboard);

bool HasNoGun(Blackboard* pBlackboard);

bool WillHit(Blackboard* pBlackboard);

bool HasMedKit(Blackboard* pBlackboard);

bool WasBitten(Blackboard* pBlackboard);

bool NeedsHealing(Blackboard* pBlackboard);

bool HasFood(Blackboard* pBlackboard);

bool NeedsEnergy(Blackboard* pBlackboard);

bool HasInventoryFreeSpace(Blackboard* pBlackboard);

bool IsInHouse(Blackboard* pBlackboard);

bool IsHouseNotVisited(Blackboard* pBlackboard);

bool IsCloseToHouseCenter(Blackboard* pBlackboard);

bool HasHouseTarget(Blackboard* pBlackboard);

bool HasStamina(Blackboard* pBlackboard);

bool IsPurgeZoneVisible(Blackboard* pBlackboard);

bool IsInPurgeZone(Blackboard* pBlackboard);

bool IsCloseToCellCenter(Blackboard* pBlackboard);


//BEHAVIORS
//seek highest influence //later

BehaviorState ChangeToRunFrom(Blackboard* pBlackboard);

BehaviorState ChangeToSeek(Blackboard* pBlackboard);

BehaviorState GrabAndAddItem(Blackboard* pBlackboard);

BehaviorState Shoot(Blackboard* pBlackboard);

BehaviorState UseMedKit(Blackboard* pBlackboard);

BehaviorState EatFood(Blackboard* pBlackboard);

BehaviorState ChangeToFace(Blackboard* pBlackboard);

BehaviorState SetHouseVisited(Blackboard* pBlackboard);

BehaviorState LeaveHouse(Blackboard* pBlackboard);

BehaviorState Sprint(Blackboard* pBlackboard);

BehaviorState StopSprint(Blackboard* pBlackboard);

BehaviorState ReturnFailure(Blackboard* pBlackboard);

BehaviorState Explore(Blackboard* pBlackboard);

BehaviorState SetCellExplored(Blackboard* pBlackboard);

//HELPERS

std::vector<EnemyInfo*> GetVisibleEnemiesInfo(IExamInterface* pInterface, std::vector<EntityInfo>* pEntities);

bool GetVisibleItemTypePosition(Blackboard* pBlackboard, eItemType info);

bool HasInventoryFreeSlots(Blackboard* pBlackboard, unsigned int& firstFreeSlot);

bool HasItem(Blackboard* pBlackboard, eItemType type);

bool IsHouseVisited(Blackboard* pBlackboard, const Elite::Vector2& houseCenter);

BehaviorState UseItem(Blackboard* pBlackboard, eItemType type);

bool HasAmmo(Blackboard* pBlackboard, eItemType type);

bool operator==(const HouseInfo& h1, const HouseInfo& h2);

bool operator==(const ItemInfo& i1, const ItemInfo& i2);