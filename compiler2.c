// Paul Vicino
// COP3402
// Spring 2021
// pa788323

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SYMBOL_TABLE 500
#define MAX_COMMANDS 500 
#define MAX_STACK_HEIGHT 50
#define MAX_CODE_LENGTH 100

// global variables
int procedurecount = 0;

// HW2 -> HW3 -> HW1

// this program recieves token list from HW2
// token list:
// 29 2 x 17 2 y 18 21 2 x 20 2 y 6 3 2 18 22 19

// this program prints
// 0    INC    0    6  
// 1    LOD    0    5  
// 2    LIT    0    2  
// 3    OPR    0    4  
// 4    STO    0    4  
// 5    SYS    0    3

// and produces the input for HW1 of
// 6 0 6
// 3 0 5
// 1 0 7
// 2 0 3
// 4 0 4
// 9 0 3

// for lex
enum token_types { 
	modsym = 1, identsym, numbersym, plussym, minussym, multsym, 
	slashsym, oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, 
	geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, 
	becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, 
	callsym, constsym, varsym, procsym, writesym, readsym, elsesym, returnsym 
};

// for parser
typedef struct {
	int kind;			// const = 1, var = 2, proc = 3
	char name[12];
	int val;
	int level;			// L level
	int addr;			// M address
	int mark;			// indicates that code has been generated already for a block (0 = unmarked)
	int param;			// indicates if the parameter for a procedure has been declared
} symbol;

// for lex
typedef struct {
	char name[11];
	int value;
	int token_type;
} token_table;

// function prototypes
int symbolTableCheck(symbol symbol_table[MAX_SYMBOL_TABLE], char *string, int *sip, int lexlevel);

int symbolTableSearch(symbol symbol_table[MAX_SYMBOL_TABLE], char *string, int *sip, int lexlevel, 
			int kind);

int findProcedure(symbol symbol_table[MAX_SYMBOL_TABLE], int *sip, int val);

void mark(symbol symbol_table[MAX_SYMBOL_TABLE], int *sip, int count);

int const_declaration(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel);

int var_declaration(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel, int param);

int procedure_declaration(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, 
			int lexlevel);

void statement(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel);

void condition(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel);

void expression(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel);

void term(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel);

void factor(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel);

void parse(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag,
			int lexlevel, int param, int procedureIndex);

// for VM
int base(int stack[], int level, int BP)
{
	int base = BP;
	int counter = level;

	while (counter > 0)
	{
		base = stack[base];
		counter--;
	}

	return base;
}


// is called to print out the 4 possible error codes
void lex_errors(char *error_type)
{
	// error type 1: variable doesnt start with letter
	if (strcmp(error_type, "digit_start") == 0)
	{
		printf("Error : Identifiers cannot begin with a digit\n");
		exit(0);
	}

	// error type 2: number too long
	if (strcmp(error_type, "digit_length") == 0)
	{
		printf("Error : Numbers cannot exceed 5 digits\n");
		exit(0);
	}

	// error type 3: name too long
	if (strcmp(error_type, "ident_length") == 0)
	{
		printf("Error : Identifier names cannot exceed 11 characters\n");
		exit(0);
	}

	// error type 4: invalid special symbol
	if (strcmp(error_type, "invalid_symbol") == 0)
	{
		printf("Error : Invalid Symbol\n");
		exit(0);
	}
}

// gets passed an error code and prints out the type of error
void parse_errors(int error_type)
{
	if (error_type == 1)
		printf("Error : program must end with period\n");
	else if (error_type == 2)
		printf("Error : const, var, procedure, call, and read keywords must be followed by identifier\n");	
	else if (error_type == 3)
		printf("Error : competing symbol declarations at the same level\n");	
	else if (error_type == 4)
		printf("Error : constants must be assigned with =\n");	
	else if (error_type == 5)
		printf("Error : constants must be assigned an integer value\n");	
	else if (error_type == 6)
		printf("Error : symbol declarations must be followed by a semicolon\n");	
	else if (error_type == 7)
		printf("Error : undeclared variable or constant in equation\n");	
	else if (error_type == 8)
		printf("Error : only variable values may be altered\n");	
	else if (error_type == 9)
		printf("Error : assignment statements must use :=\n");	
	else if (error_type == 10)
		printf("Error : begin must be followed by end\n");	
	else if (error_type == 11)
		printf("Error : if must be followed by then\n");	
	else if (error_type == 12)
		printf("Error : while must be followed by do\n");	
	else if (error_type == 13)
		printf("Error : condition must contain comparison operator\n");	
	else if (error_type == 14)
		printf("Error : right parenthesis must follow left parenthesis\n");
	else if (error_type == 15)
		printf("Error : arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
	else if (error_type == 16)
		printf("Error : undeclared procedure for call\n");
	else if (error_type == 17)
		printf("Error : parameters may only be specified by an identifier\n");
	else if (error_type == 18)
		printf("Error : parameters must be declared\n");
	else if (error_type == 19)
		printf("Error : cannot return from main\n");
}

// classifies the token type of string that is passed to it
// adds the data to the struct
// prints the name of the token and the token number (lexeme table)
void classification(char *string, token_table *tokens, int *jp, int lflag)
{
	enum token_types token;
	int length = strlen(string);
	int i = 0;


	// if string is purely whitespace, ignore
	if (length == 0)
		return;

	// strcpy(tokens[*jp].name, string);
	// tokens[*jp].value = (situational);
	// tokens[*jp].token_type = constsym;

	// check if string is a reserved word
	// const, var, procedure, call, if, then, else, while, do, read, write, odd, begin, end, return
	if (strcmp(string, "const") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = constsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, constsym);
		return;
	}
	else if(strcmp(string, "var") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = varsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, varsym);
		return;
	}
	else if(strcmp(string, "procedure") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = procsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, procsym);
		return;
	}
	else if(strcmp(string, "call") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = callsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, callsym);
		return;
	}
	else if(strcmp(string, "if") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = ifsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, ifsym);
		return;
	}
	else if(strcmp(string, "then") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = thensym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, thensym);
		return;
	}
	else if(strcmp(string, "else") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = elsesym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, elsesym);
		return;
	}
	else if(strcmp(string, "while") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = whilesym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, whilesym);
		return;
	}
	else if(strcmp(string, "do") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = dosym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, dosym);
		return;
	}
	else if(strcmp(string, "read") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = readsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, readsym);
		return;
	}
	else if(strcmp(string, "write") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = writesym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, writesym);
		return;
	}
	else if(strcmp(string, "odd") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = oddsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, oddsym);
		return;
	}
	else if(strcmp(string, "begin") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = beginsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, beginsym);
		return;
	}
	else if(strcmp(string, "end") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = endsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, endsym);
		return;
	}
	else if(strcmp(string, "return") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = returnsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, returnsym);
		return;
	}


	// check if string is special symbol
	// ‘+’, ‘-‘, ‘*’, ‘/’, ‘(‘, ‘)’, ‘=’, '<>', ’,’, ‘.’, ‘ <’, '<=', ‘>’, '>=', ‘;’, ’:’, ‘%’
	if(strcmp(string, "+") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = plussym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, plussym);
		return;
	}
	else if(strcmp(string, "-") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = minussym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, minussym);
		return;
	}
	else if(strcmp(string, "*") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = multsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, multsym);
		return;
	}
	else if(strcmp(string, "/") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = slashsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, slashsym);
		return;
	}
	else if(strcmp(string, "(") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = lparentsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, lparentsym);
		return;
	}
	else if(strcmp(string, ")") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = rparentsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, rparentsym);
		return;
	}
	else if(strcmp(string, "=") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = eqlsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, eqlsym);
		return;
	}
	else if(strcmp(string, "<>") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = neqsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, neqsym);
		return;
	}
	else if(strcmp(string, ",") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = commasym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, commasym);
		return;
	}
	else if(strcmp(string, ".") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = periodsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, periodsym);
		return;
	}
	else if(strcmp(string, "<") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = lessym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, lessym);
		return;
	}
	else if(strcmp(string, "<=") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = leqsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, leqsym);
		return;
	}
	else if(strcmp(string, ">") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = gtrsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, gtrsym);
		return;
	}
	else if(strcmp(string, ">=") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = geqsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, geqsym);
		return;
	}
	else if(strcmp(string, ";") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = semicolonsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, semicolonsym);
		return;
	}
	else if(strcmp(string, ":=") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = becomessym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, becomessym);
		return;
	}
	else if(strcmp(string, "%") == 0)
	{
		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = modsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, modsym);
		return;
	}

	// check if string is a number
	// check for errors (longer than 5 digits)
	if (isdigit(string[0]))
	{
		for (i = 0; i < length; i++)
		{
			if (isdigit(string[i]))
			{
				if (i > 4)
				{
					lex_errors("digit_length");
					return;
				}
			}
			else if (!isdigit(string[i]))
			{
				lex_errors("digit_start");
				return;
			}
		}

		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = numbersym;
		tokens[*jp].value = atoi(string);
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, numbersym);
		return;
	}

	// check if string is an indentifier (letter (letter|digit)*)
	// check for errors (identifier too long or does not start with letter)
	if (isalpha(string[0]))
	{
		if (length > 11)
		{
			lex_errors("ident_length");
			return;
		}

		strcpy(tokens[*jp].name, string);
		tokens[*jp].token_type = identsym;
		*jp = *jp + 1;
		if (lflag == 1)
			printf("	%s		%d\n", string, identsym);
		return;
	}


	// check for errors (doesnt match any special symbols)
	lex_errors("invalid_symbol");
	return;
}

// a function useful for testing and debugging
void printSymbolTable(symbol symbol_table[MAX_SYMBOL_TABLE], int *sip)
{
	int length = *sip;
	int i = 0;

	for (i = 0; i < length; i++)
	{
		printf("%d kind = %d name = %s val = %d level = %d addr = %d mark = %d param = %d\n", i, symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, 
			symbol_table[i].level, symbol_table[i].addr, symbol_table[i].mark, symbol_table[i].param);
	}

	return;
}

// checks if const or vars are within the symbol table at a certain lexlevel
int symbolTableCheck(symbol symbol_table[MAX_SYMBOL_TABLE], char *string, int *sip, int lexlevel)
{
	int length = *sip;
	int i = 0;

	for (i = 0; i < length; i++)
	{
		if (strcmp(symbol_table[i].name, string) == 0 && symbol_table[i].level == lexlevel && 
			symbol_table[i].mark == 0)
		{
			return i;
		}
	}

	return -1;
}

// searches name and level, returns index for exact match of 
// string and kind, unmarked, with nearest lexlevel
int symbolTableSearch(symbol symbol_table[MAX_SYMBOL_TABLE], char *string, int *sip, int lexlevel, int kind)
{
	int length = *sip;
	int i = 0;
	int j = lexlevel;

	for (j = lexlevel; j >= 0; j--)
	{
		for (i = 0; i < length; i++)
		{
			if (strcmp(symbol_table[i].name, string) == 0 && symbol_table[i].mark == 0 && 
				symbol_table[i].kind == kind && symbol_table[i].level == j)
			{
				return i;
			}
		}
	}

	return -1;
}

// search through table looking at procedure values, return index of value that matches
int findProcedure(symbol symbol_table[MAX_SYMBOL_TABLE], int *sip, int val)
{
	int length = *sip;
	int i = 0;

	for (i = 0; i < length; i++)
	{
		if (symbol_table[i].kind == 3)
		{
			if (symbol_table[i].val == val)
			{
				return i;
			}
		}
	}

	return -1;
}

// starts at end of symbol table and loops backwards.
// If entry is unmarked, mark it, count --
// else continue
void mark(symbol symbol_table[MAX_SYMBOL_TABLE], int *sip, int count)
{
	int length = *sip;
	int i = 0;

	for (i = length-1; i >= 0; i--)
	{
		if (symbol_table[i].mark == 0 && count > 0)
		{
			symbol_table[i].mark = 1;
			count = count - 1;
		}
		else
			continue;
	}

	return;
}


// is called when a constant needs to be added to the symbol table
int const_declaration(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel)
{
	char ident_name[1024];
	int numConst = 0;

	if (list[*tip].token_type == constsym)
	{
		do {
			numConst++;
			*tip = *tip + 1;

			if (list[*tip].token_type != identsym)
			{
				// error
				parse_errors(2);
				exit(0);
			}
			
			// makes sure const isnt already in the symbol table
			if (symbolTableCheck(symbol_table, list[*tip].name, sip, lexlevel) != -1)
			{
				// error
				parse_errors(3);
				exit(0);
			}

			strcpy(ident_name, list[*tip].name);

			*tip = *tip + 1;

			if (list[*tip].token_type != eqlsym)
			{
				// error
				parse_errors(4);
				exit(0);
			}

			*tip = *tip + 1;

			if (list[*tip].token_type != numbersym)
			{
				// error
				parse_errors(5);
				exit(0);
			}

			// adds const to symbol table
			symbol_table[*sip].kind = 1;
			strcpy(symbol_table[*sip].name, ident_name);
			symbol_table[*sip].val = list[*tip].value;
			symbol_table[*sip].level = lexlevel;
			symbol_table[*sip].addr = 0;
			symbol_table[*sip].mark = 0;
			symbol_table[*sip].param = 0;

			*sip = *sip + 1;
			*tip = *tip + 1;

		} while (list[*tip].token_type == commasym);

		if (list[*tip].token_type != semicolonsym)
		{
			// error
			parse_errors(6);
			exit(0);
		}

		*tip = *tip + 1;
	}

	return numConst;
}

// is called when a variable needs to be added to the symbol table
int var_declaration(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel, int param)
{
	int numVars = 0;

	if (param == 1)
		numVars = 1;
	else
		numVars = 0;

	if (list[*tip].token_type == varsym)
	{
		do {
			numVars++;
			*tip = *tip + 1;

			if (list[*tip].token_type != identsym)
			{
				// error
				parse_errors(2);
				exit(0);
			}
			
			// makes sure var isnt already in the symbol table
			if (symbolTableCheck(symbol_table, list[*tip].name, sip, lexlevel) != -1)
			{
				// error
				parse_errors(3);
				exit(0);
			}

			// adds var to the symbol table
			symbol_table[*sip].kind = 2;
			strcpy(symbol_table[*sip].name, list[*tip].name);
			symbol_table[*sip].val = 0;
			symbol_table[*sip].level = lexlevel;
			symbol_table[*sip].addr = numVars + 3;
			symbol_table[*sip].mark = 0;
			symbol_table[*sip].param = 0;

			*sip = *sip + 1;
			*tip = *tip + 1;
		} while (list[*tip].token_type == commasym);

		if (list[*tip].token_type != semicolonsym)
		{
			// error
			parse_errors(6);
			exit(0);
		}

		*tip = *tip + 1;
	}

	return numVars;
}

// declares and counts all the procedures
int procedure_declaration(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, 
			int lexlevel)
{
	int count = 0;
	int procIdx = 0;
	int numProc = 0;

	if (list[*tip].token_type == procsym)
	{
		do {
			// count = count + 1;
			numProc++;
			*tip = *tip + 1;

			if (list[*tip].token_type != identsym)
			{
				// error
				parse_errors(2);
				exit(0);
			}

			// makes sure var isnt already in the symbol table
			if (symbolTableCheck(symbol_table, list[*tip].name, sip, lexlevel) != -1)
			{
				// error
				parse_errors(3);
				exit(0);
			}

			procIdx = *sip;

			// adds proc to the symbol table
			symbol_table[*sip].kind = 3;
			strcpy(symbol_table[*sip].name, list[*tip].name);
			symbol_table[*sip].val = procedurecount;
			symbol_table[*sip].level = lexlevel;
			symbol_table[*sip].addr = 0;
			symbol_table[*sip].mark = 0;
			symbol_table[*sip].param = 0;

			procedurecount++;

			*sip = *sip + 1;
			*tip = *tip + 1;

			if (list[*tip].token_type == lparentsym)
			{
				*tip = *tip + 1;

				if (list[*tip].token_type != identsym)
				{
					// error
					parse_errors(2);
					exit(0);
				}

				// adds var to the symbol table
				symbol_table[*sip].kind = 2;
				strcpy(symbol_table[*sip].name, list[*tip].name);
				symbol_table[*sip].val = 0;
				symbol_table[*sip].level = lexlevel + 1;
				symbol_table[*sip].addr = 3;
				symbol_table[*sip].mark = 0;
				symbol_table[*sip].param = 0;

				*sip = *sip + 1;

				symbol_table[procIdx].param = 1;

				*tip = *tip + 1;

				if (list[*tip].token_type != rparentsym)
				{
					// error
					parse_errors(14);
					exit(0);
				}

				*tip = *tip + 1;

				if (list[*tip].token_type != semicolonsym)
				{
					// error
					parse_errors(6);
					exit(0);
				}

				*tip = *tip + 1;

				parse(list, symbol_table, length, commands, tip, sip, cip, aflag, 
					lexlevel + 1, 1, procIdx);
			}
			else
			{
				if (list[*tip].token_type != semicolonsym)
				{
					// error
					parse_errors(6);
					exit(0);
				}

				*tip = *tip + 1;

				parse(list, symbol_table, length, commands, tip, sip, cip, aflag, 
					lexlevel + 1, 0, procIdx);
			}

			if ((commands[*cip - 1][0] != 2 && commands[*cip - 1][2] != 0))
			{

				// EMIT LIT/1 0
				commands[*cip][0] = 1;
				commands[*cip][1] = 0;
				commands[*cip][2] = 0;

				*cip = *cip + 1;

				// EMIT RTN/2 0 0
				commands[*cip][0] = 2;
				commands[*cip][1] = 0;
				commands[*cip][2] = 0;

				*cip = *cip + 1;
			}

			if (list[*tip].token_type != semicolonsym)
			{
				// error
				parse_errors(6);
				exit(0);
			}

			*tip = *tip + 1;

		} while (list[*tip].token_type == procsym);
	}

	return count;
}

// is called whenever a statement is believed to be encountered
// produces error codes if expectations are not met
void statement(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel)
{
	int symIndex = 0;
	int jpcIndex = 0;
	int loopIndex = 0;
	int jmpIndex = 0;

	if (list[*tip].token_type == identsym)
	{
		symIndex = symbolTableSearch(symbol_table, list[*tip].name, sip, lexlevel, 2);

		if (symIndex == -1)
		{
			// error
			parse_errors(7);
			exit(0);
		}
		
		*tip = *tip + 1;

		if (list[*tip].token_type != becomessym)
		{
			// error
			parse_errors(9);
			exit(0);
		}

		*tip = *tip + 1;

		expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		// EMIT STO (L = lexlevel - symIndex.level, M = table[symIndex].addr)
		// STO/4 L M
		commands[*cip][0] = 4;
		commands[*cip][1] = lexlevel - symbol_table[symIndex].level;
		commands[*cip][2] = symbol_table[symIndex].addr;

		*cip = *cip + 1;

		return;
	}

	if (list[*tip].token_type == callsym)
	{
		*tip = *tip + 1;

		if (list[*tip].token_type != identsym)
		{
			// error
			parse_errors(2);
			exit(0);
		}

		symIndex = symbolTableSearch(symbol_table, list[*tip].name, sip, lexlevel, 3);

		if (symIndex == -1)
		{
			// error
			parse_errors(7);
			exit(0);
		}

		*tip = *tip + 1;

		if (list[*tip].token_type == lparentsym)
		{
			*tip = *tip + 1;

			if (symbol_table[symIndex].param != 1)
			{
				// error
				parse_errors(18);
				exit(0);
			}

			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			if (list[*tip].token_type != rparentsym)
			{
				// error
				parse_errors(14);
				exit(0);
			}

			*tip = *tip + 1;
		}
		else
		{
			// EMIT LIT 0
			commands[*cip][0] = 1;
			commands[*cip][1] = 0;
			commands[*cip][2] = 0;

			*cip = *cip + 1;
		}

		// EMIT CAL L = lexlevel - symIndex.level, M = symIndex.value
		commands[*cip][0] = 5;
		commands[*cip][1] = lexlevel - symbol_table[symIndex].level;
		commands[*cip][2] = symbol_table[symIndex].val;

		*cip = *cip + 1;

		return;
	}

	if (list[*tip].token_type == returnsym)
	{
		if (lexlevel == 0)
		{
			// error
			parse_errors(19);
			exit(0);
		}

		*tip = *tip + 1;

		if (list[*tip].token_type == lparentsym)
		{
			*tip = *tip + 1;

			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT RTN
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 0;

			*cip = *cip + 1;

			if (list[*tip].token_type != rparentsym)
			{
				// error
				parse_errors(14);
				exit(0);
			}

			*tip = *tip + 1;
		}
		else
		{
			// EMIT LIT 0
			commands[*cip][0] = 1;
			commands[*cip][1] = 0;
			commands[*cip][2] = 0;

			*cip = *cip + 1;

			// EMIT RTN
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 0;

			*cip = *cip + 1;
		}

		return;
	}

	if (list[*tip].token_type == beginsym)
	{
		do {
			*tip = *tip + 1;
			statement(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		} while (list[*tip].token_type == semicolonsym);

		if (list[*tip].token_type != endsym)
		{
			// error
			parse_errors(10);
			exit(0);
		}

		*tip = *tip + 1;

		return;
	}

	if (list[*tip].token_type == ifsym)
	{
		*tip = *tip + 1;
		condition(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);
		jpcIndex = *cip;

		// EMIT JPC/8 0 M
		commands[*cip][0] = 8;
		commands[*cip][1] = 0;
		commands[*cip][2] = jpcIndex;
		
		*cip = *cip + 1;

		if (list[*tip].token_type != thensym)
		{
			// error
			parse_errors(11);
			exit(0);
		}

		*tip = *tip + 1;

		statement(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		// MIGHT GO IN OR AFTER THE IF STATEMENT BELOW IT
		commands[jpcIndex][2] = *cip;

		if (list[*tip].token_type == elsesym)
		{
			*tip = *tip + 1;

			jmpIndex = *cip;

			// EMIT JMP/7 0 M
			commands[*cip][0] = 7;
			commands[*cip][1] = 0;
			commands[*cip][2] = jmpIndex;

			commands[jpcIndex][2] = *cip;

			statement(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			commands[jmpIndex][2] = *cip;
		}
		else
		{
			commands[jpcIndex][2] = *cip;
		}

		return;
	}

	if (list[*tip].token_type == whilesym)
	{
		*tip = *tip + 1;
		loopIndex = *cip;

		condition(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		if (list[*tip].token_type != dosym)
		{
			// error
			parse_errors(12);
			exit(0);
		}

		*tip = *tip + 1;

		jpcIndex = *cip;

		// EMIT JPC/8 0 M
		commands[*cip][0] = 8;
		commands[*cip][1] = 0;
		commands[*cip][2] = jpcIndex;

		*cip = *cip + 1;

		statement(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		// EMIT JMP/7 0 M (M = loopIndex)
		commands[*cip][0] = 7;
		commands[*cip][1] = 0;
		commands[*cip][2] = loopIndex;

		*cip = *cip + 1;

		commands[jpcIndex][2] = *cip;

		return;
	}

	if (list[*tip].token_type == readsym)
	{
		*tip = *tip + 1;

		if (list[*tip].token_type != identsym)
		{
			// error
			parse_errors(2);
			exit(0);
		}

		symIndex = symbolTableSearch(symbol_table, list[*tip].name, sip, lexlevel, 2);

		if (symIndex == -1)
		{
			// error
			parse_errors(7);
			exit(0);
		}

		*tip = *tip + 1;

		// EMIT READ/9 0 2
		commands[*cip][0] = 9;
		commands[*cip][1] = 0;
		commands[*cip][2] = 2;

		*cip = *cip + 1;

		// EMIT STORE/4 (L = lexlevel - symIndex.level, M = symIndex.addr)
		commands[*cip][0] = 4;
		commands[*cip][1] = lexlevel - symbol_table[symIndex].level;
		commands[*cip][2] = symbol_table[symIndex].addr;

		*cip = *cip + 1;

		return;
	}

	if (list[*tip].token_type == writesym)
	{
		*tip = *tip + 1;
		expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		// EMIT WRITE/9 0 1
		commands[*cip][0] = 9;
		commands[*cip][1] = 0;
		commands[*cip][2] = 1;

		*cip = *cip + 1;

		return;
	}
}

// is called whenever a condition is believed to be encountered
// produces error codes if expectations are not met
void condition(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel)
{

	if (list[*tip].token_type == oddsym)
	{
		*tip = *tip + 1;
		expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		// EMIT ODD/2 0 6
		commands[*cip][0] = 2;
		commands[*cip][1] = 0;
		commands[*cip][2] = 6;

		*cip = *cip + 1;
	}
	else
	{
		expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		if (list[*tip].token_type == eqlsym)
		{
			*tip = *tip + 1;
			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT EQL/2 0 8
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 8;

			*cip = *cip + 1;
		}
		else if (list[*tip].token_type == neqsym)
		{
			*tip = *tip + 1;
			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT NEQ/2 0 9
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 9;

			*cip = *cip + 1;
		}
		else if (list[*tip].token_type == lessym)
		{
			*tip = *tip + 1;
			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT LESS/2 0 10
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 10;

			*cip = *cip + 1;
		}
		else if (list[*tip].token_type == leqsym)
		{
			*tip = *tip + 1;
			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT LEQ/2 0 11
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 11;

			*cip = *cip + 1;
		}
		else if (list[*tip].token_type == gtrsym)
		{
			*tip = *tip + 1;
			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT GTR/2 0 12
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 12;

			*cip = *cip + 1;
		}
		else if (list[*tip].token_type == geqsym)
		{
			*tip = *tip + 1;
			expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			//EMIT GEQ/2 0 13
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 13;

			*cip = *cip + 1;
		}
		else
		{
			// error
			parse_errors(13);
			exit(0);
		}
	}
}

// is called whenever an expression is believed to be encountered
// produces error codes if expectations are not met
void expression(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel)
{

	if (list[*tip].token_type == minussym)
	{
		*tip = *tip + 1;
		term(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		// EMIT NEG/2 0 1
		commands[*cip][0] = 2;
		commands[*cip][1] = 0;
		commands[*cip][2] = 1;

		*cip = *cip + 1;

		while (list[*tip].token_type == plussym || list[*tip].token_type == minussym)
		{
			if (list[*tip].token_type == plussym)
			{
				*tip = *tip + 1;
				term(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

				// EMIT ADD/2 0 2
				commands[*cip][0] = 2;
				commands[*cip][1] = 0;
				commands[*cip][2] = 2;

				*cip = *cip + 1;
			}
			else
			{
				*tip = *tip + 1;
				term(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

				// EMIT SUB/2 0 3
				commands[*cip][0] = 2;
				commands[*cip][1] = 0;
				commands[*cip][2] = 3;

				*cip = *cip + 1;
			}
		}
	}
	else
	{
		if (list[*tip].token_type == plussym)
		{
			*tip = *tip + 1;
		}

		term(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		while (list[*tip].token_type == plussym || list[*tip].token_type == minussym)
		{
			if (list[*tip].token_type == plussym)
			{
				*tip = *tip + 1;
				term(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

				// EMIT ADD/2 0 2
				commands[*cip][0] = 2;
				commands[*cip][1] = 0;
				commands[*cip][2] = 2;

				*cip = *cip + 1;
			}
			else
			{
				*tip = *tip + 1;
				term(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

				// EMIT SUB/2 0 3
				commands[*cip][0] = 2;
				commands[*cip][1] = 0;
				commands[*cip][2] = 3;

				*cip = *cip + 1;
			}
		}
	}
}

// is called whenever a term is believed to be encountered
// produces error codes if expectations are not met
void term(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel)
{

	factor(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

	while (list[*tip].token_type == multsym || list[*tip].token_type == slashsym || list[*tip].token_type == modsym)
	{

		if (list[*tip].token_type == multsym)
		{
			*tip = *tip + 1;
			factor(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

			// EMIT MUL/2 0 4
			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 4;

			*cip = *cip + 1;
		}
		else if (list[*tip].token_type == slashsym)
		{
			*tip = *tip + 1;
			factor(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);
			// EMIT DIV/2 0 5

			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 5;

			*cip = *cip + 1;
		}
		else
		{
			*tip = *tip + 1;
			factor(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);
			// EMIT MOD/2 0 7

			commands[*cip][0] = 2;
			commands[*cip][1] = 0;
			commands[*cip][2] = 7;

			*cip = *cip + 1;
		}
	}
}

// is called whenever a factor is believed to be encountered
// produces error codes if expectations are not met
void factor(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag, int lexlevel)
{
	int symIndexV = 0;
	int symIndexC = 0;
	int symIndex = 0;

	if (list[*tip].token_type == identsym)
	{
		symIndexV = symbolTableSearch(symbol_table, list[*tip].name, sip, lexlevel, 2);
		symIndexC = symbolTableSearch(symbol_table, list[*tip].name, sip, lexlevel, 1);

		if (symIndexV == -1 && symIndexC == -1)
		{
			// error
			parse_errors(7);
			exit(0);
		}
		else if (symIndexC == -1 || (symIndexV != -1 && 
			symbol_table[symIndexV].level > symbol_table[symIndexC].level)) 
		{
			// EMIT LOD/3 L = lexlevel - symIndexV.level, M = symIndexV.addr
			commands[*cip][0] = 3;
			commands[*cip][1] = lexlevel - symbol_table[symIndexV].level;
			commands[*cip][2] = symbol_table[symIndexV].addr;

			*cip = *cip + 1;
		}
		else
		{
			// EMIT LIT/1 0 M (M = symIndexC.value)
			commands[*cip][0] = 1;
			commands[*cip][1] = 0;
			commands[*cip][2] = symbol_table[symIndexC].val;

			*cip = *cip + 1;
		}

		*tip = *tip + 1;
	}
	else if (list[*tip].token_type == numbersym)
	{
		// EMIT LIT/1 0 M (M = table[symIndex].val)
		commands[*cip][0] = 1;
		commands[*cip][1] = 0;
		commands[*cip][2] = list[*tip].value;

		*cip = *cip + 1;

		*tip = *tip + 1;
	}
	else if (list[*tip].token_type == lparentsym)
	{
		*tip = *tip + 1;
	
		expression(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

		if (list[*tip].token_type != rparentsym)
		{
			// error
			parse_errors(14);
			exit(0);
		}

		*tip = *tip + 1;
	}
	else if (list[*tip].token_type == callsym)
	{
		statement(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);
	}
	else
	{
		// error
		parse_errors(15);
		exit(0);
	}
}

// BLOCK
void parse(token_table *list, symbol symbol_table[MAX_SYMBOL_TABLE], 
			int length, int **commands, int *tip, int *sip, int *cip, int aflag,
			int lexlevel, int param, int procedureIndex)
{
	int i = 0;
	int m = 0;
	int numVars = 0;

	int c = const_declaration(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);
	int v = var_declaration(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel, param);
	int p = procedure_declaration(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

	// procedureIndex.addr = *cip;
	symbol_table[procedureIndex].addr = *cip;

	m = 4 + v;

	// EMIT INC (M = 4 + numVars)
	// 6 0 M
	commands[*cip][0] = 6;
	commands[*cip][1] = 0;
	commands[*cip][2] = m;

	*cip = *cip + 1;

	// will jump around throughout the entire token list
	statement(list, symbol_table, length, commands, tip, sip, cip, aflag, lexlevel);

	mark(symbol_table, sip, c + v + p);
}


// change void to text file
// change return int to token list
token_table *lex_main(FILE *ifp, int lflag, int *jp)
{
	token_table *tokens = malloc(sizeof(token_table) * 500);
	char buffer[1024] = "";
	int token_type = 0;
	char ch;
	int comment_flag = 0;
	int punct_flag = 0;
	int i;

	if (lflag == 1)
	{
		printf("Lexeme Table: \n");
		printf("	lexeme		token type\n");
	}

	// read input file
	i = 0;

	while(1)
	{
		ch = fgetc(ifp);

		// checks if we reached the end of the file
		// removes anything still left in the buffer
		if (feof(ifp))
		{
			if (buffer != "NULL")
			{
				classification(buffer, tokens, jp, lflag);
			}
			break;
		}

		// removes comments from being tokenized
		if (ch == '/')
		{
			ch = fgetc(ifp);

			if (feof(ifp))
				break;
			else if (ch == '*')
			{
				comment_flag = 1;
			}
			else if (ch == ' ' && comment_flag == 0)
			{
				classification("/", tokens, jp, lflag);
				memset(buffer, 0, sizeof(buffer));
				i = 0;
			}
		}

		if (ch == '*' && comment_flag == 1)
		{
			ch = fgetc(ifp);

			if (feof(ifp))
				break;
			else if (ch == '/')
			{
				comment_flag = 0;

				ch = fgetc(ifp);

				if (feof(ifp))
					break;
			}
		}

		// checks if we are currently in a comment
		if (comment_flag)
			continue;

		// handles the logic behind seperating tokens
		if (ch == ':')
		{
			ch = fgetc(ifp);

			if (feof(ifp))
				break;
			else if (ch == '=')
			{
				// printf(":=\n");
				classification(buffer, tokens, jp, lflag);
				memset(buffer, 0, sizeof(buffer));
				i = 0;
				classification(":=", tokens, jp, lflag);
				continue;
			}
			else
			{
				// printf("%s\n", buffer);
				classification(buffer, tokens, jp, lflag);
				// printf(":\n");
				classification(":", tokens, jp, lflag);
				memset(buffer, 0, sizeof(buffer));
				i = 0;
			}
		}
		else if (isspace(ch))
		{
			// printf("%s\n", buffer);
			classification(buffer, tokens, jp, lflag);
			memset(buffer, 0, sizeof(buffer));
			i = 0;
			punct_flag = 0;
			continue;
		}
		else if (ispunct(ch))
		{
			// printf("%s\n", buffer);

			if (ch == '<')
			{
				ch = fgetc(ifp);

				if (feof(ifp))
					break;
				else if (ch == '>')
				{
					classification("<>", tokens, jp, lflag);
					memset(buffer, 0, sizeof(buffer));
					i = 0;
					punct_flag = 1;
				}
				else if (ch == '=')
				{
					classification("<=", tokens, jp, lflag);
					memset(buffer, 0, sizeof(buffer));
					i = 0;
					punct_flag = 1;
				}
				else
				{
					classification("<", tokens, jp, lflag);
					memset(buffer, 0, sizeof(buffer));
					i = 0;
					punct_flag = 1;
					//printf("%c", ch);
				}
			}
			else if (ch == '>')
			{
				ch = fgetc(ifp);

				if (feof(ifp))
					break;
				else if (ch == '=')
				{
					classification(">=", tokens, jp, lflag);
					memset(buffer, 0, sizeof(buffer));
					i = 0;
					punct_flag = 1;
				}
				else
				{
					classification(">", tokens, jp, lflag);
					memset(buffer, 0, sizeof(buffer));
					i = 0;
					punct_flag = 1;
					//printf("%c", ch);
				}
			}
			else
			{
				classification(buffer, tokens, jp, lflag);
				memset(buffer, 0, sizeof(buffer));
				i = 0;
				punct_flag = 1;
				//printf("%c", ch);
				buffer[i] = ch;
			}
		}

		if ((!isspace(ch) && !ispunct(ch)))
		{
			if (punct_flag == 1)
			{
				// printf("%s\n", buffer);
				classification(buffer, tokens, jp, lflag);
				memset(buffer, 0, sizeof(buffer));
				i = 0;
			}

			buffer[i] = ch;
			punct_flag = 0;
		}

		i++;
	}

	if (lflag == 1)
	{
		printf("\n");
		printf("Token List:\n");

		// prints the token list
		for (i = 0; i < *jp; i++)
		{
			printf("%d ", tokens[i].token_type);

			if (tokens[i].token_type == 2)
				printf("%s ", tokens[i].name);

			if (tokens[i].token_type == 3) 
				printf("%d ", tokens[i].value);
		}

		printf("\n");
	}

	return tokens;
}

// prints out the general assembly commands
void printCommands(int **commands)
{
	int i = 0;

	for (i = 0; i < MAX_COMMANDS; i++)
	{
		if (commands[i][0] == 1)
			printf("%d	LIT	%d 	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 2)
			printf("%d	OPR	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 3)
			printf("%d	LOD	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 4)
			printf("%d	STO	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 5)
			printf("%d	CAL	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 6)
			printf("%d	INC	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 7)
			printf("%d	JMP	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 8)
			printf("%d	JPC	%d	%d\n", i, commands[i][1], commands[i][2]);
		else if (commands[i][0] == 9)
			printf("%d	SYS	%d	%d\n", i, commands[i][1], commands[i][2]);
	}
}

// input will be the output from homework 2 (token table)
// output will be the input for homework 1 (2d array)
// PROGRAM
int **parser_main(token_table *list, int aflag, int length, int **commands)
{
	symbol symbol_table[MAX_SYMBOL_TABLE];
	int table_index = 0;
	int *tip = &table_index;
	int symbol_index = 0;
	int *sip = &symbol_index;
	int command_index = 0;
	int *cip = &command_index;

	int numProc = 1;
	int i = 0;

	// EMIT JMP/7 0 M (we dont have m yet)
	commands[*cip][0] = 7;
	commands[*cip][1] = 0;
	commands[*cip][2] = 0;

	*cip = *cip + 1;

	// goes through entire token list, will return on the last token 
	// parse(list, symbol_table, length, commands, tip, sip, cip, aflag);

	for (i = 0; i < length; i++)
	{
		if (list[*tip].token_type == procsym)
		{
			numProc = numProc + 1;
			// emit JMP/7 0 M
			commands[*cip][0] = 7;
			commands[*cip][1] = 0;
			commands[*cip][2] = 0;
			*cip = *cip + 1;
		}
		*tip = *tip + 1;
	}

	*tip = 0;

	symbol_table[*sip].kind = 3;
	strcpy(symbol_table[*sip].name, "main");
	symbol_table[*sip].val = 0;
	symbol_table[*sip].level = 0;
	symbol_table[*sip].addr = 0;
	symbol_table[*sip].mark = 0;
	symbol_table[*sip].param = 0;

	*sip = *sip + 1;

	procedurecount = procedurecount + 1;

	// the 0 1 0 0 is lexlevel, numProc, param, procedureIndex
	parse(list, symbol_table, length, commands, tip, sip, cip, aflag, 0, 0, 0);

	// checks if the last token is a period, shows error if it isnt
	if (list[*tip].token_type != periodsym)
	{
		// error
		parse_errors(1);
		exit(0);
	}


	for (i = 0; i < numProc; i++)
	{
		commands[i][2] = symbol_table[findProcedure(symbol_table, sip, i)].addr;
	}

	for (i = 0; i <= *cip; i++)
	{
		if (commands[i][0] == 5)
			commands[i][2] = symbol_table[findProcedure(symbol_table, sip, commands[i][2])].addr;
	}

	if (aflag == 1)
	{
		// printf("\n");
		printf("Generated Assembly:\n");
		printf("Line	OP	L	M\n");
	}

	// EMIT SYS/9 0 3 (EMIT means add to commands)
	commands[*cip][0] = 9;
	commands[*cip][1] = 0;
	commands[*cip][2] = 3;

	return commands;
}

// goes through the commands and modifies the stack
void vm_main(int **commands, int vflag) 
{
	// int **commands = code;

	int PC;
	int BP;
	int SP;
	int stack[MAX_STACK_HEIGHT] = {0};
	int AR[1024] = {0};

	int i = 0;
	int j = 0;
	int lineCount = 0;
	int Halt = 1;

	// printf("\n");

	// initialize variables
	PC = 0;
	BP = 0;
	SP = -1;

	if (vflag == 1)
	{
		printf(" 		PC	BP	SP  	stack\n");
		printf("Initial values: %d	%d	%d \n", PC, BP, SP);
	}

	i = 0;

	// i = row
	// j = col
	// commands[i][0] = OP code
	// commands[i][1] = L
	// commands[i][2] = M

	// OP L M
	// 7  0 15
	// 7  0 2
	while (Halt == 1)
	{
		// if/else statements that lead to each individual operation

		if (commands[i][0] == 1)
		{
			// case 1 - LIT
			// Pushes a constant value (literal) M onto the stack

			SP = SP + 1;
			stack[SP] = commands[i][2];
			if (vflag == 1)
				printf("%d LIT ", i);
		}

		if (commands[i][0] == 2)
		{
			// case 2 - OPR
			//		  - 13 sub cases/operations
			// Operation to be performed on the data at the top of the stack

			if (commands[i][2] == 0)
			{
				// RTN
				stack[BP - 1] = stack[SP];
				SP = BP - 1;
				BP = stack[SP + 2];
				PC = stack[SP + 3];

				//printf("%d RTN ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 1)
			{
				// NEG
				stack[SP] = -1 * stack[SP];

				// printf("%d NEG ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 2)
			{
				// ADD
				SP = SP - 1;
				stack[SP] = stack[SP] + stack[SP + 1];

				// printf("%d ADD ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 3)
			{
				// SUB
				SP = SP - 1;
				stack[SP] = stack[SP] - stack[SP + 1];

				// printf("%d SUB ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 4)
			{
				// MUL
				SP = SP - 1;
				stack[SP] = stack[SP] * stack[SP + 1];

				// printf("%d MUL ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 5)
			{
				// DIV
				SP = SP - 1;
				stack[SP] = stack[SP] / stack[SP + 1];

				// printf("%d DIV ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 6)
			{
				// ODD
				stack[SP] = (stack[SP] % 2);

				// printf("%d ODD ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 7)
			{
				// MOD
				SP = SP - 1;
				stack[SP] = (stack[SP] % stack[SP + 1]);

				// printf("%d MOD ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 8)
			{
				// EQL
				SP = SP - 1;
				if (stack[SP] == stack[SP + 1])
					stack[SP] = 1;
				else
					stack[SP] = 0;

				// printf("%d EQL ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 9)
			{
				// NEQ (Not Equal)
				SP = SP - 1;
				if (stack[SP] != stack[SP + 1])
					stack[SP] = 1;
				else
					stack[SP] = 0;

				// printf("%d NEQ ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 10)
			{
				// LSS
				SP = SP - 1;
				if (stack[SP] < stack[SP + 1])
					stack[SP] = 1;
				else
					stack[SP] = 0;

				// printf("%d LSS ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 11)
			{
				// LEQ (Less than or Equal to)
				SP = SP - 1;
				if (stack[SP] <= stack[SP + 1])
					stack[SP] = 1;
				else
					stack[SP] = 0;

				// printf("%d LEQ ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 12)
			{
				// GTR
				SP = SP - 1;
				if (stack[SP] > stack[SP + 1])
					stack[SP] = 1;
				else
					stack[SP] = 0;

				// printf("%d GTR ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
			else if (commands[i][2] == 13)
			{
				// GEQ (Greater than or equal to)
				SP = SP - 1;
				if (stack[SP] >= stack[SP + 1])
					stack[SP] = 1;
				else
					stack[SP] = 0;

				// printf("%d GEQ ", i);
				if (vflag == 1)
					printf("%d OPR ", i);
			}
		}

		if (commands[i][0] == 3)
		{
			// case 3 - LOD
			// Load value to top of stack from the stack location at  
			// offset M from L lexicographical levels down

			SP = SP + 1;
			stack[SP] = stack[base(stack, commands[i][1], BP) + commands[i][2]];

			if (vflag == 1)
				printf("%d LOD ", i);
		}

		if (commands[i][0] == 4)
		{
			// case 4 - STO
			// Store value at top of stack in the stack location at 
			// offset M from L lexicographical levels down

			stack[base(stack, commands[i][1], BP) + commands[i][2]] = stack[SP];
			SP = SP - 1;

			if (vflag == 1)
				printf("%d STO ", i);
		}

		if (commands[i][0] == 5)
		{
			// case 5 - CAL
			// Call procedure at code index M (generates new Activation Record and PC <- M)

			// Static Link (SL)
			stack[SP + 1] = base(stack, commands[i][1], BP);

			// Dynamic Link (DL)
			stack[SP + 2] = BP;

			// Return Address (RA)
			stack[SP + 3] = PC;

			// parameter (P)
			stack[SP + 4] = stack[SP];
			BP = SP + 1;
			PC = commands[i][2];

			if (vflag == 1)
				printf("%d CAL ", i);
		}

		if (commands[i][0] == 6)
		{
			// case 6 - INC
			// Allocate  M  memory  words  (increment  SP  by  M).  
			// First  four  are  reserved  to Static  Link  (SL),  
			// Dynamic  Link  (DL),Return Address (RA), and Parameter (P)

			SP = SP + commands[i][2];

			if (vflag == 1)
				printf("%d INC ", i);
		}

		if (commands[i][0] == 7)
		{
			// case 7 - JMP
			// Jump to instruction M (PC <- M)

			PC = commands[i][2];

			if (vflag == 1)
				printf("%d JMP ", i);
		}

		if (commands[i][0] == 8)
		{
			// case 8 - JPC
			// Jump to instruction M if  top stack element is 0
			if (stack[SP] == 0)
				PC = commands[i][2];
			SP = SP - 1;

			if (vflag == 1)
				printf("%d JPC ", i);
		}

		if (commands[i][0] == 9)
		{
			// case 9 - SYS
			//		  - 3 sub cases
			// v1     - Write the top stack element to the screen
			// v2     - Read in input from the user and store it on top of the stack
			// v3     - End of program (Set Halt flag to zero)

			if (commands[i][2] == 1)
			{
				printf("Top of Stack Value: %d \n", stack[SP]);
				SP = SP - 1;

				if (vflag == 1)
					printf("%d SYS ", i);
			}

			if (commands[i][2] == 2)
			{
				if (vflag == 0)
					printf("\n");

				printf("Please Enter an Integer: \n");

				SP = SP + 1;
				scanf("%d", &stack[SP]);

				if (vflag == 1)
					printf("%d SYS ", i);
			}

			if (commands[i][2] == 3)
			{
				Halt = 0;

				if (vflag == 1)
					printf("%d SYS ", i);
			}
		}

		// increments PC
		if (PC == 0)
			PC++;

		// stores different activation records
		if (commands[i][0] == 5)
		{
			AR[SP + 1] = 1;
		}

		if (commands[i][0] == 2 && commands[i][1] == 0 && commands[i][2] == 0)
		{
			AR[SP + 1] = 0;
		}

		if (vflag == 1)
			printf("%d %d 	%d 	%d 	%d 	", commands[i][1], commands[i][2], PC, BP, SP);

		// print the stack length based on the stack pointer
		for (j = 0; j <= SP; j++)
		{
			// AR stuff from CAL
			if (j == BP && j != 0 || AR[j] == 1)
			{
				if (vflag == 1)
				{
					printf("| ");
					printf("%d ", stack[j]);
				}
			}
			else
			{
				if (vflag == 1)
					printf("%d ", stack[j]);
			}
		}

		if (vflag == 1)
			printf("\n");

		if (i == PC)
		{
			i++;
		}
		else
			i = PC;
		PC++;
	}
}

// driver.c
// call all 3 of the other programs
// LEX -> PARSER -> VM
// recieves a text file and directives as input
// must follow output based off directives
int main(int argc, char **argv)
{
	FILE *ifp = fopen(argv[1], "r");
	int **commands;
	int lflag = 0;
	int aflag = 0;
	int vflag = 0;
	int i;
	int j = 0;
	int *jp = &j;
	int n;

	if (argc < 2)
	{
		printf("Error : please include the file name\n");
		fclose(ifp);
		return 0;
	}
	else if (argc > 4)
	{
		lflag = 1;
		aflag = 1;
		vflag = 1;
	}
	else if (argc == 3 || argc == 4)
	{
		if (argv[2][1] == 'l')
			lflag = 1;
		else if (argv[2][1] == 'a')
			aflag = 1;
		else
			vflag = 1;

		if (argc == 4)
		{
			if (argv[3][1] == 'l')
				lflag = 1;
			else if (argv[3][1] == 'a')
				aflag = 1;
			else
				vflag = 1;
		}
	}

	if (ifp == NULL)
	{
		printf("No input file.\n");
		fclose(ifp);
		return 0;
	}

	commands = malloc(MAX_COMMANDS * sizeof(int *));

	for (i = 0; i < MAX_COMMANDS; i++)
	{
		commands[i] = (int *)malloc(3 * sizeof(int));
		commands[i][0] = -1;
		commands[i][1] = -1;
		commands[i][2] = -1;
	}

	token_table *list = lex_main(ifp, lflag, jp);

	if (lflag == 1 && (aflag == 1 || vflag == 1))
		printf("\n\n");

	parser_main(list, aflag, j, commands);

	if (aflag == 1)
		printCommands(commands);

	if (aflag == 1 && vflag == 1)
		printf("\n\n");

	vm_main(commands, vflag);

	for (i = 0; i < MAX_COMMANDS; i++)
		free(commands[i]);

	free(commands);

	fclose(ifp);
	return 0;
}