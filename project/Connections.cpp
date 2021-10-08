#include "stdafx.h"
#include "Connections.h"

Connection::Connection(int from, int to, float cost)
	: m_From(from)
	, m_To(to)
	, m_Cost(cost)
{
}

bool Connection::operator==(const Connection& rhs) const
{
	return rhs.m_From == this->m_From &&
		rhs.m_To == this->m_To &&
		rhs.m_Cost == this->m_Cost;
}

bool Connection::operator!=(const Connection& rhs) const
{
	return !(*this == rhs);
}