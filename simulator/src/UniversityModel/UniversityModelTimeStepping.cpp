
#include "../main.hpp"
#include "UniversityModelTimeStepping.hpp"
#include "UniversityModelClient.hpp"
#include "UniversityModel.hpp"
#include "UniversityModelPlanningEvent.hpp"
#include "UniversityModelNewClientEvent.hpp"
#include "UniversityModelIssueTransactionEvent.hpp"
#include "UniversityModelAddAddressEvent.hpp"
#include "../ParameterList.hpp"
#include "../NetworkEvent.hpp"
#include "../NewClientEvent.hpp"
#include "../NewBitcoinSellerEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../TimeStepping.hpp"
#include "../Sampler.hpp"
#include "../Statistics/StatisticsCalculator.hpp"
#include "../Statistics/StatisticsContainer.hpp"


#include <ctime>
#include <queue>
#include <vector>

UniversityModelTimeStepping::UniversityModelTimeStepping(): //(BitcoinNetwork* btcNetwork):
	TimeStepping(), //(btcNetwork),
	//maxInitAddressCount(ParameterList::getParameterByName<unsigned int>("maxInitialAddressCount")),
	//minInitAddressCount(ParameterList::getParameterByName<unsigned int>("minInitialAddressCount")),
	//planningPeriodInDays(ParameterList::getParameterByName<unsigned int>("planningPeriodInDays")),
	planningPeriodInDays(7), // hard code this to 7 for the moment
	exchangeRate(ParameterList::getParameterByName<double>("exchangeRate")),
	allowedFailedTransactionCount(ParameterList::getParameterByName<unsigned int>("allowedIlliquideTransactions")),
	issueTransactionCount_(0),
	issueTransactionFailureCount_(0),
	plannedBitcoinPurchasesCount_(0),
	plannedPrivacyAwareTransCount_(0),
	executedBitcoinPurchasesCount_(0),
	privacyAwareUserCount_(0),
	numDifferentBitcoinSellerAddresses_(ParameterList::getParameterByName<unsigned int>("numBitcoinSellerAddresses")),
	stdForTimeOfEvents_(ParameterList::getParameterByName<unsigned int>("stdForTimeOfEvents")),
	eventOneTimePad_(ParameterList::getParameterByName<bool>("eventOneTimePad")),
	probabilityForBitcoinTransaction_(ParameterList::getParameterByName<double>("probForBitcoinTransaction")),
	//sellerRepresentations(UniversityModel::stores.size()) // set appropriate size
	statContainer("InterarrivalTimeBlockGen")
{
		// initial setting
		tm nextTimestamp = currentTime_;

		// normal clients
		while(numClients_ < maxClients_)
		{
			UniversityModelNewClientEvent* nce = createNewClient(nextTimestamp);
			eventQueue_.push(nce);
			numClients_++;
		}

		// sellers
		const std::map<unsigned int, Store>& stores = UniversityModelConfig::getInstance().stores;
		for(std::map<unsigned int, Store>::const_iterator it = stores.begin(); it != stores.end(); ++it)
		{
			UniversityModelNewClientEvent* nce = createNewSeller(nextTimestamp, it->second.id);
			eventQueue_.push(nce);
		}

		// add the Bitcoin seller a bit later
		Sampler::addTimeInterval(nextTimestamp, 0, 0, 0, 0, 0, 1);
		NewBitcoinSellerEvent* nbse = new NewBitcoinSellerEvent(nextTimestamp, this, 3333, numDifferentBitcoinSellerAddresses_);
		eventQueue_.push(nbse);

		Sampler::addTimeInterval(nextTimestamp, 0, 0, 0, 0, 0, 1);
		GenerateBlockEvent* gbe = new GenerateBlockEvent(nextTimestamp, this);
		eventQueue_.push(gbe);
		numBlocks_++;

		// 2 secs later: make sure all clients have been created
		Sampler::addTimeInterval(nextTimestamp, 0, 0, 0, 0, 0, 2);
		UniversityModelPlanningEvent* pEv = createPlanningEvent(nextTimestamp);
		eventQueue_.push(pEv);

		statContainer.addCalculator<MeanStd>();
}

UniversityModelPlanningEvent* UniversityModelTimeStepping::createPlanningEvent(const tm& nextTimestamp)
{
	UniversityModelPlanningEvent* pEv = new UniversityModelPlanningEvent(nextTimestamp, this);
	return pEv;
}

UniversityModelNewClientEvent* UniversityModelTimeStepping::createNewSeller(const tm&nextTimestamp, unsigned int storeID)
{
	const std::vector<std::pair<double, unsigned int> >& profileProbs = UniversityModelConfig::getInstance().sellerProfileProbabilities;
	const std::map<unsigned int, Profile> & profiles = UniversityModelConfig::getInstance().sellerProfiles;
	unsigned int chosenIndex = Sampler::sampleFromCategoricalDistribution(separatePairVect(profileProbs));
	assert(chosenIndex >= 0 && chosenIndex < profileProbs.size());
	unsigned int profileID = profileProbs.at(chosenIndex).second;

	const Profile& profile =  profiles.at(profileID);
	bool isMiner = (Sampler::rand01() <= profile.miners) ? 1 : 0;
	bool isPrivacyAware = false; // not needed since the stores do not issue transactions
	int initialAddressesCount = Sampler::randRange(profile.minInitialAddressCount, profile.maxInitialAddressCount+1);
	btc initialCoins = Sampler::randRangeBTC(profile.minInitialAmount, profile.maxInitialAmount);

	UniversityModelClient umC(profileID, nextTimestamp, isPrivacyAware, true);
	umC.set_storeID(storeID);

	UniversityModelNewClientEvent* nce =
			new UniversityModelNewClientEvent(nextTimestamp, this, profileID, isMiner, initialAddressesCount, initialCoins, umC);
	return nce;
}

UniversityModelNewClientEvent* UniversityModelTimeStepping::createNewClient(const tm& nextTimestamp)
{
	const std::vector<std::pair<double, unsigned int> >& profileProbs = UniversityModelConfig::getInstance().profileProbabilities;
	const std::map<unsigned int, Profile>& profiles = UniversityModelConfig::getInstance().profiles;
	unsigned int chosenIndex = Sampler::sampleFromCategoricalDistribution(separatePairVect(profileProbs));
	assert(chosenIndex >= 0 && chosenIndex < profileProbs.size());
	unsigned int profileID = profileProbs.at(chosenIndex).second;

	const Profile& profile =  profiles.at(profileID);
	bool isMiner = (Sampler::rand01() <= profile.miners) ? 1 : 0;
	bool isPrivacyAware = false;
	if(Sampler::rand01() <= profile.privacyAwarePercentage)
	{
		isPrivacyAware = true;
		privacyAwareUserCount_++;
	}
	int initialAddressesCount = Sampler::randRange(profile.minInitialAddressCount, profile.maxInitialAddressCount+1);
	btc initialCoins = Sampler::randRangeBTC(profile.minInitialAmount, profile.maxInitialAmount);

	UniversityModelClient umC(profileID, nextTimestamp, isPrivacyAware, false);

	UniversityModelNewClientEvent* nce =
			new UniversityModelNewClientEvent(nextTimestamp, this, profileID, isMiner, initialAddressesCount, initialCoins, umC);
	return nce;
}

UniversityModelTimeStepping::~UniversityModelTimeStepping() {
}

void UniversityModelTimeStepping::planAndEnqueueTransactions(const tm& planTimeStamp)
{
	for(unsigned int i = 0; i < clientRepresentations.size(); ++i)
	{
		UniversityModelClient& client = clientRepresentations.at(i);
		if(!client.get_isSeller()) // important! (stores do not issue any transactions)
		{
			unsigned int clientID = client.get_ID();
			assert(clientID == i);
			unsigned int profileID = client.get_profileID();

			const Profile& clientProfile = UniversityModelConfig::getInstance().profiles.at(profileID);
			for(std::map<unsigned int, ProfileEvent>::const_iterator it = clientProfile.profileEvents.begin();
					it != clientProfile.profileEvents.end(); ++it)
			{
				const ProfileEvent& profileEvent = it->second;
				if(!eventOneTimePad_ || client.getActiveOneTimeBit(profileEvent))
				{
					unsigned int eventID = profileEvent.eventid;
					const Event& event = UniversityModelConfig::getInstance().events.at(eventID);

					planAndEnqueueTransactions(planTimeStamp, client, clientProfile, profileEvent, event, i);
				}
			}

			// interexchange (only for normal clients != seller profiles)
			if(clientProfile.interExchange.active)
			{
				planAndEnqueueInterExchanges(planTimeStamp, client, clientProfile, i);
			}

			if(clientProfile.privacyAwareness.active && client.get_isPrivacyAware())
			{
				planAndEnqueuePrivacyAwareness(planTimeStamp, client, clientProfile, i);
			}

		}
	}
}

void UniversityModelTimeStepping::planAndEnqueuePrivacyAwareness(const tm& plantimeStamp, const UniversityModelClient& client,
																const Profile& clientProfile, unsigned int umcPos)
{
	const PrivacyAwareness& iex = clientProfile.privacyAwareness;
	unsigned int numberOfTransactions = Sampler::randRange(iex.minFreqPerWeek, iex.maxFreqPerWeek +1);
	if(numberOfTransactions > 0)
	{
		std::vector<unsigned int> daysOfTransactions = Sampler::randVectPermutation(iex.availableDaysOfWeek, numberOfTransactions);
		std::vector<unsigned int> diffInDays =
				Sampler::weekDayDiffVec(daysOfTransactions, Sampler::getWeekDayFromTm(plantimeStamp));

		for(unsigned int i = 0; i < numberOfTransactions; ++i)
		{
			tm transactionTimestamp = Sampler::addTimeInterval(plantimeStamp, 0, 0, diffInDays[i], 0, 0, 0, true);

			unsigned int secsFromEventStart = Sampler::randRange(iex.durationInSecs()+1);

			btc amount = Sampler::randRangeBTC(iex.minPrice, iex.maxPrice+1);
			transactionTimestamp.tm_hour = iex.startHour;
			transactionTimestamp.tm_min = iex.startMin;
			Sampler::addTimeInterval(transactionTimestamp, 0, 0, 0, 0, 0, secsFromEventStart);

			if(maxTransactions_ > 0 && maxTransactions_ <= numTransactions_)
			{
				//stop
				break;
			}
			else
			{
				unsigned int senderAndReceiver = client.get_ID();

				btc amountAdjusted = amount*exchangeRate;

				UniversityModelAddAddressEvent* umaae = new UniversityModelAddAddressEvent(transactionTimestamp,
						this, amountAdjusted, senderAndReceiver, umcPos);
				eventQueue_.push(umaae);
				numTransactions_++;
				plannedPrivacyAwareTransCount_++;
			}
		}
	}
}

void UniversityModelTimeStepping::planAndEnqueueInterExchanges(const tm& plantimeStamp, const UniversityModelClient& client,
																const Profile& clientProfile, unsigned int umcPos)
{
	const InterExchange& iex = clientProfile.interExchange;
	//const tm nextPlanningTimestamp = Sampler::addTimeInterval(plantimeStamp, 0, 0, planningPeriodInDays, 0, 0, 0, true);

	unsigned int numberOfTransactions = Sampler::randRange(iex.minFreqPerWeek, iex.maxFreqPerWeek +1);
	if(numberOfTransactions > 0)
	{
		//std::cout << iex.availableDaysOfWeek << std::endl;
		std::vector<unsigned int> daysOfTransactions = Sampler::randVectPermutation(iex.availableDaysOfWeek, numberOfTransactions);
		//std::cout << daysOfTransactions << std::endl;

		std::vector<unsigned int> diffInDays =
				Sampler::weekDayDiffVec(daysOfTransactions, Sampler::getWeekDayFromTm(plantimeStamp));

		for(unsigned int i = 0; i < numberOfTransactions; ++i)
		{
			tm transactionTimestamp = Sampler::addTimeInterval(plantimeStamp, 0, 0, diffInDays[i], 0, 0, 0, true);
			//unsigned int dayOfweek = Sampler::getWeekDayFromTm(transactionTimestamp);

			unsigned int secsFromEventStart = Sampler::randRange(iex.durationInSecs()+1);

			btc amount = Sampler::randRangeBTC(iex.minPrice, iex.maxPrice+1);
			transactionTimestamp.tm_hour = iex.startHour;
			transactionTimestamp.tm_min = iex.startMin;
			Sampler::addTimeInterval(transactionTimestamp, 0, 0, 0, 0, 0, secsFromEventStart);

			// now issue the transaction
			if(maxTransactions_ > 0 && maxTransactions_ <= numTransactions_)
			{
				//stop
				break;
			}
			else
			{
				unsigned int sender = client.get_ID();

				unsigned int numClientsForProfile = profileToClientLinks.at(clientProfile.id).size();
				unsigned int randomPosition = Sampler::randRange(numClientsForProfile);
				unsigned int chosenClient = profileToClientLinks.at(clientProfile.id).at(randomPosition);

				unsigned int receiver = clientRepresentations.at(chosenClient).get_ID();

				btc amountAdjusted = amount*exchangeRate;

				UniversityModelIssueTransactionEvent* umite = new UniversityModelIssueTransactionEvent(transactionTimestamp,
						this, amountAdjusted, sender, receiver, umcPos, 6);
				eventQueue_.push(umite);
				numTransactions_++;
			}
		}
	}
}

void UniversityModelTimeStepping::planAndEnqueueTransactions(const tm& plantimeStamp, UniversityModelClient& client,
					const Profile& clientProfile, const ProfileEvent& profileEvent, const Event& event, unsigned int umcPos)
{
	//std::cout << "Planning transactions on " << Sampler::tmToStringWday(plantimeStamp) << std::endl;
	assert(clientProfile.id == client.get_profileID());
	assert(event.id == profileEvent.eventid);
	// schedule transactions
	const tm nextPlanningTimestamp = Sampler::addTimeInterval(plantimeStamp, 0, 0, planningPeriodInDays, 0, 0, 0, true);
	int testTimeDiff = Sampler::dayDiff(plantimeStamp, nextPlanningTimestamp, true);
	assert(testTimeDiff == planningPeriodInDays);

	tm nextPeriodStartTime = client.getNextPeriodStartTime(profileEvent);
	int daysTillNextPeriodStartTime = Sampler::dayDiff(plantimeStamp, nextPeriodStartTime, true);
	assert(daysTillNextPeriodStartTime >= 0);

	assert(daysTillNextPeriodStartTime < planningPeriodInDays); // TODO: maybe weaken this later
	if(daysTillNextPeriodStartTime < planningPeriodInDays) // we need to plan transactions
	{
		unsigned int numberOfTransactions = Sampler::randRange(profileEvent.minFreqPerWeek, profileEvent.maxFreqPerWeek +1);
		if(numberOfTransactions > 0)
		{
			std::vector<unsigned int> availDays = getAvailableDaysOfWeek(event);
			std::vector<unsigned int> daysOfTransactions = Sampler::randVectPermutation(availDays, numberOfTransactions);
			//std::cout << "Available days: :" << availDays << std::endl;
			//std::cout << "Transaction days: " << daysOfTransactions << std::endl;
			std::vector<unsigned int> diffInDays =
					Sampler::weekDayDiffVec(daysOfTransactions, Sampler::getWeekDayFromTm(plantimeStamp));
			//std::cout << "difference in days: " << diffInDays << std::endl;

			assert(diffInDays.size() == numberOfTransactions);
			assert(daysOfTransactions.size() == numberOfTransactions);
			for(unsigned int i = 0; i < numberOfTransactions; ++i)
			{
				tm transactionTimestamp = Sampler::addTimeInterval(plantimeStamp, 0, 0, diffInDays[i], 0, 0, 0, true);
				/*
				std::cout << "Transaction time: " << Sampler::tmToStringWday(transactionTimestamp) << ": client id=" << client.get_ID()
						  << " profileID=" << clientProfile.id << " " << profileEvent.toString() << " event id="
						  << event.id << " eventname=" << event.name << std::endl;
				*/
				unsigned int dayOfweek = Sampler::getWeekDayFromTm(transactionTimestamp);
				//std::cout << "Day of week: " << dayOfweek << std::endl;
				assert(dayOfweek >= 1 && dayOfweek <= 7);
				std::vector<std::pair<double, unsigned int> > probVect = client.getProbabilityVector(profileEvent, dayOfweek);
				assert(probVect.size() > 0);
				unsigned int chosenIndex = Sampler::sampleFromCategoricalDistribution(separatePairVect(probVect));
				assert(chosenIndex >= 0 && chosenIndex < probVect.size());
				unsigned int chosenStoreID = probVect.at(chosenIndex).second;

				const Store& store = UniversityModelConfig::getInstance().stores.at(chosenStoreID);
				//unsigned int secsFromEventStart = Sampler::randRange(event.durationInSecs()+1);
				unsigned int secsFromEventStart = Sampler::sampleFromNormalDistribution(client.getSecsFromEventStart(profileEvent), stdForTimeOfEvents_);
				ProbPriceRange ppr = clientProfile.getProbRange(event.id, store.id);
				btc amount = Sampler::randRangeBTC(ppr.minPrice, ppr.maxPrice+1);
				transactionTimestamp.tm_hour = event.startHour;
				transactionTimestamp.tm_min = event.startMin;
				Sampler::addTimeInterval(transactionTimestamp, 0, 0, 0, 0, 0, secsFromEventStart);
				/*
				std::cout << "client " << client.get_ID() << " with profile ID " << client.get_profileID()
						  << " will buy for " << amount << " at " << store.name << " at "
						  << Sampler::tmToString(transactionTimestamp) << std::endl;
				*/

				// now issue the transaction

				if(Sampler::rand01() > probabilityForBitcoinTransaction_)
				{
					// user pays with conventional coins. stop adding transaction
					break;
				}

				if(maxTransactions_ > 0 && maxTransactions_ <= numTransactions_)
				{
					//stop
					break;
				}
				else
				{
					unsigned int sender = client.get_ID();
					unsigned int receiverVecPosition = storeSellerLinks.at(store.id);
					unsigned int receiver = clientRepresentations.at(receiverVecPosition).get_ID();

					btc amountAdjusted = amount*exchangeRate;

					UniversityModelIssueTransactionEvent* umite = new UniversityModelIssueTransactionEvent(transactionTimestamp,
							this, amountAdjusted, sender, receiver, umcPos, event.id);
					eventQueue_.push(umite);
					numTransactions_++;
				}
			}
		}
		client.setNextPeriodStartTime(profileEvent, nextPlanningTimestamp);
	}
}

void UniversityModelTimeStepping::planAndEnqueueBitcoinPurchase(const UniversityModelClient& client, const tm& timestampOfLastFailure)
{
	if(maxTransactions_ > 0 && maxTransactions_ <= numTransactions_)
	{
		//stop
		return;
	}
	else
	{
		unsigned int profileID = client.get_profileID();
		unsigned int receiver = client.get_ID();

		const Profile& clientProfile = UniversityModelConfig::getInstance().profiles.at(profileID);
		const Supplies& suppl = clientProfile.supplies;

		if(!suppl.active)
			return; // abort;

		btc amount = Sampler::randRangeBTC(suppl.minAmount, suppl.maxAmount+1);

		unsigned int sender = BitcoinSellerID_;

		btc amountAdjusted = amount*exchangeRate;

		unsigned int minute = Sampler::randRange(suppl.minMinutesAfterLastFailure, suppl.maxMinutesAfterLastFailure+1);
		unsigned int second = Sampler::randRange(0, 60);
		tm transactionTimestamp = Sampler::addTimeInterval(timestampOfLastFailure, 0, 0, 0, 0, minute, second);

		UniversityModelIssueTransactionEvent* umite = new UniversityModelIssueTransactionEvent(transactionTimestamp,
				this, amountAdjusted, sender, receiver, 8);

		eventQueue_.push(umite);
		numTransactions_++;
		plannedBitcoinPurchasesCount_++;
	}
}

std::vector<unsigned int> UniversityModelTimeStepping::getAvailableDaysOfWeek(const Event& ev) const
{
	std::vector<bool> availableDaysOfWeekBool;
	std::vector<unsigned int> availableDaysOfWeek;
	availableDaysOfWeekBool.resize(7, false);
	bool atLeastOnce = false;
	for(unsigned int i = 0; i < ev.associatedStores.size(); ++i)
	{
		const bool* weekDaysArray = UniversityModelConfig::getInstance().stores.at(ev.associatedStores[i]).availableDaysOfWeek;
		for(unsigned int p = 1; p <= 7; p++)
		{
			if(weekDaysArray[p-1])
			{
				availableDaysOfWeekBool[p-1] = true;
				atLeastOnce = true;
			}
		}
	}
	assert(availableDaysOfWeekBool.size() == 7);
	assert(atLeastOnce);
	for(unsigned int p = 1; p <= 7; p++)
	{
		if(availableDaysOfWeekBool.at(p-1))
		{
			availableDaysOfWeek.push_back(p);
		}
	}
	assert(availableDaysOfWeek.size() > 0 && availableDaysOfWeek.size() <= 7);
	return availableDaysOfWeek;
}

void UniversityModelTimeStepping::postProcessing(NetworkEvent* networkEvent)
{
	assert(networkEvent != NULL);
	assert(networkEvent->executed());

	std::string eventName = networkEvent->getName();
	tm old_EventTime = networkEvent->getTimeStamp();

	NetworkEvent* nee = NULL;
	if(eventName == "PlanningEvent") // newClientEvent
	{
		planAndEnqueueTransactions(old_EventTime);

		if(maxTransactions_ > 0 && maxTransactions_ <= numTransactions_)
		{
			std::cout << "PlanningEvent: Reached " << maxTransactions_ << ", no more new transactions created" << std::endl;
		}
		else
		{
			tm nextPlanningTimestamp = old_EventTime;
			Sampler::addTimeInterval(nextPlanningTimestamp, 0, 0, planningPeriodInDays, 0, 0, 0, true);
			nee = createPlanningEvent(nextPlanningTimestamp);
		}
	}
	else if(eventName == "GenerateBlockEvent") // generateBlockEvent
	{
		if(maxBlocks_ > 0 && maxBlocks_ <= numBlocks_)
		{
			std::cout << "GenerateBlockEvent: Reached " << maxBlocks_ << ", no more new blocks" << std::endl;
		}
		else
		{
			// TODO: when is the next block generated? -> ok so far.
			int secondsToNextBlockAvg =
					ParameterList::getParameterByName<unsigned int>("blockCreationInterarrivalTimeInMin")*60;
			int secondsToNextBlock = Sampler::sampleFromExponentialDistribution(1.0/((double)secondsToNextBlockAvg));
			tm nextOccurence = old_EventTime;
			Sampler::addTimeInterval(nextOccurence, 0, 0, 0, 0, 0, secondsToNextBlock);

			statContainer.addValue(secondsToNextBlock);

			nee = new GenerateBlockEvent(nextOccurence, this);
			nextBlockGeneration_ = nee->getTimeStamp();
			numBlocks_++;
		}
	}
	else if(eventName == "NewClientEvent")
	{
		// store the client representation
		UniversityModelNewClientEvent *nce = dynamic_cast<UniversityModelNewClientEvent*>(networkEvent);
		assert(nce != NULL);
		UniversityModelClient& addedClient = nce->getClientRepresentation();
		if(addedClient.get_isSeller())
		{
			unsigned int storeID = addedClient.get_storeID();
			assert(storeID < UniversityModelConfig::getInstance().stores.size());
			unsigned int nextPos = clientRepresentations.size();
			storeSellerLinks[storeID] = nextPos;
		}
		clientRepresentations.push_back(addedClient);
		unsigned int positionOfNewClient = clientRepresentations.size()-1;
		profileToClientLinks[addedClient.get_profileID()].push_back(positionOfNewClient);
		unsigned int addedClientID = addedClient.get_ID();
		assert(addedClientID == positionOfNewClient);
	}
	else if(eventName == "IssueTransactionEvent") //  IssueTransactionEvent
	{
		issueTransactionCount_++;
		if(!networkEvent->successful())
			issueTransactionFailureCount_++;

		UniversityModelIssueTransactionEvent *ite = dynamic_cast<UniversityModelIssueTransactionEvent*>(networkEvent);
		assert(ite != NULL);

		if(ite->isNotBitcoinPurchase())
		{
			UniversityModelClient& sendingClient = ite->getClientRepresentation();
			assert(!sendingClient.get_isSeller());

			if(sendingClient.get_failedTransactionCount() >= allowedFailedTransactionCount)
			{
				sendingClient.clearFailedTransactionCount();
				// buy new coins
				planAndEnqueueBitcoinPurchase(sendingClient, old_EventTime);
			}
		}
		else // was a Bitcoin Purchase that hase been executed
		{
			executedBitcoinPurchasesCount_++;
		}
	}
	else if(eventName == "NewBitcoinSellerEvent") //  IssueTransactionEvent
	{
		NewBitcoinSellerEvent* nbse = dynamic_cast<NewBitcoinSellerEvent*>(networkEvent);
		BitcoinSellerID_ = nbse->getIDofNewClient();
	}

	if(nee != NULL) eventQueue_.push(nee);
}

std::ostream& UniversityModelTimeStepping::outputStatistics(std::ostream& o) const
{
	o << "Statistics for UniversityModel" << std::endl;
	o << statContainer;
	o << "Number of planned transactions: " << numTransactions_ << std::endl;
	o << "Number of planned block generations: " << numBlocks_ << std::endl;
	o << "Number of planned client creations: " << numClients_ << std::endl;
	o << "Number of created clients which are privacy aware: " << privacyAwareUserCount_ << std::endl;
	o << "Number of stores: " << storeSellerLinks.size() << std::endl;
	for(std::map<unsigned int, std::vector<unsigned int> >::const_iterator it = profileToClientLinks.begin();
			it != profileToClientLinks.end(); it++)
	{
		if(it->second.size() > 0)
		{
			bool isSeller = clientRepresentations.at(it->second[0]).get_isSeller();
			if(isSeller)
			{
				o << "Number of " << UniversityModelConfig::getInstance().sellerProfiles.at(it->first).name << ": "
						<< it->second.size() << std::endl;
			}
			else
			{
				o << "Number of " << UniversityModelConfig::getInstance().profiles.at(it->first).name << ": "
						<< it->second.size() << std::endl;
			}
		}
	}
	o << "Number of times a transaction is issued: " << issueTransactionCount_ << std::endl;
	o << "Number of times issuing a transaction has failed: " << issueTransactionFailureCount_ << std::endl;
	o << "Number of times a Bitcoin Purchase from a seller was planned: " << plannedBitcoinPurchasesCount_ << std::endl;
	o << "Number of times a privacy awareness transaction was planned: " << plannedPrivacyAwareTransCount_ << std::endl;
	o << "Number of times a Bitcoin Purchase from a seller was executed: " << executedBitcoinPurchasesCount_ << std::endl;
	o << "Number of events remaining in queue: " << get_remainingCount() << std::endl;
	return o;
}
