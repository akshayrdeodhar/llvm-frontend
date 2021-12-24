## Operator Precedence Parsing
- ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) will parse the next
  expression with operator precedence _greater than or equal to_ `ExprPrec`,
  given LHS
- If the precedence of the next token is < required precedence, then the LHS
  itself is the expression
- If the precedence of the next token is > required precedence, then process
  that expression, and then return (op, LHS, processed expr)

- For accessing ptrs from a vector<unique_ptr<T>>, use for (const auto& x: vec)!
