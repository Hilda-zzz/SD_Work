#include "ActorHandle.hpp"

const ActorHandle ActorHandle::INVALID=ActorHandle(MAX_ACTOR_UID, MAX_ACTOR_INDEX);

ActorHandle::ActorHandle():m_data(INVALID.m_data)
{
}

ActorHandle::ActorHandle(unsigned int uid, unsigned int index)
{
	if (uid > MAX_ACTOR_UID)
	{
		uid = MAX_ACTOR_UID;
	}
	if (index > MAX_ACTOR_INDEX)
	{
		index = MAX_ACTOR_INDEX;
	}
	m_data = uid << 16 | (0x0000FFFF & index);
}

bool ActorHandle::IsValid() const
{
	return *this != INVALID;
}

unsigned int ActorHandle::GetIndex() const
{
	return m_data & 0x0000FFFF;
}

bool ActorHandle::operator==(const ActorHandle& other) const
{
	return m_data == other.m_data;
}

bool ActorHandle::operator!=(const ActorHandle& other) const
{
	return m_data != other.m_data;
}
