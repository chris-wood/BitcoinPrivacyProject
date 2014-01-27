#include "Client.hpp"
#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "NewBitcoinSellerEvent.hpp"
#include "Sampler.hpp"
#include "ParameterList.hpp"

#include <ctime>
#include <string>
#include <sstream>
#include <iostream>


bool NewBitcoinSellerEvent::executeInternal(BitcoinNetwork* network)
{
	IDofNewClient_ = network->generateNewBitcoinSeller(profileID_, numInitialAddresses_);
	return true;
}

NewBitcoinSellerEvent::NewBitcoinSellerEvent(const tm& timestamp, TimeStepping* timeStepping,
					unsigned int profileID, int numInitialAddresses):
		NetworkEvent(timestamp, "NewBitcoinSellerEvent", timeStepping),
		profileID_(profileID),
		numInitialAddresses_(numInitialAddresses)
{
}

NewBitcoinSellerEvent::NewBitcoinSellerEvent(const tm& timestamp, TimeStepping* timeStepping):
		NetworkEvent(timestamp, "NewBitcoinSellerEvent", timeStepping),
		profileID_(0),
		numInitialAddresses_(0)
{
}

void NewBitcoinSellerEvent::setFields(unsigned int profileID, int numInitialAddresses)
{
	profileID_ = profileID;
	numInitialAddresses_ = numInitialAddresses;
}

NewBitcoinSellerEvent::~NewBitcoinSellerEvent() {
}

std::ostream& NewBitcoinSellerEvent::logOutputInternal(std::ostream& o) const
{
	o << "New BitcoinSeller created: " << "id=" << IDofNewClient_
	  << ", numInitialAddresses=" << numInitialAddresses_
	  << ", profileID=" << profileID_;

	return o;
}

