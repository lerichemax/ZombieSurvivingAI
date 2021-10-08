#pragma once
#include "../inc/Exam_HelperStructs.h"

struct VisitedHouse
{
	Elite::Vector2 houseCenter;
	float timer;
	const float maxTimer{ 30.f };//time until a house will be unvisited again

	VisitedHouse(HouseInfo* pHouseInfo);
	VisitedHouse(const HouseInfo& houseInfo);
	VisitedHouse(const Elite::Vector2& center);
	VisitedHouse& operator=(const VisitedHouse& house);
};
