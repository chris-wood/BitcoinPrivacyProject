
#ifndef UNIVERSITYMODELTIMESTEPPING_HPP_
#define UNIVERSITYMODELTIMESTEPPING_HPP_

#include "../main.hpp"
#include "UniversityModelPlanningEvent.hpp"
#include "UniversityModelClient.hpp"
#include "UniversityModel.hpp"
#include "../ParameterList.hpp"
#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../TimeStepping.hpp"
#include "../NewClientEvent.hpp"
#include "../GenerateBlockEvent.hpp"
#include "../ChangeAddressEvent.hpp"
#include "../IssueTransactionEvent.hpp"
#include "../Sampler.hpp"
#include "../Statistics/StatisticsCalculator.hpp"
#include "../Statistics/StatisticsContainer.hpp"

#include <ctime>
#include <queue>
#include <vector>

class UniversityModelNewClientEvent;

class UniversityModelTimeStepping : public TimeStepping {
public:
	explicit UniversityModelTimeStepping(); //(BitcoinNetwork* btcNetwork);
	virtual ~UniversityModelTimeStepping();
	void postProcessing(NetworkEvent* networkEvent); // implement abstract class
	std::ostream& outputStatistics(std::ostream& o) const;

	inline UniversityModelClient& getClientRepresentation(unsigned int pos)
	{
		assert(pos < clientRepresentations.size());
		return clientRepresentations.at(pos);
	}

	inline const UniversityModelClient& getClientRepresentation(unsigned int pos) const
	{
		assert(pos < clientRepresentations.size());
		return clientRepresentations.at(pos);
	}

	inline const std::map<unsigned int, std::vector<unsigned int> >& getProfileToClientsMapping() const
	{
		return profileToClientLinks;
	}

	inline unsigned int getBitcoinSellerID(void) const
	{
		return BitcoinSellerID_;
	}

private:
	void planAndEnqueueTransactions(const tm& planTimeStamp);
	void planAndEnqueueTransactions(const tm& planTimeStamp, UniversityModelClient& client,
						const Profile& clientProfile, const ProfileEvent& profileEvent, const Event& event, unsigned int umcPos);
	void planAndEnqueueInterExchanges(const tm& plantimeStamp, const UniversityModelClient& client,
									const Profile& clientProfile, unsigned int umcPos);
	void planAndEnqueuePrivacyAwareness(const tm& plantimeStamp, const UniversityModelClient& client,
									const Profile& clientProfile, unsigned int umcPos);
	void planAndEnqueueBitcoinPurchase(const UniversityModelClient& client, const tm& timestampOfLastFailure);

	UniversityModelNewClientEvent* createNewClient(const tm& ts);
	UniversityModelNewClientEvent* createNewSeller(const tm& ts, unsigned int storeID);
	UniversityModelPlanningEvent* createPlanningEvent(const tm& nextTimestamp);
	std::vector<unsigned int> getAvailableDaysOfWeek(const Event& ev) const;
	//unsigned int maxInitAddressCount;
	//unsigned int minInitAddressCount;
	int planningPeriodInDays;
	double exchangeRate;
	unsigned int allowedFailedTransactionCount;
	unsigned long issueTransactionCount_;
	unsigned long issueTransactionFailureCount_;
	unsigned long plannedBitcoinPurchasesCount_;
	unsigned long plannedPrivacyAwareTransCount_;
	unsigned long executedBitcoinPurchasesCount_;
	unsigned long privacyAwareUserCount_;
	unsigned int numDifferentBitcoinSellerAddresses_;
	unsigned int stdForTimeOfEvents_;
	bool eventOneTimePad_;
	double probabilityForBitcoinTransaction_;
	std::vector<UniversityModelClient> clientRepresentations;
	unsigned int BitcoinSellerID_;
	std::map<unsigned int, unsigned int> storeSellerLinks;
	std::map<unsigned int, std::vector<unsigned int> > profileToClientLinks;
	StatisticsContainer statContainer;
};

#endif /* UNIVERSITYMODELTIMESTEPPING_HPP_ */
