#include "Sampler.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <memory>
#include <cstring>

// this is a util/helper class to sample from probability distributions and do other interesting stuff

Sampler::Sampler(void) {
	// do nothing this should not be instantiated
}

Sampler::~Sampler() {
	// nothing
}


std::vector<unsigned int> Sampler::weekDayDiffVec(const std::vector<unsigned int>& daysOfTransactions,
		unsigned int shiftDay)
{
	std::vector<unsigned int> v(daysOfTransactions.size());
	assert(shiftDay >=1 && shiftDay <= 7);
	unsigned int dimShift = shiftDay-1;
	for(unsigned int i = 0; i < daysOfTransactions.size(); ++i)
	{
		assert(daysOfTransactions[i] >= 1 && daysOfTransactions[i] <= 7);
		unsigned int dimDay = daysOfTransactions[i] -1 +7;
		v[i] = (dimDay - dimShift)%7;
		if(v[i] == 0) v[i] = 7;
		assert(v[i] >= 1 && v[i] <= 7);
	}
	return v;
}


unsigned int Sampler::getWeekDayFromTm(const tm& timestamp)
{
	unsigned int dayOfweek = timestamp.tm_wday; /* Wochentag (0 bis 6, Sonntag = 0) */
	dayOfweek = (dayOfweek+6)%7 +1;
	assert(dayOfweek >= 1 && dayOfweek <= 7); // monday = 1, ..., sunday = 7
	return dayOfweek;
}

std::string Sampler::tmToLogFormat(const tm& timestamp)
{
	// format 13 1 2009 9 37 41
	  char buffer [40];
	  tm timeinfo = timestamp;
	  size_t numChars = strftime (buffer,40,"%d %m %Y %H %M %S",&timeinfo);
	  return std::string(buffer, numChars);
}

std::string Sampler::tmToString(const tm& timestamp, bool dayTimeOnly) {
	  char buffer[40];
	  //std::memset(buffer, 0, 40*sizeof(char));
	  tm timeinfo;
	  //std::memset(&timeinfo, 0, sizeof(timeinfo));
	  timeinfo = timestamp;
	  size_t numChars;
	  if(dayTimeOnly)
	  {
		  numChars = std::strftime (buffer,40,"%H:%M:%S",&timeinfo);
	  }
	  else
	  {
		  numChars = std::strftime (buffer,40,"%Y-%m-%d-%H:%M:%S",&timeinfo);
	  }
	  return std::string(buffer, numChars);
}

std::string Sampler::tmToStringWday(const tm& timestamp) {
	char buffer [40];
	tm timeinfo = timestamp;
	size_t numChars = strftime (buffer,40,"%Y-%m-%d-%H:%M:%S (%a)",&timeinfo);
	return std::string(buffer, numChars);
}

std::string Sampler::tmToStringDayTime(const tm& timestamp) {
	char buffer [40];
	tm timeinfo = timestamp;
	size_t numChars = strftime (buffer,40,"%H:%M:%S",&timeinfo);
	return std::string(buffer, numChars);
}

unsigned int Sampler::secondsFromMidnight(const tm& timestamp)
{
	unsigned int result = timestamp.tm_hour*3600 + timestamp.tm_min*60 + timestamp.tm_sec;
	assert(result <= 86400);
	return result;
}

void Sampler::tmToFilename(std::string& outStr, tm & timestamp) {
	  char buffer [40];
	  tm timeinfo = timestamp;
	  size_t numChars = strftime (buffer,40,"%Y%m%d%H%M%S",&timeinfo);
	  outStr = string(buffer, numChars);
}

tm Sampler::stringToTm(const std::string &inStr)
{
	tm parsedTm;
	std::memset(&parsedTm, 0, sizeof(parsedTm));
	char stringFormat[] = "%Y-%m-%d-%H:%M:%S";
	strptime(inStr.c_str(), stringFormat, &parsedTm);
	//std::cout << "parsed: " << tmToString(parsedTm) << "; "<< parsedTm.tm_gmtoff <<  "; " << parsedTm.tm_isdst << " fdsa"<< std::endl;
	/*parsedTm.tm_gmtoff = 0;
	parsedTm.tm_isdst = -300;
	std::cout << "before mktime: " << tmToString(parsedTm) << std::endl;
	mktime(&parsedTm);
	std::cout << "after mktime: " << tmToString(parsedTm) << std::endl;*/
	time_t timeT = mktime(&parsedTm);
	tm *ptm = gmtime ( &timeT );
	tm tmReturn = *ptm;
	return tmReturn;
}

std::string Sampler::dayTimeStr(unsigned int hours, unsigned int mins, unsigned int seconds)
{
	tm parsedTm;
	std::memset(&parsedTm, 0, sizeof(parsedTm));
	parsedTm.tm_hour = hours;
	parsedTm.tm_min = mins;
	parsedTm.tm_sec = seconds;
	return tmToStringDayTime(parsedTm);
}

bool Sampler::tm1BeforeTm2(const tm& tm1, const tm& tm2){
	//we need local copies (mktime modifies tm)
	tm tm1loc = tm1;
	tm tm2loc = tm2;
	time_t time1 = mktime(&tm1loc);
	time_t time2 = mktime(&tm2loc);
	if (time1 < time2) return true;
	return false;
}

bool Sampler::tm1BeforeORsmultanousWithTm2(const tm& tm1, const tm& tm2){
	//we need local copies (mktime modifies tm)
	tm tm1loc = tm1;
	tm tm2loc = tm2;
	time_t time1 = mktime(&tm1loc);
	time_t time2 = mktime(&tm2loc);
	if (time1 <= time2) return true;
	return false;
}


void Sampler::addTimeInterval(tm& timeStamp, int year, int month, int day, int hour, int min, int sec, bool noTimesaving) {
	int dlsv_before = timeStamp.tm_isdst;
	timeStamp.tm_year += year;
	timeStamp.tm_mon += month;
	timeStamp.tm_mday += day;
	timeStamp.tm_hour += hour;
	timeStamp.tm_min += min;
	timeStamp.tm_sec += sec;
	tm temp = timeStamp;
    mktime(&timeStamp); /* normalize result */

    int dlsv_after = timeStamp.tm_isdst;

    if(noTimesaving && dlsv_before != dlsv_after)
    {
    	timeStamp = temp;
    	timeStamp.tm_isdst = dlsv_after;

    	mktime(&timeStamp);
    }
}

tm Sampler::addTimeInterval(const tm& timeStamp, int year, int month, int day, int hour, int min, int sec, bool noTimesaving) {
	tm tS = timeStamp;
	addTimeInterval(tS, year, month, day, hour, min, sec, noTimesaving);
	return tS;
}

int Sampler::dayDiff(const tm& tm1, const tm& tm2, bool noTimesaving)
{
	//std::cout << tmToString(tm1) << " - " << tmToString(tm2) << std::endl;
	tm ltm1 = tm1;
	tm ltm2 = tm2;
	ltm1.tm_hour = ltm2.tm_hour = 4;
	ltm1.tm_min = ltm2.tm_min = 0;
	ltm1.tm_sec = ltm2.tm_sec = 0;
	time_t time1 = mktime(&ltm1);
	time_t time2 = mktime(&ltm2);
	if(noTimesaving && ltm2.tm_isdst != ltm1.tm_isdst)
	{
		ltm2.tm_isdst = ltm1.tm_isdst;
		time2 = mktime(&ltm2);
	}
	double doubleDiff = difftime(time2, time1);
	int diff = (int) doubleDiff;
	diff = diff/(24*60*60);
	//std::cout << doubleDiff << ", " << diff << std::endl;
	return diff;
}

// TODO: Might need better algorithms for drawing numbers... SSL or boost libraries might be an option

// Return a random number uniformly distributed in [0, 1]
double Sampler::rand01() {
	return (double)rand() / (double)RAND_MAX;
}

double Sampler::randFloatNr(double fMin, double fMax)
{
	if(fMin == fMax) return fMin;
	assert(fMin < fMax);
    double f = rand01();
    return fMin + f * (fMax - fMin);
}


// Return a random integer between [0, end)
unsigned int Sampler::randRange(unsigned int end) {
	assert(end <= RAND_MAX);

	// Generate an integer between [0, end) uniformly
	int r = rand();
	int remainder = RAND_MAX % end;
	while (r < remainder) r = rand();
	return r % end;
}

// Return a random vector with numbers between [start, end), if numElem is zero, return value is empty vector
std::vector<unsigned int> Sampler::randRangeVect(int start, int end , int numElem) {
	assert(numElem >= 0);
	std::vector<unsigned int> vec;
	for(int i = 0; i < numElem; ++i)
	{
		vec.push_back(randRange(start, end));
	}
	return vec;
}

std::vector<unsigned int> Sampler::randVectPermutation(const std::vector<unsigned int>& vec, unsigned int numElems)
{
	std::vector<unsigned int> baseVec = vec;
	assert(numElems > 0);
	vector<unsigned int> res;
	unsigned int size = baseVec.size();
	while(numElems > size)
	{
		res.insert(res.end(), baseVec.begin(), baseVec.end());
		numElems -= size;
	}
	assert(numElems <= (size));
	std::random_shuffle (baseVec.begin(), baseVec.end());
	baseVec.resize(numElems);
	res.insert(res.end(), baseVec.begin(), baseVec.end());
	return res;
}

std::vector<unsigned int> Sampler::randVectPermutation(unsigned int minNr, unsigned int maxNr, unsigned int numElems)
{
	assert(numElems > 0);
	vector<unsigned int> res;
	assert(maxNr >= minNr);
	vector<unsigned int> v;
	for (unsigned int i = minNr; i <= maxNr; ++i)
		v.push_back (i);

	unsigned int size = maxNr-minNr+1;
	assert(size == v.size());

	while(numElems > (size))
	{
		res.insert(res.end(), v.begin(), v.end());
		numElems -= size;
	}


	assert(numElems <= (size));

	std::random_shuffle (v.begin(), v.end());
	v.resize(numElems);
	res.insert(res.end(), v.begin(), v.end());

	return res;
}

// Return a random number between [start, end)
int Sampler::randRange(int start, int end) {
	assert(start < end);
	return start + randRange(end - start);
}

// Sample in BTC range uniformly [start, end)
btc Sampler::randRangeBTC(btc start, btc end)
{
	assert(start < end);
	return start + randRange(end - start);
}

// Return a random string of length len consisting of alphanumeric symbols
std::string Sampler::randAlphaNumString(const unsigned int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    char* s = new char[len+1];
    for (unsigned int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
    std::string result = s;
    delete[] s;
    return result;
}

double Sampler::sampleMinutesUntilNextBlockGen(double p, double bucketSizeInMins)
{
	double lambda = -log(1 - p);
	double expDistr = sampleFromExponentialDistribution(lambda);
	return expDistr*bucketSizeInMins; // TODO: not absolutely sure about this
}

double Sampler::sampleMinutesUntilNextClientGen(double lambda)
{
	double expDistr = sampleFromExponentialDistribution(lambda);
	return expDistr;
}

double Sampler::sampleFromExponentialDistribution(double lambda)
{
	double r = ((double)rand())/((double)RAND_MAX);
	if(r > 0)
	{
		double tau = -log(r)/(lambda);
		if(tau != std::numeric_limits<double>::infinity() && tau != std::numeric_limits<double>::quiet_NaN())
		{
			return tau;
		}
	}
	return 0;
}

unsigned int Sampler::sampleFromCategoricalDistribution(const std::vector<double>& propensities)
{
	double r = rand01();
	double sum = 0.0;
	double total = 0.0;
	for(size_t i = 0; i < propensities.size(); ++i)
	{
		if(propensities[i] > 0.0)
			total += propensities[i];
	}
	assert(total > 0.0);
	unsigned int mu=0;
	for(; mu < propensities.size(); mu++)
	{
		if(propensities[mu] <= 0.0) continue;
		double r_val = r*total;
		if(sum < r_val && r_val <= sum+propensities[mu])
		{
			break;
		}
		sum += propensities[mu];
	}
	assert(propensities[mu] > 0.0);
	return mu;
}

int Sampler::sampleFromPoissonDistribution(double lambda, double tau)
{
	// Not sure if that is correct?
	/*Real l = lambda * tau;
	Real cdf = exp(-l);
	Real pdf = cdf;
	int k = 0;
	Real r = ((double)rand())/((double)RAND_MAX)*0.999999;

	while(r > cdf)
	{
		k++;
		assert(k > 0);
		pdf = pdf * l/((Real)k);
		cdf += pdf;
	}
	return k;*/

	// Implementation form MCMC course:
	double L = exp(-lambda * tau);
	int k = 0;
	double p = 1;
	do
	{
		k = k+1;
		double r = ((double)rand())/((double)RAND_MAX);
		p = r * p;
	}
	while(p > L);

	return k-1;
}

double Sampler::sampleFromNormalDistribution(double mu, double sigma)
{
	double u1 = 0.0;
	double u2 = 0.0;

	do
	{
		u1 = rand01();
		u2 = rand01();
	}	while (u1 <= 0.0 && u2 <= 0.0);

	double z = sqrt((-2.0)*log(u1)) * cos(2.0* atan(1.0)*4.0 *u2);

	return z*sigma + mu;
}

std::vector<btc> Sampler::sampleCompositions(unsigned int numCompositions, btc totalAmount)
{
	// TODO: this might be biased
	std::vector<btc> result;
	btc remaining = totalAmount;
	for(unsigned int i = 0; i < (numCompositions-1); ++i)
	{
		btc part = Sampler::randRangeBTC(0, remaining+1);
		result.push_back(part);
		remaining -= part;
	}
	result.push_back(remaining);

	assert(result.size() == numCompositions);
	btc sum = 0;
	for(unsigned int i = 0; i < result.size(); ++i) sum += result[i];
	assert(sum == totalAmount);
	return result;
}

std::vector<double> Sampler::sampleUniformlyFromSimplex(unsigned int dim)
{
	std::vector<double> result;
	double sum = 0.0;
	for(unsigned int i = 0; i < dim; ++i)
	{
		double expVal = Sampler::sampleFromExponentialDistribution(1.0);
		result.push_back(expVal);
		sum += expVal;
	}
	for(unsigned int i = 0; i < dim; ++i)
	{
		result[i] /= sum;
	}
	return result;
}

/*
std::vector<double> Sampler::sampleUniformlyFromSimplexSubPart(const std::vector<double> probabilities)
{
	double sumCheck = 0.0;
	for(unsigned int i = 0; i < probabilities.size(); ++i)
	{
		assert(probabilities[i] >= 0.0 && probabilities[i] <= 1.0);
		//std::cout << ranges[i].toString() << std::endl;
		sumCheck += probabilities[i];
	}
	//std::cout << "Sum of maxProbs: " << sumCheck << std::endl;
	assert(sumCheck == 1.0); //probability distribution

	// rejection sampling
	std::vector<double> simplexRes;
	bool inRanges = false;
	do
	{
		simplexRes = sampleUniformlyFromSimplex(probabilities.size());
		//
		//for(unsigned int i = 0; i < simplexRes.size(); ++i)
		//{
		//	std::cout << simplexRes[i] << ", ";
		//}
		//std::cout << std::endl;
		//
		inRanges = true;
		for(unsigned int i = 0; i < ranges.size(); ++i)
		{
			if(ranges[i].minProb == ranges[i].maxProb)
			{
				simplexRes[i] = ranges[i].minProb;
			}
			else if(simplexRes[i] < ranges[i].minProb || simplexRes[i] > ranges[i].maxProb)
			{
				//std::cout << "pos " << i << " violated" << std::endl;
				inRanges = false;
				break;
			}
		}
	} while(!inRanges);

	return simplexRes;
}
*/

/*
std::vector<double> Sampler::sampleUniformlyFromSimplexSubPartBIASED(const std::vector<ProbPriceRange> ranges)
{
	double sumCheck = 0.0;
	for(unsigned int i = 0; i < ranges.size(); ++i)
	{
		assert(ranges[i].minProb >= 0 && ranges[i].maxProb >= 0);
		//std::cout << ranges[i].toString() << std::endl;
		sumCheck += ranges[i].maxProb;
	}
	//std::cout << "Sum of maxProbs: " << sumCheck << std::endl;
	assert(sumCheck >= 0.9999); //probability distribution

	std::vector<double> res(ranges.size());
	double sum = 0.0;
	for(unsigned int i = 0; i < ranges.size(); ++i)
	{
		if(ranges[i].minProb == ranges[i].maxProb)
		{
			res[i] = ranges[i].minProb;
		}
		else
		{
			res[i] = randFloatNr(ranges[i].minProb, ranges[i].maxProb);
		}
		sum += res[i];
	}
	// normalize
	for(unsigned int i = 0; i < ranges.size(); ++i)
	{
		res[i] /= sum;
	}
	return res;
}
*/

std::vector<double> Sampler::sampleProbabilityDistrFromPrefs(const std::vector<ProbPriceRange> ranges)
{
	std::vector<double> res(ranges.size());
	double sum = 0.0;
	double probSum = 0.0;
	for(unsigned int i = 0; i < ranges.size(); ++i)
	{
		assert(ranges[i].minPref <= ranges[i].maxPref);
		double randPref = ranges[i].minPref;
		if(ranges[i].minPref < ranges[i].maxPref) randPref = randFloatNr(ranges[i].minPref, ranges[i].maxPref);
		sum += randPref;
		res[i] = randPref;
	}
	for(unsigned int i = 0; i < res.size(); ++i)
	{
		res[i] /= sum;
		probSum += res[i];
	}

	assert(std::abs(1.0 - probSum) <= 0.0001);

	return res;
}

std::ostream& operator<<(std::ostream& o, const ProbPriceRange& ppr)
{
	assert(ppr.minPref >= 0);
	assert(ppr.minPref <= ppr.maxPref);

	//assert(ppr.maxProb >= 0.0 && ppr.maxProb <= 1.0);
	//assert(ppr.minProb >= 0.0 && ppr.minProb <= 1.0);
	//assert(ppr.minProb <= ppr.maxProb);

	assert(ppr.minPrice > 0.0);
	assert(ppr.minPrice <= ppr.maxPrice);

	o << "Range[";
	//o << "Prob=[" << ppr.minProb << "," << ppr.maxProb << "]";
	o << "Pref=";
	o << "[" << ppr.minPref << "," << ppr.maxPref << "]";
	o << ",Price=";
	o << "[" << ppr.minPrice << "," << ppr.maxPrice << "]";
	o << "]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const TimeSpan& timespan)
{
	assert(Sampler::tm1BeforeTm2(timespan.startTime, timespan.endTime));
	o << "[";
	o << Sampler::tmToString(timespan.startTime);
	o << " - ";
	o << Sampler::tmToString(timespan.endTime);
	o << "]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const tm& ctimeStruct)
{
	o << Sampler::tmToString(ctimeStruct);
	return o;
}
