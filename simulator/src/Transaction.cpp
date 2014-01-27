#include <vector>
#include <ctime>
#include <string>
#include <cmath>

#include "main.hpp"
#include "Client.hpp"
#include "Transaction.hpp"
#include "Address.hpp"

using namespace std;


const vector<Address>& Transaction::get_senders() const							{ return senders_; }
//void Transaction::set_senders(const vector<Address> &new_senders)				{ senders_ = new_senders; }


//const vector<Address>& Transaction::get_recipients() const						{ return recipients_; }
//void Transaction::set_recipients(const vector<Address> &new_recipients)			{ recipients_ = new_recipients; }

void Transaction::addReceiver(Address &addr)
{
	//recipients_.push_back(addr);
	recipient_ = addr;
}

void Transaction::addChangeReceiver(Address &addr)
{
	//recipients_.push_back(addr);
	shadow_ = addr;
}

void Transaction::addSender(Address &addr)
{
	senders_.push_back(addr);
}

void Transaction::calculateTotalAmounts(void)
{
	assert(!amountsCalculated_);
	btc sumIN = 0;
	//btc sumOUT = 0.0;
	for(unsigned int i = 0; i<senders_.size(); i++)
	{
		//std::cout << senders_[i].toString() << std::endl;
		sumIN += senders_[i].get_totalAmount();
		//senders_[i].set_owner(senderID_);
		assert(senders_[i].get_owner() == senderID_);
	}
	/*
	for(unsigned int i = 0; i<recipients_.size(); i++)
		sumOUT += recipients_[i].get_amount();
	*/

	//shadow_.set_owner(senderID_);
	//recipient_.set_owner(receiverID_);

	assert(!isShadowAddrNeeded_ | (shadow_.get_owner() == senderID_));
	assert(recipient_.get_owner() == receiverID_);

	totalAmountIN_ = sumIN;
	totalAmountOUT_ = shadow_.get_totalAmount() + recipient_.get_totalAmount();
	assert(totalAmountIN_ == totalAmountOUT_);

	amountsCalculated_ = true;
}

std::string Transaction::toString(const std::string tab) const
{
	std::stringstream out;
	out << tab << "[" << "TotalIn: " <<  Str(totalAmountIN_) << ", TotalOut: " << Str(totalAmountOUT_) << std::endl;
	out << tab << " " << "id: " << id_ << ", AppearedInLog: " << isAppearedInLog << ", Accepted: " <<  isAccepted_ << std::endl;
	out << tab << " " << "BlockCount: " << blockInclusionCount_ << " , log_position: " << transactionOrderingPosition_ << std::endl;
	out << tab << " ";
				if(isAccepted_)
					out << "TimeAccepted: " <<  Sampler::tmToString(timestampLastInclusion_) << ", ";
				if(isAppearedInLog)
			        out   << "TimeAppeared: " << Sampler::tmToString(timestampFirstInclusion_) << ", ";

				out << "TimeBroadcasted: " << Sampler::tmToString(timestampBroadcast_) << std::endl;

	if(senderID_ == GENERATION)
	{
		out << tab << " SenderID: " << "GENERATION" << ", ReceiverID: " << receiverID_ << std::endl;
	}
	else
	{
		out << tab << " SenderID: " << senderID_ << ", ReceiverID: " << receiverID_ << std::endl;
	}
	for(unsigned int i = 0; i < senders_.size(); ++i)
	{
		out << tab << " Sender Address:    " << senders_[i].toString() << std::endl;
	}
	out << tab << " Recipient Address: " << recipient_.toString();
	if(isShadowAddrNeeded_)
	{
		out << std::endl << tab << " Shadow Address:    " << shadow_.toString();
	}
	out << "]" << std::endl;
	return out.str();
}

/*
void Transaction::Set(const Transaction &c) // copies all member values
{
	set_senders(c.get_senders());
	set_recipients(c.get_recipients());
	calculateTotalAmounts();
}
*/

Transaction::Transaction(const tm& timestamp, unsigned int id, int eventID): // Default constructor
	senders_(vector<Address>()),
	//recipients_(vector<Address>()),
	recipient_(Address(0, id)),
	shadow_(Address(0, id)),
	timestampBroadcast_(timestamp),
	totalAmountIN_(0),
	totalAmountOUT_(0),
	isAccepted_(false),
	isShadowAddrNeeded_(false),
	isAppearedInLog(false),
	blockInclusionCount_(0),
	id_(id),
	transactionOrderingPosition_(-1),
	amountsCalculated_(false),
	eventID_(eventID)
{
}

/*
Transaction::Transaction(const Transaction &c): // Copy constructor
	senders_(vector<Address>()),
	recipients_(vector<Address>()),
	totalAmountIN_(0.0),
	totalAmountOUT_(0.0),
	isAccepted_(false),
	blockInclusionCount_(0)
{
	Set(c);
}
*/

Transaction::~Transaction()
{
	//dtor
	//might be needed some time
}

/*
const Transaction& Transaction::operator=(const Transaction &c) // Assignment operator
{
	if(this != &c)
	{
		Set(c);
	}
	return *this;
}
*/
