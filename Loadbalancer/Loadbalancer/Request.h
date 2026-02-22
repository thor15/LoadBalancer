#pragma once
#include "pch.h"

enum JobType
{
	S=1,
	P=2
};

struct Request
{
	std::string requestIP;
	std::string destinationIP;
	int duration;
	JobType jobType;
};