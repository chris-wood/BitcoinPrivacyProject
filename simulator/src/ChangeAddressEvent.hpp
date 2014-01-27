#ifndef CHANGEADDRESSEVENT_HPP_
#define CHANGEADDRESSEVENT_HPP_

#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"

#include <ctime>

class AddAddressEvent : public NetworkEvent {
public:
	AddAddressEvent(const tm& timestamp, TimeStepping* timeStepping);
	virtual ~AddAddressEvent();
protected:
	bool executeInternal(BitcoinNetwork* network);
	std::ostream& logOutputInternal(std::ostream& o) const;
private:
	unsigned int client_;
	unsigned int affectedAddress_;
};

#endif /* CHANGEADDRESSEVENT_HPP_ */
