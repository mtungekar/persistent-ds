// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"
#include "EntityWriter.h"
#include "EntityReader.h"
#include "EntityValidator.h"

#include <stack>
#include <set>
#include <queue>

namespace pds
	{
	enum DirectedGraphFlags : uint
		{
		Acyclic = 0x1, // if set, validation make sure the directed graph is acyclic (DAG)
		Rooted = 0x2, // if set, validation will make sure all graph vertices can be reachable from the root(s)
		SingleRoot = 0x4, // if set, validation will make sure there is a single graph root vertex
		};

	template<class _Ty, uint _Flags = 0, class _SetTy = std::set<std::pair<const _Ty, const _Ty>>>
	class DirectedGraph
		{
		public:
			using node_type = _Ty;
			using allocator_type = typename _SetTy::allocator_type;

			using set_type = _SetTy;
			using value_type = typename set_type::value_type;
			using iterator = typename set_type::iterator;
			using const_iterator = typename set_type::iterator;

			static const bool type_acyclic = (_Flags & DirectedGraphFlags::Acyclic) != 0;
			static const bool type_rooted = (_Flags & DirectedGraphFlags::Rooted) != 0;
			static const bool type_single_root = (_Flags & DirectedGraphFlags::SingleRoot) != 0;

			class MF;
			friend MF;

			DirectedGraph() = default;
			DirectedGraph( const DirectedGraph &other ) = default;
			DirectedGraph &operator=( const DirectedGraph &other ) = default;
			DirectedGraph( DirectedGraph &&other ) = default;
			DirectedGraph &operator=( DirectedGraph &&other ) = default;
			~DirectedGraph() = default;

		private:
			std::set<_Ty> v_Roots;
			set_type v_Edges;

		public:
			// inserts an edge, unless it already exists
			void InsertEdge( const node_type &key, const node_type &value );

			// find a particular key-value pair (directed edge)
			bool HasEdge( const node_type &key, const node_type &value ) const;

			// get the range of iterators to enumerate all successors of the key, or end() if no successor exists in the graph
			std::pair<iterator, iterator> GetSuccessors( const node_type &key );
			std::pair<const_iterator,const_iterator> GetSuccessors( const node_type &key ) const;

			// direct access to Edges structure
			set_type &Edges() noexcept { return this->v_Edges; }
			const set_type &Edges() const noexcept { return this->v_Edges; }

			// direct access to Roots set
			std::set<_Ty> &Roots() noexcept { return this->v_Roots; }
			const std::set<_Ty> &Roots() const noexcept { return this->v_Roots; }
		};

	template<class _Ty, uint _Flags, class _SetTy>
	inline void DirectedGraph<_Ty, _Flags, _SetTy>::InsertEdge( const node_type &key, const node_type &value ) 
		{
		this->v_Edges.emplace( key, value );
		}

	template<class _Ty, uint _Flags, class _SetTy>
	inline bool DirectedGraph<_Ty,_Flags,_SetTy>::HasEdge( const node_type &key, const node_type &value ) const 
		{
		return this->v_Edges.find( pair_type( key, value ) ) != this->v_Edges.end();
		}

	template<class _Ty, uint _Flags, class _SetTy>
	inline std::pair<typename DirectedGraph<_Ty,_Flags,_SetTy>::iterator,typename DirectedGraph<_Ty,_Flags,_SetTy>::iterator> 
		DirectedGraph<_Ty,_Flags,_SetTy>::GetSuccessors( const node_type &key ) 
		{
		return std::pair<iterator, iterator> (
			this->v_Edges.lower_bound( std::pair<_Ty,_Ty>(key,data_type_information<_Ty>::inf) ),
			this->v_Edges.lower_bound( std::pair<_Ty,_Ty>(key,data_type_information<_Ty>::sup) )
			); 
		}

	template<class _Ty, uint _Flags, class _SetTy>
	inline std::pair<typename DirectedGraph<_Ty,_Flags,_SetTy>::const_iterator,typename DirectedGraph<_Ty,_Flags,_SetTy>::const_iterator> 
		DirectedGraph<_Ty,_Flags,_SetTy>::GetSuccessors( const node_type &key ) const 
		{
		return std::pair<const_iterator, const_iterator> (
			this->v_Edges.lower_bound( std::pair<_Ty,_Ty>(key,data_type_information<_Ty>::inf) ),
			this->v_Edges.lower_bound( std::pair<_Ty,_Ty>(key,data_type_information<_Ty>::sup) )
			); 
		}


	class EntityWriter;
	class EntityReader;
	class EntityValidator;

	template<class _Ty, uint _Flags, class _SetTy>
	class DirectedGraph<_Ty,_Flags,_SetTy>::MF
		{
		using _MgmCl = DirectedGraph<_Ty,_Flags,_SetTy>;

		inline static bool set_contains( const std::set<_Ty> &set, const _Ty &val )
			{
			return set.find( val ) != set.end();
			}

		public:
			static void Clear( _MgmCl &obj )
				{
				obj.v_Roots.clear();
				obj.v_Edges.clear();
				}

			static void DeepCopy( _MgmCl &dest, const _MgmCl *source )
				{
				if( !source )
					{
					MF::Clear( dest );
					return;
					}

				// replace contents
				dest.v_Roots = std::set<_Ty>(source->v_Roots.begin(), source->v_Roots.end());
				dest.v_Edges = set_type(source->v_Edges.begin(), source->v_Edges.end());
				}
			
			static bool Equals( const _MgmCl *lval, const _MgmCl *rval )
				{
				// early out if the pointers are equal (includes nullptr)
				if( lval == rval )
					return true;

				// early out if one of the pointers is nullptr (both can't be null because of above test)
				if( !lval || !rval )
					return false;

				// early out if the sizes are not the same 
				if( lval->v_Roots.size() != rval->v_Roots.size() )
					return false;
				if( lval->v_Edges.size() != rval->v_Edges.size() )
					return false;

				// compare roots
				auto lval_roots_it = lval->v_Roots.begin();
				auto rval_roots_it = rval->v_Roots.begin();
				while( lval_roots_it != lval->v_Roots.end() )
					{
					if( (*lval_roots_it) != (*rval_roots_it) )
						return false;
					++lval_roots_it;
					++rval_roots_it;
					}

				// compare all the edges
				auto lval_edges_it = lval->v_Edges.begin();
				auto rval_edges_it = rval->v_Edges.begin();
				while( lval_edges_it != lval->v_Edges.end() )
					{
					if( (*lval_edges_it) != (*rval_edges_it) )
						return false;
					++lval_edges_it;
					++rval_edges_it;
					}

				return true;
				}

			static bool Write( const _MgmCl &obj , EntityWriter &writer )
				{
				// store the roots 
				std::vector<_Ty> roots( obj.v_Roots.begin(), obj.v_Roots.end() );
				if( !writer.Write( pdsKeyMacro("Roots"), roots ) )
					return false;

				// collect the keys-value pairs into a vector and store as an array
				std::vector<_Ty> graph_pairs(obj.v_Edges.size()*2);
				size_t index = 0;
				for( auto it = obj.v_Edges.begin(); it != obj.v_Edges.end(); ++it, ++index )
					{
					graph_pairs[index*2+0] = it->first;
					graph_pairs[index*2+1] = it->second;
					}
				if( !writer.Write( pdsKeyMacro("Edges"), graph_pairs ) )
					return false;

				// sanity check, make sure all sections were written
				pdsSanityCheckDebugMacro( index == obj.v_Edges.size() );

				return true;
				}

			static bool Read( _MgmCl &obj , EntityReader &reader )
				{
				size_t map_size = {};
				typename _MgmCl::iterator it = {};

				// read the roots 
				std::vector<_Ty> roots;
				if( !reader.Read( pdsKeyMacro("Roots"), roots ) )
					return false;
				obj.v_Roots = std::set<_Ty>(roots.begin(), roots.end());
				
				// read in the graph pairs
				std::vector<_Ty> graph_pairs;
				if( !reader.Read( pdsKeyMacro("Edges"), graph_pairs ) )
					return false;
				
				// insert into map
				obj.v_Edges.clear();
				map_size = graph_pairs.size() / 2;
				for( size_t index = 0; index < map_size; ++index )
					{
					obj.InsertEdge( graph_pairs[index * 2 + 0], graph_pairs[index * 2 + 1] );
					}

				return true;
				}

		private:
			static void ValidateNoCycles( const _MgmCl::set_type &edges, EntityValidator &validator )
				{
				// Do a depth-first search, find all nodes starting from the root nodes 
				// Note: Only reports first found cycle, if any
				std::stack<_Ty> stack;
				std::set<_Ty> on_stack;
				std::set<_Ty> checked;

				// try all nodes
				for( const auto &p : edges )
					{
					const _Ty &node = p.first;

					// if already checked, skip
					if( set_contains( checked, node ) )
						continue;

					// push on to stack
					stack.push( node );

					// run until all items on stack are popped again
					while( !stack.empty() )
						{
						// get the top from the stack
						_Ty curr = stack.top();

						if( !set_contains( checked, curr ) )
							{
							// if we havent checked, mark the node as on the stack
							checked.insert( curr );
							on_stack.insert( curr );
							}
						else
							{
							// we are done with it, remove from stack
							on_stack.erase( curr );
							stack.pop();
							}

						// list all nodes downstream from curr
						auto itr = edges.lower_bound( std::pair<_Ty,_Ty>(curr,data_type_information<_Ty>::inf) );
						auto itr_end = edges.upper_bound( std::pair<_Ty,_Ty>(curr,data_type_information<_Ty>::sup) );
						while( itr != itr_end )
							{
							pdsSanityCheckCoreDebugMacro( itr->first == curr );
							const _Ty &child = itr->second;

							if( !set_contains( checked, child ) )
								{
								// this has not been checked, add on top of stack to be checked next
								stack.push( child );
								}
							else if( set_contains( on_stack, child ) )
								{
								// This child node is already marked on the stack, so we have already visited it once 
								// We have a cycle, report it, and return
								pdsValidationError( ValidationError::InvalidSetup )
									<< "The node " << child << " in Graph is a part of a cycle, but the graph is acyclic."
									<< pdsValidationErrorEnd;
								return;
								}

							++itr;
							}
						}
					}

				// no cycles found, all good
				}
			
			static void ValidateRooted( const std::set<_Ty> &roots , const std::set<_Ty> &downstream_nodes , const _MgmCl::set_type &edges, EntityValidator &validator )
				{
				std::queue<_Ty> queue;
				std::set<_Ty> reached;

				// push all the roots onto the queue
				for( const auto &n : roots )
					{
					queue.push( n );
					}

				// try to reach all downstream nodes from the roots
				while( !queue.empty() )
					{
					// get the first value in the queue
					_Ty curr = queue.front();
					queue.pop();

					// if already reached, skip
					if( set_contains( reached, curr ) )
						continue;

					// mark it as reached
					reached.insert( curr );

					// check downstream nodes
					auto itr = edges.lower_bound( std::pair<_Ty,_Ty>(curr,data_type_information<_Ty>::inf) );
					auto itr_end = edges.upper_bound( std::pair<_Ty,_Ty>(curr,data_type_information<_Ty>::sup) );
					while( itr != itr_end )
						{
						pdsSanityCheckCoreDebugMacro( itr->first == curr );
						const _Ty &child = itr->second;

						if( !set_contains( reached, child ) )
							{
							queue.push( child );
							}

						++itr;
						}
					}

				// make sure all downstream nodes were reached
				for( auto n : downstream_nodes )
					{
					if( !set_contains( reached, n ) )
						{
						// This child node is already marked on the stack, so we have already visited it once 
						// We have a cycle, report it, and return
						pdsValidationError( ValidationError::InvalidSetup )
							<< "The node " << n << " in Graph could not be reached from (any of) the root(s) in the Roots set."
							<< pdsValidationErrorEnd;
						}
					}
				}

		public:
			static bool Validate( const _MgmCl &obj, EntityValidator &validator )
				{
				// make a set of all nodes with incoming edges
				std::set<_Ty> downstream_nodes;
				for( const auto &p : obj.v_Edges )
					{
					if( !set_contains( downstream_nodes , p.second ) )
						downstream_nodes.insert( p.second );
					}

				// the rest of the nodes are root nodes (no incoming edges)
				std::set<_Ty> root_nodes;
				for( const auto &p : obj.v_Edges )
					{
					if( !set_contains( downstream_nodes , p.first ) )
						root_nodes.insert( p.first );
					}

				// check for single root object
				if( type_single_root )
					{
					if( root_nodes.size() != 1 )
						{
						pdsValidationError( ValidationError::InvalidCount ) << "The number of roots found when searching through the graph is " << root_nodes.size() << " but the graph is required to have exactly one root." << pdsValidationErrorEnd;
						}
					}

				// if this is a rooted graph, make sure that the v_Roots set is correct
				if( type_rooted )
					{
					if( type_single_root )
						{
						if( obj.v_Roots.size() != 1 )
							{
							pdsValidationError( ValidationError::InvalidCount ) << "The graph is single rooted, but the Roots set has " << obj.v_Roots.size() << " nodes. The Roots set must have exactly one node." << pdsValidationErrorEnd;
							}
						}

					// make sure that all nodes in the v_Roots list do not have incoming edges
					for( auto n : obj.v_Roots )
						{
						if( set_contains( downstream_nodes , n ) )
							{
							pdsValidationError( ValidationError::InvalidObject )
								<< "Node " << n << " in the Roots set has incoming edges, which makes it invalid as a root node."
								<< pdsValidationErrorEnd;
							}
						}

					// make sure that all nodes that are root nodes (no incoming edges) are in the v_Roots list
					for( auto n : root_nodes )
						{
						if( !set_contains( obj.v_Roots , n ) )
							{
							pdsValidationError( ValidationError::MissingObject )
								<< "Node " << n << " has no incoming edges, so is by definition a root, but is not listed in the Roots set."
								<< pdsValidationErrorEnd;
							}
						}

					// make sure no node is unreachable from the roots
					ValidateRooted( obj.v_Roots, downstream_nodes, obj.v_Edges, validator );
					}

				// check for cycles if the graph is acyclic
				if( type_acyclic )
					{
					ValidateNoCycles( obj.v_Edges, validator );
					}

				return true;
				}

			// additional validation with external data
			template<class _Table> static bool ValidateAllKeysAreContainedInTable( const _MgmCl &obj, EntityValidator &validator, const _Table &otherTable, const char *otherTableName );

		};

		template<class _Ty, uint _Flags, class _SetTy>
		template<class _Table>
		bool DirectedGraph<_Ty,_Flags,_SetTy>::MF::ValidateAllKeysAreContainedInTable( const _MgmCl &obj, EntityValidator &validator, const _Table &otherTable, const char *otherTableName )
			{
			// collect all items 
			std::set<_Ty> nodes;
			for( const auto &p : obj.v_Edges )
				{
				if( !set_contains( nodes , p.first ) )
					nodes.insert( p.first );
				if( !set_contains( nodes , p.second ) )
					nodes.insert( p.second );
				}

			// make sure they are all in the table
			for( auto it = nodes.begin(); it != nodes.end(); ++it )
				{
				if( !_Table::MF::ContainsKey( otherTable, (*it) ) )
					{
					pdsValidationError( ValidationError::MissingObject ) << "The key " << (*it) << " is missing in " << otherTableName << pdsErrorLogEnd;
					}
				}

			return true;
			}
	};