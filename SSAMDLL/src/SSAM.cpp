#include "stdafx.h"
#include "SSAM.h"
#include<iostream>
#include<set>
#include<cfloat>
#include<cmath>
#include <iomanip>

using namespace std;
using namespace SSAMFuncs;

const float SSAM::DEFAULT_TTC = 1.5; 
const float SSAM::DEFAULT_PET = 5.0; 

SSAM::SSAM()
	: m_MaxTTC(DEFAULT_TTC)
	, m_MaxPET(DEFAULT_PET)
	, m_RearEndAngleThreshold(DEFAULT_REARENDANGLE)
	, m_CrossingAngleThreshold(DEFAULT_CROSSINGANGLE)
	, m_NThreads (1)
	, m_IsWriteDat(false)
	, m_IsCalcPUEA(false)
	, m_Units(0)
	, m_ZoneSize(50.0)
	, m_AnalysisTime(0)
	, m_StartTime(0)
	, m_EndTime(0)
	, m_IsFirstTimeStep(true)
	, m_NSteps(10)
	, m_IsPrintProgress (false)
	, m_pDimensions (NULL)
	, m_pCurStep (NULL)
{
	m_Boundary[0] = INT_MAX;
	m_Boundary[1] = INT_MAX;
	m_Boundary[2] = INT_MIN;
	m_Boundary[3] = INT_MIN;
}

SSAM::~SSAM()
{
}

void SSAM::Initialize()
{
	m_InitEventParams.m_V1 = NULL;
	m_InitEventParams.m_V2 = NULL;
	m_InitEventParams.m_MaxTTC = m_MaxTTC;
	m_InitEventParams.m_MaxPET = m_MaxPET;
	m_InitEventParams.m_RearEndAngleThreshold = m_RearEndAngleThreshold;
	m_InitEventParams.m_CrossingAngleThreshold = m_CrossingAngleThreshold;
	m_InitEventParams.m_IsCalcPUEA = m_IsCalcPUEA;
	m_InitEventParams.m_NSteps = m_NSteps;
	m_InitEventParams.m_CollisionThreshold = 0;
	m_InitEventParams.m_pNormalAdaption = NULL;
	m_InitEventParams.m_pEvasiveAction = NULL;
	m_StartTime = std::clock();
}

void SSAM::Terminate()
{
	CalcSummaries();
	m_EndTime = std::clock();	
	m_AnalysisTime = m_EndTime - m_StartTime;
}

void SSAM::Analyze()
{
	if (!m_TrjFileNames.empty())
		Analyze(m_TrjFileNames);

	if (!m_TrjDataLists.empty())
		Analyze(m_TrjDataLists);
}

void SSAM::Analyze(const std::list<std::string>& trjFileNames)
{	
	Initialize();
	for (std::list<std::string>::const_iterator it = trjFileNames.begin();
		it != trjFileNames.end(); ++it)
	{
		if (it->empty())
			continue;

		m_TrjSrcName = *it;
		if (m_TrjSrcName.substr(m_TrjSrcName.length() - 4, 4) != ".trj")
			throw SSAMException("File \"" + m_TrjSrcName + "\" is not a .trj file."); 

		m_TrjFile.open(m_TrjSrcName, std::ifstream::binary);
		if(!m_TrjFile.good()) 
			throw SSAMException("File \"" + m_TrjSrcName + "\" does not exist.");

		if (m_IsWriteDat)
			m_DatFile.open(m_TrjSrcName.substr(0, m_TrjSrcName.length() - 4)+"_dat.csv");

		ReadTrjInputSize();

		int recordType = ReadByte(m_TrjFile);
		if(recordType != TrjRecord::FORMAT)
			throw SSAMException("Read error, expected FORMAT data not found.");
		ReadInputFormat();
		ValidateInputFormat();

		recordType = ReadByte(m_TrjFile);
		if(recordType != TrjRecord::DIMENSIONS)
			throw SSAMException("Read error, expected DIMENSIONS data not found.");
		ReadDimensions(); 
		m_pDimensions->Validate();
		if (m_IsWriteDat)
			m_pDimensions->Print(m_DatFile);
		ApplyDimensions();
		
		m_ReadTimeStep = -1;
		m_AnalysisTimeStep = -1;
		m_IsFirstTimeStep = true;
		m_StepDataList.clear();
		m_EventList.clear();
		m_pCurStep = NULL;
		
		char typeChar;
		while (m_TrjFile.get(typeChar))
		{
			int recordType = typeChar;
			if (recordType == TrjRecord::TIMESTEP)
			{
				float t = ReadFloat(m_TrjFile);
				SetTimeStep(t);
			} else if (recordType == TrjRecord::VEHICLE)
			{
				SP_Vehicle v = std::make_shared<Vehicle>();
				v->SetTimeStep(m_pCurStep->GetTimestep());
				ReadVehicle(v); 
				if (m_IsWriteDat)
					v->Print(m_DatFile, m_Format.m_Version);
				if ( ValidateVehicle(v)) 
					m_pCurStep->AddVehicle(v->GetVehicleID(), v);
			} else
			{
				throw SSAMException("Invalid trajectory record type (outside of header): " + std::to_string(recordType));
			}
		}
		CloseRun();
		m_TrjFile.close();
	}
	Terminate();
}

void SSAM::Analyze(const std::list<TrjDataList>& trjDataLists)
{
	Initialize();
	for (std::list<TrjDataList>::const_iterator it = trjDataLists.begin();
		it != trjDataLists.end(); ++it)
	{
		if (it->second->empty())
			continue;

		SetTrjSrcName(it->first);
		m_TrjDataList = *(it->second);
		ReadTrjInputSize();

		std::list<TrjRecord>::iterator itRec = m_TrjDataList.begin();
		if(itRec->GetRecordType() != TrjRecord::FORMAT)
			throw SSAMException("Expected FORMAT data not found.");
		SetFormat(itRec->GetFormat());
		
		itRec++;
		if(itRec == m_TrjDataList.end() 
			|| itRec->GetRecordType() != TrjRecord::DIMENSIONS)
			throw SSAMException("Expected DIMENSIONS data not found.");
		SetDimensions(itRec->GetDimensions());
		
		itRec++;
		while (itRec != m_TrjDataList.end()) 
		{
			int recordType = itRec->GetRecordType();
			if (recordType == TrjRecord::TIMESTEP)
			{
				SetTimeStep(itRec->GetTimestep());
			} else if (recordType == TrjRecord::VEHICLE)
			{
				SetVehicle(itRec->GetVehicle());
			} else
			{
				throw SSAMException("Invalid trajectory record type (outside of header): " + std::to_string(recordType));
			}
			itRec++;
		}
		CloseRun();
	}
	Terminate();
}

void SSAM::SetTrjSrcName(const std::string& s) 
{ 
	m_TrjSrcName = s;
	
	// if convert to csv file, open the csv file
	if (m_IsWriteDat)
		m_DatFile.open(m_TrjSrcName+"_dat.csv");
}

void SSAM::SetFormat(const InputFormat& f)
{
	m_Format = f;
	ValidateInputFormat();
}

void SSAM::SetDimensions(SP_Dimensions pDims)
{
	m_pDimensions = std::make_shared<Dimensions>(*pDims);
	m_pDimensions->Validate();
	if (m_IsWriteDat)
		m_pDimensions->Print(m_DatFile);
	ApplyDimensions();
}

void SSAM::SetTimeStep(float t)
{
	if (m_IsFirstTimeStep)
	{
		m_ReadTimeStep = -1;
		m_AnalysisTimeStep = -1;
		m_StepDataList.clear();
		m_EventList.clear();
		m_pCurStep = NULL;
	}

	if (m_pCurStep)
	{
		AnalyzeOneStep();
	}
				
	m_pCurStep = std::make_shared<TimeStepData>();
	m_pCurStep->SetTimestep(t); // 
	if (m_IsPrintProgress && fmod(t, 100) == 0)
	{
		std::cout << "Time: " << t <<std::endl;
	}

	if (m_IsWriteDat)
		PrintTimeStep(t);

	if (m_IsFirstTimeStep)
	{
		m_IsFirstTimeStep = false;
	}
}

void SSAM::SetVehicle(SP_Vehicle pVeh)
{
	SP_Vehicle v = std::make_shared<Vehicle>(*pVeh);
	if (m_IsWriteDat)
		v->Print(m_DatFile, m_Format.m_Version);
	if ( ValidateVehicle(v)) 
		m_pCurStep->AddVehicle(v->GetVehicleID(), v);
}

void SSAM::CloseRun()
{
	// check whether to process the last set of steps and proceed if enough data
	if (m_pCurStep)
	{
		AnalyzeOneStep();
	}
		 
	if (m_IsWriteDat)
		m_DatFile.close();

	m_IsFirstTimeStep = true;
}

void SSAM::ApplyDimensions()
{
	m_Units = m_pDimensions->GetUnits();
	if(m_Units == Dimensions::ENGLISH_UNITS)
		m_ZoneSize = (int)(50.0/m_pDimensions->GetScale());
	else if(m_Units == Dimensions::METRIC_UNITS)
		m_ZoneSize = (int)(15.0/m_pDimensions->GetScale());
		
	if (m_pZoneGrid == NULL)
	{
		m_pZoneGrid = std::make_shared<ZoneGrid>(m_pDimensions->GetMinX(), 
			m_pDimensions->GetMinY(), 
			m_pDimensions->GetMaxX(),
			m_pDimensions->GetMaxY(),
			m_ZoneSize);
	} else
	{
		m_pZoneGrid->ResetGrid(m_pDimensions->GetMinX(), 
			m_pDimensions->GetMinY(), 
			m_pDimensions->GetMaxX(),
			m_pDimensions->GetMaxY(),
			m_ZoneSize);
	}

	if (m_pDimensions->GetMinX() < m_Boundary[0])
		m_Boundary[0] = m_pDimensions->GetMinX();

	if (m_pDimensions->GetMinY() < m_Boundary[1])
		m_Boundary[1] = m_pDimensions->GetMinY();

	if (m_pDimensions->GetMaxX() > m_Boundary[2])
		m_Boundary[2] = m_pDimensions->GetMaxX();

	if (m_pDimensions->GetMaxY() > m_Boundary[3])
		m_Boundary[3] = m_pDimensions->GetMaxY();

	if (m_IsCalcPUEA) 
	{
		int nTrajs = 100;
		int nRate = m_NSteps * m_NSteps;
		float ttcSteerMax = 0.2 / float(m_NSteps);
		float pueaSteerMax = 0.5 / float(m_NSteps);
		float maxSpeed = 75.0 * 5280.0 / 3600.0 / (float(m_NSteps));
		float collisionThreshold = 6.0;
		float ttcAccelMax = 6.56 / float(nRate);
		float pueaAccelMin = -29.86 / float(nRate);
		float pueaAccelMax = 14.11 / float(nRate);
		if (m_pDimensions->GetUnits()  == Dimensions::METRIC_UNITS)
		{
			maxSpeed = 90.0 / 3.6/ (float(m_NSteps));
			collisionThreshold = 1.8;
			ttcAccelMax = 2.0 / float(nRate);
			pueaAccelMin = -9.1 / float(nRate);
			pueaAccelMax = 4.3 / float(nRate);
		}
		using namespace MotPredNameSpace;
			
		SP_NormalAdaption normalAdaption = std::make_shared<NormalAdaption>(maxSpeed, 
			nTrajs, 
			ttcAccelMax, 
			ttcSteerMax);
			
		SP_EvasiveAction evasiveAction =  std::make_shared<EvasiveAction>(maxSpeed, 
			nTrajs, 
			pueaAccelMin,
			pueaAccelMax, 
			pueaSteerMax);

		m_InitEventParams.m_CollisionThreshold = collisionThreshold;
		m_InitEventParams.m_pNormalAdaption = normalAdaption;
		m_InitEventParams.m_pEvasiveAction = evasiveAction;
	}	
}

void SSAM::AnalyzeOneStep()
{
	m_ReadTimeStep = m_pCurStep->GetTimestep();

	//	Link vehicles from this step to previous step	
	if(!m_StepDataList.empty())
	{
		std::map<int, SP_Vehicle> *prevStep = m_StepDataList.back()->GetVehicleMap();
		if(prevStep == NULL)
			throw SSAMException("Processing failed to link vehicle from previous timestep to current timestep.");
				
		std::map<int, SP_Vehicle>::iterator iVeh = prevStep->begin();
		for(; iVeh != prevStep->end(); ++iVeh)
		{
			SP_Vehicle vPrev = iVeh->second;
			if(vPrev == NULL)
				throw SSAMException("Step analysis failed, linking invalid vehicle from previous timestep.");
					
			int preID = vPrev->GetVehicleID();
			SP_Vehicle vNext = NULL;
			std::map<int, SP_Vehicle> *curStepVeh = m_pCurStep->GetVehicleMap();
			if (curStepVeh->find(preID) != curStepVeh->end())
				vNext = curStepVeh->at(preID); 

			if(vNext != NULL)
				vPrev->SetNext(vNext);
		}
	}
	else
	{
		m_AnalysisTimeStep = m_ReadTimeStep - 1;
	}
	m_StepDataList.push_back(m_pCurStep);

	// check whether to process the set of steps and proceed if enough data
	// and then remove the first step in the current set
	while (m_ReadTimeStep - m_AnalysisTimeStep >= m_MaxPET) 
	{
		m_AnalysisTimeStep = m_StepDataList.front()->GetTimestep();
		DetectConflicts(m_pZoneGrid, m_EventList);
		//	Discard the first timestep (if any)
		if(!m_StepDataList.empty())
			m_StepDataList.pop_front();
	}
}

void SSAM::DetectConflicts(SP_ZoneGrid pZoneGrid, std::map<VehiclePair, SP_Event>& eventList)
{
	pZoneGrid->ClearGrid();
		
	SP_TimeStepData step = m_StepDataList.front();
	std::map<int, SP_Vehicle>* stepVehMap = step->GetVehicleMap();
	std::vector<SP_Vehicle>* stepVehVec = step->GetVehicleVec();
		
#ifdef _OPENMP_LOCAL
	omp_set_num_threads(m_NThreads);

	#pragma omp parallel for shared(pZoneGrid, eventList)
#endif
	for(int iv = 0; iv < stepVehVec->size(); ++iv)
	{
		SP_Vehicle v = stepVehVec->at(iv);
		SP_Vehicle vProj = v->CalcProjection(m_MaxTTC,m_MaxPET);

		std::map<int, SP_Vehicle> newCrashVehicles;
		pZoneGrid->AddVehicle(vProj, newCrashVehicles);
		if(!newCrashVehicles.empty())
		{
			std::map<int, SP_Vehicle>::iterator it = newCrashVehicles.begin();
			for (;it != newCrashVehicles.end(); ++it)
			{
				SP_Vehicle vCrash = it->second;
				SP_Vehicle vActual = NULL;
				if (stepVehMap->find(vCrash->GetVehicleID()) != stepVehMap->end())
					vActual = stepVehMap->at(vCrash->GetVehicleID()); 
				else
					continue;
					
				int idLo = min(vActual->GetVehicleID(), v->GetVehicleID());
				int idHi = max(vActual->GetVehicleID(), v->GetVehicleID());
				VehiclePair  vehPair(idLo, idHi);
				SP_Event pEvent = NULL;
#ifdef _OPENMP_LOCAL
				#pragma omp critical (ADDEVENTDATA)
#endif
				if (eventList.find(vehPair)  != eventList.end())
				{
					pEvent = eventList[vehPair]; 
					pEvent->AddVehicleData(vActual, v);
				} else
				{
					m_InitEventParams.m_V1 = vActual;
					m_InitEventParams.m_V2 = v;
					pEvent = std::make_shared<Event>(m_InitEventParams);
					eventList[vehPair] = pEvent;
				}
			}
		}
	}

	if (!eventList.empty())
	{	
		AnalyzEvents(m_TrjSrcName, eventList);
	}
}

void SSAM::AnalyzEvents(const std::string& trjSrcName, std::map<VehiclePair, SP_Event>& eventList)
{
	std::map<VehiclePair, SP_Event>::iterator it = eventList.begin();
	while (it != eventList.end())
	{
		SP_Event e = it->second;
		if(e->AnalyzeData(m_AnalysisTimeStep) == false)
		{
			if(e->IsConflict())
			{
				CreateConflict(e,trjSrcName);
			}
			eventList.erase(it++);	
		}
		else
		{
			++it;
		}
	}
}

void SSAM::CalcSummaries()
{
	m_Summaries.clear();
	if (m_ConflictList.empty())
		return;

	m_pSummary = std::make_shared<Summary>("Unfiltered-All Files", m_ConflictList);
	m_Summaries.push_back(m_pSummary);
	for (std::map<std::string, std::list<SP_Conflict> >::iterator it = m_FileToConflictsMap.begin();
		it != m_FileToConflictsMap.end(); ++it)
	{
		SP_Summary pFileSummary = std::make_shared<Summary>("Unfiltered-"+it->first, it->second);
		m_Summaries.push_back(pFileSummary);
	}
}

void SSAM::ExportResults()
{
	try
	{
		if(m_ConflictList.empty())
			throw SSAMException("No conflicts to export to a .csv file.");
		
		std::ofstream csvFile(m_CsvFileName);
		csvFile << std::fixed;			
		
		csvFile << "Summary Statistics\n";
		int m = 0;
		csvFile << "Stats,";
		for(m = 0; m < Conflict::NUM_MEASURES; m++)
			csvFile << Conflict::MEASURE_LABEL[m] << ",";
		csvFile << std::endl;
		
		csvFile << "Min,";
		for(m = 0; m < Conflict::NUM_MEASURES; m++)
			csvFile << m_pSummary->GetMinVals()[m]<< ",";
		csvFile<< std::endl;
		
		csvFile << "Max,";
		for(m = 0; m < Conflict::NUM_MEASURES; m++)
			csvFile << m_pSummary->GetMaxVals()[m] << ",";
		csvFile<< std::endl;
		
		csvFile << "Mean,";
		for(m = 0; m < Conflict::NUM_MEASURES; m++)
			csvFile << m_pSummary->GetMeanVals()[m] << ",";
		csvFile<< std::endl;
				
		csvFile << "Var,";
		for(m = 0; m < Conflict::NUM_MEASURES; m++)
			csvFile << m_pSummary->GetVarVals()[m] << ",";
		csvFile<< std::endl;
		csvFile<< std::endl;

		csvFile << "Conflict Listing," <<m_ConflictList.size() << "\n";
		csvFile << "Time (min TTC),X (min PET), Y (min PET),Z (min PET), ConflictType, FristVID,SecondVID,TTC,PET,MaxS,DeltaS,DR,MaxD,xFirstCSP,yFirstCSP,xFirstCEP,yFirstCEP,xSecondCSP,ySecondCSP,xSecondCEP,ySecondCEP\n";
		
		std::list<SP_Conflict>::iterator it = m_ConflictList.begin();
		for (; it != m_ConflictList.end(); ++it)
		{
			SP_Conflict c  = *it;
			
			csvFile<<c->tMinTTC;
			csvFile<<",";
			csvFile<<c->xMinPET;
			csvFile<<",";
			csvFile<<c->yMinPET;
			csvFile<<",";
			csvFile<<c->zMinPET;
			csvFile<<",";
			csvFile<<Conflict::CONFLICT_TYPE_LABEL[c->ConflictType];
			csvFile<<",";
			csvFile<<c->FirstVID;
			csvFile<<",";
			csvFile<<c->SecondVID;
			csvFile<<",";
			csvFile<<c->TTC;
			csvFile<<",";
			csvFile<<c->PET;
			csvFile<<",";
			csvFile<<c->MaxS;
			csvFile<<",";
			csvFile<<c->DeltaS;
			csvFile<<",";
			csvFile<<c->DR;
			csvFile<<",";
			csvFile<<c->MaxD;
			csvFile<<",";
			csvFile<<c->xFirstCSP;
			csvFile<<",";
			csvFile<<c->yFirstCSP;
			csvFile<<",";
			csvFile<<c->xFirstCEP;
			csvFile<<",";
			csvFile<<c->yFirstCEP;
			csvFile<<",";
			csvFile<<c->xSecondCSP;
			csvFile<<",";
			csvFile<<c->ySecondCSP;
			csvFile<<",";
			csvFile<<c->xSecondCEP;
			csvFile<<",";
			csvFile<<c->ySecondCEP;
			csvFile<<std::endl;
		}
		csvFile.close();
			
		std::cout << "Completed exporting to CSV file: " << m_CsvFileName <<std::endl;
	}
	catch(std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return;
}

void SSAM::ReadTrjInputSize()
{
	int fileSize = 0;
	std::string unitStr (" bytes");
	if (m_TrjFile.is_open())
	{
		m_TrjFile.seekg(0, m_TrjFile.end);
		fileSize = m_TrjFile.tellg();
		m_TrjFile.seekg(0, m_TrjFile.beg);
	} else if (!(m_TrjDataList.empty()))
	{
		fileSize = m_TrjDataList.size();
		unitStr = " records";
	}

	if(m_IsWriteDat)
	{
		m_DatFile << "TRJ Source," << m_TrjSrcName <<std::endl;
		m_DatFile << "Input Size," << fileSize << unitStr << std::endl;
	}
}

void SSAM::ReadInputFormat()
{
	try
	{
		m_Format.m_Endian = ReadByte(m_TrjFile);
		m_Format.m_Version = ReadFloat(m_TrjFile);
		if (m_Format.m_Version > ORIG_FORMAT_VERSION)
		{
			m_Format.m_ZOption = ReadByte(m_TrjFile);
		}
	} catch (SSAMException &e)
	{
		std::string errMsg("Error in reading format record: ");
		ThrowFileReadingException(e, errMsg);
	}
}

void SSAM::ReadDimensions()
{
	try
	{
		m_pDimensions = std::make_shared<Dimensions>();
		m_pDimensions->SetUnits(ReadByte(m_TrjFile));
		m_pDimensions->SetScale(ReadFloat(m_TrjFile));
		m_pDimensions->SetMinX(ReadInt(m_TrjFile));
		m_pDimensions->SetMinY(ReadInt(m_TrjFile));
		m_pDimensions->SetMaxX(ReadInt(m_TrjFile));
		m_pDimensions->SetMaxY(ReadInt(m_TrjFile));
	} catch (SSAMException &e)
	{
		std::string errMsg("Error in reading dimension record: ");
		ThrowFileReadingException(e, errMsg);
	}
}

void SSAM::ReadVehicle(SP_Vehicle v)
{
	try
	{
		v->SetVehicleID(ReadInt(m_TrjFile));
		v->SetLinkID(ReadInt(m_TrjFile));
		v->SetLaneID(ReadByte(m_TrjFile));
		float vFrontX = ReadFloat(m_TrjFile);
		float vFrontY = ReadFloat(m_TrjFile);
		float vRearX = ReadFloat(m_TrjFile);
		float vRearY = ReadFloat(m_TrjFile);
		v->setScale(m_pDimensions->GetScale());					
		v->SetLength(ReadFloat(m_TrjFile));
		v->SetWidth(ReadFloat(m_TrjFile));
		v->SetSpeed(ReadFloat(m_TrjFile));
		v->SetAcceleration(ReadFloat(m_TrjFile));
		v->SetPosition(vFrontX, vFrontY, vRearX, vRearY);
		if (m_Format.m_Version > ORIG_FORMAT_VERSION)
		{
			v->SetFrontZ( ReadFloat(m_TrjFile));
			v->SetRearZ( ReadFloat(m_TrjFile));
		}
	} catch (SSAMException &e)
	{
		std::string errMsg("Error in reading vehicle record: ");
		ThrowFileReadingException(e, errMsg);
	}
}

void SSAM::ValidateInputFormat()
{
	if(m_Format.m_Endian < 0)
		throw SSAMException("Invalid endian encoding: " + std::to_string(m_Format.m_Endian) + ".\n");
	if(m_Format.m_Endian != 'L' && m_Format.m_Endian != 'B')
	{
		throw SSAMException("Unrecognized encoding (neither big/little endian).\n");
	} 

	if (m_IsWriteDat)
		m_Format.Print(m_DatFile);
}

bool SSAM::ValidateVehicle(SP_Vehicle v)
{
	return ((v->GetCenterX() >= m_pDimensions->GetMinX() 
			  && v->GetCenterX() <= m_pDimensions->GetMaxX())
			&& (v->GetCenterY() >= m_pDimensions->GetMinY() 
			  && v->GetCenterY() <= m_pDimensions->GetMaxY())) ;
}

void SSAM::ThrowFileReadingException(SSAMException &e, std::string& errMsg)
{
	if (m_TrjFile.eof())
	{
		errMsg += "End of trj file is reached. \n";
	} else 
	{
		errMsg += e.what();	
	}
	throw SSAMException(errMsg);
}

void SSAM::PrintTimeStep(float t)
{
	if (m_IsFirstTimeStep)
	{
		m_DatFile<<"Timestep,"
			<<"Vehicle ID,"
			<<"Link ID,"
			<<"Lane ID,"
			<<"Front X,"
			<<"Front Y,"
			<<"Rear X,"
			<<"Rear Y,"
			<<"Length,"
			<<"Width,"
			<<"Speed,"
			<<"Acceleration,";
		if (m_Format.m_Version > ORIG_FORMAT_VERSION)
		{
			m_DatFile << "Front Z,"
				<< "Rear Z,";
		}
		m_DatFile << std::endl;
	}
	m_DatFile << t << std::endl;
}

void InputFormat::Print(std::ostream& output)
{
	output << "FORMAT\n";
	output << ",Byte Order," << m_Endian << std::endl;
	output << ",Version," <<  m_Version << std::endl;
	if (m_Version > ORIG_FORMAT_VERSION)
		output << ",zOption," <<  m_ZOption << std::endl;
	output << std::endl;
}

void Dimensions::Validate()
{
	if(m_Units < 0 || m_Units > 1)
		throw SSAMException("Invalid units specified for observation area dimensions.");

	if(m_Scale <= 0)
		throw SSAMException("Invalid (non-positive) scale factor specified for observation area dimensions.");

	if(m_MinX >= m_MaxX)
		throw SSAMException("Dimension record is invalid.  Min X >= Max X (" + std::to_string(m_MinX) + " >= " + std::to_string(m_MaxX) + ").");
	if(m_MinY >= m_MaxY)
		throw SSAMException("Dimension record is invalid.  Min Y >= Max Y (" + std::to_string(m_MinY) + " >= " + std::to_string(m_MaxY) + ").");
}

void Dimensions::Print(std::ostream& output)
{
	output << "DIMENSIONS\n";
	output << ",Units," << m_Units;
	if(m_Units == ENGLISH_UNITS)
		output << ",\"English (i.e., feet, feet/sec, feet/sec2)\"\n";
	else if(m_Units == METRIC_UNITS)
		output << ",\"Metric (i.e., meters, meters/sec, meters/sec2)\"\n";
	else
		output << ",unrecognized";

	output << ",Scale," <<  m_Scale << std::endl;
	output << ",MinX," << m_MinX << std::endl;
	output << ",MinY," << m_MinY << std::endl;
	output << ",MaxX," << m_MaxX << std::endl;
	output << ",MaxY," << m_MaxY << std::endl;
	output << std::endl;			
}
