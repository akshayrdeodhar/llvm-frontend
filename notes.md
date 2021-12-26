## Operator Precedence Parsing
- ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) will parse the next
  expression with operator precedence _greater than or equal to_ `ExprPrec`,
  given LHS
- If the precedence of the next token is < required precedence, then the LHS
  itself is the expression
- If the precedence of the next token is > required precedence, then process
  that expression, and then return (op, LHS, processed expr)

- For accessing ptrs from a vector<unique_ptr<T>>, use for (const auto& x: vec)!

## Codegen
- **Value**: Anything that can be used as an operand to other values.
  Instructions, Functions, Constants- are all Values. (I like to think of this
  as an SSA register)

- **Module** contains all information related to an LLVM module- list of global
  variables, functions, libraries that it depends on, symbol table

- **LLVMContext**: An opaque class for isolating different programs being
  compiled- I could have one program which uses n contexts for compiling n
  different programs- (a compiler server if you will). The context object can be
  used to store, get, use llvm constructs related to a _particular_ program.

- **IRBuilder**: This provides a uniform API for creating instructions and inserting them into a basic block: either at the end of a BasicBlock, or at a specific iterator location in a block. [ref](https://llvm.org/doxygen/classllvm_1_1IRBuilder.html#details)

- While generating code, you _create_ Calls, Functions, Instructions using the
  IRBuilder object. But you _get_ Types, Numeric Constants, 

- A `Constant` inherits `Value`. To generate a constant numeric value, one creates a constant

- ArrayRef means "anything that can be accessed using array notation []"

- Although undocumented, arg_size() returns the number of arguments to function

- You stupid man- a `Function` is a `Value`! So is a CallInst!

- Apparently, CreateCall cannot fail!- possibly because all it is doing is emitting some code

- Static class methods can be used without instantiating a class object

- The innermost error will do LogErrorV- when the error is received from something you call- don't use LogErrorV again.


- a * a + 2 * a * b + b * b
- LHS = a, Prec = 0
  tokprec = 40, BinOp = *, o
- The bug was: I was trying to look at precedence of next token before parsing the Primary Expression

- 