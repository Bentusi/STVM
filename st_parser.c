/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 6 "st_parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "mmgr.h"
#include "symbol_table.h"

extern int yylex(void);
extern int yylineno;
extern int yycolumn;

void yyerror(const char *s);

/* 全局AST根节点 */
static ast_node_t *g_ast_root = NULL;

#line 89 "st_parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "st_parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INTEGER_LITERAL = 3,            /* INTEGER_LITERAL  */
  YYSYMBOL_REAL_LITERAL = 4,               /* REAL_LITERAL  */
  YYSYMBOL_BOOL_LITERAL = 5,               /* BOOL_LITERAL  */
  YYSYMBOL_STRING_LITERAL = 6,             /* STRING_LITERAL  */
  YYSYMBOL_WSTRING_LITERAL = 7,            /* WSTRING_LITERAL  */
  YYSYMBOL_TIME_LITERAL = 8,               /* TIME_LITERAL  */
  YYSYMBOL_DATE_LITERAL = 9,               /* DATE_LITERAL  */
  YYSYMBOL_TOD_LITERAL = 10,               /* TOD_LITERAL  */
  YYSYMBOL_DT_LITERAL = 11,                /* DT_LITERAL  */
  YYSYMBOL_IDENTIFIER = 12,                /* IDENTIFIER  */
  YYSYMBOL_PRAGMA = 13,                    /* PRAGMA  */
  YYSYMBOL_PROGRAM = 14,                   /* PROGRAM  */
  YYSYMBOL_END_PROGRAM = 15,               /* END_PROGRAM  */
  YYSYMBOL_FUNCTION = 16,                  /* FUNCTION  */
  YYSYMBOL_END_FUNCTION = 17,              /* END_FUNCTION  */
  YYSYMBOL_FUNCTION_BLOCK = 18,            /* FUNCTION_BLOCK  */
  YYSYMBOL_END_FUNCTION_BLOCK = 19,        /* END_FUNCTION_BLOCK  */
  YYSYMBOL_LIBRARY = 20,                   /* LIBRARY  */
  YYSYMBOL_END_LIBRARY = 21,               /* END_LIBRARY  */
  YYSYMBOL_IMPORT = 22,                    /* IMPORT  */
  YYSYMBOL_FROM = 23,                      /* FROM  */
  YYSYMBOL_AS = 24,                        /* AS  */
  YYSYMBOL_EXPORT = 25,                    /* EXPORT  */
  YYSYMBOL_VERSION = 26,                   /* VERSION  */
  YYSYMBOL_VAR = 27,                       /* VAR  */
  YYSYMBOL_END_VAR = 28,                   /* END_VAR  */
  YYSYMBOL_VAR_INPUT = 29,                 /* VAR_INPUT  */
  YYSYMBOL_VAR_OUTPUT = 30,                /* VAR_OUTPUT  */
  YYSYMBOL_VAR_IN_OUT = 31,                /* VAR_IN_OUT  */
  YYSYMBOL_VAR_EXTERNAL = 32,              /* VAR_EXTERNAL  */
  YYSYMBOL_VAR_GLOBAL = 33,                /* VAR_GLOBAL  */
  YYSYMBOL_VAR_ACCESS = 34,                /* VAR_ACCESS  */
  YYSYMBOL_VAR_TEMP = 35,                  /* VAR_TEMP  */
  YYSYMBOL_VAR_CONFIG = 36,                /* VAR_CONFIG  */
  YYSYMBOL_VAR_CONSTANT = 37,              /* VAR_CONSTANT  */
  YYSYMBOL_IF = 38,                        /* IF  */
  YYSYMBOL_THEN = 39,                      /* THEN  */
  YYSYMBOL_ELSE = 40,                      /* ELSE  */
  YYSYMBOL_ELSIF = 41,                     /* ELSIF  */
  YYSYMBOL_END_IF = 42,                    /* END_IF  */
  YYSYMBOL_CASE = 43,                      /* CASE  */
  YYSYMBOL_OF = 44,                        /* OF  */
  YYSYMBOL_END_CASE = 45,                  /* END_CASE  */
  YYSYMBOL_FOR = 46,                       /* FOR  */
  YYSYMBOL_TO = 47,                        /* TO  */
  YYSYMBOL_BY = 48,                        /* BY  */
  YYSYMBOL_DO = 49,                        /* DO  */
  YYSYMBOL_END_FOR = 50,                   /* END_FOR  */
  YYSYMBOL_WHILE = 51,                     /* WHILE  */
  YYSYMBOL_END_WHILE = 52,                 /* END_WHILE  */
  YYSYMBOL_REPEAT = 53,                    /* REPEAT  */
  YYSYMBOL_UNTIL = 54,                     /* UNTIL  */
  YYSYMBOL_END_REPEAT = 55,                /* END_REPEAT  */
  YYSYMBOL_EXIT = 56,                      /* EXIT  */
  YYSYMBOL_CONTINUE = 57,                  /* CONTINUE  */
  YYSYMBOL_RETURN = 58,                    /* RETURN  */
  YYSYMBOL_TYPE_BOOL = 59,                 /* TYPE_BOOL  */
  YYSYMBOL_TYPE_BYTE = 60,                 /* TYPE_BYTE  */
  YYSYMBOL_TYPE_WORD = 61,                 /* TYPE_WORD  */
  YYSYMBOL_TYPE_DWORD = 62,                /* TYPE_DWORD  */
  YYSYMBOL_TYPE_LWORD = 63,                /* TYPE_LWORD  */
  YYSYMBOL_TYPE_SINT = 64,                 /* TYPE_SINT  */
  YYSYMBOL_TYPE_INT = 65,                  /* TYPE_INT  */
  YYSYMBOL_TYPE_DINT = 66,                 /* TYPE_DINT  */
  YYSYMBOL_TYPE_LINT = 67,                 /* TYPE_LINT  */
  YYSYMBOL_TYPE_USINT = 68,                /* TYPE_USINT  */
  YYSYMBOL_TYPE_UINT = 69,                 /* TYPE_UINT  */
  YYSYMBOL_TYPE_UDINT = 70,                /* TYPE_UDINT  */
  YYSYMBOL_TYPE_ULINT = 71,                /* TYPE_ULINT  */
  YYSYMBOL_TYPE_REAL = 72,                 /* TYPE_REAL  */
  YYSYMBOL_TYPE_LREAL = 73,                /* TYPE_LREAL  */
  YYSYMBOL_TYPE_STRING = 74,               /* TYPE_STRING  */
  YYSYMBOL_TYPE_WSTRING = 75,              /* TYPE_WSTRING  */
  YYSYMBOL_TYPE_TIME = 76,                 /* TYPE_TIME  */
  YYSYMBOL_TYPE_DATE = 77,                 /* TYPE_DATE  */
  YYSYMBOL_TYPE_TIME_OF_DAY = 78,          /* TYPE_TIME_OF_DAY  */
  YYSYMBOL_TYPE_DATE_AND_TIME = 79,        /* TYPE_DATE_AND_TIME  */
  YYSYMBOL_TYPE_ARRAY = 80,                /* TYPE_ARRAY  */
  YYSYMBOL_TYPE_STRUCT = 81,               /* TYPE_STRUCT  */
  YYSYMBOL_END_STRUCT = 82,                /* END_STRUCT  */
  YYSYMBOL_TYPE_UNION = 83,                /* TYPE_UNION  */
  YYSYMBOL_END_UNION = 84,                 /* END_UNION  */
  YYSYMBOL_TYPE_ENUM = 85,                 /* TYPE_ENUM  */
  YYSYMBOL_END_ENUM = 86,                  /* END_ENUM  */
  YYSYMBOL_TYPE = 87,                      /* TYPE  */
  YYSYMBOL_END_TYPE = 88,                  /* END_TYPE  */
  YYSYMBOL_ASSIGN = 89,                    /* ASSIGN  */
  YYSYMBOL_EQ = 90,                        /* EQ  */
  YYSYMBOL_NE = 91,                        /* NE  */
  YYSYMBOL_LT = 92,                        /* LT  */
  YYSYMBOL_LE = 93,                        /* LE  */
  YYSYMBOL_GT = 94,                        /* GT  */
  YYSYMBOL_GE = 95,                        /* GE  */
  YYSYMBOL_PLUS = 96,                      /* PLUS  */
  YYSYMBOL_MINUS = 97,                     /* MINUS  */
  YYSYMBOL_MULTIPLY = 98,                  /* MULTIPLY  */
  YYSYMBOL_DIVIDE = 99,                    /* DIVIDE  */
  YYSYMBOL_MOD = 100,                      /* MOD  */
  YYSYMBOL_POWER = 101,                    /* POWER  */
  YYSYMBOL_AND = 102,                      /* AND  */
  YYSYMBOL_OR = 103,                       /* OR  */
  YYSYMBOL_XOR = 104,                      /* XOR  */
  YYSYMBOL_NOT = 105,                      /* NOT  */
  YYSYMBOL_LPAREN = 106,                   /* LPAREN  */
  YYSYMBOL_RPAREN = 107,                   /* RPAREN  */
  YYSYMBOL_LBRACKET = 108,                 /* LBRACKET  */
  YYSYMBOL_RBRACKET = 109,                 /* RBRACKET  */
  YYSYMBOL_LBRACE = 110,                   /* LBRACE  */
  YYSYMBOL_RBRACE = 111,                   /* RBRACE  */
  YYSYMBOL_SEMICOLON = 112,                /* SEMICOLON  */
  YYSYMBOL_COLON = 113,                    /* COLON  */
  YYSYMBOL_COMMA = 114,                    /* COMMA  */
  YYSYMBOL_DOT = 115,                      /* DOT  */
  YYSYMBOL_RANGE = 116,                    /* RANGE  */
  YYSYMBOL_ERROR_TOKEN = 117,              /* ERROR_TOKEN  */
  YYSYMBOL_UMINUS = 118,                   /* UMINUS  */
  YYSYMBOL_UPLUS = 119,                    /* UPLUS  */
  YYSYMBOL_YYACCEPT = 120,                 /* $accept  */
  YYSYMBOL_compilation_unit = 121,         /* compilation_unit  */
  YYSYMBOL_import_list = 122,              /* import_list  */
  YYSYMBOL_import_declaration = 123,       /* import_declaration  */
  YYSYMBOL_program_declaration = 124,      /* program_declaration  */
  YYSYMBOL_library_declaration = 125,      /* library_declaration  */
  YYSYMBOL_declaration_list = 126,         /* declaration_list  */
  YYSYMBOL_declaration = 127,              /* declaration  */
  YYSYMBOL_var_declaration = 128,          /* var_declaration  */
  YYSYMBOL_var_category = 129,             /* var_category  */
  YYSYMBOL_var_list = 130,                 /* var_list  */
  YYSYMBOL_var_item = 131,                 /* var_item  */
  YYSYMBOL_type_specifier = 132,           /* type_specifier  */
  YYSYMBOL_basic_type = 133,               /* basic_type  */
  YYSYMBOL_array_type = 134,               /* array_type  */
  YYSYMBOL_array_bounds = 135,             /* array_bounds  */
  YYSYMBOL_bound_specification = 136,      /* bound_specification  */
  YYSYMBOL_struct_type = 137,              /* struct_type  */
  YYSYMBOL_struct_members = 138,           /* struct_members  */
  YYSYMBOL_type_declaration = 139,         /* type_declaration  */
  YYSYMBOL_function_declaration = 140,     /* function_declaration  */
  YYSYMBOL_function_type = 141,            /* function_type  */
  YYSYMBOL_parameter_list = 142,           /* parameter_list  */
  YYSYMBOL_parameter_declaration = 143,    /* parameter_declaration  */
  YYSYMBOL_parameter_category = 144,       /* parameter_category  */
  YYSYMBOL_statement_list = 145,           /* statement_list  */
  YYSYMBOL_statement = 146,                /* statement  */
  YYSYMBOL_assignment_statement = 147,     /* assignment_statement  */
  YYSYMBOL_expression_statement = 148,     /* expression_statement  */
  YYSYMBOL_if_statement = 149,             /* if_statement  */
  YYSYMBOL_elsif_list = 150,               /* elsif_list  */
  YYSYMBOL_elsif_statement = 151,          /* elsif_statement  */
  YYSYMBOL_case_statement = 152,           /* case_statement  */
  YYSYMBOL_case_list = 153,                /* case_list  */
  YYSYMBOL_case_item = 154,                /* case_item  */
  YYSYMBOL_case_values = 155,              /* case_values  */
  YYSYMBOL_for_statement = 156,            /* for_statement  */
  YYSYMBOL_while_statement = 157,          /* while_statement  */
  YYSYMBOL_repeat_statement = 158,         /* repeat_statement  */
  YYSYMBOL_return_statement = 159,         /* return_statement  */
  YYSYMBOL_exit_statement = 160,           /* exit_statement  */
  YYSYMBOL_continue_statement = 161,       /* continue_statement  */
  YYSYMBOL_expression = 162,               /* expression  */
  YYSYMBOL_logical_expr = 163,             /* logical_expr  */
  YYSYMBOL_comparison_expr = 164,          /* comparison_expr  */
  YYSYMBOL_additive_expr = 165,            /* additive_expr  */
  YYSYMBOL_multiplicative_expr = 166,      /* multiplicative_expr  */
  YYSYMBOL_unary_expr = 167,               /* unary_expr  */
  YYSYMBOL_primary_expr = 168,             /* primary_expr  */
  YYSYMBOL_function_call = 169,            /* function_call  */
  YYSYMBOL_argument_list = 170,            /* argument_list  */
  YYSYMBOL_array_access = 171,             /* array_access  */
  YYSYMBOL_member_access = 172             /* member_access  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1225

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  120
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  153
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  292

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   374


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   130,   130,   137,   142,   150,   159,   163,   169,   172,
     175,   178,   185,   188,   191,   194,   201,   204,   211,   214,
     218,   224,   225,   226,   231,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   249,   252,   259,   264,   269,   276,
     277,   278,   279,   283,   284,   285,   286,   287,   288,   289,
     290,   295,   301,   304,   311,   319,   325,   329,   351,   358,
     361,   365,   371,   372,   376,   379,   383,   389,   392,   398,
     399,   400,   401,   406,   409,   413,   419,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   433,   439,   446,   449,
     452,   455,   461,   465,   471,   478,   481,   487,   491,   497,
     503,   507,   514,   517,   523,   529,   536,   539,   545,   551,
     558,   562,   563,   566,   569,   575,   576,   579,   582,   585,
     588,   591,   597,   598,   601,   607,   608,   611,   614,   617,
     623,   624,   627,   630,   636,   639,   642,   645,   648,   651,
     654,   657,   660,   663,   666,   669,   676,   679,   682,   685,
     691,   695,   702,   708
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INTEGER_LITERAL",
  "REAL_LITERAL", "BOOL_LITERAL", "STRING_LITERAL", "WSTRING_LITERAL",
  "TIME_LITERAL", "DATE_LITERAL", "TOD_LITERAL", "DT_LITERAL",
  "IDENTIFIER", "PRAGMA", "PROGRAM", "END_PROGRAM", "FUNCTION",
  "END_FUNCTION", "FUNCTION_BLOCK", "END_FUNCTION_BLOCK", "LIBRARY",
  "END_LIBRARY", "IMPORT", "FROM", "AS", "EXPORT", "VERSION", "VAR",
  "END_VAR", "VAR_INPUT", "VAR_OUTPUT", "VAR_IN_OUT", "VAR_EXTERNAL",
  "VAR_GLOBAL", "VAR_ACCESS", "VAR_TEMP", "VAR_CONFIG", "VAR_CONSTANT",
  "IF", "THEN", "ELSE", "ELSIF", "END_IF", "CASE", "OF", "END_CASE", "FOR",
  "TO", "BY", "DO", "END_FOR", "WHILE", "END_WHILE", "REPEAT", "UNTIL",
  "END_REPEAT", "EXIT", "CONTINUE", "RETURN", "TYPE_BOOL", "TYPE_BYTE",
  "TYPE_WORD", "TYPE_DWORD", "TYPE_LWORD", "TYPE_SINT", "TYPE_INT",
  "TYPE_DINT", "TYPE_LINT", "TYPE_USINT", "TYPE_UINT", "TYPE_UDINT",
  "TYPE_ULINT", "TYPE_REAL", "TYPE_LREAL", "TYPE_STRING", "TYPE_WSTRING",
  "TYPE_TIME", "TYPE_DATE", "TYPE_TIME_OF_DAY", "TYPE_DATE_AND_TIME",
  "TYPE_ARRAY", "TYPE_STRUCT", "END_STRUCT", "TYPE_UNION", "END_UNION",
  "TYPE_ENUM", "END_ENUM", "TYPE", "END_TYPE", "ASSIGN", "EQ", "NE", "LT",
  "LE", "GT", "GE", "PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MOD", "POWER",
  "AND", "OR", "XOR", "NOT", "LPAREN", "RPAREN", "LBRACKET", "RBRACKET",
  "LBRACE", "RBRACE", "SEMICOLON", "COLON", "COMMA", "DOT", "RANGE",
  "ERROR_TOKEN", "UMINUS", "UPLUS", "$accept", "compilation_unit",
  "import_list", "import_declaration", "program_declaration",
  "library_declaration", "declaration_list", "declaration",
  "var_declaration", "var_category", "var_list", "var_item",
  "type_specifier", "basic_type", "array_type", "array_bounds",
  "bound_specification", "struct_type", "struct_members",
  "type_declaration", "function_declaration", "function_type",
  "parameter_list", "parameter_declaration", "parameter_category",
  "statement_list", "statement", "assignment_statement",
  "expression_statement", "if_statement", "elsif_list", "elsif_statement",
  "case_statement", "case_list", "case_item", "case_values",
  "for_statement", "while_statement", "repeat_statement",
  "return_statement", "exit_statement", "continue_statement", "expression",
  "logical_expr", "comparison_expr", "additive_expr",
  "multiplicative_expr", "unary_expr", "primary_expr", "function_call",
  "argument_list", "array_access", "member_access", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-145)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-88)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       2,     5,     9,    21,    56,     2,  -145,  -145,  -145,   336,
      16,    -5,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  -145,
    -145,  -145,  -145,   -92,  -145,  -145,  -145,  -145,  -145,  -145,
    -145,  -145,  -145,  -145,  -145,  -145,  1056,  1056,    29,  1056,
     989,  -145,  -145,  1056,    74,  1056,   421,  -145,  -145,    80,
    -145,  -145,    83,   553,  -145,   -12,   -10,  -145,  -145,  -145,
    -145,  -145,    -2,    18,    20,   -53,    23,  -145,  -145,    99,
    1073,   106,   121,  -145,   294,   125,  1056,  1056,  1056,   100,
      24,    30,   -67,   -16,  -145,   -54,  -145,    96,    54,    97,
     609,  -145,    32,    47,  -145,  -145,   626,    40,     0,  -145,
    1051,  -145,  -145,  -145,  -145,  -145,  -145,  -145,  1056,  1056,
     144,  1138,  -145,    59,   -21,  -145,  -145,   -47,    66,  -145,
    -145,  -145,   989,  1056,  1056,  1056,  1056,  1056,  1056,  1056,
    1056,  1056,  1056,  1056,  1056,  1056,  1056,  1056,  1056,  1056,
     989,  1056,  1121,  -145,  -145,  1121,  -145,  -145,    46,  1121,
     480,  -145,   -89,  -145,  1095,  -145,   168,  -145,  -145,  1056,
     313,   536,    30,    30,    30,   -67,   -67,   -67,   -67,   -67,
     -67,   -16,   -16,  -145,  -145,  -145,  -145,  1045,  -145,   -34,
    -145,   128,   682,   122,  -145,  -145,  -145,  -145,  -145,  -145,
    -145,  -145,  -145,    70,    80,    67,  -145,  -145,  -145,   -62,
      68,  -145,  -145,  -145,   -42,  -145,   170,  1138,   698,  -145,
    -145,    71,  -145,  -145,   -43,   989,  1056,  -145,   109,  -145,
     989,  -145,  -145,   989,  1056,  1056,  -145,  -145,   181,  -145,
      -8,   101,  1056,  -145,  -145,    72,   134,    77,   480,  -145,
    -145,  -145,   754,   148,   989,  -145,  -145,   770,   989,  -145,
      41,    75,   -41,  -145,  -145,  -145,  -145,    81,  1121,  -145,
    1121,   826,  -145,   989,   843,  -145,  1056,   989,   189,   152,
     181,  -145,  1138,   108,  -145,   989,  -145,   155,   900,  -145,
    1121,  -145,   480,  1056,   989,  -145,  -145,   916,  -145,   973,
    -145,  -145
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     6,     3,     5,     0,
      18,     0,     1,     7,     2,     4,   134,   135,   136,   137,
     138,   139,   140,   141,    15,    62,    63,    25,    26,    27,
      28,    29,    30,    32,    33,    31,     0,     0,     0,     0,
      73,   108,   109,   106,     0,     0,     0,    19,    21,     0,
      23,    22,     0,     0,    74,     0,     0,    78,    79,    80,
      81,    82,     0,     0,     0,     0,   142,   143,   144,     0,
       0,     0,     0,     8,     0,     0,     0,     0,     0,     0,
     110,   111,   115,   122,   125,   130,   142,     0,     0,     0,
       0,   107,     0,     0,    14,    20,     0,     0,     0,    34,
      18,    13,    75,    76,    77,    83,    84,    85,     0,     0,
       0,    18,    17,     0,     0,   146,   150,     0,     0,   133,
     132,   131,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,     0,     0,   145,    12,     0,    24,    35,    64,     0,
      73,    86,     0,   153,     0,    10,     0,     9,   147,     0,
       0,     0,   112,   113,   114,   116,   117,   118,   119,   120,
     121,   123,   124,   126,   127,   128,   129,     0,    97,     0,
     100,     0,     0,     0,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,     0,     0,    39,    40,    41,     0,
      40,    70,    71,    72,     0,    65,     0,    18,     0,   152,
      16,     0,   151,   148,     0,    73,     0,    88,     0,    92,
      73,    95,    98,    73,     0,     0,   104,   105,     0,    56,
       0,     0,     0,    36,    38,     0,    69,     0,    73,    61,
      11,   149,     0,     0,    73,    90,    93,     0,    99,   101,
       0,     0,     0,    52,    55,    57,    58,     0,     0,    66,
       0,     0,    89,    73,     0,    96,     0,    73,     0,     0,
       0,    37,    18,    67,    59,    94,    91,     0,     0,    54,
       0,    53,    73,     0,    73,   102,    51,     0,    68,     0,
      60,   103
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -145,  -145,  -145,   196,   200,   202,    -7,   -35,  -145,  -145,
    -145,   -88,  -144,  -145,    63,  -145,   -60,  -145,  -145,  -145,
    -145,  -145,  -145,   -24,  -145,    26,   -27,  -145,  -145,  -145,
    -145,    -1,  -145,  -145,    36,  -145,  -145,  -145,  -145,  -145,
    -145,  -145,   -30,  -145,    44,    31,   -15,   -38,    -9,    48,
    -101,  -145,  -145
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     4,     5,     6,     7,     8,    46,    47,    48,    49,
      98,    99,   195,   196,   197,   252,   253,   198,   230,    50,
      51,    52,   204,   205,   206,    53,    54,    55,    56,    57,
     218,   219,    58,   177,   178,   179,    59,    60,    61,    62,
      63,    64,   116,    80,    81,    82,    83,    84,    85,    86,
     117,    67,    68
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,   199,   156,    70,    97,   207,    79,    87,   152,    89,
     147,    95,    97,    91,    74,    93,     1,     9,    71,    72,
     209,    10,     2,    75,     3,   159,   102,   232,   146,   132,
     133,    65,    25,    11,    26,    95,   108,    65,   119,   120,
     121,    88,    69,    27,    65,    28,    29,    30,    31,    32,
     233,    33,    34,    35,   109,   109,    12,    66,   -69,   214,
     158,   110,   110,   102,   241,   235,    90,   159,   269,   102,
     236,   159,    96,   270,   254,   201,   202,   203,   151,   223,
     224,    65,   134,   135,   136,   137,    92,    65,    66,   266,
     267,   157,    97,   150,    66,   100,   173,   174,   175,   176,
     103,    66,   104,    44,   154,   111,   229,    73,   180,   181,
     105,   183,   113,    65,   272,    95,   273,   171,   172,    95,
     126,   127,   128,   129,   130,   131,   123,   124,   125,   212,
     106,    65,   107,   114,   102,   -87,   286,   118,    66,   122,
     138,    65,   255,   139,    66,   142,   140,   180,   161,   244,
     216,   245,    65,   145,   143,   102,   153,   165,   166,   167,
     168,   169,   170,   201,   202,   203,   182,   162,   163,   164,
      66,   155,   160,    65,   211,   225,   208,   227,   228,   231,
     234,   102,   237,   240,   251,   258,   243,   263,    66,   256,
     260,   268,   279,   271,   249,   250,   280,   283,    66,    65,
     238,    13,   257,    95,   284,    14,    65,    15,   200,    66,
     281,    65,   259,   222,    65,   102,     0,   246,     0,     0,
     102,   102,     0,     0,     0,     0,     0,     0,     0,    65,
      66,     0,     0,    65,   102,    65,   277,   102,    65,    65,
       0,   242,     0,     0,     0,     0,   247,    95,   102,   248,
       0,   102,    65,   288,    65,    65,    66,     0,    65,     0,
     102,     0,   102,    66,   261,   282,    65,     0,    66,    65,
     264,    66,     0,    65,     0,    65,     0,     0,    65,     0,
      65,     0,     0,     0,     0,     0,    66,     0,     0,   275,
      66,     0,    66,   278,     0,    66,    66,    16,    17,    18,
      19,    20,    21,    22,     0,     0,    23,     0,   287,    66,
     289,    66,    66,     0,     0,    66,    16,    17,    18,    19,
      20,    21,    22,    66,     0,    23,    66,     0,     0,     0,
      66,     0,    66,     0,     0,    66,     0,    66,     0,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       0,    24,    25,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,    27,     0,    28,    29,    30,    31,    32,
       0,    33,    34,    35,    36,     0,     0,     0,     0,    37,
       0,     0,    38,     0,     0,     0,     0,    39,     0,    40,
      76,    77,    41,    42,    43,     0,     0,     0,     0,    78,
      45,   115,     0,     0,     0,     0,     0,     0,     0,    76,
      77,     0,     0,     0,     0,     0,     0,     0,    78,    45,
     213,     0,     0,    44,    16,    17,    18,    19,    20,    21,
      22,     0,     0,    23,     0,     0,    94,    25,     0,    26,
       0,     0,    45,     0,     0,     0,     0,     0,    27,     0,
      28,    29,    30,    31,    32,     0,    33,    34,    35,    36,
       0,     0,     0,     0,    37,     0,     0,    38,     0,     0,
       0,     0,    39,     0,    40,     0,     0,    41,    42,    43,
       0,     0,     0,    16,    17,    18,    19,    20,    21,    22,
       0,     0,    23,     0,     0,     0,    25,     0,    26,     0,
       0,     0,     0,     0,     0,     0,     0,    27,    44,    28,
      29,    30,    31,    32,     0,    33,    34,    35,    36,     0,
       0,     0,     0,    37,     0,     0,    38,    45,     0,     0,
       0,    39,     0,    40,     0,     0,    41,    42,    43,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       0,     0,     0,     0,     0,     0,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,     0,    44,   101,     0,
       0,     0,     0,     0,    36,     0,   215,   216,   217,    37,
       0,     0,    38,     0,     0,     0,    45,    39,     0,    40,
       0,    36,    41,    42,    43,     0,    37,     0,     0,    38,
       0,     0,     0,     0,    39,     0,    40,     0,     0,    41,
      42,    43,    16,    17,    18,    19,    20,    21,    22,     0,
       0,    23,     0,     0,     0,     0,     0,     0,     0,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       0,   144,    45,     0,     0,     0,     0,    36,     0,     0,
       0,     0,    37,     0,     0,    38,     0,     0,     0,    45,
      39,     0,    40,   141,    36,    41,    42,    43,     0,    37,
       0,     0,    38,     0,     0,     0,     0,    39,     0,    40,
       0,     0,    41,    42,    43,    16,    17,    18,    19,    20,
      21,    22,     0,     0,    23,     0,     0,     0,     0,     0,
       0,    16,    17,    18,    19,    20,    21,    22,     0,     0,
      23,     0,     0,     0,     0,    45,     0,   239,     0,     0,
      36,     0,     0,     0,     0,    37,     0,     0,    38,     0,
       0,     0,    45,    39,   226,    40,    36,     0,    41,    42,
      43,    37,     0,     0,    38,     0,     0,     0,     0,    39,
       0,    40,     0,     0,    41,    42,    43,    16,    17,    18,
      19,    20,    21,    22,     0,     0,    23,     0,     0,     0,
       0,     0,     0,    16,    17,    18,    19,    20,    21,    22,
       0,     0,    23,     0,     0,     0,     0,     0,    45,     0,
       0,     0,    36,     0,     0,     0,   262,    37,     0,     0,
      38,     0,     0,     0,    45,    39,     0,    40,    36,     0,
      41,    42,    43,    37,     0,   265,    38,     0,     0,     0,
       0,    39,     0,    40,     0,     0,    41,    42,    43,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       0,     0,     0,   274,     0,     0,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,     0,     0,     0,     0,
      45,     0,     0,     0,    36,     0,     0,     0,     0,    37,
       0,     0,    38,     0,     0,     0,    45,    39,     0,    40,
       0,    36,    41,    42,    43,   276,    37,     0,     0,    38,
       0,     0,     0,     0,    39,     0,    40,     0,     0,    41,
      42,    43,     0,    16,    17,    18,    19,    20,    21,    22,
       0,     0,    23,     0,     0,     0,     0,     0,     0,    16,
      17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
       0,     0,    45,   290,     0,     0,     0,     0,    36,     0,
       0,     0,     0,    37,     0,     0,    38,     0,     0,    45,
     285,    39,     0,    40,    36,     0,    41,    42,    43,    37,
       0,     0,    38,     0,     0,     0,     0,    39,     0,    40,
       0,     0,    41,    42,    43,     0,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,     0,     0,     0,     0,
       0,     0,    16,    17,    18,    19,    20,    21,    22,     0,
       0,    23,     0,     0,     0,     0,    45,     0,     0,     0,
       0,    36,     0,     0,     0,     0,    37,     0,     0,    38,
       0,     0,    45,   291,    39,     0,    40,    36,     0,    41,
      42,    43,    37,     0,     0,    38,     0,     0,     0,     0,
      39,     0,    40,     0,     0,    41,    42,    43,    16,    17,
      18,    19,    20,    21,    22,     0,     0,    23,     0,    16,
      17,    18,    19,    20,    21,    22,     0,    25,    23,    26,
       0,     0,     0,     0,     0,     0,     0,     0,    27,    45,
      28,    29,    30,    31,    32,   220,    33,    34,    35,    25,
     221,    26,     0,     0,   112,    45,     0,     0,     0,     0,
      27,     0,    28,    29,    30,    31,    32,     0,    33,    34,
      35,    25,     0,    26,     0,     0,   210,     0,     0,     0,
       0,     0,    27,     0,    28,    29,    30,    31,    32,     0,
      33,    34,    35,   184,     0,     0,     0,     0,    44,     0,
       0,    76,    77,     0,     0,     0,     0,     0,     0,     0,
      78,    45,    76,    77,    25,     0,    26,   148,     0,     0,
      44,    78,    45,     0,   149,    27,     0,    28,    29,    30,
      31,    32,     0,    33,    34,    35,     0,     0,     0,     0,
     185,   186,    44,     0,     0,     0,   187,   188,     0,     0,
       0,     0,     0,   189,   190,   191,     0,   192,     0,     0,
       0,   193,   194,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    44
};

static const yytype_int16 yycheck[] =
{
       9,   145,    23,    10,    12,   149,    36,    37,   109,    39,
      98,    46,    12,    43,   106,    45,    14,    12,    23,    24,
     109,    12,    20,   115,    22,   114,    53,    89,    28,    96,
      97,    40,    16,    12,    18,    70,    89,    46,    76,    77,
      78,    12,    26,    27,    53,    29,    30,    31,    32,    33,
     112,    35,    36,    37,   108,   108,     0,     9,    12,   160,
     107,   115,   115,    90,   107,   107,    40,   114,   109,    96,
     112,   114,    46,   114,    82,    29,    30,    31,   108,   113,
     114,    90,    98,    99,   100,   101,    12,    96,    40,    48,
      49,   112,    12,   100,    46,    12,   134,   135,   136,   137,
     112,    53,   112,    87,   111,     6,   194,   112,   138,   139,
     112,   141,     6,   122,   258,   150,   260,   132,   133,   154,
      90,    91,    92,    93,    94,    95,   102,   103,   104,   159,
     112,   140,   112,    12,   161,   112,   280,    12,    90,    39,
      44,   150,   230,    89,    96,   113,    49,   177,   122,    40,
      41,    42,   161,   113,   107,   182,    12,   126,   127,   128,
     129,   130,   131,    29,    30,    31,   140,   123,   124,   125,
     122,   112,   106,   182,     6,    47,   150,    55,   108,   112,
     112,   208,    12,   112,     3,   113,   216,    39,   140,    88,
     113,   116,     3,   112,   224,   225,    44,    89,   150,   208,
     207,     5,   232,   238,    49,     5,   215,     5,   145,   161,
     270,   220,   236,   177,   223,   242,    -1,   218,    -1,    -1,
     247,   248,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   238,
     182,    -1,    -1,   242,   261,   244,   266,   264,   247,   248,
      -1,   215,    -1,    -1,    -1,    -1,   220,   282,   275,   223,
      -1,   278,   261,   283,   263,   264,   208,    -1,   267,    -1,
     287,    -1,   289,   215,   238,   272,   275,    -1,   220,   278,
     244,   223,    -1,   282,    -1,   284,    -1,    -1,   287,    -1,
     289,    -1,    -1,    -1,    -1,    -1,   238,    -1,    -1,   263,
     242,    -1,   244,   267,    -1,   247,   248,     3,     4,     5,
       6,     7,     8,     9,    -1,    -1,    12,    -1,   282,   261,
     284,   263,   264,    -1,    -1,   267,     3,     4,     5,     6,
       7,     8,     9,   275,    -1,    12,   278,    -1,    -1,    -1,
     282,    -1,   284,    -1,    -1,   287,    -1,   289,    -1,     3,
       4,     5,     6,     7,     8,     9,    -1,    -1,    12,    -1,
      -1,    15,    16,    -1,    18,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    46,    -1,    -1,    -1,    -1,    51,    -1,    53,
      96,    97,    56,    57,    58,    -1,    -1,    -1,    -1,   105,
     106,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    96,
      97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,   106,
     107,    -1,    -1,    87,     3,     4,     5,     6,     7,     8,
       9,    -1,    -1,    12,    -1,    -1,    15,    16,    -1,    18,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    30,    31,    32,    33,    -1,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    43,    -1,    -1,    46,    -1,    -1,
      -1,    -1,    51,    -1,    53,    -1,    -1,    56,    57,    58,
      -1,    -1,    -1,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    12,    -1,    -1,    -1,    16,    -1,    18,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    87,    29,
      30,    31,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    46,   106,    -1,    -1,
      -1,    51,    -1,    53,    -1,    -1,    56,    57,    58,     3,
       4,     5,     6,     7,     8,     9,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,     8,     9,    -1,    -1,    12,    -1,    87,    15,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    40,    41,    42,    43,
      -1,    -1,    46,    -1,    -1,    -1,   106,    51,    -1,    53,
      -1,    38,    56,    57,    58,    -1,    43,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    51,    -1,    53,    -1,    -1,    56,
      57,    58,     3,     4,     5,     6,     7,     8,     9,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,     8,     9,    -1,    -1,    12,    -1,
      -1,    15,   106,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    43,    -1,    -1,    46,    -1,    -1,    -1,   106,
      51,    -1,    53,    54,    38,    56,    57,    58,    -1,    43,
      -1,    -1,    46,    -1,    -1,    -1,    -1,    51,    -1,    53,
      -1,    -1,    56,    57,    58,     3,     4,     5,     6,     7,
       8,     9,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,     8,     9,    -1,    -1,
      12,    -1,    -1,    -1,    -1,   106,    -1,    19,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    43,    -1,    -1,    46,    -1,
      -1,    -1,   106,    51,    52,    53,    38,    -1,    56,    57,
      58,    43,    -1,    -1,    46,    -1,    -1,    -1,    -1,    51,
      -1,    53,    -1,    -1,    56,    57,    58,     3,     4,     5,
       6,     7,     8,     9,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    42,    43,    -1,    -1,
      46,    -1,    -1,    -1,   106,    51,    -1,    53,    38,    -1,
      56,    57,    58,    43,    -1,    45,    46,    -1,    -1,    -1,
      -1,    51,    -1,    53,    -1,    -1,    56,    57,    58,     3,
       4,     5,     6,     7,     8,     9,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    17,    -1,    -1,     3,     4,     5,     6,
       7,     8,     9,    -1,    -1,    12,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    46,    -1,    -1,    -1,   106,    51,    -1,    53,
      -1,    38,    56,    57,    58,    42,    43,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    51,    -1,    53,    -1,    -1,    56,
      57,    58,    -1,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,     8,     9,    -1,    -1,    12,    -1,
      -1,    -1,   106,    17,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    46,    -1,    -1,   106,
      50,    51,    -1,    53,    38,    -1,    56,    57,    58,    43,
      -1,    -1,    46,    -1,    -1,    -1,    -1,    51,    -1,    53,
      -1,    -1,    56,    57,    58,    -1,     3,     4,     5,     6,
       7,     8,     9,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,     8,     9,    -1,
      -1,    12,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    43,    -1,    -1,    46,
      -1,    -1,   106,    50,    51,    -1,    53,    38,    -1,    56,
      57,    58,    43,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      51,    -1,    53,    -1,    -1,    56,    57,    58,     3,     4,
       5,     6,     7,     8,     9,    -1,    -1,    12,    -1,     3,
       4,     5,     6,     7,     8,     9,    -1,    16,    12,    18,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,   106,
      29,    30,    31,    32,    33,    40,    35,    36,    37,    16,
      45,    18,    -1,    -1,    21,   106,    -1,    -1,    -1,    -1,
      27,    -1,    29,    30,    31,    32,    33,    -1,    35,    36,
      37,    16,    -1,    18,    -1,    -1,    21,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    30,    31,    32,    33,    -1,
      35,    36,    37,    12,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,   106,    96,    97,    16,    -1,    18,   106,    -1,    -1,
      87,   105,   106,    -1,   113,    27,    -1,    29,    30,    31,
      32,    33,    -1,    35,    36,    37,    -1,    -1,    -1,    -1,
      59,    60,    87,    -1,    -1,    -1,    65,    66,    -1,    -1,
      -1,    -1,    -1,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    14,    20,    22,   121,   122,   123,   124,   125,    12,
      12,    12,     0,   123,   124,   125,     3,     4,     5,     6,
       7,     8,     9,    12,    15,    16,    18,    27,    29,    30,
      31,    32,    33,    35,    36,    37,    38,    43,    46,    51,
      53,    56,    57,    58,    87,   106,   126,   127,   128,   129,
     139,   140,   141,   145,   146,   147,   148,   149,   152,   156,
     157,   158,   159,   160,   161,   168,   169,   171,   172,    26,
     126,    23,    24,   112,   106,   115,    96,    97,   105,   162,
     163,   164,   165,   166,   167,   168,   169,   162,    12,   162,
     145,   162,    12,   162,    15,   127,   145,    12,   130,   131,
      12,    15,   146,   112,   112,   112,   112,   112,    89,   108,
     115,     6,    21,     6,    12,   107,   162,   170,    12,   167,
     167,   167,    39,   102,   103,   104,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,    44,    89,
      49,    54,   113,   107,    15,   113,    28,   131,   106,   113,
     126,   162,   170,    12,   126,   112,    23,   112,   107,   114,
     106,   145,   164,   164,   164,   165,   165,   165,   165,   165,
     165,   166,   166,   167,   167,   167,   167,   153,   154,   155,
     162,   162,   145,   162,    12,    59,    60,    65,    66,    72,
      73,    74,    76,    80,    81,   132,   133,   134,   137,   132,
     134,    29,    30,    31,   142,   143,   144,   132,   145,   109,
      21,     6,   162,   107,   170,    40,    41,    42,   150,   151,
      40,    45,   154,   113,   114,    47,    52,    55,   108,   131,
     138,   112,    89,   112,   112,   107,   112,    12,   126,    19,
     112,   107,   145,   162,    40,    42,   151,   145,   145,   162,
     162,     3,   135,   136,    82,   131,    88,   162,   113,   143,
     113,   145,    42,    39,   145,    45,    48,    49,   116,   109,
     114,   112,   132,   132,    17,   145,    42,   162,   145,     3,
      44,   136,   126,    89,    49,    50,   132,   145,   162,   145,
      17,    50
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   120,   121,   121,   121,   121,   122,   122,   123,   123,
     123,   123,   124,   124,   124,   124,   125,   125,   126,   126,
     126,   127,   127,   127,   128,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   130,   130,   131,   131,   131,   132,
     132,   132,   132,   133,   133,   133,   133,   133,   133,   133,
     133,   134,   135,   135,   136,   137,   138,   138,   139,   140,
     140,   140,   141,   141,   142,   142,   142,   143,   143,   144,
     144,   144,   144,   145,   145,   145,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   147,   148,   149,   149,
     149,   149,   150,   150,   151,   152,   152,   153,   153,   154,
     155,   155,   156,   156,   157,   158,   159,   159,   160,   161,
     162,   163,   163,   163,   163,   164,   164,   164,   164,   164,
     164,   164,   165,   165,   165,   166,   166,   166,   166,   166,
     167,   167,   167,   167,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   169,   169,   169,   169,
     170,   170,   171,   172
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     2,     1,     1,     2,     3,     5,
       5,     7,     5,     4,     4,     3,     6,     4,     0,     1,
       2,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     4,     6,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     6,     1,     3,     3,     3,     1,     2,     6,     7,
      10,     5,     1,     1,     0,     1,     3,     4,     6,     0,
       1,     1,     1,     0,     1,     2,     2,     2,     1,     1,
       1,     1,     1,     2,     2,     2,     3,     1,     5,     7,
       6,     8,     1,     2,     4,     5,     7,     1,     2,     3,
       1,     3,     9,    11,     5,     5,     1,     2,     1,     1,
       1,     1,     3,     3,     3,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     4,     5,     6,
       1,     3,     4,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* compilation_unit: import_list program_declaration  */
#line 130 "st_parser.y"
                                    {
        (yyval.node) = ast_create_program((yyvsp[0].node)->data.program.name, (yyvsp[-1].node), 
                               (yyvsp[0].node)->data.program.declarations, 
                               (yyvsp[0].node)->data.program.statements);
        g_ast_root = (yyval.node);
        ast_set_root((yyval.node));
    }
#line 1672 "st_parser.c"
    break;

  case 3: /* compilation_unit: program_declaration  */
#line 137 "st_parser.y"
                          {
        (yyval.node) = (yyvsp[0].node);
        g_ast_root = (yyval.node);
        ast_set_root((yyval.node));
    }
#line 1682 "st_parser.c"
    break;

  case 4: /* compilation_unit: import_list library_declaration  */
#line 142 "st_parser.y"
                                      {
        (yyval.node) = (yyvsp[0].node);
        if ((yyvsp[0].node)->data.library.declarations) {
            /* 将导入添加到库声明前 */
        }
        g_ast_root = (yyval.node);
        ast_set_root((yyval.node));
    }
#line 1695 "st_parser.c"
    break;

  case 5: /* compilation_unit: library_declaration  */
#line 150 "st_parser.y"
                          {
        (yyval.node) = (yyvsp[0].node);
        g_ast_root = (yyval.node);
        ast_set_root((yyval.node));
    }
#line 1705 "st_parser.c"
    break;

  case 6: /* import_list: import_declaration  */
#line 159 "st_parser.y"
                       {
        (yyval.node) = ast_create_list(AST_IMPORT_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 1714 "st_parser.c"
    break;

  case 7: /* import_list: import_list import_declaration  */
#line 163 "st_parser.y"
                                     {
        (yyval.node) = ast_list_append((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 1722 "st_parser.c"
    break;

  case 8: /* import_declaration: IMPORT IDENTIFIER SEMICOLON  */
#line 169 "st_parser.y"
                                {
        (yyval.node) = ast_create_import((yyvsp[-1].string), NULL, NULL, NULL, false);
    }
#line 1730 "st_parser.c"
    break;

  case 9: /* import_declaration: IMPORT IDENTIFIER AS IDENTIFIER SEMICOLON  */
#line 172 "st_parser.y"
                                                {
        (yyval.node) = ast_create_import((yyvsp[-3].string), (yyvsp[-1].string), NULL, NULL, false);
    }
#line 1738 "st_parser.c"
    break;

  case 10: /* import_declaration: IMPORT IDENTIFIER FROM STRING_LITERAL SEMICOLON  */
#line 175 "st_parser.y"
                                                      {
        (yyval.node) = ast_create_import((yyvsp[-3].string), NULL, (yyvsp[-1].string), NULL, false);
    }
#line 1746 "st_parser.c"
    break;

  case 11: /* import_declaration: IMPORT IDENTIFIER AS IDENTIFIER FROM STRING_LITERAL SEMICOLON  */
#line 178 "st_parser.y"
                                                                    {
        (yyval.node) = ast_create_import((yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), NULL, false);
    }
#line 1754 "st_parser.c"
    break;

  case 12: /* program_declaration: PROGRAM IDENTIFIER declaration_list statement_list END_PROGRAM  */
#line 185 "st_parser.y"
                                                                   {
        (yyval.node) = ast_create_program((yyvsp[-3].string), NULL, (yyvsp[-2].node), (yyvsp[-1].node));
    }
#line 1762 "st_parser.c"
    break;

  case 13: /* program_declaration: PROGRAM IDENTIFIER statement_list END_PROGRAM  */
#line 188 "st_parser.y"
                                                    {
        (yyval.node) = ast_create_program((yyvsp[-2].string), NULL, NULL, (yyvsp[-1].node));
    }
#line 1770 "st_parser.c"
    break;

  case 14: /* program_declaration: PROGRAM IDENTIFIER declaration_list END_PROGRAM  */
#line 191 "st_parser.y"
                                                      {
        (yyval.node) = ast_create_program((yyvsp[-2].string), NULL, (yyvsp[-1].node), NULL);
    }
#line 1778 "st_parser.c"
    break;

  case 15: /* program_declaration: PROGRAM IDENTIFIER END_PROGRAM  */
#line 194 "st_parser.y"
                                     {
        (yyval.node) = ast_create_program((yyvsp[-1].string), NULL, NULL, NULL);
    }
#line 1786 "st_parser.c"
    break;

  case 16: /* library_declaration: LIBRARY IDENTIFIER VERSION STRING_LITERAL declaration_list END_LIBRARY  */
#line 201 "st_parser.y"
                                                                           {
        (yyval.node) = ast_create_library((yyvsp[-4].string), (yyvsp[-2].string), (yyvsp[-1].node), NULL);
    }
#line 1794 "st_parser.c"
    break;

  case 17: /* library_declaration: LIBRARY IDENTIFIER declaration_list END_LIBRARY  */
#line 204 "st_parser.y"
                                                      {
        (yyval.node) = ast_create_library((yyvsp[-2].string), "1.0", (yyvsp[-1].node), NULL);
    }
#line 1802 "st_parser.c"
    break;

  case 18: /* declaration_list: %empty  */
#line 211 "st_parser.y"
                { 
        (yyval.node) = NULL; 
    }
#line 1810 "st_parser.c"
    break;

  case 19: /* declaration_list: declaration  */
#line 214 "st_parser.y"
                  {
        (yyval.node) = ast_create_list(AST_DECLARATION_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 1819 "st_parser.c"
    break;

  case 20: /* declaration_list: declaration_list declaration  */
#line 218 "st_parser.y"
                                   {
        (yyval.node) = ast_list_append((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 1827 "st_parser.c"
    break;

  case 21: /* declaration: var_declaration  */
#line 224 "st_parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 1833 "st_parser.c"
    break;

  case 22: /* declaration: function_declaration  */
#line 225 "st_parser.y"
                           { (yyval.node) = (yyvsp[0].node); }
#line 1839 "st_parser.c"
    break;

  case 23: /* declaration: type_declaration  */
#line 226 "st_parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1845 "st_parser.c"
    break;

  case 24: /* var_declaration: var_category var_list END_VAR  */
#line 231 "st_parser.y"
                                  {
        (yyval.node) = ast_create_var_declaration((yyvsp[-2].var_cat), (yyvsp[-1].node));
    }
#line 1853 "st_parser.c"
    break;

  case 25: /* var_category: VAR  */
#line 237 "st_parser.y"
        { (yyval.var_cat) = SYMBOL_VAR_LOCAL; }
#line 1859 "st_parser.c"
    break;

  case 26: /* var_category: VAR_INPUT  */
#line 238 "st_parser.y"
                { (yyval.var_cat) = SYMBOL_VAR_INPUT; }
#line 1865 "st_parser.c"
    break;

  case 27: /* var_category: VAR_OUTPUT  */
#line 239 "st_parser.y"
                 { (yyval.var_cat) = SYMBOL_VAR_OUTPUT; }
#line 1871 "st_parser.c"
    break;

  case 28: /* var_category: VAR_IN_OUT  */
#line 240 "st_parser.y"
                 { (yyval.var_cat) = SYMBOL_VAR_IN_OUT; }
#line 1877 "st_parser.c"
    break;

  case 29: /* var_category: VAR_EXTERNAL  */
#line 241 "st_parser.y"
                   { (yyval.var_cat) = SYMBOL_VAR_EXTERNAL; }
#line 1883 "st_parser.c"
    break;

  case 30: /* var_category: VAR_GLOBAL  */
#line 242 "st_parser.y"
                 { (yyval.var_cat) = SYMBOL_VAR_GLOBAL; }
#line 1889 "st_parser.c"
    break;

  case 31: /* var_category: VAR_CONSTANT  */
#line 243 "st_parser.y"
                   { (yyval.var_cat) = SYMBOL_VAR_CONSTANT; }
#line 1895 "st_parser.c"
    break;

  case 32: /* var_category: VAR_TEMP  */
#line 244 "st_parser.y"
               { (yyval.var_cat) = SYMBOL_VAR_TEMP; }
#line 1901 "st_parser.c"
    break;

  case 33: /* var_category: VAR_CONFIG  */
#line 245 "st_parser.y"
                 { (yyval.var_cat) = SYMBOL_VAR_CONFIG; }
#line 1907 "st_parser.c"
    break;

  case 34: /* var_list: var_item  */
#line 249 "st_parser.y"
             {
        (yyval.node) = (yyvsp[0].node);
    }
#line 1915 "st_parser.c"
    break;

  case 35: /* var_list: var_list var_item  */
#line 252 "st_parser.y"
                        {
        (yyvsp[-1].node)->next = (yyvsp[0].node);
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1924 "st_parser.c"
    break;

  case 36: /* var_item: IDENTIFIER COLON type_specifier SEMICOLON  */
#line 259 "st_parser.y"
                                              {
        (yyval.node) = ast_create_var_item((yyvsp[-3].string), (yyvsp[-1].type), NULL, NULL);
        /* 添加到符号表 */
        add_symbol((yyvsp[-3].string), SYMBOL_VAR, (yyvsp[-1].type));
    }
#line 1934 "st_parser.c"
    break;

  case 37: /* var_item: IDENTIFIER COLON type_specifier ASSIGN expression SEMICOLON  */
#line 264 "st_parser.y"
                                                                  {
        (yyval.node) = ast_create_var_item((yyvsp[-5].string), (yyvsp[-3].type), (yyvsp[-1].node), NULL);
        /* 添加到符号表 */
        add_symbol((yyvsp[-5].string), SYMBOL_VAR, (yyvsp[-3].type));
    }
#line 1944 "st_parser.c"
    break;

  case 38: /* var_item: IDENTIFIER COLON array_type SEMICOLON  */
#line 269 "st_parser.y"
                                            {
        (yyval.node) = ast_create_var_item((yyvsp[-3].string), NULL, NULL, (yyvsp[-1].node));
    }
#line 1952 "st_parser.c"
    break;

  case 39: /* type_specifier: basic_type  */
#line 276 "st_parser.y"
               { (yyval.type) = (yyvsp[0].type); }
#line 1958 "st_parser.c"
    break;

  case 40: /* type_specifier: array_type  */
#line 277 "st_parser.y"
                 { (yyval.type) = NULL; /* 处理数组类型 */ }
#line 1964 "st_parser.c"
    break;

  case 41: /* type_specifier: struct_type  */
#line 278 "st_parser.y"
                  { (yyval.type) = NULL; /* 处理结构体类型 */ }
#line 1970 "st_parser.c"
    break;

  case 42: /* type_specifier: IDENTIFIER  */
#line 279 "st_parser.y"
                 { (yyval.type) = NULL; /* 用户定义类型 */ }
#line 1976 "st_parser.c"
    break;

  case 43: /* basic_type: TYPE_BOOL  */
#line 283 "st_parser.y"
              { (yyval.type) = create_basic_type(TYPE_BOOL_ID); }
#line 1982 "st_parser.c"
    break;

  case 44: /* basic_type: TYPE_BYTE  */
#line 284 "st_parser.y"
                { (yyval.type) = create_basic_type(TYPE_BOOL_ID); /* 简化映射 */ }
#line 1988 "st_parser.c"
    break;

  case 45: /* basic_type: TYPE_INT  */
#line 285 "st_parser.y"
               { (yyval.type) = create_basic_type(TYPE_INT_ID); }
#line 1994 "st_parser.c"
    break;

  case 46: /* basic_type: TYPE_DINT  */
#line 286 "st_parser.y"
                { (yyval.type) = create_basic_type(TYPE_DINT_ID); }
#line 2000 "st_parser.c"
    break;

  case 47: /* basic_type: TYPE_REAL  */
#line 287 "st_parser.y"
                { (yyval.type) = create_basic_type(TYPE_REAL_ID); }
#line 2006 "st_parser.c"
    break;

  case 48: /* basic_type: TYPE_LREAL  */
#line 288 "st_parser.y"
                 { (yyval.type) = create_basic_type(TYPE_REAL_ID); }
#line 2012 "st_parser.c"
    break;

  case 49: /* basic_type: TYPE_STRING  */
#line 289 "st_parser.y"
                  { (yyval.type) = create_basic_type(TYPE_STRING_ID); }
#line 2018 "st_parser.c"
    break;

  case 50: /* basic_type: TYPE_TIME  */
#line 290 "st_parser.y"
                { (yyval.type) = create_basic_type(TYPE_TIME_ID); }
#line 2024 "st_parser.c"
    break;

  case 51: /* array_type: TYPE_ARRAY LBRACKET array_bounds RBRACKET OF type_specifier  */
#line 295 "st_parser.y"
                                                                {
        (yyval.node) = ast_create_array_type((yyvsp[0].type), (yyvsp[-3].node));
    }
#line 2032 "st_parser.c"
    break;

  case 52: /* array_bounds: bound_specification  */
#line 301 "st_parser.y"
                        {
        (yyval.node) = (yyvsp[0].node);
    }
#line 2040 "st_parser.c"
    break;

  case 53: /* array_bounds: array_bounds COMMA bound_specification  */
#line 304 "st_parser.y"
                                             {
        (yyvsp[-2].node)->next = (yyvsp[0].node);
        (yyval.node) = (yyvsp[-2].node);
    }
#line 2049 "st_parser.c"
    break;

  case 54: /* bound_specification: INTEGER_LITERAL RANGE INTEGER_LITERAL  */
#line 311 "st_parser.y"
                                          {
        ast_node_t *start = ast_create_literal(LITERAL_INT, &(yyvsp[-2].integer));
        ast_node_t *end = ast_create_literal(LITERAL_INT, &(yyvsp[0].integer));
        (yyval.node) = ast_create_binary_operation(OP_ASSIGN, start, end);
    }
#line 2059 "st_parser.c"
    break;

  case 55: /* struct_type: TYPE_STRUCT struct_members END_STRUCT  */
#line 319 "st_parser.y"
                                          {
        (yyval.node) = ast_create_struct_type((yyvsp[-1].node));
    }
#line 2067 "st_parser.c"
    break;

  case 56: /* struct_members: var_item  */
#line 325 "st_parser.y"
             {
        (yyval.node) = ast_create_list(AST_STRUCT_MEMBER_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2076 "st_parser.c"
    break;

  case 57: /* struct_members: struct_members var_item  */
#line 329 "st_parser.y"
                              {
        (yyval.node) = ast_list_append((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2084 "st_parser.c"
    break;

  case 58: /* type_declaration: TYPE IDENTIFIER COLON type_specifier SEMICOLON END_TYPE  */
#line 351 "st_parser.y"
                                                            {
        (yyval.node) = ast_create_type_declaration((yyvsp[-4].string), (yyvsp[-2].type));
    }
#line 2092 "st_parser.c"
    break;

  case 59: /* function_declaration: function_type IDENTIFIER COLON type_specifier declaration_list statement_list END_FUNCTION  */
#line 358 "st_parser.y"
                                                                                               {
        (yyval.node) = ast_create_function_declaration((yyvsp[-5].string), (yyvsp[-6].func_type), NULL, (yyvsp[-3].type), (yyvsp[-2].node), (yyvsp[-1].node));
    }
#line 2100 "st_parser.c"
    break;

  case 60: /* function_declaration: function_type IDENTIFIER LPAREN parameter_list RPAREN COLON type_specifier declaration_list statement_list END_FUNCTION  */
#line 362 "st_parser.y"
                                                   {
        (yyval.node) = ast_create_function_declaration((yyvsp[-8].string), (yyvsp[-9].func_type), (yyvsp[-6].node), (yyvsp[-3].type), (yyvsp[-2].node), (yyvsp[-1].node));
    }
#line 2108 "st_parser.c"
    break;

  case 61: /* function_declaration: function_type IDENTIFIER declaration_list statement_list END_FUNCTION_BLOCK  */
#line 365 "st_parser.y"
                                                                                  {
        (yyval.node) = ast_create_function_declaration((yyvsp[-3].string), (yyvsp[-4].func_type), NULL, NULL, (yyvsp[-2].node), (yyvsp[-1].node));
    }
#line 2116 "st_parser.c"
    break;

  case 62: /* function_type: FUNCTION  */
#line 371 "st_parser.y"
             { (yyval.func_type) = FUNC_FUNCTION; }
#line 2122 "st_parser.c"
    break;

  case 63: /* function_type: FUNCTION_BLOCK  */
#line 372 "st_parser.y"
                     { (yyval.func_type) = FUNC_FUNCTION_BLOCK; }
#line 2128 "st_parser.c"
    break;

  case 64: /* parameter_list: %empty  */
#line 376 "st_parser.y"
                { 
        (yyval.node) = NULL; 
    }
#line 2136 "st_parser.c"
    break;

  case 65: /* parameter_list: parameter_declaration  */
#line 379 "st_parser.y"
                            {
        (yyval.node) = ast_create_list(AST_PARAMETER_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2145 "st_parser.c"
    break;

  case 66: /* parameter_list: parameter_list SEMICOLON parameter_declaration  */
#line 383 "st_parser.y"
                                                     {
        (yyval.node) = ast_list_append((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2153 "st_parser.c"
    break;

  case 67: /* parameter_declaration: parameter_category IDENTIFIER COLON type_specifier  */
#line 389 "st_parser.y"
                                                       {
        (yyval.node) = ast_create_parameter((yyvsp[-2].string), (yyvsp[0].type), (yyvsp[-3].param_cat), NULL);
    }
#line 2161 "st_parser.c"
    break;

  case 68: /* parameter_declaration: parameter_category IDENTIFIER COLON type_specifier ASSIGN expression  */
#line 392 "st_parser.y"
                                                                           {
        (yyval.node) = ast_create_parameter((yyvsp[-4].string), (yyvsp[-2].type), (yyvsp[-5].param_cat), (yyvsp[0].node));
    }
#line 2169 "st_parser.c"
    break;

  case 69: /* parameter_category: %empty  */
#line 398 "st_parser.y"
                { (yyval.param_cat) = PARAM_INPUT; }
#line 2175 "st_parser.c"
    break;

  case 70: /* parameter_category: VAR_INPUT  */
#line 399 "st_parser.y"
                { (yyval.param_cat) = PARAM_INPUT; }
#line 2181 "st_parser.c"
    break;

  case 71: /* parameter_category: VAR_OUTPUT  */
#line 400 "st_parser.y"
                 { (yyval.param_cat) = PARAM_OUTPUT; }
#line 2187 "st_parser.c"
    break;

  case 72: /* parameter_category: VAR_IN_OUT  */
#line 401 "st_parser.y"
                 { (yyval.param_cat) = PARAM_IN_OUT; }
#line 2193 "st_parser.c"
    break;

  case 73: /* statement_list: %empty  */
#line 406 "st_parser.y"
                { 
        (yyval.node) = NULL; 
    }
#line 2201 "st_parser.c"
    break;

  case 74: /* statement_list: statement  */
#line 409 "st_parser.y"
                {
        (yyval.node) = ast_create_list(AST_STATEMENT_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2210 "st_parser.c"
    break;

  case 75: /* statement_list: statement_list statement  */
#line 413 "st_parser.y"
                               {
        (yyval.node) = ast_list_append((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2218 "st_parser.c"
    break;

  case 76: /* statement: assignment_statement SEMICOLON  */
#line 419 "st_parser.y"
                                   { (yyval.node) = (yyvsp[-1].node); }
#line 2224 "st_parser.c"
    break;

  case 77: /* statement: expression_statement SEMICOLON  */
#line 420 "st_parser.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 2230 "st_parser.c"
    break;

  case 78: /* statement: if_statement  */
#line 421 "st_parser.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 2236 "st_parser.c"
    break;

  case 79: /* statement: case_statement  */
#line 422 "st_parser.y"
                     { (yyval.node) = (yyvsp[0].node); }
#line 2242 "st_parser.c"
    break;

  case 80: /* statement: for_statement  */
#line 423 "st_parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 2248 "st_parser.c"
    break;

  case 81: /* statement: while_statement  */
#line 424 "st_parser.y"
                      { (yyval.node) = (yyvsp[0].node); }
#line 2254 "st_parser.c"
    break;

  case 82: /* statement: repeat_statement  */
#line 425 "st_parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 2260 "st_parser.c"
    break;

  case 83: /* statement: return_statement SEMICOLON  */
#line 426 "st_parser.y"
                                 { (yyval.node) = (yyvsp[-1].node); }
#line 2266 "st_parser.c"
    break;

  case 84: /* statement: exit_statement SEMICOLON  */
#line 427 "st_parser.y"
                               { (yyval.node) = (yyvsp[-1].node); }
#line 2272 "st_parser.c"
    break;

  case 85: /* statement: continue_statement SEMICOLON  */
#line 428 "st_parser.y"
                                   { (yyval.node) = (yyvsp[-1].node); }
#line 2278 "st_parser.c"
    break;

  case 86: /* assignment_statement: primary_expr ASSIGN expression  */
#line 433 "st_parser.y"
                                   {
        (yyval.node) = ast_create_assignment((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2286 "st_parser.c"
    break;

  case 87: /* expression_statement: function_call  */
#line 439 "st_parser.y"
                  {
        (yyval.node) = ast_create_expression_statement((yyvsp[0].node));
    }
#line 2294 "st_parser.c"
    break;

  case 88: /* if_statement: IF expression THEN statement_list END_IF  */
#line 446 "st_parser.y"
                                             {
        (yyval.node) = ast_create_if_statement((yyvsp[-3].node), (yyvsp[-1].node), NULL, NULL);
    }
#line 2302 "st_parser.c"
    break;

  case 89: /* if_statement: IF expression THEN statement_list ELSE statement_list END_IF  */
#line 449 "st_parser.y"
                                                                   {
        (yyval.node) = ast_create_if_statement((yyvsp[-5].node), (yyvsp[-3].node), NULL, (yyvsp[-1].node));
    }
#line 2310 "st_parser.c"
    break;

  case 90: /* if_statement: IF expression THEN statement_list elsif_list END_IF  */
#line 452 "st_parser.y"
                                                          {
        (yyval.node) = ast_create_if_statement((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), NULL);
    }
#line 2318 "st_parser.c"
    break;

  case 91: /* if_statement: IF expression THEN statement_list elsif_list ELSE statement_list END_IF  */
#line 455 "st_parser.y"
                                                                              {
        (yyval.node) = ast_create_if_statement((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 2326 "st_parser.c"
    break;

  case 92: /* elsif_list: elsif_statement  */
#line 461 "st_parser.y"
                    {
        (yyval.node) = ast_create_list(AST_ELSIF_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2335 "st_parser.c"
    break;

  case 93: /* elsif_list: elsif_list elsif_statement  */
#line 465 "st_parser.y"
                                 {
        (yyval.node) = ast_list_append((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2343 "st_parser.c"
    break;

  case 94: /* elsif_statement: ELSIF expression THEN statement_list  */
#line 471 "st_parser.y"
                                         {
        (yyval.node) = ast_create_elsif_statement((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2351 "st_parser.c"
    break;

  case 95: /* case_statement: CASE expression OF case_list END_CASE  */
#line 478 "st_parser.y"
                                          {
        (yyval.node) = ast_create_case_statement((yyvsp[-3].node), (yyvsp[-1].node), NULL);
    }
#line 2359 "st_parser.c"
    break;

  case 96: /* case_statement: CASE expression OF case_list ELSE statement_list END_CASE  */
#line 481 "st_parser.y"
                                                                {
        (yyval.node) = ast_create_case_statement((yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 2367 "st_parser.c"
    break;

  case 97: /* case_list: case_item  */
#line 487 "st_parser.y"
              {
        (yyval.node) = ast_create_list(AST_CASE_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2376 "st_parser.c"
    break;

  case 98: /* case_list: case_list case_item  */
#line 491 "st_parser.y"
                          {
        (yyval.node) = ast_list_append((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2384 "st_parser.c"
    break;

  case 99: /* case_item: case_values COLON statement_list  */
#line 497 "st_parser.y"
                                     {
        (yyval.node) = ast_create_case_item((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2392 "st_parser.c"
    break;

  case 100: /* case_values: expression  */
#line 503 "st_parser.y"
               {
        (yyval.node) = ast_create_list(AST_EXPRESSION_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2401 "st_parser.c"
    break;

  case 101: /* case_values: case_values COMMA expression  */
#line 507 "st_parser.y"
                                   {
        (yyval.node) = ast_list_append((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2409 "st_parser.c"
    break;

  case 102: /* for_statement: FOR IDENTIFIER ASSIGN expression TO expression DO statement_list END_FOR  */
#line 514 "st_parser.y"
                                                                             {
        (yyval.node) = ast_create_for_statement((yyvsp[-7].string), (yyvsp[-5].node), (yyvsp[-3].node), NULL, (yyvsp[-1].node));
    }
#line 2417 "st_parser.c"
    break;

  case 103: /* for_statement: FOR IDENTIFIER ASSIGN expression TO expression BY expression DO statement_list END_FOR  */
#line 517 "st_parser.y"
                                                                                             {
        (yyval.node) = ast_create_for_statement((yyvsp[-9].string), (yyvsp[-7].node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 2425 "st_parser.c"
    break;

  case 104: /* while_statement: WHILE expression DO statement_list END_WHILE  */
#line 523 "st_parser.y"
                                                 {
        (yyval.node) = ast_create_while_statement((yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 2433 "st_parser.c"
    break;

  case 105: /* repeat_statement: REPEAT statement_list UNTIL expression END_REPEAT  */
#line 529 "st_parser.y"
                                                      {
        (yyval.node) = ast_create_repeat_statement((yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 2441 "st_parser.c"
    break;

  case 106: /* return_statement: RETURN  */
#line 536 "st_parser.y"
           {
        (yyval.node) = ast_create_return_statement(NULL);
    }
#line 2449 "st_parser.c"
    break;

  case 107: /* return_statement: RETURN expression  */
#line 539 "st_parser.y"
                        {
        (yyval.node) = ast_create_return_statement((yyvsp[0].node));
    }
#line 2457 "st_parser.c"
    break;

  case 108: /* exit_statement: EXIT  */
#line 545 "st_parser.y"
         {
        (yyval.node) = ast_create_exit_statement();
    }
#line 2465 "st_parser.c"
    break;

  case 109: /* continue_statement: CONTINUE  */
#line 551 "st_parser.y"
             {
        (yyval.node) = ast_create_continue_statement();
    }
#line 2473 "st_parser.c"
    break;

  case 110: /* expression: logical_expr  */
#line 558 "st_parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 2479 "st_parser.c"
    break;

  case 111: /* logical_expr: comparison_expr  */
#line 562 "st_parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 2485 "st_parser.c"
    break;

  case 112: /* logical_expr: logical_expr AND comparison_expr  */
#line 563 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_AND, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2493 "st_parser.c"
    break;

  case 113: /* logical_expr: logical_expr OR comparison_expr  */
#line 566 "st_parser.y"
                                      {
        (yyval.node) = ast_create_binary_operation(OP_OR, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2501 "st_parser.c"
    break;

  case 114: /* logical_expr: logical_expr XOR comparison_expr  */
#line 569 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_XOR, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2509 "st_parser.c"
    break;

  case 115: /* comparison_expr: additive_expr  */
#line 575 "st_parser.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 2515 "st_parser.c"
    break;

  case 116: /* comparison_expr: comparison_expr EQ additive_expr  */
#line 576 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_EQ, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2523 "st_parser.c"
    break;

  case 117: /* comparison_expr: comparison_expr NE additive_expr  */
#line 579 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_NE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2531 "st_parser.c"
    break;

  case 118: /* comparison_expr: comparison_expr LT additive_expr  */
#line 582 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_LT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2539 "st_parser.c"
    break;

  case 119: /* comparison_expr: comparison_expr LE additive_expr  */
#line 585 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_LE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2547 "st_parser.c"
    break;

  case 120: /* comparison_expr: comparison_expr GT additive_expr  */
#line 588 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_GT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2555 "st_parser.c"
    break;

  case 121: /* comparison_expr: comparison_expr GE additive_expr  */
#line 591 "st_parser.y"
                                       {
        (yyval.node) = ast_create_binary_operation(OP_GE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2563 "st_parser.c"
    break;

  case 122: /* additive_expr: multiplicative_expr  */
#line 597 "st_parser.y"
                        { (yyval.node) = (yyvsp[0].node); }
#line 2569 "st_parser.c"
    break;

  case 123: /* additive_expr: additive_expr PLUS multiplicative_expr  */
#line 598 "st_parser.y"
                                             {
        (yyval.node) = ast_create_binary_operation(OP_ADD, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2577 "st_parser.c"
    break;

  case 124: /* additive_expr: additive_expr MINUS multiplicative_expr  */
#line 601 "st_parser.y"
                                              {
        (yyval.node) = ast_create_binary_operation(OP_SUB, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2585 "st_parser.c"
    break;

  case 125: /* multiplicative_expr: unary_expr  */
#line 607 "st_parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 2591 "st_parser.c"
    break;

  case 126: /* multiplicative_expr: multiplicative_expr MULTIPLY unary_expr  */
#line 608 "st_parser.y"
                                              {
        (yyval.node) = ast_create_binary_operation(OP_MUL, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2599 "st_parser.c"
    break;

  case 127: /* multiplicative_expr: multiplicative_expr DIVIDE unary_expr  */
#line 611 "st_parser.y"
                                            {
        (yyval.node) = ast_create_binary_operation(OP_DIV, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2607 "st_parser.c"
    break;

  case 128: /* multiplicative_expr: multiplicative_expr MOD unary_expr  */
#line 614 "st_parser.y"
                                         {
        (yyval.node) = ast_create_binary_operation(OP_MOD, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2615 "st_parser.c"
    break;

  case 129: /* multiplicative_expr: multiplicative_expr POWER unary_expr  */
#line 617 "st_parser.y"
                                           {
        (yyval.node) = ast_create_binary_operation(OP_POWER, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2623 "st_parser.c"
    break;

  case 130: /* unary_expr: primary_expr  */
#line 623 "st_parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 2629 "st_parser.c"
    break;

  case 131: /* unary_expr: NOT unary_expr  */
#line 624 "st_parser.y"
                     {
        (yyval.node) = ast_create_unary_operation(OP_NOT, (yyvsp[0].node));
    }
#line 2637 "st_parser.c"
    break;

  case 132: /* unary_expr: MINUS unary_expr  */
#line 627 "st_parser.y"
                                    {
        (yyval.node) = ast_create_unary_operation(OP_NEG, (yyvsp[0].node));
    }
#line 2645 "st_parser.c"
    break;

  case 133: /* unary_expr: PLUS unary_expr  */
#line 630 "st_parser.y"
                                  {
        (yyval.node) = ast_create_unary_operation(OP_POS, (yyvsp[0].node));
    }
#line 2653 "st_parser.c"
    break;

  case 134: /* primary_expr: INTEGER_LITERAL  */
#line 636 "st_parser.y"
                    {
        (yyval.node) = ast_create_literal(LITERAL_INT, &(yyvsp[0].integer));
    }
#line 2661 "st_parser.c"
    break;

  case 135: /* primary_expr: REAL_LITERAL  */
#line 639 "st_parser.y"
                   {
        (yyval.node) = ast_create_literal(LITERAL_REAL, &(yyvsp[0].real));
    }
#line 2669 "st_parser.c"
    break;

  case 136: /* primary_expr: BOOL_LITERAL  */
#line 642 "st_parser.y"
                   {
        (yyval.node) = ast_create_literal(LITERAL_BOOL, &(yyvsp[0].boolean));
    }
#line 2677 "st_parser.c"
    break;

  case 137: /* primary_expr: STRING_LITERAL  */
#line 645 "st_parser.y"
                     {
        (yyval.node) = ast_create_literal(LITERAL_STRING, (yyvsp[0].string));
    }
#line 2685 "st_parser.c"
    break;

  case 138: /* primary_expr: WSTRING_LITERAL  */
#line 648 "st_parser.y"
                      {
        (yyval.node) = ast_create_literal(LITERAL_WSTRING, (yyvsp[0].string));
    }
#line 2693 "st_parser.c"
    break;

  case 139: /* primary_expr: TIME_LITERAL  */
#line 651 "st_parser.y"
                   {
        (yyval.node) = ast_create_literal(LITERAL_TIME, (yyvsp[0].string));
    }
#line 2701 "st_parser.c"
    break;

  case 140: /* primary_expr: DATE_LITERAL  */
#line 654 "st_parser.y"
                   {
        (yyval.node) = ast_create_literal(LITERAL_DATE, (yyvsp[0].string));
    }
#line 2709 "st_parser.c"
    break;

  case 141: /* primary_expr: IDENTIFIER  */
#line 657 "st_parser.y"
                 {
        (yyval.node) = ast_create_identifier((yyvsp[0].string));
    }
#line 2717 "st_parser.c"
    break;

  case 142: /* primary_expr: function_call  */
#line 660 "st_parser.y"
                    {
        (yyval.node) = (yyvsp[0].node);
    }
#line 2725 "st_parser.c"
    break;

  case 143: /* primary_expr: array_access  */
#line 663 "st_parser.y"
                   {
        (yyval.node) = (yyvsp[0].node);
    }
#line 2733 "st_parser.c"
    break;

  case 144: /* primary_expr: member_access  */
#line 666 "st_parser.y"
                    {
        (yyval.node) = (yyvsp[0].node);
    }
#line 2741 "st_parser.c"
    break;

  case 145: /* primary_expr: LPAREN expression RPAREN  */
#line 669 "st_parser.y"
                               {
        (yyval.node) = (yyvsp[-1].node);
    }
#line 2749 "st_parser.c"
    break;

  case 146: /* function_call: IDENTIFIER LPAREN RPAREN  */
#line 676 "st_parser.y"
                             {
        (yyval.node) = ast_create_function_call((yyvsp[-2].string), NULL, NULL);
    }
#line 2757 "st_parser.c"
    break;

  case 147: /* function_call: IDENTIFIER LPAREN argument_list RPAREN  */
#line 679 "st_parser.y"
                                             {
        (yyval.node) = ast_create_function_call((yyvsp[-3].string), NULL, (yyvsp[-1].node));
    }
#line 2765 "st_parser.c"
    break;

  case 148: /* function_call: IDENTIFIER DOT IDENTIFIER LPAREN RPAREN  */
#line 682 "st_parser.y"
                                              {
        (yyval.node) = ast_create_function_call((yyvsp[-2].string), (yyvsp[-4].string), NULL);
    }
#line 2773 "st_parser.c"
    break;

  case 149: /* function_call: IDENTIFIER DOT IDENTIFIER LPAREN argument_list RPAREN  */
#line 685 "st_parser.y"
                                                            {
        (yyval.node) = ast_create_function_call((yyvsp[-3].string), (yyvsp[-5].string), (yyvsp[-1].node));
    }
#line 2781 "st_parser.c"
    break;

  case 150: /* argument_list: expression  */
#line 691 "st_parser.y"
               {
        (yyval.node) = ast_create_list(AST_EXPRESSION_LIST);
        (yyval.node) = ast_list_append((yyval.node), (yyvsp[0].node));
    }
#line 2790 "st_parser.c"
    break;

  case 151: /* argument_list: argument_list COMMA expression  */
#line 695 "st_parser.y"
                                     {
        (yyval.node) = ast_list_append((yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2798 "st_parser.c"
    break;

  case 152: /* array_access: primary_expr LBRACKET argument_list RBRACKET  */
#line 702 "st_parser.y"
                                                 {
        (yyval.node) = ast_create_array_access((yyvsp[-3].node), (yyvsp[-1].node));
    }
#line 2806 "st_parser.c"
    break;

  case 153: /* member_access: primary_expr DOT IDENTIFIER  */
#line 708 "st_parser.y"
                                {
        (yyval.node) = ast_create_member_access((yyvsp[-2].node), (yyvsp[0].string));
    }
#line 2814 "st_parser.c"
    break;


#line 2818 "st_parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 713 "st_parser.y"


/* ========== 错误处理函数 ========== */
void yyerror(const char *s) {
    fprintf(stderr, "语法错误 (行 %d, 列 %d): %s\n", yylineno, yycolumn, s);
}

/* ========== 全局AST访问函数 ========== */
ast_node_t* get_ast_root(void) {
    return g_ast_root;
}
