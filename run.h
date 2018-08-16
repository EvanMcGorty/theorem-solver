#include<vector>
#include<string>
#include<iostream>
#include<map>
#include<unordered_set>
#include<functional>
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




std::unordered_set<std::string> history_of_expressions{};


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

	void put_all_subexpressions(expression const& target, std::function<void(expression&&)> to_call)
	{
		auto g = target.get_function();
		if(g)
		{
			for(int i = 0; i!= g->arguments.size(); ++i)
			{
				put_all_subexpressions(g->arguments[i], std::function<void(expression&&)>([&](expression&& a)
				{
					if(a!=expression::make_empty())
					{
						expression newtarget = expression(target);
						newtarget.get_function()->arguments[i] = std::move(a);
						to_call(std::move(newtarget));
					}
				})
				);
			}
		}

		to_call(put(target));
	}

};


std::vector<implication> all_implications{
	implication(
		expression::make("true"),
		expression::make("not(false)")
	), implication(
		expression::make("not(false)"),
		expression::make("true")
	), implication(
		expression::make("not(not(=x))"),
		expression::make("=x")
	), implication(
		expression::make("=x"),
		expression::make("not(not(=x))")
	)
};


void run()
{
    std::vector<expression> current_expressions{expression::make("not(true)")};
    std::vector<expression> next_expressions;
	while(current_expressions.size() != 0 && history_of_expressions.size() < 100)
	{
		for(auto& impl_it : all_implications)
		{
			for(auto& expr_it : current_expressions)
			{
				impl_it.put_all_subexpressions(expr_it, std::function<void(expression&&)>([&](expression&& a)
				{
					if(a != expression::make_empty())
					{
						auto l = history_of_expressions.insert(a.str());
						if(l.second)
						{
							std::cout << *l.first << '\n';
							next_expressions.push_back(std::move(a));
						}
					}
				})
				);
			}
		}
		std::swap(current_expressions,next_expressions);
		next_expressions.resize(0);
	}

	// for(auto& it : history_of_expressions)
	// {
	// 	std::cout << it << '\n';
	// }
	std::cout << std::flush;
	system("pause");
}
