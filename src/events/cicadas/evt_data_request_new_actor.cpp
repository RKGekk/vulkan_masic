#include "evt_data_request_new_actor.h"
#include "../../tools/string_tools.h"

const std::string EvtData_Request_New_Actor::sk_EventName = "EvtData_Request_New_Actor";

EvtData_Request_New_Actor::EvtData_Request_New_Actor() {
	m_actor_name = "";
	m_serverActorId = -1;
}

EvtData_Request_New_Actor::EvtData_Request_New_Actor(std::string actor_name, const ActorId serverActorId) {
	m_actor_name = actor_name;
	m_serverActorId = serverActorId;
}

EventTypeId EvtData_Request_New_Actor::VGetEventType() const {
	return sk_EventType;
}

void EvtData_Request_New_Actor::VDeserialize(std::istream& in) {
	in >> m_actor_name;
	in >> m_serverActorId;
}

IEventDataPtr EvtData_Request_New_Actor::VCopy() const {
	return IEventDataPtr(new EvtData_Request_New_Actor(m_actor_name, m_serverActorId));
}

void EvtData_Request_New_Actor::VSerialize(std::ostream& out) const {
	out << m_actor_name << " ";
	out << m_serverActorId << " ";
}

const std::string& EvtData_Request_New_Actor::GetName() const {
	return sk_EventName;
}

const std::string& EvtData_Request_New_Actor::GetActorName() const {
	return m_actor_name;
}

const ActorId EvtData_Request_New_Actor::GetServerActorId() const {
	return m_serverActorId;
}

std::ostream& operator<<(std::ostream& os, const EvtData_Request_New_Actor& evt) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "Event type id: " << evt.sk_EventType << std::endl;
	os << "Event name: " << evt.sk_EventName << std::endl;
	os << "Event time stamp: " << evt.GetTimeStamp().time_since_epoch().count() << "ns" << std::endl;
	os << "Event server actorId id: " << evt.m_serverActorId << std::endl;
	os << "Event actor name: " << evt.m_actor_name << std::endl;

	os.flags(oldFlag);
	return os;
}