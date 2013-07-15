#pragma once

#include <string>

namespace uppaal2octopus
{
	typedef uint32_t clock_t;
	
	typedef std::string process_t;
	typedef std::string location_name_t;
	
	typedef std::pair<process_t, location_name_t> location_t;
	
	enum class startend_e
	{
		start,
		end
	};
}
