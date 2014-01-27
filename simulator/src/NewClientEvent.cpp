#include "Client.hpp"
#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "NewClientEvent.hpp"
#include "Sampler.hpp"
#include "ParameterList.hpp"

#include <ctime>
#include <string>
#include <sstream>
#include <iostream>


bool NewClientEvent::executeInternal(BitcoinNetwork* network)
{
	IDofNewClient_ = network->generateNewClient(isMiner_, profileID_, numInitialAddresses_, friends_, initialCoins_);
	return true;
}

NewClientEvent::NewClientEvent(const tm& timestamp, TimeStepping* timeStepping,
					unsigned int profileID, bool isMiner, int numInitialAddresses, btc initialCoins):
		NetworkEvent(timestamp, "NewClientEvent", timeStepping),
		profileID_(profileID),
		isMiner_(isMiner),
		numInitialAddresses_(numInitialAddresses),
		numFriends_(0),
		initialCoins_(initialCoins),
		friends_()
{
}

NewClientEvent::NewClientEvent(const tm& timestamp, TimeStepping* timeStepping):
		NetworkEvent(timestamp, "NewClientEvent", timeStepping),
		profileID_(0),
		isMiner_(0),
		numInitialAddresses_(0),
		numFriends_(0),
		initialCoins_(0.0),
		friends_()
{
}

void NewClientEvent::setFields(unsigned int profileID, bool isMiner, int numInitialAddresses, btc initialCoins)
{
	profileID_ = profileID;
	isMiner_ = isMiner;
	numInitialAddresses_ = numInitialAddresses;
	initialCoins_ = initialCoins;
}

NewClientEvent::~NewClientEvent() {
}

std::ostream& NewClientEvent::logOutputInternal(std::ostream& o) const
{
	o << "New client created: " << "id=" << IDofNewClient_ << ", Miner=" << isMiner_
	  << ", numFriends=" << numFriends_ << ", numInitialAddresses=" << numInitialAddresses_
	  << ", profileID=" << profileID_ << ", initialCoins=" << initialCoins_ << ", Friends=";
	for(unsigned int i = 0; i<friends_.size(); ++i)
		o << friends_[i] << ",";

	return o;
}

