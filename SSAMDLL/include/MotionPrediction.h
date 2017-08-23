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
#ifndef MOTIONPREDICTION_H
#define MOTIONPREDICTION_H
#include "Utility.h"
#include <vector>

/* Developed from paper
Mohamed, Mohamed, and Nicolas Saunier. 
"Motion prediction methods for surrogate safety analysis." 
Transportation Research Record: Journal of the Transportation Research Board 2386 (2013): 168-178.
*/

/** MotPredNameSpace organizes classes, structs and functions
  * for implementing motion prediction methods 
  * to calculate P(UEA), mTTC, mPET.
*/
namespace MotPredNameSpace
{
/** PredObj organizes vehicle position and velocity at one step.
*/
struct PredObj
{
	SSAMPoint::point pos; /*!< Vehicle position */
	SSAMPoint::point vel; /*!< Vehicle velocity*/
};

/** NormAngle represents a point in the form of norm and angle.
*/
class NormAngle
{
public:
	double m_Norm;
	double m_Angle;  // in radian

	NormAngle()
		: m_Norm(0)
		, m_Angle(0)
	{}
	~NormAngle(){}
	
	NormAngle(double norm, double angle)
		: m_Norm(norm)
		, m_Angle(angle)
	{
	}

	NormAngle(const SSAMPoint::point& p)
	{
		m_Norm = p.norm();
		if (m_Norm > 0)
			m_Angle = atan2(p.y, p.x);
		else
			m_Angle = 0.0;
	}

	NormAngle operator+(const NormAngle& rhs)
	{
		double newNorm = m_Norm + rhs.m_Norm;
		if (newNorm < 0)
			newNorm = 0;
		return NormAngle(newNorm, m_Angle + rhs.m_Angle);
	}

	SSAMPoint::point getPoint()
	{
		return SSAMPoint::point(m_Norm * cos(m_Angle), m_Norm * sin(m_Angle));
	}

	static NormAngle fromPoint(SSAMPoint::point p)
	{
        double norm = p.norm();
		double angle = 0;
        if (norm > 0)
            angle = atan2(p.y, p.x);
        
        return NormAngle(norm, angle);
	}
};

/** TriangularDistri generates trigngular distribution values 
*/
class TriangularDistri
{
public:
	TriangularDistri()
		: m_Low(0)
		, m_High(0)
		, m_Mode(0)
	{}

	TriangularDistri(double low, double high, double mode)
		: m_Low(low)
		, m_High(high)
		, m_Mode(mode)
	{}
	~TriangularDistri() {}

	double operator()()
	{
		double u = double(rand()) / double(RAND_MAX);
		
		double c = (m_Mode - m_Low) / (m_High - m_Low);
		if (u > c)
		{
			u = 1.0 - u;
			c = 1.0 - c;
			double tmp = m_High;
			m_High = m_Low;
			m_Low = tmp;
		}
		
		return m_Low + (m_High - m_Low) * sqrt(u * c);
	}

private:
	double m_Low;  
	double m_High;
	double m_Mode;
};

/** PredTraj represents one predicted vehicle trajectory
*/
class PredTraj
{
public:
	/** Constructor generates a vehicle trajectory
	  * @param initObj initial position and speed orientation of vehicle
	  * @param maxSpeed the maximum speed allowed in the roadway network
	*/
	PredTraj(const PredObj& initObj,
		double maxSpeed = 100)
		: m_MaxSpd (maxSpeed)
	{
		m_PredPoses.push_back(initObj.pos);
		m_PredSpdOriens.push_back(NormAngle::fromPoint(initObj.vel));
	}

	PredTraj(){}
	~PredTraj(){}

	/** Calculate position at a target step
	  * @param nSteps the target step
	  * @return the position at the target step
	*/
	SSAMPoint::point GetPos(int nSteps);

	/** Get a set of acceleration rate and steering angle
	*/
	virtual NormAngle GetControl() = 0;
protected:
	std::vector<SSAMPoint::point> m_PredPoses; /*!< the array of all predicted positions */
	std::vector<NormAngle> m_PredSpdOriens; /*!< the array of all predicted speeds and orientations */

	/** Calculate the next position
	  * @param pos the current position.
	  * @param spdOrien the current speed and orientation
	  * @param[out] nextPos the next position
	  * @param[out] nextSpdOrien the speed and orientation at next position 
	*/
	void GetNextPos(const SSAMPoint::point& pos, 
					NormAngle& spdOrien,
					SSAMPoint::point& nextPos,
					NormAngle& nextSpdOrien);
private:
	double m_MaxSpd; /*!< the maximum speed allowed in the roadway network */
};
/** Smart pointer type to PredTraj class.
*/
typedef std::shared_ptr<PredTraj> SP_PredTraj;


/** PredTrajRandom represents one vehicle trajectory 
  * predicted with random control at each step.
*/
class PredTrajRandom: public PredTraj
{
public:
	/** Constructor genarates a random vehicle trajectory
	  * @param initObj initial vehicle position and velocity
	  * @param maxSpeed the maximum speed allowed in the roadway network
	*/
	PredTrajRandom(const PredObj& initObj,
		double maxSpeed)
		: PredTraj(initObj, maxSpeed)
	{	}

	~PredTrajRandom(){}

	/** Set triangular distributions for generating acceleration rate and steering angle at each step
	  * @param accelDistri a triangular distribution for generating acceleration rate
	  * @param steerDistri a triangular distribution for generating steering angle
	*/
	void SetDistributions(const TriangularDistri& accelDistri, 
		const TriangularDistri& steerDistri)
	{
		m_AccelDistri = accelDistri;
		m_SteerDistri = steerDistri;
	}

	/** Get a set of acceleration rate and steering angle
	*/
	virtual NormAngle GetControl()
	{
		return NormAngle(m_AccelDistri(), m_SteerDistri());
	}
private:
	TriangularDistri m_AccelDistri;  /*!< a triangular distribution for generating acceleration rate */
	TriangularDistri m_SteerDistri;  /*!< a triangular distribution for generating steering angle */
};

/** Smart pointer type to PredTrajRandom class.
*/
typedef std::shared_ptr<PredTrajRandom> SP_PredTrajRandom;

/** PredTrajConstant represents one vehicle trajectory 
  * predicted with constant control at each step.
*/
class PredTrajConstant : public PredTraj
{
public:
	/** Constructor genarates a constant vehicle trajectory
	  * @param initObj initial vehicle position and velocity
	  * @param maxSpeed the maximum speed allowed in the roadway network
	*/
	PredTrajConstant(const PredObj& initObj,
		double maxSpeed)
		: PredTraj(initObj, maxSpeed)
		, m_Control( NormAngle(0, 0))
	{	}

	/** Set the constant vehicle trajectory
	  * @param icontrol the constant acceleration rate and steering angle
	*/
	void SetControl(const NormAngle& icontrol)
	{
		m_Control = icontrol;
	}

	/** Get the set of acceleration rate and steering angle
	*/
	virtual NormAngle GetControl()
	{
		return m_Control;
	}
private:
	NormAngle m_Control; /*!< the acceleration rate and steering angle */
};

/** Smart pointer type to PredTrajConstant class.
*/
typedef std::shared_ptr<PredTrajConstant> SP_PredTrajConstant;

/** PredMethod defines a motion prediction method.
*/
class PredMethod
{
public:
	enum METHOD_TYPE
	{
		NORMALADAPTION,
		EVASIVEACTION,
	};

	PredMethod(){}
	~PredMethod(){}

	/** Constructor genarates a motion prediction method
	  * @param mType motion prediction method type: normal adaption or evasive action
	  * @param name method name
	  * @param nPredTrajs number of trajectories to predict
	  * @param maxSpeed the maximum speed allowed in the roadway network
	  * @param maxSteering max of triangular distribution for steering angle: radians/s
	  * @param maxAccRate max of triangular distribution for acceleration rate: ft/s^2
	  * @param minAccRate min of triangular distribution for acceleration rate: ft/s^2
	*/
	PredMethod(METHOD_TYPE mType,
		const std::string& name,
		int nPredTrajs,
		double maxSpeed,
		double  maxSteering,
		double maxAccRate,
		double minAccRate) 
		: m_Type(mType)
		, m_Name(name)
		, m_nPredTrajs(nPredTrajs)
		, m_MaxSpeed(maxSpeed)
		, m_AccelDistri(minAccRate, maxAccRate, 0)
		, m_SteerDistri((-1.0)*maxSteering, maxSteering, 0)
	{
	}

	
protected:
	int m_nPredTrajs; /*!< number of trajectories to predict */
	double m_MaxSpeed; /*!< maximum speed allowed in the roadway network */
	TriangularDistri m_AccelDistri;  /*!< a triangular distribution for generating acceleration rate */
	TriangularDistri m_SteerDistri;  /*!< a triangular distribution for generating steering angle */

	/** Generate a set of trajectories
	  * @param obj initial vehicle position and velocity 
	  * @param[out] predTrajs the set of generated trajectories
	*/
	virtual void GenPredTrajs(const PredObj& obj,
		std::vector<SP_PredTraj>& predTrajs) = 0;

	/** Detect collision between two vehicles
	  * @param pTrj1 smart pointer to the trajectory of first vehicle
	  * @param pTrj2 smart pointer to the trajectory of second vehicle
	  * @param collisionThreshold a distance threshold to determine whether two vehicles collide
	  * @param nSteps number of steps to detect
	  * @param[out] t the step when the collision is detected
	  * @param[out] p1 position of the first vehicle when the collision is detected 
	  * @param[out] p2 position of the second vehicle when the collision is detected
	  * @return a flag to indicate whether a collision is detected
	*/
	bool DetectCollision(SP_PredTraj pTrj1, SP_PredTraj pTrj2, 
		float collisionThreshold, int nSteps,
		int& t, SSAMPoint::point& p1, SSAMPoint::point& p2);

	/** Detect the crossing zone between two vehicles
	  * @param pTrj1 smart pointer to the trajectory of first vehicle
	  * @param pTrj2 smart pointer to the trajectory of second vehicle
	  * @param collisionThreshold a distance threshold to determine whether two vehicles collide
	  * @param nSteps number of steps to detect
	  * @param[out] pet PET if crossing zone is detected
	  * @return a flag to indicate whether a crossing zone is detected
	*/
	bool DetectCrossingZone(SP_PredTraj pTrj1, SP_PredTraj pTrj2, 
		float collisionThreshold, int nSteps,
		double& pet);

private:
	METHOD_TYPE m_Type; /*!< motion prediction method type: normal adaption or evasive action */
	std::string m_Name; /*!< name of the motion prediction method */
	
};

/** NormalAdaption defines the normal adaption motion prediction method
  * used for calculating mTTC and mPET.
*/
class NormalAdaption : public PredMethod
{
public:
	NormalAdaption(){}
	~NormalAdaption(){}
	/** Constructor genarates a normal adaption motion prediction method
	  * @param nPredTrajs number of trajectories to predict
	  * @param maxSpeed the maximum speed allowed in the roadway network
	  * @param maxSteering max of triangular distribution for steering angle: radians/s
	  * @param maxAccRate max of triangular distribution for acceleration rate: ft/s^2
	*/
	NormalAdaption(int nPredTrajs, 
		double maxSpeed, 
		double maxSteering,
		double maxAccRate) 
		: PredMethod(NORMALADAPTION, "normal adaptation", 
		nPredTrajs, maxSpeed, maxSteering, 
		maxAccRate, (-1.0)*maxAccRate)
	{
	}

	/** Generate a set of trajectories
	  * @param obj initial vehicle position and velocity 
	  * @param[out] predTrajs the set of generated trajectories
	*/
	virtual void GenPredTrajs(const PredObj& obj,
		std::vector<SP_PredTraj>& predTrajs);

	/** Calculate mTTC and mPET
	  * @param obj1 initial position and velocity of first vehicle
	  * @param obj2 initial position and velocity of second vehicle
	  * @param collisionThreshold a distance threshold to determine whether two vehicles collide
	  * @param nSteps number of steps to detect
	  * @param[out] mTTC the calculated mTTC
	  * @param[out] mPET the calculated mPET
	*/
	void CalcMTTCMPET(const PredObj& obj1, 
		const PredObj& obj2, 
		double collisionThreshold, 
		int nSteps,
		float& mTTC, float& mPET);	
};

/** Smart pointer type to NormalAdaption class.
*/
typedef std::shared_ptr<NormalAdaption> SP_NormalAdaption;

/** EvasiveAction defines the evasive action motion prediction method
  * used for calculating P(UEA).
*/
class EvasiveAction : public PredMethod
{
public:
	EvasiveAction(){}
	~EvasiveAction(){}

	/** Constructor genarates an evasive action motion prediction method
	  * @param nPredTrajs number of trajectories to predict
	  * @param maxSpeed the maximum speed allowed in the roadway network
	  * @param maxSteering max of triangular distribution for steering angle: radians/s
	  * @param maxAccRate max of triangular distribution for acceleration rate: ft/s^2
	  * @param minAccRate min of triangular distribution for acceleration rate: ft/s^2
	*/
	EvasiveAction(int nPredTrajs,
		double maxSpeed, 
		double  maxSteering, 
		double maxAccRate,
		double minAccRate) 
		: PredMethod(EVASIVEACTION, "evasive action", 
		nPredTrajs, maxSpeed, maxSteering, 
		maxAccRate, minAccRate)
	{
	}

	/** Generate a set of trajectories
	  * @param obj initial vehicle position and velocity 
	  * @param[out] predTrajs the set of generated trajectories
	*/
	virtual void GenPredTrajs(const PredObj& obj,
		std::vector<SP_PredTraj>& predTrajs);
	
	/** Calculate P(UEA)
	  * @param obj1 initial position and velocity of first vehicle
	  * @param obj2 initial position and velocity of second vehicle
	  * @param collisionThreshold a distance threshold to determine whether two vehicles collide
	  * @param nSteps number of steps to detect
	  * @return the calcualted P(UEA)
	*/
	float CalcPUEA(const PredObj& obj1, 
		const PredObj& obj2, 
		double collisionThreshold, 
		int nSteps);
};

/** Smart pointer type to EvasiveAction class.
*/
typedef std::shared_ptr<EvasiveAction> SP_EvasiveAction;

/** PredTrajFactory creates a smart pointer to PredTrajRandom or PredTrajConstant
  * accroding to motion prediction method type.
*/
class PredTrajFactory
{
public:
	/** Create smart pointer to PredTraj. The pointed object could be PredTrajRandom or PredTrajConstant
	  * accroding to motion prediction method type.
	  * @param mType motion prediction method type: normal adaption or evasive action
	  * @param initObj initial vehicle position and velocity 
	  * @param maxSpeed the maximum speed allowed in the roadway network
	  * @return a smart pointer to PredTrajRandom or PredTrajConstant accroding to motion prediction method type.
	*/
	SP_PredTraj CreatePredTraj(PredMethod::METHOD_TYPE mType,
		const PredObj& initObj,
		double maxSpeed);
};
};

#endif //MOTIONPREDICTION_H