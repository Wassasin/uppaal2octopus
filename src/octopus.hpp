#pragma once

#include <ostream>

namespace uppaal2octopus
{
	class octopus
	{
	public:
		enum class indicator_e
		{
			start,
			end
		};
	
		struct event_t
		{
			std::string jobId;
			uint32_t pageNumber;
			std::string scenario, resource;
			uint32_t eventId;
			indicator_e startEnd;
			uint32_t timeStamp;
			std::string label;
		};
	};
}

namespace std
{
	inline std::string to_string(uppaal2octopus::octopus::indicator_e e)
	{
		typedef uppaal2octopus::octopus::indicator_e type;
	
		switch(e)
		{
		case type::start:
			return "start";
		case type::end:
			return "end";
		}
	}
}

namespace uppaal2octopus
{
	inline std::ostream& operator<<(std::ostream& o, const octopus::event_t& rhs)
	{
		const static char sep = '\t';
		
		o	<< rhs.jobId << sep
			<< rhs.pageNumber << sep
			<< rhs.scenario << sep
			<< rhs.resource << sep
			<< rhs.eventId << sep
			<< std::to_string(rhs.startEnd) << sep
			<< rhs.timeStamp << sep
			<< '"' << rhs.label << '"';
		
		return o;
	}
}
