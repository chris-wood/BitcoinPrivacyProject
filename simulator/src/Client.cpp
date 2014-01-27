#include "Client.hpp"
#include "Address.hpp"
#include "Sampler.hpp"

#include <algorithm>

Client::Client(bool isMiner, unsigned int id, int profileID, unsigned int numInitialAddresses,
		const std::vector<unsigned int>& listOfFriends, btc initialCoins):
			setOfAddresses_(Address(id)),
			shadowAddressesLink_(),
			initialCoinCount_(0),
			outstandingChangeMoney_(0)
{
	//initialAddressesCount_ = numInitialAddresses;
	initialAmount_ = initialCoins;
	miner_ = isMiner;
	id_ = id;
	profileID_ = profileID;
	listOfFriends_ = listOfFriends;

	std::vector<btc> coinPerAddressDistribution = Sampler::sampleCompositions(numInitialAddresses, initialCoins);

	for(unsigned int i = 0; i < coinPerAddressDistribution.size(); ++i)
	{
		/*if(coinPerAddressDistribution[i] == 0)
			continue;*/

		Address addr(coinPerAddressDistribution[i], id);
		setOfAddresses_.addElement(addr);
		initialCoinCount_ += addr.get_coins().size();
	}

	initialAddressesCount_ = numAddresses();

	assert(get_totalBalance() == initialCoins);
	assert(numInitialAddresses == initialAddressesCount_);

	/*
	int distributionDivisor = Sampler::randRange(1, numInitialAddresses+1);
	btc remaining = initialCoins;
	btc part = initialCoins / distributionDivisor;
	for(unsigned int i = 0; i < numInitialAddresses; ++i)
	{
		double tmp;
		if(remaining <= 0.0)
		{
			remaining = 0.0;
			tmp = 0.0;
		}
		else
		{
			tmp = part;
			remaining -= part;
		}
		// TODO: rearrange this to avoid copy construction
		Address addr(tmp, id);
		setOfAddresses_.addElement(addr);
		//setOfAddresses_.push_back(addr);
	}
	*/
}

std::map<unsigned int, std::vector<Coin> > Client::selectCoins(btc amount, btc& sumCoinsOut)
{
	assert(amount > 0);
	btc balanceBefore = get_totalBalance();
	unsigned int numCoinsBefore = get_numCoins();
	assert(balanceBefore >= amount);

	//std::cout << Str(amount) << std::endl;

	sumCoinsOut = 0;
	btc lowerSum = 0;
	std::map<unsigned int, std::vector<Coin> > retMap;

	std::vector< std::pair< btc, unsigned int > > coinVec;

	std::pair< btc, unsigned int > coinLowestLarger;

	coinLowestLarger.first = std::numeric_limits<btc>::max();

	bool found = false;
	for(int i = 0; i < setOfAddresses_.size() && !found; ++i)
	{
		//std::cout << setOfAddresses_.getElement(i).toString() << " " << setOfAddresses_.getElement(i).get_coins().size() << std::endl;
		const std::multiset<Coin> coinsFromAddr = setOfAddresses_.getElement(i).get_coins();
		for(std::multiset<Coin>::const_iterator it = coinsFromAddr.begin(); it != coinsFromAddr.end() && !found; it++)
		{
			btc coinAmount = it->getValue();
			if(coinAmount == amount)
			{
				coinVec.clear();
				coinVec.push_back(std::make_pair(coinAmount, i)); // we only have one entry
				found = true; // exit loops
			}
			else if(coinAmount < amount)
			{
				coinVec.push_back(std::make_pair(coinAmount, i));
				lowerSum += coinAmount;
			}
			else if(coinAmount < coinLowestLarger.first)
			{
				coinLowestLarger = std::make_pair(coinAmount, i);
			}
		}
	}

	//std::cout << coinVec << std::endl;
	//std::cout << coinLowestLarger << std::endl;

	retMap.clear();

	if(found || lowerSum == amount)
	{
		for(unsigned int i = 0; i < coinVec.size(); ++i)
		{
			retMap[coinVec[i].second].push_back(coinVec[i].first);
			sumCoinsOut += coinVec[i].first;
		}
	}
	else if(lowerSum < amount)
	{
		retMap[coinLowestLarger.second].push_back(coinLowestLarger.first);
		sumCoinsOut = coinLowestLarger.first;
	}
	else
	{
		std::sort(coinVec.begin(), coinVec.end());
		//std::cout << coinVec << std::endl;

		btc sumCoins = 0;
		int i = coinVec.size()-1;
		while(sumCoins < amount)
		{
			while(i > 0 && coinVec[i-1].first >= (amount - sumCoins)) { --i; }
			//std::cout << coinVec[i].first << " ";
			sumCoins += coinVec[i].first;
			retMap[coinVec[i].second].push_back(coinVec[i].first);
			i--;
		}
		sumCoinsOut = sumCoins;

		if(sumCoins > coinLowestLarger.first)
		{
			//std::cout << "not using this: " << retMap << std::endl;
			retMap.clear();
			retMap[coinLowestLarger.second].push_back(coinLowestLarger.first);
			sumCoinsOut = coinLowestLarger.first;
		}
	}

	assert(!retMap.empty());
	//std::cout << std::endl << retMap << std::endl << Str(sumCoinsOut) << std::endl;

	//now *consume* the coins
	unsigned int deletedCoinsCount = 0;
	for(std::map<unsigned int, std::vector<Coin> >::const_iterator it = retMap.begin(); it != retMap.end(); it++)
	{
		setOfAddresses_.getElement(it->first).deleteCoins(it->second);
		deletedCoinsCount += it->second.size();
	}

	/*//DEBUG
	for(int i = 0; i < setOfAddresses_.size() && !found; ++i)
	{
		std::cout << setOfAddresses_.getElement(i).toString() << " " << setOfAddresses_.getElement(i).get_coins().size() << std::endl;
	}
	*/
	assert(deletedCoinsCount > 0);

	assert(sumCoinsOut >= amount);

	unsigned int numCoinsAfterwards = get_numCoins();
	assert(numCoinsAfterwards == (numCoinsBefore - deletedCoinsCount));
	btc balanceAfterwards = get_totalBalance();
	btc reconstructedBalance = (balanceAfterwards + sumCoinsOut);
	assert(balanceBefore == reconstructedBalance);

	return retMap;
}


Client::~Client() {
}

