// SSAMFuncsDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SSAMFuncsDll.h"
#include <stdexcept>

using namespace std;

namespace SSAMFuncs
{
	double MySSAMFuncs::Add(double a, double b)
	{
		return a + b;
	}

	double MySSAMFuncs::Subtract(double a, double b)
	{
		return a - b;
	}

	double MySSAMFuncs::Multiply(double a, double b)
	{
		return a * b;
	}

	double MySSAMFuncs::Divide(double a, double b)
	{
		if (b == 0)
		{
			throw invalid_argument("b cannot be zero!");
		}

		return a / b;
	}
}

