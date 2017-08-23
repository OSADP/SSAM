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
#include "MotionPrediction.h"
using namespace SSAMPoint;

extern bool GetLineIntersection(const point& p0, const point& p1,
						 const point& p2, const point& p3,
						 point& ip);

namespace MotPredNameSpace
{
void PredTraj::GetNextPos(const point& pos, 
					NormAngle& spdOrien,
					point& nextPos,
					NormAngle& nextSpdOrien)
{
	nextSpdOrien = spdOrien + GetControl();
	if (nextSpdOrien.m_Norm > m_MaxSpd)
		nextSpdOrien.m_Norm = m_MaxSpd;

	nextPos = pos + nextSpdOrien.getPoint();
}
 
point PredTraj::GetPos(int nSteps)
{
	if (m_PredPoses.empty())
		return point();

	if (nSteps >= m_PredPoses.size())
	{
		GetPos(nSteps - 1);
		m_PredPoses.push_back(point());
		m_PredSpdOriens.push_back(NormAngle());
		
		GetNextPos(m_PredPoses[nSteps - 1], 
			m_PredSpdOriens[nSteps-1],
			m_PredPoses[nSteps], 
			m_PredSpdOriens[nSteps]);
	}
	return m_PredPoses[nSteps];
}

bool PredMethod::DetectCollision(SP_PredTraj pTrj1, SP_PredTraj pTrj2, 
		float collisionThreshold, int nSteps,
		int& t, point& pos1, point& pos2)
{
	t = 1;
	pos1 = pTrj1->GetPos(t);
	pos2 = pTrj2->GetPos(t);
	bool isCollision = ((pos1 - pos2).norm() <= collisionThreshold);
	while (t < nSteps && !isCollision)
	{
		t += 1;
		pos1 = pTrj1->GetPos(t);
		pos2 = pTrj2->GetPos(t);
		isCollision = ((pos1 - pos2).norm() <= collisionThreshold);
	}
	return isCollision;
}

bool PredMethod::DetectCrossingZone(SP_PredTraj pTrj1, SP_PredTraj pTrj2, 
		float collisionThreshold, int nSteps,
		double& pet)
{
	int t1 = 0;
	bool isCrossingZone = false;
	while (!isCrossingZone && t1 < nSteps)
	{
		int t2 = 0;
		point ip;
		while (!isCrossingZone && t2 < nSteps)
		{
			point p11 = pTrj1->GetPos(t1);
			point p12 = pTrj1->GetPos(t1+1);
			point p21 = pTrj2->GetPos(t2);
			point p22 = pTrj2->GetPos(t2+1);
			isCrossingZone = GetLineIntersection(p11, p12, p21, p22, ip);
			if (isCrossingZone)
			{
				float deltaV = (p11 - p12 - p21 + p22).norm();
				pet = abs(double(abs(t1 - t2)) - (collisionThreshold / deltaV));
			}
			t2 += 1;
		}
		t1 += 1;
	}
	return isCrossingZone;
}

void NormalAdaption::GenPredTrajs(const PredObj& obj,
		std::vector<SP_PredTraj>& predTrajs)
{
	predTrajs.clear();
	PredTrajFactory PTF;
	for (int i = 0; i < m_nPredTrajs; ++i)
	{
		SP_PredTraj pt = PTF.CreatePredTraj(NORMALADAPTION, obj, m_MaxSpeed); 
		((PredTrajRandom*)(pt.get()))->SetDistributions(m_AccelDistri, m_SteerDistri);
		predTrajs.push_back(pt);
	}
}

void NormalAdaption::CalcMTTCMPET(const PredObj& obj1, 
		const PredObj& obj2, 
		double collisionThreshold, 
		int nSteps,
		float& mTTC, float& mPET)
{
	int nTTCs = 0;
	int nPETs = 0;
	float sumTTC = 0;
	float sumPET = 0;
	
	int nCollisions = 0;
	std::vector<SP_PredTraj> predTrajs1, predTrajs2;
	GenPredTrajs(obj1, predTrajs1);
	GenPredTrajs(obj2, predTrajs2);

	int t = 0;
	point pos1, pos2;
	for (std::vector<SP_PredTraj>::iterator it1 = predTrajs1.begin();
		it1 != predTrajs1.end(); ++it1)
	{
		for (std::vector<SP_PredTraj>::iterator it2 = predTrajs2.begin();
			it2 != predTrajs2.end(); ++it2)
		{
			SP_PredTraj pTrj1 = *it1;
			SP_PredTraj pTrj2 = *it2;
			bool isCollision = DetectCollision(pTrj1, pTrj2, 
				collisionThreshold, nSteps,
				t, pos1, pos2);
			if (isCollision)
			{
				sumTTC += double(t);
				nTTCs++;
			} else
			{
				double pet = 0;
				if (DetectCrossingZone(pTrj1, pTrj2, collisionThreshold, nSteps,pet))
				{
					sumPET += pet;
					nPETs++;
				}
			}
		}
	}

	if (nTTCs != 0 )
		mTTC = sumTTC/float(nTTCs)/10.0;

	if (nPETs != 0 )
		mPET = sumPET/float(nPETs)/10.0;
}

void EvasiveAction::GenPredTrajs(const PredObj& obj,
		std::vector<SP_PredTraj>& predTrajs)
{
	predTrajs.clear();
	PredTrajFactory PTF;
	for (int i = 0; i < m_nPredTrajs; ++i)
	{
		SP_PredTraj pt = PTF.CreatePredTraj(EVASIVEACTION, obj, m_MaxSpeed); 
		((PredTrajConstant*)(pt.get()))->SetControl(NormAngle(m_AccelDistri(), m_SteerDistri()));
		predTrajs.push_back(pt);
	}
}

float EvasiveAction::CalcPUEA(const PredObj& obj1, 
		const PredObj& obj2, 
		double collisionThreshold, 
		int nSteps)
{
	int nCollisions = 0;
	std::vector<SP_PredTraj> predTrajs1, predTrajs2;
	GenPredTrajs(obj1, predTrajs1);
	GenPredTrajs(obj2, predTrajs2);

	int t = 0;
	point pos1, pos2;
	for (std::vector<SP_PredTraj>::iterator it1 = predTrajs1.begin();
		it1 != predTrajs1.end(); ++it1)
	{
		for (std::vector<SP_PredTraj>::iterator it2 = predTrajs2.begin();
			it2 != predTrajs2.end(); ++it2)
		{
			SP_PredTraj pTrj1 = *it1;
			SP_PredTraj pTrj2 = *it2;
			bool isCollision = DetectCollision(pTrj1, pTrj2, 
				collisionThreshold, nSteps,
				t, pos1, pos2);
			if (isCollision)
			{
				nCollisions += 1;
			} 
		}
	}
  
	int nSamples = predTrajs1.size() * predTrajs2.size();
	return float(nCollisions) / float(nSamples);
}

SP_PredTraj PredTrajFactory::CreatePredTraj(PredMethod::METHOD_TYPE mType,
		const PredObj& initObj,
		double maxSpeed)
{
	switch (mType)
	{
	case PredMethod::NORMALADAPTION:
		return std::make_shared<PredTrajRandom>(initObj, maxSpeed);
		break;
	case PredMethod::EVASIVEACTION:
		return std::make_shared<PredTrajConstant>(initObj, maxSpeed);
		break;
	default:
		return NULL;
		break;
	};
}

};