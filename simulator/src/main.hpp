// TODO:
// - change address event / privacy awareness: todo
// - non-availability: todo
// - improve planning time step
// - interexchange: min/max friends
// - Bitcoin sellers
// - Validation


#ifndef MAIN_HPP_
#define MAIN_HPP_


#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <cassert>
#include <set>
#include <iostream>

typedef unsigned long long btc;

// Streams for logging (defined in main.cpp)
extern std::ofstream logFile;
//extern std::ofstream compactLog;
//extern std::ofstream parameterLog;

extern const unsigned int GENERATION;
extern const unsigned int GAID_NOTSET;

extern const btc ONECENT;

extern int daylightSavingOffset;

// the program's keyword/value option pairs
typedef std::map<std::string, std::string> options_t;

struct Coin
{
public:
	inline Coin(const Coin& b):
		amount_(b.getValue())
	{
	}

	inline Coin(const btc& amount):
		amount_(amount)
	{
		assert(amount_ > 0);
	}

	inline ~Coin() {}

	inline void outputBTC(std::ostream& os) const
	{
		   btc decimals = (amount_%100);
		   os << (amount_/100) << '.';
		   if(decimals<10) os << '0';
		   os << decimals;
		   //os << " BTC";
	}

	inline btc getValue(void) const
	{
		return amount_;
	}

	inline bool operator <(const Coin& b) const{
		return (amount_ < b.getValue());
	}

	inline const Coin& operator =(const Coin& b) {
		if(this != &b)
		{
			amount_ = b.getValue();
		}
		return *this;
	}
private:
	btc amount_;
};

extern std::ostream& operator<<(std::ostream& o, const Coin& coin);

template <typename T>
std::string Str( const T & t ) {
   std::ostringstream os;
   os << t;
   return os.str();
}

/*
std::string secondsToDayTime(unsigned int seconds)
{
   std::ostringstream os;
   os <<
   return os.str();
}
*/

template <typename T, typename U>
std::vector<T> separatePairVect(const std::vector<std::pair<T, U> >& vec) {
	std::vector<T> v;
	for(unsigned int i = 0; i < vec.size(); ++i)
	{
		v.push_back(vec[i].first);
	}
	assert(v.size() == vec.size());
	return v;
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& o, const std::pair<T, U>& pair)
{
	o << "(" << pair.first << "," << pair.second << ")";
	return o;
}

template <typename T>
std::ostream& operator<<(std::ostream& o, const std::vector<T>& vec)
{
	o << "[";
	for(unsigned int i = 0; i < vec.size(); ++i)
	{
		o << vec[i];
		if(i < (vec.size()-1)) o << ", ";
	}
	o << "]";
	return o;
}

template <typename T>
std::ostream& operator<<(std::ostream& o, const std::set<T>& set)
{
	unsigned int count = 0;
	o << "{";
	for(typename std::set<T>::const_iterator cit = set.begin(); cit != set.end(); ++cit)
	{
		o << *cit;
		if(count < (set.size()-1)) o << ", ";
		count++;
	}
	o << "}";
	return o;
}

template <typename T>
std::ostream& operator<<(std::ostream& o, const std::multiset<T>& set)
{
	unsigned int count = 0;
	o << "{";
	for(typename std::multiset<T>::const_iterator cit = set.begin(); cit != set.end(); ++cit)
	{
		o << *cit;
		if(count < (set.size()-1)) o << ", ";
		count++;
	}
	o << "}";
	return o;
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& o, const std::map<T, U>& map)
{
	for(typename std::map<T, U>::const_iterator it = map.begin(); it != map.end(); ++it)
	{
		o << it->first << " -> " << it->second << std::endl;
	}
	return o;
}

template <typename T>
bool convertStringToType(const std::string &str, T &val) {
	std::istringstream iss(str);
	if(!(iss >> val)) return false;
	else return true;
}

template <typename S>
class Singleton
{
public:
	template <typename P>
	static const S& getInstance(const P& pval)
	{
		if(instancePointer_ == NULL) instancePointer_ = new S(pval);
		return *instancePointer_;
	}

	static const S& getInstance(void)
	{
		if(instancePointer_ == NULL) instancePointer_ = new S;
		return *instancePointer_;
	}

	static void destroy(void)
	{
		if ( instancePointer_ != NULL )
			delete instancePointer_;
		instancePointer_ = NULL;
	}
protected:
	Singleton() {}
	~Singleton() {}
private:
	static S* instancePointer_;
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&) { return *this; }
};

template <typename S>
S* Singleton<S>::instancePointer_ = NULL;

#endif /* MAIN_HPP_ */
