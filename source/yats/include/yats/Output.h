#pragma once

#include <yats/Input.h>


class Output
{
public:

	Output& operator>>(Input)
	{
		return *this;
	}
};


