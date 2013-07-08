/*
   xtrparser.hpp for uppaal2octopus
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

#pragma once

#include <cstdio>
#include <climits>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <functional>

#include "octopus.hpp"

/* This xtrparser takes an UPPAAL model in the UPPAAL intermediate
 * format and a UPPAAL XTR trace file and returns this as a usable object.
 *
 * Notice that the intermediate format uses a global numbering of
 * clocks, variables, locations, etc. This is in contrast to the XTR
 * format, which makes a clear distinction between e.g. clocks and
 * variables and uses process local number of locations and
 * edges. Care must be taken to convert between these two numbering
 * schemes.
 */

namespace uppaal2octopus
{
	class xtrparser
	{
	public:
		typedef std::function<void(const octopus::event_t&)> callback_t;
	
	private:
		enum type_t { CONST, CLOCK, VAR, META, COST, LOCATION, FIXED };
		enum flags_t { NONE, COMMITTED, URGENT };

		/* Representation of a memory cell.
		 */
		struct cell_t
		{
			/** The type of the cell. */
			type_t type;

			/** Name of cell. Not all types have names. */
			std::string name;

			union
			{
				int value;
				struct
				{
					int nr;
				} clock;
				struct
				{
					int min;
					int max;
					int init;
					int nr;
				} var;
				struct
				{
					int min;
					int max;
					int init;
					int nr;
				} meta;
				struct
				{
					int flags;
					int process;
					int invariant;
				} location;
				struct
				{
					int min;
					int max;
				} fixed;
			};
		};

		/* Representation of a process.
		 */
		struct process_t
		{
			int initial;
			std::string name;
			std::vector<int> locations;
			std::vector<int> edges;
		};

		/* Representation of an edge.
		 */
		struct edge_t
		{
			int process;
			int source;
			int target;
			int guard;
			int sync;
			int update;
		};

		struct uppaalmodel_t
		{
			/* The UPPAAL model in intermediate format.
			 */
			std::vector<cell_t> layout;
			std::vector<int> instructions;
			std::vector<process_t> processes;
			std::vector<edge_t> edges;
			std::map<int,std::string> expressions;

			/* These are mappings from variable and clock indicies to
			 * the names of these variables and clocks.
			 */
			std::vector<std::string> clocks;
			std::vector<std::string> variables;
			
			uppaalmodel_t() = default;
			uppaalmodel_t(uppaalmodel_t&) = delete;
			uppaalmodel_t operator=(uppaalmodel_t&) = delete;
		};
		
		/* A bound for a clock constraint. A bound consists of a value and a
		 * bit indicating whether the bound is strict or not.
		 */
		struct bound_t
		{
			int value   : 31; // The value of the bound
			bool strict : 1;  // True if the bound is strict
		};
		
		/* Thrown by xtrparser upon parse errors.
		 */
		class invalid_format : public std::runtime_error
		{
		public:
			explicit invalid_format(const std::string& arg);
		};

		/* A symbolic state. A symbolic state consists of a location vector, a
		 * variable vector and a zone describing the possible values of the
		 * clocks in a symbolic manner.
		 */
		class State
		{
		public:
			State();
			State(const uppaalmodel_t& m, FILE *);

			int &getLocation(int i)
			{
				return locations[i];
			}
			int &getVariable(int i)
			{
				return integers[i];
			}
			bound_t &getConstraint(const uppaalmodel_t& m, int i, int j)
			{
				return dbm[i * m.clocks.size() + j];
			}
			int getLocation(int i) const
			{
				return locations[i];
			}
			int getVariable(int i) const
			{
				return integers[i];
			}
			bound_t getConstraint(const uppaalmodel_t& m, int i, int j) const
			{
				return dbm[i * m.clocks.size() + j];
			}
		private:
			std::vector<int> locations;
			std::vector<int> integers;
			std::vector<bound_t> dbm;
			void allocate(const uppaalmodel_t& m);
		};

		/* A transition consists of one or more edges. Edges are indexes from
		 * 0 in the order they appear in the input file.
		 */
		class Transition
		{
		public:
			Transition(const uppaalmodel_t& m, FILE *);

			int getEdge(int32_t process) const
			{
				return edges[process];
			}
		private:
			std::vector<int> edges;
		};
		
		// The bound (infinity, <).
		static constexpr const bound_t infinity = { INT_MAX >> 1, true };

		// The bound (0, <=).
		static constexpr const bound_t zero = { 0, false };
		
		// Reads one line from file. Skips comments.
		bool read(FILE *file, char *str, size_t n) const;

		// xtrparser for intermediate format.
		void loadIF(uppaalmodel_t& m, FILE *file) const;
		
		size_t findClock(const uppaalmodel_t& m, const std::string str) const;
		int getClock(const uppaalmodel_t& m, const State& s) const;
		
		void output(const uppaalmodel_t& m, const callback_t& f, uint32_t i, uint32_t p, int l, uint32_t clock, octopus::indicator_e startEnd) const;
		
		// Read and output a trace file.
		void loadTrace(const uppaalmodel_t& m, FILE *file, const callback_t& f) const;
		
		void workaround(uppaalmodel_t& m, int l) const;

	public:
		void parse(const std::string model, const std::string trace, const callback_t& f) const;
	};
}
