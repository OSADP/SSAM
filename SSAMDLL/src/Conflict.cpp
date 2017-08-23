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
#include "stdafx.h"
#include "Conflict.h"

const std::string Conflict::MEASURE_LABEL[Conflict::NUM_MEASURES] = {
		"trjFile",
		"tMinTTC",
		"xMinPET",
		"yMinPET",
		"zMinPET",
		"TTC",
		"PET",
		"MaxS",
		"DeltaS",
		"DR",
		"MaxD",
		"MaxDeltaV",
		"ConflictAngle",
		"ClockAngle",
		"ConflictType",
		"PostCrashV",
		"PostCrashHeading", 	
		"FirstVID",
		"FirstLink",
		"FirstLane",
		"FirstLength",
		"FirstWidth",
		"FirstHeading",
		"FirstVMinTTC",
		"FirstDeltaV",
		"xFirstCSP",		
		"yFirstCSP", 		
		"xFirstCEP",		
		"yFirstCEP", 		
		"SecondVID",
		"SecondLink",
		"SecondLane",
		"SecondLength",
		"SecondWidth",
		"SecondHeading",
		"SecondVMinTTC",
		"SecondDeltaV",
		"xSecondCSP", 		
		"ySecondCSP",		
		"xSecondCEP",		
		"ySecondCEP",
		"P(UEA)",
		"mTTC",
		"mPET"
};

const std::string  Conflict::CONFLICT_TYPE_LABEL[Conflict::NUM_CONFLICT_TYPES] = 
{
	"unclassified",
   	"crossing",
   	"rear end",
   	"lane change"		
};

const bool Conflict::SUM_MEASURES [Conflict::NUM_MEASURES] =
{ 
	false, 	//	trjFile
	false, 	//	tMinTTC,
	false, 	//	xMinPET,
	false,	//	yMinPET
	false,	//	zMinPET
	true, 	//	TTC
	true, 	//	PET
	true, 	//	MaxS
	true, 	//	DeltaS
	true, 	//	DR
	true, 	//	MaxD
	true, 	//	MaxDeltaV
	false,	//	ConflictAngle
	false,  //  ClockAngle
	false,	//	ConflictType
	false,	// 	PostCrashV
	false,  //	PostCrashHeading		
	false, 	//	FirstVID
	false,	//	FirstLink
	false,	//	FirstLane
	false,	//	FirstLength
	false,	//	FirstWidth
	false,	//	FirstHeading
	false, 	//	FirstVMinTTC
	false, 	//	FirstDeltaV
	false, 	//	xFirstCSP
	false,	//	yFirstCSP
	false, 	//	xFirstCEP
	false, 	//	yFirstCEP
	false, 	//	SecondVID
	false,	//	SecondLink
	false,	//	SecondLane
	false,	//	SecondLength
	false,	//	SecondWidth
	false,	//	SecondHeading
	false, 	//	SecondVMinTTC
	false, 	//	SecondDeltaV
	false, 	//	xSecondCSP
	false, 	//	ySecondCSP
	false, 	//	xSecondCEP
	false,	//	ySecondCEP
	true,  // P(UEA)
	true,  // mTTC
	true,  // mPET
};

const bool Conflict::KEY_MEASURES [Conflict::NUM_MEASURES] =
{ 
	true, 	//	trjFile
	true, 	//	tMinTTC,
	true, 	//	xMinPET,
	true,	//	yMinPET
	true,	//	zMinPET
	true, 	//	TTC
	true, 	//	PET
	true, 	//	MaxS
	true, 	//	DeltaS
	true, 	//	DR
	true, 	//	MaxD
	true, 	//	MaxDeltaV
	false,	//	ConflictAngle
	false,  //  ClockAngle
	true,	//	ConflictType
	false,	// 	PostCrashV
	false,  //	PostCrashHeading		
	true, 	//	FirstVID
	true,	//	FirstLink
	true,	//	FirstLane
	false,	//	FirstLength
	false,	//	FirstWidth
	false,	//	FirstHeading
	false, 	//	FirstVMinTTC
	false, 	//	FirstDeltaV
	false, 	//	xFirstCSP
	false,	//	yFirstCSP
	false, 	//	xFirstCEP
	false, 	//	yFirstCEP
	true, 	//	SecondVID
	true,	//	SecondLink
	true,	//	SecondLane
	false,	//	SecondLength
	false,	//	SecondWidth
	false,	//	SecondHeading
	false, 	//	SecondVMinTTC
	false, 	//	SecondDeltaV
	false, 	//	xSecondCSP
	false, 	//	ySecondCSP
	false, 	//	xSecondCEP
	false,	//	ySecondCEP
	true,  // P(UEA)
	true,  // mTTC
	true,  // mPET
};
