#pragma once

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
