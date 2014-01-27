
#include "main.hpp"
#include "BitcoinSeller.hpp"
#include "Client.hpp"
#include "Sampler.hpp"
#include <limits>

BitcoinSeller::BitcoinSeller(unsigned int id, int profileID, unsigned int numInitialAddresses):
	Client(false, id, profileID, numInitialAddresses, std::vector<unsigned int>(0) , 0)
{
	assert(get_numCoins() == 0);
}

BitcoinSeller::~BitcoinSeller() {

}

std::map<unsigned int, std::vector<Coin> > BitcoinSeller::selectCoins(btc amount, btc& sumCoinsOut)
{
	assert(amount > 0);
	sumCoinsOut = amount;
	std::map<unsigned int, std::vector<Coin> > retMap;

	unsigned int chosenAddress = Sampler::randRange(initialAddressesCount_);

	assert(initialAddressesCount_ <= numAddresses());

	retMap.insert(std::make_pair(chosenAddress, std::vector<Coin>(1, amount)));

	return retMap;
}
