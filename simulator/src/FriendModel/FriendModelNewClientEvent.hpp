

#ifndef FRIENDMODELNEWCLIENTEVENT_HPP_
#define FRIENDMODELNEWCLIENTEVENT_HPP_

#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../NewClientEvent.hpp"

#include <ctime>
#include <vector>

class FriendModelNewClientEvent : public NewClientEvent {
public:
	FriendModelNewClientEvent(const tm& timestamp, TimeStepping* timeStepping);
	virtual ~FriendModelNewClientEvent();
	bool executeInternal(BitcoinNetwork* network);
};

#endif /* FRIENDMODELNEWCLIENTEVENT_HPP_ */
