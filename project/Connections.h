#pragma once
/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
// Authors: Yosha Vandaele
/*=============================================================================*/
// EGraphConnectionTypes.h: Various edge types for graphs
/*=============================================================================*/

class Connection
{
public:
	explicit Connection(int from = -1, int to = -1, float cost = 1.f);
	virtual ~Connection() = default;

	int GetFrom() const { return m_From; }
	void SetFrom(int newFrom) { m_From = newFrom; }

	int GetTo() const { return m_To; }
	void SetTo(int newTo) { m_To = newTo; }

	float GetCost() const { return m_Cost; }
	void SetCost(float newCost) { m_Cost = newCost; }

	bool IsValid() const { return (m_From != -1 && m_To != -1); }

	bool operator==(const Connection& rhs) const;
	bool operator!=(const Connection& rhs) const;

protected:
	int m_From;
	int m_To;

	// the cost of traversing the edge
	float m_Cost;
};