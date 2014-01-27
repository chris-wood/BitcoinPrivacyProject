#ifndef SAMPLER_HPP_
#define SAMPLER_HPP_

#include "ParameterList.hpp"
#include "main.hpp"

#include <ctime>
#include <string>
#include <vector>
#include <sstream>

// this is a util/helper class to sample from probability distributions and do other interesting stuff


struct ProbPriceRange
{
	//double minProb;
	//double maxProb;

	btc minPrice;
	btc maxPrice;

	double minPref;
	double maxPref;
};

extern std::ostream& operator<<(std::ostream& o, const ProbPriceRange& ppr);

class Sampler {
public:

	static void addTimeInterval(tm& x, int y, int month, int d, int h, int min, int sec, bool noTimesaving = false);
	static tm addTimeInterval(const tm& timeStamp, int year, int month, int day, int hour, int min, int sec, bool noTimesaving = false);
	static int dayDiff(const tm& tm1, const tm& tm2, bool noTimesaving = false);

	static double rand01();
	static double randFloatNr(double fMin, double fMax);
	static unsigned int randRange(unsigned int end);
	static btc randRangeBTC(btc start, btc end);
	static int randRange(int start, int end);
	static std::string tmToString(const tm& timestamp, bool dayTimeOnly = false);
	static std::string tmToStringWday(const tm& timestamp);
	static std::string tmToLogFormat(const tm& timestamp);
	static std::string tmToStringDayTime(const tm& timestamp);
	static unsigned int secondsFromMidnight(const tm& timestamp);
	static void tmToFilename(std::string& outStr, tm & timestamp);
	static bool tm1BeforeTm2(const tm& tm1, const tm& tm2);
	static bool tm1BeforeORsmultanousWithTm2(const tm& tm1, const tm& tm2);
	static tm stringToTm(const std::string &inStr);
	static std::string dayTimeStr(unsigned int hours, unsigned int mins, unsigned int seconds);
	static std::vector<unsigned int> randRangeVect(int start, int end , int numElem);
	static std::vector<unsigned int> randVectPermutation(unsigned int minNr, unsigned int maxNr, unsigned int numElems);
	static std::vector<unsigned int> randVectPermutation(const std::vector<unsigned int>& vec, unsigned int numElems);
	static std::string randAlphaNumString(const unsigned int len);

	static unsigned int getWeekDayFromTm(const tm& timestamp);
	static std::vector<unsigned int> weekDayDiffVec(const std::vector<unsigned int>& daysOfTransactions,
			unsigned int shiftDay);

	static double sampleFromExponentialDistribution(double lambda);
	static unsigned int sampleFromCategoricalDistribution(const std::vector<double>& propensities);
	static int sampleFromPoissonDistribution(double lambda, double tau);
	static double sampleFromNormalDistribution(double mu, double sigma);

	static double sampleMinutesUntilNextClientGen(double lambda);
	static double sampleMinutesUntilNextBlockGen(double p, double bucketSizeInMins);

	static std::vector<double> sampleUniformlyFromSimplex(unsigned int dim);
	static std::vector<double> sampleUniformlyFromSimplexSubPart(const std::vector<ProbPriceRange> ranges);
	static std::vector<double> sampleUniformlyFromSimplexSubPartBIASED(const std::vector<ProbPriceRange> ranges);

	static std::vector<double> sampleProbabilityDistrFromPrefs(const std::vector<ProbPriceRange> ranges);

	static std::vector<btc> sampleCompositions(unsigned int numCompositions, btc totalAmount);
private:
	Sampler();
	~Sampler();
};

struct TimeSpan
{
	tm startTime;
	tm endTime;
};

extern std::ostream& operator<<(std::ostream& o, const TimeSpan& timespan);

extern std::ostream& operator<<(std::ostream& o, const tm& ctimeStruct);

#endif /* SAMPLER_HPP_ */
