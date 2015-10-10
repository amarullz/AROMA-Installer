/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "./parser.y"

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "yydefs.h"
#include "parser.h"

extern int gLine;
extern int gColumn;

void yyerror(Expr ** root, int * error_count, const char * s);
int yyparse(Expr ** root, int * error_count);



/* Line 189 of yacc.c  */
#line 106 "parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
/* Put the tokens into the symbol table, so that GDB and other debuggers
   know about them.  */
enum yytokentype {
  AND = 258,
  OR = 259,
  SUBSTR = 260,
  SUPERSTR = 261,
  EQ = 262,
  NE = 263,
  IF = 264,
  THEN = 265,
  ELSE = 266,
  ENDIF = 267,
  STRING = 268,
  BAD = 269
};
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE {

  /* Line 214 of yacc.c  */
#line 36 "./parser.y"
  
  char * str;
  Expr * expr;
  struct {
    int argc;
    Expr ** argv;
  } args;
  
  
  
  /* Line 214 of yacc.c  */
#line 167 "parser.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 192 "parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
/* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void * malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc {
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
  ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
   + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
  __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
  do					\
  {					\
    YYSIZE_T yyi;				\
    for (yyi = 0; yyi < (Count); yyi++)	\
      (To)[yyi] = (From)[yyi];		\
  }					\
  while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
  do									\
  {									\
    YYSIZE_T yynewbytes;						\
    YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
    Stack = &yyptr->Stack_alloc;					\
    yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
    yyptr += yynewbytes / sizeof (*yyptr);				\
  }									\
  while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   129

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  21
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  4
/* YYNRULES -- Number of rules.  */
#define YYNRULES  19
/* YYNRULES -- Number of states.  */
#define YYNSTATES  39

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] = {
  0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,    18,     2,     2,     2,     2,     2,     2,
  19,    20,     2,    17,    16,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,    15,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
  2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
  5,     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] = {
  0,     0,     3,     5,     7,    11,    14,    18,    22,    26,
  30,    34,    38,    42,    45,    51,    59,    64,    65,    67
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] = {
  22,     0,    -1,    23,    -1,    13,    -1,    19,    23,    20,
  -1,    23,    15,    -1,    23,    15,    23,    -1,     1,    15,
  23,    -1,    23,    17,    23,    -1,    23,     7,    23,    -1,
  23,     8,    23,    -1,    23,     3,    23,    -1,    23,     4,
  23,    -1,    18,    23,    -1,     9,    23,    10,    23,    12,
  -1,     9,    23,    10,    23,    11,    23,    12,    -1,    13,
  19,    24,    20,    -1,    -1,    23,    -1,    24,    16,    23,
  -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] = {
  0,    65,    65,    68,    77,    78,    79,    80,    81,    82,
  83,    84,    85,    86,    87,    88,    89,   106,   110,   115
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char * const yytname[] = {
  "$end", "error", "$undefined", "AND", "OR", "SUBSTR", "SUPERSTR", "EQ",
  "NE", "IF", "THEN", "ELSE", "ENDIF", "STRING", "BAD", "';'", "','",
  "'+'", "'!'", "'('", "')'", "$accept", "input", "expr", "arglist", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] = {
  0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
  265,   266,   267,   268,   269,    59,    44,    43,    33,    40,
  41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] = {
  0,    21,    22,    23,    23,    23,    23,    23,    23,    23,
  23,    23,    23,    23,    23,    23,    23,    24,    24,    24
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] = {
  0,     2,     1,     1,     3,     2,     3,     3,     3,     3,
  3,     3,     3,     2,     5,     7,     4,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] = {
  0,     0,     0,     3,     0,     0,     0,     2,     0,     0,
  0,    13,     0,     1,     0,     0,     0,     0,     0,     0,
  7,     0,    18,     0,     4,    11,    12,     9,    10,     6,
  8,     0,     0,    16,     0,    14,    19,     0,    15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] = {
  -1,     6,     7,    23
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -16
static const yytype_int8 yypact[] = {
  46,   -11,    46,   -12,    46,    46,    11,    95,    46,    74,
  9,   -16,    53,   -16,    46,    46,    46,    46,    33,    46,
  110,    46,    95,   -15,   -16,   112,   108,    -8,    -8,   110,
  -16,    68,    46,   -16,    46,   -16,    95,    89,   -16
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] = {
  -16,   -16,    -2,   -16
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -18
static const yytype_int8 yytable[] = {
  9,    32,    11,    12,     8,    33,    20,    10,    22,    19,
  1,    13,    25,    26,    27,    28,    29,    30,     2,    31,
  0,     0,     3,     0,     0,   -17,     0,     4,     5,   -17,
  36,     0,    37,    -5,     1,     0,    -5,    -5,     0,     0,
  -5,    -5,     2,    -5,    -5,    -5,     3,     1,    -5,    -5,
  -5,     4,     5,    -5,     0,     2,    14,    15,     0,     3,
  16,    17,     0,     0,     4,     5,     0,     0,    18,     0,
  19,    14,    15,    24,     0,    16,    17,    14,    15,    34,
  35,    16,    17,    18,    21,    19,     0,     0,     0,    18,
  0,    19,    14,    15,     0,     0,    16,    17,    14,    15,
  0,    38,    16,    17,    18,     0,    19,     0,     0,     0,
  18,    14,    19,    14,    15,    16,    17,    16,    17,    16,
  17,     0,     0,     0,     0,    19,     0,    19,     0,    19
};

static const yytype_int8 yycheck[] = {
  2,    16,     4,     5,    15,    20,     8,    19,    10,    17,
  1,     0,    14,    15,    16,    17,    18,    19,     9,    21,
  -1,    -1,    13,    -1,    -1,    16,    -1,    18,    19,    20,
  32,    -1,    34,     0,     1,    -1,     3,     4,    -1,    -1,
  7,     8,     9,    10,    11,    12,    13,     1,    15,    16,
  17,    18,    19,    20,    -1,     9,     3,     4,    -1,    13,
  7,     8,    -1,    -1,    18,    19,    -1,    -1,    15,    -1,
  17,     3,     4,    20,    -1,     7,     8,     3,     4,    11,
  12,     7,     8,    15,    10,    17,    -1,    -1,    -1,    15,
  -1,    17,     3,     4,    -1,    -1,     7,     8,     3,     4,
  -1,    12,     7,     8,    15,    -1,    17,    -1,    -1,    -1,
  15,     3,    17,     3,     4,     7,     8,     7,     8,     7,
  8,    -1,    -1,    -1,    -1,    17,    -1,    17,    -1,    17
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] = {
  0,     1,     9,    13,    18,    19,    22,    23,    15,    23,
  19,    23,    23,     0,     3,     4,     7,     8,    15,    17,
  23,    10,    23,    24,    20,    23,    23,    23,    23,    23,
  23,    23,    16,    20,    11,    12,    23,    23,    12
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
/* This is here to suppress warnings from the GCC cpp's
   -Wunused-macros.  Normally we don't worry about that warning, but
   some users do, and we want to make it easy for users to remove
   YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
  do								\
    if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
    else								\
    {								\
      yyerror (root, error_count, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
  while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
  do									\
    if (YYID (N))                                                    \
    {								\
      (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
      (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
      (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
      (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
    }								\
    else								\
    {								\
      (Current).first_line   = (Current).last_line   =		\
                               YYRHSLOC (Rhs, 0).last_line;				\
      (Current).first_column = (Current).last_column =		\
                               YYRHSLOC (Rhs, 0).last_column;				\
    }								\
  while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
  fprintf (File, "%d.%d-%d.%d",			\
           (Loc).first_line, (Loc).first_column,	\
           (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
  do {						\
    if (yydebug)					\
      YYFPRINTF Args;				\
  } while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
  do {									  \
    if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
                       Type, Value, Location, root, error_count); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
  } while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE * yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, Expr ** root, int * error_count)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, root, error_count)
FILE * yyoutput;
int yytype;
YYSTYPE const * const yyvaluep;
YYLTYPE const * const yylocationp;
Expr ** root;
int * error_count;
#endif
{
  if (!yyvaluep) {
    return;
  }
  
  YYUSE (yylocationp);
  YYUSE (root);
  YYUSE (error_count);
# ifdef YYPRINT
  
  if (yytype < YYNTOKENS) {
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
  }
  
# else
  YYUSE (yyoutput);
# endif
  
  switch (yytype) {
    default:
      break;
  }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE * yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, Expr ** root, int * error_count)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, root, error_count)
FILE * yyoutput;
int yytype;
YYSTYPE const * const yyvaluep;
YYLTYPE const * const yylocationp;
Expr ** root;
int * error_count;
#endif
{
  if (yytype < YYNTOKENS) {
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  }
  else {
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);
  }
  
  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, root, error_count);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 * yybottom, yytype_int16 * yytop)
#else
static void
yy_stack_print (yybottom, yytop)
yytype_int16 * yybottom;
yytype_int16 * yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  
  for (; yybottom <= yytop; yybottom++) {
    int yybot = *yybottom;
    YYFPRINTF (stderr, " %d", yybot);
  }
  
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
  do {								\
    if (yydebug)							\
      yy_stack_print ((Bottom), (Top));				\
  } while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE * yyvsp, YYLTYPE * yylsp, int yyrule, Expr ** root, int * error_count)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, root, error_count)
YYSTYPE * yyvsp;
YYLTYPE * yylsp;
int yyrule;
Expr ** root;
int * error_count;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
             
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++) {
    YYFPRINTF (stderr, "   $%d = ", yyi + 1);
    yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
                     &(yyvsp[(yyi + 1) - (yynrhs)])
                     , &(yylsp[(yyi + 1) - (yynrhs)])		       , root, error_count);
    YYFPRINTF (stderr, "\n");
  }
}

# define YY_REDUCE_PRINT(Rule)		\
  do {					\
    if (yydebug)				\
      yy_reduce_print (yyvsp, yylsp, Rule, root, error_count); \
  } while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char * yystr)
#else
static YYSIZE_T
yystrlen (yystr)
const char * yystr;
#endif
{
  YYSIZE_T yylen;

  for (yylen = 0; yystr[yylen]; yylen++) {
    continue;
  }

  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char * yydest, const char * yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
char * yydest;
const char * yysrc;
#endif
{
  char * yyd = yydest;
  const char * yys = yysrc;

  while ((*yyd++ = *yys++) != '\0') {
    continue;
  }

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char * yyres, const char * yystr) {
  if (*yystr == '"') {
    YYSIZE_T yyn = 0;
    char const * yyp = yystr;
    
    for (;;)
      switch (*++yyp) {
        case '\'':
        case ',':
          goto do_not_strip_quotes;
          
        case '\\':
          if (*++yyp != '\\') {
            goto do_not_strip_quotes;
          }
          
          /* Fall through.  */
        default:
          if (yyres) {
            yyres[yyn] = *yyp;
          }
          
          yyn++;
          break;
          
        case '"':
          if (yyres) {
            yyres[yyn] = '\0';
          }
          
          return yyn;
      }
      
  do_not_strip_quotes:
    ;
  }
  
  if (! yyres) {
    return yystrlen (yystr);
  }
  
  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char * yyresult, int yystate, int yychar) {
  int yyn = yypact[yystate];
  
  if (! (YYPACT_NINF < yyn && yyn <= YYLAST)) {
    return 0;
  }
  else {
    int yytype = YYTRANSLATE (yychar);
    YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
    YYSIZE_T yysize = yysize0;
    YYSIZE_T yysize1;
    int yysize_overflow = 0;
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    char const * yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
    int yyx;
# if 0
    /* This is so xgettext sees the translatable formats that are
    constructed on the fly.  */
    YY_("syntax error, unexpected %s");
    YY_("syntax error, unexpected %s, expecting %s");
    YY_("syntax error, unexpected %s, expecting %s or %s");
    YY_("syntax error, unexpected %s, expecting %s or %s or %s");
    YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
    char * yyfmt;
    char const * yyf;
    static char const yyunexpected[] = "syntax error, unexpected %s";
    static char const yyexpecting[] = ", expecting %s";
    static char const yyor[] = " or %s";
    char yyformat[sizeof yyunexpected
                  + sizeof yyexpecting - 1
                  + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
                     * (sizeof yyor - 1))];
    char const * yyprefix = yyexpecting;
    /* Start YYX at -YYN if negative to avoid negative indexes in
    YYCHECK.  */
    int yyxbegin = yyn < 0 ? -yyn : 0;
    /* Stay within bounds of both yycheck and yytname.  */
    int yychecklim = YYLAST - yyn + 1;
    int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
    int yycount = 1;
    yyarg[0] = yytname[yytype];
    yyfmt = yystpcpy (yyformat, yyunexpected);
    
    for (yyx = yyxbegin; yyx < yyxend; ++yyx)
      if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR) {
        if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
          yycount = 1;
          yysize = yysize0;
          yyformat[sizeof yyunexpected - 1] = '\0';
          break;
        }
        
        yyarg[yycount++] = yytname[yyx];
        yysize1 = yysize + yytnamerr (0, yytname[yyx]);
        yysize_overflow |= (yysize1 < yysize);
        yysize = yysize1;
        yyfmt = yystpcpy (yyfmt, yyprefix);
        yyprefix = yyor;
      }
      
    yyf = YY_(yyformat);
    yysize1 = yysize + yystrlen (yyf);
    yysize_overflow |= (yysize1 < yysize);
    yysize = yysize1;
    
    if (yysize_overflow) {
      return YYSIZE_MAXIMUM;
    }
    
    if (yyresult) {
      /* Avoid sprintf, as that infringes on the user's name space.
         Don't have undefined behavior even if the translation
         produced a string with the wrong number of "%s"s.  */
      char * yyp = yyresult;
      int yyi = 0;
      
      while ((*yyp = *yyf) != '\0') {
        if (*yyp == '%' && yyf[1] == 's' && yyi < yycount) {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyf += 2;
        }
        else {
          yyp++;
          yyf++;
        }
      }
    }
    
    return yysize;
  }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char * yymsg, int yytype, YYSTYPE * yyvaluep, YYLTYPE * yylocationp, Expr ** root, int * error_count)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, root, error_count)
const char * yymsg;
int yytype;
YYSTYPE * yyvaluep;
YYLTYPE * yylocationp;
Expr ** root;
int * error_count;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (root);
  YYUSE (error_count);
  
  if (!yymsg) {
    yymsg = "Deleting";
  }
  
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);
  
  switch (yytype) {
    default:
      break;
  }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void * YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (Expr ** root, int * error_count);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void * YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
void * YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (Expr ** root, int * error_count)
#else
int
yyparse (root, error_count)
Expr ** root;
int * error_count;
#endif
#endif
{
  int yystate;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* The stacks and their tools:
     `yyss': related to states.
     `yyvs': related to semantic values.
     `yyls': related to locations.
  
     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */
  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 * yyss;
  yytype_int16 * yyssp;
  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE * yyvs;
  YYSTYPE * yyvsp;
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE * yyls;
  YYLTYPE * yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];
  YYSIZE_T yystacksize;
  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char * yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif
#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))
  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;
  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;
  YYDPRINTF ((stderr, "Starting parse\n"));
  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif
  goto yysetstate;
  /*------------------------------------------------------------.
  | yynewstate -- Push a new state, which is found in yystate.  |
  `------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;
yysetstate:
  *yyssp = yystate;
  
  if (yyss + yystacksize - 1 <= yyssp) {
    /* Get the current used size of the three stacks, in elements.  */
    YYSIZE_T yysize = yyssp - yyss + 1;
#ifdef yyoverflow
    {
      /* Give user a chance to reallocate the stack.  Use copies of
         these so that the &'s don't force the real ones into
         memory.  */
      YYSTYPE * yyvs1 = yyvs;
      yytype_int16 * yyss1 = yyss;
      YYLTYPE * yyls1 = yyls;
      /* Each stack pointer address is followed by the size of the
         data in use in that stack, in bytes.  This used to be a
         conditional around just the two extra args, but that might
         be undefined if yyoverflow is a macro.  */
      yyoverflow (YY_("memory exhausted"),
                  &yyss1, yysize * sizeof (*yyssp),
                  &yyvs1, yysize * sizeof (*yyvsp),
                  &yyls1, yysize * sizeof (*yylsp),
                  &yystacksize);
      yyls = yyls1;
      yyss = yyss1;
      yyvs = yyvs1;
    }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
    goto yyexhaustedlab;
# else
    
    /* Extend the stack our own way.  */
    if (YYMAXDEPTH <= yystacksize) {
      goto yyexhaustedlab;
    }
    
    yystacksize *= 2;
    
    if (YYMAXDEPTH < yystacksize) {
      yystacksize = YYMAXDEPTH;
    }
    
    {
      yytype_int16 * yyss1 = yyss;
      union yyalloc * yyptr =
            (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
    
      if (! yyptr) {
        goto yyexhaustedlab;
      }
    
      YYSTACK_RELOCATE (yyss_alloc, yyss);
      YYSTACK_RELOCATE (yyvs_alloc, yyvs);
      YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
    
      if (yyss1 != yyssa) {
        YYSTACK_FREE (yyss1);
      }
    }
    
# endif
#endif /* no yyoverflow */
    yyssp = yyss + yysize - 1;
    yyvsp = yyvs + yysize - 1;
    yylsp = yyls + yysize - 1;
    YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                (unsigned long int) yystacksize));
                
    if (yyss + yystacksize - 1 <= yyssp) {
      YYABORT;
    }
  }
  
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  
  if (yystate == YYFINAL) {
    YYACCEPT;
  }
  
  goto yybackup;
  /*-----------.
  | yybackup.  |
  `-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */
  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  
  if (yyn == YYPACT_NINF) {
    goto yydefault;
  }
  
  /* Not known => get a lookahead token if don't already have one.  */
  
  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY) {
    YYDPRINTF ((stderr, "Reading a token: "));
    yychar = YYLEX;
  }
  
  if (yychar <= YYEOF) {
    yychar = yytoken = YYEOF;
    YYDPRINTF ((stderr, "Now at end of input.\n"));
  }
  else {
    yytoken = YYTRANSLATE (yychar);
    YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
  }
  
  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken) {
    goto yydefault;
  }
  
  yyn = yytable[yyn];
  
  if (yyn <= 0) {
    if (yyn == 0 || yyn == YYTABLE_NINF) {
      goto yyerrlab;
    }
    
    yyn = -yyn;
    goto yyreduce;
  }
  
  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus) {
    yyerrstatus--;
  }
  
  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;
  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  
  if (yyn == 0) {
    goto yyerrlab;
  }
  
  goto yyreduce;
  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];
  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.
  
     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1 - yylen];
  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  
  switch (yyn) {
    case 2:
      /* Line 1464 of yacc.c  */
#line 65 "./parser.y"
      {
        *root = (yyvsp[(1) - (1)].expr); ;
      }
      break;
      
    case 3:
      /* Line 1464 of yacc.c  */
#line 68 "./parser.y"
      {
        (yyval.expr) = malloc(sizeof(Expr));
        (yyval.expr)->fn = Literal;
        (yyval.expr)->name = (yyvsp[(1) - (1)].str);
        (yyval.expr)->argc = 0;
        (yyval.expr)->argv = NULL;
        (yyval.expr)->start = (yyloc).start;
        (yyval.expr)->end = (yyloc).end;
        ;
      }
      break;
      
    case 4:
      /* Line 1464 of yacc.c  */
#line 77 "./parser.y"
      {
        (yyval.expr) = (yyvsp[(2) - (3)].expr);
        (yyval.expr)->start = (yyloc).start;
        (yyval.expr)->end = (yyloc).end; ;
      }
      break;
      
    case 5:
      /* Line 1464 of yacc.c  */
#line 78 "./parser.y"
      {
        (yyval.expr) = (yyvsp[(1) - (2)].expr);
        (yyval.expr)->start = (yylsp[(1) - (2)]).start;
        (yyval.expr)->end = (yylsp[(1) - (2)]).end; ;
      }
      break;
      
    case 6:
      /* Line 1464 of yacc.c  */
#line 79 "./parser.y"
      {
        (yyval.expr) = Build(SequenceFn, (yyloc), 2, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;
      }
      break;
      
    case 7:
      /* Line 1464 of yacc.c  */
#line 80 "./parser.y"
      {
        (yyval.expr) = (yyvsp[(3) - (3)].expr);
        (yyval.expr)->start = (yyloc).start;
        (yyval.expr)->end = (yyloc).end; ;
      }
      break;
      
    case 8:
      /* Line 1464 of yacc.c  */
#line 81 "./parser.y"
      {
        (yyval.expr) = Build(ConcatFn, (yyloc), 2, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;
      }
      break;
      
    case 9:
      /* Line 1464 of yacc.c  */
#line 82 "./parser.y"
      {
        (yyval.expr) = Build(EqualityFn, (yyloc), 2, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;
      }
      break;
      
    case 10:
      /* Line 1464 of yacc.c  */
#line 83 "./parser.y"
      {
        (yyval.expr) = Build(InequalityFn, (yyloc), 2, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;
      }
      break;
      
    case 11:
      /* Line 1464 of yacc.c  */
#line 84 "./parser.y"
      {
        (yyval.expr) = Build(LogicalAndFn, (yyloc), 2, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;
      }
      break;
      
    case 12:
      /* Line 1464 of yacc.c  */
#line 85 "./parser.y"
      {
        (yyval.expr) = Build(LogicalOrFn, (yyloc), 2, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;
      }
      break;
      
    case 13:
      /* Line 1464 of yacc.c  */
#line 86 "./parser.y"
      {
        (yyval.expr) = Build(LogicalNotFn, (yyloc), 1, (yyvsp[(2) - (2)].expr)); ;
      }
      break;
      
    case 14:
      /* Line 1464 of yacc.c  */
#line 87 "./parser.y"
      {
        (yyval.expr) = Build(IfElseFn, (yyloc), 2, (yyvsp[(2) - (5)].expr), (yyvsp[(4) - (5)].expr)); ;
      }
      break;
      
    case 15:
      /* Line 1464 of yacc.c  */
#line 88 "./parser.y"
      {
        (yyval.expr) = Build(IfElseFn, (yyloc), 3, (yyvsp[(2) - (7)].expr), (yyvsp[(4) - (7)].expr), (yyvsp[(6) - (7)].expr)); ;
      }
      break;
      
    case 16:
      /* Line 1464 of yacc.c  */
#line 89 "./parser.y"
      {
        (yyval.expr) = malloc(sizeof(Expr));
        (yyval.expr)->fn = FindFunction((yyvsp[(1) - (4)].str));
        
        if ((yyval.expr)->fn == NULL) {
          char buffer[256];
          snprintf(buffer, sizeof(buffer), "unknown function \"%s\"", (yyvsp[(1) - (4)].str));
          yyerror(root, error_count, buffer);
          YYERROR;
        }
        
        (yyval.expr)->name = (yyvsp[(1) - (4)].str);
        (yyval.expr)->argc = (yyvsp[(3) - (4)].args).argc;
        (yyval.expr)->argv = (yyvsp[(3) - (4)].args).argv;
        (yyval.expr)->start = (yyloc).start;
        (yyval.expr)->end = (yyloc).end;
        ;
      }
      break;
      
    case 17:
      /* Line 1464 of yacc.c  */
#line 106 "./parser.y"
      {
        (yyval.args).argc = 0;
        (yyval.args).argv = NULL;
        ;
      }
      break;
      
    case 18:
      /* Line 1464 of yacc.c  */
#line 110 "./parser.y"
      {
        (yyval.args).argc = 1;
        (yyval.args).argv = malloc(sizeof(Expr *));
        (yyval.args).argv[0] = (yyvsp[(1) - (1)].expr);
        ;
      }
      break;
      
    case 19:
      /* Line 1464 of yacc.c  */
#line 115 "./parser.y"
      {
        (yyval.args).argc = (yyvsp[(1) - (3)].args).argc + 1;
        (yyval.args).argv = realloc((yyval.args).argv, (yyval.args).argc * sizeof(Expr *));
        (yyval.args).argv[(yyval.args).argc - 1] = (yyvsp[(3) - (3)].expr);
        ;
      }
      break;
      /* Line 1464 of yacc.c  */
#line 1635 "parser.c"
      
    default:
      break;
  }
  
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  *++yyvsp = yyval;
  *++yylsp = yyloc;
  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  yyn = yyr1[yyn];
  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp) {
    yystate = yytable[yystate];
  }
  else {
    yystate = yydefgoto[yyn - YYNTOKENS];
  }
  
  goto yynewstate;
  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
yyerrlab:

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus) {
    ++yynerrs;
#if ! YYERROR_VERBOSE
    yyerror (root, error_count, YY_("syntax error"));
#else
    {
      YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
    
      if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM) {
        YYSIZE_T yyalloc = 2 * yysize;
    
        if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM)) {
          yyalloc = YYSTACK_ALLOC_MAXIMUM;
        }
    
        if (yymsg != yymsgbuf) {
          YYSTACK_FREE (yymsg);
        }
    
        yymsg = (char *) YYSTACK_ALLOC (yyalloc);
    
        if (yymsg) {
          yymsg_alloc = yyalloc;
        }
        else {
          yymsg = yymsgbuf;
          yymsg_alloc = sizeof yymsgbuf;
        }
      }
    
      if (0 < yysize && yysize <= yymsg_alloc) {
        (void) yysyntax_error (yymsg, yystate, yychar);
        yyerror (root, error_count, yymsg);
      }
      else {
        yyerror (root, error_count, YY_("syntax error"));
    
        if (yysize != 0) {
          goto yyexhaustedlab;
        }
      }
    }
#endif
  }
  
  yyerror_range[1] = yylloc;
  
  if (yyerrstatus == 3) {
    /* If just tried and failed to reuse lookahead token after an
    error, discard it.  */
    if (yychar <= YYEOF) {
      /* Return failure if at end of input.  */
      if (yychar == YYEOF) {
        YYABORT;
      }
    }
    else {
      yydestruct ("Error: discarding",
                  yytoken, &yylval, &yylloc, root, error_count);
      yychar = YYEMPTY;
    }
  }
  
  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;
  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0) {
    goto yyerrorlab;
  }
  
  yyerror_range[1] = yylsp[1 - yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;
  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */
  
  for (;;) {
    yyn = yypact[yystate];
    
    if (yyn != YYPACT_NINF) {
      yyn += YYTERROR;
      
      if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
        yyn = yytable[yyn];
        
        if (0 < yyn) {
          break;
        }
      }
    }
    
    /* Pop the current state because it cannot handle the error token.  */
    if (yyssp == yyss) {
      YYABORT;
    }
    
    yyerror_range[1] = *yylsp;
    yydestruct ("Error: popping",
                yystos[yystate], yyvsp, yylsp, root, error_count);
    YYPOPSTACK (1);
    yystate = *yyssp;
    YY_STACK_PRINT (yyss, yyssp);
  }
  
  *++yyvsp = yylval;
  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;
  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);
  yystate = yyn;
  goto yynewstate;
  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;
  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;
#if !defined(yyoverflow) || YYERROR_VERBOSE
  /*-------------------------------------------------.
  | yyexhaustedlab -- memory exhaustion comes here.  |
  `-------------------------------------------------*/
yyexhaustedlab:
  yyerror (root, error_count, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif
yyreturn:

  if (yychar != YYEMPTY)
    yydestruct ("Cleanup: discarding lookahead",
                yytoken, &yylval, &yylloc, root, error_count);
                
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  
  while (yyssp != yyss) {
    yydestruct ("Cleanup: popping",
                yystos[*yyssp], yyvsp, yylsp, root, error_count);
    YYPOPSTACK (1);
  }
  
#ifndef yyoverflow
  
  if (yyss != yyssa) {
    YYSTACK_FREE (yyss);
  }
  
#endif
#if YYERROR_VERBOSE
  
  if (yymsg != yymsgbuf) {
    YYSTACK_FREE (yymsg);
  }
  
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1684 of yacc.c  */
#line 122 "./parser.y"

int errline = -1;
int errcol  = -1;
int yyErrLine() {
  return errline;
}
int yyErrCol() {
  return errcol;
}
void yyerror(Expr ** root, int * error_count, const char * s) {
  if (strlen(s) == 0) {
    s = "syntax error";
  }
  
  if (errline == -1) {
    errline = gLine;
    errcol = gColumn;
  }
  
  printf("line %d col %d: %s\n", gLine, gColumn, s);
  ++*error_count;
}

