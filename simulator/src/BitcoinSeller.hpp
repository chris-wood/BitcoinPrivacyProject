#ifndef BITCOINSELLER_HPP_
#define BITCOINSELLER_HPP_

#include "main.hpp"
#include "Client.hpp"
#include <limits>

class BitcoinSeller : public Client {
public:
	BitcoinSeller(unsigned int id, int profileID, unsigned int numInitialAddresses);
	virtual ~BitcoinSeller();

	btc get_totalBalance(void) const
	{
		return numeric_limits<btc>::max()-100;
	}

	std::map<unsigned int, std::vector<Coin> > selectCoins(btc amount, btc& sumCoinsOut);
};

#endif /* BITCOINSELLER_HPP_ */
