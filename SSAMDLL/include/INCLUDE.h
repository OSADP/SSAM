#pragma once
#ifndef INCLUDE_H
#define INCLUDE_H
#include <memory>
#include <string>
#include <map>
#include <vector>
/** SSAMException is thrown when any error occurs in SSAM analysis 
*/
class SSAMException : public std::runtime_error
{
public:
	/** Create a SSAMException
	* @param s Error message from std::runtime_error.
	*/
	explicit SSAMException(const std::string &s)
		: std::runtime_error(s) {}
};

#endif