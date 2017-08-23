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
#ifndef CONFLICT_H
#define CONFLICT_H
#include <vector>
#include <string>
#include "INCLUDE.h"
#include "Utility.h"
#include "Event.h"

#ifdef SSAMDLL_EXPORTS
#define SSAMFUNCSDLL_API __declspec(dllexport) 
#else
#define SSAMFUNCSDLL_API __declspec(dllimport) 
#endif

extern std::string GetFloatString(float x);

/** Conflict manages the safety measures for one confirmed conflict. 
  * Reference for measure meanings: 
  * Surrogate Safety Assessment Model and Validation: Final Report, 
  * Publication No. FHWA-HRT-08-051, JUNE 2008,
  * CHAPTER 2. SSAM SOFTWARE, TERMS AND DEFINITIONS,
  * Page 20-25.
*/
class SSAMFUNCSDLL_API Conflict
{
public:
	Conflict(){}
	~Conflict() {}

	const static int NUM_MEASURES = 44;  /*!< The number of SSAM measures to record */
	/** SSAM_MEASURE enum defines the column number of safety measures 
	*/
	enum SSAM_MEASURE
	{
		trjFile_MEASURE				,
		tMinTTC_MEASURE 			,
		xMinPET_MEASURE 			,
		yMinPET_MEASURE 			,
		zMinPET_MEASURE 			,
		TTC_MEASURE 				,
		PET_MEASURE 				,
		MaxS_MEASURE 				,
		DeltaS_MEASURE 				,
		DR_MEASURE 					,
		MaxD_MEASURE 				,
		MaxDeltaV_MEASURE			,
		ConflictAngle_MEASURE 		,
		ClockAngle_MEASURE 			,
		ConflictType_MEASURE 		,
		PostCrashV_MEASURE			,
		PostCrashHeading_MEASURE	,
		FirstVID_MEASURE 			,
		FirstLink_MEASURE 			,
		FirstLane_MEASURE 			,
		FirstLength_MEASURE 		,
		FirstWidth_MEASURE 			,
		FirstHeading_MEASURE  		,
		FirstVMinTTC_MEASURE  		,
		FirstDeltaV_MEASURE  		,
		xFirstCSP_MEASURE			,
		yFirstCSP_MEASURE 			,
		xFirstCEP_MEASURE			,
		yFirstCEP_MEASURE 			,
		SecondVID_MEASURE			,
		SecondLink_MEASURE			,
		SecondLane_MEASURE			,
		SecondLength_MEASURE		,
		SecondWidth_MEASURE			,
		SecondHeading_MEASURE 		,
		SecondVMinTTC_MEASURE  		,
		SecondDeltaV_MEASURE 		,
		xSecondCSP_MEASURE 			,
		ySecondCSP_MEASURE			,
		xSecondCEP_MEASURE			,
		ySecondCEP_MEASURE 			,
		PUEA_MEASURE 				,
		mTTC_MEASURE 				,
		mPET_MEASURE 				,
	};
	const static std::string MEASURE_LABEL[NUM_MEASURES]; /*!< Strings represent names of safety measures*/

	const static int NUM_CONFLICT_TYPES  = 4; /*!< The number of conflict types*/
	/** CONFLICT_TYPE enum defines integer representing each conflict type 
	*/
	enum CONFLICT_TYPE
	{
		UNCLASSIFIED,
		CROSSING,
		REAR_END,
		LANE_CHANGE,
	};
	const static std::string  CONFLICT_TYPE_LABEL[NUM_CONFLICT_TYPES]; /*!< Strings represent names of conflict types*/
	
	/** An array of flags indicate whether a safety measure should be summarized
    */
	const static bool SUM_MEASURES[NUM_MEASURES];

	/** An array of flags indicate whether a safety measure should be displayed in
	  * conflict list
    */
	const static bool KEY_MEASURES [NUM_MEASURES];
	
	
	std::string trjFile; 
	float 	tMinTTC; 
	float 	xMinPET;
	float 	yMinPET;
	float 	zMinPET;
	float 	TTC;
	float 	PET;
	float 	MaxS;
	float 	DeltaS;
	float 	DR;
	float 	MaxD;
	float	MaxDeltaV;
	float 	ConflictAngle;	
	std::string ClockAngle;
	int   	ConflictType;  	
	float	PostCrashV;
	float	PostCrashHeading;
	int   	FirstVID;
	int	FirstLink;
	int	FirstLane;
	float	FirstLength;
	float	FirstWidth;
	float 	FirstHeading;	
	float 	FirstVMinTTC;	
	float 	FirstDeltaV;	
	float 	xFirstCSP;	
	float 	yFirstCSP;	
	float 	xFirstCEP;	
	float 	yFirstCEP;	
	int   	SecondVID;
	int	SecondLink;
	int	SecondLane;
	float	SecondLength;
	float	SecondWidth;
	float 	SecondHeading;	
	float 	SecondVMinTTC;	
	float 	SecondDeltaV;	
	float 	xSecondCSP;	
	float 	ySecondCSP;	
	float 	xSecondCEP;	
	float 	ySecondCEP;	
	float PUEA; 
	float mTTC; 
	float mPET; 
	
	/** A constructor populates safety measures from a conflict event.
	  * @param pEvent A pointer to a conflict event.
	  * @param trjfile Name of the trj file.
    */
	Conflict(SP_Event e, const std::string& file) 
	{
		trjFile 			= file;
		tMinTTC 			= e->GetMinTTCTime();
		xMinPET 			= e->GetXMinPET();
		yMinPET 			= e->GetYMinPET();
		zMinPET 			= e->GetZMinPET();
		TTC 				= e->GetTTC();
		PET 				= e->GetPET();
		MaxS 				= e->GetMaxS();
		DeltaS 				= e->GetDeltaS();
		DR 					= e->GetDR();
		MaxD 				= e->GetMaxD();
		MaxDeltaV 			= e->GetMaxDeltaV();
		ConflictAngle 		= e->GetConflictAngle();
		ClockAngle 			= e->GetClockAngleString();
		ConflictType 		= e->GetConflictType();
		PostCrashV 			= e->GetPostCrashV();
		PostCrashHeading 	= e->GetPostCrashHeading();
		FirstVID 			= e->GetFirstVID();
		FirstLink 			= e->GetFirstLink();
		FirstLane 			= e->GetFirstLane();
		FirstLength 		= e->GetFirstLength();
		FirstWidth 			= e->GetFirstWidth();
		FirstHeading 		= e->GetFirstHeading();
		FirstVMinTTC 		= e->GetFirstVMinTTC();
		FirstDeltaV 		= e->GetFirstDeltaV();
		xFirstCSP 			= e->GetXFirstCSP();
		yFirstCSP 			= e->GetYFirstCSP();
		xFirstCEP 			= e->GetXFirstCEP();
		yFirstCEP 			= e->GetYFirstCEP();
		SecondVID 			= e->GetSecondVID();
		SecondLink 			= e->GetSecondLink();
		SecondLane 			= e->GetSecondLane();
		SecondLength 		= e->GetSecondLength();
		SecondWidth 		= e->GetSecondWidth();
		SecondHeading 		= e->GetSecondHeading();
		SecondVMinTTC 		= e->GetSecondVMinTTC();
		SecondDeltaV 		= e->GetSecondDeltaV();
		xSecondCSP 			= e->GetXSecondCSP();
		ySecondCSP 			= e->GetYSecondCSP();
		xSecondCEP 			= e->GetXSecondCEP();
		ySecondCEP 			= e->GetYSecondCEP();
		PUEA				= e->GetPUEA();
		mTTC				= e->GetMTTC();
		mPET				= e->GetMPET();
	}

	/** Get a numeric safety measure using its column order.
	  * @param i column order of the safety measure, starting from 0.
    */
	float GetMeasure(int i)
	{
		switch(i)
		{
			case tMinTTC_MEASURE: 			return tMinTTC;
			case xMinPET_MEASURE: 			return xMinPET;
			case yMinPET_MEASURE: 			return yMinPET;
			case zMinPET_MEASURE: 			return zMinPET;
			case TTC_MEASURE: 				return TTC;
			case PET_MEASURE: 				return PET;
			case MaxS_MEASURE:				return MaxS;
			case DeltaS_MEASURE:			return DeltaS;
			case DR_MEASURE: 				return DR;
			case MaxD_MEASURE:				return MaxD;
			case MaxDeltaV_MEASURE:			return MaxDeltaV;
			case ConflictAngle_MEASURE:		return ConflictAngle;
			case FirstVID_MEASURE: 			return FirstVID;
			case xFirstCSP_MEASURE: 		return xFirstCSP;
			case yFirstCSP_MEASURE: 		return yFirstCSP;
			case xFirstCEP_MEASURE: 		return xFirstCEP;
			case yFirstCEP_MEASURE: 		return yFirstCEP;
			case FirstHeading_MEASURE:		return FirstHeading;
			case SecondVID_MEASURE: 		return SecondVID;
			case xSecondCSP_MEASURE: 		return xSecondCSP;
			case ySecondCSP_MEASURE: 		return ySecondCSP;
			case xSecondCEP_MEASURE: 		return xSecondCEP;
			case ySecondCEP_MEASURE: 		return ySecondCEP;
			case SecondHeading_MEASURE:		return SecondHeading;
			case FirstLink_MEASURE:			return FirstLink;
			case SecondLink_MEASURE:		return SecondLink;	
			case PUEA_MEASURE:				return PUEA;
			case mTTC_MEASURE:				return mTTC;
			case mPET_MEASURE:				return mPET;
			default: 
				return 0;
		}
	}//	getMeasure()
	
	/** Get the trj file name without path.
    */
	std::string GetStrippedTrjName()
	{ 
		std::string result=trjFile;
		int numberoftokens;
		std::size_t pos = result.find_last_of('\\');
		if (pos != std::string::npos)
		{
			result = result.substr(pos+1);
		}
		return result;
	}
	

	//--------------------------------------------------------------------------
	/** Get a safety measure value as a string using its column order.
	  * @param i column order of the safety measure, starting from 0.
    */
	std::string GetValueString(int i)
	{ 
		std::string nulStr = "N/A";
		switch(i)
		{
		case	trjFile_MEASURE:	return	GetStrippedTrjName();	
		case	tMinTTC_MEASURE:	return	GetFloatString(	tMinTTC);
		case	xMinPET_MEASURE:	return	GetFloatString(	xMinPET);
		case	yMinPET_MEASURE:	return	GetFloatString(	yMinPET);
		case	zMinPET_MEASURE:	return	GetFloatString(	zMinPET);
		case	DeltaS_MEASURE:	return	GetFloatString(	DeltaS);
		case	MaxD_MEASURE:	return	GetFloatString(	MaxD);
		case	DR_MEASURE:	return	GetFloatString(	DR);
		case	MaxS_MEASURE:	return	GetFloatString(	MaxS);
		case	PET_MEASURE:	return	GetFloatString(	PET);
		case	TTC_MEASURE:	return	GetFloatString(	TTC);
		case	MaxDeltaV_MEASURE:	return	GetFloatString(	MaxDeltaV);
		case	ConflictAngle_MEASURE:	return	GetFloatString(	ConflictAngle);
		case	ClockAngle_MEASURE:	return	ClockAngle;	
		case	ConflictType_MEASURE:	return	CONFLICT_TYPE_LABEL[ConflictType];	
		case	PostCrashV_MEASURE:	return	GetFloatString(	PostCrashV);
		case	PostCrashHeading_MEASURE:	return	GetFloatString(	PostCrashHeading);
		case	FirstVID_MEASURE:	return	std::to_string(	FirstVID);
		case	FirstLink_MEASURE:	return	std::to_string(	FirstLink);
		case	FirstLane_MEASURE:	return	std::to_string(	FirstLane);
		case	FirstLength_MEASURE:	return	GetFloatString(	FirstLength);
		case	FirstWidth_MEASURE:	return	GetFloatString(	FirstWidth);
		case	FirstHeading_MEASURE:	return	GetFloatString(	FirstHeading);
		case	FirstVMinTTC_MEASURE:	return	GetFloatString(	FirstVMinTTC);
		case	FirstDeltaV_MEASURE:	return	GetFloatString(	FirstDeltaV);
		case	xFirstCSP_MEASURE:	return	GetFloatString(	xFirstCSP);
		case	yFirstCSP_MEASURE:	return	GetFloatString(	yFirstCSP);
		case	xFirstCEP_MEASURE:	return	GetFloatString(	xFirstCEP);
		case	yFirstCEP_MEASURE:	return	GetFloatString(	yFirstCEP);
		case	SecondVID_MEASURE:	return	std::to_string(	SecondVID);
		case	SecondLink_MEASURE:	return	std::to_string(	SecondLink);
		case	SecondLane_MEASURE:	return	std::to_string(	SecondLane);
		case	SecondLength_MEASURE:	return	GetFloatString(	SecondLength);
		case	SecondWidth_MEASURE:	return	GetFloatString(	SecondWidth);
		case	SecondHeading_MEASURE:	return	GetFloatString(	SecondHeading);
		case	SecondVMinTTC_MEASURE:	return	GetFloatString(	SecondVMinTTC);
		case	SecondDeltaV_MEASURE:	return	GetFloatString(	SecondDeltaV);
		case	xSecondCSP_MEASURE:	return	GetFloatString(	xSecondCSP);
		case	ySecondCSP_MEASURE:	return	GetFloatString(	ySecondCSP);
		case	xSecondCEP_MEASURE:	return	GetFloatString(	xSecondCEP);
		case	ySecondCEP_MEASURE:	return	GetFloatString(	ySecondCEP);
		case	PUEA_MEASURE: return GetFloatString(PUEA);
		case	mTTC_MEASURE: return GetFloatString(mTTC);
		case	mPET_MEASURE: return GetFloatString(mPET);
		default: return nulStr;
		}	
	}
};
/** Smart pointer type to Conflict class.
*/
typedef std::shared_ptr<Conflict> SP_Conflict;
#endif
