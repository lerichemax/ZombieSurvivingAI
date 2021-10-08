#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include "Behaviors.h"
#include "VisitedHouse.h"


struct Cell
{
	Elite::Vector2 Center;
	float Width;
	float Height;
	bool IsExplored;
	std::vector<HouseInfo*> Houses;
	std::vector<int> ToConnections;

	Cell() = default;
	Cell(float x, float y, float width, float height)
		:Center{x + width/2, y + height/2},
		Width{width},
		Height{height},
		IsExplored{}
	{
	}

	Cell(const Cell& cell)
		:Center{cell.Center},
		Width{cell.Width},
		Height{cell.Height},
		IsExplored{cell.IsExplored},
		Houses{cell.Houses},
		ToConnections{cell.ToConnections}
	{
	}

	bool IsHouseInCell(HouseInfo* pHouse)
	{
		return std::find_if(Houses.begin(), Houses.end(), [&pHouse](HouseInfo* pH)
			{
				return *pHouse == *pH;
			}) != Houses.end();
	}
};

class WorldState
{
public:
	WorldState() = default;
	WorldState(float width, float height, float cellDimension, int nrCols, int nrRows);
	WorldState(float width, float height, float cellWidth, float cellHeight, int nrCols, int nrRows);
	WorldState(const WorldState& other) = delete;
	WorldState(WorldState&& other) = delete;
	WorldState& operator=(const WorldState& rhs) = delete;
	WorldState& operator=(WorldState&& rhs);
	~WorldState();

	int PositionToIndex(const Elite::Vector2& pos) const;
	void Render(IExamInterface* pInterface) const;
	bool IsWithinBounds(int col, int row) const;
	bool IsCellExplored(int index) const { return m_Cells[index]->IsExplored; }
	Cell* GetCell(int idx) const { return m_Cells[idx]; }
	Elite::Vector2 GetCellCenter(int idx) const { return m_Cells[idx]->Center;}
	std::vector<int> GetAdjacentCells(int idx) const { return m_Cells[idx]->ToConnections; }
	void SetExplored(int idx) { m_Cells[idx]->IsExplored = true; }
	void AddKnownHouse(HouseInfo* pHouse);
	std::vector<HouseInfo*> GetCellHouses(int cellIdx) const { return m_Cells[cellIdx]->Houses; }
	bool DoesCellHasHouses(int cellIdx) const { return (m_Cells[cellIdx]->Houses.size() > 0 ? true : false); }
	bool AreKnownItems() const { return (m_KnownItems.size() > 0 ? true : false); }
	void AddKnownItem(const ItemInfo& item);
	void RemoveKnownItem(const ItemInfo& item);
	std::vector<ItemInfo> GetKnownItems() const { return m_KnownItems; }
	bool IsAgentExploring() const { return m_IsAgentExploring; }
	void SetIsAgentExploring(bool isExbloring) { m_IsAgentExploring = isExbloring; }
	Cell* GetClosestUnexploredCell(const Elite::Vector2& pos) const;
	HouseInfo* GetClosestUnvisitedHouse(const Elite::Vector2& pos, const std::vector<VisitedHouse>* pVisitedHouses) const;
private:
	const vector<Elite::Vector2> m_StraightDirections = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	const vector<Elite::Vector2> m_DiagonalDirections = { { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } };

	float m_Width;
	float m_Height;
	float m_CellWidth;
	float m_CellHeight;

	std::vector<Cell*> m_Cells;
	std::vector<ItemInfo> m_KnownItems;
	int m_NrCols;
	int m_NrRows;

	bool m_IsFullyExplored;
	bool m_IsAgentExploring;

	void PartitionSpace();
	void AddConnectionsToAdjacentCells(int col, int row);
	void AddConnectionsInDirections(int idx, int col, int row, vector<Elite::Vector2> directions);
	bool IsHouseKnown(HouseInfo* pHouse) const;
	bool IsItemKnown(const ItemInfo& item) const;
	bool AreAllCellsExplored() const;
};