#ifndef NEWBITCOINSELLEREVENT_HPP_
#define NEWBITCOINSELLEREVENT_HPP_

#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "Client.hpp"

#include <ctime>

class NewBitcoinSellerEvent : public NetworkEvent {
public:
	NewBitcoinSellerEvent(const tm& timestamp, TimeStepping* timeStepping,
					unsigned int profileID, int numInitialAddresses);
	virtual ~NewBitcoinSellerEvent();
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
	NewBitcoinSellerEvent(const tm& timestamp, TimeStepping* timeStepping);
	void setFields(unsigned int profileID, int numInitialAddresses);

	virtual bool executeInternal(BitcoinNetwork* network);
	virtual std::ostream& logOutputInternal(std::ostream& o) const;

	unsigned int profileID_;
	int numInitialAddresses_;
	unsigned int IDofNewClient_;
};


#endif /* NEWBITCOINSELLEREVENT_HPP_ */
