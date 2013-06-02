#pragma once

#include <boost/program_options.hpp>
#include "parser.hpp"

namespace uppaal2octopus
{
	class cli
	{
		cli() = delete;
		cli(cli&) = delete;
		void operator=(cli&) = delete;
	
	public:
	
		static int main(int argc, char** argv)
		{
			std::string model_file, trace_file;

			boost::program_options::options_description o_general("General options");
			o_general.add_options()
			("help,h", "display this message");
			
			boost::program_options::options_description o_hidden("Hidden options");
			o_hidden.add_options()
			("model", boost::program_options::value<decltype(model_file)>(&model_file), "path to model file in intermediate format")
			("trace", boost::program_options::value<decltype(trace_file)>(&trace_file), "path to trace file in xtr");

			boost::program_options::variables_map vm;
			boost::program_options::positional_options_description pos;
			pos.add("model", 1);
			pos.add("trace", 1);
			
			boost::program_options::options_description options("Allowed options");
			options.add(o_general).add(o_hidden);
	
			try
			{
				boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).positional(pos).run(), vm);
			} catch(boost::program_options::unknown_option &e)
			{
				std::cerr << "Unknown option --" << e.get_option_name() << ", see --help" << std::endl;
				return -1;
			}
			
			if(vm.count("help"))
			{
				std::cout
					<< "Program for converting UPPAAL traces to Octopus traces. [https://github.com/Wassasin/uppaal2octopus]" << std::endl
					<< "Usage: ./uppaal2octopus [options] <model> <trace>" << std::endl
					<< std::endl
					<< o_general;
				
				return 0;
			}
			
			try
			{
				boost::program_options::notify(vm);
			} catch(const boost::program_options::required_option &e)
			{
				std::cerr << "You forgot this: " << e.what() << std::endl;
				return -1;
			}
			
			if(model_file == "" || trace_file == "")
			{
				std::cerr << "Please specify both a model and a trace, see --help" << std::endl;
				return -1;
			}
			
			std::cerr
				<< "Model: " << model_file << std::endl
				<< "Trace: " << trace_file << std::endl;
			
			parser p;
			p.parse(model_file, trace_file, [&](const octopus::event_t& e) {
				std::cout << e << std::endl;
			});
			
			/*for(uint32_t i = 0; i < 1000; ++i)
			{
				std::stringstream s;
				s << "test.a" << i;
				
				octopus::event_t start{"a", 1, "scenario", "test", i, octopus::indicator_e::start, i, s.str()};
				octopus::event_t end{"a", 1, "scenario", "test", i, octopus::indicator_e::end, i+1, s.str()};
				
				std::cout << start << std::endl;
				std::cout << end << std::endl;
			}*/
			
			return 1;
		}
	};
}
