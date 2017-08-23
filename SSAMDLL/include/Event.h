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
#ifndef EVENT_H
#define EVENT_H
#include <list>
#include <string>
#include <vector>
#include "Vehicle.h"
#include "MotionPrediction.h"

/** InitParams organizes parameters for creating a conflict event.
*/
struct InitEventParams
{
	SP_Vehicle m_V1; /*!< First Vehicle in the conflict event*/
	SP_Vehicle m_V2; /*!< Second Vehicle in the conflict event*/
	float m_MaxTTC; /*!< Max TTC threshold*/
	float m_MaxPET; /*!< Max PET threshold*/
	int m_RearEndAngleThreshold; /*!< Rear-end Angle Threshold*/
	int m_CrossingAngleThreshold; /*!< Crossing Angle Threshold*/
	bool m_IsCalcPUEA; /*!< Flag to indicate whether to calculate P(UEA), mTTC, mPET*/
	int m_NSteps;
	double m_CollisionThreshold; /*!< a distance threshold to determine whether two vehicles collide*/
	MotPredNameSpace::SP_NormalAdaption m_pNormalAdaption;
	MotPredNameSpace::SP_EvasiveAction m_pEvasiveAction;
};

/** Event maintains the continueous vehicle data for the pair of vehicles
  * involved in one conflict event, and calculates safety measures when the conflict is confirmed.
  * Safety measure variables are defined same as the safety measures defined in document: 
  * Surrogate Safety Assessment Model and Validation: Final Report, 
  * Publication No. FHWA-HRT-08-051, JUNE 2008,
  * CHAPTER 2. SSAM SOFTWARE, TERMS AND DEFINITIONS,
  * Page 20-25.
*/
class Event
{
public:
	Event(){}
	~Event(){}

	/** Create a new conflict event using the initial parameters.
	* @param params Initial parameters.
	*/
	Event(const InitEventParams& params); 

	const static float INVALID_SSM_VALUE;

	/**Add new vehicle data.
	 * @param v1 First vehicle.
	 * @param v2 Second vehicle.
	 */
	//--------------------------------------------------------------------------
	void AddVehicleData(SP_Vehicle v1, SP_Vehicle v2); 
	
	/**Analyze event data up this time step.
	 * @param t This time step for analysis.
	 */
	bool AnalyzeData(float t); 

	//	Get()/Set() methods
	float	GetMinTTCTime()	{ return tMinTTC; }
	float	GetXMinPET()	{ return xMinPET; }
	float	GetYMinPET()	{ return yMinPET; }
	float	GetZMinPET()	{ return zMinPET; }
	float 	GetTTC()		{ return TTC; }
	float	GetPET()		{ return PET; }
	float	GetMaxS()		{ return MaxS; }
	float	GetDeltaS()		{ return DeltaS; }
	float	GetDR()			{ return DR; }
	float	GetMaxD()		{ return MaxD; }
	float	GetMaxDeltaV()			{ return MaxDeltaV;	}
	float	GetConflictAngle() 		{ return ConflictAngle; }
	std::string   GetClockAngleString()	{ return ClockAngleString;	}
	int		GetConflictType() 		{ return ConflictType; }
	float	GetPostCrashV()			{ return PostCrashV; }
	float	GetPostCrashHeading()	{ return PostCrashHeading; }
	int		GetFirstVID()	{ return (FirstVID >= 0)? FirstVID : m_LowVID; }
	int		GetFirstLink() 			{ return FirstLink; }
	int		GetFirstLane() 			{ return FirstLane; }
	float	GetFirstLength()		{ return FirstLength; }
	float	GetFirstWidth()			{ return FirstWidth; }
	float	GetFirstHeading() 		{ return FirstHeading; }
	float	GetFirstVMinTTC()		{ return FirstVMinTTC; }
	float	GetFirstDeltaV()		{ return FirstDeltaV; }
	float	GetXFirstCSP()	{ return xFirstCSP; }
	float	GetYFirstCSP()	{ return yFirstCSP; }
	float	GetXFirstCEP()	{ return xFirstCEP; }
	float	GetYFirstCEP()	{ return yFirstCEP; }
	int		GetSecondVID()	{ return (SecondVID >= 0)? SecondVID : m_HighVID; }
	int		GetSecondLink() 		{ return SecondLink; }
	int		GetSecondLane() 		{ return SecondLane; }
	float	GetSecondLength()		{ return SecondLength; }
	float	GetSecondWidth()		{ return SecondWidth; }
	float	GetSecondHeading() 		{ return SecondHeading; }
	float	GetSecondVMinTTC()		{ return SecondVMinTTC; }
	float	GetSecondDeltaV()		{ return SecondDeltaV; }
	float	GetXSecondCSP()	{ return xSecondCSP; }
	float	GetYSecondCSP()	{ return ySecondCSP; }
	float	GetXSecondCEP()	{ return xSecondCEP; }
	float	GetYSecondCEP()	{ return ySecondCEP; }
	float	GetPUEA() {return PUEA;}
	float	GetMTTC() {return mTTC;}
	float	GetMPET() {return mPET;}
	bool 	IsConflict()	{ return m_IsConflict; }
private:
	// safety measure variables
	float tMinTTC;
	float xMinPET;
	float yMinPET;
	float zMinPET;
	float TTC;
	float PET;
	float MaxS; 	
	float DeltaS;
	float DR;	
	float MaxD;	
	float MaxDeltaV;	
	float ConflictAngle;
	std::string ClockAngleString;
	int   ConflictType;	
	float PostCrashV;		
	float PostCrashHeading;	
	int FirstVID;
	int FirstLink;	
	int FirstLane;	
	float FirstLength;
	float FirstWidth;
	float FirstHeading; 
	float FirstVMinTTC;	
	float FirstDeltaV;	
	float xFirstCSP; 
	float yFirstCSP; 
	float xFirstCEP; 
	float yFirstCEP; 
	int SecondVID;
	int SecondLink;	
	int SecondLane;	
	float SecondLength;	
	float SecondWidth;
	float SecondHeading;
	float SecondVMinTTC;
	float SecondDeltaV; 
	float xSecondCSP; 
	float ySecondCSP; 
	float xSecondCEP; 
	float ySecondCEP; 
	float PUEA; 
	float mTTC; 
	float mPET; 
	
	// member variables
	int m_LowVID;  /*!< Lower ID of the pair of vehicles*/
	int m_HighVID; /*!< Higher ID of the pair of vehicles*/
	std::vector<SP_Vehicle> m_LowVData;  /*!< Data of vehicle with lower ID*/
	std::vector<SP_Vehicle> m_HighVData; /*!< Data of vehicle with higher ID*/
	float m_MaxTTC; /*!< Max TTC threshold*/
	float m_MaxPET; /*!< Max PET threshold*/
	int m_RearEndAngle; /*!< Rear-end Angle Threshold*/
	int m_CrossingAngle; /*!< Crossing Angle Threshold*/
	float m_StepSize; /*!< Step size when decrementing maxTTC to find the exact TTC*/
	float m_FirstTTC; /*!< First time step when conflict of vehicles is detected*/
	float m_LastTTC; /*!< Last time step when vehicles in collision*/
	float m_PreTimeStep; /*!< Previous time step when vehicles in collision*/
	float m_FirstPET; /*!< First time step of PET interval*/
	float m_LastPET; /*!< Last time step of PET interval*/
	int m_LastTTCIdx;	/*!< Index in the Vehicle data array of the last TTC*/	
	int m_LastPETIdx; /*!< Index in the Vehicle data array of the last location of the first vehicle for which PET is available*/  
	bool m_IsActive; /*!< Flag of whether TTC values are still less than the threshold*/	
	bool m_IsConflict; 	/*!< Flag of whether current event is a conflict*/
	bool m_IsPETComplete;	/*!< Flag of whether PET calculations are complete */
	bool m_IsCalculatePUEA; /*!< Flag to calculate P(UEA), mTTC and mPET */

	// variables for calculating mTTC, mPET and P(UEA)
	MotPredNameSpace::SP_NormalAdaption m_pNormalAdaption;
	MotPredNameSpace::SP_EvasiveAction m_pEvasiveAction;
	int m_NSteps; /*!< Number of steps per second */
	int m_TotalSteps; /*!< Total number of steps to detect collision or crossing zone */
	double m_CollisionThreshold; /*!< a distance threshold to determine whether two vehicles collide*/
	
	/**Calculate safety measures.
	 */
	void CalcMeasures();
	
};

/** Smart pointer type to Event class.
*/
typedef std::shared_ptr<Event> SP_Event;

#endif
