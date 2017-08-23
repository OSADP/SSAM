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
#include "SSAMDoc.h"
#include<iostream>
#include<fstream>
#include<set>
#include<cfloat>
#include<cmath>
#include <iomanip>

#ifdef _OPENMP_LOCAL
#include <omp.h>
#endif

using namespace std;

SSAMDoc::SSAMDoc()
	: m_IsNewCase(true)
	, m_IsFilterApplied(false)
{
	m_pFilterParams = std::make_shared<FilterParams>();
}

void SSAMDoc::ResetDoc()
{
	m_AnalysisTime = 0;
	m_StartTime = 0;
	m_EndTime = 0;

	m_TrjFileNames.clear();
	m_EventList.clear();
	m_ConflictList.clear();
	m_Summaries.clear();
	m_FileToConflictsMap.clear();

	m_IsNewCase = true;
	m_IsFilterApplied = false;
	m_IsCalcPUEA = false;
	m_pFilterParams = std::make_shared<FilterParams>();

	m_Boundary[0] = INT_MAX;
	m_Boundary[1] = INT_MAX;
	m_Boundary[2] = INT_MIN;
	m_Boundary[3] = INT_MIN;
}

void SSAMDoc::InitFilterParams()
{
	SP_Summary pSummary = NULL;
	for (std::list<SP_Summary>::iterator it = m_Summaries.begin();
		it != m_Summaries.end(); ++it)
	{
		pSummary = *it;
		if (pSummary->GetTrjFile() == "Unfiltered-All Files")
			break;
	}

	if (pSummary != NULL)
	{
		m_pFilterParams->m_MinTTC = RoundDouble( pSummary->GetMinVals()[Conflict::TTC_MEASURE], 100.0);
		m_pFilterParams->m_MaxTTC = RoundDouble(pSummary->GetMaxVals()[Conflict::TTC_MEASURE], 100.0, false);

		m_pFilterParams->m_MinPET = RoundDouble(pSummary->GetMinVals()[Conflict::PET_MEASURE], 100.0);
		m_pFilterParams->m_MaxPET = RoundDouble(pSummary->GetMaxVals()[Conflict::PET_MEASURE], 100.0, false);

		m_pFilterParams->m_MinMaxS = RoundDouble(pSummary->GetMinVals()[Conflict::MaxS_MEASURE], 1000.0);
		m_pFilterParams->m_MaxMaxS = RoundDouble(pSummary->GetMaxVals()[Conflict::MaxS_MEASURE], 1000.0, false);

		m_pFilterParams->m_MinDeltaS = RoundDouble(pSummary->GetMinVals()[Conflict::DeltaS_MEASURE] , 1000.0);
		m_pFilterParams->m_MaxDeltaS = RoundDouble(pSummary->GetMaxVals()[Conflict::DeltaS_MEASURE] , 1000.0, false);

		m_pFilterParams->m_MinDR = RoundDouble(pSummary->GetMinVals()[Conflict::DR_MEASURE], 1000.0);
		m_pFilterParams->m_MaxDR = RoundDouble(pSummary->GetMaxVals()[Conflict::DR_MEASURE], 1000.0, false);

		m_pFilterParams->m_MinMaxD = RoundDouble(pSummary->GetMinVals()[Conflict::MaxD_MEASURE], 1000.0);
		m_pFilterParams->m_MaxMaxD = RoundDouble(pSummary->GetMaxVals()[Conflict::MaxD_MEASURE], 1000.0, false);

		m_pFilterParams->m_MinMaxDeltaV = RoundDouble(pSummary->GetMinVals()[Conflict::MaxDeltaV_MEASURE], 1000.0);
		m_pFilterParams->m_MaxMaxDeltaV = RoundDouble(pSummary->GetMaxVals()[Conflict::MaxDeltaV_MEASURE], 1000.0, false);
	} else
	{
		m_pFilterParams->m_MinTTC = 0.0;
		m_pFilterParams->m_MaxTTC = 0.0;

		m_pFilterParams->m_MinPET = 0.0;
		m_pFilterParams->m_MaxPET = 0.0;

		m_pFilterParams->m_MinMaxS = 0.0;
		m_pFilterParams->m_MaxMaxS = 0.0;

		m_pFilterParams->m_MinDeltaS = 0.0;
		m_pFilterParams->m_MaxDeltaS = 0.0;

		m_pFilterParams->m_MinDR = 0.0;
		m_pFilterParams->m_MaxDR = 0.0;

		m_pFilterParams->m_MinMaxD = 0.0;
		m_pFilterParams->m_MaxMaxD = 0.0;

		m_pFilterParams->m_MinMaxDeltaV = 0.0;
		m_pFilterParams->m_MaxMaxDeltaV = 0.0;
	}


	for (int i = 0; i < 4; ++i)
	{
		m_pFilterParams->m_Area[i] = m_Boundary[i];
	}

	m_pFilterParams->m_ConflictTypes.clear();
	m_pFilterParams->m_Links.clear();
	m_pFilterParams->m_TRJFiles.clear();

	m_FilteredConflictList.clear();
	m_FilteredFileToConflictsMap.clear();
	m_FilteredSummaries.clear();

	m_IsFilterApplied = false;
}

bool SSAMDoc::ApplyFilter(bool IsAreaOnly)
{
	m_FilteredConflictList.clear();
	m_FilteredFileToConflictsMap.clear();
	m_FilteredSummaries.clear();

	FilterConflicts(m_ConflictList, m_FilteredConflictList, IsAreaOnly);
	if (m_FilteredConflictList.empty())
	{
		m_IsFilterApplied = false;
		return false;
	}

	m_FilteredSummaries.clear();
	m_FilteredSummaries.push_back(m_pSummary); // "Unfiltered-All Files"

	m_pFilteredSummary = std::make_shared<Summary>("Filtered-All Files", m_FilteredConflictList);
	m_FilteredSummaries.push_back(m_pFilteredSummary);

	for (std::map<std::string, std::list<SP_Conflict> >::iterator it = m_FilteredFileToConflictsMap.begin();
		it != m_FilteredFileToConflictsMap.end(); ++it)
	{
		SP_Summary pFileSummary = std::make_shared<Summary>("Filtered-"+it->first, it->second);
		m_FilteredSummaries.push_back(pFileSummary);
	}

	m_IsFilterApplied = true;
	return true;
}

void SSAMDoc::FilterConflicts(const std::list<SP_Conflict>& origList, 
						   std::list<SP_Conflict>& filteredList,
						   bool IsAreaOnly)
{
	//filteredList.clear();
	for (std::list<SP_Conflict>::const_iterator it = origList.begin();
		it != origList.end(); ++it)
	{
		const SP_Conflict pConflict = *it;

		if (IsAreaOnly)
		{
			if (int(pConflict->xMinPET * 10.0 + 0.5) >= int(m_pFilterParams->m_Area[0] * 10.0 + 0.5)
				&& int(pConflict->xMinPET * 10.0 + 0.5) <= int(m_pFilterParams->m_Area[2] * 10.0 + 0.5)
				&& int(pConflict->yMinPET * 10.0 + 0.5) >= int(m_pFilterParams->m_Area[1] * 10.0 + 0.5)
				&& int(pConflict->yMinPET * 10.0 + 0.5) <= int(m_pFilterParams->m_Area[3] * 10.0 + 0.5) )
			{
				filteredList.push_back(pConflict);
				m_FilteredFileToConflictsMap[pConflict->trjFile].push_back(pConflict);
			}
		} else
		{
			if (int(pConflict->TTC * 100.0 + 0.5) >= int(m_pFilterParams->m_MinTTC * 100.0 + 0.5)
				&& int(pConflict->TTC * 100.0 + 0.5) <= int(m_pFilterParams->m_MaxTTC * 100.0 + 0.5)
				&& int(pConflict->PET * 100.0 + 0.5) >= int(m_pFilterParams->m_MinPET * 100.0 + 0.5)
				&& int(pConflict->PET * 100.0 + 0.5) <= int(m_pFilterParams->m_MaxPET * 100.0 + 0.5)
				&& int(pConflict->MaxS * 1000.0 + 0.5) >= int(m_pFilterParams->m_MinMaxS * 1000.0 + 0.5)
				&& int(pConflict->MaxS * 1000.0 + 0.5) <= int(m_pFilterParams->m_MaxMaxS * 1000.0 + 0.5)
				&& int(pConflict->DeltaS * 1000.0 + 0.5) >= int(m_pFilterParams->m_MinDeltaS * 1000.0 + 0.5)
				&& int(pConflict->DeltaS * 1000.0 + 0.5) <= int(m_pFilterParams->m_MaxDeltaS * 1000.0 + 0.5)
				&& int(pConflict->DR * 1000.0 + 0.5) >= int(m_pFilterParams->m_MinDR * 1000.0 + 0.5)
				&& int(pConflict->DR * 1000.0 + 0.5) <= int(m_pFilterParams->m_MaxDR * 1000.0 + 0.5)
				&& int(pConflict->MaxD * 1000.0 + 0.5) >= int(m_pFilterParams->m_MinMaxD * 1000.0 + 0.5)
				&& int(pConflict->MaxD * 1000.0 + 0.5) <= int(m_pFilterParams->m_MaxMaxD * 1000.0 + 0.5)
				&& int(pConflict->MaxDeltaV * 1000.0 + 0.5) >= int(m_pFilterParams->m_MinMaxDeltaV * 1000.0 + 0.5)
				&& int(pConflict->MaxDeltaV * 1000.0 + 0.5) <= int(m_pFilterParams->m_MaxMaxDeltaV * 1000.0 + 0.5)
				&& int(pConflict->xMinPET * 10.0 + 0.5) >= int(m_pFilterParams->m_Area[0] * 10.0 + 0.5)
				&& int(pConflict->xMinPET * 10.0 + 0.5) <= int(m_pFilterParams->m_Area[2] * 10.0 + 0.5)
				&& int(pConflict->yMinPET * 10.0 + 0.5) >= int(m_pFilterParams->m_Area[1] * 10.0 + 0.5)
				&& int(pConflict->yMinPET * 10.0 + 0.5) <= int(m_pFilterParams->m_Area[3] * 10.0 + 0.5) )
			{
				if (!m_pFilterParams->m_ConflictTypes.empty()
					&& m_pFilterParams->m_ConflictTypes.find(pConflict->ConflictType) == m_pFilterParams->m_ConflictTypes.end())
				{
					continue;
				}

				if (!m_pFilterParams->m_Links.empty()
					&& m_pFilterParams->m_Links.find(pConflict->FirstLink) == m_pFilterParams->m_Links.end()
					&& m_pFilterParams->m_Links.find(pConflict->SecondLink) == m_pFilterParams->m_Links.end())
				{
					continue;
				}

				bool IsValid = false;
				if (m_pFilterParams->m_TRJFiles.empty())
				{
					filteredList.push_back(pConflict);
					m_FilteredFileToConflictsMap[pConflict->trjFile].push_back(pConflict);
				} else
				{
					for (std::vector<std::string>::iterator itf = m_pFilterParams->m_TRJFiles.begin();
						itf != m_pFilterParams->m_TRJFiles.end(); ++itf)
					{
						if ( pConflict->trjFile == *itf)
						{
							filteredList.push_back(pConflict);
							m_FilteredFileToConflictsMap[pConflict->trjFile].push_back(pConflict);
							break;
						}
					}
				}
			}
		}
	}
}

void SSAMDoc::Save(const std::string& fileName)
{
	m_DocName = fileName;
	std::ofstream ssamFile(fileName);
	
	float val = 0.0;
	int ival = 0;
	char ch = ' ';

	// std::list<SP_Conflict> m_ConflictList;
	ssamFile << m_ConflictList.size() << ",";
	for (std::list<SP_Conflict>::iterator it = m_ConflictList.begin(); it != m_ConflictList.end(); ++it)
	{
		ssamFile << (*it)->trjFile << ",";
		for (int iCol = 1; iCol < Conflict::NUM_MEASURES; iCol++)
		{
			if (iCol != Conflict::ConflictType_MEASURE)
				ssamFile << (*it)->GetValueString(iCol) << ",";
			else
				ssamFile << (*it)->ConflictType << ",";
		}
	}
	
	// SP_FilterParams m_pFilterParams;
	ssamFile << m_pFilterParams->	m_MinTTC	<< ",";
	ssamFile << m_pFilterParams->	m_MaxTTC	<< ",";
	ssamFile << m_pFilterParams->	m_MinPET	<< ",";
	ssamFile << m_pFilterParams->	m_MaxPET	<< ",";
	ssamFile << m_pFilterParams->	m_MinMaxS	<< ",";
	ssamFile << m_pFilterParams->	m_MaxMaxS	<< ",";
	ssamFile << m_pFilterParams->	m_MinDeltaS	<< ",";
	ssamFile << m_pFilterParams->	m_MaxDeltaS	<< ",";
	ssamFile << m_pFilterParams->	m_MinDR	<< ",";
	ssamFile << m_pFilterParams->	m_MaxDR	<< ",";
	ssamFile << m_pFilterParams->	m_MinMaxD	<< ",";
	ssamFile << m_pFilterParams->	m_MaxMaxD	<< ",";
	ssamFile << m_pFilterParams->	m_MinMaxDeltaV	<< ",";
	ssamFile << m_pFilterParams->	m_MaxMaxDeltaV	<< ",";

	for (int i = 0; i < 4; ++i)
	{
		ssamFile <<  m_pFilterParams->m_Area[i] << ",";
	}
	
	ssamFile <<  m_pFilterParams->m_ConflictTypes.size() << ",";
	for (std::set<int>::iterator it = m_pFilterParams->m_ConflictTypes.begin();
		it != m_pFilterParams->m_ConflictTypes.end(); ++it)
	{
		ssamFile <<   *it << ",";
	}
	
	ssamFile <<  m_pFilterParams->m_Links.size() << ",";
	for (std::set<int>::iterator it = m_pFilterParams->m_Links.begin();
		it != m_pFilterParams->m_Links.end(); ++it)
	{
		ssamFile <<   *it << ",";
	}
	
	ssamFile <<  m_pFilterParams->m_TRJFiles.size() << ",";
	for (std::vector<std::string>::iterator it = m_pFilterParams->m_TRJFiles.begin();
		it != m_pFilterParams->m_TRJFiles.end(); ++it)
	{
		ssamFile <<   *it << ",";
	}
	
	// std::list<std::string> m_TrjFileNames
	ssamFile <<  m_TrjFileNames.size() << ",";
	for (std::list<std::string>::iterator it = m_TrjFileNames.begin();
		it != m_TrjFileNames.end(); ++it)
	{
		ssamFile <<   *it << ",";
	}

	// double m_Boundary[4]; // 0: minX, 1: minY, 2: maxX, 3: maxY
	for (int i = 0; i < 4; ++i)
	{
		ssamFile <<   m_Boundary[i] << ","; 
	}

	//bool m_IsFilterApplied;
	ssamFile <<  ( (m_IsFilterApplied) ? 1 : 0)<< ",";
	
	// removed variables for backward compatibility
	ssamFile <<  ",";
	ssamFile <<  ",";
	ssamFile <<   m_CsvFileName << ",";
	ssamFile <<  ",";
	ssamFile <<  ",";
	ssamFile <<  ",";
	ssamFile <<   m_MaxTTC << ",";
	ssamFile <<   m_MaxPET << ",";
	ssamFile <<   m_RearEndAngleThreshold << ",";
	ssamFile <<   m_CrossingAngleThreshold << ",";

	//bool m_IsCalcPUEA;
	ssamFile <<  (  (m_IsCalcPUEA) ? 1 : 0)<< ",";
	
	ssamFile.close();
}

void SSAMDoc::Open(const std::string& fileName)
{
	m_DocName = fileName;
	std::ifstream ssamFile(fileName);

	std::vector<std::string> fields;
	char delimiter = ',';
	for (std::string token; std::getline(ssamFile, token, delimiter); fields.push_back(token));
	try
	{
		ResetDoc();
		int i = 0;
		//std::list<SP_Conflict> m_ConflictList;;
		int nConflicts = stoi(fields[i++]);
		for (int ic = 0; ic < nConflicts; ++ic)
		{
			SP_Conflict c = std::make_shared<Conflict>();
			c->trjFile = fields[i++];
			c->tMinTTC	=	stof(fields[i++]);
			c->	xMinPET	=	stof(fields[i++]);	
			c->	yMinPET	=	stof(fields[i++]);	
			c->	zMinPET	=	stof(fields[i++]);	
			c->	TTC	=	stof(fields[i++]);	
			c->	PET	=	stof(fields[i++]);	
			c->	MaxS	=	stof(fields[i++]);	
			c->	DeltaS	=	stof(fields[i++]);	
			c->	DR	=	stof(fields[i++]);	
			c->	MaxD	=	stof(fields[i++]);	
			c->	MaxDeltaV	=	stof(fields[i++]);	
			c->	ConflictAngle	=	stof(fields[i++]);	
			c->	ClockAngle = fields[i++];	
			c->	ConflictType	=	stoi(fields[i++]);	
			c->	PostCrashV	=	stof(fields[i++]);	
			c->	PostCrashHeading	=	stof(fields[i++]);	
			c->	FirstVID	=	stoi(fields[i++]);	
			c->	FirstLink	=	stoi(fields[i++]);	
			c->	FirstLane	=	stoi(fields[i++]);	
			c->	FirstLength	=	stof(fields[i++]);	
			c->	FirstWidth	=	stof(fields[i++]);	
			c->	FirstHeading	=	stof(fields[i++]);	
			c->	FirstVMinTTC	=	stof(fields[i++]);	
			c->	FirstDeltaV	=	stof(fields[i++]);	
			c->	xFirstCSP	=	stof(fields[i++]);	
			c->	yFirstCSP	=	stof(fields[i++]);	
			c->	xFirstCEP	=	stof(fields[i++]);	
			c->	yFirstCEP	=	stof(fields[i++]);	
			c->	SecondVID	=	stoi(fields[i++]);	
			c->	SecondLink	=	stoi(fields[i++]);	
			c->	SecondLane	=	stoi(fields[i++]);	
			c->	SecondLength	=	stof(fields[i++]);	
			c->	SecondWidth	=	stof(fields[i++]);	
			c->	SecondHeading	=	stof(fields[i++]);	
			c->	SecondVMinTTC	=	stof(fields[i++]);	
			c->	SecondDeltaV	=	stof(fields[i++]);	
			c->	xSecondCSP	=	stof(fields[i++]);	
			c->	ySecondCSP	=	stof(fields[i++]);	
			c->	xSecondCEP	=	stof(fields[i++]);	
			c->	ySecondCEP	=	stof(fields[i++]);	
			c->	PUEA	=	stof(fields[i++]);	
			c->	mTTC	=	stof(fields[i++]);	
			c->	mPET	=	stof(fields[i++]);	

			m_ConflictList.push_back(c);
			m_FileToConflictsMap[c->trjFile].push_back(c);
		}
	
		// SP_FilterParams m_pFilterParams;
		m_pFilterParams->	m_MinTTC	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxTTC	=	RoundDouble(stof(fields[i++]), 1000.0, false);
		m_pFilterParams->	m_MinPET	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxPET	=	RoundDouble(stof(fields[i++]), 1000.0, false);
		m_pFilterParams->	m_MinMaxS	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxMaxS	=	RoundDouble(stof(fields[i++]), 1000.0, false);
		m_pFilterParams->	m_MinDeltaS	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxDeltaS	=	RoundDouble(stof(fields[i++]), 1000.0, false);
		m_pFilterParams->	m_MinDR	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxDR	=	RoundDouble(stof(fields[i++]), 1000.0, false);
		m_pFilterParams->	m_MinMaxD	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxMaxD	=	RoundDouble(stof(fields[i++]), 1000.0, false);
		m_pFilterParams->	m_MinMaxDeltaV	=	RoundDouble(stof(fields[i++]), 1000.0);
		m_pFilterParams->	m_MaxMaxDeltaV	=	RoundDouble(stof(fields[i++]), 1000.0, false);



		for (int ic = 0; ic < 4; ++ic)
		{
			m_pFilterParams->m_Area[ic] =	stof(fields[i++]);
		}

		int nConflictTypes = stoi(fields[i++]);	
		for (int ic = 0; ic < nConflictTypes; ++ic)
		{
			m_pFilterParams->m_ConflictTypes.insert(stoi(fields[i++]));
		}
	
		int nLinks = stoi(fields[i++]);
		for (int ic = 0; ic < nLinks; ++ic)
		{
			m_pFilterParams->m_Links.insert(stoi(fields[i++]));
		}

		int nFiles = stoi(fields[i++]);
		for (int ic = 0; ic < nFiles; ++ic)
		{
			m_pFilterParams->m_TRJFiles.push_back(fields[i++]);
		}

		// std::list<std::string>		m_TrjFileNames;
		nFiles = stoi(fields[i++]);
		for (int ic = 0; ic < nFiles; ++ic)
		{
			m_TrjFileNames.push_back(fields[i++]);
		}
	
	
		// double m_Boundary[4]; // 0: minX, 1: minY, 2: maxX, 3: maxY
		for (int ic = 0; ic < 4; ++ic)
		{
			m_Boundary[ic] = stof(fields[i++]);
		}

		//bool m_IsFilterApplied;
		int ival = stoi(fields[i++]);
		m_IsFilterApplied = (ival == 1);
		
		std::string tmp; // removed variables for backward compatibility
		tmp = fields[i++];
		tmp = fields[i++];
		m_CsvFileName = fields[i++];
		tmp = fields[i++];
		tmp = fields[i++];
		tmp = fields[i++];
		m_MaxTTC = stof(fields[i++]);
		m_MaxPET = stof(fields[i++]);
		m_RearEndAngleThreshold = stoi(fields[i++]);
		m_CrossingAngleThreshold = stoi(fields[i++]);

		//bool m_IsCalcPUEA;
		ival = stoi(fields[i++]);
		m_IsCalcPUEA = (ival == 1);
		
		ssamFile.close();

		m_IsNewCase = true;
		CalcSummaries();
		if (m_IsFilterApplied)
		{
			ApplyFilter();
		}
	} catch (const std::invalid_argument& e)
	{
		std::string errMsg(e.what());
		errMsg = "Error in reading SSAM file: " + errMsg;
		throw SSAMException(errMsg);
	}
}
