#pragma once

#include <yatslib/yatslib_api.h>
#include <yatslib/Input.h>


class Output
{
public:

	Output& operator>>(Input)
	{
		return *this;
	}
};


