
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lr_parser.h"
#include "../lr_parser_utils.h"



struct FLExpr;
struct FLInstr;
struct FLProgram;

enum FLExprKind
{
	FL_EXPR_INVALID = 0,
	FL_EXPR_VAR,
	FL_EXPR_CALL,
	FL_EXPR_CONSTR,
	FL_EXPR_SWITCH
};

typedef struct FLExpr
{
	enum FLExprKind kind;

	union {

		char * varName;

		struct {
			struct FLExpr * func;
			unsigned nbArgs;
			struct FLExpr ** args;
		} call;

		struct {
			char * constrName;
			unsigned nbArgs;
			struct FLExpr ** args;
		} constr;

		struct {
			struct FLExpr * toEvaluate;
			unsigned nbCases;
			char ** caseConstrNames;
			unsigned * caseNbArgs;
			char *** caseArgNames;
			struct FLExpr ** caseReturns;
			struct FLExpr * defaultReturn;
		} switching;
	};

} FLExpr;


void flExprPrint(const FLExpr * const expr)
{
	switch (expr->kind){

	case FL_EXPR_VAR:
		printf("%s", expr->varName);
		return;

	case FL_EXPR_CALL:
		printf("(");
		flExprPrint(expr->call.func);
		for (unsigned i = 0 ; i < expr->call.nbArgs ; i++){
			printf(" ");
			flExprPrint(expr->call.args[i]);
		}
		printf(")");
		return;

	case FL_EXPR_CONSTR:
		printf("{%s", expr->constr.constrName);
		for (unsigned i = 0 ; i < expr->constr.nbArgs ; i++){
			printf(" ");
			flExprPrint(expr->constr.args[i]);
		}
		printf("}");
		return;

	case FL_EXPR_SWITCH:
		printf("switch ");
		flExprPrint(expr->switching.toEvaluate);
		for (unsigned i = 0 ; i < expr->switching.nbCases ; i++){
			printf(" | %s", expr->switching.caseConstrNames[i]);
			for (unsigned j = 0 ; j < expr->switching.caseNbArgs[i] ; j++){
				printf(" %s", expr->switching.caseArgNames[i][j]);
			}
			printf(" -> ");
			flExprPrint(expr->switching.caseReturns[i]);
		}
		printf(" | _ -> ");
		flExprPrint(expr->switching.defaultReturn);
		printf(" end");
		return;

	case FL_EXPR_INVALID:
		printf("<INVALID>");
		return;

	default:
		printf("<UNKNOWN>");
		return;
	}
}



FLExpr * flExprNewVar(char * name)
{
	FLExpr * output = malloc(sizeof(FLExpr));
	if (output == NULL){
		return NULL;
	}
	output->kind = FL_EXPR_VAR;
	output->varName = name;
	return output;
}


FLExpr * flExprNewCall(FLExpr * func, unsigned nbArgs, FLExpr ** args)
{
	FLExpr * output = malloc(sizeof(FLExpr));
	if (output == NULL){
		return NULL;
	}
	output->kind = FL_EXPR_CALL;
	output->call.func = func;
	output->call.nbArgs = nbArgs;
	output->call.args = args;
	return output;
}


FLExpr * flExprNewConstr(char * constrName, unsigned nbArgs, FLExpr ** args)
{
	FLExpr * output = malloc(sizeof(FLExpr));
	if (output == NULL){
		return NULL;
	}
	output->kind = FL_EXPR_CONSTR;
	output->constr.constrName = constrName;
	output->constr.nbArgs = nbArgs;
	output->constr.args = args;
	return output;
}

FLExpr * flExprNewEmptySwitch(unsigned nbCases)
{
	FLExpr * expr = malloc(sizeof(FLExpr));
	if (expr == NULL){
		return NULL;
	}
	expr->kind = FL_EXPR_SWITCH;
	expr->switching.nbCases = nbCases;

	expr->switching.caseConstrNames = malloc(nbCases * sizeof(char *));
	if (expr->switching.caseConstrNames == NULL) goto ERR1;

	expr->switching.caseNbArgs = malloc(nbCases * sizeof(unsigned));
	if (expr->switching.caseNbArgs == NULL) goto ERR2;

	expr->switching.caseArgNames = malloc(nbCases * sizeof(char **));
	if (expr->switching.caseArgNames == NULL) goto ERR3;

	expr->switching.caseReturns = malloc(nbCases * sizeof(FLExpr *));
	if (expr->switching.caseReturns == NULL) goto ERR4;

	return expr;

	ERR4: free(expr->switching.caseArgNames);
	ERR3: free(expr->switching.caseNbArgs);
	ERR2: free(expr->switching.caseConstrNames);
	ERR1: free(expr);
	return NULL;
}



enum FLInstrKind
{
	FL_INSTR_INVALID = 0,
	FL_INSTR_TYPEDEF,
	FL_INSTR_LET
};

typedef struct FLInstr
{
	enum FLInstrKind kind;

	union {

		struct {
			char * name;
			unsigned nbArgs;
			char ** args;
			FLExpr * body;
		} let;

		struct {
			char * typeName;
			unsigned nbConstrs;
			char ** constrNames;
			unsigned * constrNbArgs;
			char *** constrArgTypes;
		} typeDef;
	};
} FLInstr;


void flInstrPrint(const FLInstr * const instr)
{
	switch (instr->kind){

	case FL_INSTR_LET:
		printf("let (%s", instr->let.name);
		for (unsigned i = 0 ; i < instr->let.nbArgs ; i++){
			printf(" %s", instr->let.args[i]);
		}
		printf(")\n\t");
		flExprPrint(instr->let.body);
		return;

	case FL_INSTR_TYPEDEF:
		printf("type %s", instr->typeDef.typeName);
		for (unsigned i = 0 ; i < instr->typeDef.nbConstrs ; i++){
			printf("\n\t| %s", instr->typeDef.constrNames[i]);
			for (unsigned j = 0 ; j < instr->typeDef.constrNbArgs[i] ; j++){
				printf(" %s", instr->typeDef.constrArgTypes[i][j]);
			}
		}
		return;

	case FL_INSTR_INVALID:
		printf("<INVALID>");
		return;

	default:
		printf("<UNKNOWN>");
		return;
	}
}

FLInstr * flInstrNewLet(char * name, unsigned nbArgs, char ** args, FLExpr * body)
{
	FLInstr * output = malloc(sizeof(FLInstr));
	if (output == NULL){
		return NULL;
	}
	output->kind = FL_INSTR_LET;
	output->let.name = name;
	output->let.nbArgs = nbArgs;
	output->let.args = args;
	output->let.body = body;
	return output;
}

FLInstr * flInstrNewEmptyTypeDef(char * typeName, unsigned nbConstrs)
{
	FLInstr * output = malloc(sizeof(FLInstr));
	if (output == NULL){
		return NULL;
	}
	output->kind = FL_INSTR_TYPEDEF;
	output->typeDef.nbConstrs = nbConstrs;
	output->typeDef.typeName = typeName;

	output->typeDef.constrNames = malloc(nbConstrs * sizeof(char *));
	if (output->typeDef.constrNames == NULL) goto ERR1;

	output->typeDef.constrNbArgs = malloc(nbConstrs * sizeof(unsigned));
	if (output->typeDef.constrNames == NULL) goto ERR2;

	output->typeDef.constrArgTypes = malloc(nbConstrs * sizeof(char **));
	if (output->typeDef.constrNames == NULL) goto ERR3;

	return output;

	ERR3: free(output->typeDef.constrNbArgs);
	ERR2: free(output->typeDef.constrNames);
	ERR1: free(output);
	return NULL;
}


typedef struct FLProgram
{
	unsigned nbInstructions;
	FLInstr ** instructions;
} FLProgram;

void flProgramPrint(const FLProgram * const prog)
{
	for (unsigned i = 0 ; i < prog->nbInstructions ; i++){
		flInstrPrint(prog->instructions[i]);
		printf("\n;\n");
	}
}



FLProgram * flProgramNew(unsigned nbInstructions, FLInstr ** instructions)
{
	FLProgram * output = malloc(sizeof(FLProgram));
	if (output == NULL){
		return NULL;
	}
	output->nbInstructions = nbInstructions;
	output->instructions = instructions;
	return output;
}




LR_PARSING_FUNCTION(flParsingFunction, {

		LR_SYMBOL(0, "let", "let", return NULL;)
		LR_SYMBOL(1, "type", "type", return NULL;)
		LR_SYMBOL(2, "switch", "switch", return NULL;)
		LR_SYMBOL(3, "end", "end", return NULL;)
		LR_SYMBOL(4, "|", "\\|", return NULL;)
		LR_SYMBOL(5, "->", "\\->", return NULL;)
		LR_SYMBOL(6, "(", "\\(", return NULL;)
		LR_SYMBOL(7, ")", "\\)", return NULL;)
		LR_SYMBOL(8, "{", "\\{", return NULL;)
		LR_SYMBOL(9, "}", "\\}", return NULL;)
		LR_SYMBOL(10, ";", ";", return NULL;)
		LR_SYMBOL(11, "_", "_", return NULL;)
		LR_SYMBOL(12, "var", "[a-zA-Z_]([a-zA-Z0-9_]*)", return strdup(symbolString);)
		LR_SYMBOL(13, "<SKIPPED>", "#((.-#)*)#", return NULL;)

}, {

		LR_RULE(0, "<START>", "ILIST <eof>", {
				LRPointerArray * array = values[0];
				FLProgram * prog = flProgramNew(array->nbElements, (void *) array->elements);
				free(array);
				return prog;
		})

		LR_RULE(1, "ILIST", "ILIST INSTR", {
				lrPointerArrayPushBack(values[0], values[1]);
				return values[0];
		})

		LR_RULE(2, "ILIST", "INSTR", {
				LRPointerArray * array = lrPointerArrayNew(1);
				lrPointerArrayPushBack(array, values[0]);
				return array;
		})

		LR_RULE(3, "VLIST", "VLIST var", {
				lrPointerArrayPushBack(values[0], values[1]);
				return values[0];
		})

		LR_RULE(4, "VLIST", "var", {
				LRPointerArray * array = lrPointerArrayNew(1);
				lrPointerArrayPushBack(array, values[0]);
				return array;
		})

		LR_RULE(5, "ELIST", "ELIST EXPR", {
				lrPointerArrayPushBack(values[0], values[1]);
				return values[0];
		})

		LR_RULE(6, "ELIST", "EXPR", {
				LRPointerArray * array = lrPointerArrayNew(1);
				lrPointerArrayPushBack(array, values[0]);
				return array;
		})

		LR_RULE(7, "INSTR", "LET", return values[0];)
		LR_RULE(8, "INSTR", "TYPEDEF", return values[0];)

		LR_RULE(9, "LET", "let ( VLIST ) EXPR ;", {
				LRPointerArray * array = values[2];
				char * name = lrPointerArrayPopFront(array);
				FLInstr * instr = flInstrNewLet(name, array->nbElements, (void *) array->elements, values[4]);
				free(array);
				return instr;
		})

		LR_RULE(10, "EXPR", "var", return flExprNewVar(values[0]);)

		LR_RULE(11, "EXPR", "( ELIST )", {
				LRPointerArray * array = values[1];
				FLExpr * func = lrPointerArrayPopFront(array);
				FLExpr * expr = flExprNewCall(func, array->nbElements, (void *) array->elements);
				free(array);
				return expr;
		})

		LR_RULE(12, "EXPR", "{ var ELIST }", {
				LRPointerArray * array = values[2];
				FLExpr * expr = flExprNewConstr(values[1], array->nbElements, (void *) array->elements);
				free(array);
				return expr;
		})

		LR_RULE(13, "EXPR", "switch EXPR SWITCHLIST | _ -> EXPR end", {
				LRPointerArray * array = values[2];
				const unsigned nbCases = array->nbElements;
				FLExpr * expr = flExprNewEmptySwitch(nbCases);
				expr->switching.toEvaluate = values[1];
				expr->switching.defaultReturn = values[6];

				for (unsigned i = 0 ; i < nbCases ; i++){
					LRPointerCouple * couple = array->elements[i];
					LRPointerArray * caseNameArray = couple->first;
					FLExpr * caseReturn = couple->second;
					char * constrName = lrPointerArrayPopFront(caseNameArray);
					expr->switching.caseConstrNames[i] = constrName;
					expr->switching.caseNbArgs[i] = caseNameArray->nbElements;
					expr->switching.caseArgNames[i] = (void *) caseNameArray->elements;
					expr->switching.caseReturns[i] = caseReturn;
					free(caseNameArray);
					free(couple);
				}
				return expr;
		})

		LR_RULE(14, "SWITCHLIST", "SWITCHLIST | CONSTRDEF -> EXPR", {
				LRPointerCouple * couple = lrPointerCoupleNew(values[2], values[4]);
				lrPointerArrayPushBack(values[0], couple);
				return values[0];
		})

		LR_RULE(15, "SWITCHLIST", "| CONSTRDEF -> EXPR", {
				LRPointerArray * array = lrPointerArrayNew(1);
				LRPointerCouple * couple = lrPointerCoupleNew(values[1], values[3]);
				lrPointerArrayPushBack(array, couple);
				return array;
		})

		LR_RULE(16, "TYPEDEF", "type var TYPEDEFLIST ;", {
				LRPointerArray * array = values[2];
				unsigned nbConstr = array->nbElements;
				FLInstr * instr = flInstrNewEmptyTypeDef(values[1], nbConstr);

				for (unsigned i = 0 ; i < nbConstr ; i++){
					LRPointerArray * nameArray = array->elements[i];
					char * constrName = lrPointerArrayPopFront(nameArray);
					instr->typeDef.constrNames[i] = constrName;
					instr->typeDef.constrNbArgs[i] = nameArray->nbElements;
					instr->typeDef.constrArgTypes[i] = (void *) nameArray->elements;
					free(nameArray);
				}
				lrPointerArrayFree(array);
				return instr;
		})

		LR_RULE(17, "TYPEDEFLIST", "TYPEDEFLIST | CONSTRDEF", {
				lrPointerArrayPushBack(values[0], values[2]);
				return values[0];
		})

		LR_RULE(18, "TYPEDEFLIST", "| CONSTRDEF", {
				LRPointerArray * array = lrPointerArrayNew(1);
				lrPointerArrayPushBack(array, values[1]);
				return array;
		})

		LR_RULE(19, "CONSTRDEF", "{ VLIST }", {
				return values[1];
		})

		LR_RULE(20, "CONSTRDEF", "var", {
				LRPointerArray * array = lrPointerArrayNew(1);
				lrPointerArrayPushBack(array, values[0]);
				return array;
		})
})


int main(int argc, char * argv[])
{
	const char * errorMessage;
	LRParser * parser = lrParserNewFromFunc(flParsingFunction);

	if (parser == NULL){
		fprintf(stderr, "Couldn't load parser\n");
		exit(EXIT_FAILURE);
	}

	printf("Done. Printing parser...\n");
	lrParserPrint(parser);
	char * sourceFile = "source_file.txt";
	printf("Done. Parsing %s...\n", sourceFile);
	LRParseTree * tree = lrParserParseFile(parser, sourceFile, &errorMessage);

	if (tree == NULL){
		fprintf(stderr, "Couldn't parse file\n");
		exit(EXIT_FAILURE);
	}

	printf("Done. Printing parse tree...\n");
	lrParseTreePrint(parser, tree);
	printf("Done. Applying parsing function...\n");

	FLProgram * parsedProgram = lrParserApplyFuncToParseTree(parser, tree);

	if (parsedProgram == NULL){
		fprintf(stderr, "Couldn't apply parsing function\n");
		exit(EXIT_FAILURE);
	}

	printf("Done. Printing program...\n");
	flProgramPrint(parsedProgram);
	printf("Done.\n");

	return EXIT_SUCCESS;
}
