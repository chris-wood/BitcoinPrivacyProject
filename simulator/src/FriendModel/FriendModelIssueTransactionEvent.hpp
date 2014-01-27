

#ifndef FRIENDMODELISSUETRANSACTIONEVENT_HPP_
#define FRIENDMODELISSUETRANSACTIONEVENT_HPP_

#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../IssueTransactionEvent.hpp"

#include <ctime>
#include <vector>


class FriendModelIssueTransactionEvent : public IssueTransactionEvent {
public:
	FriendModelIssueTransactionEvent(const tm& timestamp, TimeStepping* timeStepping);
	virtual ~FriendModelIssueTransactionEvent();
	bool executeInternal(BitcoinNetwork* network);
};

#endif /* FRIENDMODELISSUETRANSACTIONEVENT_HPP_ */
