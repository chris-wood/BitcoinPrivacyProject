#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include "Address.hpp"
#include "UnionFind/DisjointSets.hpp"
#include <map>

class Client {
public:
    friend class BitcoinNetwork;
	virtual ~Client();

	unsigned int numAddresses(void) const
	{
		//return setOfAddresses_.NumElements();
		return setOfAddresses_.size();
	}

	unsigned int get_id(void) const
	{
		return id_;
	}

	unsigned int get_numInitialCoins(void) const
	{
		return initialCoinCount_;
	}

	btc get_outstandingChangeMoney(void) const
	{
		return outstandingChangeMoney_;
	}

	unsigned int numAddressesSeen(void) const
	{
		unsigned int count = 0;
		const std::vector<Address>& vec = setOfAddresses_.getUnderlyingVector();
		for(unsigned int i = 0; i < vec.size(); ++i)
		{
			if(vec.at(i).hasAppeared()) count++;
		}
		return count;
	}

	unsigned int numGAs(void) const
	{
		int count = setOfAddresses_.NumSets();
		int numAddr = numAddresses();
		int numAddrSeen = numAddressesSeen();
		int numGA = count - (numAddr - numAddrSeen); //subtract those that have not appeared
		//TODO check if purchaser... if it is one then then GA must be > 0
		assert(numGA >= 0 && numGA <= setOfAddresses_.NumSets() && numGA <= setOfAddresses_.NumElements());
		return numGA;
	}

	virtual btc get_totalBalance(void) const
	{
		btc sum = 0;
		for(int i = 0; i < setOfAddresses_.size(); ++i) sum += setOfAddresses_.getElement(i).get_totalAmount();
		//assert(sum >= 0);
		return sum;
	}

	unsigned int get_numCoins(void) const
	{
		unsigned int sum = 0;
		for(int i = 0; i < setOfAddresses_.size(); ++i)
			sum += setOfAddresses_.getElement(i).get_coins().size();

		return sum;
	}

	virtual std::map<unsigned int, std::vector<Coin> > selectCoins(btc amount, btc& sumCoinsOut);

protected:
	Client(bool isMiner, unsigned int id, int profileID, unsigned int numInitialAddresses,
			const std::vector<unsigned int>& listOfFriends, btc initialCoins);

	std::vector<unsigned int> listOfFriends_;
	std::vector<unsigned int> listOfTransactionsAsSender_;
	std::vector<unsigned int> listOfTransactionsAsReceiver_;
	DisjointSets<Address, AddressCompare> setOfAddresses_;
	std::map<Address, int, AddressCompare> shadowAddressesLink_;
	//vector<Address> setOfAddresses_;

	bool miner_;
	unsigned int id_;
	int profileID_;
	btc initialAmount_;
	unsigned int initialAddressesCount_;
	unsigned int initialCoinCount_;
	btc outstandingChangeMoney_;
};


#endif /* CLIENT_HPP_ */
