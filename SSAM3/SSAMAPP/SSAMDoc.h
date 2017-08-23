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
#ifndef SSAMDOC_H
#define SSAMDOC_H

#include <set>
#include "SSAM.h"

/** FilterParams organizes parameters for filtering conflict points
*/
struct FilterParams
{
	double m_MinTTC, m_MaxTTC;
	double m_MinPET, m_MaxPET;
	double m_MinMaxS, m_MaxMaxS;
	double m_MinDeltaS, m_MaxDeltaS;
	double m_MinDR, m_MaxDR;
	double m_MinMaxD, m_MaxMaxD;
	double m_MinMaxDeltaV, m_MaxMaxDeltaV;
	/*!< Array of filtering area; elements indexes: 0: minX, 1: minY, 2: maxX, 3: maxY */
	double m_Area[4];  
	std::set<int> m_ConflictTypes;
	std::set<int> m_Links;
	std::vector<std::string> m_TRJFiles;

	FilterParams()
		: m_MinTTC (0)
		, m_MaxTTC (0)
		, m_MinPET (0)
		, m_MaxPET (0)
		, m_MinMaxS (0)
		, m_MaxMaxS (0)
		, m_MinDeltaS (0)
		, m_MaxDeltaS (0)
		, m_MinDR (0)
		, m_MaxDR (0)
		, m_MinMaxD (0)
		, m_MaxMaxD (0)
		, m_MinMaxDeltaV (0)
		, m_MaxMaxDeltaV(0)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_Area[i] = 0;
		}
	}

	~FilterParams(){};
};
/** Smart pointer type to FilterParams struct.
*/
typedef std::shared_ptr<FilterParams> SP_FilterParams;

using namespace SSAMFuncs;
/** SSAMDoc derives from SSAM class and adds GUI related variables and methods
*/
class SSAMDoc : public SSAM
{
public:
	SSAMDoc();
	~SSAMDoc(){};
	
	/** Initialize parameters for filtering conflict points. 
	*/
	void InitFilterParams();

	/** Apply filtering parameters to conflict points and calculate filtered summaries.
	* @param IsAreaOnly a flag to indicate whether to filter area only
	* @return a flag to indicate whether filtered values exist.
	* true: filtered values exist; false: no filtered values
	*/
	bool ApplyFilter(bool IsAreaOnly = false);

	/** Save SSAMDoc to a file.
	* @fileName the file name to save SSAMDoc.
	*/
	void Save(const std::string& fileName);

	/** Open SSAMDoc from a file.
	* @fileName the file name to open SSAMDoc.
	*/
	void Open(const std::string& fileName);

	/** Reset SSAMDoc to run a new SSAM analysis.
	*/
	void ResetDoc();
	
	/** Filter conflict points.
	* @param origList list of conflict points to filter
	* @param filteredList output filtered list of conflict points
	* @IsAreaOnly a flag to indicat whether to filter area only
	*/
	void FilterConflicts(const std::list<SP_Conflict>& origList, 
		std::list<SP_Conflict>& filteredList,
		bool IsAreaOnly = false);

	/** Round a double number to target number of digits.
	* @param x double number to round
	* @param m the number of digits to remain, represented by exponent of 10
	* @param IsMin a flag to indicate whether x is the minimum threshold
	* @return the rounded double number
	*/
	double RoundDouble(double x, double m, bool IsMin = true)
	{
		return (x >= 0) ? double(int(x * m + 0.5)) / m : double(int(x * m - 0.5)) / m;
	}
	
	//	get()/set() methods
	bool IsNewCase() {return m_IsNewCase;}
	void SetIsNewCase(bool isNewCase) {m_IsNewCase = isNewCase;}
	bool IsFilterApplied() {return m_IsFilterApplied;}
	SP_FilterParams GetFilterParams() { return m_pFilterParams; }
	std::list<SP_Conflict>& GetFilteredConflictList() {return m_FilteredConflictList;}
	SP_Summary GetFilteredSummary() {return m_pFilteredSummary;}
	std::list<SP_Summary>& GetFilteredSummaries() {return m_FilteredSummaries;}
	std::string GetDocName() 
	{
		return GetFileName(m_DocName);
	}
	std::string GetCsvName()
	{ 
		return GetFileName(m_CsvFileName);
	}
	
private:
	bool m_IsNewCase; /*!< a flag to indicate whether this is new SSAM analysis result */
	/*!< a flag to indicate whether filtered values exist.
	* true: filtered values exist; false: no filtered values
	*/
	bool m_IsFilterApplied; 
	SP_FilterParams m_pFilterParams; /*!< parameters for filtering conflict points. */
	std::list<SP_Conflict> m_FilteredConflictList; /*!< filtered list of conflict points */
	/*!< A map container stores filtered conflict points using TRJ source names as keys*/
	std::map<std::string, std::list<SP_Conflict> > m_FilteredFileToConflictsMap;
	SP_Summary m_pFilteredSummary; /*!< summary over all filtered TRJ inputs */
	std::list<SP_Summary> m_FilteredSummaries; /*!< list of summaries, each for one TRJ source */
	std::string m_DocName; /*!< file name for saving current SSAMDoc. */

	/** Get file name without path.
	* @param fullName file name with path
	* @return file name without path
	*/
	std::string GetFileName(const std::string& fullName)
	{
		std::string fileName(fullName);
		std::size_t pos = fileName.find_last_of('\\');
		if (pos != std::string::npos)
		{
			fileName = fileName.substr(pos+1);
		}
		return fileName;
	}
};
/** Smart pointer type to SSAMDoc class.
*/
typedef std::shared_ptr<SSAMDoc> SP_SSAMDoc;

#endif 
