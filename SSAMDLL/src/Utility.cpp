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
#include "Utility.h"

using namespace SSAMPoint;

std::string GetFloatString(float x)
{
	double y = (x >= 0) ? float(int(x * 100.0 + 0.5)) / 100.0 : float(int(x * 100.0 - 0.5)) / 100.0;
	std::string str(std::to_string(y));
	std::size_t pos = str.find('.');
	if (pos != std::string::npos)
	{
		str = str.substr(0, pos + 3);
	}
	return str;
}

bool GetLineIntersection(const point& p0, const point& p1,
						 const point& p2, const point& p3,
						 point& ip)
{
	float p0_x = p0.x, p0_y = p0.y; 
	float p1_x = p1.x, p1_y = p1.y;
    float p2_x = p2.x, p2_y = p2.y;
	float p3_x = p3.x, p3_y = p3.y; 

	return CheckLinesIntersect(p0_x, p0_y, p1_x, p1_y, 
		p2_x, p2_y, p3_x, p3_y,
		&(ip.x), &(ip.y));
}

bool CheckLinesIntersect(float p0_x, float p0_y, float p1_x, float p1_y, 
	float p2_x, float p2_y, float p3_x, float p3_y,
	float* x, float* y)
{
	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		if (x != NULL && y != NULL)	
		{
			*x = p0_x + (t * s1_x);
			*y = p0_y + (t * s1_y);
		}
		return true;
	}

	return false; 
}