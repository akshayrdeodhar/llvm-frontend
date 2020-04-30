#include <iostream>
#include <string>

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
static double Value;

static int gettok(void){
    static char LastChar = ' ';
    Token_t Token;

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

	Value = strtod(NumStr.c_str(), nullptr);
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

int main(void) {
    int Token;
    while((Token = gettok())){
	switch(Token) {
	    case tok_number:
		std::cout << "(" << Token << ", " <<  Value << ")" << std::endl;
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
