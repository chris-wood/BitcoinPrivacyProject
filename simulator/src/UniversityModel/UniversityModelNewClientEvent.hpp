

#ifndef UNIVERSITYMODELNEWCLIENTEVENT_HPP_
#define UNIVERSITYMODELNEWCLIENTEVENT_HPP_

#include "../BitcoinNetwork.hpp"
#include "../NewClientEvent.hpp"
#include "UniversityModelClient.hpp"
#include "UniversityModelClientSelector.hpp"
#include "../TimeStepping.hpp"

#include <ctime>
#include <vector>


class UniversityModelNewClientEvent : public NewClientEvent, public UniversityModelClientSelector {
public:
	UniversityModelNewClientEvent(const tm& timestamp, TimeStepping* timeStepping,
			unsigned int profileID, bool isMiner, int numInitialAddresses, btc initialCoins, UniversityModelClient umC);
	virtual ~UniversityModelNewClientEvent();
	bool executeInternal(BitcoinNetwork* network);
	std::ostream& logOutputInternal(std::ostream& o) const;
};

#endif /* UNIVERSITYMODELNEWCLIENTEVENT_HPP_ */
