#pragma once

#include <ostream>
#include "concepts.hpp"

namespace uppaal2octopus
{
	class octopus
	{
	public:
		struct event_t
		{
			std::string jobId;
			uint32_t pageNumber;
			std::string scenario, resource;
			uint32_t eventId;
			startend_e startEnd;
			uint32_t timeStamp;
			std::string label;
		};
	};
}

namespace std
{
	inline std::string to_string(uppaal2octopus::startend_e e)
	{
		typedef uppaal2octopus::startend_e type;
	
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
