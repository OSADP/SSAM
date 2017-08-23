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
#ifndef SUMMARY_H
#define SUMMARY_H
#include "INCLUDE.h"
#include "Conflict.h"
#include <list>

#ifdef SSAMDLL_EXPORTS
#define SSAMFUNCSDLL_API __declspec(dllexport) 
#else
#define SSAMFUNCSDLL_API __declspec(dllimport) 
#endif

/** Summary manages the summaries of safety measures for confirmed conflict 
  * in one TRJ input source.
*/
class SSAMFUNCSDLL_API Summary
{
public:
	Summary(){}
	~Summary(){}
	Summary(const std::string& trjFile, const std::list<SP_Conflict>& conflictList);

	static const int NUM_SUMMARY_LABELS = 6; /*!< The number of summary labels */
	const static std::string SUMMARY_LABEL[NUM_SUMMARY_LABELS]; /*!< Strings represent summary labels*/

	const std::string& GetTrjFile() const {return m_TrjFile; }
	const std::vector<float>& GetMinVals() const {return m_MinVals;}
	const std::vector<float>& GetMaxVals() const {return m_MaxVals;}
	const std::vector<float>& GetMeanVals() const {return m_MeanVals;}
	const std::vector<float>& GetVarVals() const {return m_VarVals;}
	const std::vector<int>& GetConflictTypeCounts() const {return m_ConflictCounts;}
private:
	std::string m_TrjFile; /*!< The name of the trajectory file where the conflicts are identified */
	std::vector<float> m_MinVals; /*!< The minimum values of summarized safety measures */
	std::vector<float> m_MaxVals; /*!< The maximum values of summarized safety measures */
	std::vector<float> m_MeanVals; /*!< The mean values of summarized safety measures */
	std::vector<float> m_VarVals; /*!< The variance values of summarized safety measures */

	/** 
     * The numbers of conflicts of all conflict types.
     * The last element is the total number of conflicts 
     */
	std::vector<int> m_ConflictCounts; 
};

/** Smart pointer type to Summary class.
*/
typedef std::shared_ptr<Summary> SP_Summary;
#endif // SUMMARY_H