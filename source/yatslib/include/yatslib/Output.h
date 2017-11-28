#pragma once

#include <yatslib/Input.h>


class Output
{
public:

	Output& operator>>(Input)
	{
		return *this;
	}
};


