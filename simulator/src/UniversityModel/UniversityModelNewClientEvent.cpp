
#include "UniversityModelNewClientEvent.hpp"
#include "UniversityModel.hpp"
#include "UniversityModelTimeStepping.hpp"
#include "UniversityModelClientSelector.hpp"
#include "../NewClientEvent.hpp"
#include "UniversityModelClient.hpp"


#include <ctime>
#include <vector>

UniversityModelNewClientEvent::UniversityModelNewClientEvent(const tm& timestamp, TimeStepping* timeStepping,
		unsigned int profileID, bool isMiner, int numInitialAddresses, btc initialCoins, UniversityModelClient umC):
		NewClientEvent(timestamp, timeStepping, profileID, isMiner, numInitialAddresses, initialCoins),
		UniversityModelClientSelector(umC)
{
}

bool UniversityModelNewClientEvent::executeInternal(BitcoinNetwork* network)
{
	bool retVal = NewClientEvent::executeInternal(network);
	getClientRepresentation().set_ID(IDofNewClient_);
	return retVal;
}

UniversityModelNewClientEvent::~UniversityModelNewClientEvent() {
}

std::ostream& UniversityModelNewClientEvent::logOutputInternal(std::ostream& o) const
{
	o << "New client created: " << "id=" << IDofNewClient_ << ", Miner=" << isMiner_
	  << ", numFriends=" << numFriends_ << ", numInitialAddresses=" << numInitialAddresses_
	  << ", profileID=" << profileID_ << ", initialCoins=" << initialCoins_ << ", isSeller=" << getClientRepresentation().get_isSeller()
	  << std::endl << getClientRepresentation();

	return o;
}

