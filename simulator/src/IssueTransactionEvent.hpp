#ifndef ISSUETRANSACTIONEVENT_HPP_
#define ISSUETRANSACTIONEVENT_HPP_

#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"

#include <ctime>
#include <vector>

class IssueTransactionEvent : public NetworkEvent {
public:
	IssueTransactionEvent(const tm& timestamp,
								TimeStepping* timeStepping, btc amount,
								unsigned int sender, unsigned int receiver,
//								const std::vector<unsigned int>& senderAddresses,
								unsigned int receiverAddress, int eventID);
	virtual ~IssueTransactionEvent();
protected:
	IssueTransactionEvent(const tm& timestamp, TimeStepping* timeStepping);
	void setFields(btc amount, unsigned int sender, unsigned int receiver,
//								const std::vector<unsigned int>& senderAddresses,
								unsigned int receiverAddress);
	virtual bool executeInternal(BitcoinNetwork* network);
	virtual std::ostream& logOutputInternal(std::ostream& o) const;

	btc amount_;
	unsigned int sender_;
	unsigned int receiver_;
	//std::vector<unsigned int> senderAddresses_;
	unsigned int receiverAddress_;
	std::string errorExplanation_;
	int eventID_;
};

#endif /* ISSUETRANSACTIONEVENT_HPP_ */
