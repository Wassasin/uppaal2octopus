#include "hrparser.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace uppaal2octopus
{
	void inline error()
	{
		throw std::runtime_error("Failed to parse trace");
	}
	
	std::pair<std::string, std::string> inline split(const std::string& str)
	{
		const static boost::regex r("(.+)\\.(.+)");
		boost::smatch m;
		
		if(!boost::regex_match(str, m, r))
			error();
		
		return std::make_pair(m[1], m[2]);
	}

	bool hrparser::match(const std::string x)
	{
		if(x == buffer)
		{
			consume();
			return true;
		}
		
		return false;
	}
	
	void hrparser::consume()
	{
		if(!try_consume())
			error();
	}
	
	bool hrparser::try_consume()
	{
		return is >> buffer;
	}

	hrparser::state_t hrparser::read_state()
	{	
		state_t s;
		
		if(!match("State") || !match("("))
			error();
			
		while(buffer != ")")
		{
			s.locations.emplace_back(split(buffer));
			consume();
		}

		bool found_lower_clock = false, found_upper_clock = false;
		while(try_consume() && buffer != "Transitions:")
		{
			if(buffer.size() > 0 && buffer[buffer.size()-1] == ',')
				buffer.pop_back(); //Remove superfluous comma
		
			const static boost::regex r_clock("^([^-]+)([><]=)(.+)$");
			const static boost::regex r_dbm("^(.+)-(.+)([><]=)(.+)$");
			const static boost::regex r_var("^(.+)=(.+)$");
			
			boost::smatch m;
			if(boost::regex_match(buffer, m, r_clock))
			{
				if(m[1] == "c" && m[2] == ">=") // Take the lower bound, has precedence
				{
					s.clock = boost::lexical_cast<size_t>(m[3]);
					found_lower_clock = true;
				}
				else if(!found_lower_clock && m[1] == "c" && m[2] == "<=")
				{
					s.clock = boost::lexical_cast<size_t>(m[3]);
					found_upper_clock = true;
				}
			}
			else if(boost::regex_match(buffer, m, r_dbm))
				; // Do nothing
			else if(boost::regex_match(buffer, m, r_var))
				; // Do nothing
			else
				error();
		}
		
		if(!found_lower_clock && !found_upper_clock)
			throw std::runtime_error("Cannot find clock 'c' in State");
		
		return s;
	}
	
	void process_transition_meta(const std::string /*str*/, const size_t /*i*/)
	{
		/* Do nothing with transition meta information
		if(i == 0 && str != "1")
			std::cout << "[GUARD] " << str << std::endl;
		else if(i == 1 && str != "tau")
			std::cout << "[SYNC] " << str << std::endl;
		else if(i >= 2 && str != "1")
			std::cout << "[UPDATE] " << str << std::endl;
		*/
		
		return;
	}
	
	std::vector<hrparser::transition_t> hrparser::read_transition()
	{
		std::vector<transition_t> result;
	
		if(!match("Transitions:"))
			error();

		do
		{
			{
				const static boost::regex r("(.+)->(.+)");
				boost::smatch m;
			
				if(!boost::regex_match(buffer, m, r))
					error();
			
				const auto s1 = split(m[1]);
				const auto s2 = split(m[2]);
			
				result.push_back({s1, s2});
			}
			
			consume();
			if(!match("{"))
				error();
			
			{
				size_t i = 0; // 0 = guard, 1 = synchronizations, 2.. = updates
				std::string str;
				
				while(!match("}")) //A block ends with either a string with ',' at the end, or the string "}"
				{
					str.append(buffer);
					str.append(" ");
					consume();
					
					if(str[str.size()-2] == ',')
					{
						str.pop_back(); //Remove superfluous space
						str.pop_back(); //Remove superfluous comma
						
						process_transition_meta(str, i++);
						str.clear();
					}
				}
				
				if(str.size() > 0)
					str.pop_back(); //Remove superfluous space
				
				process_transition_meta(str, i++);
			}
			
		} while(buffer != "State");
		
		return result;
	}

	void hrparser::parse(const std::string file, const hrparser::callback_t& f)
	{
		hrparser p(file);
		p.consume();
		
		{
			const state_t s = p.read_state();
			for(const auto loc : s.locations)
				f(loc, s.clock, startend_e::start);
		}
		
		while(p.buffer == "Transitions:")
		{
			const std::vector<transition_t> ts = p.read_transition();
			const state_t s = p.read_state();
			
			for(const transition_t t : ts)
			{
				f(t.from, s.clock, startend_e::end);
				f(t.to, s.clock, startend_e::start);
			}
		}
	}
}
