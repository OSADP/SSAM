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
#ifndef ZONEGRID_H
#define ZONEGRID_H

#include <map>
#include <vector>
#include "INCLUDE.h"
#include "Vehicle.h"
#include <fstream>

/** ZoneGrid is constructed to conver the entire rectangular analysis area
  * using the width and height from TRJ file.
*/
class ZoneGrid
{
public:
	
	ZoneGrid();

	/** Constructor creates the ZoneGrid from parameters
	 * @param xMin Left edge of grid
	 * @param yMin Bottom edge of grid
	 * @param xMax Right edge of grid
	 * @param yMax Top edge of grid
	 * @param size Dimension of a single zone in pixels
	 */
	ZoneGrid(int xMin, int yMin, int xMax, int yMax, int size);

	~ZoneGrid(){}

	/** Reset the ZoneGrid with parameters
	 * @param xMin Left edge of grid
	 * @param yMin Bottom edge of grid
	 * @param xMax Right edge of grid
	 * @param yMax Top edge of grid
	 * @param size Dimension of a single zone in pixels
	 */
	void ResetGrid(int xMin, int yMin, int xMax, int yMax, int size);

	/**
	 * Remove occupants in used zones.
	 */
	void ClearGrid()
	{
		for (std::vector<UsedZone>::iterator it = m_UsedZones.begin();
			it != m_UsedZones.end(); ++it)
		{
			m_Zones[it->first][it->second]->Clear();
		}
		m_UsedZones.clear();
	}

	/**
	 * Add the new vehicle to the zone grid and check whether there is
	 * any other vehicle it crashes with.  
	 * @param vNew New vehicle to add
	 * @param allCrashes A set of all other vehicles the new vehicle crashes with
	 */
	void AddVehicle(SP_Vehicle vNew, std::map<int, SP_Vehicle>& allCrashes);
	
private:
	/** Zone maintains a list of occupying vehicles.
	*/
	class Zone
	{
	public:
		/**
		 * Add a new occupying vehicle to the zone and checks whether it crashes 
		 * with other vehicles in the zone.
		 * @param vNew A pointer to a new vehicle.
		 * @param allCrashes A list of vehicles crashing with new vehicle.
		 */
		void AddVehicle(SP_Vehicle vNew, std::map<int, SP_Vehicle>& allCrashes);

		/** Remove all vehicles from this zone.
		*/
		void Clear()
		{
			m_Occupants.clear();	
		}
	private:	
		/** The list of vehicles occupying this zone
		*/
	    std::list<SP_Vehicle> m_Occupants;
	};

	/** Smart pointer type to Zone class.
	*/
	typedef std::shared_ptr<Zone> SP_Zone;
	typedef std::pair<int, int> UsedZone;

	int m_OrigMinX; /*!< Left edge of original grid */
	int m_OrigMinY; /*!< Bottom edge of original grid */
	int m_OrigMaxX; /*!< Right edge of original grid */
	int m_OrigMaxY; /*!< Top edge of original grid */
	int m_MinX;  /*!< Left edge of calculated grid */
	int m_MinY;  /*!< Bottom edge of calculated grid */
	int m_MaxX;  /*!< Bottom edge of calculated grid */
	int m_MaxY;  /*!< Bottom edge of calculated grid */
	int m_ZoneSize; /*!< Dimension of a single zone */
	int m_NXZones;  /*!< Number of zones along x-axis */
	int m_NYZones;	/*!< Number of zones along y-axis */
	int m_NZones;   /*!< Total number of zones */
	int m_NZonesUsed ; /*!< Total number of used zones */
	std::vector<std::vector<SP_Zone> > m_Zones; /*!< A matrix of zones */
	std::vector<UsedZone> m_UsedZones; /*!< A vector stores all used zones. */
	
};

/** Smart pointer type to ZoneGrid class.
*/
typedef std::shared_ptr<ZoneGrid> SP_ZoneGrid;

#endif
