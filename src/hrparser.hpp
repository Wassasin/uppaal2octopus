#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "concepts.hpp"

namespace uppaal2octopus
{
	class hrparser
	{
	public:
		typedef std::function<void(const location_t loc, const clock_t clock, const startend_e startEnd)> callback_t;
	
	private:
		struct state_t
		{
			std::vector<location_t> locations;
			clock_t clock;
		};
		
		struct transition_t
		{
			location_t from, to;
		};
	
		std::ifstream is;
		std::string buffer;
		
		hrparser(const std::string file)
		: is(file)
		, buffer()
		{}
		
		hrparser(hrparser&) = delete;
		void operator=(hrparser&) = delete;
		
		bool match(const std::string x);
		void consume();
		bool try_consume();
		
		state_t read_state();
		std::vector<transition_t> read_transition();
		
	public:
		static void parse(const std::string file, const callback_t& f);
	};
}
