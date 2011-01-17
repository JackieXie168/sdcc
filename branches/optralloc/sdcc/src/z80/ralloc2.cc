// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2011
//
// (c) 2010-2011 Goethe-Universität Frankfurt
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any
// later version.
//   
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//   
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// An optimal, polynomial-time register allocator.
// The current version can handle general-purpose registers only.

#include "SDCCralloc.hpp"

#ifdef TARGET_IS_Z80
#define REG_C 0
#define REG_B 1
#define REG_E 2
#define REG_D 3
#define NUM_REGS 4
#elif defined TARGET_IS_GBZ80
#define REG_C 0
#define REG_B 1
#define NUM_REGS 2
#endif

template <class G_t, class I_t>
float default_operand_cost(const operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
	float c = 0.0f;

	std::map<int, var_t>::const_iterator oi, oi_end;

	var_t byteregs[4];	// Todo: Change this when sdcc supports variables larger than 4 bytes.
	unsigned short int size;

	if(o && IS_SYMOP(o))
	{
		boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL(o)->key);
		if(oi != oi_end)
		{
			var_t v = oi->second;

			// In registers.
			if(a.local.find(v) != a.local.end())
			{
				c += 1.0f;
				byteregs[I[v].byte] = a.global[v];
				size = 1;

				while(++oi != oi_end)
				{
					v = oi->second;
					c += (a.local.find(v) != a.local.end() ? 1.0f : std::numeric_limits<float>::infinity());
					byteregs[I[v].byte] = a.global[v];
					size++;
				}

				// Penalty for not placing 2- and 4-byte variables in register pairs
				// Todo: Extend this once the register allcoator can use registers other than bc, de:
				if((size == 2 || size == 4) && (byteregs[1] != byteregs[0] + 1 || byteregs[0] != REG_C && byteregs[0] != REG_E))
					c += 2.0f;
				if(size == 4 && (byteregs[3] != byteregs[2] + 1 || byteregs[2] != REG_C && byteregs[2] != REG_E))
					c += 2.0f;
			}
			// Spilt.
			else
			{
				c += 4.0f;
				while(++oi != oi_end)
				{
					v = oi->second;
					c += (a.local.find(v) == a.local.end() ? 4.0f : std::numeric_limits<float>::infinity());
				}
			}
		}
	}

	return(c);
}

template <class G_t, class I_t>
float default_instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
	float c = 0.0f;

	const iCode *ic = G[i].ic;

	c += default_operand_cost(IC_RESULT(ic), a, i, G, I);
	c += default_operand_cost(IC_LEFT(ic), a, i, G, I);
	c += default_operand_cost(IC_RIGHT(ic), a, i, G, I);
	
	return(c);
}

// Treat assignment separately to handle coalescing.
template <class G_t, class I_t>
float assign_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
	float c = 0.0f;
	
	const iCode *ic = G[i].ic;

	operand *right = IC_RIGHT(ic);
	operand *result = IC_RESULT(ic);
	
	if(!right || !IS_SYMOP(right) || !result || !IS_SYMOP(result))
		return(default_instruction_cost(a, i, G, I));

	reg_t byteregs[4] = {-1, -1, -1, -1};	// Todo: Change this when sdcc supports variables larger than 4 bytes.
	
	std::map<int, var_t>::const_iterator oi, oi_end;

	int size1 = 0, size2 = 0;
	
	boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL(right)->key);
	if(oi != oi_end)
	{
		var_t v = oi->second;

		if(a.local.find(v) == a.local.end())
			return(default_instruction_cost(a, i, G, I));

		c += 1.0f;
		byteregs[I[v].byte] = a.global[v];
		size1 = 1;

		while(++oi != oi_end)
		{
			v = oi->second;
			c += (a.local.find(v) != a.local.end() ? 1.0f : std::numeric_limits<float>::infinity());
			byteregs[I[v].byte] = a.global[v];
			size1++;
		}
	}
	
	if(!size1)
		return(default_instruction_cost(a, i, G, I));
	
	boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL(result)->key);
	if(oi != oi_end)
	{
		var_t v = oi->second;

		if(a.local.find(v) == a.local.end())
			return(default_instruction_cost(a, i, G, I));

		c += 1.0f;
		if(byteregs[I[v].byte] == a.global[v])
			c -= 2.0f;
		size2 = 1;

		while(++oi != oi_end)
		{
			v = oi->second;
			c += (a.local.find(v) != a.local.end() ? 1.0f : std::numeric_limits<float>::infinity());
			if(byteregs[I[v].byte] == a.global[v])
				c -= 2.0f;
			size2++;
		}
	}
	
	if(!size2)
		return(default_instruction_cost(a, i, G, I));

	return(c);
}

template <class G_t, class I_t>
float ifx_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
	const iCode *ic = G[i].ic;
	
	return(default_operand_cost(IC_COND(ic), a, i, G, I));
}

template <class G_t, class I_t>
float jumptab_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
	const iCode *ic = G[i].ic;
	
	return(default_operand_cost(IC_JTCOND(ic), a, i, G, I));
}

// Cost function.
template <class G_t, class I_t>
float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
	const iCode *ic = G[i].ic;

	switch(ic->op)
	{
	case '=':
		return(assign_cost(a, i, G, I));
	case IFX:
		return(ifx_cost(a, i, G, I));
	case JUMPTABLE:
		return(jumptab_cost(a, i, G, I));
	default:
		return(default_instruction_cost(a, i, G, I));
	}
}

template <class T_t, class G_t, class I_t>
void tree_dec_ralloc(T_t &T, const G_t &G, const I_t &I)
{
	con2_t I2(boost::num_vertices(I));
	for(unsigned int i = 0; i < boost::num_vertices(I); i++)
	{
		I2[i].v = I[i].v;
		I2[i].byte = I[i].byte;
		I2[i].name = I[i].name;
	}
	typename boost::graph_traits<I_t>::edge_iterator e, e_end;
	for(boost::tie(e, e_end) = boost::edges(I); e != e_end; ++e)
		add_edge(boost::source(*e, I), boost::target(*e, I), I2);
	

	typename boost::graph_traits<T_t>::vertex_iterator t, t_end;
	boost::tie(t, t_end) = boost::vertices(T);
	tree_dec_ralloc_nodes(T, *t, G, I2);

	
	std::map<unsigned short int, unsigned short int>::const_iterator i;

	/*std::cout << "Winner: ";
	std::cout << "[";
	for(i = T[*t].assignments.begin()->global.begin(); i != T[*t].assignments.begin()->global.end(); ++i)
		std::cout << "(" << i->first << ", " << i->second << "), ";
	std::cout << "] s: " << T[*t].assignments.begin()->s << "\n";*/
	
	const assignment &winner = *(T[*t].assignments.begin());
	
	// Todo: Make this an assertion
	if(winner.global.size() != boost::num_vertices(I))
		std::cerr << "ERROR\n";
	
	for(var_t v = 0; v < boost::num_vertices(I); v++)
	{
		symbol *sym = (symbol *)(hTabItemWithKey(liveRanges, I[v].v));
		if(winner.global[v] >= 0)
			sym->regs[I[v].byte] = regsZ80 + winner.global[v];
		else
			spillThis(sym);
	}
	
	//std::cout << "Applied.\n";
}

void z80_ralloc2_cc(ebbIndex *ebbi)
{
	//std::cout << "Processing " << currFunc->name << " from " << dstFileName << "\n";
	//std::cout.flush();

	cfg_t control_flow_graph;

	con_t conflict_graph;

	create_cfg(control_flow_graph, conflict_graph, ebbi);

	if(z80_opts.dump_graphs)
		dump_cfg(control_flow_graph);

	if(z80_opts.dump_graphs)
		dump_con(conflict_graph);

	tree_dec_t tree_decomposition;

	thorup_tree_decomposition(tree_decomposition, control_flow_graph);

	nicify(tree_decomposition);

	alive_tree_dec(tree_decomposition, control_flow_graph);

	if(z80_opts.dump_graphs)
		dump_tree_decomposition(tree_decomposition);

	tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph);
}

