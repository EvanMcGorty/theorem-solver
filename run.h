#include<vector>
#include<string>
#include<iostream>
#include<map>
#include"expression-evaluator/include/expression-evaluator/expression.h"

using namespace expr;


//returns true upon failure to match
bool match(expression const& target, expression const& pattern, std::map<std::string,expression>& result)
{
	if(pattern.get_variable())
	{
		auto found = result.find(pattern.get_variable()->var_name);
		if(found==result.end())
		{
			result.insert({pattern.get_variable()->var_name,expression(target)});
		}
		else
		{
			if(found->second != target)
			{
				return true;
			}
		}
	}
	else if(pattern.get_function())
	{
		auto tp = target.get_function();
		auto mp = pattern.get_function();
		if(tp==nullptr || tp->fn_name!=mp->fn_name || tp->arguments.size()!=mp->arguments.size())
		{
			return true;
		}
		for(int i = 0; i!=tp->arguments.size(); ++i)
		{
			if(match(tp->arguments[i],mp->arguments[i],result))
			{
				return true;
			}
		}
	}
	else
	{
		if(target != pattern)
		{
			return true;
		}
	}

	return false;
}

expression build(expression const& output, std::map<std::string,expression>& substitutions)
{
	if(output.get_variable())
	{
		return substitutions[output.get_variable()->var_name];
	}
	else if(output.get_function())
	{
		auto const& g = *output.get_function();
		std::vector<expression> ret;
		for(auto const& it:g.arguments)
		{
			ret.push_back(build(it,substitutions));
		}
		return expression::make(function_value(std::string(g.fn_name),std::move(ret)));
	}
	else
	{
		return expression(output);
	}
}


struct implication
{
	implication() = default;
	implication(implication&&) = default;
	implication(implication const&) = default;
	~implication() = default;
	implication& operator=(implication&&) = default;
	implication& operator=(implication const&) = default;

	implication(expression&& pat, expression&& tem)
	{
		pattern = std::move(pat);
		output = std::move(tem);
	}

	expression pattern;
	expression output;

	expression put(expression const& target)
	{
		std::map<std::string,expression> substitutions;
		if(match(target,pattern,substitutions))
		{
			return expression::make_empty();
		}
		return build(output,substitutions);
	}
};


void run()
{
    implication a{expression::make("sum(=a,sum(=b,=a))"),expression::make("sum(sum(=a,=b),=a)")};
	std::cout << "pattern: " << a.pattern << '\n' << "output: " << a.output << '\n' << std::flush;
	while(true)
	{
		expression cur;
		std::cin >> cur;
		std::cout << a.put(cur) << '\n' << std::flush;
	}
}
