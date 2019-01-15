#pragma once
#include "randomThings.h"
#include "range-like-stuffs.h"

using namespace std::string_literals;

class commandLineParser {
public:
	using functionType = std::function<void(const std::vector<std::string_view>&)>;
	using defaultFunctionType = std::function<void(std::string_view)>;

	template<typename string,std::enable_if_t<std::is_same_v<std::decay_t<string>,std::string>,int> = 0>
	commandLineParser& parseLine(string&& data){//for rvalues and lvalues
		m_line.reserve(data.size());
		for (char i : data) {			
			if((i == '\t' || i == '\n' || i==' ') && m_line.back() != ' ')
				m_line.push_back(' ');
			else
				m_line.push_back(i);			
		}parse_impl(split(std::string_view(m_line)));
		return *this;
	}
	template<typename rng,std::enable_if_t<is_range_of_v<rng,std::string_view>,int> = 0>
	commandLineParser& parseLine(rng&& data){
		parse_impl(data);
		return *this;
	}

	template<typename rng, std::enable_if_t<!is_range_of_v<rng,std::string_view> && std::is_convertible_v<std::decay_t<range_type<rng>>,std::string_view>,int> = 0>
	commandLineParser& parseLine(rng&& data){
		std::vector<std::string_view> stuffs;
		stuffs.reserve(data.size());
		for(auto&& i:data)
			stuffs.emplace_back(i);		
		parse_impl(stuffs);
		return *this;
	}
	commandLineParser& parseLine(const char** argv, const int argc){
		std::vector<std::string_view> stuffs;
		stuffs.resize(argc);
		for (int i = 0; i < argc; ++i) 
			stuffs[i] = argv[i];		
		parse_impl(stuffs);
		return *this;
	}

	void callThings(){
		for (auto& param : m_defaultFn.second)
			m_defaultFn.first(param);
		for(auto& [funs, param_set]:m_fns)
			for(auto& params: param_set)
				funs(params);
	}
	commandLineParser& addOption(std::string op, functionType fn,int n, const bool required = false){
		m_options.insert({std::move(op), std::make_pair(m_fns.size(), n)});
		m_required |= static_cast<size_t>(required) << m_fns.size();
		m_fns.emplace_back().first = std::move(fn);
		return *this;
	}
	commandLineParser& setDefaultfun(defaultFunctionType fn) {
		m_defaultFn.first = std::move(fn);
		return *this;
	}
	commandLineParser& setFirstFn(functionType fn, const size_t argc) {
		m_firstFn.first = std::move(fn);
		m_firstFnNum = argc;		
		return *this;
	}
	void reset_args() {
		for(auto& i:m_fns){
			i.second.clear();
		}
		m_defaultFn.second.clear();
		m_firstFn.second.clear();
	}
private:
	template<typename rng>
	void parse_impl(rng&& argv){
		size_t used = 0;
		auto argv_it = argv.begin();
		if(m_firstFn.first) {			
			const auto next_it= [&]() {
				if (m_firstFnNum == -1) return std::find_if(argv_it, argv.end(), [&](const std::string_view& item) {return is_in_map(item).second; });
				else return argv_it + m_firstFnNum;
			}();
			m_firstFn.second.resize(std::distance(argv_it, next_it));
			std::copy(argv.begin(), next_it, m_firstFn.second.begin());
			argv_it = next_it;
		}
		for (; argv_it != argv.end(); ) {
			const auto[it, is_option] = is_in_map(*argv_it);
			if(!is_option){
				if (!add_default_arg(*argv_it++))
					throw std::runtime_error("invalid flag");
				continue;
			}
			const auto numArgs = it->second.second; 
			auto& vec = m_fns[it->second.first].second.emplace_back();
			const auto next_it = [&](){
				if (numArgs == -1) return std::find_if(argv_it, argv.end(), [&](const std::string_view& item) {return is_in_map(item).second; });					
				else return argv_it + it->second.second + 1;				
			}();
			vec.resize(std::distance(argv_it + 1, next_it));
			std::copy(argv_it + 1, next_it, vec.begin());
			argv_it = next_it;
			used |= 1 << it->second.first;
		}
		if((m_required & used) != m_required)
			throw std::runtime_error("thingy needs the required options");
	}
	bool add_default_arg(const std::string_view arg) {
		if(m_defaultFn.first)
			m_defaultFn.second.push_back(arg);
		else return false;
		return true;
	}
	std::pair<std::map<std::string, std::pair<int, int>, transparent_less>::iterator,bool> is_in_map(const std::string_view thing){
		const auto it = m_options.find(thing);
		return { it,it != m_options.end() };
	}
	
	std::vector<std::pair<functionType, std::vector<std::vector<std::string_view>>>> m_fns;
	std::map<std::string, std::pair<int, int>, transparent_less> m_options;//first is index,second is number of things needed after
	std::string m_line;
	size_t m_required = 0;
	std::pair<defaultFunctionType, std::vector<std::string_view>> m_defaultFn;

	std::pair<functionType, std::vector<std::string_view>> m_firstFn;
	int m_firstFnNum = 0;
};

struct command_thingy{
	commandLineParser& add_command(std::string command_name, commandLineParser command = {}) {
		return m_commands.insert(std::make_pair(std::move(command_name), std::move(command))).first->second;
	}
	template<typename rng>
	std::enable_if_t<std::is_convertible_v<range_type<rng>,std::string>> run_command(rng&& r) {//cant use if constexpr ;-; cuz sfinae
		std::vector<std::string> stuff;
		//if constexpr(std::sized_range<rng>){stuff.reserve(r.size());}
		for(auto&& i:r) {
			stuff.push_back(i);
		}
		auto& command = m_commands[stuff[0]];

	}
private:
	std::map<std::string, commandLineParser, transparent_less> m_commands;
};
