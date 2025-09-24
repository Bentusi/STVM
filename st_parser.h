/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_ST_PARSER_H_INCLUDED
# define YY_YY_ST_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INTEGER_LITERAL = 258,         /* INTEGER_LITERAL  */
    REAL_LITERAL = 259,            /* REAL_LITERAL  */
    BOOL_LITERAL = 260,            /* BOOL_LITERAL  */
    STRING_LITERAL = 261,          /* STRING_LITERAL  */
    WSTRING_LITERAL = 262,         /* WSTRING_LITERAL  */
    TIME_LITERAL = 263,            /* TIME_LITERAL  */
    DATE_LITERAL = 264,            /* DATE_LITERAL  */
    TOD_LITERAL = 265,             /* TOD_LITERAL  */
    DT_LITERAL = 266,              /* DT_LITERAL  */
    IDENTIFIER = 267,              /* IDENTIFIER  */
    PRAGMA = 268,                  /* PRAGMA  */
    PROGRAM = 269,                 /* PROGRAM  */
    END_PROGRAM = 270,             /* END_PROGRAM  */
    FUNCTION = 271,                /* FUNCTION  */
    END_FUNCTION = 272,            /* END_FUNCTION  */
    FUNCTION_BLOCK = 273,          /* FUNCTION_BLOCK  */
    END_FUNCTION_BLOCK = 274,      /* END_FUNCTION_BLOCK  */
    LIBRARY = 275,                 /* LIBRARY  */
    END_LIBRARY = 276,             /* END_LIBRARY  */
    IMPORT = 277,                  /* IMPORT  */
    FROM = 278,                    /* FROM  */
    AS = 279,                      /* AS  */
    EXPORT = 280,                  /* EXPORT  */
    VERSION = 281,                 /* VERSION  */
    VAR = 282,                     /* VAR  */
    END_VAR = 283,                 /* END_VAR  */
    VAR_INPUT = 284,               /* VAR_INPUT  */
    VAR_OUTPUT = 285,              /* VAR_OUTPUT  */
    VAR_IN_OUT = 286,              /* VAR_IN_OUT  */
    VAR_EXTERNAL = 287,            /* VAR_EXTERNAL  */
    VAR_GLOBAL = 288,              /* VAR_GLOBAL  */
    VAR_ACCESS = 289,              /* VAR_ACCESS  */
    VAR_TEMP = 290,                /* VAR_TEMP  */
    VAR_CONFIG = 291,              /* VAR_CONFIG  */
    VAR_CONSTANT = 292,            /* VAR_CONSTANT  */
    IF = 293,                      /* IF  */
    THEN = 294,                    /* THEN  */
    ELSE = 295,                    /* ELSE  */
    ELSIF = 296,                   /* ELSIF  */
    END_IF = 297,                  /* END_IF  */
    CASE = 298,                    /* CASE  */
    OF = 299,                      /* OF  */
    END_CASE = 300,                /* END_CASE  */
    FOR = 301,                     /* FOR  */
    TO = 302,                      /* TO  */
    BY = 303,                      /* BY  */
    DO = 304,                      /* DO  */
    END_FOR = 305,                 /* END_FOR  */
    WHILE = 306,                   /* WHILE  */
    END_WHILE = 307,               /* END_WHILE  */
    REPEAT = 308,                  /* REPEAT  */
    UNTIL = 309,                   /* UNTIL  */
    END_REPEAT = 310,              /* END_REPEAT  */
    EXIT = 311,                    /* EXIT  */
    CONTINUE = 312,                /* CONTINUE  */
    RETURN = 313,                  /* RETURN  */
    TYPE_BOOL = 314,               /* TYPE_BOOL  */
    TYPE_BYTE = 315,               /* TYPE_BYTE  */
    TYPE_WORD = 316,               /* TYPE_WORD  */
    TYPE_DWORD = 317,              /* TYPE_DWORD  */
    TYPE_LWORD = 318,              /* TYPE_LWORD  */
    TYPE_SINT = 319,               /* TYPE_SINT  */
    TYPE_INT = 320,                /* TYPE_INT  */
    TYPE_DINT = 321,               /* TYPE_DINT  */
    TYPE_LINT = 322,               /* TYPE_LINT  */
    TYPE_USINT = 323,              /* TYPE_USINT  */
    TYPE_UINT = 324,               /* TYPE_UINT  */
    TYPE_UDINT = 325,              /* TYPE_UDINT  */
    TYPE_ULINT = 326,              /* TYPE_ULINT  */
    TYPE_REAL = 327,               /* TYPE_REAL  */
    TYPE_LREAL = 328,              /* TYPE_LREAL  */
    TYPE_STRING = 329,             /* TYPE_STRING  */
    TYPE_WSTRING = 330,            /* TYPE_WSTRING  */
    TYPE_TIME = 331,               /* TYPE_TIME  */
    TYPE_DATE = 332,               /* TYPE_DATE  */
    TYPE_TIME_OF_DAY = 333,        /* TYPE_TIME_OF_DAY  */
    TYPE_DATE_AND_TIME = 334,      /* TYPE_DATE_AND_TIME  */
    TYPE_ARRAY = 335,              /* TYPE_ARRAY  */
    TYPE_STRUCT = 336,             /* TYPE_STRUCT  */
    END_STRUCT = 337,              /* END_STRUCT  */
    TYPE_UNION = 338,              /* TYPE_UNION  */
    END_UNION = 339,               /* END_UNION  */
    TYPE_ENUM = 340,               /* TYPE_ENUM  */
    END_ENUM = 341,                /* END_ENUM  */
    TYPE = 342,                    /* TYPE  */
    END_TYPE = 343,                /* END_TYPE  */
    ASSIGN = 344,                  /* ASSIGN  */
    EQ = 345,                      /* EQ  */
    NE = 346,                      /* NE  */
    LT = 347,                      /* LT  */
    LE = 348,                      /* LE  */
    GT = 349,                      /* GT  */
    GE = 350,                      /* GE  */
    PLUS = 351,                    /* PLUS  */
    MINUS = 352,                   /* MINUS  */
    MULTIPLY = 353,                /* MULTIPLY  */
    DIVIDE = 354,                  /* DIVIDE  */
    MOD = 355,                     /* MOD  */
    POWER = 356,                   /* POWER  */
    AND = 357,                     /* AND  */
    OR = 358,                      /* OR  */
    XOR = 359,                     /* XOR  */
    NOT = 360,                     /* NOT  */
    LPAREN = 361,                  /* LPAREN  */
    RPAREN = 362,                  /* RPAREN  */
    LBRACKET = 363,                /* LBRACKET  */
    RBRACKET = 364,                /* RBRACKET  */
    LBRACE = 365,                  /* LBRACE  */
    RBRACE = 366,                  /* RBRACE  */
    SEMICOLON = 367,               /* SEMICOLON  */
    COLON = 368,                   /* COLON  */
    COMMA = 369,                   /* COMMA  */
    DOT = 370,                     /* DOT  */
    RANGE = 371,                   /* RANGE  */
    ERROR_TOKEN = 372,             /* ERROR_TOKEN  */
    UMINUS = 373,                  /* UMINUS  */
    UPLUS = 374                    /* UPLUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 25 "st_parser.y"

    int integer;
    double real;
    int boolean;
    char *string;
    ast_node_t *node;
    struct type_info *type;
    operator_type_t op;
    var_category_t var_cat;
    param_category_t param_cat;
    function_type_t func_type;

#line 196 "st_parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_ST_PARSER_H_INCLUDED  */
