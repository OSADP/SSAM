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
#ifndef VEHICLE_H
#define VEHICLE_H
#include "Utility.h"

#ifdef SSAMDLL_EXPORTS
#define SSAMFUNCSDLL_API __declspec(dllexport) 
#else
#define SSAMFUNCSDLL_API __declspec(dllimport) 
#endif

// Forward declaration of Vehicle class
class Vehicle;
/** Smart pointer type to Vehicle class.
*/
typedef std::shared_ptr<Vehicle> SP_Vehicle;


/** Vehicle manages data of a vehicle in one time step
*/
class Vehicle
{
public:
	/** VEHICLE_CORNER enumerates the four corners of vehicle shape
	*/
	enum VEHICLE_CORNER
	{
		FRONT_LEFT,
		FRONT_RIGHT,
		REAR_RIGHT,
		REAR_LEFT
	};

	Vehicle() 
		: m_TimeStep (-1)
		, m_VehicleID (0)
		, m_LinkID (0) 
		, m_LaneID (0)
		, m_Length (0)
		, m_Width (0)
		, m_Speed (0)
		, m_Acceleration (0)
		, m_FrontX (0)
		, m_FrontY (0)
		, m_FrontZ (0)
		, m_RearX (0)
		, m_RearY (0)
		, m_RearZ (0)
		, m_Scale (1.0)
		, m_ScaledLength (0)
		, m_ScaledWidth (0)
		, m_MinX (0)
		, m_MinY (0)
		, m_MaxX (0)
		, m_MaxY (0)
		, m_pNext (NULL)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_CornerX[i] = 0;
			m_CornerY[i] = 0;
		}
	}

	/** Copy contructor from another Vehicle object
	* @param rhs Const reference to a Vehicle object from which the values are copied
	*/
	Vehicle(const Vehicle& rhs)
	{
		CopyValues(rhs);
	}

	/** Overloaded assignment operator
	  * @param rhs Const reference to a Vehicle object from which the values are copied
	*/
	Vehicle& operator=(const Vehicle& rhs)
	{
		if (this != &rhs)
			CopyValues(rhs);
		return *this;
	}

	~Vehicle() {}
	
	/** Set vehicle position from coordinates of front and rear bumpers
	* @param frontX X coordinate of the middle front bumper of the vehicle
	* @param frontY Y coordinate of the middle front bumper of the vehicle
	* @param rearX X coordinate of the middle rear bumper of the vehicle
	* @param rearY Y coordinate of the middle rear bumper of the vehicle
	*/
	SSAMFUNCSDLL_API void SetPosition(float frontX, float frontY, float rearX, float rearY);

	/** Calculate vehicle projection position in maxTTC.
	* @param maxTTC maxTTC threshold
	* @param maxPET maxPET threshold
	*/
	SP_Vehicle CalcProjection(float maxTTC, float maxPET);
	
	/** Check whether the input vehicle intersects with this vehicle.
	* @param v A vehicle for collision check
	*/
	bool IsCollided(SP_Vehicle v);

	/** Print current vehicle info.
	* @param output the output stream
	* @param version the version of TRJ format
	*/
	void Print(std::ostream& output, float version);

	//	Set()/Get() methods
	void SetTimeStep(float t) { m_TimeStep = t; }
	void SetVehicleID(int i) { m_VehicleID = i; }
	void SetLinkID(int i) { m_LinkID = i; }
	void SetLaneID(char b) { m_LaneID = b; }
	void SetFrontX(float i) { m_FrontX = i; }
	void SetFrontY(float i) { m_FrontY = i; }
	void SetFrontZ(float i) { m_FrontZ = i; }
	void SetRearX(float i) { m_RearX = i; }
	void SetRearY(float i) { m_RearY = i; }
	void SetRearZ(float i) { m_RearZ = i; }
	void setScale(float f) { m_Scale = f; }
	void SetSpeed(float f) { m_Speed = f; }
	void SetAcceleration(float f) { m_Acceleration = f; }
	void SetNext(SP_Vehicle v) { m_pNext = v; }
	void SetLength(float f)
	{
		m_Length = f; 
		if(m_Scale > 0) 
			m_ScaledLength = m_Length/m_Scale;
		else
			m_ScaledLength = 0;
	}
	void SetWidth(float f)
	{
		m_Width = f;
		if(m_Scale > 0) 
			m_ScaledWidth = m_Width/m_Scale;
		else
			m_ScaledWidth = 0;
	}

	float GetTimeStep() const { return m_TimeStep; }
	int GetVehicleID() const { return m_VehicleID; }
	int GetLinkID() const { return m_LinkID; }
	int GetLaneID() const { return m_LaneID; }
	float GetFrontX() const { return m_FrontX; }
	float GetFrontY() const { return m_FrontY; }
	float GetFrontZ() const { return m_FrontZ; }
	float GetRearX() const { return m_RearX; }
	float GetRearY() const { return m_RearY; }
	float GetRearZ() const { return m_RearZ; }
	float GetLength() const { return m_Length; }
	float GetWidth() const { return m_Width; }
	float GetSpeed() const { return m_Speed; }
	float GetAcceleration() const { return m_Acceleration; }
	SP_Vehicle GetNext() const { return m_pNext; }
	float GetCenterX(){return m_RearX + (m_FrontX-m_RearX)/2.0; }
	float GetCenterY(){return m_RearY + (m_FrontY-m_RearY)/2.0; }
	float GetCenterZ(){return m_RearZ + (m_FrontZ-m_RearZ)/2.0; }
	const float* GetCornerXs() const { return m_CornerX; }
	const float* GetCornerYs() const { return m_CornerY; }
	float GetMaxX() const {return m_MaxX;}
	float GetMaxY() const {return m_MaxY;}
	float GetMinX() const {return m_MinX;}
	float GetMinY() const {return m_MinY;}
private:
	float m_TimeStep; /*!< Seconds since the start of the simulation */
	int m_VehicleID; /*!< Unique identifier number of the vehicle */
	int m_LinkID; /*!< Unique identifier number of the link */
	char m_LaneID; /*!< Unique identifier number of the lane */
	float m_Length; /*!< Vehicle length (front to back) in Units (feet or meters) */
	float m_Width; /*!< Vehicle width (left to right) in Units (feet or meters) */
	float m_Speed; /*!< Instantaneous forward speed (Units/sec) */
	float m_Acceleration; /*!< Instantaneous forward acceleration (Units/sec2) */
	float m_FrontX; /*!< X coordinate of the middle front bumper of the vehicle */
	float m_FrontY; /*!< Y coordinate of the middle front bumper of the vehicle */
	float m_FrontZ; /*!< Z coordinate of the middle front bumper of the vehicle */
	float m_RearX; /*!< X coordinate of the middle rear bumper of the vehicle */
	float m_RearY; /*!< Y coordinate of the middle rear bumper of the vehicle */
	float m_RearZ; /*!< Z coordinate of the middle rear bumper of the vehicle */
	float m_Scale; /*!< Distance per unit of X or Y */
	float m_ScaledLength; /*!< Vehicle length (front to back) in X, Y Units */
	float m_ScaledWidth; /*!< Vehicle width (left to right) in X, Y Units */
	float m_CornerX[4]; /*!< X coordinates of the corners of the vehicle */
	float m_CornerY[4]; /*!< Y coordinates of the corners of the vehicle */
	float m_MinX; /*!< Left edge of the vehicle occupying area.*/
	float m_MinY; /*!< Bottom edge of the vehicle occupying area.*/
	float m_MaxX; /*!< Right edge of the vehicle occupying area.*/
	float m_MaxY; /*!< Top edge of the vehicle occupying area.*/
	SP_Vehicle m_pNext; /*!< Pointer to vehicle data of next time step */ 

	/** Copy values from another Vehicle object
	* @param rhs Const reference to a Vehicle object from which the values are copied
	*/
	void CopyValues(const Vehicle& rhs)
	{
		m_TimeStep	=	rhs.m_TimeStep	;
		m_VehicleID	=	rhs.m_VehicleID	;
		m_LinkID	=	rhs.m_LinkID	;
		m_LaneID	=	rhs.m_LaneID	;
		m_Length	=	rhs.m_Length	;
		m_Width		=	rhs.m_Width	;
		m_Speed		=	rhs.m_Speed	;
		m_Acceleration	=	rhs.m_Acceleration	;
		m_FrontX    =	rhs.m_FrontX	;
		m_FrontY    =	rhs.m_FrontY	;
		m_FrontZ    =	rhs.m_FrontZ	;
		m_RearX	    =	rhs.m_RearX	;
		m_RearY	    =	rhs.m_RearY	;
		m_RearZ	    =	rhs.m_RearZ	;
		m_Scale	    =	rhs.m_Scale	;
		m_ScaledLength	    =	rhs.m_ScaledLength	;
		m_ScaledWidth	    =	rhs.m_ScaledWidth	;
		m_MinX	    =	rhs.m_MinX	;
		m_MinY	    =	rhs.m_MinY	;
		m_MaxX	    =	rhs.m_MaxX	;
		m_MaxY	    =	rhs.m_MaxY	;

		for (int i = 0; i < 4; ++i)
		{
			m_CornerX[i] = rhs.m_CornerX[i];
			m_CornerY[i] = rhs.m_CornerY[i];
		}

		m_pNext = rhs.m_pNext;
	}
	
	/** Get the distance between the input vehicle and this vehicle
	* @param v A vehicle for calculating the distance
	*/
	float GetV2VDistance(Vehicle& v)
	{
		float x = GetCenterX() - v.GetCenterX();
		float y = GetCenterY() - v.GetCenterY();
		return sqrt(x*x + y*y);
	}

	/** Calculate the offset for a point perpendicular to the line between
	* point 1 and and point 2 with a specified distance from the line,
	* returned point is on the right side of direction P2->P1
	* @param x1 x value of starting point p1
	* @param y1 y value of starting point p1
	* @param x2 x value of ending point p2
	* @param y2 y value of ending point p2
	* @param dist Distance of the perpendicular offset
	* @param dx output x offset
	* @param dy output y offset
	*/
	void CalcPerpOffset(float x1, float y1, float x2, float y2, float dist,
		float &dx, float &dy);
};


#endif
