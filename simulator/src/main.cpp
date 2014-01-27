#include "main.hpp"
#include "ParameterList.hpp"
#include "BitcoinNetwork.hpp"
#include "TimeStepping.hpp"
#include "Sampler.hpp"
#include "Transaction.hpp"
#include "NetworkEvent.hpp"
#include "UniversityModel/UniversityModel.hpp"
#include "UniversityModel/UniversityModelTimeStepping.hpp"
#include "FriendModel/FriendModelTimeStepping.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <algorithm>


// global variables for main()
//============================================
bool simulationRunning = true;
struct sigaction osa;
int daylightSavingOffset;
//============================================


// Streams for logging
//=============================================
std::ofstream logFile;    // A verbose human-readable log
std::ofstream parameterLog; // Saves all the parameters that have been used by the simulation
//=============================================


// handler that is executed when sigint is recieved. i.e. ctrl-c
//=============================================
void catch_sigint(int sig_no)
{
	simulationRunning = false;
	// reinstall old signal handlers
	sigaction(SIGINT,&osa,NULL);
}
//=============================================


// The main simulation (interaction) loop with the network
//=============================================
void mainLoop(BitcoinNetwork *bitcoinNetwork_a, TimeStepping *timeStepping_a)
{
	std::cout << "Simulation started" << std::endl;
	std::cout << "'startTime' is: " << Sampler::tmToString(timeStepping_a->get_startTime()) << std::endl;

	tm stoppingTime;
	bool stoppingTimeDefined = false;
	if(ParameterList::parameterExists("endTime"))
	{
		std::string stoppingTimeStr = ParameterList::getParameterByName<std::string>("endTime");
		stoppingTime = Sampler::stringToTm(stoppingTimeStr);
		stoppingTimeDefined = true;
	}

	// simulation loop
	while(simulationRunning && timeStepping_a->isRunning())
	{
		if(!timeStepping_a->empty())
		{
			NetworkEvent *newEvent = timeStepping_a->getNextEvent();
			assert(newEvent != NULL);
			newEvent->execute(bitcoinNetwork_a);

			//write to log file
			logFile << *newEvent;

			if(stoppingTimeDefined)
			{
				if(Sampler::tm1BeforeTm2(stoppingTime, newEvent->getTimeStamp()))
				{
					timeStepping_a->abort();
					std::cout << "Reached 'endTime': " << Sampler::tmToString(stoppingTime)<< std::endl;
					std::cout << "Last event was " << newEvent->getName() << " with execution time " << Sampler::tmToString(newEvent->getTimeStamp()) << ": " << std::endl;
					std::cout << *newEvent;
					std::cout << "There are " << timeStepping_a->get_remainingCount() << " remaining events residing in the priority queue." << std::endl;
				}
			}
			delete newEvent;
		}
		else
		{
			std::cout << "No more events in queue. Stopping." << std::endl;
			timeStepping_a->abort();
		}
	}

	/*  //DEBUGGING
	std::vector<unsigned int> vec;
	vec.push_back(0); vec.push_back(1);
	unsigned int transactionID = bitcoinNetwork_a->issueTransaction(stoppingTime, 0, 1, vec, 0, 3.0);
	bitcoinNetwork_a->testUnionDBG(transactionID, stoppingTime);
	*/

	// sigint received, stopping.
	std::cout << std::endl << "Exiting mainLoop" << std::endl;
}
//=============================================


// program entry point
//=============================================
int main(int argc, char *argv[])
{
	//srand(1); // set the random generator seed to the same constant every time for reproducibility and debugging
	srand(time(NULL));

	tzset();

	daylightSavingOffset = (int) difftime(0, timezone);

	std::cout << "BitcoinSimulator" << std::endl;
	std::cout << "[hit control-c to quit]" << std::endl;
	if (argc < 2 || argc > 4) {
		std::cerr << "ERROR: Incorrect number of arguments" << std::endl;
		std::cerr << "The first argument should indicate the location of the configuration "
				  << "file, the second (optional) argument should indicate the name for the log files "
				  << " and the third (optional) argument specifies whether the truth-file is written to disk (0 or 1). Default is 0." << std::endl;
		std::cerr << "e.g. $ BitcoinSimulator CONFIGURATION_FILE [LOG_FILE_NAME] [WRITE_TRUTH_FILE]" << std::endl;
		return -1;
	}

	bool writeTruthFile = false;
	if(argc == 4)
	{
		if(std::string(argv[3]) == "1") writeTruthFile = true;
	}

	// Set up logging
	time_t now = time (NULL);
	tm* nowAsTm = localtime(&now);
	std::string defaultLogFile;
	Sampler::tmToFilename(defaultLogFile, *nowAsTm);

	std::string log_file = argc < 3 ? defaultLogFile : argv[2];
	logFile.open((log_file + "_log.txt").c_str());
	//compactLog.open((log_file + "_compact.txt").c_str());
	parameterLog.open((log_file + "_params.txt").c_str());

	// file names for output files
	std::string truthOutputFile = log_file + "_truth.txt";
	std::string truthMappingFile = log_file + "_mapping.txt";
	std::string bcOutputFile = log_file + "_network.txt";
	std::string tracesFile = log_file + "_traces.txt";
	std::string tracesFileFull = log_file + "_traces_full.txt";
	std::string featureOverviewFile = log_file + "_featOverview.txt";
	std::string statisticsFile = log_file + "_statistics.txt";

	// default parameter / configuration options
	options_t defaultParameters;
	// Default configuration values defined here...
	defaultParameters["maxClients"] = "20";
	defaultParameters["blocksNeeded"] = "6";
	defaultParameters["minerProb"] = "0.3";

	// Read parameters
	// test whether we can read the configuration file
	std::ifstream configurationInputFile(argv[1]);
	if (!configurationInputFile.is_open()) {
		std::cerr << "ERROR: Could not open file '" << argv[1] << "' now exiting" << std::endl;
		return -1;
	}


	ParameterList::setDefaults(defaultParameters);
	try
	{
		ParameterList::readParameters(configurationInputFile);
	}
	catch (rapidxml::parse_error &pE)
	{
		char* before = pE.where<char>();
		std::cerr << "ERROR while parsing Parameter configuration: " << std::endl;
		std::cerr << "What: " << pE.what() << std::endl;
		std::cerr << "Before: " << std::string(before,std::min<unsigned int>(80, std::strlen(before))) << "..." << std::endl;
		std::cerr << "Please make sure to check the XML syntax of the configuration file using a web browser or any other tool." << std::endl;
		std::cerr << "Also, do not make any successive or nested comment nodes in the XML file." << std::endl;
		return -4;
	}

	configurationInputFile.close();
	ParameterList::writeParametersToFile(parameterLog);
	parameterLog.close();


	////// Set up the main class instances /////////
	//===============================================
	BitcoinNetwork *bitcoinNetwork = new BitcoinNetwork();

	TimeStepping *timeStepping = NULL;
	// set the TimeStepping instance according to the model which has been chosen
	if(!ParameterList::parameterExists("model") ||
			ParameterList::getParameterByName<std::string>("model") == "FriendModel")
	{
		std::cout << "Using FriendModel" << std::endl;
		// initialize timeStepping
		timeStepping = new FriendModelTimeStepping();
	}
	else if(ParameterList::getParameterByName<std::string>("model") == "UniversityModel")
	{
		std::cout << "Using UniversityModel" << std::endl;
		std::ofstream universityModelParamFile((log_file + "_uniModel.txt").c_str());
		try
		{
			// write all the used parameters to a file
			universityModelParamFile << UniversityModelConfig::getInstance(argv[1]); // create the singleton and output everything to file
			universityModelParamFile.close();
		}
		catch (rapidxml::parse_error &pE)
		{
			char* before = pE.where<char>();
			std::cerr << "ERROR while parsing UniversityModel configuration: " << std::endl;
			std::cerr << "What: " << pE.what() << std::endl;
			std::cerr << "Before: " << std::string(before,std::min<unsigned int>(80, std::strlen(before))) << "..." << std::endl;
			return -3;
		}
		catch (exception& e)
		{
			std::cerr << "ERROR while reading UniversityModel configuration: " << e.what() << std::endl;
			return -3;
		}
		// initialize timeStepping
		timeStepping = new UniversityModelTimeStepping();
	}
	else
	{
		std::cerr << "ERROR: Unknown model: "
				  << ParameterList::getParameterByName<std::string>("model")
				  << std::endl;
		return -2;
	}
	//===============================================
	

	// install signal handler
	//======================================
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &catch_sigint;
	sigaction(SIGINT, &sa, &osa);
	//======================================


	// Run the main simulation loop
	//======================================
	mainLoop(bitcoinNetwork, timeStepping);
	//======================================


	// loop ended, sigint received, stopping
	//======================================
	std::cout << std::endl << "Writing memory to output file. Please be patient." << std::endl;
	// write everything to file
	bitcoinNetwork->outputNetworkLog(bcOutputFile);
	if(writeTruthFile) bitcoinNetwork->outputGroundTruth(truthOutputFile);

	if(ParameterList::parameterExists("model") &&
				ParameterList::getParameterByName<std::string>("model") == "UniversityModel")
	{
		const UniversityModelTimeStepping *univTimeStepping = dynamic_cast<UniversityModelTimeStepping*>(timeStepping);
		assert(univTimeStepping != NULL);
		bitcoinNetwork->outputGrundTruthMapping(truthMappingFile, univTimeStepping);
		bitcoinNetwork->outputTransactionTracesRandomClients(tracesFile, univTimeStepping, 3);
		bitcoinNetwork->outputTransactionTraces(tracesFile, true);
		bitcoinNetwork->outputTransactionTraces(tracesFileFull, false);
	}
	else
	{
		bitcoinNetwork->outputGrundTruthMapping(truthMappingFile);
	}
	bitcoinNetwork->outputFeatureOverview(featureOverviewFile);
	std::cout << std::endl << "Simulation ended" << std::endl << std::endl;
	//======================================


	// close log files
	//======================================
	logFile.close();
	//======================================


	//output statistics after simulation run
	//======================================
	std::ofstream statisticsFileStream(statisticsFile.c_str());
	std::cout << "Time stepping statistics:" << std::endl;
	std::cout << *timeStepping << std::endl;
	statisticsFileStream << *timeStepping << std::endl;
	std::cout << "Bitcoin network statistics:" << std::endl;
	std::cout << *bitcoinNetwork << std::endl;
	statisticsFileStream << *bitcoinNetwork << std::endl;
	statisticsFileStream.close();
	//======================================


	// clean up
	//======================================
	delete timeStepping;
	delete bitcoinNetwork;
	UniversityModelConfig::destroy();
	//======================================

	return 0;
}
