/*------------------------------------------------------------------------------
   Copyright © 2016-2017
   New Global Systems for Intelligent Transportation Management Corp.
   
   This file is part of SSAM.
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.
  
   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <sstream>
#include "SSAM.h" 

////////////////////////////////////////////////////////////////////////////////
// command-line argument names
////////////////////////////////////////////////////////////////////////////////
namespace argnames
{
	const std::string trjfiles = "trjfiles";
	const std::string csvfile = "csvfile";
	const std::string TTC  = "ttc";
	const std::string PET  = "pet"; 
	const std::string nthreads = "nthreads";
	const std::string dat = "-dat";
	const std::string h			= "-h";
	const std::string help		= "-help";
	const std::string p = "-print";
	const std::string puea = "-puea";
}

////////////////////////////////////////////////////////////////////////////////
// usage
//
// Provide a description of how to use the program.
////////////////////////////////////////////////////////////////////////////////
void usage()
{
	using namespace argnames;

	std::cout << std::endl << "usage:" << std::endl;
	std::cout << "SSAM trjfiles=\"c:\\full path to\\input1.trj\",\"c:\\full path to\\input2.trj\"... [options]" << std::endl << std::endl;
	std::cout << "Recognized Options:" << std::endl << std::endl;
	
	std::cout << csvfile << "=\"c:\\full path to\\output.csv\"" << std::endl;
	std::cout << dat << "\t\t- output trj file in text format" << std::endl; 
	std::cout << TTC << "=f\t\t- specify maximum TTC (range [0.0, 5.0], default = 1.5)" << std::endl;
	std::cout << PET << "=f\t\t- specify maximum PET (range [0.0, 10.0], default = 5.0)" << std::endl;

#ifdef _OPENMP_LOCAL
	std::cout << nthreads << "=n\t- specify the number of threads (default is the number of logic processors)" << std::endl;
#endif
	std::cout << p << "\t\t- output progress to screen" << std::endl;
	std::cout << std::endl << "options may be specified in any order." << std::endl;
	std::cout << std::endl;
}

int main(int argc, char* args[])
{
	try
	{
		using namespace argnames;
		
		SSAMFuncs::SSAM SSAMRunner;

#ifdef _OPENMP_LOCAL
        int nThreads = omp_get_num_procs() / 2;
		std::cout << "System Processors: " << nThreads << std::endl;
#endif

		std::string errMsg;
		std::string trjFiles;
		std::string csvFile;

		std::cout << "SSAM received " << argc << " argument(s)\n";
		for (int i = 1; i < argc; ++i)
		{
			std::string argument (args[i]);
			std::cout << "arg " << i << ": " << argument << std::endl;

			if(argument.substr(0, trjfiles.length()) == trjfiles)
			{
				trjFiles = argument.substr(trjfiles.size() + 1);
			}
			else if(argument.substr(0, TTC.length()) == TTC)
			{
				if( argument.length() <= TTC.length()+2)
				{
					std::cerr << "warning: ttc argument with no value ignored.\n";
					continue;
				} 
				float ttc = 1.5;
				try 
				{ 
					ttc = std::stof(argument.substr(TTC.length()+1));
					if(ttc < 0 || ttc > 5)
						throw SSAMException("value " + std::to_string(ttc) + " not in acceptable range, [0.0, 5.0]");
				} catch (const std::invalid_argument& e)
				{
					errMsg = "error: invalid decimal value, use ttc=1.5 (for example)\nextra error info: "; 
					errMsg += e.what();
					throw SSAMException(errMsg);
				} 
				SSAMRunner.SetMaxTTC(ttc);
			}
			else if(argument.substr(0, PET.length()) == PET)
			{
				if(argument.length() <= PET.length()+2)
				{
					std::cerr << "warning: pet argument with no value ignored.\n";
					continue;
				} 
				float pet = 5.0;
				try 
				{ 
					pet = std::stof(argument.substr(PET.length()+1));
					if(pet < 0 || pet > 10)
						throw SSAMException("value " + std::to_string(pet) + " not in acceptable range, [0.0, 10.0]");
				} catch (const std::invalid_argument& e)
				{
					errMsg = "error: invalid decimal value, use -pet5.0 (for example)\nextra error info: "; 
					errMsg += e.what();
					throw SSAMException(errMsg);
				} 
				SSAMRunner.SetMaxPET(pet);
			}//	-pet
			else if(argument.substr(0, csvfile.length()) == csvfile)
			{
				if (argument.substr(argument.length() - 4, 4) != ".csv")
				{
					std::cerr << "warning: csv file argument is not a csv file and ignored.\n";
					continue;
				}
				csvFile = argument.substr(csvfile.size() + 1);
				SSAMRunner.SetCSVFile(csvFile);
			}
#ifdef _OPENMP_LOCAL
			else if(argument.substr(0, nthreads.length()) == nthreads)
			{
				if( argument.length() <= nthreads.length()+2)
				{
					std::cerr << "warning: nthreads argument with no value ignored.\n";
					continue;
				} 
				
				try 
				{ 
					nThreads = std::stoi(argument.substr(nthreads.length()+1));
					if(nThreads < 0)
						throw SSAMException("value " + std::to_string(nThreads) + " must be a positive number");
				} catch (const std::invalid_argument& e)
				{
					errMsg = "error: invalid decimal value, use -nthreads=4 (for example)\nextra error info: "; 
					errMsg += e.what();
					throw SSAMException(errMsg);
				} 
			}
#endif
			else if(argument == p)
			{
				SSAMRunner.SetPrintProgess(true);
			}
			else if(argument.substr(0, dat.size()) == dat)
			{
				SSAMRunner.SetWriteDat(true);
			}
			else if(argument.substr(0, puea.size()) == puea)
			{
				SSAMRunner.SetIsCalcPUEA(true);
				
			}
			else if(argument.substr(0, help.size()) ==  help || argument.substr(0, h.size()) == h)
			{
				usage();
				return 1;
			}
			else
			{
				std::cerr << "warning: Unrecognized argument ignored." << std::endl;
			}
		}

		if (trjFiles.empty())
		{
			usage();
			return 1;
		}

		std::stringstream ss(trjFiles);
		for (std::string field; std::getline(ss, field, ','); SSAMRunner.AddTrjFile(field));
	
#ifdef _OPENMP_LOCAL
		std::cout << "nThreads: " << nThreads << std::endl;
		SSAMRunner.SetNThreads(nThreads);
#endif
		SSAMRunner.Analyze();
		std::cout << "Analysis complete.\n";

		if(!csvFile.empty())
		{
			SSAMRunner.ExportResults();
		}
		std::cout << "Total analysis time: " << SSAMRunner.GetAnalysisTime() << " ms." << std::endl;		
	} catch (std::runtime_error& e)
	{
		std::string errMsg("Analysis error.  Aborting analysis.\nExtra info:\n");
		errMsg += e.what();
		std::cout << errMsg;
	}

	return 0;
}
