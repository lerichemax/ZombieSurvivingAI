/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
// Authors: Yosha Vandaele
/*=============================================================================*/
// EGraphNodeTypes.h: Various node types for graphs
/*=============================================================================*/
#pragma once
#include "../inc/EliteMath/EMath.h"
#include "InfluenceGraph.h"

class InfluenceNode final
{
public:
	InfluenceNode(int index, Elite::Vector2 pos = { 0,0 }, float influence = 0.f)
		: m_Index{ index }, m_Position{ pos }, m_Color{ 0.7f,0.7f,0.7f }, m_Influence(influence) {}

	virtual ~InfluenceNode() = default;

	int GetIndex() const { return m_Index; }
	void SetIndex(int newIdx) { m_Index = newIdx; }

	Elite::Vector2 GetPosition() const { return m_Position; }
	void SetPosition(const Elite::Vector2 & newPos) { m_Position = newPos; }

	Color GetColor() const { return m_Color; }
	void SetColor(const Color& color) { m_Color = color; }

	bool operator==(const InfluenceNode& rhs) { return m_Index == rhs.m_Index; }

	float GetInfluence() const { return m_Influence; }
	void SetInfluence(float influence) { m_Influence = influence; }

private:
	float m_Influence;
	int m_Index;
	Elite::Vector2 m_Position;
	Color m_Color;
};