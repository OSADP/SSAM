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
#ifndef UTILITY_H
#define UTILITY_H
#include "INCLUDE.h"
#include <stdexcept>

#define ORIG_FORMAT_VERSION 1.04

namespace SSAMPoint
{
/** Point class defines operators 
*/
class point 
{
public:
	float x;
	float y;
	float z;

	~point(){}
	point(float ix = 0.0, float iy = 0.0, float iz = 0.0)
		: x(ix)
		, y(iy)
		, z(iz)
	{
	}

	point operator+(const point& rhs) const
	{
		return point(x+rhs.x, y+rhs.y, z+rhs.z);
	}

	point operator+(float addition) const
	{
		return point(x+addition, y+addition, z+addition);
	}

	point& operator+=(const point& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		return *this;
	}

	point operator-(const point& rhs) const
	{
		return point(x-rhs.x, y-rhs.y, z-rhs.z);
	}

	point& operator-=(const point& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		return *this;
	}

	point operator*(float scale) const
	{
		return point(x*scale, y*scale, z*scale);
	}

	point operator/(float scale) const
	{
		if (scale > DBL_EPSILON)
		{
			return point(x/scale, y/scale, z/scale);
		} else
		{
			return point();
		}
	}

	bool operator==(const point& rhs)
	{
		return (abs(x - rhs.x) <= DBL_EPSILON
			&& abs(y - rhs.y) <= DBL_EPSILON
			&& abs(z - rhs.z) <= DBL_EPSILON);
	}

	bool operator!=(const point& rhs)
	{
		return (abs(x - rhs.x) > DBL_EPSILON
			|| abs(y - rhs.y) > DBL_EPSILON
			|| abs(z - rhs.z) > DBL_EPSILON);
	}
	
	float norm(void) const
	{
		return sqrt(x*x + y*y + z*z);
	}
};
};


/** Get a string for a float with proper rounding.
	* @param x A float to process.
*/
std::string GetFloatString(float x);

/** Find the intersection point of two line segments.
  * @param p0 the first point of first line segment
  * @param p1 the second point of first line segment
  * @param p2 the first point of second line segment
  * @param p3 the second point of second line segment
  * @param[out] ip the intesecting point if exists
  * @return a flag, true if the intersecting point exists 
*/
bool GetLineIntersection(const SSAMPoint::point& p0, const SSAMPoint::point& p1,
						 const SSAMPoint::point& p2, const SSAMPoint::point& p3,
						 SSAMPoint::point& ip);

/** Check whether two lines intersect, and return the coordinate of intersecting point at request if exists
  * @param p0_x x coordinate of the first point of first line segment
  * @param p0_y y coordinate of the first point of first line segment
  * @param p1_x x coordinate of the second point of first line segment
  * @param p1_y y coordinate of the second point of first line segment
  * @param p2_x x coordinate of the first point of second line segment
  * @param p2_y y coordinate of the first point of second line segment
  * @param p3_x x coordinate of the second point of second line segment
  * @param p3_y y coordinate of the second point of second line segment
  * @param[out] x x coordinate of the intesecting point at request if exists
  * @param[out] y y coordinate of the intesecting point at request if exists
  * @return a flag, true if the two lines intersect 
*/
bool CheckLinesIntersect(float p0_x, float p0_y, float p1_x, float p1_y, 
	float p2_x, float p2_y, float p3_x, float p3_y,
	float* x = NULL, float* y = NULL);

#endif // UTILITY_H