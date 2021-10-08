#include "stdafx.h"
#include "World.h"

WorldState::WorldState(float width, float height, float cellDimension, int nrCols, int nrRows)
	:WorldState(width, height, cellDimension, cellDimension, nrCols, nrRows)
{
}

WorldState::WorldState(float width, float height, float cellWidth, float cellHeight, int nrCols, int nrRows)
	:m_Width{ width },
	m_Height{ height },
	m_CellWidth{ cellWidth },
	m_CellHeight{ cellHeight },
	m_NrCols{ nrCols },
	m_NrRows{ nrRows },
	m_IsFullyExplored{}
{
	PartitionSpace();
	for (auto r = 0; r < m_NrRows; ++r)
	{
		for (auto c = 0; c < m_NrCols; ++c)
		{
			AddConnectionsToAdjacentCells(c, r);
		}
	}
}

WorldState& WorldState::operator=(WorldState&& rhs)
{
	m_Width = rhs.m_Width;
	m_Height = rhs.m_Height;
	m_CellWidth = rhs.m_CellWidth;
	m_CellHeight = rhs.m_CellHeight;
	m_NrCols = rhs.m_NrCols;
	m_NrRows = rhs.m_NrRows;
	m_IsFullyExplored = rhs.m_IsFullyExplored;

	if (!rhs.m_Cells.empty())
	{
		for (Cell* pCell : rhs.m_Cells)
		{
			m_Cells.push_back(pCell);
		}
		rhs.m_Cells.clear();
	}
	return *this;
}

WorldState::~WorldState()
{
	for (Cell* pCell : m_Cells)
	{
		delete pCell;
	}

	m_Cells.clear();
}

int WorldState::PositionToIndex(const Elite::Vector2& pos) const
{
	int col = int(pos.x + m_Width / 2) / int(m_CellWidth);
	int row = int(pos.y + m_Height / 2) / int(m_CellHeight);

	int index{ row * m_NrCols + col };

	if (index < 0 || index >= (int)m_Cells.size())
	{
		return -1;
	}

	return index;
}

void WorldState::PartitionSpace()
{
	float x{ -m_Width / 2 };
	float y{ -m_Height / 2 };

	for (int i = 0; i < m_NrCols; i++)
	{
		for (int j = 0; j < m_NrRows; j++)
		{
			m_Cells.push_back(new Cell{ x, y, m_CellWidth, m_CellHeight });
			x += m_CellWidth;
		}
		y += m_CellHeight;
		x = -m_Width / 2;
	}
}

void WorldState::Render(IExamInterface* pInterface) const
{
	Elite::Vector2 start{ -m_Width / 2, -m_Height / 2 };
	Elite::Vector2 end{ start.x, m_Height / 2 };

	for (int i = 0; i < m_NrCols; i++)
	{
		pInterface->Draw_Segment(start, end, { 1,0,0});
		end.x = start.x += m_CellWidth;
	}

	start.x = -m_Width / 2;
	start.y = -m_Height / 2;
	end.y = -m_Height / 2;
	end.x = m_Width / 2;

	for (int i = 0; i < m_NrRows; i++)
	{
		pInterface->Draw_Segment(start, end, { 1,0,0 });
		end.y = start.y += m_CellHeight;
	}
}

bool WorldState::IsWithinBounds(int col, int row) const
{
	return (col >= 0 && col < m_NrCols&& row >= 0 && row < m_NrRows);
}

void WorldState::AddConnectionsToAdjacentCells(int col, int row)
{
	int idx = row * m_NrCols + col;

	AddConnectionsInDirections(idx, col, row, m_StraightDirections);

	AddConnectionsInDirections(idx, col, row, m_DiagonalDirections);
}

void WorldState::AddConnectionsInDirections(int idx, int col, int row, vector<Elite::Vector2> directions)
{
	for (auto d : directions)
	{
		int neighborCol = col + (int)d.x;
		int neighborRow = row + (int)d.y;

		if (IsWithinBounds(neighborCol, neighborRow))
		{
			int neighborIdx = neighborRow * m_NrCols + neighborCol;

			m_Cells[idx]->ToConnections.push_back(neighborIdx);
		}
	}
}

bool WorldState::IsHouseKnown(HouseInfo* pHouse) const
{
	int idx = PositionToIndex(pHouse->Center);
	Cell* pCell = GetCell(idx);
	return pCell->IsHouseInCell(pHouse);
}

void WorldState::AddKnownHouse(HouseInfo* pHouse)
{
	if (!IsHouseKnown(pHouse))
	{
		int idx = PositionToIndex(pHouse->Center);
		Cell* pCell = GetCell(idx);
		pCell->Houses.push_back(pHouse);
	}
}

bool WorldState::IsItemKnown(const ItemInfo& item) const
{
	return std::find_if(m_KnownItems.begin(), m_KnownItems.end(), [&item](const ItemInfo& ent) {
		return ent == item;
		}) != m_KnownItems.end();
}

void WorldState::AddKnownItem(const ItemInfo& item)
{
	if (item.Location == Elite::Vector2{})
	{
		return;
	}
	if (!IsItemKnown(item))
	{
		m_KnownItems.push_back(item);
	}
}

void WorldState::RemoveKnownItem(const ItemInfo& item)
{
	if (IsItemKnown(item))
	{
		m_KnownItems.erase(std::remove(m_KnownItems.begin(), m_KnownItems.end(), item));
	}
}

Cell* WorldState::GetClosestUnexploredCell(const Elite::Vector2& pos) const
{
	Cell* pClosest{nullptr};

	if (AreAllCellsExplored())
	{
		return pClosest;
	}
	float distance{ FLT_MAX };
	std::for_each(m_Cells.begin(), m_Cells.end(), [&distance, &pos, &pClosest](Cell* pCell) {
		if (!pCell->IsExplored)
		{
			float newDistance{ (pos - pCell->Center).Magnitude() };
			if (newDistance < distance)
			{
				distance = newDistance;
				pClosest = pCell;
			}
		}
		});
	return pClosest;

	//Recursive search, causes stack overflow...
	//auto adjacentIndices{ GetAdjacentCells(PositionToIndex(pos)) };
	//float distance{ FLT_MAX };
	//std::for_each(adjacentIndices.begin(), adjacentIndices.end(), [this, &distance, &pos, &pClosest](int i) {
	//	if (!m_Cells[i]->IsExplored)
	//	{
	//		float newDistance{ (m_Cells[i]->Center - pos).Magnitude() };
	//		if (newDistance < distance)
	//		{
	//			distance = newDistance;
	//			pClosest = m_Cells[i];
	//		}
	//	}
	//	});
	//if (pClosest == nullptr)
	//{
	//	return GetClosestUnexploredCell(m_Cells[adjacentIndices[0]]->Center);
	//}
	//else
	//{
	//	return pClosest;
	//}
}

bool WorldState::AreAllCellsExplored() const
{
	return std::find_if(m_Cells.begin(), m_Cells.end(), [](Cell* pCell)
		{
			return !pCell->IsExplored;
		}) == m_Cells.end();
}

HouseInfo* WorldState::GetClosestUnvisitedHouse(const Elite::Vector2& pos, const std::vector<VisitedHouse>* pVisitedHouses) const
{
	auto adjacentIndices{ GetAdjacentCells(PositionToIndex(pos)) };
	HouseInfo* pClosest{ nullptr };

	float distance{ FLT_MAX };

	//for each house in each adjacent cell, check if the house is visited and if not compare its distance to the agent
	std::for_each(adjacentIndices.begin(), adjacentIndices.end(), [this, &pClosest, &distance, &pos, &pVisitedHouses](int i) {
		std::for_each(m_Cells[i]->Houses.begin(), m_Cells[i]->Houses.end(), [&pClosest, &distance, &pos, &pVisitedHouses](HouseInfo* pHouse) {
			if (std::find_if(pVisitedHouses->begin(), pVisitedHouses->end(), [&pHouse](const VisitedHouse& house)
				{
					return house.houseCenter == pHouse->Center;
				}) != pVisitedHouses->end())
			{
				float newDistance{ (pos - pHouse->Center).Magnitude() };
				if (newDistance < distance)
				{
					distance = newDistance;
					pClosest = pHouse;
				}
			}
			});
		});
	return pClosest;
}