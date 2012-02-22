// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2011
//
// (c) 2011 Goethe-Universität Frankfurt
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
// A stack allocator.

// void thorup_C_p(p_t &p, const G_t &G, const std::map<unsigned int, std::set<unsigned int> > &S)
// Constructs the partial map p used in Thorup's coloring heuristic algorithm C from the underlying graph G and the separators S.

#ifndef SDCCSALLOC_HH
#define SDCCSALLOC_HH 1

extern "C"
{
#include "SDCCmem.h"
#include "SDCCglobl.h"
}

#define SALLOC_CH (options.salloc == 1 || options.salloc == 2) // Chaitin
#define SALLOC_CHA (options.salloc == 2) // Chaitin with alignment
#define SALLOC_TD (options.salloc == 3 || options.salloc == 4 || options.salloc == 5)
#define SALLOC_TDS (options.salloc == 3) // Simplified
#define SALLOC_TDR (options.salloc == 5) // With greedy post-recoloring.

#undef DEBUG_SALLOC

#if defined(TD_SALLOC) || defined(CH_SALLOC)
template<class G_t, class I_t, class SI_t>
static void set_spilt(G_t &G, const I_t &I, SI_t &scon)
{
  std::map<int, var_t> symbol_to_sindex;
  symbol *sym;
  var_t j, j_mark;

  // Add variables that need to be on the stack due to having had their address taken.
  for(sym = static_cast<symbol *>(setFirstItem(istack->syms)), j = 0; sym; sym = static_cast<symbol *>(setNextItem(istack->syms)))
    {
      if(sym->_isparm)
        continue;
        
      if(!(IS_AGGREGATE(sym->type) || sym->allocreq && (sym->addrtaken || isVolatile(sym->type))))
        continue;
      
      boost::add_vertex(scon);
      scon[j].sym = sym;
      scon[j].color = -1;
      j++;
      
      //std::cout << "Symbol " << sym->name << " needs stack space.\n";
    }
  j_mark = j;
  
  // Add edges due to scope (see C99 standard, verse 1233, which requires things to have different addresses, not allowing us to allocate them to the same location, even if we otherwise could).
  for(unsigned int i = 0; i < boost::num_vertices(scon); i++)
     for(unsigned int j = i + 1; j < boost::num_vertices(scon); j++)
        {
          short p = btree_lowest_common_ancestor(scon[i].sym->block, scon[j].sym->block);
          if(p == scon[i].sym->block || p == scon[j].sym->block)
            boost::add_edge(i, j, scon);
        }

  // Set stack live ranges
  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      for(unsigned int j = 0; j < boost::num_vertices(scon); j++)
        {
          short p = btree_lowest_common_ancestor(G[i].ic->block, scon[j].sym->block);
          if(p == G[i].ic->block || p == scon[j].sym->block)
            G[i].stack_alive.insert(j);
        }
    }

  // Add variables that have been spilt in register allocation.
  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      std::set<var_t>::const_iterator v, v_end;
      for (v = G[i].alive.begin(), v_end = G[i].alive.end(); v != v_end; ++v)
        {
          var_t vs;

          symbol *const sym = (symbol *)(hTabItemWithKey(liveRanges, I[*v].v));

          if (sym->regs[0] || sym->accuse || sym->remat || !sym->nRegs)
            continue;

          if (symbol_to_sindex.find(I[*v].v) == symbol_to_sindex.end())
            {
              boost::add_vertex(scon);
              scon[j].sym = sym;
              scon[j].color = -1;
              symbol_to_sindex[I[*v].v] = j;
              j++;
            }

          vs = symbol_to_sindex[I[*v].v];
        
          G[i].stack_alive.insert(vs); // Needs to be allocated on the stack.
        }
    }

  // Add edges to conflict graph.
  typename boost::graph_traits<I_t>::edge_iterator e, e_end;
  for (boost::tie(e, e_end) = boost::edges(I); e != e_end; ++e)
    {
      if (I[boost::source(*e, I)].v == I[boost::target(*e, I)].v || symbol_to_sindex.find(I[boost::source(*e, I)].v) == symbol_to_sindex.end() || symbol_to_sindex.find(I[boost::target(*e, I)].v) == symbol_to_sindex.end())
        continue;
        
      boost::add_edge(symbol_to_sindex[I[boost::source(*e, I)].v], symbol_to_sindex[I[boost::target(*e, I)].v], scon);
    }
    
  // Add conflicts between variables that had their address taken and those that have been spilt by register allocation.
  // TODO: More exact live range analysis for variables that had their address taken (to reduce stack space consumption further, by reducing the number of conflicts here).
  for(unsigned int i = 0; i < j_mark; i++)
    for(unsigned int j = j_mark; j < boost::num_vertices(scon); j++)
      {
        short p = btree_lowest_common_ancestor(scon[i].sym->block, scon[j].sym->block);
        if(p == scon[i].sym->block || p == scon[j].sym->block)
          boost::add_edge(i, j, scon);
      }

  // Ugly hack: Regparms.
  for(sym = static_cast<symbol *>(setFirstItem(istack->syms)), j = boost::num_vertices(scon); sym; sym = static_cast<symbol *>(setNextItem(istack->syms)))
    {
      if(!sym->_isparm || !IS_REGPARM(sym->etype) || !sym->onStack || !sym->allocreq)
        continue;
      
      boost::add_vertex(scon);
      scon[j].sym = sym;
      scon[j].color = -1;

      // Extend liverange to cover everything.
      for(unsigned int i = 0; i < boost::num_vertices(G); i++)
        G[i].stack_alive.insert(j);

      // Conflict with everything.
      for(unsigned int i = 0; i < j; i++)
        boost::add_edge(i, j, scon);

      j++;
    }
}
#endif

#ifdef TD_SALLOC
#include <boost/graph/max_cardinality_matching.hpp>
#include <boost/graph/filtered_graph.hpp>

template <class F_t>
struct in_separator_edge {
  in_separator_edge() { }
  in_separator_edge(const std::set<unsigned int> &S, const F_t &F) : s(S) { f = &F; }
  template <typename Edge>
  bool operator()(const Edge& e) const {
    return(s.count(boost::source(e, *f)) && s.count(boost::source(e, *f)));
  }
  
  std::set<unsigned int> s;
  const F_t *f;
};

struct in_separator_node {
  in_separator_node() { }
  in_separator_node(const std::set<unsigned int> &S) : s(S) { }
  template <typename node>
  bool operator()(const node& n) const {
    return(s.count(n));
  }
  
  std::set<unsigned int> s;
};

// Construct the partial map p used in Thorup's algorithm C from the underlying graph G and the separators S.
template <class p_t, class G_t>
void thorup_C_p(p_t &p, const G_t &G, const std::list<unsigned int> &ordering, const std::map<unsigned int, std::set<unsigned int> > &S)
{
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> F_t;
  F_t F(boost::num_vertices(G));
  std::list<unsigned int>::const_iterator i, i_end;

  for(i = ordering.begin(), i_end = ordering.end(); i != i_end; ++i)
    {
      in_separator_edge<F_t> edge_filter(S.find(*i)->second, F);
      in_separator_node node_filter(S.find(*i)->second);
      
      boost::filtered_graph<F_t, in_separator_edge<F_t>, in_separator_node > f(F, edge_filter, node_filter);

      std::vector<typename boost::graph_traits<G_t>::vertex_descriptor> M(boost::num_vertices(G));

      boost::edmonds_maximum_cardinality_matching(f, &M[0]);

      if(boost::matching_size(f, &M[0]) * 2 < S.find(*i)->second.size())
        {
          boost::add_edge(*i, *S.find(*i)->second.begin(), F);
          p[*i] = *S.find(*i)->second.begin();
        }
    }
}
#endif

#if defined(TD_SALLOC) || defined(CH_SALLOC)
template <class SI_t>
void color_stack_var(const var_t v, SI_t &SI, int start, int *ssize)
{
  symbol *const sym = SI[v].sym;
  const int size = getSize(sym->type);
  
  SI[v].color = start;
  
  if(sym->usl.spillLoc)
    SPEC_STAK(sym->usl.spillLoc->etype) = sym->usl.spillLoc->stack = (port->stack.direction > 0) ? start + 1 : -start - size;
  else
    SPEC_STAK(sym->etype) = sym->stack = (port->stack.direction > 0) ? start + 1 : -start - size;
    
  if(ssize)
    *ssize = (start + size > *ssize) ? start + size : *ssize;

#if DEBUG_SALLOC    
  std::cout << "Placing " << sym->name << " at [" << start << ", " << (start + size - 1) << "]\n";
#endif
    
  // Mark stack location as used for all conflicting variables.
  typename boost::graph_traits<SI_t>::adjacency_iterator n, n_end;
  for(boost::tie(n, n_end) = boost::adjacent_vertices(v, SI); n != n_end; ++n)
    SI[*n].free_stack -= boost::icl::discrete_interval<int>::type(start, start + size);
}

// Place a single variable on the stack greedily.
template <class SI_t>
void color_stack_var_greedily(const var_t v, SI_t &SI, int alignment, int *ssize)
{
  int start;
  symbol *const sym = SI[v].sym;
  const int size = getSize(sym->type);
 
  // Find a suitable free stack location.
  boost::icl::interval_set<int>::iterator si;
  for(si = SI[v].free_stack.begin();; ++si)
    {
       start = boost::icl::first(*si);
       
       // Adjust start address for alignment
       if(start % alignment)
         start = start + alignment - start % alignment;
                    
       if(boost::icl::last(*si) >= start + size - 1)
         break; // Found one.
    }
    
  color_stack_var(v, SI, start, ssize);
}
#endif

#ifdef TD_SALLOC
template <class SI_t>
void color_stack_var_thorup(const var_t v, SI_t &SI, boost::icl::interval_set<int> &free_stack, int alignment, int *ssize)
{
  int start;
  symbol *const sym = SI[v].sym;
  const int size = getSize(sym->type);

  // Find a suitable free stack location.
  boost::icl::interval_set<int>::iterator si;
  for(si = free_stack.begin();; ++si)
    {
       start = boost::icl::first(*si);
       
       // Adjust start address for alignment
       if(start % alignment)
         start = start + alignment - start % alignment;
                    
       if(boost::icl::last(*si) >= start + size - 1)
         break; // Found one.
    }


  color_stack_var(v, SI, start, ssize);

  free_stack -= boost::icl::discrete_interval<int>::type(start, start + size);
}
#endif

#ifdef TD_SALLOC
// Place a two variables that need to share a stack location on the stack greedily.
template <class SI_t>
void color_stack_vars_greedily(var_t v1, var_t v2, SI_t &SI, int alignment, int *ssize)
{
  int start;
  symbol *const sym1 = SI[v1].sym;
  symbol *const sym2 = SI[v2].sym;
  if(getSize(sym1->type) != getSize(sym2->type))
    std::cerr << "Size mismatch in stack allocation: " << sym1->name << " and " << sym2->name << "\n";
  const int size = getSize(sym1->type);

  // Find a suitable free stack location.
  boost::icl::interval_set<int> free_stack = (SI[v1].free_stack & SI[v2].free_stack);
  boost::icl::interval_set<int>::iterator si;
  for(si = free_stack.begin();; ++si)
    {
       start = boost::icl::first(*si);
       
       // Adjust start address for alignment
       if(start % alignment)
         start = start + alignment - start % alignment;
                    
       if(boost::icl::last(*si) >= start + size - 1)
         break; // Found one.
    }
  
  color_stack_var(v1, SI, start, ssize);
  color_stack_var(v2, SI, start, ssize);
}

struct bpt_node
{
  var_t v;
};

typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, bpt_node> bpt_t;

template <class G_t, class SI_t>
void color_biclique_thorup(unsigned int i, unsigned int pi, const G_t &G, SI_t &SI, boost::icl::interval_set<int> &free_stack, int size, int alignment, int *ssize)
{
  std::map<unsigned int, unsigned int> i_to_i;
  unsigned int j;
  
  bpt_t b;

  // Add nodes to bipartite graph b.
  std::set<var_t>::const_iterator s;
  for(s = G[i].stack_alive.begin(), j = 0; s != G[i].stack_alive.end(); ++s)
    {
      var_t v = *s;
      
      if(getSize(SI[v].sym->type) != size)
        continue; // Wrong size.
        
      if(i_to_i.find(v) != i_to_i.end())
        continue; // Already added to bipartite graph.

      boost::add_vertex(b);
      b[j].v = v;
      i_to_i[v] = j++;
    }
  for(s = G[pi].stack_alive.begin(); s != G[pi].stack_alive.end(); ++s)
    {
      var_t v = *s;
      
      if(getSize(SI[v].sym->type) != size)
        continue; // Wrong size.
        
      if(i_to_i.find(v) != i_to_i.end())
        continue; // Already added to bipartite graph.

      boost::add_vertex(b);
      b[j].v = v;
      i_to_i[v] = j++;
    }

  // Add edges to b.
  for(unsigned int i = 0; i < boost::num_vertices(b); i++)
    for(unsigned int j = i + 1; j < boost::num_vertices(b); j++)
       if(!boost::edge(b[i].v, b[j].v, SI).second)
         boost::add_edge(i, j, b);
  
  // Find maximum matching in b.
  std::vector<typename boost::graph_traits<G_t>::vertex_descriptor> M(boost::num_vertices(b));
  boost::edmonds_maximum_cardinality_matching(b, &M[0]);
  
  // Allocate.
  for(unsigned int i = 0; i < boost::num_vertices(b); i++)
    {
      const var_t v1 = b[i].v;

      if(M[i] == boost::graph_traits<bpt_t>::null_vertex())
        {
          if(SI[v1].color < 0)
            color_stack_var_thorup(v1, SI, free_stack, alignment, ssize);
          continue;
        }
        
      const var_t v2 = b[M[i]].v;
      const int c1 = SI[v1].color;
      const int c2 = SI[v2].color;
      if(c1 >= 0 && c2 >= 0) // Both already colored.
        continue;
        
      if(c1 < 0 && c2 < 0) // Both still uncolored.
        std::cerr << "Pair of uncolored non-conflicting variables found while coloring biclique.\n";
        
      // Give the uncolored one the color of the colored one.
      var_t vc, vu;
      if(c1 >= 0)
        vc = v1, vu = v2;
      else
        vu = v1, vc = v2;

      color_stack_var(vu, SI, SI[vc].color, ssize);
    }
}
#endif

#ifdef TD_SALLOC

template <class p_t, class G_t, class SI_t>
void thorup_C_color(const p_t &p, const G_t &G, SI_t &SI, const std::list<unsigned int> &ordering, const std::map<unsigned int, std::set<unsigned int> > &S, int size, int *ssize)
{
  //std::vector<boost::icl::interval_set<int> free_stack> free_stacks;

  int base = *ssize;
  boost::icl::interval_set<int> free_stack;

  std::list<unsigned int>::const_iterator i, i_end;
  for(i = ordering.begin(), i_end = ordering.end(); i != i_end; ++i)
    {
      // Calculate set of available stack locations.
      free_stack.insert(boost::icl::discrete_interval<int>::type(base, 1 << 15));
      std::set<unsigned int>::iterator j, j_end;
      const std::set<unsigned int> &S_i = S.find(*i)->second;
      for(j = S_i.begin(), j_end = S_i.end(); j != j_end; ++j)
        for(std::set<var_t>::const_iterator s = G[*j].stack_alive.begin(); s != G[*j].stack_alive.end(); ++s)
          if(SI[*s].color >= 0)
            free_stack -= boost::icl::discrete_interval<int>::type(SI[*s].color, SI[*s].color + getSize(SI[*s].sym->type));

      //std::cout << "Coloring at " << *i << "\n";

      std::set<symbol *>::const_iterator s;
      typename p_t::const_iterator pi = p.find(*i);
      if(SALLOC_TDS || pi == p.end()) // Just color all uncolored variables at X_{v_i} greedily.
        {
          std::set<var_t>::const_iterator s, s_end;
          for(s = G[*i].stack_alive.begin(), s_end = G[*i].stack_alive.end(); s != s_end; ++s)
            if(getSize(SI[*s].sym->type) == size && SI[*s].color < 0)
              color_stack_var_thorup(*s, SI, free_stack, (size == 2 || size == 4) ? size : 1, ssize);
        }
      else // Optimally color the biclique X_{v_i} \cup X_{p(v_i)} and rename the colors greedily.
        color_biclique_thorup(*i, pi->second, G, SI, free_stack, size, (size == 2 || size == 4) ? size : 1, ssize);
    }
}

template <class G_t, class SI_t>
void tree_dec_recolor_greedily(const G_t &G, SI_t &SI, const std::list<unsigned int> &ordering, int *ssize)
{
  std::set<int> starts;

  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      std::set<var_t>::const_iterator s;
      for(s = G[i].stack_alive.begin(); s != G[i].stack_alive.end(); ++s)
        starts.insert(SI[*s].color);
    }
    
  for(unsigned int i = 0; i < boost::num_vertices(SI); i++)
      SI[i].free_stack.insert(boost::icl::discrete_interval<int>::type(0, 1 << 15));
// Todo: Do this more efficiently by sorting vars first by live range (as per i below), then by starts (as above), and just recolor in that order.
  for(std::set<int>::const_iterator start = starts.begin(); start != starts.end(); ++start)
    {
      std::list<unsigned int>::const_iterator i, i_end;
      for(i = ordering.begin(), i_end = ordering.end(); i != i_end; ++i)
        {
          std::set<var_t>::const_iterator s, s_end;
          for(s = G[*i].stack_alive.begin(), s_end = G[*i].stack_alive.end(); s != s_end; ++s)
            if(SI[*s].color == *start)
              {
                int size = getSize(SI[*s].sym->type);
                color_stack_var_greedily(*s, SI, (size == 2 || size == 4) ? size : 1, ssize);
              }
        }
    }
}

template <class G_t, class SI_t>
void tree_dec_salloc(const G_t &G, SI_t &SI, const std::list<unsigned int> &ordering, const std::map<unsigned int, std::set<unsigned int> > &S)
{
  std::map<unsigned int, unsigned int> p;
  std::set<int> sizes;
  
  thorup_C_p(p, G, ordering, S);

  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      std::set<var_t>::const_iterator s;
      for(s = G[i].stack_alive.begin(); s != G[i].stack_alive.end(); ++s)
        sizes.insert(getSize(SI[*s].sym->type));
    }
    
  for(unsigned int i = 0; i < boost::num_vertices(SI); i++)
      SI[i].free_stack.insert(boost::icl::discrete_interval<int>::type(0, 1 << 15));
    
  int ssize = 0;
  clearStackOffsets();
    
  for(std::set<int>::const_reverse_iterator s = sizes.rbegin(); s != sizes.rend(); ++s)
    thorup_C_color(p, G, SI, ordering, S, *s, &ssize);
  
  if(SALLOC_TDR)
    {
      ssize = 0;
      tree_dec_recolor_greedily(G, SI, ordering, &ssize);
    }

  if(currFunc)
    {
      currFunc->stack += ssize;
      SPEC_STAK (currFunc->etype) += ssize;
    }
}
#endif

#ifdef CH_SALLOC
template <class SI_t>
void chaitin_ordering(const SI_t &SI, std::list<var_t> &ordering)
{
  std::vector<bool> marked(boost::num_vertices(SI));
  unsigned int num_marked, i, d, mind, minn;
  std::stack<var_t> stack;
  
  for(num_marked = 0; num_marked < boost::num_vertices(SI); num_marked++)
    {
      mind = UINT_MAX;
      minn = -1;
      for(i = 0; i < boost::num_vertices(SI); i++)
        {
          if(marked[i])
            continue;
          
          typename boost::graph_traits<const SI_t>::adjacency_iterator n, n_end;
          for(boost::tie(n, n_end) = boost::adjacent_vertices(i, SI), d = 0; n != n_end; ++n)
             d += !marked[*n];
             
          if(d < mind)
            {
              mind = d;
              minn = i;
            }
        }
        
      stack.push(minn);
      marked[minn] = true;
    }
    
  while(!stack.empty())
    {
      ordering.push_back(stack.top());
      stack.pop();
    }
}

template <class SI_t>
void chaitin_salloc(SI_t &SI)
{
  std::list<var_t> ordering;
  
  chaitin_ordering(SI, ordering);
  
  for(unsigned int i = 0; i < boost::num_vertices(SI); i++)
      SI[i].free_stack.insert(boost::icl::discrete_interval<int>::type(0, 1 << 15));
    
  int ssize = 0;
  
  clearStackOffsets();
  
  std::list<var_t>::const_iterator i, i_end;
  for(i = ordering.begin(), i_end = ordering.end(); i != i_end; ++i)
    color_stack_var_greedily(*i, SI, ((getSize(SI[*i].sym->type) == 2 || getSize(SI[*i].sym->type) == 4) && SALLOC_CHA) ? getSize(SI[*i].sym->type) : 1, &ssize);
  
  if(currFunc)
    {
      currFunc->stack += ssize;
      SPEC_STAK (currFunc->etype) += ssize;
    }
}
#endif

#if defined(TD_SALLOC) || defined (CH_SALLOC)
// Dump stack conflict graph, with numbered and named nodes.
static
void dump_scon(const scon_t &scon)
{
  if(!currFunc)
    return;

  std::ofstream dump_file((std::string(dstFileName) + ".dumpscon" + currFunc->rname + ".dot").c_str());

  std::string *name = new std::string[boost::num_vertices(scon)];
  for(var_t i = 0; static_cast<boost::graph_traits<scon_t>::vertices_size_type>(i) < boost::num_vertices(scon); i++)
    {
      std::ostringstream os;
      os << i;
      if (scon[i].sym->name)
        os << " : " << scon[i].sym->name << " : " << getSize(scon[i].sym->type);
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, scon, boost::make_label_writer(name));
  delete[] name;
}
#endif

#endif

