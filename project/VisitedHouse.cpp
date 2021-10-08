#include "stdafx.h"
#include "VisitedHouse.h"


VisitedHouse::VisitedHouse(HouseInfo* pHouseInfo)
	:houseCenter{ pHouseInfo->Center },
	timer{}
{
}

VisitedHouse::VisitedHouse(const HouseInfo& houseInfo)
	:houseCenter{ houseInfo.Center },
	timer{}
{
}

VisitedHouse::VisitedHouse(const Elite::Vector2& center)
	:houseCenter{ center },
	timer{}
{
}

VisitedHouse& VisitedHouse::operator=(const VisitedHouse& house)
{
	houseCenter = house.houseCenter;
	timer = house.timer;
	return *this;
}