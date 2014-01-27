

#ifndef UNIVERSITYMODELISSUETRANSACTIONEVENT_HPP_
#define UNIVERSITYMODELISSUETRANSACTIONEVENT_HPP_

#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../IssueTransactionEvent.hpp"
#include "UniversityModelClient.hpp"
#include "UniversityModelClientSelector.hpp"

#include <ctime>
#include <vector>

class UniversityModelTimeStepping;

class UniversityModelIssueTransactionEvent : public IssueTransactionEvent, public UniversityModelClientSelector {
public:
	UniversityModelIssueTransactionEvent(const tm& timestamp,
			UniversityModelTimeStepping* timeStepping, btc amount, unsigned int sender, unsigned int receiver,
			unsigned int umcPos, int eventID);

	UniversityModelIssueTransactionEvent(const tm& timestamp,
			UniversityModelTimeStepping* timeStepping, btc amount, unsigned int sender, unsigned int receiver,
			int eventID);

	virtual ~UniversityModelIssueTransactionEvent();
	virtual bool executeInternal(BitcoinNetwork* network);

	inline bool isNotBitcoinPurchase(void) const { return isNotBitcoinPurchase_; }
protected:
	bool receiverAddressIsSet_;
	bool isNotBitcoinPurchase_;
};

#endif /* UNIVERSITYMODELISSUETRANSACTIONEVENT_HPP_ */
