#ifndef ADDRESS_HPP_
#define ADDRESS_HPP_

#include "main.hpp"

#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <cassert>
#include <functional>
#include <algorithm>
#include <limits>


using namespace std;



class Address
{
private:
	string hashString_;
	//std::vector<btc> coins_;
	std::multiset<Coin> coins_;
	btc totalAmount_;
	unsigned int owner_;
	bool hasAppearedInLog_;
	unsigned int GAID_;

	inline void set_hashString(const string &new_hashString)		{ hashString_ = new_hashString; }
	inline void set_owner(unsigned int new_owner)					{ owner_ = new_owner; }
	inline void set_coins(const std::multiset<Coin>& newCoins)
	{
		coins_ = newCoins;
		btc sum = 0;
		for(std::multiset<Coin>::const_iterator it = coins_.begin(); it != coins_.end(); it++) { sum += it->getValue(); }
		totalAmount_ = sum;
	}

	void Set(const Address &c, bool copyCoins);
	void Init(btc initial_balance);

public:
	explicit Address(unsigned int owner);
	explicit Address(btc initial_balance, unsigned int owner);
	explicit Address(const Address &c, bool copyCoins);
	~Address();
	const Address &operator=(const Address &c); // does always copy the coins

	// getter & setters
	inline const string& get_hashString() const						{ return hashString_; }
	inline const std::multiset<Coin>& get_coins(void) const			{ return coins_; }
	inline unsigned int get_owner() const							{ return owner_; }
	inline unsigned int get_gaid() const							{ return GAID_; }
	inline void set_gaid(unsigned int new_gaid)						{ GAID_ = new_gaid; }

	inline bool hasAppeared(void) const								{ return hasAppearedInLog_; }
	inline void apprearsNow(void)									{ hasAppearedInLog_ = true; }

	inline btc get_totalAmount(void) const							{ return totalAmount_; }

	inline void addCoin(const Coin& coin)
	{
		coins_.insert(coin);
		totalAmount_ += coin.getValue();
	}

	void addCoins(const std::vector<Coin>& coins)
	{
		for(unsigned int i = 0; i < coins.size(); ++i)
		{
			addCoin(coins[i]);
		}
	}

	inline void deleteCoin(const Coin& coin)
	{
		std::multiset<Coin>::iterator it = coins_.find(coin);
		assert(it != coins_.end()); // coin must exist
		coins_.erase(it);
		if(coins_.size() == 0) // prevents numerical issues
			totalAmount_ = 0;
		else
			totalAmount_ -= coin.getValue();
	}

	void deleteCoins(const std::vector<Coin>& coinAmounts)
	{
		for(unsigned int i = 0; i < coinAmounts.size(); ++i)
		{
			deleteCoin(coinAmounts[i]);
		}
	}

	std::string toString(void) const;
};

// compare object for Address class
class AddressCompare {
public:
    bool operator()(const Address& a1, const Address& a2) const
    {
    	std::less<std::string> stringCompare;
    	return stringCompare(a1.get_hashString(), a2.get_hashString());
    }
};

extern std::ostream& operator<<(std::ostream& o, const Address& add);

#endif /* ADDRESS_HPP_ */
