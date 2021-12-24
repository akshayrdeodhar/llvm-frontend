#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cctype>
#include <cstdio>
#include <cstdlib>

using namespace llvm;

typedef enum {
		// EOF
		tok_eof = -1,

		// keywords
		tok_def = -2,
		tok_extern = -3,

		// things
		tok_number = -4,
		tok_identifier = -5,

		tok_error = -6
}Token_t;

static std::string IdentifierString;
static double NumValue;

static int gettok(void){
		static char LastChar = ' ';

		while(isspace(LastChar)){
				LastChar = getchar();
		}

		if (isalpha(LastChar)){
				IdentifierString = "";
				while(isalnum(LastChar) || LastChar == '_'){
						IdentifierString += LastChar;
						LastChar = getchar();
				}
				if (IdentifierString == "def"){
						return tok_def;
				}
				else if (IdentifierString == "extern"){
						return tok_extern;
				}
				else{
						return tok_identifier;
				}
		}
		else if (isdigit(LastChar) || LastChar == '.'){
				std::string NumStr = "";
				bool decimal = false;
				while(isdigit(LastChar) || (!decimal && (LastChar == '.'))){
						NumStr += LastChar;
						if (LastChar == '.') {
								decimal = true;
						}
						LastChar = getchar();
				}

				if (decimal && LastChar == '.'){
						return tok_error;
				}

				NumValue = strtod(NumStr.c_str(), nullptr);
				return tok_number;
		}
		else if (LastChar == '#'){
				while(LastChar != EOF && LastChar != '\n' && LastChar != '\r') {
						LastChar = getchar();
				}

				if (LastChar != EOF){
						return gettok(); // LOL, cheap trick, but well played
				}
		}
		else if (LastChar == EOF){
				return tok_eof;
		}
		else {
				int ThisChar = LastChar;
				LastChar = getchar();
				return ThisChar;
		}

		return tok_error;
}

// The different types of expressions:
// 
// ExprAST an expression a + f(b) + 5
// 
// 	NumExprAST a number 5
//
// 	VariableExprAST an identifier `a`
//
// 	CallExprAST a function call `f(b)
//
// 	BinaryExprAST a binary expression a + b
//
// PrototypeExprAST a function prototype f(a, b, c, d, e)
//
// FunctionExprAST a function declaration prototype
// - f(a, b)
//       a + f(b) + 5

class ASTVisitor;

class ExprAST {
		public:

				virtual ~ExprAST() {} // if a base class pointer points to a derived class object
				// when it goes out of scope, it should be
				// deallocated properly (TODO: understand properly)

				// TODO: throws "undefined reference to vtable for ExprAST" 
				// if accept is not declared as pure virtual
				// Why?
				virtual void accept(ASTVisitor& visitor) = 0;
};

class NumExprAST;
class VariableExprAST;
class BinaryExprAST;
class CallExprAST;
class PrototypeAST;
class FunctionAST;

// Visitor class for ExprAST

class ASTVisitor {
	public:

		virtual void visit(NumExprAST *p_obj) = 0;

		virtual void visit(VariableExprAST *p_obj) = 0;

		virtual void visit(CallExprAST *p_obj) = 0;

		virtual void visit(FunctionAST *p_obj) = 0;

		virtual void visit(PrototypeAST *p_obj) = 0;

		virtual void visit(BinaryExprAST *p_obj) = 0;

};




class NumExprAST: public ExprAST {
		private: // default access is private, be explicit
				double Val; 
		public:
				NumExprAST(double Val): Val(Val) {}
				void accept(ASTVisitor& visitor) { visitor.visit(this); }
				double GetVal() { return Val; }
};


class VariableExprAST: public ExprAST {
		private:
				std::string Name;
		public:
				VariableExprAST(const std::string &Name): Name(Name) {};
				void accept(ASTVisitor& visitor) { visitor.visit(this); }
				// Reference used because the string is not going to be used later
				// again, so why waste space? (and it's not going to be modified, so
				// const
				std::string& GetName() { return Name; }
};

class CallExprAST: public ExprAST {
		private:
				std::string Callee;
		public:
				// TODO: figure out a way to keep this private
				std::vector<std::unique_ptr <ExprAST> > Args;

				CallExprAST(std::string &Callee, 
								std::vector< std::unique_ptr <ExprAST> > Args_):
						Callee(Callee), Args(std::move(Args_)) {}
				// TODO: figure out why I need to use move here (because vector itself
				// is not a unique_ptr!, to get deleted before use)
				// probably so because moving a vector does not move it's contents, just
				// it's meta information, while moving a string moves its contents
				void accept(ASTVisitor& visitor) { visitor.visit(this); }
				std::string& GetCallee() { return Callee; }
};

class BinaryExprAST: public ExprAST {
		private: 
				char Op;
		public:
				// TODO: figure out a way to keep these private
				std::unique_ptr<ExprAST> LHS, RHS;
				BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
								std::unique_ptr<ExprAST> RHS):
						Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
				void accept(ASTVisitor& visitor) { visitor.visit(this); }
				char GetOp() { return Op; }
};

// Neither a prototype, nor a function is an "expression"

class PrototypeAST {
		private:
				std::string Name;
				std::vector< std::string > Args;
		public:
				PrototypeAST(const std::string &Name, 
								std::vector< std::string> Args):
						Name(Name), Args(std::move(Args)) {}

				void accept(ASTVisitor& visitor) { visitor.visit(this); }
				std::string& GetName() { return Name; }
				std::vector<std::string>& GetArgs() { return Args; }
};

class FunctionAST {
		private:
				// this is a tree! these should be pointers to members!
				// (a function should be an object pointing to these things, not an
				// object *containing* these things
		public:
				// TODO: Figure out a way to make these
				// unique_ptr members private, and still
				// access them from the visitor 
				std::unique_ptr<PrototypeAST> Proto;
				std::unique_ptr<ExprAST> Body;

				FunctionAST(std::unique_ptr<PrototypeAST> Proto, 
								std::unique_ptr<ExprAST> Body):
						Proto(std::move(Proto)), Body(std::move(Body)) {}
				// probably when I am getting passed a unique_ptr, the compiler sees
				// that it will get deleted when it goes out of scope- *move* is used to
				// indicate that I want to be a cannibal

				void accept(ASTVisitor& visitor) { visitor.visit(this); }
};

static int CurTok;

static void print_tok() {
		switch(CurTok) {
				case tok_number:
						std::cout << "(" << CurTok << ", " <<  NumValue << ")" << std::endl;
						break;
				case tok_identifier:
						std::cout << "(" << CurTok << ", " << IdentifierString << ")" << std::endl;
						break;
				case tok_def: case tok_extern:
						std::cout << "(" << CurTok << ", " << IdentifierString << ")" << std::endl;
						break;
				case tok_eof:
						std::cout << "(" << "End" << "," << 0 << ")" << std::endl;
						break;
				case tok_error:
						std::cout << "(" << "Error" << "," << 0 << ")" << std::endl;
						break;
				default:
						std::cout << "(" << (char)CurTok << "," << 0 << ")" << std::endl;
						break;
		}
}


int getNextToken() {
		CurTok = gettok();
		//print_tok();
		return CurTok;
}

std::unique_ptr<ExprAST> LogError(const char *Str) {
		fprintf(stderr, "LogError: %s\n", Str);
		return nullptr;
}

// TODO: what is this for?
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
		LogError(Str);
		return nullptr;
}

class LispPrintVisitor : public ASTVisitor {
	public:

		LispPrintVisitor(): nesting_depth(0) {}

		void visit(NumExprAST *p_obj) {
				std::cout << std::string(2 * nesting_depth, ' ') << p_obj->GetVal();
		}

		void visit(VariableExprAST *p_obj) {
				std::cout << std::string(2 * nesting_depth, ' ') << p_obj->GetName();
		}

		void visit(CallExprAST *p_obj) {
				std::cout << std::string(2 * nesting_depth, ' ') << '(' << p_obj->GetCallee();
				++nesting_depth;
				for (const auto& arg: p_obj->Args) {
						std::cout << std::endl;
						arg->accept(*this);
				}
				--nesting_depth;
				std::cout << ')';
		}

		void visit(FunctionAST *p_obj) {
				p_obj->Proto->accept(*this);
				std::cout << std::endl;
				++nesting_depth;
				p_obj->Body->accept(*this);
				--nesting_depth;
		}

		void visit(PrototypeAST *p_obj) {
				std::cout << "(def (" << p_obj->GetName();
				for (auto arg: p_obj->GetArgs()) {
						std::cout << ' ' << arg;
				}
				std::cout << ')';
		}

		void visit(BinaryExprAST *p_obj) {
				std::cout << std::string(2 * nesting_depth, ' ') << '(' << p_obj->GetOp() << std::endl;
				++nesting_depth;
				p_obj->LHS->accept(*this);
				std::cout << std::endl;
				p_obj->RHS->accept(*this);
				--nesting_depth;
				std::cout << ")";
		}

	private:
		int nesting_depth;

};




static std::unique_ptr<ExprAST> ParseNumberExpr();

static std::unique_ptr<ExprAST> ParseParenExpr();

static std::unique_ptr<ExprAST> ParseIdentifierExpr();

static std::unique_ptr<ExprAST> ParsePrimary();

static std::unique_ptr<ExprAST> ParseExpression();

static std::unique_ptr<ExprAST> ParseBinOpRHS(int, std::unique_ptr<ExprAST>);


// numberexpr ::= number
// the number has already been detected in gettok() and is present in 
static std::unique_ptr<ExprAST> ParseNumberExpr() {
		auto numberExpr = std::make_unique<NumExprAST>(NumValue);
		getNextToken();
		//fprintf(stderr, "debug: numberexpr\n");
		return std::move(numberExpr);
}

// parenexpr:: '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
		getNextToken();
		auto v = ParseExpression();

		if (CurTok != ')') {
				return LogError("expected: ')'");
		}

		//fprintf(stderr, "debug: parenexpr\n");

		getNextToken();
		return v;
}

// identifierexpr:
// 	::= identifier
// 	::= identifier '(' e + expression
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {

		std::string IdName =  IdentifierString; // produced by the tokenizer

		getNextToken(); // MUST EAT UP TOKEN BEFORE RETURNING, CURRENT TOKEN IS ID, GET NEXT TOKEN

		if (CurTok != '(') {
				//fprintf(stderr, "debug: identifier single\n");
				return std::make_unique<VariableExprAST>(IdName);
		}

		getNextToken();

		std::vector< std::unique_ptr<ExprAST> > Args;

		if (CurTok != ')') {
				while (1) {
						if (auto v = ParseExpression()) 
								Args.push_back(std::move(v)); // reuse v
						else
								return nullptr; // there should be an expression if parentheses don't close immediately

						if (CurTok == ')') 
								break;

						if (CurTok != ',') 
								return LogError("expected ',' or ')' in argument list");

						getNextToken();
				}
		}

		getNextToken(); // eat ')'

		//fprintf(stderr, "debug: identifier two\n");

		return std::make_unique<CallExprAST>(IdName, std::move(Args));
}


// primary:
// 	::= identifier
// 	::= numberexpr
// 	::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
		// lookahead?
		switch(CurTok) {
				case tok_number:
						return ParseNumberExpr();
						break;
				case tok_identifier:
						return ParseIdentifierExpr();
						break;
				case '(':
						return ParseParenExpr();
						break;
				default:
						return LogError("unknown token while trying to parse expression");
						break;
		}
		//fprintf(stderr, "debug: primary\n");
		return nullptr;
}


static std::map<char, int> BinopPrecedence;

static int getTokPrecedence() {
		//printf("debug: getting precedence of: ");
		if (!isascii(CurTok)) // not an operator, stop parsing expression
				return -1;

		int TokPrec = BinopPrecedence[CurTok];

		if (TokPrec <= 0) return -1;

		return TokPrec;
}

// TODO: install precedence in main()
// BinopPrecedence['<'] = 10;
// BinopPrecedence['+'] = 20;
// BinopPrecedence['-'] = 20;
// BinopPrecedence['*'] = 40;


// expression = 
// 	::= primary binoprhs
static std::unique_ptr<ExprAST> ParseExpression() {

		auto LHS = ParsePrimary();

		////fprintf(stderr, "debug: Parsed LHS\n");

		if (!LHS)
				return nullptr;

		////fprintf(stderr, "debug: Calling ParseBinOpRHS\n");

		//fprintf(stderr, "debug: expression\n");
		return ParseBinOpRHS(0, std::move(LHS));
}


// binoprhs = 
// 	::= (op binoprhs)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
		while (1) { // parses (op binoprhs)

				int TokPrec = getTokPrecedence();

				if (TokPrec < ExprPrec) {
						//fprintf(stderr, "debug: less precedence\n");
						return LHS;
				}

				int BinOp = CurTok;

				getNextToken(); // ate op

				int NextOpPrec = getTokPrecedence();

				//fprintf(stderr, "debug: Parsing RHS");
				auto RHS = ParsePrimary(); // ate binoprhs

				if (!RHS)
						return nullptr;

				// peek at next operator
				if (NextOpPrec > TokPrec) {
						// has larger precedence, eat all the large precedence
						// operators first
						RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));

						if (!RHS) 
								return nullptr;
				}

				// now the precedence of the next operator is less than or equal
				// to tokprec, so the previous expressions can be safely
				// combined
				//

				//std::cerr << "(" << "LHS" << " " << BinOp << " " << "RHS" << std::endl;

				LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));

				// we have our new LHS, on to the next!
		}
}


// prototype:
// 	::= identifier '(' identifier* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {

		if (CurTok != tok_identifier)
				return LogErrorP("Expected function name in prototype");

		std::string FunctionName = std::move(IdentifierString);

		getNextToken();

		if (CurTok != '(')
				return LogErrorP("Expected '(' in prototype");

		std::vector<std::string> Args;

		while (getNextToken() == tok_identifier)
				Args.push_back(std::move(IdentifierString));

		if (CurTok !=  ')')
				LogErrorP("Expected ',' in prototype");

		getNextToken(); // after parsing is done, fetch next token

		auto prot = std::make_unique<PrototypeAST>(FunctionName, std::move(Args));
		//fprintf(stderr, "debug: prototype\n");
		return prot;
}


// definition:
// 	::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {

		// eat up "def"
		getNextToken();

		auto Proto = ParsePrototype();

		if (!Proto) {
				return nullptr;
		}

		auto E = ParseExpression();

		if (!E) {
				return nullptr;
		}else {
				//fprintf(stderr, "debug: definition\n");
				return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
		}
}

// extern:
// 	::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {

		getNextToken();

		return ParsePrototype();
}

// toplevelexpr:
// 	::= expr
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {

		if (auto E = ParseExpression()) {

				auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());

				//fprintf(stderr, "debug: toplevelexpr\n");
				return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
		}

		return nullptr;
}


static void HandleDefinition() {
		LispPrintVisitor lvt;
		auto def = ParseDefinition();
		if (def) {
				def->accept(lvt);
				//fprintf(stderr, "Parsed a function definition\n");
		}else {
				getNextToken(); // skip token
		}
}


static void HandleExtern() {
		LispPrintVisitor lvt;
		const auto extn = ParseExtern();
		if (extn) {
				extn->accept(lvt);
				//fprintf(stderr, "Parsed an extern\n");
		}else {
				getNextToken();
		}
}

static void HandleTopLevelExpression() {
		LispPrintVisitor lvt;
		const auto tle = ParseTopLevelExpr();
		if (tle) {
				tle->accept(lvt);
				//fprintf(stderr, "Parsed a top level expression\n");
		}else {
				getNextToken();
		}
}

// top = definition | expression | external | ;
static void MainLoop() {
		while(1) {
				//fprintf(stderr, "ready>");
				switch (CurTok) {
						case tok_eof:
								return;
								break;
						case tok_def:
								HandleDefinition();
								break;
						case tok_extern:
								HandleExtern();
								break;
						case ';':
								getNextToken();
								break;
						default:
								HandleTopLevelExpression();
								break;
				}
		}
}


int oldmain(void) {
		int Token;
		while((Token = gettok())){
				switch(Token) {
						case tok_number:
								std::cout << "(" << Token << ", " <<  NumValue << ")" << std::endl;
								break;
						case tok_identifier:
								std::cout << "(" << Token << ", " << IdentifierString << ")" << std::endl;
								break;
						case tok_def: case tok_extern:
								std::cout << "(" << Token << ", " << IdentifierString << ")" << std::endl;
								break;
						case tok_eof:
								std::cout << "(" << "End" << "," << 0 << ")" << std::endl;
								return 0;
								break;
						case tok_error:
								std::cout << "(" << "Error" << "," << 0 << ")" << std::endl;
								break;
						default:
								std::cout << "(" << (char)Token << "," << 0 << ")" << std::endl;
								break;
				}
		}
		return 0;
}

int main(void) {
		BinopPrecedence['>'] = 10;
		BinopPrecedence['+'] = 20;
		BinopPrecedence['-'] = 20;
		BinopPrecedence['*'] = 40;

		fprintf(stderr, "ready>");
		getNextToken();

		MainLoop();
		//oldmain();

		return 0;
}


