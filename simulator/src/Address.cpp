#include <vector>
#include <ctime>
#include <string>
#include <sstream>
#include <cassert>
#include <limits>

#include "main.hpp"
#include "Address.hpp"
#include "Sampler.hpp"


using namespace std;


std::string Address::toString(void) const
{
	std::stringstream out;
	out << "[" << hashString_ << " : ";
	out << coins_ << " : ";
	out << Str(totalAmount_) << " : ";
	if (owner_ == GENERATION)
	{
		out << "GENERATION";
	}
	else
	{
		out << owner_;
	}
	out << " : " << hasAppearedInLog_;
	if (GAID_ == GAID_NOTSET)
	{
		out << " : " << "not_set";
	}
	else
	{
		out << " : " << GAID_;
	}
	out << "]";
	return out.str();
}

void Address::Init(btc initial_balance) // calculate hash string
{
	//assert(initial_balance >= 0);
	//string rndStr = "1Bc65vSPs17qPtcD8nUSQ5BJARxb651S2o"; // TODO: sampleNewAddressHash()
	string rndStr = Sampler::randAlphaNumString(34);
	set_hashString(rndStr);

	assert(coins_.size() == 0);

	if(initial_balance > 0)
	{
		if(owner_ == GENERATION)
		{
			addCoin(initial_balance);
		}
		else
		{
			unsigned int numCoins = Sampler::randRange(1, ParameterList::getParameterByName<unsigned int>("maxNumCoinsPerAddress")+1);
			std::vector<btc> coinDistribution = Sampler::sampleCompositions(numCoins, initial_balance);

			for(unsigned int i = 0; i < coinDistribution.size(); ++i)
			{
				if(coinDistribution[i] > 0)
					addCoin(coinDistribution[i]);
			}
		}
	}
}

void Address::Set(const Address &c, bool copyCoins) // Update all member values
{
	/*
	std::cout << "THIS " << this->toString() << " gets assigned ";

	std::cout << " ==== ";

	std::string strZ = c.toString();

	std::cout << strZ << std::endl;
	*/

	set_hashString(c.get_hashString());
	if(copyCoins)
	{
		set_coins(c.get_coins());
	}
	else
	{
		totalAmount_ = 0;
	}
	set_owner(c.get_owner());
	if(c.hasAppeared()) apprearsNow();
	set_gaid(c.get_gaid());

}

Address::Address(btc initial_balance, unsigned int owner): // constructor for given initial balance and owner
	totalAmount_(0), owner_(owner),
	hasAppearedInLog_(false), GAID_(GAID_NOTSET)
{
	assert(initial_balance >= 0);
	Init(initial_balance);
	assert(get_totalAmount() == initial_balance);
}

Address::Address(unsigned int owner): // constructor for given owner, balance is set to zero
	totalAmount_(0), owner_(owner),
	hasAppearedInLog_(false), GAID_(GAID_NOTSET)
{
	Init(0);
}

Address::Address(const Address &c, bool copyCoins): // Copy constructor
	totalAmount_(0), hasAppearedInLog_(false), GAID_(GAID_NOTSET)
{
	Set(c, copyCoins);
}

Address::~Address()
{
	//dtor
	//might be needed some time
}

const Address& Address::operator=(const Address &c) // Assignment operator
{
	if(this != &c)
	{
		Set(c, true);
	}
	return *this;
}

std::ostream& operator<<(std::ostream& o, const Address& add)
{
	o << add.get_hashString();
	return o;
}

