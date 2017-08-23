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
#pragma once
#ifndef SSAM_H
#define SSAM_H
#include <string>
#include <list>
#include <map>
#include <vector>
#include <ctime>
#include <fstream>
#include "Vehicle.h"
#include "ZoneGrid.h"
#include "Event.h"
#include "Conflict.h"
#include "MotionPrediction.h"
#include "Summary.h"
#ifdef _OPENMP_LOCAL
#include <omp.h>
#endif

#ifdef SSAMDLL_EXPORTS
#define SSAMFUNCSDLL_API __declspec(dllexport) 
#else
#define SSAMFUNCSDLL_API __declspec(dllimport) 
#endif


/** SSAMFuncs namespace encloses all SSAM DLL classes
*/
namespace SSAMFuncs
{
	/** Dimensions specifies the extent of the rectangular region of 
	  * the vehicle observation area in terms of x-y coordinates.
	*/
	class Dimensions
	{
	public:
		const static int ENGLISH_UNITS = 0;/*!< feet, feet/sec, feet/sec*sec */
		const static int METRIC_UNITS = 1; /*!< meters, meters/sec, meters/sec*sec */

		Dimensions()
			: m_MinX (0)
			, m_MaxX (0)
			, m_MinY (0)
			, m_MaxY (0)
			, m_Units (0)
			, m_Scale (1.0)
		{}

		~Dimensions(){}

		Dimensions(const Dimensions& rhs)
		{
			CopyValues(rhs);
		}

		Dimensions& operator=(const Dimensions& rhs)
		{
			if (this != &rhs)
				CopyValues(rhs);
			return *this;
		}

		/**Validate current dimension parameters.
		*/
		void Validate();

		/** Print current dimension parameters.
		* @param output the output stream
		*/
		void Print(std::ostream& output);

		//	Get()/Set() methods
		int GetMinX() { return m_MinX; }
		int GetMaxX() { return m_MaxX; }
		int GetMinY() { return m_MinY; }
		int GetMaxY() { return m_MaxY; }
		int GetUnits(){ return m_Units; }
		float GetScale() { return m_Scale; }
		void SetMinX(int i) { m_MinX = i;	}
		void SetMaxX(int i) { m_MaxX = i; }
		void SetMinY(int i) { m_MinY = i; }
		void SetMaxY(int i) { m_MaxY = i; }
		void SetUnits(int i){ m_Units = i; }
		void SetScale(float f) { m_Scale = f; }

	private:
		int m_MinX; /*!< Left edge of the observation area.*/
		int m_MaxX; /*!< Right edge of the observation area.*/
		int m_MinY; /*!< Bottom edge of the observation area.*/
		int m_MaxY; /*!< Top edge of the observation area.*/
		int m_Units;  /*!< Engligh or Metric units*/
		float m_Scale; /*!< Distance per unit of X or Y*/

		void CopyValues(const Dimensions& rhs)
		{
			m_MinX = rhs.m_MinX;
			m_MaxX = rhs.m_MaxX;
			m_MinY = rhs.m_MinY;
			m_MaxY = rhs.m_MaxY;
			m_Units = rhs.m_Units;
			m_Scale = rhs.m_Scale;
		}
	};

	/** Smart pointer type to Dimensions class.
	*/
	typedef std::shared_ptr<Dimensions> SP_Dimensions;

	/** InputFormat specifies the byte format and file format of the trj source
	*/
	struct InputFormat
	{
		InputFormat()
			: m_Endian('L')
			, m_Version(3.0)
			, m_ZOption(0)
		{}
		char m_Endian; /*!< Byte endianness: 'L' - little endian; 'B' - big endian */
		float m_Version; /*!< TRJ file format version */
		int m_ZOption; /*!< Flag to indicate whether to use z-value */

		/** Print current format parameters.
		* @param output the output stream
		*/
		void Print(std::ostream& output);
	};


	/** TrjRecord contains TRJ records from TRJ file or TRJ data set
	*/
	class TrjRecord
	{
	public:

		TrjRecord() 
			: m_RecordType(INVALID)
			, m_TimeStep (-1)
			, m_pDimensions (NULL)
			, m_pVehicle(NULL)
		{}
		~TrjRecord() {}

		/** Copy contructor from another TrjRecord object
		* @param rhs Const reference to a TrjRecord object from which the values are copied
		*/
		TrjRecord(const TrjRecord& rhs)
		{
			CopyValues(rhs);
		}

		/** Overloaded assignment operator
		* @param rhs Const reference to a TrjRecord object from which the values are copied
		*/
		TrjRecord& operator=(const TrjRecord& rhs)
		{
			if (this != &rhs)
				CopyValues(rhs);
			return *this;
		}

		/** RECORD_TYPE enumerates the record types in TRJ input
		*/
		enum RECORD_TYPE
		{
			INVALID = -1, /*!< Invalid record type in TRJ input*/
			FORMAT, /*!< Format record in TRJ input*/
			DIMENSIONS, /*!< Dimenstions record in TRJ input*/
			TIMESTEP, /*!< Time step record in TRJ input*/
			VEHICLE /*!< Vehicle record in TRJ input*/
		};
	
		//	Set()/Get()
		void SetRecordType(RECORD_TYPE typeTag) { m_RecordType = typeTag; }
		void SetFormat(const InputFormat& f) { m_Format = f; }
		void SetDimensions(SP_Dimensions d) { m_pDimensions = d; }
		void SetTimestep(float t) { m_TimeStep = t; }
		void SetVehicle(SP_Vehicle v) { m_pVehicle = v; }
		int GetRecordType() const { return (int) m_RecordType; }
		const InputFormat& GetFormat() const { return m_Format; }
		SP_Dimensions GetDimensions() const { return m_pDimensions; }
		float GetTimestep() const { return m_TimeStep; }
		SP_Vehicle GetVehicle() const { return m_pVehicle; }

	private:
		RECORD_TYPE m_RecordType; /*!< TRJ record type*/
		InputFormat m_Format; /*!< Byte format and file format */
		SP_Dimensions m_pDimensions; /*!< Smart pointer to dimensions of analysis region */
		float m_TimeStep; /*!< Seconds since the start of the simulation */
		SP_Vehicle m_pVehicle; /*!< Smart pointer to data of a vehicle in one time step */
	
		/** Copy values from another TrjRecord object
		* @param rhs Const reference to a TrjRecord object from which the values are copied
		*/
		void CopyValues(const TrjRecord& rhs)
		{
			m_RecordType = rhs.m_RecordType;
			m_Format = rhs.m_Format;
			m_TimeStep = rhs.m_TimeStep;
			if (rhs.m_pDimensions != NULL) 
				m_pDimensions = std::make_shared<Dimensions>(*(rhs.m_pDimensions));
			if (rhs.m_pVehicle != NULL)
				m_pVehicle = std::make_shared<Vehicle>(*(rhs.m_pVehicle));
		}
	};

	/** Smart pointer type to TrjRecord class.
	*/
	typedef std::shared_ptr<TrjRecord> SP_TrjRecord;

	/** TimeStepData manages vehicle data in one time step
	*/
	class TimeStepData
	{
	public:
		TimeStepData(): m_TimeStep(-1) {}

		void  SetTimestep(float t)	{ m_TimeStep = t; }
		float GetTimestep() const { return m_TimeStep; }
		std::map<int, SP_Vehicle>* GetVehicleMap() {return &m_VehicleMap;}
		std::vector<SP_Vehicle>* GetVehicleVec() {return &m_VehicleVec;}

		/** Add one vehicle data of current time step
		  * @param vID ID of the vehicle to add
		  * @param v smart pointer to the vehicle data
		*/
		void AddVehicle(int vID, SP_Vehicle v)
		{
			m_MapReturn = m_VehicleMap.insert(std::pair<int, SP_Vehicle>(vID, v));
			if (m_MapReturn.second)
				m_VehicleVec.push_back(v);
		}
	private:
		float m_TimeStep; /*!< Seconds since the start of the simulation */
		std::map<int, SP_Vehicle>  m_VehicleMap; /*!< A map container stores vehicle smart pointers using vehicle IDs as keys*/
		std::vector<SP_Vehicle> m_VehicleVec; /*!< A vector container stores vehicle smart pointers*/
		std::pair<std::map<int, SP_Vehicle>::iterator, bool> m_MapReturn; /*!< A pair stores the return value of inserting new vehicle into map */
	};
	/** Smart pointer type to TimeStepData class.
	*/
	typedef std::shared_ptr<TimeStepData> SP_TimeStepData;

	/** Define a pair of project name and a list of TRJ records as one TRJ Input source
     */
	typedef std::pair<std::string, std::list<TrjRecord>* > TrjDataList;  

	typedef std::pair<int, int> VehiclePair;

	/** SSAM reads TRJ input, runs SSAM simulation, and maintains conflict results
     */
	class SSAM
	{
	public:
		SSAMFUNCSDLL_API SSAM();
		SSAMFUNCSDLL_API ~SSAM();

		//	default values
		const static float DEFAULT_TTC; /*!< default maximum TTC */
		const static float DEFAULT_PET; /*!< default maximum PET */
		const static int DEFAULT_REARENDANGLE=30; /*!< default rear end angle threshold */
		const static int DEFAULT_CROSSINGANGLE=80; /*!< default crossing angle threshold */

		int m_Boundary[4]; /*!< Boundary coordinates of the observation area: 0: minX; 1: minY; 2: maxX; 3: maxY*/

		/** Initialize SSAM analysis parameters.
		 */
		SSAMFUNCSDLL_API void Initialize();

		/** Wrap up SSAM analysis: calculate summaries and analysis time.
		 */
		SSAMFUNCSDLL_API void Terminate();

		/** Finish current SSAM run.
		 */
		SSAMFUNCSDLL_API void CloseRun();

		/** Run SSAM analysis.
		 */
		SSAMFUNCSDLL_API void Analyze();

		/** Export conflict points and summary to the csv file.
		 */
		SSAMFUNCSDLL_API void ExportResults();

		/** Set the name of TRJ data source.
		 * @param s name of TRJ data source
		 */
		SSAMFUNCSDLL_API void SetTrjSrcName(const std::string& s);
		
		/** Set the TRJ format.
		 * @param f TRJ format
		 */
		SSAMFUNCSDLL_API void SetFormat(const InputFormat& f);

		/** Set the dimensions of analysis area.
		 * @param pDims smart pointer to the dimensions of analysis area
		 */
		SSAMFUNCSDLL_API void SetDimensions(SP_Dimensions pDims);

		/** Set time step value and run one step of SSAM analysis.
		 * @param t time step
		 */
		SSAMFUNCSDLL_API void SetTimeStep(float t);

		/** Add a vehicle to the current time step.
		 * @param pVeh smart pointer to a vehicle as input value
		 */
		SSAMFUNCSDLL_API void SetVehicle(SP_Vehicle pVeh);

		/** Calculate summary on safety measures of all conflicts
		 */
		SSAMFUNCSDLL_API void CalcSummaries();
		
		//	Get()/Set() methods
		void SetMaxTTC(float maxTTC) { m_MaxTTC = maxTTC; }
		void SetMaxPET(float max) { m_MaxPET = max; }
		void SetRearEndAngle(int rna) {m_RearEndAngleThreshold = rna;}
		void SetCrossingAngle(int ca) {m_CrossingAngleThreshold = ca;}
		void SetCSVFile(const std::string& s) { m_CsvFileName = s; }
		void SetNThreads(int n) {m_NThreads = n;}
		void SetIsCalcPUEA(bool isCalcPUEA) {m_IsCalcPUEA = isCalcPUEA;}
		void SetPrintProgess(bool b) {m_IsPrintProgress = b;}
		void SetWriteDat(bool b) { m_IsWriteDat = b;}
		void AddTrjFile(const std::string& s) { m_TrjFileNames.push_back(s); }
		void AddTrjDataList(const std::string& s, std::list<TrjRecord>* trjDataList) 
		{
			m_TrjDataLists.push_back(TrjDataList(s, trjDataList));
		}

		float GetMaxTTC() { return m_MaxTTC; }
		float GetMaxPET() { return m_MaxPET; }
		bool GetIsCalcPUEA() { return m_IsCalcPUEA;}
		int GetRearEndAngle() { return m_RearEndAngleThreshold;}
		int GetCrossingAngle() { return m_CrossingAngleThreshold;}
		int GetUnits(){ return m_Units; }
		std::string& GetCSVFile() { return m_CsvFileName; }
		std::list<SP_Conflict>& GetConflictList() {return m_ConflictList;}
		std::list<SP_Summary>& GetSummaries() {return m_Summaries;}
		SP_Summary GetSummary() const {return m_pSummary;}
		int GetAnalysisTime() const { return m_AnalysisTime; }
		const std::list<std::string>& GetTrjFileNames() const {return m_TrjFileNames;}
	protected:
		float m_MaxTTC; /*!< Max TTC threshold */
		float m_MaxPET; /*!< Max PET threshold */
		int m_RearEndAngleThreshold;  /*!< Rear-End Angle Threshold */
		int m_CrossingAngleThreshold; /*!< Crossing Angle Threshold */
		int m_AnalysisTime; /*!< Total time for analysis */
		int m_StartTime; /*!< Start time for analysis */
		int m_EndTime; /*!< End time for analysis */
		std::list<std::string> m_TrjFileNames; /*!< A list of TRJ files to analyze */
		std::map<VehiclePair, SP_Event> m_EventList; /*!< List of detected conflict events */
		std::list<SP_Conflict> m_ConflictList; /*!< List of smart pointers to conflict points */
		/*!< A map container stores conflict smart pointers using TRJ source names as keys*/
		std::map<std::string, std::list<SP_Conflict> > m_FileToConflictsMap; 
		SP_Summary m_pSummary; /*!< Smart pointer to the summary over all TRJ inputs */
		std::list<SP_Summary> m_Summaries; /*!< A list of summary smart pointers, each for one TRJ source */
		bool m_IsCalcPUEA; /*!< Flag to indicate whether to calculate P(UEA), mTTC, mPET */
		std::string m_CsvFileName; /*!< A csv file to output analysis results */
	private:
		std::string m_TrjSrcName; /*!< Name of TRJ data source */
		std::ifstream m_TrjFile; /*!< A TRJ file to analyze */
		std::list<TrjDataList> m_TrjDataLists; /*!< A list of TRJ data lists to analyze */
		std::list<TrjRecord> m_TrjDataList; /*!< A TRJ data lists to analyze */
		bool m_IsWriteDat; /*!< Flag indicates whether to write input TRJ records to a csv file */
		std::ofstream m_DatFile; /*!< A csv file to write input TRJ records */
		int m_NThreads;  /*!< Number of threads to use */
		SP_ZoneGrid m_pZoneGrid;  /*!< Smart pointer to the zone grid object */
		float m_Units; /*!< Engligh or Metric units */
		float m_ZoneSize; /*!< Size of one zone */
		float m_ReadTimeStep; /*!< Current time step read from TRJ source*/
		float m_AnalysisTimeStep; /*!< Current time step to run SSAM analysis*/
		InputFormat m_Format; /*!< TRJ format */
		SP_Dimensions m_pDimensions; /*!< Smart pointer to the dimensions of analysis area */
		std::list<SP_TimeStepData> m_StepDataList; /*!< Current list of time step data to run SSAM analysis */
		SP_TimeStepData m_pCurStep; /*!< Current time step data to run SSAM analysis */
		bool m_IsFirstTimeStep; /*!< Flag to indicate whether the current read time step is the first time step */
		int m_NSteps; /*!< Number of steps per second for motion prediction analysis */
		InitEventParams m_InitEventParams; /*!< Parameters to initialize a conflict event */
		bool m_IsPrintProgress; /*!< Flag to indicate whether to print the SSAM analysis progress in time step*/

		/** Run SSAM analysis on a list of TRJ files.
		 */
		void Analyze(const std::list<std::string>& trjFileNames);

		/** Run SSAM analysis on a list of TRJ data lists.
		 */
		void Analyze(const std::list<TrjDataList>& trjDataLists);

		/** Read the size of TRJ source: bytes for TRJ file, and records for TRJ data list.
		 */
		void ReadTrjInputSize();

		/** Read TRJ format from TRJ file.
		 */
		void ReadInputFormat();

		/** Read dimensions of analysis area from TRJ file.
		 */
		void ReadDimensions();

		/** Use dimensions to set grid zone, boundary coordinates and motion prediction parameters.
		 */
		void ApplyDimensions();

		/** Read a vehicle from TRJ file.
		 * @param pVeh smart pointer to a vehicle as output value
		 */
		void ReadVehicle(SP_Vehicle pVeh);
		
		/** Validate current TRJ format.
		 */
		void ValidateInputFormat();

		/** Validate a vehicle.
		 * @param pVeh smart pointer to a vehicle to validate
		 * @return true if intput is valid vehicle data.
		 */
		bool ValidateVehicle(SP_Vehicle pVeh);

		/** Print a read time step to csv file.
		 */
		void PrintTimeStep(float t);

		/** Throw a file reading exception.
		 * @param e a SSAM execption
		 * @param errMsg basic error messages
		 */
		void ThrowFileReadingException(SSAMException &e, std::string& errMsg);

		/** Run one step of SSAM analysis.
		 */
		void AnalyzeOneStep();

		/** Detect conflicts in the current step of vehicles.
		 * @param pZoneGrid smart pointer to the zone grid
		 * @param eventList a map container stores conflict events using vehicle pairs as keys 
		 */
		void DetectConflicts(SP_ZoneGrid pZoneGrid, std::map<VehiclePair, SP_Event>& eventList);

		/** Analyze conflicts detected in the current step.
		 * @param trjSrcName name of TRJ source
		 * @param eventList a map container stores smart pointers to conflict events using vehicle pairs as keys 
		 */
		void AnalyzEvents(const std::string& trjSrcName, std::map<VehiclePair, SP_Event>& eventList);
		
		/** Create a conflict record.
		 * @param e smart pointer to the conflict event for creating conflict record
		 * @param trjSrcName name of TRJ source
		 */
		void CreateConflict(SP_Event e, const std::string& trjSrcName)
		{
			SP_Conflict c  = std::make_shared<Conflict>(e, trjSrcName);
			m_ConflictList.push_back(c); 
			m_FileToConflictsMap[trjSrcName].push_back(c);
		}
		
		//////////////////////////////////////////
		// parse values from a binary file
		const static int INT_SIZE = sizeof(int);
		char m_BufferInt[INT_SIZE];
		const static int FLOAT_SIZE = sizeof(float);
		char m_BufferFloat[FLOAT_SIZE];

		/** Read a byte from binaray file.
		 * @param in the input stream
		 * @return byte value
		 */
		char ReadByte(std::ifstream& in)
		{
			char x;
			if (!in.get(x))
				throw SSAMException("file reading failed");
			return x;
		}

		/** Read a float number from binaray file.
		 * @param in the input stream
		 * @return float value
		 */
		float ReadFloat(std::ifstream& in)
		{
			float x = 0.0;
			if (!in.read( m_BufferFloat, FLOAT_SIZE))
				throw SSAMException("file reading failed");
			if (m_Format.m_Endian == 'B')
			{
				ConvertEndianness(m_BufferFloat, FLOAT_SIZE);
			}
			memcpy(&x, &m_BufferFloat, FLOAT_SIZE);
			return x;
		}

		/** Read an integer number from binaray file.
		 * @param in the input stream
		 * @return int value
		 */
		int ReadInt(std::ifstream& in)
		{
			int x = 0;
			if (!in.read( m_BufferInt, INT_SIZE))
				throw SSAMException("file reading failed");
			if (m_Format.m_Endian == 'B')
			{
				ConvertEndianness(m_BufferInt, INT_SIZE);
			}
			memcpy(&x, &m_BufferInt, INT_SIZE);
			return x;
		}

		/** Convert the endianness from Big Endian to Little Endian.
		 * @param buffer a char array stores the value
		 * @param size size of the buffer
		 */
		void ConvertEndianness(char* buffer, int size)
		{
			for (int i = 0; i < size/2; ++i)
			{
				char tmp = buffer[i];
				buffer[i] = buffer[size - i];
				buffer[size - i] = tmp;
			}
		}
	};
	/** Smart pointer type to SSAM class.
      */
	typedef std::shared_ptr<SSAM> SP_SSAM;
}

#endif
