
#include "StatisticsCalculator.hpp"
#include <cassert>
#include <limits>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

StatisticsCalculator::~StatisticsCalculator() {
}

StatisticsCalculator::StatisticsCalculator(std::string& name):
	name_(name),
	count_(0)
{
}

void StatisticsCalculator::addNewValue(double newVal)
{
	count_++;
	addValue(newVal);
}

//===========================================================

MeanStd::~MeanStd() {
}

MeanStd::MeanStd(std::string& name):
	StatisticsCalculator(name),
	mean_(0.0),
	M2_(0.0)
{
}

void MeanStd::addValue(double newVal)
{
	double delta = newVal - mean_;
	mean_ = mean_ + delta/((double)count_);
	M2_ = M2_ + delta*(newVal - mean_);
}

std::ostream& MeanStd::output(std::ostream& o) const
{
	o << "Mean and Std of " << name_ << ": ";
	double variance = M2_/((double)(count_ - 1));
	double std = std::sqrt(variance);
	o << "mean=" << mean_ << " std=" << std;
	return o;
}

//===========================================================

MinAndMax::~MinAndMax() {
}

MinAndMax::MinAndMax(std::string& name):
	StatisticsCalculator(name),
	smallest_(std::numeric_limits<double>::min()),
	largest_(std::numeric_limits<double>::max())
{
}

void MinAndMax::addValue(double newVal)
{
	if (newVal > largest_)
		largest_ = newVal;
	else if (newVal < smallest_)
		smallest_ = newVal;
}

std::ostream& MinAndMax::output(std::ostream& o) const
{
	o << "Min and Max Value of " << name_ << ": ";
	o << "minAndMax=(" << smallest_ << "," << largest_ << ")";
	return o;
}

//===========================================================

HistogramAndMedian::~HistogramAndMedian() {
}

HistogramAndMedian::HistogramAndMedian(std::string& name, unsigned int buckets):
	StatisticsCalculator(name),
	elements_(),
	buckets_(0)
{
	assert(buckets > 0);
}

void HistogramAndMedian::addValue(double newVal)
{
	elements_.push_back(newVal);
}

std::ostream& HistogramAndMedian::output(std::ostream& o) const
{
	o << "Median and Histogram of " << name_ << ": ";
	assert(elements_.size() > 0);
	assert(count_ == elements_.size());
	std::less<double> ls;
	std::sort(elements_.begin(), elements_.end(), ls);
	double median;
	unsigned int ind = count_ / 2; //floor division
	if ((count_ % 2) == 1)
	{
		median = elements_[ind];
	}
	else
	{
		median = (elements_[ind]+elements_[ind-1])/2.0;
	}
	o << "median=" << median << " histogram=";
	double smallest = elements_[0];
	double largest = elements_[elements_.size() - 1];
	double bucketSize = (largest - smallest)/((double) buckets_);
	assert(bucketSize > 0);
	unsigned int p = 0;
	unsigned int tot = 0;
	for(unsigned int i = 0; i < buckets_; ++i)
	{
		unsigned int contents = 0;
		for(; elements_[p] <= ((i+1)*bucketSize); ++p, contents++);
		o << "[" << (i*bucketSize) << "," << ((i+1)*bucketSize) << "]:" << contents;
		if(i < (buckets_-1)) o << ",";
		tot += contents;
	}
	assert(tot == elements_.size());
	return o;
}
