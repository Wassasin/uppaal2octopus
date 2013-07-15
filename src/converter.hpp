#pragma once

#include <functional>
#include <map>
#include <string>

#include "concepts.hpp"
#include "octopus.hpp"

namespace uppaal2octopus
{
	class converter
	{
	public:
		typedef std::function<void(const octopus::event_t&)> callback_t;
	
		typedef size_t event_id_t;
		typedef size_t location_id_t;

	private:
		struct event_t
		{
			location_t l;
			clock_t start;
		};
	
	private:
		callback_t f;
	
		clock_t last;
		event_id_t next_event_id;
		location_id_t next_location_id;
		
		std::map<process_t, event_t> events;
		std::map<location_t, location_id_t> location_ids;
		
		location_id_t get_location_id(const location_t l);
		
		void output(const event_t& e, clock_t end);
		
	public:
		converter(const callback_t& f);
		
		void add(location_t loc, clock_t clock, startend_e startEnd);
		void flush();
	};
}
