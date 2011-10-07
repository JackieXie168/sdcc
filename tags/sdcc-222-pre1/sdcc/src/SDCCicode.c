/*-------------------------------------------------------------------------

  SDCCicode.c - intermediate code generation etc.                  
                Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1998)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   
   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!  
-------------------------------------------------------------------------*/

#include "common.h"

/*-----------------------------------------------------------------*/
/* global variables       */

set *iCodeChain = NULL ;
int iTempNum = 0;
int iTempLblNum = 0;
int operandKey = 0 ;
int iCodeKey = 0;
char *filename ;
int lineno ;
int block;
int scopeLevel;
int lvaluereq;

symbol *returnLabel ; /* function return label */
symbol *entryLabel  ; /* function entry  label */
/*-----------------------------------------------------------------*/
/* forward definition of some functions */
operand *geniCodeDivision (operand *,operand *);
operand *geniCodeAssign   (operand *,operand *,int);
operand *geniCodeArray (operand *,operand *);
operand *geniCodeArray2Ptr (operand *);
operand *geniCodeRValue (operand *, bool );
operand *geniCodeDerefPtr (operand *);

#define PRINTFUNC(x) void x (FILE *of, iCode *ic, char *s)
/* forward definition of print functions */
PRINTFUNC(picGetValueAtAddr);
PRINTFUNC(picSetValueAtAddr);
PRINTFUNC(picAddrOf);
PRINTFUNC(picGeneric);
PRINTFUNC(picGenericOne);
PRINTFUNC(picCast);
PRINTFUNC(picAssign);
PRINTFUNC(picLabel);
PRINTFUNC(picGoto);
PRINTFUNC(picIfx);
PRINTFUNC(picJumpTable);
PRINTFUNC(picInline);
PRINTFUNC(picReceive);

iCodeTable codeTable[] = {
  { '!'                 ,  "not", picGenericOne    , NULL },
  { '~'                 ,  "~"  , picGenericOne    , NULL },
  { RRC                 ,  "rrc", picGenericOne    , NULL },
  { RLC                 ,  "rlc", picGenericOne    , NULL },
  { GETHBIT             ,"ghbit", picGenericOne    , NULL },
  { UNARYMINUS          ,  "-"  , picGenericOne    , NULL },
  { IPUSH               ,  "push",picGenericOne    , NULL },
  { IPOP                ,  "pop", picGenericOne    , NULL },
  { CALL                ,  "call",picGenericOne    , NULL },
  { PCALL               , "pcall",picGenericOne    , NULL }, 
  { FUNCTION            , "proc", picGenericOne    , NULL },
  { ENDFUNCTION         ,"eproc", picGenericOne    , NULL },
  { RETURN              ,  "ret", picGenericOne    , NULL },
  { '+'                 ,  "+"  , picGeneric       , NULL },
  { '-'                 ,  "-"  , picGeneric       , NULL },
  { '*'                 ,  "*"  , picGeneric       , NULL },
  { '/'                 ,  "/"  , picGeneric       , NULL },
  { '%'                 ,  "%"  , picGeneric       , NULL },
  { '>'                 ,  ">"  , picGeneric       , NULL },
  { '<'                 ,  "<"  , picGeneric       , NULL },
  { LE_OP               ,  "<=" , picGeneric       , NULL },
  { GE_OP               ,  ">=" , picGeneric       , NULL },
  { EQ_OP               ,  "==" , picGeneric       , NULL },
  { NE_OP               ,  "!=" , picGeneric       , NULL },  
  { AND_OP              ,  "&&" , picGeneric       , NULL },
  { OR_OP               ,  "||" , picGeneric       , NULL },
  { '^'                 ,  "^"  , picGeneric       , NULL },
  { '|'                 ,  "|"  , picGeneric       , NULL },
  { BITWISEAND          ,  "&"  , picGeneric       , NULL },
  { LEFT_OP             ,  "<<" , picGeneric       , NULL },
  { RIGHT_OP            ,  ">>" , picGeneric       , NULL },
  { GET_VALUE_AT_ADDRESS,  "@"  , picGetValueAtAddr, NULL },
  { ADDRESS_OF          ,  "&"  , picAddrOf        , NULL },
  { CAST                ,  "<>" , picCast          , NULL },
  { '='                 ,  ":=" , picAssign        , NULL },
  { LABEL               ,  ""   , picLabel         , NULL },
  { GOTO                ,  ""   , picGoto          , NULL },
  { JUMPTABLE           ,"jtab" , picJumpTable     , NULL },
  { IFX                 ,  "if" , picIfx           , NULL },
  { INLINEASM           , ""    , picInline        , NULL },
  { RECEIVE             , "recv", picReceive       , NULL },
  { SEND                , "send", picGenericOne    , NULL }
};


/*-----------------------------------------------------------------*/
/* operandName - returns the name of the operand                   */
/*-----------------------------------------------------------------*/
int printOperand (operand *op, FILE *file)
{
    link *opetype;
    int pnl = 0;

    if (!op)
	return 1;

    if (!file) {
	file = stdout;
	pnl = 1;
    }
    switch (op->type) {
	
    case VALUE:
	opetype = getSpec (operandType(op));
	if (SPEC_NOUN(opetype) == V_FLOAT)
	    fprintf (file,"%g {", SPEC_CVAL(opetype).v_float);
	else 
	    fprintf (file,"0x%x {",(int) floatFromVal(op->operand.valOperand));
	printTypeChain(operandType(op),file);
	fprintf(file,"}");
	break;
	
    case SYMBOL :
#define REGA 1
#ifdef REGA	
	fprintf (file,"%s [k%d lr%d:%d so:%d]{ ia%d re%d rm%d}",/*{ar%d rm%d ru%d p%d a%d u%d i%d au%d k%d ks%d}"  ,*/
		 (OP_SYMBOL(op)->rname[0] ? OP_SYMBOL(op)->rname : OP_SYMBOL(op)->name), 
		 op->key,
		 OP_LIVEFROM(op),OP_LIVETO(op),
		 OP_SYMBOL(op)->stack,
		 op->isaddr, OP_SYMBOL(op)->isreqv,OP_SYMBOL(op)->remat
		 );
	{
	    fprintf(file,"{"); printTypeChain(operandType(op),file); 
	    if (SPIL_LOC(op) && IS_ITEMP(op))
		fprintf(file,"}{ sir@ %s",SPIL_LOC(op)->rname);
	    fprintf(file,"}");

	}
	
	/* if assigned to registers */
	if (OP_SYMBOL(op)->nRegs) {
	    if (OP_SYMBOL(op)->isspilt) {
		if (!OP_SYMBOL(op)->remat)
		    if (OP_SYMBOL(op)->usl.spillLoc)
			fprintf(file,"[%s]",(OP_SYMBOL(op)->usl.spillLoc->rname[0] ?
					     OP_SYMBOL(op)->usl.spillLoc->rname :
					     OP_SYMBOL(op)->usl.spillLoc->name));
		    else
			fprintf(file,"[err]");
		else
		    fprintf(file,"[remat]");
	    }
	    else {
		int i;
		fprintf(file,"[");
		for(i=0;i<OP_SYMBOL(op)->nRegs;i++)
		    fprintf(file,"%s ", port->getRegName(OP_SYMBOL(op)->regs[i]));
		fprintf(file,"]");
	    }
	} 
#else
	fprintf(file,"%s",(OP_SYMBOL(op)->rname[0] ?
			   OP_SYMBOL(op)->rname : OP_SYMBOL(op)->name));
	/* if assigned to registers */
	if (OP_SYMBOL(op)->nRegs && !OP_SYMBOL(op)->isspilt) {
	    int i;
	    fprintf(file,"[");
	    for(i=0;i<OP_SYMBOL(op)->nRegs;i++)
		fprintf(file,"%s ",(OP_SYMBOL(op)->regs[i] ? 
				    OP_SYMBOL(op)->regs[i]->name :
				    "err"));
	    fprintf(file,"]");
	} 
#endif
	break ;
	
    case TYPE:
	fprintf(file,"(");
	printTypeChain(op->operand.typeOperand,file);
	fprintf(file,")");
	break;
    }
    
    if (pnl)
	fprintf(file,"\n");
    return 0;
}


/*-----------------------------------------------------------------*/
/*                    print functions                              */
/*-----------------------------------------------------------------*/
PRINTFUNC(picGetValueAtAddr)
{
    fprintf(of,"\t");
    printOperand (IC_RESULT(ic),of);
    fprintf (of," = ");
    fprintf (of,"@[");
    printOperand (IC_LEFT(ic), of);
    fprintf (of,"]");
    
    fprintf(of,"\n");
}

PRINTFUNC(picSetValueAtAddr)
{
    fprintf(of,"\t");
    fprintf(of,"*[");
    printOperand(IC_LEFT(ic),of);
    fprintf(of,"] = ");
    printOperand(IC_RIGHT(ic),of);
    fprintf (of,"\n");
}

PRINTFUNC(picAddrOf)
{
    fprintf(of,"\t");
    printOperand(IC_RESULT(ic),of);
    if (IS_ITEMP(IC_LEFT(ic)))
	fprintf(of," = ");
    else
	fprintf(of," = &[");
    printOperand(IC_LEFT(ic),of);
    if (IC_RIGHT(ic)) {
	if (IS_ITEMP(IC_LEFT(ic)))
	    fprintf(of," offsetAdd ");
	else
	    fprintf(of," , ");
	printOperand(IC_RIGHT(ic),of);
    }
    if (IS_ITEMP(IC_LEFT(ic)))
	fprintf (of,"\n");
    else
	fprintf (of,"]\n");
}

PRINTFUNC(picJumpTable)
{
    symbol *sym;

    fprintf(of,"\t");
    fprintf(of,"%s\t",s);
    printOperand(IC_JTCOND(ic),of);
    fprintf(of,"\n");
    for ( sym = setFirstItem(IC_JTLABELS(ic)); sym;
	  sym = setNextItem(IC_JTLABELS(ic))) 
	fprintf(of,"\t\t\t%s\n",sym->name);
}

PRINTFUNC(picGeneric)
{
    fprintf(of,"\t");
    printOperand(IC_RESULT(ic),of);
    fprintf(of," = ");
    printOperand(IC_LEFT(ic),of);
    fprintf(of," %s ",s);
    printOperand(IC_RIGHT(ic),of);
    fprintf(of,"\n");
}

PRINTFUNC(picGenericOne)
{
    fprintf(of,"\t");
    if ( IC_RESULT(ic) ) {
	printOperand(IC_RESULT(ic),of);
	fprintf (of," = ");
    }
    
    if (IC_LEFT(ic)) {
	fprintf (of,"%s ",s);
	printOperand(IC_LEFT(ic),of);     
    }
    
    if (! IC_RESULT(ic) && !IC_LEFT(ic))
	fprintf (of,s);
    
    fprintf(of,"\n");
}

PRINTFUNC(picCast)
{
    fprintf(of,"\t");
    printOperand(IC_RESULT(ic),of);
    fprintf (of," = ");
    printOperand(IC_LEFT(ic),of);
    printOperand(IC_RIGHT(ic),of);
    fprintf(of,"\n");
}


PRINTFUNC(picAssign)
{
    fprintf(of,"\t");
    
    if (IC_RESULT(ic)->isaddr && IS_ITEMP(IC_RESULT(ic)))
	fprintf(of,"*(");
    
    printOperand(IC_RESULT(ic),of);  
    
    if (IC_RESULT(ic)->isaddr && IS_ITEMP(IC_RESULT(ic)))
	fprintf(of,")");
    
    fprintf(of," %s ", s);
    printOperand (IC_RIGHT(ic),of);
    
    fprintf(of,"\n");
}

PRINTFUNC(picLabel)
{
    fprintf(of," %s($%d) :\n",IC_LABEL(ic)->name,IC_LABEL(ic)->key);
}

PRINTFUNC(picGoto)
{
   fprintf(of,"\t");
   fprintf (of," goto %s($%d)\n", IC_LABEL(ic)->name,IC_LABEL(ic)->key);
}

PRINTFUNC(picIfx)
{    
    fprintf(of,"\t");
    fprintf (of,"if ");
    printOperand(IC_COND(ic),of);
    
    if ( ! IC_TRUE(ic) ) 
	fprintf (of," == 0 goto %s($%d)\n",IC_FALSE(ic)->name,IC_FALSE(ic)->key);
    else {
	fprintf (of," != 0 goto %s($%d)\n",IC_TRUE(ic)->name,IC_TRUE(ic)->key);
	if (IC_FALSE(ic))
	    fprintf (of,"\tzzgoto %s\n",IC_FALSE(ic)->name);
    }
}

PRINTFUNC(picInline)
{
    fprintf(of,"%s",IC_INLINE(ic));
}

PRINTFUNC(picReceive)
{
    printOperand(IC_RESULT(ic),of);
    fprintf(of," = %s ",s);
    printOperand(IC_LEFT(ic),of);
    fprintf(of,"\n");
}

/*-----------------------------------------------------------------*/
/* piCode - prints one iCode                                       */
/*-----------------------------------------------------------------*/
int piCode (void *item, FILE *of)
{
    iCode *ic = item;
    iCodeTable *icTab ;
    
    if (!of)
	of = stdout;

    icTab = getTableEntry(ic->op) ;
    fprintf(stdout,"%s(%d:%d:%d:%d:%d)\t",
		    ic->filename,ic->lineno,
		    ic->seq,ic->key,ic->depth,ic->supportRtn);
    icTab->iCodePrint(of,ic,icTab->printName);
    return 1;
}

/*-----------------------------------------------------------------*/
/* printiCChain - prints intermediate code for humans              */
/*-----------------------------------------------------------------*/
void printiCChain (iCode *icChain, FILE *of)
{
    iCode *loop ;
    iCodeTable *icTab ;

    if (!of)
	of = stdout;
    for ( loop = icChain ; loop ; loop = loop->next ) {
	if ((icTab = getTableEntry (loop->op ))) {
	    fprintf(of,"%s(%d:%d:%d:%d:%d)\t",
		    loop->filename,loop->lineno,
		    loop->seq,loop->key,loop->depth,loop->supportRtn);

	    icTab->iCodePrint (of,loop,icTab->printName);
	}
    }
}


/*-----------------------------------------------------------------*/
/* newOperand - allocate, init & return a new iCode                */
/*-----------------------------------------------------------------*/
operand *newOperand ()
{
    operand *op ;
    
    ALLOC(op,sizeof(operand));
    
    op->key = 0 ;
    return op;
}

/*-----------------------------------------------------------------*/
/* newiCode - create and return a new iCode entry initialised      */
/*-----------------------------------------------------------------*/
iCode *newiCode (int op, operand *left, operand *right)
{
    iCode *ic ;
    
    ALLOC(ic,sizeof(iCode));
   
    ic->lineno = lineno ;
    ic->filename= filename ;
    ic->block = block;
    ic->level = scopeLevel;
    ic->op = op;
    ic->key= iCodeKey++ ;
    IC_LEFT(ic) = left;
    IC_RIGHT(ic)= right;

    return ic;
}     	

/*-----------------------------------------------------------------*/
/* newiCode for conditional statements                             */
/*-----------------------------------------------------------------*/
iCode *newiCodeCondition (operand *condition,
			  symbol  *trueLabel, 
			  symbol  *falseLabel )
{
    iCode *ic ;
    
    ic = newiCode(IFX,NULL,NULL);
    IC_COND(ic) = condition ;
    IC_TRUE(ic) = trueLabel ;
    IC_FALSE(ic) = falseLabel;
    return ic;
}

/*-----------------------------------------------------------------*/
/* newiCodeLabelGoto - unconditional goto statement| label stmnt   */
/*-----------------------------------------------------------------*/
iCode *newiCodeLabelGoto (int op, symbol *label)
{
    iCode *ic ;
    
    ic = newiCode(op,NULL,NULL);
    ic->op = op ;
    ic->argLabel.label = label ;
    IC_LEFT(ic) = NULL ;
    IC_RIGHT(ic) = NULL ;
    IC_RESULT(ic) = NULL ;
    return ic;
}

/*-----------------------------------------------------------------*/
/* newiTemp - allocate & return a newItemp Variable                */
/*-----------------------------------------------------------------*/
symbol *newiTemp (char *s)
{ 
    symbol *itmp;
    
    if (s) 
	sprintf(buffer,"%s",s);
    else
	sprintf (buffer,"iTemp%d",iTempNum++);  
    itmp =  newSymbol (buffer,1);
    strcpy(itmp->rname,itmp->name);
    itmp->isitmp = 1;
    
    return itmp;
}

/*-----------------------------------------------------------------*/
/* newiTempLabel - creates a temp variable label                   */
/*-----------------------------------------------------------------*/
symbol *newiTempLabel (char *s)
{
    symbol *itmplbl;

    /* check if this alredy exists */
    if (s && (itmplbl = findSym(LabelTab, NULL, s)))
	return itmplbl ;

    if (s) 
	itmplbl = newSymbol(s,1);
    else {
	sprintf(buffer,"iTempLbl%d",iTempLblNum++);
	itmplbl = newSymbol(buffer,1);	
    }
    
    itmplbl->isitmp = 1;
    itmplbl->islbl = 1;
    itmplbl->key = labelKey++ ;
    addSym (LabelTab, itmplbl, itmplbl->name,0,0);
    return itmplbl ;  
}

/*-----------------------------------------------------------------*/
/* newiTempPreheaderLabel - creates a new preheader label          */
/*-----------------------------------------------------------------*/
symbol *newiTempPreheaderLabel()
{
    symbol *itmplbl ;

    sprintf(buffer,"preHeaderLbl%d",iTempLblNum++);
    itmplbl = newSymbol(buffer,1);    
    
    itmplbl->isitmp = 1;
    itmplbl->islbl = 1;
    itmplbl->key = labelKey++ ;
    addSym (LabelTab, itmplbl, itmplbl->name,0,0);
    return itmplbl ;  
}


/*-----------------------------------------------------------------*/
/* initiCode - initialises some iCode related stuff                */
/*-----------------------------------------------------------------*/
void initiCode ()
{

}

/*-----------------------------------------------------------------*/
/* copyiCode - make a copy of the iCode given                      */
/*-----------------------------------------------------------------*/
iCode *copyiCode (iCode *ic)
{
    iCode *nic = newiCode(ic->op,NULL,NULL);

    nic->lineno = ic->lineno ;
    nic->filename= ic->filename ;
    nic->block = ic->block;
    nic->level = ic->level;

    /* deal with the special cases first */
    switch (ic->op) {
    case IFX:
	IC_COND(nic) = operandFromOperand(IC_COND(ic));
	IC_TRUE(nic) = IC_TRUE(ic);
	IC_FALSE(nic)= IC_FALSE(ic);
	break;

    case JUMPTABLE:
	IC_JTCOND(nic) = operandFromOperand(IC_JTCOND(ic));
	IC_JTLABELS(nic) = IC_JTLABELS(ic);
	break;

    case CALL:
    case PCALL:
	IC_RESULT(nic) = operandFromOperand(IC_RESULT(ic));
	IC_LEFT(nic)   = operandFromOperand(IC_LEFT(ic));
	IC_ARGS(nic)   = IC_ARGS(ic);
	break;

    case INLINEASM:
	IC_INLINE(nic) = IC_INLINE(ic);
	break;
	
    default:
	IC_RESULT(nic) = operandFromOperand(IC_RESULT(ic));
	IC_LEFT(nic) = operandFromOperand(IC_LEFT(ic));
	IC_RIGHT(nic)= operandFromOperand(IC_RIGHT(ic));
    }

    return nic;
}

/*-----------------------------------------------------------------*/
/* getTableEntry - gets the table entry for the given operator     */
/*-----------------------------------------------------------------*/
iCodeTable *getTableEntry (int oper )
{
    int i ;
    
    for ( i = 0 ; i < (sizeof(codeTable)/sizeof(iCodeTable)); i++ ) 
	if (oper == codeTable[i].icode)
	    return &codeTable[i] ;
    
    return NULL ;
}

/*-----------------------------------------------------------------*/
/* newiTempOperand - new intermediate temp operand                 */
/*-----------------------------------------------------------------*/
operand *newiTempOperand (link *type, char throwType)
{
    symbol *itmp;
    operand *op = newOperand();
    link *etype;

    op->type = SYMBOL ;
    itmp = newiTemp(NULL);

    etype = getSpec(type);

    if (IS_LITERAL(etype) )
	throwType = 0 ;

    /* copy the type information */
    if (type) 
	itmp->etype = getSpec (itmp->type = (throwType ? type :
					     copyLinkChain(type)));
    if (IS_LITERAL(itmp->etype)) {
	SPEC_SCLS(itmp->etype) = S_REGISTER ;
	SPEC_OCLS(itmp->etype) = reg;
    }
	
    op->operand.symOperand = itmp;
    op->key = itmp->key = ++operandKey ;
    return op;
}

/*-----------------------------------------------------------------*/
/* operandType - returns the type chain for an operand             */
/*-----------------------------------------------------------------*/
link *operandType (operand *op) 
{
    /* depending on type of operand */
    switch (op->type) {
	
    case VALUE :
	return op->operand.valOperand->type ;
	
    case SYMBOL:
	return op->operand.symOperand->type ;
	
    case TYPE :
	return op->operand.typeOperand ;
    default:
	werror (E_INTERNAL_ERROR,__FILE__,__LINE__,
		" operand type not known ");
	assert (0) ; /* should never come here */
	/*  Just to keep the compiler happy */
	return (link *)0;
    }
}

/*-----------------------------------------------------------------*/
/* isParamterToCall - will return 1 if op is a parameter to args   */
/*-----------------------------------------------------------------*/
int isParameterToCall (value *args, operand *op)
{
    value *tval = args ;

    while (tval) {
	if (tval->sym && 
	    isSymbolEqual(op->operand.symOperand,tval->sym))
	    return 1;
	tval = tval->next ;
    }
    return 0;
}

/*-----------------------------------------------------------------*/
/* isOperandGlobal   - return 1 if operand is a global variable    */
/*-----------------------------------------------------------------*/
int isOperandGlobal ( operand *op )
{
    if (!op)
	return 0;

    if (IS_ITEMP(op))
	return 0;

    if (op->type == SYMBOL &&       
	 (op->operand.symOperand->level == 0 ||	 
	 IS_STATIC(op->operand.symOperand->etype) ||
	 IS_EXTERN(op->operand.symOperand->etype))
	  )
	return 1;
    
    return 0;
}

/*-----------------------------------------------------------------*/
/* isOperandVolatile - return 1 if the operand is volatile         */
/*-----------------------------------------------------------------*/
int isOperandVolatile ( operand *op , bool chkTemp)
{
    link *optype ;
    link *opetype ;

    if (IS_ITEMP(op) && !chkTemp)
	return 0;

    opetype = getSpec(optype = operandType(op));
    
    if (IS_PTR(optype) && DCL_PTR_VOLATILE(optype))
	return 1;

    if (IS_VOLATILE(opetype))
	return 1;
    return 0;
}

/*-----------------------------------------------------------------*/
/* isOperandLiteral - returns 1 if an operand contains a literal   */
/*-----------------------------------------------------------------*/
int isOperandLiteral ( operand *op )
{
    link *opetype ;
    
    if (!op)
	return 0;
    
    opetype = getSpec (operandType(op));

    if (IS_LITERAL(opetype))
	return 1;

    return 0;
}
/*-----------------------------------------------------------------*/
/* isOperandInFarSpace - will return true if operand is in farSpace*/
/*-----------------------------------------------------------------*/
bool isOperandInFarSpace (operand *op)
{
    link *etype;

    if (!op)
	return FALSE;

    if (!IS_SYMOP(op))
	return FALSE ;

    if (!IS_TRUE_SYMOP(op)) {
	if (SPIL_LOC(op))
	    etype = SPIL_LOC(op)->etype;
	else		
	    return FALSE;
    }
    else
    {
    	etype = getSpec(operandType(op));
    }
    return (IN_FARSPACE(SPEC_OCLS(etype)) ? TRUE : FALSE);
}

/*-----------------------------------------------------------------*/
/* isOperandOnStack - will return true if operand is on stack      */
/*-----------------------------------------------------------------*/
bool isOperandOnStack(operand *op)
{
    link *etype;

    if (!op)
	return FALSE;

    if (!IS_SYMOP(op))
	return FALSE ;

    etype = getSpec(operandType(op));

    return ((IN_STACK(etype)) ? TRUE : FALSE);
}

/*-----------------------------------------------------------------*/
/* operandLitValue - literal value of an operand                   */
/*-----------------------------------------------------------------*/
double operandLitValue ( operand *op )
{
    assert(isOperandLiteral(op));
    
    return floatFromVal(op->operand.valOperand);    
}

/*-----------------------------------------------------------------*/
/* operandOperation - perforoms operations on operands             */
/*-----------------------------------------------------------------*/
operand *operandOperation (operand *left,operand *right,
			   int op, link *type)
{
    operand *retval = (operand *)0;
        
    assert(isOperandLiteral(left));
    if (right) 
	assert(isOperandLiteral(right));
    
    switch (op) {
    case '+' :
	retval =  operandFromValue (valCastLiteral(type,
						   operandLitValue(left) + 
						   operandLitValue(right)));
	break ;
    case '-' :
	retval = operandFromValue(valCastLiteral(type,
						 operandLitValue(left) -
						 operandLitValue(right)));
	break;
    case '*':
	retval = operandFromValue(valCastLiteral(type,
						 operandLitValue(left) *
						 operandLitValue(right)));
	break;
    case '/':
	if ((unsigned long) operandLitValue(right) == 0){	    
	    werror(E_DIVIDE_BY_ZERO);
	    retval = right;
	    
	}
	else
	    retval = operandFromValue (valCastLiteral(type,
						      operandLitValue(left) /
						      operandLitValue(right)));
	break;
    case '%':      
	if ((unsigned long) operandLitValue(right) == 0){	    
	    werror(E_DIVIDE_BY_ZERO);
	    retval = right;	    
	}
	else
	    retval = operandFromLit ((unsigned long) operandLitValue(left) %
				     (unsigned long) operandLitValue(right));
	break;
    case LEFT_OP :
	retval = operandFromLit ((unsigned long) operandLitValue(left) <<
				 (unsigned long) operandLitValue(right));
	break;
    case RIGHT_OP :
	retval = operandFromLit ((unsigned long) operandLitValue(left) >>
				 (unsigned long) operandLitValue(right));
	break;
    case EQ_OP :
	retval = operandFromLit (operandLitValue(left) ==
				 operandLitValue(right));
	break;
    case '<' :
	retval = operandFromLit (operandLitValue(left) <
				 operandLitValue(right));
	break;
    case LE_OP :
	retval = operandFromLit (operandLitValue(left) <=
				 operandLitValue(right));
	break;
    case NE_OP :
	retval = operandFromLit (operandLitValue(left) !=
				 operandLitValue(right));
	break;
    case '>' :
	retval = operandFromLit (operandLitValue(left) >
				 operandLitValue(right));
	break;
    case GE_OP :
	retval = operandFromLit (operandLitValue(left) >=
				 operandLitValue(right));
	break;
    case BITWISEAND :
	retval = operandFromLit ((unsigned long) operandLitValue(left) &
				 (unsigned long) operandLitValue(right));
	break;
    case '|' :
	retval = operandFromLit ((unsigned long) operandLitValue(left) |
				 (unsigned long) operandLitValue(right));	
	break;
    case '^' :
	retval = operandFromLit ((unsigned long) operandLitValue(left) ^
				 (unsigned long) operandLitValue(right));
	break;
    case AND_OP:
	retval = operandFromLit (operandLitValue(left) &&
				 operandLitValue(right));
	break;
    case OR_OP:
	retval = operandFromLit (operandLitValue(left) ||
				 operandLitValue(right));
	break;
    case RRC:
	{
	    long i = operandLitValue(left);
	    
	    retval = operandFromLit ((i >> (getSize(operandType(left))*8 - 1)) |
				     (i << 1));
	}
	break;
    case RLC:
	{
	    long i = operandLitValue(left);
	    
	    retval = operandFromLit ((i << (getSize(operandType(left))*8 - 1)) |
				     (i >> 1));
	}
	break;
	
    case UNARYMINUS:
	retval = operandFromLit(-1 * operandLitValue(left));
	break;
	
    case '~':
	retval = operandFromLit(~ ((long) operandLitValue(left)));
	break;

    case '!':
	retval = operandFromLit(! operandLitValue(left));
	break;

    default :
	werror(E_INTERNAL_ERROR,__FILE__,__LINE__,
	       " operandOperation invalid operator ");
	assert (0);
    }
    
    return retval;
}


/*-----------------------------------------------------------------*/
/* isOperandEqual - compares two operand & return 1 if they r =    */
/*-----------------------------------------------------------------*/
int isOperandEqual (operand *left, operand *right)
{
    /* if the pointers are equal then they are equal */
    if ( left == right )
	return 1;
    
    /* if either of them null then false */
    if ( !left || !right)
	return 0;

    if (left->type != right->type)
	return 0;

    if (IS_SYMOP(left) && IS_SYMOP(right))
	return left->key == right->key ;

    /* if types are the same */
    switch (left->type) {
    case SYMBOL :
	return isSymbolEqual(left->operand.symOperand,
			     right->operand.symOperand);
    case VALUE :
	return (floatFromVal(left->operand.valOperand) ==
		floatFromVal(right->operand.valOperand));
    case TYPE :
	if (checkType(left->operand.typeOperand,
		      right->operand.typeOperand) == 1)
	    return 1;      
    }

    return 0;
}

/*-----------------------------------------------------------------*/
/* isiCodeEqual - comapres two iCodes are returns true if yes      */
/*-----------------------------------------------------------------*/
int isiCodeEqual (iCode *left, iCode *right)
{
    /* if the same pointer */
    if (left == right)
	return 1;
    
    /* if either of them null */
    if (!left || !right)
	return 0;

    /* if operand are the same */
    if ( left->op == right->op ) {
	
	/* compare all the elements depending on type */
	if (left->op != IFX ) {
	    if (!isOperandEqual(IC_LEFT(left),IC_LEFT(right)))
		return 0;
	    if (!isOperandEqual(IC_RIGHT(left),IC_RIGHT(right)))
		return 0;

	} else {
	    if (!isOperandEqual(IC_COND(left),IC_COND(right)))
		return 0;
	    if (!isSymbolEqual (IC_TRUE(left),IC_TRUE(right)))
		return 0;
	    if (!isSymbolEqual(IC_FALSE(left),IC_FALSE(right)))
		return 0;
	}
	return 1;
    }
    return 0;
}

/*-----------------------------------------------------------------*/
/* newiTempFromOp - create a temp Operand with same attributes     */
/*-----------------------------------------------------------------*/
operand *newiTempFromOp (operand *op)
{
    operand *nop;

    if (!op)
	return NULL;
    
    if (!IS_ITEMP(op))
	return op;

    nop = newiTempOperand(operandType(op),TRUE);
    nop->isaddr = op->isaddr ;
    nop->isvolatile = op->isvolatile ;
    nop->isGlobal = op->isGlobal ;
    nop->isLiteral= op->isLiteral ;
    nop->noSpilLoc= op->noSpilLoc;
    nop->usesDefs = op->usesDefs;
    nop->isParm = op->isParm;
    nop->parmBytes = op->parmBytes;
    return nop;
}

/*-----------------------------------------------------------------*/
/* operand from operand - creates an operand holder for the type   */
/*-----------------------------------------------------------------*/
operand *operandFromOperand (operand *op)
{
    operand *nop ;
    
    if (!op)
	return NULL;
    nop = newOperand();
    nop->type = op->type;
    nop->isaddr = op->isaddr ;
    nop->key = op->key ;
    nop->isvolatile = op->isvolatile ;
    nop->isGlobal = op->isGlobal ;
    nop->isLiteral= op->isLiteral ;
    nop->noSpilLoc= op->noSpilLoc;
    nop->usesDefs = op->usesDefs;
    nop->isParm = op->isParm;
    nop->parmBytes = op->parmBytes;

    switch (nop->type) {
    case SYMBOL :
	nop->operand.symOperand = op->operand.symOperand ;	
	break;
    case VALUE :
	nop->operand.valOperand = op->operand.valOperand;
	break;
    case TYPE :
	nop->operand.typeOperand = op->operand.typeOperand ;
	break ;
    }   

    return nop;
}

/*-----------------------------------------------------------------*/
/* opFromOpWithDU - makes a copy of the operand and DU chains      */
/*-----------------------------------------------------------------*/
operand *opFromOpWithDU (operand *op, bitVect *defs, bitVect *uses)
{
    operand *nop = operandFromOperand(op);

    if (nop->type == SYMBOL) {
	OP_SYMBOL(nop)->defs = bitVectCopy(defs);
	OP_SYMBOL(nop)->uses = bitVectCopy(uses);
    }

    return nop;
}

/*-----------------------------------------------------------------*/
/* operandFromSymbol - creates an operand from a symbol            */
/*-----------------------------------------------------------------*/
operand *operandFromSymbol (symbol *sym)
{
    operand *op ;
    iCode *ic ;
    int ok =1 ;
    /* if the symbol's type is a literal */
    /* then it is an enumerator type     */
    if (IS_LITERAL(sym->etype) && SPEC_ENUM(sym->etype)) 
	return operandFromValue (valFromType(sym->etype));

    if (!sym->key)
	sym->key = ++operandKey ;

    /* if this an implicit variable, means struct/union */
    /* member so just return it                         */
    if (sym->implicit || IS_FUNC(sym->type)) {
	op = newOperand();
	op->type = SYMBOL ;
	op->operand.symOperand = sym;
	op->key = sym->key ;
	op->isvolatile = isOperandVolatile(op,TRUE);
	op->isGlobal   = isOperandGlobal(op);
	op->parmBytes  = sym->argStack;
	return op;
    }
    
    /* under the following conditions create a
       register equivalent for a local symbol */
    if (sym->level && sym->etype && SPEC_OCLS(sym->etype) &&
	(IN_FARSPACE(SPEC_OCLS(sym->etype)) && (!IS_DS390_PORT)) &&
	options.stackAuto == 0)
	ok =0;

    if (!IS_AGGREGATE(sym->type) &&     /* not an aggregate */
	!IS_FUNC(sym->type)      &&     /* not a function   */
	!sym->_isparm            &&     /* not a parameter  */
	sym->level               &&     /* is a local variable */
	!sym->addrtaken          &&     /* whose address has not been taken */
	!sym->reqv               &&     /* does not already have a register euivalence */
	!IS_VOLATILE(sym->etype) &&     /* not declared as volatile */
	!IS_STATIC(sym->etype)   &&     /* and not declared static  */
	!sym->islbl              &&     /* not a label */
	ok                       &&     /* farspace check */
	!IS_BITVAR(sym->etype)          /* not a bit variable */
	) {
	
	/* we will use it after all optimizations
	   and before liveRange calculation */
	sym->reqv = newiTempOperand(sym->type,0);	
	sym->reqv->key = sym->key ;
	OP_SYMBOL(sym->reqv)->key = sym->key;
	OP_SYMBOL(sym->reqv)->isreqv = 1;
	OP_SYMBOL(sym->reqv)->islocal = 1;
	SPIL_LOC(sym->reqv) = sym;
    }
   
    if (!IS_AGGREGATE(sym->type)) {
	op = newOperand();
	op->type = SYMBOL;
	op->operand.symOperand = sym;
	op->isaddr = 1;
	op->key = sym->key;
	op->isvolatile = isOperandVolatile(op,TRUE);
	op->isGlobal   = isOperandGlobal(op);
	op->isPtr = IS_PTR(operandType(op));
	op->isParm = sym->_isparm ;
	return op;
    }
    
    /* create :-                     */
    /*    itemp = &[_symbol]         */
    
    ic = newiCode(ADDRESS_OF,newOperand(),NULL);
    IC_LEFT(ic)->type = SYMBOL ;
    IC_LEFT(ic)->operand.symOperand = sym ;
    IC_LEFT(ic)->key = sym->key;    
    (IC_LEFT(ic))->isvolatile = isOperandVolatile(IC_LEFT(ic),TRUE);
    (IC_LEFT(ic))->isGlobal   = isOperandGlobal(IC_LEFT(ic));
    IC_LEFT(ic)->isPtr = IS_PTR(operandType(IC_LEFT(ic)));

    /* create result */
    IC_RESULT(ic) = newiTempOperand(sym->type,0);
    if (IS_ARRAY(sym->type)) {
	IC_RESULT(ic) = geniCodeArray2Ptr (IC_RESULT(ic));
	IC_RESULT(ic)->isaddr = 0;
    } else
	IC_RESULT(ic)->isaddr = (!IS_AGGREGATE(sym->type));

    IC_RESULT(ic)->operand.symOperand->args = sym->args;

    ADDTOCHAIN(ic);
    
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* operandFromValue - creates an operand from value                */
/*-----------------------------------------------------------------*/
operand *operandFromValue (value *val)
{
    operand *op ;
    
    /* if this is a symbol then do the symbol thing */
    if (val->sym)
	return operandFromSymbol (val->sym);
    
    /* this is not a symbol */
    op = newOperand();
    op->type = VALUE ;
    op->operand.valOperand = val ;
    op->isLiteral = isOperandLiteral(op);
    return op;
}

/*-----------------------------------------------------------------*/
/* operandFromLink - operand from typeChain                        */
/*-----------------------------------------------------------------*/
operand *operandFromLink (link *type)
{
    operand *op ;
    
    /* operand from link */
    if ( ! type )
	return NULL ;
    
    op = newOperand();
    op->type = TYPE ;
    op->operand.typeOperand = copyLinkChain(type);
    return op;
}

/*-----------------------------------------------------------------*/
/* operandFromLit - makes an operand from a literal value          */
/*-----------------------------------------------------------------*/
operand *operandFromLit ( float i)
{
    return operandFromValue (valueFromLit (i));
}

/*-----------------------------------------------------------------*/
/* operandFromAst - creates an operand from an ast                 */
/*-----------------------------------------------------------------*/
operand *operandFromAst ( ast *tree )
{
    
    if (! tree )
	return NULL ;
    
    /* depending on type do */
    switch (tree->type ) { 	
    case EX_OP : 
	return ast2iCode (tree) ;    
	break ;
	
    case EX_VALUE :
	return operandFromValue(tree->opval.val) ;
	break ;
	
    case EX_LINK :
	return operandFromLink (tree->opval.lnk) ; 
    }
    
    assert(0);
    /*  Just to keep the comiler happy */
    return (operand *)0;
}

/*-----------------------------------------------------------------*/
/* setOperandType - sets the operand's type to the given type      */
/*-----------------------------------------------------------------*/
void setOperandType (operand *op, link *type)
{
    /* depending on the type of operand */
    switch (op->type) {
	
    case VALUE :
	op->operand.valOperand->etype = 
	    getSpec( op->operand.valOperand->type = 
		     copyLinkChain (type )) ;
	return ;
	
    case SYMBOL :
	if (op->operand.symOperand->isitmp )
	    op->operand.symOperand->etype = 
		getSpec( op->operand.symOperand->type = 
			 copyLinkChain (type )) ;
	else
	    werror (E_INTERNAL_ERROR,__FILE__,__LINE__,
		    "attempt to modify type of source");
	return;
	
    case TYPE:
	op->operand.typeOperand = copyLinkChain (type);
	return ;
    }
    
}

/*-----------------------------------------------------------------*/
/* geniCodeValueAtAddress - generate intermeditate code for value  */
/*                          at address                             */
/*-----------------------------------------------------------------*/
operand *geniCodeRValue (operand *op, bool force)
{
    iCode *ic ;
    link *type = operandType(op);
    link *etype= getSpec(type);
    
    /* if this is an array & already */
    /* an address then return this   */
    if (IS_AGGREGATE(type) || 
	(IS_PTR(type) && !force && !op->isaddr))
	return operandFromOperand(op);
        
    /* if this is not an address then must be */
    /* rvalue already so return this one      */
    if (!op->isaddr)
	return op ;
    
    /* if this is not a temp symbol then */
    if (!IS_ITEMP(op) && 
	!force        && 
	!IN_FARSPACE(SPEC_OCLS(etype))) {
	op = operandFromOperand(op);
	op->isaddr = 0;
	return op;
    }
    
    if (IS_SPEC(type) && 
	IS_TRUE_SYMOP(op) &&
	!IN_FARSPACE(SPEC_OCLS(etype))) {
	op = operandFromOperand(op);
	op->isaddr = 0;
	return op;
    }

    ic = newiCode(GET_VALUE_AT_ADDRESS,op,NULL);
    if (IS_PTR(type) && op->isaddr && force) 
	type = type->next;
    
    type = copyLinkChain(type);

    IC_RESULT(ic) = newiTempOperand (type,1);
    IC_RESULT(ic)->isaddr = 0;
 
/*     ic->supportRtn = ((IS_GENPTR(type) | op->isGptr) & op->isaddr); */

    /* if the right is a symbol */
    if (op->type == SYMBOL)
	IC_RESULT(ic)->operand.symOperand->args = 
	    op->operand.symOperand->args ;
    ADDTOCHAIN(ic);
    
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeCast - changes the value from one type to another       */
/*-----------------------------------------------------------------*/
operand *geniCodeCast (link *type, operand *op, bool implicit) 
{
    iCode *ic ;
    link *optype ;
    link *opetype = getSpec(optype = operandType(op));
    link *restype ;
    
    /* one of them has size zero then error */
    if (IS_VOID(optype)) {
	werror(E_CAST_ZERO);
	return op;
    }

    /* if the operand is already the desired type then do nothing */
    if ( checkType (type,optype) == 1)	
	return op;
    
    /* if this is a literal then just change the type & return */
    if (IS_LITERAL(opetype) && op->type == VALUE && !IS_PTR(type) && !IS_PTR(optype))
	return operandFromValue(valCastLiteral(type,
					       operandLitValue(op)));
          
    /* if casting to some pointer type &&
       the destination is not a generic pointer 
       then give a warning : (only for implicit casts)*/
    if (IS_PTR(optype) && implicit &&
	(DCL_TYPE(optype) != DCL_TYPE(type)) && 
	!IS_GENPTR(type)) {
	werror(E_INCOMPAT_CAST);
	werror(E_CONTINUE,"from type '");
	printTypeChain(optype,stderr);fprintf(stderr,"' to type '");      
	printTypeChain(type,stderr);fprintf(stderr,"'\n");
    }

    /* if they are the same size create an assignment */
    if (getSize(type) == getSize(optype) && 
	!IS_BITFIELD(type)               &&
	!IS_FLOAT(type)                  &&
	!IS_FLOAT(optype)                &&
	((IS_SPEC(type) && IS_SPEC(optype)) ||
	 (!IS_SPEC(type) && !IS_SPEC(optype)))) {

	ic = newiCode('=',NULL,op);	
	IC_RESULT(ic) = newiTempOperand(type,0);
	 SPIL_LOC(IC_RESULT(ic))  =
	     (IS_TRUE_SYMOP(op) ? OP_SYMBOL(op) : NULL);
	IC_RESULT(ic)->isaddr = 0;
    } else { 
	ic = newiCode(CAST,operandFromLink(type),
		      geniCodeRValue(op,FALSE));
	
	IC_RESULT(ic)= newiTempOperand(type,0);
    }
    
    /* preserve the storage class & output class */
    /* of the original variable                  */
    restype = getSpec(operandType(IC_RESULT(ic)));
    SPEC_SCLS(restype) = SPEC_SCLS(opetype);
    SPEC_OCLS(restype) = SPEC_OCLS(opetype);
    
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeLabel - will create a Label                             */
/*-----------------------------------------------------------------*/
void geniCodeLabel (symbol *label)
{
    iCode *ic;
    
    ic = newiCodeLabelGoto(LABEL,label);
    ADDTOCHAIN(ic);
}

/*-----------------------------------------------------------------*/
/* geniCodeGoto  - will create a Goto                              */
/*-----------------------------------------------------------------*/
void geniCodeGoto (symbol *label)
{
    iCode *ic;
    
    ic = newiCodeLabelGoto(GOTO,label);
    ADDTOCHAIN(ic);
}

/*-----------------------------------------------------------------*/
/* geniCodeMultiply - gen intermediate code for multiplication     */
/*-----------------------------------------------------------------*/
operand *geniCodeMultiply (operand *left, operand *right)
{ 
    iCode *ic ;
    int p2 = 0;
    link *resType ;
    LRTYPE ;
    
    /* if they are both literal then we know the result */
    if (IS_LITERAL(letype) && IS_LITERAL(retype)) 
	return operandFromValue (valMult(left->operand.valOperand,
					 right->operand.valOperand));
        
    resType = computeType (ltype,rtype) ;
    left = geniCodeCast(resType,left,TRUE);
    right= geniCodeCast(resType,right,TRUE);
    
    /* if the right is a literal & power of 2 */
    /* then make it a left shift              */
    if (IS_LITERAL(retype) && !IS_FLOAT(letype) &&
	(p2 = powof2 ((unsigned long)floatFromVal(right->operand.valOperand)))) 
	ic = newiCode(LEFT_OP, left,operandFromLit(p2)); /* left shift */
    else {
	ic = newiCode('*',left,right);  /* normal multiplication */
	/* if the size left or right > 1 then support routine */
	if (getSize(ltype) > 1 || getSize(rtype) > 1)
	    ic->supportRtn = 1;

    }
    IC_RESULT(ic) = newiTempOperand(resType,1);
    
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeDivision - gen intermediate code for division           */
/*-----------------------------------------------------------------*/
operand *geniCodeDivision (operand *left, operand *right)
{ 
    iCode *ic ;
    int p2 = 0;
    link *resType;
    link *rtype = operandType(right);
    link *retype= getSpec(rtype);
    link *ltype = operandType(left);
    link *letype= getSpec(ltype);
    
    resType = computeType (ltype,rtype) ;
    left = geniCodeCast(resType,left,TRUE);
    right= geniCodeCast(resType,right,TRUE);
    
    /* if the right is a literal & power of 2 */
    /* then make it a right shift             */
    if (IS_LITERAL(retype) && 
	!IS_FLOAT(letype)  &&
	(p2 = powof2 ((unsigned long) 
		      floatFromVal(right->operand.valOperand)))) 
	ic = newiCode(RIGHT_OP, left,operandFromLit(p2)); /* right shift */
    else {
	ic = newiCode('/',left,right);  /* normal division */
	/* if the size left or right > 1 then support routine */
	if (getSize(ltype) > 1 || getSize(rtype) > 1)
	    ic->supportRtn = 1;
    }
    IC_RESULT(ic) = newiTempOperand(resType,0);
    
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}
/*-----------------------------------------------------------------*/
/* geniCodeModulus  - gen intermediate code for modulus            */
/*-----------------------------------------------------------------*/
operand *geniCodeModulus (operand *left, operand *right)
{ 
    iCode *ic ;
    link *resType;
    LRTYPE ;
    
    /* if they are both literal then we know the result */
    if (IS_LITERAL(letype) && IS_LITERAL(retype)) 
	return operandFromValue (valMod(left->operand.valOperand,
					right->operand.valOperand));
    
    resType = computeType (ltype,rtype) ;
    left = geniCodeCast(resType,left,TRUE);
    right= geniCodeCast(resType,right,TRUE);
    
    /* now they are the same size */
    ic = newiCode('%',left,right);

    /* if the size left or right > 1 then support routine */
    if (getSize(ltype) > 1 || getSize(rtype) > 1)
	ic->supportRtn = 1;
    IC_RESULT(ic) = newiTempOperand(resType,0);
    
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodePtrPtrSubtract - subtracts pointer from pointer         */
/*-----------------------------------------------------------------*/
operand *geniCodePtrPtrSubtract (operand *left, operand *right)
{
    iCode *ic ;
    operand *result;
    LRTYPE ;
    
    /* if they are both literals then */
    if (IS_LITERAL(letype) && IS_LITERAL(retype)) {
	result = operandFromValue (valMinus(left->operand.valOperand,
					    right->operand.valOperand));
	goto subtractExit;
    }
    
    ic = newiCode('-',left,right);
    
    IC_RESULT(ic) = result = newiTempOperand(newIntLink(),1);
    ADDTOCHAIN(ic);
    
 subtractExit:
    return geniCodeDivision (result,
			     operandFromLit(getSize(ltype->next)));   
}

/*-----------------------------------------------------------------*/
/* geniCodeSubtract - generates code for subtraction               */
/*-----------------------------------------------------------------*/
operand *geniCodeSubtract (operand *left, operand *right)
{
    iCode *ic ;
    int isarray= 0;
    link *resType;
    LRTYPE ;
    
    /* if they both pointers then */
    if ((IS_PTR(ltype) || IS_ARRAY(ltype)) &&
	(IS_PTR(rtype) || IS_ARRAY(rtype)))
	return geniCodePtrPtrSubtract (left,right);
    
    /* if they are both literal then we know the result */
    if (IS_LITERAL(letype) && IS_LITERAL(retype)
	&& left->isLiteral && right->isLiteral) 
	return operandFromValue (valMinus(left->operand.valOperand,
					  right->operand.valOperand));
    
    /* if left is an array or pointer */
    if ( IS_PTR(ltype) || IS_ARRAY(ltype) ) {    
	isarray = left->isaddr ;    
	right = geniCodeMultiply (right,
				  operandFromLit(getSize(ltype->next)));
	resType = copyLinkChain(IS_ARRAY(ltype) ? ltype->next : ltype);
    }
    else { /* make them the same size */
	resType = computeType (ltype,rtype) ;
	left = geniCodeCast(resType,left,TRUE);
	right= geniCodeCast(resType,right,TRUE);    
    }
    
    ic = newiCode('-',left,right);
    
    IC_RESULT(ic)= newiTempOperand(resType,1);
    IC_RESULT(ic)->isaddr = (isarray ? 1 : 0);

    /* if left or right is a float */
    if (IS_FLOAT(ltype) || IS_FLOAT(rtype))
	ic->supportRtn = 1;

    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeAdd - generates iCode for addition                      */
/*-----------------------------------------------------------------*/
operand *geniCodeAdd (operand *left, operand *right )
{
    iCode *ic ;
    link *resType ;
    operand *size ;
    int isarray = 0;
    LRTYPE ;

    /* if left is an array then array access */
    if (IS_ARRAY(ltype)) 
	return geniCodeArray (left,right);           
    
    /* if the right side is LITERAL zero */
    /* return the left side              */
    if (IS_LITERAL(retype) && right->isLiteral && !floatFromVal(valFromType(retype)))
	return left;
    
    /* if left is literal zero return right */
    if (IS_LITERAL(letype) && left->isLiteral && !floatFromVal(valFromType(letype)))
	return right ;
    
    /* if left is an array or pointer then size */
    if (IS_PTR(ltype)) {    
	
	isarray = left->isaddr;
	size = 
	    operandFromLit(getSize(ltype->next));
	if (getSize(ltype) > 1 && (getSize(rtype) < INTSIZE)) 
	{
	    right = geniCodeCast(INTTYPE,right,TRUE);	    
	}
	right = geniCodeMultiply (right ,size);

	resType = copyLinkChain(ltype);
    }
    else { /* make them the same size */
	resType = computeType (ltype,rtype) ;
	left = geniCodeCast(resType,left,TRUE);
	right= geniCodeCast(resType,right,TRUE);
    }
    
    /* if they are both literals then we know */
    if (IS_LITERAL(letype) && IS_LITERAL(retype)
	&& left->isLiteral && right->isLiteral)
	return operandFromValue (valPlus(valFromType(letype),
					 valFromType(retype)));
    
    ic = newiCode('+',left,right);
    
    IC_RESULT(ic) = newiTempOperand(resType,1);
    IC_RESULT(ic)->isaddr = ( isarray ? 1 : 0);

    /* if left or right is a float then support
       routine */
    if (IS_FLOAT(ltype) || IS_FLOAT(rtype))
	ic->supportRtn = 1;

    ADDTOCHAIN(ic);
    
    return IC_RESULT(ic) ;
    
}

/*-----------------------------------------------------------------*/
/* aggrToPtr - changes an aggregate to pointer to an aggregate     */
/*-----------------------------------------------------------------*/
link *aggrToPtr ( link *type, bool force)
{
    link *etype ;
    link *ptype ;

    
    if (IS_PTR(type) && !force)
	return type;

    etype = getSpec(type);
    ptype = newLink();

    ptype->next = type;
    /* if the output class is generic */
    if ((DCL_TYPE(ptype) = PTR_TYPE(SPEC_OCLS(etype))) == CPOINTER)
	DCL_PTR_CONST(ptype) = port->mem.code_ro;

    /* if the variable was declared a constant */
    /* then the pointer points to a constant */
    if (IS_CONSTANT(etype) )
	DCL_PTR_CONST(ptype) = 1;

    /* the variable was volatile then pointer to volatile */
    if (IS_VOLATILE(etype))
	DCL_PTR_VOLATILE(ptype) = 1;
    return ptype; 
}

/*-----------------------------------------------------------------*/
/* geniCodeArray2Ptr - array to pointer                            */
/*-----------------------------------------------------------------*/
operand *geniCodeArray2Ptr (operand *op)
{
    link *optype = operandType(op);
    link *opetype = getSpec(optype);

    /* set the pointer depending on the storage class */    
    if ((DCL_TYPE(optype) = PTR_TYPE(SPEC_OCLS(opetype))) == CPOINTER)
	DCL_PTR_CONST(optype) = port->mem.code_ro;

    
    /* if the variable was declared a constant */
    /* then the pointer points to a constant */
    if (IS_CONSTANT(opetype) )
	DCL_PTR_CONST(optype) = 1;

    /* the variable was volatile then pointer to volatile */
    if (IS_VOLATILE(opetype))
	DCL_PTR_VOLATILE(optype) = 1;
    op->isaddr = 0;
    return op;
}

/*-----------------------------------------------------------------*/
/* geniCodeArray - array access                                    */
/*-----------------------------------------------------------------*/
operand *geniCodeArray (operand *left,operand *right)
{
    iCode *ic;
    link *ltype = operandType(left);
    
    if (IS_PTR(ltype)) {
	if (IS_PTR(ltype->next) && left->isaddr)
	{
	    left = geniCodeRValue(left,FALSE);
	}
	return geniCodeDerefPtr(geniCodeAdd(left,right));
    }

    /* array access */
    if (getSize(operandType(right)) < INTSIZE) 
    {
        /* Widen the index type to int first. */
	right = geniCodeCast(INTTYPE,right,TRUE);	    
    }
    right = geniCodeMultiply(right,
			     operandFromLit(getSize(ltype->next)));

    /* we can check for limits here */
    if (isOperandLiteral(right) &&
	IS_ARRAY(ltype)         &&
	DCL_ELEM(ltype)         &&
	(operandLitValue(right)/getSize(ltype->next)) >= DCL_ELEM(ltype)) {
	werror(E_ARRAY_BOUND);
	right = operandFromLit(0);
    }

    ic = newiCode('+',left,right);    

    IC_RESULT(ic) = newiTempOperand(((IS_PTR(ltype) && 
				      !IS_AGGREGATE(ltype->next) &&
				      !IS_PTR(ltype->next))
				     ? ltype : ltype->next),0);

    IC_RESULT(ic)->isaddr = (!IS_AGGREGATE(ltype->next));
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeStruct - generates intermediate code for structres      */
/*-----------------------------------------------------------------*/
operand *geniCodeStruct (operand *left, operand *right, bool islval)
{
    iCode *ic ;
    link *type = operandType(left);
    link *etype = getSpec(type);
    link *retype ;
    symbol *element = getStructElement(SPEC_STRUCT(etype), 
				       right->operand.symOperand);
    
    /* add the offset */
    ic = newiCode('+',left,operandFromLit(element->offset));
    
    IC_RESULT(ic) = newiTempOperand(element->type,0);

    /* preserve the storage & output class of the struct */
    /* as well as the volatile attribute */
    retype = getSpec(operandType(IC_RESULT(ic)));
    SPEC_SCLS(retype) = SPEC_SCLS(etype);
    SPEC_OCLS(retype) = SPEC_OCLS(etype);
    SPEC_VOLATILE(retype) |= SPEC_VOLATILE(etype);    

    if (IS_PTR(element->type)) 
	setOperandType(IC_RESULT(ic),aggrToPtr(operandType(IC_RESULT(ic)),TRUE));
    
    IC_RESULT(ic)->isaddr = (!IS_AGGREGATE(element->type));

    
    ADDTOCHAIN(ic);
    return (islval ? IC_RESULT(ic) : geniCodeRValue(IC_RESULT(ic),TRUE));
}

/*-----------------------------------------------------------------*/
/* geniCodePostInc - generate int code for Post increment          */
/*-----------------------------------------------------------------*/
operand *geniCodePostInc (operand *op)
{
    iCode *ic ;
    operand *rOp ;
    link *optype = operandType(op);
    operand *result ;
    operand *rv = (IS_ITEMP(op) ? 
		   geniCodeRValue(op,(IS_PTR(optype) ? TRUE : FALSE)) :
		   op);		   
    link *rvtype = operandType(rv);    
    int size = 0;
    
    /* if this is not an address we have trouble */
    if ( ! op->isaddr ) {
	werror (E_LVALUE_REQUIRED,"++");
	return op ;
    }
    
    rOp = newiTempOperand(rvtype,0);
    rOp->noSpilLoc = 1;

    if (IS_ITEMP(rv))
	rv->noSpilLoc = 1;

    geniCodeAssign(rOp,rv,0);
   
    size = (IS_PTR(rvtype) ? getSize(rvtype->next) : 1);
    ic = newiCode('+',rv,operandFromLit(size));          
    IC_RESULT(ic) = result =newiTempOperand(rvtype,0);
    ADDTOCHAIN(ic);

    geniCodeAssign(op,result,0);
    
    return rOp;
    
}

/*-----------------------------------------------------------------*/
/* geniCodePreInc - generate code for preIncrement                 */
/*-----------------------------------------------------------------*/
operand *geniCodePreInc (operand *op)
{
    iCode *ic ;
    link *optype = operandType(op);    
    operand *rop = (IS_ITEMP(op) ? 
		    geniCodeRValue (op,(IS_PTR(optype) ? TRUE : FALSE)) :
		    op);
    link *roptype = operandType(rop);
    operand *result;
    int size = 0;
    
    if ( ! op->isaddr ) {
	werror(E_LVALUE_REQUIRED,"++");
	return op ;
    }


    size = (IS_PTR(roptype) ? getSize(roptype->next) : 1);
    ic = newiCode('+',rop,operandFromLit(size));
    IC_RESULT(ic) = result = newiTempOperand(roptype,0) ;
    ADDTOCHAIN(ic);

    
    return geniCodeAssign(op,result,0) ;
}

/*-----------------------------------------------------------------*/
/* geniCodePostDec - generates code for Post decrement             */
/*-----------------------------------------------------------------*/
operand *geniCodePostDec (operand *op)
{
    iCode *ic ;
    operand *rOp ;
    link *optype = operandType(op);
    operand *result ;
    operand *rv = (IS_ITEMP(op) ? 
		   geniCodeRValue(op,(IS_PTR(optype) ? TRUE : FALSE)) :
		   op);		   
    link *rvtype = operandType(rv);    
    int size = 0;
    
    /* if this is not an address we have trouble */
    if ( ! op->isaddr ) {
	werror (E_LVALUE_REQUIRED,"++");
	return op ;
    }
    
    rOp = newiTempOperand(rvtype,0);
    rOp->noSpilLoc = 1;

    if (IS_ITEMP(rv))
	rv->noSpilLoc = 1;

    geniCodeAssign(rOp,rv,0);
   
    size = (IS_PTR(rvtype) ? getSize(rvtype->next) : 1);
    ic = newiCode('-',rv,operandFromLit(size));          
    IC_RESULT(ic) = result =newiTempOperand(rvtype,0);
    ADDTOCHAIN(ic);

    geniCodeAssign(op,result,0);
    
    return rOp;
    
}

/*-----------------------------------------------------------------*/
/* geniCodePreDec - generate code for pre  decrement               */
/*-----------------------------------------------------------------*/
operand *geniCodePreDec (operand *op)
{  
    iCode *ic ;
    link *optype = operandType(op);    
    operand *rop = (IS_ITEMP(op) ? 
		    geniCodeRValue (op,(IS_PTR(optype) ? TRUE : FALSE)) :
		    op);
    link *roptype = operandType(rop);
    operand *result;
    int size = 0;
    
    if ( ! op->isaddr ) {
	werror(E_LVALUE_REQUIRED,"++");
	return op ;
    }


    size = (IS_PTR(roptype) ? getSize(roptype->next) : 1);
    ic = newiCode('-',rop,operandFromLit(size));
    IC_RESULT(ic) = result = newiTempOperand(roptype,0) ;
    ADDTOCHAIN(ic);

    
    return geniCodeAssign(op,result,0) ;
}


/*-----------------------------------------------------------------*/
/* geniCodeBitwise - gen int code for bitWise  operators           */
/*-----------------------------------------------------------------*/
operand *geniCodeBitwise (operand *left, operand *right, 
			  int oper, link *resType)
{
    iCode *ic;   
    
    left = geniCodeCast(resType,left,TRUE);
    right= geniCodeCast(resType,right,TRUE);
    
    ic = newiCode(oper,left,right);
    IC_RESULT(ic) = newiTempOperand(resType,0);
    
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeAddressOf - gens icode for '&' address of operator      */
/*-----------------------------------------------------------------*/
operand *geniCodeAddressOf (operand *op) 
{
    iCode *ic;
    link *p ;
    link *optype = operandType(op);
    link *opetype= getSpec(optype);
    
    /* lvalue check already done in decorateType */
    /* this must be a lvalue */
/*     if (!op->isaddr && !IS_AGGREGATE(optype)) { */
/* 	werror (E_LVALUE_REQUIRED,"&"); */
/* 	return op; */
/*     } */
    
    p = newLink();
    p->class = DECLARATOR ;
    
    /* set the pointer depending on the storage class */
    if ((DCL_TYPE(p) = PTR_TYPE(SPEC_OCLS(opetype))) == CPOINTER)
	DCL_PTR_CONST(p) = port->mem.code_ro;

    /* make sure we preserve the const & volatile */
    if (IS_CONSTANT(opetype)) 
	DCL_PTR_CONST(p) = 1;

    if (IS_VOLATILE(opetype))
	DCL_PTR_VOLATILE(p) = 1;
    
    p->next = copyLinkChain(optype);
    
    /* if already a temp */
    if (IS_ITEMP(op)) {
	setOperandType (op,p);     
	op->isaddr= 0;
	return op;
    }
    
    /* other wise make this of the type coming in */
    ic = newiCode(ADDRESS_OF,op,NULL);
    IC_RESULT(ic) = newiTempOperand(p,1);
    IC_RESULT(ic)->isaddr = 0;
    ADDTOCHAIN(ic);
    return IC_RESULT(ic);
}
/*-----------------------------------------------------------------*/
/* setOClass - sets the output class depending on the pointer type */
/*-----------------------------------------------------------------*/
void setOClass (link *ptr, link *spec)
{
    switch (DCL_TYPE(ptr)) {
    case POINTER:
	SPEC_OCLS(spec) = data ;
	break ;
	
    case GPOINTER:
	SPEC_OCLS(spec) = generic;
	break;
	
    case FPOINTER:
	SPEC_OCLS(spec) = xdata ;
	break ;
	
    case CPOINTER:
	SPEC_OCLS(spec) = code ;
	break ;  
	
    case IPOINTER:
	SPEC_OCLS(spec) = idata;
	break;

    case PPOINTER:
	SPEC_OCLS(spec) = xstack;
	break;

    case EEPPOINTER:
	SPEC_OCLS(spec) = eeprom;
	break;

    default:
	break;

    }
}

/*-----------------------------------------------------------------*/
/* geniCodeDerefPtr - dereference pointer with '*'                 */
/*-----------------------------------------------------------------*/
operand *geniCodeDerefPtr (operand *op)
{    
    link *rtype , *retype ;
    link *optype = operandType(op);  

    /* if this is a pointer then generate the rvalue */
    if (IS_PTR(optype)) {
	if (IS_TRUE_SYMOP(op)) {
	    op->isaddr = 1;
	    op = geniCodeRValue(op,TRUE);
	}
	else 	
	    op = geniCodeRValue(op,TRUE);	
    }
    
    /* now get rid of the pointer part */
    if (lvaluereq && IS_ITEMP(op) )
    {
	retype = getSpec(rtype = copyLinkChain(optype)) ;
    }
    else
    {
	retype = getSpec(rtype = copyLinkChain(optype->next)) ;
    }
    
    /* if this is a pointer then outputclass needs 2b updated */
    if (IS_PTR(optype)) 
	setOClass(optype,retype);    
        
    op->isGptr = IS_GENPTR(optype);

    /* if the pointer was declared as a constant */
    /* then we cannot allow assignment to the derefed */
    if (IS_PTR_CONST(optype))
	SPEC_CONST(retype) = 1;
    
    op->isaddr = (IS_PTR(rtype)    ||
		  IS_STRUCT(rtype) || 
		  IS_INT(rtype)    ||
		  IS_CHAR(rtype)   ||
		  IS_FLOAT(rtype) );

    if (!lvaluereq)
	op = geniCodeRValue(op,TRUE);

    setOperandType(op,rtype);
    
    return op;    
}

/*-----------------------------------------------------------------*/
/* geniCodeUnaryMinus - does a unary minus of the operand          */
/*-----------------------------------------------------------------*/
operand *geniCodeUnaryMinus (operand *op)
{
    iCode *ic ;
    link *optype = operandType(op);
    
    if (IS_LITERAL(optype))
	return operandFromLit(- floatFromVal(op->operand.valOperand));
    
    ic = newiCode(UNARYMINUS,op,NULL);
    IC_RESULT(ic) = newiTempOperand(optype,0);
    ADDTOCHAIN(ic);
    return IC_RESULT(ic);
}

/*-----------------------------------------------------------------*/
/* geniCodeLeftShift - gen i code for left shift                   */
/*-----------------------------------------------------------------*/
operand *geniCodeLeftShift (operand *left, operand *right)
{ 
    iCode *ic;
    link *ltype = operandType(left);
    
    ic = newiCode(LEFT_OP,left,right);
    IC_RESULT(ic) = newiTempOperand(ltype,0);
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;  
}

/*-----------------------------------------------------------------*/
/* geniCodeRightShift - gen i code for right shift                 */
/*-----------------------------------------------------------------*/
operand *geniCodeRightShift (operand *left, operand *right)
{ 
    iCode *ic;
    link *ltype = operandType(left);
    
    ic = newiCode(RIGHT_OP,left,right);
    IC_RESULT(ic) = newiTempOperand(ltype,0);
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;  
}

#ifdef __BORLANDC__
#define LONG_LONG __int64
#else
#define LONG_LONG long long
#endif

/*-----------------------------------------------------------------*/
/* geniCodeLogic- logic code                                       */
/*-----------------------------------------------------------------*/
operand *geniCodeLogic (operand *left, operand *right, int op )
{
    iCode *ic ;
    link *ctype; 
    link *rtype = operandType(right);
    link *ltype = operandType(left);
    
    /* left is integral type and right is literal then
       check if the literal value is within bounds */
    if (IS_INTEGRAL(ltype) && IS_LITERAL(rtype)) {
	int nbits = bitsForType(ltype);
	long v = operandLitValue(right);

	if (v > ((LONG_LONG) 1 << nbits) && v > 0)
	    werror(W_CONST_RANGE," compare operation ");
    }

    ctype = computeType(ltype,rtype);			      
    left = geniCodeCast(ctype,left,TRUE);
    right= geniCodeCast(ctype,right,TRUE);

    ic = newiCode(op,left,right);
    IC_RESULT(ic) = newiTempOperand (newCharLink(),1);

    /* if comparing anything greater than one byte
       and not a '==' || '!=' || '&&' || '||' (these
       will be inlined */
    if (getSize(ctype) > 1 && 
	op != EQ_OP        && 
	op != NE_OP        &&
	op != AND_OP       &&
	op != OR_OP        )
	ic->supportRtn = 1;

    ADDTOCHAIN(ic);
    return IC_RESULT(ic);
}

/*-----------------------------------------------------------------*/
/* geniCodeUnary - for a a generic unary operation                 */
/*-----------------------------------------------------------------*/
operand *geniCodeUnary (operand *op, int oper )
{
    iCode *ic = newiCode (oper,op,NULL);
    
    IC_RESULT(ic)= newiTempOperand(operandType(op),0);
    ADDTOCHAIN(ic);
    return IC_RESULT(ic) ;
}

/*-----------------------------------------------------------------*/
/* geniCodeConditional - geniCode for '?' ':' operation            */
/*-----------------------------------------------------------------*/
operand *geniCodeConditional (ast *tree)
{
    iCode *ic ;
    symbol *falseLabel = newiTempLabel(NULL);
    symbol *exitLabel  = newiTempLabel(NULL);
    operand *cond = ast2iCode(tree->left);
    operand *true, *false , *result;
    
    ic = newiCodeCondition(geniCodeRValue(cond,FALSE),
			   NULL,falseLabel);
    ADDTOCHAIN(ic);
    
    true = ast2iCode(tree->right->left);
    
    /* move the value to a new Operand */
    result = newiTempOperand(operandType(true),0);
    geniCodeAssign(result,geniCodeRValue(true,FALSE),0);
    
    /* generate an unconditional goto */
    geniCodeGoto(exitLabel);
    
    /* now for the right side */
    geniCodeLabel(falseLabel);
    
    false = ast2iCode(tree->right->right);
    geniCodeAssign(result,geniCodeRValue(false,FALSE),0);
    
    /* create the exit label */
    geniCodeLabel(exitLabel);
    
    return result ;
}

/*-----------------------------------------------------------------*/
/* geniCodeAssign - generate code for assignment                   */
/*-----------------------------------------------------------------*/
operand *geniCodeAssign (operand *left, operand *right, int nosupdate)
{
    iCode *ic ;
    link *ltype = operandType(left);
    link *rtype = operandType(right);
    
    if (!left->isaddr && !IS_ITEMP(left)) {
	werror(E_LVALUE_REQUIRED,"assignment");
	return left;
    }
	
    /* left is integral type and right is literal then
       check if the literal value is within bounds */
    if (IS_INTEGRAL(ltype) && right->type == VALUE && IS_LITERAL(rtype)) {
	int nbits = bitsForType(ltype);
	long v = operandLitValue(right);

	if (v > ((LONG_LONG)1 << nbits) && v > 0)
	    werror(W_CONST_RANGE," = operation");
    }

    /* if the left & right type don't exactly match */
    /* if pointer set then make sure the check is
       done with the type & not the pointer */
    /* then cast rights type to left */   

    /* first check the type for pointer assignement */
    if (left->isaddr && IS_PTR(ltype) && IS_ITEMP(left) &&
	checkType(ltype,rtype)<0) {
	if (checkType(ltype->next,rtype) < 0)
	    right = geniCodeCast(ltype->next,right,TRUE);
    } else
	if (checkType(ltype,rtype) < 0 )
	    right = geniCodeCast(ltype,right,TRUE);

    /* if left is a true symbol & ! volatile 
       create an assignment to temporary for
       the right & then assign this temporary
       to the symbol this is SSA . isn't it simple
       and folks have published mountains of paper on it */
    if (IS_TRUE_SYMOP(left) && 
	!isOperandVolatile(left,FALSE) &&
	isOperandGlobal(left)) {
	symbol *sym = NULL;

	if (IS_TRUE_SYMOP(right))
	    sym = OP_SYMBOL(right);
	ic = newiCode('=',NULL,right);
	IC_RESULT(ic) = right = newiTempOperand(ltype,0);	
	SPIL_LOC(right)  = sym ;
	ADDTOCHAIN(ic);
    }
    
    ic = newiCode('=',NULL,right);
    IC_RESULT(ic) = left;
    ADDTOCHAIN(ic);    

    /* if left isgptr flag is set then support
       routine will be required */
    if (left->isGptr)
	ic->supportRtn = 1;

    ic->nosupdate = nosupdate;
    return left;
}

/*-----------------------------------------------------------------*/
/* geniCodeSEParms - generate code for side effecting fcalls       */
/*-----------------------------------------------------------------*/
static void geniCodeSEParms (ast *parms)
{
    if (!parms)
	return ;

    if (parms->type == EX_OP && parms->opval.op == PARAM) {
	geniCodeSEParms (parms->left) ;
	geniCodeSEParms (parms->right);
	return ;
    }

    /* hack don't like this but too lazy to think of
       something better */
    if (IS_ADDRESS_OF_OP(parms))
	parms->left->lvalue = 1;
    
    if (IS_CAST_OP(parms) && 
	IS_PTR(parms->ftype) && 
	IS_ADDRESS_OF_OP(parms->right))
	parms->right->left->lvalue = 1;

    parms->opval.oprnd = 
	geniCodeRValue(ast2iCode (parms),FALSE);
   
    parms->type = EX_OPERAND ;
}

/*-----------------------------------------------------------------*/
/* geniCodeParms - generates parameters                            */
/*-----------------------------------------------------------------*/
static void geniCodeParms ( ast *parms , int *stack, link *fetype, symbol *func)
{
    iCode *ic ;
    operand *pval ; 
    
    if ( ! parms )
	return ;
    
    /* if this is a param node then do the left & right */
    if (parms->type == EX_OP && parms->opval.op == PARAM) {
	geniCodeParms (parms->left, stack,fetype,func) ;
	geniCodeParms (parms->right, stack,fetype,func);
	return ;
    }
    
    /* get the parameter value */
    if (parms->type == EX_OPERAND)
	pval = parms->opval.oprnd ;
    else {
	/* maybe this else should go away ?? */
	/* hack don't like this but too lazy to think of
	   something better */
	if (IS_ADDRESS_OF_OP(parms))
	    parms->left->lvalue = 1;
    
	if (IS_CAST_OP(parms) && 
	    IS_PTR(parms->ftype) && 
	    IS_ADDRESS_OF_OP(parms->right))
	    parms->right->left->lvalue = 1;

	pval = geniCodeRValue(ast2iCode (parms),FALSE);	
    }

    /* if register parm then make it a send */
    if (((parms->argSym && IS_REGPARM(parms->argSym->etype)) ||
	IS_REGPARM(parms->etype)) && !func->hasVargs ) {
	ic = newiCode(SEND,pval,NULL);
	ADDTOCHAIN(ic);
    } else {
	/* now decide whether to push or assign */
	if (!(options.stackAuto || IS_RENT(fetype))) { 
	    
	    /* assign */
	    operand *top = operandFromSymbol(parms->argSym);
	    geniCodeAssign(top,pval,1);
	}
	else { 
	    link *p = operandType(pval);
	    /* push */
	    ic = newiCode(IPUSH,pval,NULL);
	    ic->parmPush = 1;
	    /* update the stack adjustment */
	    *stack += getSize(IS_AGGREGATE(p)? aggrToPtr(p,FALSE):p);
	    ADDTOCHAIN(ic);
	}
    }
    
}

/*-----------------------------------------------------------------*/
/* geniCodeCall - generates temp code for calling                  */
/*-----------------------------------------------------------------*/
operand *geniCodeCall (operand *left, ast *parms)
{ 
    iCode *ic ;
    operand *result ;
    link *type, *etype;
    int stack = 0 ;
    
    /* take care of parameters with side-effecting
       function calls in them, this is required to take care 
       of overlaying function parameters */
    geniCodeSEParms ( parms );

    /* first the parameters */
    geniCodeParms ( parms , &stack , getSpec(operandType(left)), OP_SYMBOL(left));
    
    /* now call : if symbol then pcall */
    if (IS_ITEMP(left)) 
	ic = newiCode(PCALL,left,NULL);
    else
	ic = newiCode(CALL,left,NULL);
    
    IC_ARGS(ic) = left->operand.symOperand->args ;
    type = copyLinkChain(operandType(left)->next);
    etype = getSpec(type);
    SPEC_EXTR(etype) = 0;
    IC_RESULT(ic) = result = newiTempOperand(type,1);
    
    ADDTOCHAIN(ic);
    
    /* stack adjustment after call */
    left->parmBytes = stack;

    return result;
}

/*-----------------------------------------------------------------*/
/* geniCodeReceive - generate intermediate code for "receive"      */
/*-----------------------------------------------------------------*/
static void geniCodeReceive (value *args)
{   
    /* for all arguments that are passed in registers */
    while (args) {

	if (IS_REGPARM(args->etype)) {
	    operand *opr = operandFromValue(args);
	    operand *opl ;
	    symbol *sym  = OP_SYMBOL(opr);
	    iCode *ic ;

	    /* we will use it after all optimizations
	       and before liveRange calculation */	    
	    if (!sym->addrtaken && !IS_VOLATILE(sym->etype)) {

		if (IN_FARSPACE(SPEC_OCLS(sym->etype)) &&
		   options.stackAuto == 0 &&
		   !IS_DS390_PORT) {
		} else {
		    opl = newiTempOperand(args->type,0);
		    sym->reqv = opl ;	    
		    sym->reqv->key = sym->key ;
		    OP_SYMBOL(sym->reqv)->key = sym->key;
		    OP_SYMBOL(sym->reqv)->isreqv = 1;
		    OP_SYMBOL(sym->reqv)->islocal= 0;
		    SPIL_LOC(sym->reqv) =  sym;
		}
	    }

	    ic = newiCode(RECEIVE,NULL,NULL);
	    currFunc->recvSize = getSize(sym->etype);
	    IC_RESULT(ic) = opr;
	    ADDTOCHAIN(ic);
	}
	
	args = args->next;
    }
}

/*-----------------------------------------------------------------*/
/* geniCodeFunctionBody - create the function body                 */
/*-----------------------------------------------------------------*/
void geniCodeFunctionBody (ast *tree)
{
    iCode *ic ;
    operand *func ;
    link *fetype  ;
    int savelineno ;
    
    /* reset the auto generation */
    /* numbers */
    iTempNum = 0 ;
    iTempLblNum = 0;   
    operandKey = 0 ;
    iCodeKey = 0 ;
    func  = ast2iCode(tree->left);
    fetype = getSpec(operandType(func));
    
    savelineno = lineno;
    lineno = OP_SYMBOL(func)->lineDef;
    /* create an entry label */
    geniCodeLabel(entryLabel);    
    lineno = savelineno;

    /* create a proc icode */
    ic = newiCode(FUNCTION,func,NULL);
    /* if the function has parmas   then */
    /* save the parameters information    */
    ic->argLabel.args = tree->values.args ;
    ic->lineno = OP_SYMBOL(func)->lineDef;

    ADDTOCHAIN(ic);   
    
    /* for all parameters that are passed
       on registers add a "receive" */
    geniCodeReceive( tree->values.args );

    /* generate code for the body */
    ast2iCode(tree->right);
    
    /* create a label for return */
    geniCodeLabel(returnLabel);
    
    /* now generate the end proc */
    ic = newiCode(ENDFUNCTION,func,NULL);
    ADDTOCHAIN(ic);
    return ;
}

/*-----------------------------------------------------------------*/
/* geniCodeReturn - gen icode for 'return' statement               */
/*-----------------------------------------------------------------*/
void geniCodeReturn (operand *op)
{
    iCode *ic;
    
    /* if the operand is present force an rvalue */
    if (op) 
	op = geniCodeRValue(op,FALSE);    
    
    ic = newiCode(RETURN,op,NULL);
    ADDTOCHAIN(ic);
}

/*-----------------------------------------------------------------*/
/* geniCodeIfx - generates code for extended if statement          */
/*-----------------------------------------------------------------*/
void geniCodeIfx (ast *tree)
{
    iCode *ic;
    operand *condition = ast2iCode(tree->left);
    link *cetype; 
    
    /* if condition is null then exit */
    if (!condition)
	goto exit ;
    else
	condition = geniCodeRValue(condition,FALSE);
    
    cetype = getSpec(operandType(condition));
    /* if the condition is a literal */
    if (IS_LITERAL(cetype)) {
	if (floatFromVal(condition->operand.valOperand)) {
	    if (tree->trueLabel)
		geniCodeGoto(tree->trueLabel);
	    else
		assert(1);
	}
	else {
	    if (tree->falseLabel)
		geniCodeGoto (tree->falseLabel);
	    else
		assert(1);
	}
	goto exit;
    }
    
    if ( tree->trueLabel ) {
	ic = newiCodeCondition(condition,
			       tree->trueLabel,
			       NULL );
	ADDTOCHAIN(ic);
	
	if ( tree->falseLabel) 
	    geniCodeGoto(tree->falseLabel);     
    }
    else {
	ic = newiCodeCondition (condition,
				NULL,
				tree->falseLabel);
	ADDTOCHAIN(ic);
    }
    
 exit:
    ast2iCode(tree->right);
}

/*-----------------------------------------------------------------*/
/* geniCodeJumpTable - tries to create a jump table for switch     */
/*-----------------------------------------------------------------*/
int geniCodeJumpTable (operand *cond, value *caseVals, ast *tree)
{
    int min = 0 ,max = 0, t, cnt = 0;
    value *vch;
    iCode *ic;
    operand *boundary;
    symbol *falseLabel;
    set *labels = NULL ;

    if (!tree || !caseVals)
	return 0;

    /* the criteria for creating a jump table is */
    /* all integer numbers between the maximum & minimum must */
    /* be present , the maximum value should not exceed 255 */
    min = max = (int)floatFromVal(vch = caseVals);
    sprintf(buffer,"_case_%d_%d",
	    tree->values.switchVals.swNum,
	    min);
    addSet(&labels,newiTempLabel(buffer));

    /* if there is only one case value then no need */
    if (!(vch = vch->next ))
	return 0;

    while (vch) {
	if (((t = (int)floatFromVal(vch)) - max) != 1)
	    return 0;
	sprintf(buffer,"_case_%d_%d",
		tree->values.switchVals.swNum,
		t);
	addSet(&labels,newiTempLabel(buffer));
	max = t;
	cnt++ ;
	vch = vch->next ;
    }
    
    /* if the number of case statements <= 2 then */
    /* it is not economical to create the jump table */
    /* since two compares are needed for boundary conditions */
    if ((! optimize.noJTabBoundary  && cnt <= 2) || max > (255/3))
	return 0;
    
    if ( tree->values.switchVals.swDefault )
	sprintf (buffer,"_default_%d",tree->values.switchVals.swNum);
    else
	sprintf (buffer,"_swBrk_%d",tree->values.switchVals.swNum  );
    
    falseLabel = newiTempLabel (buffer);

    /* so we can create a jumptable */
    /* first we rule out the boundary conditions */
    /* if only optimization says so */
    if ( ! optimize.noJTabBoundary ) {
	link *cetype = getSpec(operandType(cond));
	/* no need to check the lower bound if
	   the condition is unsigned & minimum value is zero */
	if (!( min == 0  && SPEC_USIGN(cetype))) {
	    boundary = geniCodeLogic (cond,operandFromLit(min),'<');
	    ic = newiCodeCondition (boundary,falseLabel,NULL);
	    ADDTOCHAIN(ic);
	}

	/* now for upper bounds */
	boundary = geniCodeLogic(cond,operandFromLit(max),'>');
	ic = newiCodeCondition (boundary,falseLabel,NULL);
	ADDTOCHAIN(ic);
    }

    /* if the min is not zero then we no make it zero */
    if (min) {
	cond = geniCodeSubtract(cond,operandFromLit(min));
	setOperandType(cond, UCHARTYPE);
    }

    /* now create the jumptable */
    ic = newiCode(JUMPTABLE,NULL,NULL);
    IC_JTCOND(ic) = cond;
    IC_JTLABELS(ic) = labels;
    ADDTOCHAIN(ic);
    return 1;       
}

/*-----------------------------------------------------------------*/
/* geniCodeSwitch - changes a switch to a if statement             */
/*-----------------------------------------------------------------*/
void geniCodeSwitch (ast *tree)
{
    iCode *ic ;
    operand *cond = geniCodeRValue(ast2iCode (tree->left),FALSE);
    value *caseVals = tree->values.switchVals.swVals ;
    symbol *trueLabel , *falseLabel;
    
    /* if we can make this a jump table */
    if ( geniCodeJumpTable (cond,caseVals,tree) )
	goto jumpTable ; /* no need for the comparison */

    /* for the cases defined do */
    while (caseVals) {
	
	operand *compare = geniCodeLogic (cond,
					  operandFromValue(caseVals),
					  EQ_OP);
	
	sprintf(buffer,"_case_%d_%d",
		tree->values.switchVals.swNum,
		(int) floatFromVal(caseVals));
	trueLabel = newiTempLabel(buffer);
	
	ic = newiCodeCondition(compare,trueLabel,NULL);
	ADDTOCHAIN(ic);
	caseVals = caseVals->next;
    }


    
    /* if default is present then goto break else break */
    if ( tree->values.switchVals.swDefault )
	sprintf (buffer,"_default_%d",tree->values.switchVals.swNum);
    else
	sprintf (buffer,"_swBrk_%d",tree->values.switchVals.swNum  );
    
    falseLabel = newiTempLabel (buffer);
    geniCodeGoto(falseLabel);
 
 jumpTable:   
    ast2iCode(tree->right);
} 

/*-----------------------------------------------------------------*/
/* geniCodeInline - intermediate code for inline assembler         */
/*-----------------------------------------------------------------*/
static void geniCodeInline (ast *tree)
{
    iCode *ic;

    ic = newiCode(INLINEASM,NULL,NULL);
    IC_INLINE(ic) = tree->values.inlineasm;
    ADDTOCHAIN(ic);
}

/*-----------------------------------------------------------------*/
/* ast2iCode - creates an icodeList from an ast                    */
/*-----------------------------------------------------------------*/
operand *ast2iCode (ast *tree)
{
    operand *left = NULL;
    operand *right= NULL;
    
    if (!tree)
	return NULL ;
    
    /* set the global variables for filename & line number */
    if ( tree->filename )
	filename =  tree->filename ;
    if ( tree->lineno)
	lineno   = tree->lineno ;
    if (tree->block)
	block = tree->block ;
    if (tree->level)
	scopeLevel = tree->level;
    
    if (tree->type == EX_VALUE )
	return operandFromValue(tree->opval.val);
    
    if (tree->type == EX_LINK )
	return operandFromLink (tree->opval.lnk);
    
    /* if we find a nullop */
    if (tree->type == EX_OP && 
	( tree->opval.op == NULLOP || 
	  tree->opval.op == BLOCK )) {
	ast2iCode (tree->left);
	ast2iCode (tree->right);
	return NULL ;
    }
    
    /* special cases for not evaluating */
    if ( tree->opval.op != ':'   && 
	 tree->opval.op != '?'   &&
	 tree->opval.op != CALL  && 
	 tree->opval.op != IFX   &&
	 tree->opval.op != LABEL &&
	 tree->opval.op != GOTO  &&     
	 tree->opval.op != SWITCH &&
	 tree->opval.op != FUNCTION &&
	 tree->opval.op != INLINEASM ) {

	if (IS_ASSIGN_OP(tree->opval.op) || 
	    IS_DEREF_OP(tree)            || 
	    (tree->opval.op == '&' && !tree->right) ||
	    tree->opval.op == PTR_OP) {
 	    lvaluereq++;
	    if ((IS_ARRAY_OP(tree->left) && IS_ARRAY_OP(tree->left->left)) ||
		(IS_DEREF_OP(tree) && IS_ARRAY_OP(tree->left)))
	    {
		int olvr = lvaluereq ;
		lvaluereq = 0;
		left = operandFromAst(tree->left);
		lvaluereq = olvr - 1;
	    } else {
		left = operandFromAst(tree->left);
		lvaluereq--;
	    }
	    if (IS_DEREF_OP(tree) && IS_DEREF_OP(tree->left))
		    left = geniCodeRValue(left,TRUE);
	} else {
	    left =  operandFromAst(tree->left);
	}
	if (tree->opval.op == INC_OP || 
	    tree->opval.op == DEC_OP) {
	    lvaluereq++;
	    right= operandFromAst(tree->right);
	    lvaluereq--;
	} else {
	    right= operandFromAst(tree->right);
	}
    }
    
    /* now depending on the type of operand */
    /* this will be a biggy                 */
    switch (tree->opval.op) {
	
    case '[' :    /* array operation */
	{
	    link *ltype = operandType(left);
	    left= geniCodeRValue (left,IS_PTR(ltype->next) ? TRUE : FALSE);
	    right=geniCodeRValue (right,TRUE);		   
	}
	
	return geniCodeArray (left,right);
	
    case '.' :   /* structure dereference */
	if (IS_PTR(operandType(left)))
	    left = geniCodeRValue(left,TRUE);
	else
	    left = geniCodeRValue(left,FALSE);       	  
	
	return geniCodeStruct (left,right,tree->lvalue);
	
    case PTR_OP: /* structure pointer dereference */
	{
	    link *pType;
	    pType = operandType(left);
	    left = geniCodeRValue(left,TRUE);
	    
	    setOClass (pType,getSpec(operandType(left)));
	}              
	
	return geniCodeStruct (left, right,tree->lvalue);
	
    case INC_OP: /* increment operator */
	if ( left )
	    return geniCodePostInc (left);
	else
	    return geniCodePreInc (right);
	
    case DEC_OP: /* decrement operator */
	if ( left )
	    return geniCodePostDec (left);
	else
	    return geniCodePreDec (right);
	
    case '&' : /* bitwise and or address of operator */
	if ( right ) { /* this is a bitwise operator   */
	    left= geniCodeRValue(left,FALSE);
	    right= geniCodeRValue(right,FALSE);	    
	    return geniCodeBitwise (left,right,BITWISEAND,tree->ftype);
	} else
	    return geniCodeAddressOf (left);
	
    case '|': /* bitwise or & xor */
    case '^':
	return geniCodeBitwise (geniCodeRValue(left,FALSE),
				geniCodeRValue(right,FALSE),
				tree->opval.op,
				tree->ftype);
	
    case '/':
	return geniCodeDivision (geniCodeRValue(left,FALSE),
				 geniCodeRValue(right,FALSE));
	
    case '%' :
	return geniCodeModulus (geniCodeRValue(left,FALSE),
				geniCodeRValue(right,FALSE));
    case '*':
	if ( right ) 
	    return geniCodeMultiply (geniCodeRValue(left,FALSE),
				     geniCodeRValue(right,FALSE));
	else	    
	    return geniCodeDerefPtr (geniCodeRValue(left,FALSE));
	
    case '-' :
	if ( right ) 
	    return geniCodeSubtract (geniCodeRValue(left,FALSE),
				     geniCodeRValue(right,FALSE));
	else
	    return geniCodeUnaryMinus (geniCodeRValue(left,FALSE));
	
    case '+' :
	if ( right ) 
	    return geniCodeAdd (geniCodeRValue(left,FALSE),
				geniCodeRValue(right,FALSE));
	else
	    return geniCodeRValue(left,FALSE) ; /* unary '+' has no meaning */
	
    case LEFT_OP:
	return geniCodeLeftShift (geniCodeRValue(left,FALSE),
				  geniCodeRValue(right,FALSE));
	
    case RIGHT_OP:
	return geniCodeRightShift (geniCodeRValue(left,FALSE),
				   geniCodeRValue(right,FALSE));
    case CAST:
	return geniCodeCast (operandType(left),
			     geniCodeRValue(right,FALSE),FALSE);
	
    case '~' :
    case '!' :
    case RRC:
    case RLC:	
	return geniCodeUnary (geniCodeRValue(left,FALSE),tree->opval.op);
	
    case GETHBIT:
	{
	    operand *op = geniCodeUnary (geniCodeRValue(left,FALSE),tree->opval.op);
	    setOperandType(op, UCHARTYPE);
	    return op;
	}
    case '>' :
    case '<' :
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
    case AND_OP:
    case OR_OP:
	return geniCodeLogic (geniCodeRValue(left,FALSE),
			      geniCodeRValue(right,FALSE),
			      tree->opval.op);
    case '?' : 
	return geniCodeConditional (tree); 
	
    case SIZEOF:
	return operandFromLit(getSize(tree->right->ftype));
	
    case '='        :
	{
	    link *rtype = operandType(right);
	    link *ltype = operandType(left);
	    if (IS_PTR(rtype) && IS_ITEMP(right) 
		&& right->isaddr && checkType(rtype->next,ltype)==1)
		right =  geniCodeRValue(right,TRUE);
	    else
		right = geniCodeRValue(right,FALSE);

	    geniCodeAssign (left,right,0);
	    return right ;
	}	       
    case MUL_ASSIGN:
	return 
	    geniCodeAssign(left,
			   geniCodeMultiply(geniCodeRValue (operandFromOperand(left),
							    FALSE),
					    geniCodeRValue(right,FALSE)),0);
						
    case DIV_ASSIGN:
 	return 
	    geniCodeAssign(left,
			   geniCodeDivision(geniCodeRValue(operandFromOperand(left),
							   FALSE),
					    geniCodeRValue(right,FALSE)),0);
    case MOD_ASSIGN:
 	return 
	    geniCodeAssign(left,
			   geniCodeModulus(geniCodeRValue(operandFromOperand(left),
							  FALSE),
					   geniCodeRValue(right,FALSE)),0);
    case ADD_ASSIGN: 
	{
	    link *rtype = operandType(right);
	    link *ltype = operandType(left);
	    if (IS_PTR(rtype) && IS_ITEMP(right) 
		&& right->isaddr && checkType(rtype->next,ltype)==1)
		right =  geniCodeRValue(right,TRUE);
	    else
		right = geniCodeRValue(right,FALSE);

	   
	    return geniCodeAssign(left,
				  geniCodeAdd (geniCodeRValue(operandFromOperand(left),
							      FALSE),
					       right),0);
	}
    case SUB_ASSIGN:
	{
	    link *rtype = operandType(right);
	    link *ltype = operandType(left);
	    if (IS_PTR(rtype) && IS_ITEMP(right) 
		&& right->isaddr && checkType(rtype->next,ltype)==1) {
		right =  geniCodeRValue(right,TRUE);
	    }
	    else {
		right = geniCodeRValue(right,FALSE);
	    }
	    return 
		geniCodeAssign (left,
				geniCodeSubtract(geniCodeRValue(operandFromOperand(left),
								FALSE),
						 right),0);
	}
    case LEFT_ASSIGN:
 	return 
	    geniCodeAssign (left,
			    geniCodeLeftShift(geniCodeRValue(operandFromOperand(left)
							     ,FALSE),
					      geniCodeRValue(right,FALSE)),0);
    case RIGHT_ASSIGN:
   	return 
	    geniCodeAssign(left,
			   geniCodeRightShift(geniCodeRValue(operandFromOperand(left)
							     ,FALSE),
					      geniCodeRValue(right,FALSE)),0);
    case AND_ASSIGN:
    	return 
	    geniCodeAssign (left,
			    geniCodeBitwise(geniCodeRValue(operandFromOperand(left),
							   FALSE),
					    geniCodeRValue(right,FALSE),
					    BITWISEAND,
					    operandType(left)),0);
    case XOR_ASSIGN:
    	return 
	    geniCodeAssign (left,
			    geniCodeBitwise (geniCodeRValue(operandFromOperand(left),
							    FALSE),
					     geniCodeRValue(right,FALSE),
					     '^',
					     operandType(left)),0);
    case OR_ASSIGN:
    	return 
	    geniCodeAssign (left,
			    geniCodeBitwise (geniCodeRValue(operandFromOperand(left)
							    ,FALSE),
					     geniCodeRValue(right,FALSE),
					     '|',
					     operandType(left)),0);
    case ',' :
	return geniCodeRValue(right,FALSE);
	
    case CALL:
	return geniCodeCall (ast2iCode(tree->left),
			     tree->right);
    case LABEL:
	geniCodeLabel(ast2iCode(tree->left)->operand.symOperand);
	return ast2iCode (tree->right);
	
    case GOTO:
	geniCodeGoto (ast2iCode(tree->left)->operand.symOperand);
	return ast2iCode (tree->right);
	
    case FUNCTION:
	geniCodeFunctionBody ( tree );
	return NULL ;
	
    case RETURN:
	geniCodeReturn (right);
	return NULL ;
	
    case IFX:
	geniCodeIfx (tree);
	return NULL ;
	
    case SWITCH:
	geniCodeSwitch (tree);
	return NULL;

    case INLINEASM:
	geniCodeInline (tree);
	return NULL ;
    }
    
    return NULL;
}

/*-----------------------------------------------------------------*/
/* reverseICChain - gets from the list and creates a linkedlist    */
/*-----------------------------------------------------------------*/
iCode *reverseiCChain ()
{
    iCode *loop = NULL ;
    iCode *prev = NULL ;
    
    while ((loop = getSet(&iCodeChain))) {
	loop->next = prev ;
	if ( prev )
	    prev->prev = loop; 
	prev = loop ;
    }
    
    return prev;
}


/*-----------------------------------------------------------------*/
/* iCodeFromAst - given an ast will convert it to iCode            */
/*-----------------------------------------------------------------*/
iCode *iCodeFromAst ( ast *tree )
{
    returnLabel = newiTempLabel("_return");
    entryLabel  = newiTempLabel("_entry") ;
    ast2iCode (tree);
    return reverseiCChain ();
}
