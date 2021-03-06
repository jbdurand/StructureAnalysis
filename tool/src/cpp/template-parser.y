/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       AMAPmod: Exploring and Modeling Plant Architecture
 *
 *       Copyright 1995-2000 UMR Cirad/Inra Modelisation des Plantes
 *
 *       File author(s): Ch. Godin (christophe.godin@cirad.fr)
 *
 *       Forum for AMAPmod developers    : amldevlp@cirad.fr
 *
 *  ----------------------------------------------------------------------------
 *
 *                      GNU General Public Licence
 *
 *       This program is free software; you can redistribute it and/or
 *       modify it under the terms of the GNU General Public License as
 *       published by the Free Software Foundation; either version 2 of
 *       the License, or (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public
 *       License along with this program; see the file COPYING. If not,
 *       write to the Free Software Foundation, Inc., 59
 *       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  ----------------------------------------------------------------------------
 */


%{

#include <iostream>

#include <string>

#include "gparser.h"

#include <math.h>  // for FLT_MAX
#include <cfloat>

  // macros parser(p) and smbtable(p,t) enable you to define the
  // parser p an its symbol table t from within
  // the action part of the grammar rules use.
  // However, before, the type of the symbol table used must be set here.
  // here the parser is of type GenericParser<float> and then:

#define SMB_TABLE_TYPE float

  // definition of an undefined value (used locally in this file)
  // corresponding to the returned type (float) of an action.

#define UNDEF_FLOAT FLT_MAX
#define ISDEFINED(x) (((x)!=UNDEF_FLOAT)?true:false)


%}

%pure_parser                    /* asks for a reentrant parser: don't remove ! */
%token_table                    /* generates a token table (names of the tokens) */

/* possible types of tokens returned by the lexer */

%union {
  int           integer;
  int           boolean;
  double        real;
  char          character;
  char*         string;
  ostream*      os;
}


/* association of types to tokens and non terminals */

%token <string>    TokSYSTEMCALL
%token <integer>   TokInt
%token <real>      TokReal
%token <character> TokCharacter
%token <string>    TokName
%token <string>    TokString
%token <string>    TokWSString

/** Associativite,
 ** avec priorite du plus faible au plus fort.
 **
 ** Les operateurs "+" et "-" sont traite de maniere particuliere a
 ** cause de presence d'une version unaire de ces memes operateurs,
 **
 **/
%left ';'
%right '='
%left '?'
%left ':'
%left ','
%left TokMINUS TokPLUS
%left TokTIMES TokSLASH
%left UMINUS
%token TokQUIT TokINFO TokCLEAR TokPRINT
%token TokSYSTEMCALL TokError

%type <os>        PrintArgs
%type <real>      GExp
%type <real>      Atom
%type <real>      Exp
%type <real>      AssignExp


%start Session

%%

Session :
   Command {}
 | GExp  {}
 | Session GExp {}
 | Session Command {}
 | Session error
   {
     parser(p);
     postream(p) << "*** PARSER: OK: Resynchronized on input at ";
     postream(p) << p._plexer->YYText() << endl;
   }
;

GExp :
   AssignExp ';'
   {
     parser(p);
     if (ISDEFINED($1))
       postream(p) << "result: " << $1 << endl;
     else postream(p) << "undef "<< endl;
   }
 | Exp ';'
   {
     parser(p);
     if (ISDEFINED($1)) postream(p)<< "result: " << $1 << endl;
     else postream(p) << "undef "<< endl;
   }
 | error ';'
   {
      parser(p);
      postream(p) << "expression error "  << endl;
   }
;

Command :
   TokQUIT {YYACCEPT;}
 | TokINFO
   {
     parser(p);                 // gets the parser p
     smbtable(p,t);             // its symbol table t
     for (SymbolTable<float>::iterator sti = t.begin();
          sti != t.end();
          sti++) postream(p) << (*sti).first << " = " << (*sti).second << endl;
   }
 | TokCLEAR
   {
     parser(p);                 // gets the parser p
     smbtable(p,t);             // its symbol table t
     t.clear();
     postream(p) << "All parser variables cleared !" << endl;
   }
 | TokSYSTEMCALL {}
 | TokPRINT '(' PrintArgs ')'
   {
     *$3 << endl; // final \n
   }
;

PrintArgs :
   PrintArgs ',' TokWSString {*$1 << $3; $$=$1; delete [] $3;}
 | PrintArgs ',' Exp {*$1 << $3; $$=$1;}
 | TokWSString {parser(p);$$ = &postream(p); *$$ << $1; delete [] $1;}
 | Exp {parser(p);$$ = &postream(p);; *$$ << $1;}
;



/***************/
/* expressions */
/***************/

AssignExp :
   TokName '=' Exp
   {

     parser(p);                 // gets the parser p
     smbtable(p,t);             // its symbol table t

     // check first if the name already exists in the symbol table
     // SymbolTable<float>::iterator sti = t.find($1);

     /*
     if (sti != t.end()) { // symbol already in the table
       t.erase(sti);
     }
     */

     // stores the new element
     t[std::string($1)]=$3;

     delete [] $1;

     $$ = $3;
   }
;

Exp :
   Atom {$$ = $1;}
 | TokName
   {
     parser(p);
     smbtable(p,t);
     SymbolTable<float>::iterator sti = t.find($1);
     if (sti != t.end()) { // symbol already in the table
       $$ = sti->second;
     }
     else {
       $$ = UNDEF_FLOAT;
     }
     delete [] $1;
   }
 | Exp TokPLUS Exp {$$ = $1 + $3;}
 | Exp TokMINUS Exp {$$ = $1 - $3;}
 | Exp TokTIMES Exp {$$ = $1 * $3;}
 | Exp TokSLASH Exp {$$ = $1 / $3;}
 | TokMINUS Exp %prec UMINUS {$$ = - $2;}
 |'(' Exp ')' {$$ = $2;}
;

Atom :
   TokInt {$$ = (float)$1;}
 | TokReal {$$ = $1;}
;



%%







