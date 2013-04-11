#pragma once

#include <set>
#include <queue>
#include <algorithm>

namespace uppaal2octopus
{
	template<typename object_t>
	class path_finder
	{
	public:
		typedef std::pair<object_t, object_t> edge_t;
		typedef std::vector<edge_t> edges_t;
		
	private:
		path_finder() = delete;
		path_finder(path_finder&) = delete;
		void operator=(path_finder&) = delete;
	
		static std::vector<edge_t> reconstruct(const edges_t& h, const object_t start, const object_t end)
		{
			std::vector<edge_t> trace;
			
			object_t current = end;
			while(current != start)
				for(const edge_t& e : h)
					if(e.second == current)
					{
						current = e.first;
						trace.emplace_back(e);
					}
			
			std::reverse(trace.begin(), trace.end());
			return trace;
		}
	
	public:
		//Perform a breadth first search
		static std::vector<edge_t> search(const edges_t& graph, const object_t start, const object_t end)
		{
			std::queue<object_t> q; //queue
			std::set<object_t> m; //set of marked id_t's
			edges_t h;
			
			m.insert(start);
			q.emplace(start);
			
			while(!q.empty())
			{
				id_t t = q.front();
				q.pop();
				
				if(t == end)
					return reconstruct(h, start, end);
				
				for(const edge_t& e : graph)
				{
					if(e.first == t && m.find(e.second) == m.end())
					{
						m.insert(e.second);
						q.emplace(e.second);
						h.emplace_back(e);
					}
				}
			}
			
			throw std::runtime_error("No path from start to end");
		}
	};
}
