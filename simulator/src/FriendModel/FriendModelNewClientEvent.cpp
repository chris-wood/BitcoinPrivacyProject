
#include "FriendModelNewClientEvent.hpp"
#include "../NewClientEvent.hpp"

FriendModelNewClientEvent::FriendModelNewClientEvent(const tm& timestamp, TimeStepping* timeStepping):
		NewClientEvent(timestamp, timeStepping)
{
}

bool FriendModelNewClientEvent::executeInternal(BitcoinNetwork* network)
{
	profileID_ = 0;

	// TODO: how to sample this probability?
	double minerProb = ParameterList::getParameterByName<double>("minerProb");
	isMiner_ = (Sampler::rand01() < minerProb) ? 1 : 0;

	numInitialAddresses_ = Sampler::randRange(10) + 1;
	initialCoins_ = Sampler::rand01() * Sampler::randRange(1, 51);

	numFriends_ = 0;
	if(network->numClients(false) > 0) numFriends_ = Sampler::randRange(15) + 1;

	friends_ = Sampler::randRangeVect(0, network->numClients(false), numFriends_);

	return NewClientEvent::executeInternal(network);
}

FriendModelNewClientEvent::~FriendModelNewClientEvent() {
}

