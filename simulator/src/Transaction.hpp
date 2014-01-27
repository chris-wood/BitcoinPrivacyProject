#ifndef TRANSACTION_HPP_
#define TRANSACTION_HPP_


#include "main.hpp"
#include "Client.hpp"
#include "Sampler.hpp"

#include <sstream>
#include <vector>
#include <ctime>
#include <string>



using namespace std;

//forward definition
class Address;


class Transaction
{
private:
	// the addresses of the sender and recipient with the corresponding amount of coins (after this transaction took place)
	vector<Address> senders_;
	Address recipient_;
	Address shadow_;

	tm timestampBroadcast_;
	tm timestampLastInclusion_;
	tm timestampFirstInclusion_;

	btc totalAmountIN_;
	btc totalAmountOUT_;

	bool isAccepted_;
	bool isShadowAddrNeeded_;
	bool isAppearedInLog;

	unsigned int senderID_;
	unsigned int receiverID_;

	unsigned int blockInclusionCount_;
	unsigned int id_;
	long transactionOrderingPosition_;

	bool amountsCalculated_;

	int eventID_;

public:
	// direct accept (only for block generation transactions)
	inline void accept(const tm& timestamp, unsigned long orderPos)
	{
		assert(senderID_ == GENERATION);
		set_transactionOrderPos(orderPos);
		assert(!isAccepted_);
		assert(!isAppearedInLog);
		assert(blockInclusionCount_ == 0);

		assert(Sampler::tm1BeforeORsmultanousWithTm2(timestampBroadcast_, timestamp));

		timestampFirstInclusion_ = timestampLastInclusion_ = timestamp;
		blockInclusionCount_ = 1;
		isAppearedInLog = true;
		isAccepted_ = true;
	}

	inline bool get_accepted(void) const { return isAccepted_; }
	inline bool get_appeared(void) const { return isAppearedInLog; }

	inline void set_transactionOrderPos(unsigned long orderPos)
	{
		assert(!isAppearedInLog);
		assert(!isAccepted_);
		assert(transactionOrderingPosition_ < 0); // -1 is not set
		transactionOrderingPosition_ = orderPos;
	}

	// only for non block generation reward transactions
	inline void increaseBlockInclusionCount(const tm& timestamp)
	{
		assert(senderID_ != GENERATION);
		assert(transactionOrderingPosition_ >= 0); // must not be -1
		unsigned int blocksNeeded = ParameterList::getParameterByName<unsigned int>("blocksNeeded");
		assert(blockInclusionCount_ < blocksNeeded && blockInclusionCount_ >= 0);
		assert(!isAccepted_);

		assert(Sampler::tm1BeforeORsmultanousWithTm2(timestampBroadcast_, timestamp));

		if(blockInclusionCount_ == 0)
		{
			timestampFirstInclusion_ = timestamp; // store first time the transaction appeared
			isAppearedInLog = true;
		}

		assert(isAppearedInLog);

		assert(Sampler::tm1BeforeORsmultanousWithTm2(timestampFirstInclusion_, timestamp));
		timestampLastInclusion_ = timestamp;
		blockInclusionCount_++;

		if(blockInclusionCount_ == blocksNeeded)
		{
			isAccepted_ = true;
		}
		assert(isAppearedInLog);
	}


	inline int get_eventID(void) const								{ return eventID_; }

	inline void set_shadowAddrNeeded(bool sAn)						{ isShadowAddrNeeded_ = sAn; }
	inline bool get_shadowAddrNeeded(void) const					{ return isShadowAddrNeeded_; }

	inline unsigned int get_senderID(void) const					{ return senderID_; }
	inline void set_senderID(unsigned int sID)						{ senderID_ = sID; }

	inline unsigned int get_receiverID(void) const					{ return receiverID_; }
	inline void set_receiverID(unsigned int rID)					{ receiverID_ = rID; }

	inline tm get_broadcastTimestamp(void) const					{ return timestampBroadcast_; }
	inline tm get_appearedTimestamp(void) const					{ assert(isAppearedInLog); return timestampFirstInclusion_; }

	inline unsigned int get_id(void) const							{ return id_; }

	const vector<Address>& get_senders() const;
	//void set_senders(const vector<Address> &new_senders);

	const Address& get_shadow() const								{ assert(isShadowAddrNeeded_); return shadow_; }
	//void set_shadow(const Address &new_recipient)					{ shadow_ = new_recipient; }

	const Address& get_recipient() const							{ return recipient_; }
	//void set_recipient(const Address &new_shadow)					{ recipient_ = new_shadow; }

	/*
	const vector<Address>& get_recipients() const;
	void set_recipients(const vector<Address> &new_recipients);
	*/

	inline btc get_totalINamount(void) const						{ assert(amountsCalculated_); return totalAmountIN_; }
	inline btc get_totalOUTamount(void) const						{ assert(amountsCalculated_); return totalAmountOUT_; }

	explicit Transaction(const tm &broadcastTimestamp, unsigned int id, int eventID);
	~Transaction();
	//Transaction(const Transaction &c);
	//const Transaction &operator=(const Transaction &c);


	void addReceiver(Address &addr);
	void addSender(Address &addr);
	void addChangeReceiver(Address & addr);
	void calculateTotalAmounts(void);

	std::string toString(const std::string tab) const;
};

#endif /* TRANSACTION_HPP_ */
