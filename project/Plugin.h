#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "VisitedHouse.h"
#include "World.h"

class IBaseInterface;
class IExamInterface;
class Blackboard;
class BehaviorTree;
class SteeringBehavior;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	
	Elite::Vector2 m_Target = {};
	SteeringBehavior* m_pSteering = nullptr;
	Blackboard* m_pBlackBoard = nullptr;
	BehaviorTree* m_pBT = nullptr;
	AgentInfo m_AgentInfo{};
	WorldState m_WorldState;
	vector<EntityInfo> m_VisibleEntities;
	vector<HouseInfo> m_VisibleHouses;
	vector<VisitedHouse> m_VisitedHouses;


	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	void InitBlackboard();
	void InitBehaviorTree();
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;
	void UpdateVisitedHouses(float deltaTime);
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}