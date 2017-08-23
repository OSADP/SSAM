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
#include <list>
#include <iostream>
#include "ZoneGrid.h"

ZoneGrid::ZoneGrid()
{
}
	
ZoneGrid::ZoneGrid(int xMin, int yMin, int xMax, int yMax, int size)
{			
	ResetGrid(xMin, yMin, xMax, yMax, size);
}

void ZoneGrid::AddVehicle(SP_Vehicle vNew, std::map<int, SP_Vehicle>& allCrashes)
{
	allCrashes.clear();
	
	if ((vNew->GetCenterX() < m_OrigMinX || vNew->GetCenterX() > m_OrigMaxX)|| 
		(vNew->GetCenterY() < m_OrigMinY || vNew->GetCenterY() > m_OrigMaxY)) 
	{
		return;
	}
	////----------------------------------------------------
		
	//	Get the axis-align bounding box, and
	//	translate it to the zoneGrid origin,
	double vxMin = vNew->GetMinX() - m_MinX;
	double vxMax = vNew->GetMaxX() - m_MinX;
	double vyMin = vNew->GetMinY() - m_MinY;
	double vyMax = vNew->GetMaxY() - m_MinY;
	//	calculate the range of x zones and y zones,
	int ixMin = (int) floor(vxMin/m_ZoneSize);
	int ixMax = (int) floor(vxMax/m_ZoneSize);
	int iyMin = (int) floor(vyMin/m_ZoneSize);
	int iyMax = (int) floor(vyMax/m_ZoneSize);
	//	narrow the zone indices such that zones off the
	//	grid are not considered
	ixMin = std::max(ixMin,0);
	ixMax = std::min(ixMax,m_NXZones-1);
	iyMin = std::max(iyMin,0);
	iyMax = std::min(iyMax,m_NYZones-1);
	//	and add the vehicle to each zone.
#ifdef _OPENMP_LOCAL
	#pragma omp critical (ADDZONE)
#endif
	for(int ix = ixMin; ix <= ixMax; ix++)
	{
		for(int iy = iyMin; iy <= iyMax; iy++)
		{
			m_UsedZones.push_back(UsedZone(ix, iy));
			m_Zones[ix][iy]->AddVehicle(vNew, allCrashes);
		}
	}
}
	
void ZoneGrid::ResetGrid(int xMin, int yMin, int xMax, int yMax, int size)
{
	//store the original grid dimensions before expanding them
	m_OrigMinX = xMin;
	m_OrigMinY = yMin;
	m_OrigMaxX = xMax;
	m_OrigMaxY = yMax;
		
	//	Expand the grid dimensions to an integer number of zones
	//	where zones are referenced starting from (minX, minY).
	m_MinX 		= (int) floor((double)xMin/(double)size) * size;
	m_MinY 		= (int) floor((double)yMin/(double)size) * size;
	m_MaxX 		= (int) ceil((double)xMax/(double)size) * size;
	m_MaxY 		= (int) ceil((double)yMax/(double)size) * size;
	m_ZoneSize 	= size;
		
	if(m_MinX >= m_MaxX)
		throw SSAMException("Zone width is too small.  Dimensions of x-axis may be invalid.");
	if(m_MinY >= m_MaxY)
		throw SSAMException("Zone height is too small.  Dimensions of y-axis may be invalid.");
	if(m_ZoneSize < 1)
		throw SSAMException("Zone size used for analysis is too small.  Dimensions are scaled with too many feet/meters per unit X or Y.");
		
	m_NXZones = (m_MaxX - m_MinX)/m_ZoneSize; // should make sure > 0
	m_NYZones = (m_MaxY - m_MinY)/m_ZoneSize; // should make sure > 0
	m_NZones = m_NXZones*m_NYZones;
	if(m_NZones > 5575680)//about 500 square miles assuming zones of 50 feet.
		throw SSAMException("Safety analysis zone is too large.");

	m_Zones = std::vector<std::vector<SP_Zone> >(m_NXZones, std::vector<SP_Zone>(m_NYZones, NULL));
	for (int i = 0; i < m_NXZones; ++i)
	{
		for (int j = 0; j < m_NYZones; ++j)
		{
			m_Zones[i][j] = std::make_shared<Zone>();
		}
	}
	m_UsedZones.clear();
}

void ZoneGrid::Zone::AddVehicle(SP_Vehicle vNew, std::map<int, SP_Vehicle>& allCrashes)
{
	
	SP_Vehicle vOther;		
	std::list<SP_Vehicle>::iterator i = m_Occupants.begin();
	for(; i != m_Occupants.end(); ++i)
	{
		vOther = *i;
		if(vNew->IsCollided(vOther))
		{
			allCrashes[vOther->GetVehicleID()] = vOther;
		}
	}
	m_Occupants.push_back(vNew);
}