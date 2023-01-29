
#pragma once

#include "../TestPackA/TestEntityA.h"

// creates a random tree using recursive calls. 
// The parameter is the type of DirectedGraph to use for the tree
template<class Graph>
void GenerateRandomTreeRecursive( Graph &graph , uint total_levels , uint current_level = 0, typename Graph::node_type parent_node = random_value<typename Graph::node_type>() )
	{
	typedef typename Graph::node_type _Ty;

	// generate a random number of subnodes
	size_t sub_nodes = capped_rand( 1, 7 );

	// add to tree
	for( size_t i = 0; i < sub_nodes; ++i )
		{
		_Ty child_node = random_value<typename Graph::node_type>();

		graph.Edges().insert( std::pair<_Ty, _Ty>( parent_node, child_node ) );

		if( current_level < total_levels )
			{
			GenerateRandomTreeRecursive( graph, total_levels, current_level + 1, child_node );
			}
		}
	}

// create random dictionary of TestEntity, with random entries, adhere to flags for the dictionary
template<class Dict>
size_t GenerateRandomItemTable( Dict &random_dict , size_t minc = 0, size_t maxc = 100)
	{
	size_t dict_size = capped_rand( minc, maxc );

	for( size_t i = 0; i < dict_size; ++i )
		{
		typename Dict::key_type key = random_value<Dict::key_type>();

		// if zero is not allowed, keep generating until we have non-zero
		if( Dict::type_no_zero_keys )
			{
			while( key == data_type_information<typename Dict::key_type>::zero )
				{
				key = random_value<Dict::key_type>();
				}
			}

		// add an entry.
		// if Dict::type_no_null_entities is true, *always* add a value, else add 50% of the time
		if( Dict::type_no_null_entities || random_value<bool>() )
			{
			random_dict.Entries()[key] = std::make_unique<TestPackA::TestEntityA>();
			random_dict.Entries()[key]->Name() = random_value<std::string>();
			}
		else
			{
			random_dict.Entries().emplace( key, nullptr );
			}
		}

	return random_dict.Size();
	}