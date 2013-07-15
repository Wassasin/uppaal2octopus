#include "converter.hpp"

#include <sstream>
#include <stdexcept>

namespace uppaal2octopus
{
	converter::location_id_t converter::get_location_id(const location_t l)
	{
		const auto l_id_i = location_ids.find(l);
		
		if(l_id_i != location_ids.end())
			return l_id_i->second;
		
		return location_ids[l] = next_location_id++;
	}
	
	void converter::output(const converter::event_t& e, clock_t end)
	{
		const event_id_t i = next_event_id++;
		const location_id_t loc_id = get_location_id(e.l);
	
		std::stringstream s;
		s << loc_id << ":" << e.l.first << "." << e.l.second; //Prepending with unique id makes ResVis happy
	
		f({
			s.str(), // Because UPPAAL does not have the concept of Jobs, we abuse this field to contain the stateId, alongside with a textual respresentation of the state
			static_cast<uint32_t>(loc_id), // No such thing as a pageNum, thus use locationId
			"UPPAALtrace",
			e.l.first,
			static_cast<uint32_t>(i), // Unique identifier for start/end pair
			startend_e::start,
			e.start,
			s.str()
		});
	
		f({
			s.str(),
			static_cast<uint32_t>(loc_id),
			"UPPAALtrace",
			e.l.first,
			static_cast<uint32_t>(i),
			startend_e::end,
			end,
			s.str()
		});
	}

	void converter::add(location_t loc, clock_t clock, startend_e startEnd)
	{
		const auto e_i = events.find(loc.first);
		if(e_i == events.end())
		{
			if(startEnd == startend_e::end)
				throw std::runtime_error("Received end-event without a corresponding start event");
			
			events[loc.first] = {loc, clock};
			return;
		}

		const event_t e = e_i->second;
		
		if(clock - e.start > 0 && loc.second.size() >= 1 && loc.second[0] != '_') // Only output events with a duration
		{
			output(e, clock);
			
			if(clock > last)
				last = clock;
		}
		
		events.erase(loc.first);
	}
	
	void converter::flush()
	{
		for(const auto ep : events)
		{
			const auto e = ep.second;
		
			if(last - e.start == 0 || (e.l.second.size() >= 1 && e.l.second[0] == '_'))
				continue;
			
			output(e, last);
		}
		
		events.clear();
	}
}
