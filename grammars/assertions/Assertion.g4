grammar Assertion;

// Parser rules
assertion : distribution_assertion EOF ;

// distribution assertions
distribution_assertion
    : dis_or_expr
    ;

// OR has lowest precedence
dis_or_expr
    : dis_and_expr ('or' dis_and_expr)*;

// AND has medium precedence
dis_and_expr
    : dis_not_expr ('and' dis_not_expr)*;

// NOT has highest precedence
dis_not_expr
    : '!' dis_not_expr
    | '(' distribution_assertion ')'
    | probability_term RP REALNUM;

// probability term
probability_term
    : 'P' '(' states_assertion ')'
    | REALNUM                   
    | pfactor (MOP pfactor)*   
    ;

pfactor
    : 'P' '(' states_assertion ')'
    | REALNUM
    ;

// states assertion
states_assertion
    : states_or_expr
    ;

states_or_expr
    : states_and_expr ('or' states_and_expr)*   # or_expr
    ;

states_and_expr
    : states_not_expr ('and' states_not_expr)*  # and_expr
    ;

states_not_expr
    : '!' states_not_expr               # not_expr
    | quantum_term '=' quantum_term           # QCompare
    | binary_term '=' binary_term           # BCompare
    | real_term ROP real_term           # RCompare
    | '(' states_assertion ')'           # Parens
    ;

// real term
real_term : 'Tr(' quantum_term ')' | REALNUM | real_term ROP real_term;

binary_term
    : binary_factor (BOP binary_factor)* ;

binary_factor
    : '[' BList ']'          
    | BINARYSTRING
    ;

quantum_term
    : quantum_factor (MOP quantum_factor)* ;

quantum_factor
    : '[' QList ']'
    | matrix
    ;

// matrices
matrix : '[' Row (',' Row)* ']';
Row : '[' REALNUM (',' REALNUM)* ']';

// binary strings
BINARYSTRING: '0' | '1' ( '0' | '1' )*;

// Binary list
BList : ID (',' ID)* ;

// Quantum list
QList : ID (',' ID)* ;


// Lexer rules (uppercase usually)

RP : '=' | '>' | '>=' | '<=';
ROP: MOP | '/';
BOP: '>>' | '<<' | '&' | '|';
MOP : '+' | '-' | '*';
REALNUM : [0-9]+ ('.' [0-9]+)?;
ID  : [a-zA-Z_][a-zA-Z_0-9]* ;
INT : [0-9]+ ;
WS  : [ \t\r\n]+ -> skip ;
