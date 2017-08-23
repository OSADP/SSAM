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
#include "Summary.h"

const std::string Summary::SUMMARY_LABEL[Summary::NUM_SUMMARY_LABELS] =
{
	"Summary Group",
	"SSAM_Measure", 
	"Min",
	"Max",
	"Mean",
	"Variance"
};

Summary::Summary(const std::string& trjFile, const std::list<SP_Conflict>&	conflictList)
{
	if (conflictList.empty())
	{
		return;
	}

	m_TrjFile = trjFile;
	//	Initialize the summary values
	for(int m = 0; m < Conflict::NUM_MEASURES; m++)
	{
		m_MinVals.push_back(FLT_MAX);
		m_MaxVals.push_back(-FLT_MAX);
		m_MeanVals.push_back(0);
		m_VarVals.push_back(0);
	}

	// initialize the conflict counts
	m_ConflictCounts.resize(Conflict::NUM_CONFLICT_TYPES+1, 0);

	//	Iterate through the conflicts and update the measures
	int count = 0;
	int nConflicts = 0;
	float measure = 0.0;
	float prevMean = 0.0;
	std::list<SP_Conflict>::const_iterator i = conflictList.begin();
	for (; i != conflictList.end(); ++i)
	{
		SP_Conflict c = *i;
		
		for(int m = 0; m < Conflict::NUM_MEASURES; m++)
		{
			measure = c->GetMeasure(m);
			m_MinVals[count] = std::min(m_MinVals[count], measure);
			m_MaxVals[count] = std::max(m_MaxVals[count], measure);
			//	calculate mean and variance in stream to prevent overflow
			prevMean = m_MeanVals[count];
			m_MeanVals[count]	= m_MeanVals[count] + (measure - m_MeanVals[count])/(float)(nConflicts + 1);
			if(nConflicts > 0)
				m_VarVals[count] = (m_VarVals[count]*(1.0 - 1.0/(float)nConflicts) + (float)(nConflicts + 1)*(m_MeanVals[count]-prevMean)*(m_MeanVals[count]-prevMean));
			count++;
		}
		count = 0;
		nConflicts++;
		
		m_ConflictCounts[c->ConflictType]++;
		m_ConflictCounts[Conflict::NUM_CONFLICT_TYPES]++; 
	}
}