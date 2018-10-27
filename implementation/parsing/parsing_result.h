#pragma once

#include<vector>

template<typename parsed_node>
struct expr_info
{
	using node_list = std::vector<parsed_node>;


	node_list parsed_values;
};