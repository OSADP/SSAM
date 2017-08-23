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
#include "Event.h"
#include "Conflict.h"
#include <cmath>

const float Event::INVALID_SSM_VALUE = 99.0;

Event::Event(const InitEventParams& params)
	: tMinTTC ( -1)
	, TTC ( INVALID_SSM_VALUE)
	, PET ( INVALID_SSM_VALUE)
	, MaxS(0)
	, DR ( INVALID_SSM_VALUE)	
	, MaxD ( INVALID_SSM_VALUE)
	, FirstVID (-1)
	, SecondVID (-1)
	, PUEA (1.0)
	, mTTC(INVALID_SSM_VALUE)
	, mPET(INVALID_SSM_VALUE)
	, m_LastTTCIdx (-1)
	, m_LastPETIdx (-1)
	, m_IsActive ( true)		
	, m_IsConflict ( false)
	, m_IsPETComplete ( false)	
	, m_IsCalculatePUEA (params.m_IsCalcPUEA)
	, m_MaxTTC( params.m_MaxTTC)
	, m_MaxPET( params. m_MaxPET)
	, m_RearEndAngle (params.m_RearEndAngleThreshold)
	, m_CrossingAngle (params.m_CrossingAngleThreshold)
	, m_NSteps (params.m_NSteps)
	, m_CollisionThreshold (params.m_CollisionThreshold)
	, m_pNormalAdaption(params.m_pNormalAdaption)
	, m_pEvasiveAction(params.m_pEvasiveAction)
{
	int id1 = params.m_V1->GetVehicleID();
	int id2 = params.m_V2->GetVehicleID();
	if(id1 == id2)
		throw SSAMException("Event cannot be instantiated with two vehicles with the same ID.");		
	if(id1 < id2)
	{
		m_LowVID = id1;
		m_HighVID = id2;
	} 
	else
	{
		m_LowVID = id2;
		m_HighVID = id1;
	}
		
	AddVehicleData(params.m_V1, params.m_V2);
				
	m_FirstTTC = m_PreTimeStep;
	m_IsActive = true;

	m_StepSize = 0.1;
	m_TotalSteps = m_MaxTTC / m_StepSize + 1;
}

void Event::AddVehicleData(SP_Vehicle v1, SP_Vehicle v2)
{
	if(v1->GetTimeStep() != v2->GetTimeStep()) 
		throw SSAMException("Event will not accept two vehicles from different time steps.");

	int size = m_LowVData.size();
	if(size > 0)
	{
		SP_Vehicle vLast = m_LowVData[size - 1];
		if(v1->GetTimeStep() <= vLast->GetTimeStep()) 
			throw SSAMException("New event data is not in chronolical order: (new timestep data) " + std::to_string(v1->GetTimeStep()) + " <= " + std::to_string(vLast->GetTimeStep()) + " (old timestep data)");
	}
		
	if(v1->GetVehicleID() < v2->GetVehicleID())
	{
		if(m_LowVID != v1->GetVehicleID() || m_HighVID != v2->GetVehicleID())
			throw SSAMException ("Tried to add incompatible data to an Event object.");
		m_LowVData.push_back(v1);
		m_HighVData.push_back(v2);
	}
	else
	{
		if(m_LowVID != v2->GetVehicleID() || m_HighVID != v1->GetVehicleID())
			throw SSAMException ("Tried to add incompatible data to an Event object.");
		m_LowVData.push_back(v2);
		m_HighVData.push_back(v1);
	}
		
	m_PreTimeStep = v1->GetTimeStep();
}

bool Event::AnalyzeData(float tCurrent) 
{			
	int iLast = m_LowVData.size()-1;
	if (iLast < 0 || m_LowVData.size() != m_HighVData.size())
		return false;

	SP_Vehicle vLo = m_LowVData[iLast];
	SP_Vehicle vHi = m_HighVData[iLast];
	while(m_PreTimeStep < tCurrent)
	{
		vLo = vLo->GetNext();
		vHi = vHi->GetNext();
		
		if(vLo == NULL || vHi == NULL)
		{
			m_IsActive = false;
			m_IsConflict = false;
			return false;
		}
		
		AddVehicleData(vLo, vHi);
		iLast++;
	}
		
	if(m_IsActive)
	{
		SP_Vehicle vLoProj;
		SP_Vehicle vHiProj;
		bool isCollision = false;
		float stepTTC = INVALID_SSM_VALUE;
		for(float ttc = m_MaxTTC; ttc > -0.01; ttc -= m_StepSize)
		{
			if(ttc < 0)
				ttc = 0;
					
			vLoProj = vLo->CalcProjection(ttc, m_MaxPET);
			vHiProj = vHi->CalcProjection(ttc, m_MaxPET);
			if(vLoProj->IsCollided(vHiProj))
			{
				isCollision = true;
				stepTTC = ttc;
				m_LastTTC = tCurrent;
				m_LastTTCIdx = iLast;	
			}
			else if(isCollision)
			{
				break;
			}
		}
		if(isCollision)
		{
			if(stepTTC < TTC)
			{
				TTC = stepTTC;
				tMinTTC = tCurrent;
			}
		}
		else
		{
			m_IsActive = false;
		}
		
		if(isCollision)
		{
			float speedLo = vLo->GetSpeed();
			float speedHi = vHi->GetSpeed();
			if(speedLo > MaxS)
				MaxS = speedLo;
			if(speedHi > MaxS)
				MaxS = speedHi;
		}
	}

	if(!m_IsPETComplete && (PET == INVALID_SSM_VALUE || SecondVID == m_HighVID))
	{
		SP_Vehicle vLoPrev;
		for(int i = std::max(0, m_LastPETIdx + 1); i <= iLast && i <= m_LastTTCIdx; i++)
		{
			vLoPrev = m_LowVData[i];
			if(vHi->IsCollided(vLoPrev))
			{
				float pet = vHi->GetTimeStep() - vLoPrev->GetTimeStep();
				if(pet < 0)
					pet = 0;
				if(pet < PET)
				{
					PET = pet;

					xMinPET = vLoPrev->GetCenterX();
					yMinPET = vLoPrev->GetCenterY();
					zMinPET = vLoPrev->GetCenterZ();
						
					if(PET < 0.01)
						m_IsPETComplete = true;
					FirstVID = m_LowVID;
					SecondVID = m_HighVID;
					m_LastPETIdx = i;					
				}
				
				if(m_FirstPET <= 0)
					m_FirstPET = tCurrent;
				m_LastPET = tCurrent;
			}
		}
	}
	if(!m_IsPETComplete && (PET == INVALID_SSM_VALUE || SecondVID == m_LowVID))
	{
		SP_Vehicle vHiPrev;
		for(int i = std::max(0, m_LastPETIdx + 1); i <= iLast && i <= m_LastTTCIdx; i++)
		{
			vHiPrev = m_HighVData[i];
			if(vLo->IsCollided(vHiPrev))
			{
				float pet = vLo->GetTimeStep() - vHiPrev->GetTimeStep();
				if(pet < 0)
					pet = 0;
				if(pet < PET)
				{
					PET = pet;
					
					xMinPET = vHiPrev->GetCenterX();
					yMinPET = vHiPrev->GetCenterY();
					zMinPET = vHiPrev->GetCenterZ();
						
					if(PET < 0.01)
						m_IsPETComplete = true;
					FirstVID = m_HighVID;
					SecondVID = m_LowVID;
					m_LastPETIdx = i;					
				}
				if(m_FirstPET <= 0)
					m_FirstPET = tCurrent;
				m_LastPET = tCurrent;
			}
		}
	}
	if(!m_IsActive)
	{
		if	(	(m_IsPETComplete)
			||	(tCurrent - m_LastTTC >= m_MaxPET)		 
			|| 	(!m_IsActive && m_LastPETIdx >= m_LastTTCIdx) 
			)
		{
			m_IsPETComplete = true;
			if(PET < m_MaxPET)
			{
				m_IsConflict = true;
				CalcMeasures();
			}
			return false;				
		}
	}
	return true;
}

void Event::CalcMeasures()
{
	if(SecondVID >= 0)
	{
		std::vector<SP_Vehicle>* secPos = &m_LowVData;
		if(SecondVID == m_HighVID)
			secPos = &m_HighVData;
		std::vector<SP_Vehicle>::iterator i = secPos->begin();
		float AR;
		float minAR = INVALID_SSM_VALUE;
		for(; i != secPos->end(); ++i)
		{
			SP_Vehicle secVeh = *i;
			if(secVeh->GetTimeStep() > m_LastPET)
				break;
			AR = secVeh->GetAcceleration();
			if(AR < 0 && DR == INVALID_SSM_VALUE)
				DR = AR;
			if(AR < minAR)
				minAR = AR;
		}
		if(DR == INVALID_SSM_VALUE)
			DR = minAR;
		MaxD = minAR;
	}
		
	std::vector<SP_Vehicle>* firstPos = &m_LowVData;
	std::vector<SP_Vehicle>* secPos   = &m_HighVData;
	if(FirstVID == m_HighVID)
	{
		firstPos = &m_HighVData;
		secPos   = &m_LowVData;
	}
	std::vector<SP_Vehicle>::iterator fit = firstPos->begin();
	std::vector<SP_Vehicle>::iterator sit = secPos->begin();
	SP_Vehicle v1st = NULL;
	SP_Vehicle v2nd = NULL;
	float t = 0;
	float m1 = 1;	//	Surrogate mass measure for the first vehicle
	float m2 = 1;	//	Surrogate mass measure for the second vehicle
	int finalFirstLink = 0;
	int finalFirstLane = 0;
	int finalSecondLink = 0;
	int finalSecondLane = 0;
		
	for(; fit != firstPos->end() && sit!= secPos->end(); ++fit, ++sit)
	{
		v1st = *fit;
		v2nd = *sit;
		t = v1st->GetTimeStep();
		if(t == m_FirstTTC)
		{
			xFirstCSP = v1st->GetCenterX();
			yFirstCSP = v1st->GetCenterY();
			FirstLink = v1st->GetLinkID();
			FirstLane = v1st->GetLaneID();
			xSecondCSP = v2nd->GetCenterX();
			ySecondCSP = v2nd->GetCenterY();
			SecondLink = v2nd->GetLinkID();
			SecondLane = v2nd->GetLaneID();
			FirstVMinTTC = v1st->GetSpeed();
			SecondVMinTTC = v2nd->GetSpeed();
			FirstLength = v1st->GetLength();
			FirstWidth = v1st->GetWidth();
			SecondLength = v2nd->GetLength();
			SecondWidth = v2nd->GetWidth();
			m1 = FirstLength*FirstWidth;
			m2 = SecondLength*SecondWidth;
		}
		
		if(t == m_LastPET)
		{
			finalFirstLink = v1st->GetLinkID();
			finalFirstLane = v1st->GetLaneID();
			finalSecondLink = v2nd->GetLinkID();
			finalSecondLane = v2nd->GetLaneID();
				
			xFirstCEP = v1st->GetCenterX();
			yFirstCEP = v1st->GetCenterY();
			xSecondCEP = v2nd->GetCenterX();
			ySecondCEP = v2nd->GetCenterY();
			break;
		}	
	}

	float deltaYFirst = yFirstCEP - yFirstCSP;
	float deltaXFirst = xFirstCEP - xFirstCSP;
	if(deltaXFirst == 0 && deltaYFirst == 0 && &v1st != NULL)
	{
		deltaYFirst = v1st->GetFrontY() - v1st->GetRearY(); 
		deltaXFirst = v1st->GetFrontX() - v1st->GetRearX();
	}			
	FirstHeading = atan2(deltaYFirst, deltaXFirst);
	FirstHeading = FirstHeading*180.0/M_PI;
	if(FirstHeading < 0)
		FirstHeading += 360.0;

	float deltaYSecond = ySecondCEP - ySecondCSP;
	float deltaXSecond = xSecondCEP - xSecondCSP;
	if(deltaXSecond == 0 && deltaYSecond == 0 && &v2nd != NULL)
	{
		deltaYSecond = v2nd->GetFrontY() - v2nd->GetRearY(); 
		deltaXSecond = v2nd->GetFrontX() - v2nd->GetRearX();
	}
	SecondHeading = atan2(deltaYSecond, deltaXSecond);
	SecondHeading = SecondHeading*180.0/M_PI;
	if(SecondHeading < 0)
		SecondHeading += 360;
		
	ConflictAngle = SecondHeading - FirstHeading;
	if(ConflictAngle > 180)
		ConflictAngle -= 360;
	else if(ConflictAngle < -180)
		ConflictAngle += 360;
	float absAngle = abs(ConflictAngle);
	
	if	(	FirstLink == 0 ||  SecondLink == 0
		||	finalFirstLink == 0 ||  finalSecondLink == 0 )		
	{
		if(absAngle < (float)m_RearEndAngle)
			ConflictType = Conflict::REAR_END;
		else if(absAngle > (float)m_CrossingAngle)	
			ConflictType = Conflict::CROSSING;
		else
			ConflictType = Conflict::LANE_CHANGE;
	}
	else 	
	{
		if	(	(FirstLink == SecondLink) &&	(FirstLane == SecondLane) )
		{
			if	(	((FirstLink == finalFirstLink) && (FirstLane == finalFirstLane))
				&&	((SecondLink == finalSecondLink) && (SecondLane == finalSecondLane)) )
			{
				ConflictType = Conflict::REAR_END;
			}	
			else if	(	((FirstLink == finalFirstLink) && (FirstLane != finalFirstLane))
					||	((SecondLink == finalSecondLink) && (SecondLane != finalSecondLane)) )
			{
				ConflictType = Conflict::LANE_CHANGE;
			}
			else 
			{
				if(absAngle < (float)m_RearEndAngle)
					ConflictType = Conflict::REAR_END;
				else
					ConflictType = Conflict::LANE_CHANGE;						
			}
		}
		else if	(	((finalFirstLink == finalSecondLink) &&	(finalFirstLane == finalSecondLane))
				&& 	(((FirstLink == finalFirstLink) && (FirstLane != finalFirstLane)) || ((SecondLink == finalSecondLink) && (SecondLane != finalSecondLane))) )
		{
			ConflictType = Conflict::LANE_CHANGE;						
		}	
		else 
		{
			if(absAngle < m_RearEndAngle)
				ConflictType = Conflict::REAR_END;
			else if(absAngle > m_CrossingAngle)
				ConflictType = Conflict::CROSSING;
			else
				ConflictType = Conflict::LANE_CHANGE;
		}
	}
		
	float clockAngle = 6.0 - ConflictAngle/30.0;
	if(0 <= clockAngle && clockAngle < 1)
		clockAngle += 12;
	int hours = floor(clockAngle);
	int minutes = 60.0*(clockAngle - float(hours))+0.5;
	if(minutes > 59)
		minutes = 59;
	std::string minutesStr(std::to_string(minutes));
	if (minutesStr.length() < 2)
		minutesStr = "0"+minutesStr;
	ClockAngleString = std::to_string(hours) + ":" + minutesStr;
		
	float norm1 = (float) sqrt(deltaXFirst*deltaXFirst + deltaYFirst*deltaYFirst);
	float norm2 = (float) sqrt(deltaXSecond*deltaXSecond + deltaYSecond*deltaYSecond);
	float vx1 = 0; // value by casting first vehicle's velocity on x-axis   
	float vy1 = 0; // value by casting first vehicle's velocity on y-axis
	if(norm1 > 0)
	{
		vx1 = FirstVMinTTC*deltaXFirst/norm1;
		vy1 = FirstVMinTTC*deltaYFirst/norm1;
	}
	float vx2 = 0;
	float vy2 = 0;
	if(norm2 > 0)
	{
		vx2 = SecondVMinTTC*deltaXSecond/norm2;
		vy2 = SecondVMinTTC*deltaYSecond/norm2;
	}
	float pcx = 0;
	if((m1 + m2) > 0)
		pcx = (m1*vx1 + m2*vx2)/(m1 + m2);
	float pcy = 0;
	if((m1 + m2) > 0)
		pcy = (m1*vy1 + m2*vy2)/(m1 + m2);
	PostCrashV = sqrt(pcx*pcx + pcy*pcy);
	PostCrashHeading = atan2(pcy, pcx);
	PostCrashHeading = PostCrashHeading*180.0/M_PI;
	if(PostCrashHeading < 0)
		PostCrashHeading += 360;
		
	float dx1 = pcx - vx1;
	float dy1 = pcy - vy1;
	FirstDeltaV = sqrt(dx1*dx1 + dy1*dy1);
	float dx2 = pcx - vx2;
	float dy2 = pcy - vy2;
	SecondDeltaV = sqrt(dx2*dx2 + dy2*dy2);
	MaxDeltaV = std::max(FirstDeltaV, SecondDeltaV);
		
	float dxS = vx1 - vx2;
	float dyS = vy1 - vy2;
	DeltaS = (float)sqrt(dxS*dxS + dyS*dyS);

	if (m_IsCalculatePUEA) 
	{
		using namespace SSAMPoint;
		using namespace MotPredNameSpace;
		PredObj obj1 = {point(xFirstCSP, yFirstCSP), point(vx1, vy1)};
		PredObj obj2 = {point(xSecondCSP, ySecondCSP), point(vx2, vy2)};
		
		m_pNormalAdaption->CalcMTTCMPET(obj1, obj2, 
			m_CollisionThreshold,
			m_TotalSteps,
			mTTC, mPET);

		PUEA = m_pEvasiveAction->CalcPUEA(obj1,
			obj2,
			m_CollisionThreshold, 
			m_TotalSteps);
	}		
}
