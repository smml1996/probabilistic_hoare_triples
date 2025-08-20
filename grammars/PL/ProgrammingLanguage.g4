grammar ProgrammingLanguage;

// Parser rules
language : algorithm EOF ;


algorithm : 'skip' | classical_statement | UNITARY '(' qlist ')'
            | algorithm ';' algorithm | 'if' '(' ID ')' '{' algorithm '}' 'else' '{' algorithm '}'
            | algorithm '%' REALNUM '%';

classical_statement : ID '=' '0' | ID '=' '1' | ID '=' ID | ID '=' 'measure' '(' ID ')';

// Quantum list
qlist : ID (',' ID)* ;

UNITARY: [a-zA-Z_][a-zA-Z_]*;
REALNUM : [0-9]+ ('.' [0-9]+)?;
ID  : [a-zA-Z_][a-zA-Z_0-9]* ;