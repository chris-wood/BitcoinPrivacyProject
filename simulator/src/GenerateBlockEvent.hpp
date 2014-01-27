#ifndef GENERATEBLOCKEVENT_HPP_
#define GENERATEBLOCKEVENT_HPP_

#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"

#include <ctime>

class GenerateBlockEvent : public NetworkEvent {
public:
	GenerateBlockEvent(const tm& timestamp, TimeStepping* timeStepping);
	virtual ~GenerateBlockEvent();
protected:
	bool executeInternal(BitcoinNetwork* network);
	std::ostream& logOutputInternal(std::ostream& o) const;
private:
	unsigned int luckyMiner_;
	unsigned int transactionsIncluded_;
};

#endif /* GENERATEBLOCKEVENT_HPP_ */
