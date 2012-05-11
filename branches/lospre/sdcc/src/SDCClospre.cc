// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2011
//
// (c) 2012 Goethe-Universität Frankfurt
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
//
// Lifetime-optimal speculative partial redundancy elimination.

#include "SDCClospre.hpp"

// A quick-and-dirty function to get the CFG from sdcc (a simplified version of the function from SDCCralloc.hpp).
void
create_cfg_lospre (cfg_lospre_t &cfg, iCode *start_ic, ebbIndex *ebbi)
{
  iCode *ic;

  std::map<int, unsigned int> key_to_index;
  {
    int i;

    for (ic = start_ic, i = 0; ic; ic = ic->next, i++)
      {
        boost::add_vertex(cfg);
        key_to_index[ic->key] = i;
        cfg[i].ic = ic;
      }
  }

  // Get control flow graph from sdcc.
  for (ic = start_ic; ic; ic = ic->next)
    {
      if (ic->op != GOTO && ic->op != RETURN && ic->op != JUMPTABLE && ic->next)
        boost::add_edge(key_to_index[ic->key], key_to_index[ic->next->key], 3.0f, cfg);

      if (ic->op == GOTO)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, ic->label)->sch->key], 6.0f, cfg);
      else if (ic->op == RETURN)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, returnLabel)->sch->key], 6.0f, cfg);
      else if (ic->op == IFX)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, IC_TRUE(ic) ? IC_TRUE(ic) : IC_FALSE(ic))->sch->key], 6.0f, cfg);
      else if (ic->op == JUMPTABLE)
        for (symbol *lbl = (symbol *)(setFirstItem (IC_JTLABELS (ic))); lbl; lbl = (symbol *)(setNextItem (IC_JTLABELS (ic))))
          boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, lbl)->sch->key], 6.0f, cfg);
    }
}

static bool
candidate_expression (const iCode *const ic)
{
  // Todo: Allow more operations!
  if (ic->op != '+' &&
    ic->op != '-' &&
    ic->op != RIGHT_OP)
    return (false);

  const operand *const left = IC_LEFT (ic);
  const operand *const right = IC_RIGHT (ic);
  const operand *const result = IC_RESULT (ic);

  // Todo: Allow more operands!
  if (left && !(IS_ITEMP (left) || IS_OP_LITERAL (left)) ||
    right && !(IS_ITEMP (right) || IS_OP_LITERAL (right)) ||
    result && !(IS_ITEMP (result) || IS_OP_LITERAL (result)))
    return (false);

  return (true);
}

static bool
same_expression (const iCode *const lic, const iCode *const ric)
{
  if (lic->op != ric->op)
    return (false);

  const operand *lleft = IC_LEFT (lic);
  const operand *lright = IC_RIGHT (lic);
  const operand *rleft = IC_LEFT (ric);
  const operand *rright = IC_RIGHT (ric);

  // Todo: Go back chain of single-definition temporaries? Might be unsafe. Alternative: Eliminate such assignments somewhere.

  if (isOperandEqual (lleft, rleft) && isOperandEqual (lright, rright) ||
    IS_COMMUTATIVE (lic) && isOperandEqual (lleft, rright) && isOperandEqual (lright, rleft))
    return (true);

  return (false);
}

static void
get_candidate_set(std::set<int> *c, const iCode *const sic)
{
  for (const iCode *ic = sic; ic; ic = ic->next)
    {
      if (!candidate_expression (ic))
        continue;
      for (const iCode *pic = sic; pic != ic; pic = pic->next)
        if (candidate_expression (pic) && same_expression (ic, pic) && c->find (pic->key) == c->end ())
          {
            // Found expression that occurs at least twice.
            c->insert (pic->key);
            break;
          }
    }
}

static void
setup_cfg_for_expression (cfg_lospre_t *const cfg, const iCode *const eic)
{
  typedef boost::graph_traits<cfg_lospre_t>::vertex_descriptor vertex_t;

  for (vertex_t i = 0; i < boost::num_vertices (*cfg); i++)
    {
       const iCode *const ic = (*cfg)[i].ic;
       (*cfg)[i].uses = same_expression (eic, ic);
       (*cfg)[i].invalidates = false;
       // Todo: Do not invalidate pointer when dereferencing?
       if (IC_RESULT (ic) && IC_LEFT (eic) && !IS_OP_LITERAL (IC_LEFT (eic)) && isOperandEqual (IC_LEFT (eic), IC_RESULT (ic)))
         (*cfg)[i].invalidates = true;
       if (IC_RESULT (ic) && IC_RIGHT (eic) && !IS_OP_LITERAL (IC_RIGHT (eic)) && isOperandEqual (IC_RIGHT (eic), IC_RESULT (ic)))
         (*cfg)[i].invalidates = true;
    }
}

void
lospre (iCode *sic, ebbIndex *ebbi)
{
  cfg_lospre_t control_flow_graph;
  tree_dec_lospre_t tree_decomposition;

  create_cfg_lospre (control_flow_graph, sic, ebbi);

  thorup_tree_decomposition (tree_decomposition, control_flow_graph);
  nicify (tree_decomposition);

  for (bool change = true; change;)
    {
      change = false;

      std::set<int> candidate_set;
      get_candidate_set (&candidate_set, sic);

      std::set<int>::iterator ci, ci_end;
      for (ci = candidate_set.begin(), ci_end = candidate_set.end(); ci != ci_end; ++ci)
        {
          const iCode *ic;
          for (ic = sic; ic && ic->key != *ci; ic = ic->next);

          if (!ic || !candidate_expression (ic))
            continue;

          setup_cfg_for_expression (&control_flow_graph, ic);

          std::cout << "Would look into removing redundancy for ic " << *ci << " now.\n";
        }
    }
}

