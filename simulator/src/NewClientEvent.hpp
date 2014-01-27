#ifndef NEWCLIENTEVENT_HPP_
#define NEWCLIENTEVENT_HPP_

#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "Client.hpp"

#include <ctime>

class NewClientEvent : public NetworkEvent {
public:
	NewClientEvent(const tm& timestamp, TimeStepping* timeStepping,
					unsigned int profileID, bool isMiner, int numInitialAddresses, btc initialCoins);
	virtual ~NewClientEvent();
	unsigned int getIDofNewClient(void) const
	{
		assert(executed());
		return IDofNewClient_;
	}
	unsigned int getProfileIDofNewClient(void) const
	{
		assert(executed());
		return profileID_;
	}
protected:
	NewClientEvent(const tm& timestamp, TimeStepping* timeStepping);
	void setFields(unsigned int profileID, bool isMiner, int numInitialAddresses, btc initialCoins);

	virtual bool executeInternal(BitcoinNetwork* network);
	virtual std::ostream& logOutputInternal(std::ostream& o) const;

	unsigned int profileID_;
	bool isMiner_;
	int numInitialAddresses_;
	int numFriends_;
	btc initialCoins_;
	std::vector<unsigned int> friends_;

	unsigned int IDofNewClient_;
};


#endif /* NEWCLIENTEVENT_HPP_ */
