#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SteeringBehavior.h"
#include "Behaviors.h"
#include "Blackboard.h"
#include <iostream>
#include <algorithm>
//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);
	float worldDim = m_pInterface->World_GetInfo().Dimensions.x;
	m_pSteering = new Seek{};
	m_AgentInfo = m_pInterface->Agent_GetInfo();
	m_WorldState = WorldState{ m_pInterface->World_GetInfo().Dimensions.x, m_pInterface->World_GetInfo().Dimensions.y, 60, 5, 5 };

	InitBlackboard();
	InitBehaviorTree();

	srand(unsigned int(time(nullptr)));

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "BotNameTEST";
	info.Student_FirstName = "Maxime";
	info.Student_LastName = "Leriche";
	info.Student_Class = "2DAE02";
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.SpawnDebugPistol = false;
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	//Update blackboard
	m_AgentInfo = m_pInterface->Agent_GetInfo();
	m_VisibleEntities = GetEntitiesInFOV();
	m_VisibleHouses = GetHousesInFOV();
	UpdateVisitedHouses(dt);

	m_pBT->Update(dt);

	auto steering = SteeringPlugin_Output();

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	//auto agentInfo = m_pInterface->Agent_GetInfo();

	auto nextTargetPos = m_Target; //To start you can use the mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y <<  " ---EntityHash: " << e.EntityHash << "---Radius: "<< zoneInfo.Radius << std::endl;
		}
	}

	//INVENTORY USAGE DEMO
	//********************

	if (m_GrabItem)
	{
		ItemInfo item;
		//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
		//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
		//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
		//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
		if (m_pInterface->Item_Grab({}, item))
		{
			//Once grabbed, you can add it to a specific inventory slot
			//Slot must be empty
			m_pInterface->Inventory_AddItem(0, item);
		}
	}

	if (m_UseItem)
	{
		//Use an item (make sure there is an item at the given inventory slot)
		m_pInterface->Inventory_UseItem(0);
	}

	if (m_RemoveItem)
	{
		//Remove an item from a inventory slot
		m_pInterface->Inventory_RemoveItem(0);
	}

	if (m_pSteering)
	{
		steering = m_pSteering->CalculateSteering(dt, &m_AgentInfo);
		if (typeid(*m_pSteering) == typeid(CollisionAvoidance))
		{
			static_cast<CollisionAvoidance*>(m_pSteering)->Render(m_pInterface);
		}
	}

	steering.RunMode = m_AgentInfo.RunMode; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

								 //SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

								 //@End (Demo Purposes)
	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
	m_WorldState.Render(m_pInterface);
	//draw map boundaries
	Elite::Vector2* points{ new Elite::Vector2[4]{
		{m_pInterface->World_GetInfo().Center.x - m_pInterface->World_GetInfo().Dimensions.x / 2, m_pInterface->World_GetInfo().Center.y - m_pInterface->World_GetInfo().Dimensions.y / 2},
		{m_pInterface->World_GetInfo().Center.x - m_pInterface->World_GetInfo().Dimensions.x / 2, m_pInterface->World_GetInfo().Center.y + m_pInterface->World_GetInfo().Dimensions.y / 2},
		{m_pInterface->World_GetInfo().Center.x + m_pInterface->World_GetInfo().Dimensions.x / 2, m_pInterface->World_GetInfo().Center.y + m_pInterface->World_GetInfo().Dimensions.y / 2},
		{m_pInterface->World_GetInfo().Center.x + m_pInterface->World_GetInfo().Dimensions.x / 2, m_pInterface->World_GetInfo().Center.y - m_pInterface->World_GetInfo().Dimensions.y / 2}} 
	};
	m_pInterface->Draw_Polygon(points, 4, Elite::Vector3{ 1,0,0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::InitBlackboard()
{
	m_pBlackBoard = new Blackboard{};
	m_pBlackBoard->AddData("SteeringBehavior", &m_pSteering);//static_cast<SteeringBehavior*>(nullptr)
	m_pBlackBoard->AddData("VisibleEntities", &m_VisibleEntities);
	m_pBlackBoard->AddData("Target", &m_Target);
	m_pBlackBoard->AddData("ClosestEnemy", static_cast<EnemyInfo*>(nullptr));
	m_pBlackBoard->AddData("Agent", &m_AgentInfo);
	m_pBlackBoard->AddData("VisibleHouses", &m_VisibleHouses);
	m_pBlackBoard->AddData("Interface", m_pInterface);
	m_pBlackBoard->AddData("EntityToGrab", static_cast<EntityInfo*>(nullptr));
	m_pBlackBoard->AddData("VisitedHouses", &m_VisitedHouses);
	m_pBlackBoard->AddData("SeekedHouse", static_cast<HouseInfo*>(nullptr));
	m_pBlackBoard->AddData("PurgeZone", static_cast<PurgeZoneInfo*>(nullptr));
	m_pBlackBoard->AddData("WorldState", &m_WorldState);
}

void Plugin::InitBehaviorTree()
{
	m_pBT = new BehaviorTree{ m_pBlackBoard, new BehaviorSelector{
		{
			new BehaviorSequence{
			{
				new BehaviorConditional{AreItemsVisible},
				new BehaviorConditional{HasItemInGrabRange},
				new BehaviorConditional{HasInventoryFreeSpace},
				new BehaviorAction{GrabAndAddItem},
				new BehaviorAction{ReturnFailure}//return failure to continue executing the tree this frame
			}},
			new BehaviorSequence{//Sprint when bitten
			{
					new BehaviorConditional{WasBitten},
					new BehaviorConditional{HasStamina},
					new BehaviorAction{Sprint},
					new BehaviorAction{ReturnFailure}//return failure to continue executing the tree this frame
			}},
			new BehaviorSequence{//Stop sprinting when out of stamina
			{
				new BehaviorSelector{
				{
					new BehaviorConditional{HasStamina},
					new BehaviorAction{StopSprint},
				}},
				new BehaviorAction{ReturnFailure}//return failure to continue executing the tree this frame
			}},
			new BehaviorSelector{//heals/replenishes energy if needed and if possible
			{
				new BehaviorSequence{
				{
					new BehaviorConditional{NeedsHealing},
					new BehaviorSequence{
					{
						new BehaviorConditional{HasMedKit},
						new BehaviorAction{UseMedKit}
					}},
					new BehaviorConditional{NeedsEnergy},
					new BehaviorSequence{
					{
						new BehaviorConditional{HasFood},
						new BehaviorAction{EatFood},
						new BehaviorAction{ReturnFailure}//return failure to continue executing the tree this frame
					}},
				}},
				new BehaviorSequence{
				{
					new BehaviorConditional{NeedsEnergy},
					new BehaviorSequence{
					{
						new BehaviorConditional{HasFood},
						new BehaviorAction{EatFood},
						new BehaviorAction{ReturnFailure}//return failure to continue executing the tree this frame
					}},
				 }},
			 }},
			new BehaviorSequence{
			{
				new BehaviorConditional{IsCloseToCellCenter},
				new BehaviorAction{SetCellExplored},
				new BehaviorAction{ReturnFailure}//return failure to continue executing the tree this frame
			}},
			new BehaviorSelector{//if has gun and enemy visible, shoot
			{
				new BehaviorSequence{
				{
					new BehaviorConditional{HasGun},
					new BehaviorSequence{
					{
						new BehaviorConditional{GetClosestVisibleEnemyPosition},
						new BehaviorSelector{
						{
							new BehaviorSequence{
							{
								new BehaviorConditional{WillHit},
								new BehaviorAction{Shoot}
							}},
							new BehaviorAction{ChangeToFace}
						}},
					}},
				}},
			}},
			new BehaviorSequence{//flee from purge zones
			{
				new BehaviorConditional{IsPurgeZoneVisible},//Includes IsInPurgeZone check
				new BehaviorAction{ChangeToRunFrom}
			}},
			new BehaviorSelector{{
				new BehaviorSequence{//item seek, grab and add
				{
					new BehaviorConditional{AreItemsVisible},
					new BehaviorConditional{HasInventoryFreeSpace},
					new BehaviorAction{ChangeToSeek}
				}},
				new BehaviorSequence{
				{
					new BehaviorConditional{AreKnownItems},
					new BehaviorSequence{
					{
						new BehaviorConditional{HasInventoryFreeSpace},
						new BehaviorAction{ChangeToSeek}
					}},
				}},
			}},
			new BehaviorSequence{
			{
				new BehaviorSelector{//house related behavior
				{
					new BehaviorSequence{
					{
						new BehaviorConditional{IsHouseVisible},
						new BehaviorSelector{
						{
							new BehaviorSequence{
							{
								new BehaviorConditional{IsHouseNotVisited},
								new BehaviorSelector{
								{
									new BehaviorSequence{
									{
										new BehaviorConditional{IsCloseToHouseCenter},
										new BehaviorAction{SetHouseVisited}
									}},
									new BehaviorAction{ChangeToSeek}
								}},
							}},
							new BehaviorSequence{
							{
								new BehaviorConditional{IsInHouse},
								new BehaviorAction{LeaveHouse}
							}},
						}},
					}},
					new BehaviorSequence{
					{
						new BehaviorConditional{HasHouseTarget},
						new BehaviorAction{ChangeToSeek}
					}},
				}},
			}},
			new BehaviorAction{ Explore }
		}
	} };
}

void Plugin::UpdateVisitedHouses(float deltaTime)
{
	for (VisitedHouse& house : m_VisitedHouses)
	{
		house.timer += deltaTime;
	}

	m_VisitedHouses.erase(std::remove_if(m_VisitedHouses.begin(), m_VisitedHouses.end(), [](VisitedHouse& house)
		{
			return house.timer >= house.maxTimer;
		}), m_VisitedHouses.end());
}