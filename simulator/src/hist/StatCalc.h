#ifndef StatCalc
#define StatCalc


#include <cassert>
#include <limits>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

class PrintableStatistic {
public:
	inline virtual ~PrintableStatistic() {}

	virtual std::ostream& output(std::ostream& o) const
	{
		o << "# " << name_ << " (" << count_ << ") : " << std::endl;
		outputInternal(o);
		return o;
	}

	virtual std::ostream& outputInternal(std::ostream& o) const = 0;

	inline long getCount(void) const { return count_; }

	inline const std::string& getName(void) const { return name_; }

	static inline char* gmTime(const time_t &time, char* timeBuf, size_t sizeBuf)
	{
		struct tm gmTime;
		gmtime_r(&time, &gmTime);
		size_t sz = strftime (timeBuf, sizeBuf, "%Y %d %m %H %M %S", &gmTime);
		assert(strlen(timeBuf)==sz && sz != 0);
		return timeBuf;
		/* is already null-terminated! below is not needed
		size_t sz =strlen(timeBuf);
		if(0<sz) timeBuf[sz-1] = 0;
		*/
	}

protected:
	std::string name_;
	long count_;

	inline PrintableStatistic(std::string& name):	name_(name), count_(0) {}
};


extern std::ostream& operator<<(std::ostream& o, const PrintableStatistic& sC);
extern void operator<<(const std::string&, const PrintableStatistic& sC);

class Statistic1D : public PrintableStatistic {
public:
	inline virtual ~Statistic1D() {}
	inline void addNewValue(double newVal)
	{
		count_++;
		addValue(newVal); //abstract method
	}
protected:
	virtual void addValue(double newVal) = 0;
	inline Statistic1D(std::string& name) : PrintableStatistic(name) {}
};


class Statistic2D : public PrintableStatistic {
public:
	inline virtual ~Statistic2D() {}
	inline void addNewValue(double newVal_x, double newVal_y)
	{
		count_++;
		addValue(newVal_x, newVal_y); //abstract method
	}
protected:
	virtual void addValue(double newVal_x, double newVal_y) = 0;
	inline Statistic2D(std::string& name) : PrintableStatistic(name) {}
};


class MeanStdMinMax : public Statistic1D {
public:
	inline virtual ~MeanStdMinMax() {}

	inline MeanStdMinMax(std::string name):
		Statistic1D(name),
		mean_(0.0),
		M2_(0.0),
		smallest_(std::numeric_limits<double>::infinity()),
		largest_(-std::numeric_limits<double>::infinity())
	{}

	virtual inline void addValue(double newVal)
	{
		if (newVal > largest_)
			largest_ = newVal;
		if (newVal < smallest_)
			smallest_ = newVal;

		double delta = newVal - mean_;
		mean_ = mean_ + delta/((double) getCount());
		M2_ = M2_ + delta*(newVal - mean_);
	}

	inline double getMean(void) const { return mean_; }
	inline double getStd(void) const { return std::sqrt(getVar()); }
	inline double getVar(void) const { return M2_/((double)(getCount() - 1)); }
	inline double getMin(void) const { return smallest_; }
	inline double getMax(void) const { return largest_; }

	std::ostream& outputInternal(std::ostream& o) const;

	inline static void toHistogramFormat(std::ostream& o, const MeanStdMinMax& it, long totalCount)
	{
		o << it.getCount() << "\t" << ((double)it.getCount())/((double)totalCount) << "\t" << it.getMean() 
		  << "\t" << it.getStd()   << "\t" << it.getVar()                                  << "\t" << it.getMin()
		  << "\t" << it.getMax()   << std::endl;
	}

private:
	double mean_;
	double M2_;
	double smallest_;
	double largest_;
};


class MeanStdMinMaxVal : public MeanStdMinMax {
public:
	inline virtual ~MeanStdMinMaxVal() {}
	inline MeanStdMinMaxVal(std::string name):
		MeanStdMinMax(name),
		val_(std::numeric_limits<double>::signaling_NaN()),
		//val_(0.0),
		isValSet_(false)
	{}
	inline double getVal(void) const { return val_; }
	inline void setVal(double val)
	{	
		//if(unlikely(std::isnan(val_)))
		if((!isValSet_))
		{
			val_ = val;
			isValSet_ = true;
		}
	}

	inline void addValue(double newVal)
	{
		MeanStdMinMax::addValue(newVal);
		setVal(newVal);
	}

	inline static void toHistogramFormat(std::ostream& o, const MeanStdMinMaxVal& it, long totalCount)
	{
		o << it.getVal() << "\t" << it.getCount() << "\t" << ((double)it.getCount())/((double)totalCount) << "\t" << it.getMean() 
		  << "\t" << it.getStd()   << "\t" << it.getVar()                                  << "\t" << it.getMin()
		  << "\t" << it.getMax()   << std::endl;
	}
private:
	double val_;
	bool isValSet_;
};

template <class BUCKET_TYPE> class Histogram
{
public:
	inline virtual ~Histogram() {}
	inline BUCKET_TYPE& addToBucket(double newVal, double affiliation)
	{
		totStats_.addNewValue(newVal);

		if((affiliation < boundaryLeft_))
		{
			leftBucket_.addNewValue(newVal);
			return leftBucket_;
		}
		else if((affiliation >= boundaryRight_))
		{
			rightBucket_.addNewValue(newVal);
			return rightBucket_;
		}
		else
		{
			long index = (long) ((affiliation-boundaryLeft_)/step_);
			buckets_[index].addNewValue(newVal);
			return buckets_[index];
		}
	}

	inline void add(double newVal) { addToBucket(newVal, newVal); }
protected:
	Histogram(long numBuckets, bool exceedingLeft, bool exceedingRight, double boundaryLeft, double boundaryRight, BUCKET_TYPE bucket):
		buckets_(numBuckets, bucket),
		totStats_(bucket),
		leftBucket_(bucket),
		rightBucket_(bucket),
		numIntermediateBuckets_(numBuckets),
		exceedingLeft_(exceedingLeft),
		exceedingRight_(exceedingRight),
		boundaryLeft_(boundaryLeft),
		boundaryRight_(boundaryRight),
		step_((boundaryRight-boundaryLeft)/((double)numBuckets))
	{}

	std::vector<BUCKET_TYPE> buckets_;
	BUCKET_TYPE totStats_;	
	BUCKET_TYPE leftBucket_;
	BUCKET_TYPE rightBucket_;
	long numIntermediateBuckets_;
	bool exceedingLeft_;
	bool exceedingRight_;
	double boundaryLeft_;
	double boundaryRight_;
	double step_;
};


class Histogram1D : public Statistic1D, public Histogram<MeanStdMinMax> {
public:
	Histogram1D(std::string name, long numBuckets, bool exceedingLeft, bool exceedingRight, double boundaryLeft, double boundaryRight):
		Statistic1D(name),
		Histogram<MeanStdMinMax>(numBuckets, exceedingLeft, exceedingRight, boundaryLeft, boundaryRight, MeanStdMinMax(name))
	{}
	inline virtual ~Histogram1D() {}
	inline void addValue(double newVal) { add(newVal); }
	std::ostream& outputInternal(std::ostream& o) const;
};


class Histogram2D : public Statistic2D, public Histogram<Histogram1D> {
public:
	Histogram2D(std::string name, long numBuckets_x, bool exceedingLeft_x, bool exceedingRight_x, double boundaryLeft_x, double boundaryRight_x,
		                      long numBuckets_y, bool exceedingLeft_y, bool exceedingRight_y, double boundaryLeft_y, double boundaryRight_y):
		Statistic2D(name),
		Histogram<Histogram1D>(numBuckets_x, exceedingLeft_x, exceedingRight_x, boundaryLeft_x, boundaryRight_x,
					Histogram1D(name, numBuckets_y, exceedingLeft_y, exceedingRight_y, boundaryLeft_y, boundaryRight_y))
	{}
	inline virtual ~Histogram2D() {};
	inline void addValue(double newVal_x, double newVal_y) { addToBucket(newVal_y, newVal_x); }
	std::ostream& outputInternal(std::ostream& o) const;
};


class TimeSeries : public Statistic2D, public Histogram<MeanStdMinMaxVal> {
public:
	TimeSeries(std::string name, long numBuckets, bool exceedingLeft, bool exceedingRight, double boundaryLeft, double boundaryRight):
		Statistic2D(name),
		Histogram<MeanStdMinMaxVal>(numBuckets, exceedingLeft, exceedingRight, boundaryLeft, boundaryRight, MeanStdMinMaxVal(name)),
		oldXVal_(-std::numeric_limits<double>::infinity())
	{}
	inline virtual ~TimeSeries() {};
	inline void addValue(double newVal_x, double newVal_y)
	{
		/*
		#ifdef MYDEBUG
			//printf("%f\n", newVal_x);
			if(newVal_x < oldXVal_)
				printf("old:%" PRIu64 ", new:%" PRIu64 " \n", (long)oldXVal_, (long)newVal_x);
			assert(newVal_x >= oldXVal_);
			oldXVal_ = newVal_x;
		#endif
		*/
		//MeanStdMinMaxVal& bucket =*/ addToBucket(newVal_y, newVal_x);
		addToBucket(newVal_y, newVal_x);
		//bucket.setVal(newVal_y);
		//assert(!std::isnan(bucket.getVal()));
	}			
	std::ostream& outputInternal(std::ostream& o) const;
private:
	double oldXVal_;
};


#endif /* StatCalc */
