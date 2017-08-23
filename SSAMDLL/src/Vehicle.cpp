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
#include <cmath>
#include "Vehicle.h"

void Vehicle::SetPosition(float frontX, float frontY, float rearX, float rearY)
{
	m_FrontX = frontX;
	m_FrontY = frontY;
	m_RearX  = rearX;
	m_RearY  = rearY;
		
	float dist = m_ScaledWidth / 2.0;
	float dx=0, dy=0;
	CalcPerpOffset(frontX, frontY, rearX, rearY, dist, dx, dy);

	m_CornerX[FRONT_RIGHT] = m_FrontX + dx; 
	m_CornerY[FRONT_RIGHT] = m_FrontY + dy; 
	m_CornerX[FRONT_LEFT]  = m_FrontX - dx; 
	m_CornerY[FRONT_LEFT]  = m_FrontY - dy; 

	CalcPerpOffset(rearX, rearY, frontX, frontY, dist, dx, dy);
	m_CornerX[REAR_LEFT]   = m_RearX  + dx; 
	m_CornerY[REAR_LEFT]   = m_RearY  + dy; 
	m_CornerX[REAR_RIGHT]  = m_RearX  - dx; 
	m_CornerY[REAR_RIGHT]  = m_RearY  - dy; 
	
	m_MinX = m_CornerX[0];
	m_MaxX = m_CornerX[0];
	m_MinY = m_CornerY[0];
	m_MaxY = m_CornerY[0];
	for(int i = 1; i < 4; i++)
	{
		if(m_CornerX[i] < m_MinX)
			m_MinX = m_CornerX[i];
		if(m_CornerX[i] > m_MaxX)
			m_MaxX = m_CornerX[i];
		if(m_CornerY[i] < m_MinY)
			m_MinY = m_CornerY[i];
		if(m_CornerY[i] > m_MaxY)
			m_MaxY = m_CornerY[i];
	}		
}

SP_Vehicle Vehicle::CalcProjection(float maxTTC, float maxPET)
{
	float dist = maxTTC * m_Speed;
	
	if(m_Scale <= 0)
		throw SSAMException("Vehicle projection not possible, since scale is unspecified.");
	float fullDist = dist/m_Scale;
	float remnDist = fullDist;

	SP_Vehicle vehLast = std::make_shared<Vehicle>( *this);
	while(remnDist > 0)
	{
		SP_Vehicle vehNext = vehLast->GetNext();
		if(vehNext != NULL)
		{
			float stepDist = vehNext->GetV2VDistance(*vehLast);
			if(stepDist <= 0) 
			{
				return vehLast;
			}
			else if(remnDist > stepDist) 
			{
				vehLast = vehNext;
				remnDist -= stepDist;
			}
			else 
			{
				float halfLengthXY = m_ScaledLength/2.0;
				float rearDistScale = (remnDist - halfLengthXY)/stepDist;
				float frontDistScale = (remnDist + halfLengthXY)/stepDist;
				float lastCX = vehLast->GetCenterX();
				float lastCY = vehLast->GetCenterY();
				float deltaX = vehNext->GetCenterX() - lastCX;
				float deltaY = vehNext->GetCenterY() - lastCY;

				float projFX = lastCX + frontDistScale * deltaX;
				float projFY = lastCY + frontDistScale * deltaY;
				float projRX = lastCX + rearDistScale * deltaX;
				float projRY = lastCY + rearDistScale * deltaY;

				SP_Vehicle v = std::make_shared<Vehicle>(*this);
				v->SetPosition(projFX, projFY, projRX, projRY);
				return v;					
			}
		}
		else 
		{
			float projTime = vehLast->GetTimeStep() - GetTimeStep();
			if(projTime < maxPET)
			{
				float remTime = maxTTC - projTime;
				float speedX 	= m_Speed*(vehLast->GetFrontX() - vehLast->GetRearX())/m_ScaledLength;
				float speedY 	= m_Speed*(vehLast->GetFrontY() - vehLast->GetRearY())/m_ScaledLength;
				float projFX 	= vehLast->GetFrontX() + remTime * speedX;
				float projFY 	= vehLast->GetFrontY() + remTime * speedY;
				float projRX 	= vehLast->GetRearX()  + remTime * speedX;
				float projRY 	= vehLast->GetRearY()  + remTime * speedY;

				SP_Vehicle v = std::make_shared<Vehicle>(*this);
				v->SetPosition(projFX, projFY, projRX, projRY);
				return v;						
			}
			return vehLast;
		}
	}
	return vehLast;
}

bool Vehicle::IsCollided(SP_Vehicle v)
{
	// considered as at two levels if more than 5 ft apart in elevation
	if (abs(GetCenterZ() - v->GetCenterZ()) > 5.0) 
		return false;
	
	if	(	(m_MaxX < v->GetMinX())
		||	(m_MinX > v->GetMaxX())
		||	(m_MaxY < v->GetMinY())
		||	(m_MinY > v->GetMaxY())
		)
		return false;

	int i,j,iNext,jNext;
	for(i = 0; i < 4; i++)
	{
		iNext = (i+1)%4;
		for(j = 0; j < 4; j++)
		{
			jNext = (j+1)%4;
			if(CheckLinesIntersect(	m_CornerX[i], 
										m_CornerY[i], 
										m_CornerX[iNext], 
										m_CornerY[iNext],
										v->m_CornerX[j], 
										v->m_CornerY[j], 
										v->m_CornerX[jNext], 
										v->m_CornerY[jNext]))
				return true;
		}
	}
	return false;
}

void Vehicle::Print(std::ostream& output, float version)
{
	output << m_TimeStep << "," 
		<< m_VehicleID << ","
		<< m_LinkID << ","
		<< int(m_LaneID) << ","
		<< m_FrontX << ","
		<< m_FrontY << ","
		<< m_RearX << ","
		<< m_RearY << ","
		<< m_Length << ","
		<< m_Width << ","
		<< m_Speed << ","
		<< m_Acceleration << ",";
	if (version > ORIG_FORMAT_VERSION)
	{
		output << m_FrontZ << ","
			<< m_RearZ << ",";
	}
	output	<< std::endl;
}

void Vehicle::CalcPerpOffset(float x1, float y1, float x2, float y2, float dist,
		float &dx, float &dy)
{
	float slope = 0;
	if(x1 == x2 )
	{
		dx = dist;
		dy = 0;
	}
	else if (y1 == y2)
	{
		dx = 0;
		dy = dist;
	}
	else
	{
		slope = (y2-y1)/(x2-x1);
		dx = dist*sqrt(abs(1.0/(1.0+1.0/(slope*slope))));
		dy = abs(-1/slope*dx);
	}
	
	if(y2 > y1)
		dx *= -1;
	if(x2 < x1)
		dy *= -1;
}
