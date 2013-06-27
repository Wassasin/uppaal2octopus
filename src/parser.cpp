/*
   parser.cpp for uppaal2octopus
   Copyright (C) 2013 Wouter Geraedts

   This file is copied and adapted from tracer.cpp in libutap.
   See http://people.cs.aau.dk/~adavid/utap/ for the original source.
   Copyright (C) 2006 Uppsala University and Aalborg University.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA
*/

#include "parser.hpp"

#include "path_finder.hpp"

#include <algorithm>
#include <sstream>
#include <boost/optional.hpp>

namespace uppaal2octopus
{
	parser::invalid_format::invalid_format(const std::string& arg) : runtime_error(arg)
	{}
	
	bool parser::read(FILE *file, char *str, size_t n) const
	{
		do
		{
			if(fgets(str, n, file) == NULL)
				return false;
		}
		while(str[0] == '#');
		return true;
	}
	
	void parser::loadIF(parser::uppaalmodel_t& m, FILE *file) const
	{
		char str[255];
		char section[16];
		char name[32];
		int index;

		while(fscanf(file, "%15s\n", section) == 1)
		{
			if(strcmp(section, "layout") == 0)
			{
				while(read(file, str, 255) && !isspace(str[0]))
				{
					char s[5];
					cell_t cell;

					if(sscanf(str, "%d:clock:%d:%31s", &index,
							&cell.clock.nr, name) == 3)
					{
						cell.type = CLOCK;
						cell.name = name;
						m.clocks.push_back(name);
					}
					else if(sscanf(str, "%d:const:%d", &index,
							&cell.value) == 2)
					{
						cell.type = CONST;
					}
					else if(sscanf(str, "%d:var:%d:%d:%d:%d:%31s", &index,
							&cell.var.min, &cell.var.max, &cell.var.init,
							&cell.var.nr, name) == 6)
					{
						cell.type = VAR;
						cell.name = name;
						m.variables.push_back(name);
					}
					else if(sscanf(str, "%d:meta:%d:%d:%d:%d:%31s", &index,
							&cell.meta.min, &cell.meta.max, &cell.meta.init,
							&cell.meta.nr, name) == 6)
					{
						cell.type = META;
						cell.name = name;
						m.variables.push_back(name);
					}
					else if(sscanf(str, "%d:location::%31s", &index, name) == 2)
					{
						cell.type = LOCATION;
						cell.location.flags = NONE;
						cell.name = name;
					}
					else if(sscanf(str, "%d:location:committed:%31s", &index, name) == 2)
					{
						cell.type = LOCATION;
						cell.location.flags = COMMITTED;
						cell.name = name;
					}
					else if(sscanf(str, "%d:location:urgent:%31s", &index, name) == 2)
					{
						cell.type = LOCATION;
						cell.location.flags = URGENT;
						cell.name = name;
					}
					else if(sscanf(str, "%d:static:%d:%d:%31s", &index,
							&cell.fixed.min, &cell.fixed.max,
							name) == 4)
					{
						cell.type = FIXED;
						cell.name = name;
					}
					else if(sscanf(str, "%d:%5s", &index, s) == 2
							&& strcmp(s, "cost") == 0)
					{
						cell.type = COST;
					}
					else
					{
						throw invalid_format(str);
					}

					m.layout.push_back(cell);
				}
			}
			else if(strcmp(section, "instructions") == 0)
			{
				while(read(file, str, 255) && !isspace(str[0]))
				{
					int address;
					int values[4];
					int cnt = sscanf(
							  str, "%d:%d%d%d%d", &address,
							  values + 0, values + 1, values + 2, values + 4);
					if(cnt < 2)
						throw invalid_format("In instruction section");

					for(int i = 0; i < cnt; i++)
						m.instructions.push_back(values[i]);
				}
			}
			else if(strcmp(section, "processes") == 0)
			{
				while(read(file, str, 255) && !isspace(str[0]))
				{
					process_t process;
					if(sscanf(str, "%d:%d:%31s", &index, &process.initial, name) != 3)
						throw invalid_format("In process section");
					process.name = name;
					m.processes.push_back(process);
				}
			}
			else if(strcmp(section, "locations") == 0)
			{
				while(read(file, str, 255) && !isspace(str[0]))
				{
					int index;
					int process;
					int invariant;

					if(sscanf(str, "%d:%d:%d", &index, &process, &invariant) != 3)
						throw invalid_format("In location section");

					m.layout[index].location.process = process;
					m.layout[index].location.invariant = invariant;
					m.processes[process].locations.push_back(index);
				}
			}
			else if(strcmp(section, "edges") == 0)
			{
				while(read(file, str, 255) && !isspace(str[0]))
				{
					edge_t edge;

					if(sscanf(str, "%d:%d:%d:%d:%d:%d", &edge.process,
							&edge.source, &edge.target,
							&edge.guard, &edge.sync, &edge.update) != 6)
						throw invalid_format("In edge section");

					m.processes[edge.process].edges.push_back(m.edges.size());
					m.edges.push_back(edge);
				}
			}
			else if(strcmp(section, "expressions") == 0)
			{
				while(read(file, str, 255) && !isspace(str[0]))
				{
					if(sscanf(str, "%d", &index) != 1)
						throw invalid_format("In expression section");

					/* Find expression string (after the third colon).
					 */
					char *s = str;
					int cnt = 3;
					while(cnt && *s)
					{
						cnt -= (*s == ':');
						s++;
					}
				
					if(cnt)
						throw invalid_format("In expression section");

					/* Trim white space.
					 */
					while(*s && isspace(*s))
						s++;

					char *t = s + strlen(s) - 1;
					while(t >= s && isspace(*t))
						t--;

					t[1] = '\0';
					m.expressions[index] = s;
				}
			}
			else
				throw invalid_format("Unknown section");
		}
	}

	parser::State::State(const uppaalmodel_t& m, FILE *file)
	{
		allocate(m);

		/* Read locations.
		 */
		for(size_t i = 0; i < m.processes.size(); i++)
			fscanf(file, "%d\n", &getLocation(i));

		fscanf(file, ".\n");

		/* Read DBM.
		 */
		int i, j, bnd;
		while(fscanf(file, "%d\n%d\n%d\n.\n", &i, &j, &bnd) == 3)
		{
			getConstraint(m, i, j).value = bnd >> 1;
			getConstraint(m, i, j).strict = bnd & 1;
		}
		fscanf(file, ".\n");

		/* Read integers.
		 */
		for(size_t i = 0; i < m.variables.size(); i++)
		{
			fscanf(file, "%d\n", &getVariable(i));
		}
		fscanf(file, ".\n");
	}

	void parser::State::allocate(const uppaalmodel_t& m)
	{
		const auto infinity_tmp = parser::infinity;
		const auto zero_tmp = parser::zero;
	
		/* Allocate.
		 */
		locations = std::vector<int>(m.processes.size());
		integers = std::vector<int>(m.variables.size());
		dbm = std::vector<bound_t>(m.clocks.size() * m.clocks.size(), infinity_tmp);
		
		/* Set diagonal and lower bounds to zero.
		 */
		for(size_t i = 0; i < m.clocks.size(); i++)
		{
		//	getConstraint(m, 0, i) = zero_tmp;
			getConstraint(m, i, i) = zero_tmp;
		}
	}
	
	parser::Transition::Transition(const uppaalmodel_t& m, FILE *file)
	{
		edges = std::vector<int>(m.processes.size(), -1);

		int process, edge;
		while(fscanf(file, "%d %d.\n", &process, &edge) == 2)
			edges[process] = edge - 1;

		fscanf(file, ".\n");
	}
	
	size_t parser::findClock(const parser::uppaalmodel_t& m, const std::string str) const
	{
		for(size_t i = 0; i < m.clocks.size(); i++)
			if(m.clocks[i] == str)
				return i;
		
		throw std::runtime_error(std::string("There is no clock with name ") + str);
	}
	
	int parser::getClock(const parser::uppaalmodel_t& m, const parser::State& s) const
	{
		/*
		 * Because a trace does not always contain "t(0)-c"-bdm's in
		 * every state, try to find a trace of clocks that contains the
		 * same information
		 */
		const size_t t0_i = findClock(m, "t(0)");
		const size_t c_i = findClock(m, "c");
		
		path_finder<size_t>::edges_t edges;
		
		for(size_t i = 0; i < m.clocks.size(); i++) 
			for(size_t j = 0; j < m.clocks.size(); j++) 
				if(i != j && s.getConstraint(m, i, j).value != infinity.value)
					edges.emplace_back(i, j);
		
		const auto trace = path_finder<size_t>::search(edges, t0_i, c_i);
		
		int result = 0;
		for(const auto& e : trace)
			result -= s.getConstraint(m, e.first, e.second).value;
		
		return result;
	}
	
	void parser::output(const parser::uppaalmodel_t& m, const parser::callback_t& f, uint32_t i, uint32_t p, int l, uint32_t clock, octopus::indicator_e startEnd) const
	{
		if(m.layout[l].name[0] == '_')
			return;
	
		std::stringstream s;
		s << l << ":" << m.processes.at(p).name << '.' << m.layout.at(l).name;
	
		//if(m.layout.at(l).type != type_t::LOCATION)
		//	throw std::runtime_error("Unexpected type");
	
		//std::cerr << m.expressions.at(m.layout.at(l).location.invariant) << std::endl;
		
		f({
			s.str(), // Because UPPAAL does not have the concept of Jobs, we abuse this field to contain the stateId, alongside with a textual respresentation of the state
			static_cast<uint32_t>(l), // No such thing as a pageNum
			"UPPAALtrace",
			m.processes.at(p).name,
			i,
			startEnd,
			clock,
			s.str()
		});
	}
	
	void parser::loadTrace(const parser::uppaalmodel_t& m, FILE *file, const parser::callback_t& f) const
	{
		//std::vector<uint32_t> eventIds(m.processes.size(), 0);
		uint32_t eventId = 0;
		std::vector<uint32_t> startClocks(m.processes.size(), 0);
		std::vector<boost::optional<int>> targets(m.processes.size(), boost::none);
		
		State state(m, file);
		uint32_t clock = static_cast<uint32_t>(getClock(m, state));
		
		//for(auto pair : m.expressions)
		//std::cerr << pair.second << std::endl;
		
		for(;;)
		{
			int c;
			
			// Skip white space.
			do
			{
				c = fgetc(file);
			}
			while(isspace(c));

			// A dot terminates the trace.
			if(c == '.')
				break;

			// Put the character back into the stream.
			std::ungetc(c, file);

			// Read a state and a transition.
			state = State(m, file);
			clock = static_cast<uint32_t>(getClock(m, state));
			
			Transition transition(m, file);

			//jobId, pageNumber, scenario, resource, eventId, startEnd, timeStamp, label
			
			for(uint32_t p = 0; p < m.processes.size(); p++)
			{
				const int idx = transition.getEdge(p);
				
				if(idx == -1)
					continue;
				
				const uint32_t edge = m.processes[p].edges[idx];
				
				targets[p] = m.edges[edge].target;
				
				if(clock - startClocks[p] > 0)
				{
					output(m, f, eventId, p, m.edges[edge].source, startClocks[p], octopus::indicator_e::start);
					output(m, f, eventId++, p, m.edges[edge].source, clock, octopus::indicator_e::end);
				}
				
				startClocks[p] = clock;
			}
		}
		
		// Output all end-states
		for(uint32_t p = 0; p < m.processes.size(); p++)
		{
			if(!targets[p])
				continue;
			
			if(clock - startClocks[p] == 0)
				continue;
			
			output(m, f, eventId, p, targets[p].get(), startClocks[p], octopus::indicator_e::start);
			output(m, f, eventId++, p, targets[p].get(), clock, octopus::indicator_e::end);
		}
	}
	
	void parser::parse(const std::string model, const std::string trace, const parser::callback_t& f) const
	{
		FILE *file;
		uppaalmodel_t m;
		
		try
		{
			file = fopen(model.c_str(), "r");
			if(file == NULL)
				exit(1);

			loadIF(m, file);
			fclose(file);

			file = fopen(trace.c_str(), "r");
			if(file == NULL)
				exit(1);

			loadTrace(m, file, f);
			fclose(file);
		}
		catch(std::exception &e)
		{
			std::cerr << "Catched exception: " << e.what() << std::endl;
		}
	}
}
