%option c++
%option noyywrap

%{
#include <map>
#include <vector>

#define MAX_LINE_LENGTH 256
#define MAX_ID_LENGTH 256
#define MAX_STRING_LENGTH 256
#define LIST                { buf = buf + std::string(YYText()); }
#define tokenString(t, s)   { LIST; if (Opt_Token) printf("<%s:%s>\n", t, s); }

int Opt_Source = 1;
int Opt_Token = 1;
int Opt_Statistic = 1;
int linenum = 1;
int *Opt = NULL;
std::string buf = "";
std::string str = "";

std::map<std::string, int> id_map;
std::vector<std::string> id_vec;
%}

keywd   (while|do|if|else|true|false|for|int|print|const|read|boolean|bool|void|float|double|string|continue|break|return)

space   [\t ]
delim   [\(\)\[\]\{\};,]
letter  [_a-zA-Z]
digit   [0-9]

 /* operator */
add     \+
sub     \-
mul     \*
div     \/
mod     \%
assign  =
compare \<|\<=|!=|\>=|\>|==
logical &&|\|\||!

%S SINGLE_COMMENT MULTI_COMMENT PRAGMA PRAGMA_VAL PRAGMA_END PRAGMA_COMMENT KEYWD STRING STRING_ESCAPE


%%

 /* Pragma */
<INITIAL>^"#pragma"{space}+ {
    BEGIN PRAGMA;
    LIST;
}

<PRAGMA>source{space}+ {
    BEGIN PRAGMA_VAL;
    LIST;
    Opt = &Opt_Source;
}
<PRAGMA>statistic{space}+ {
    BEGIN PRAGMA_VAL;
    LIST;
    Opt = &Opt_Statistic;
}
<PRAGMA>token{space}+ {
    BEGIN PRAGMA_VAL;
    LIST;
    Opt = &Opt_Token;
}
<PRAGMA_VAL>(on|off) {
    LIST;
    if(strcmp(YYText(), "on") == 0)
        *Opt = 1;
    else if(strcmp(YYText(), "off") == 0)
        *Opt = 0;
    BEGIN PRAGMA_END;
}

<PRAGMA_END>{space}* { LIST; }

 /* COMMENTS */
<PRAGMA_END>"//" {
    LIST;
    BEGIN SINGLE_COMMENT;
}
<PRAGMA_END>"/*" {
    LIST;
    BEGIN PRAGMA_COMMENT;
}
<PRAGMA_COMMENT>"*/" {
    LIST;
    BEGIN PRAGMA_END;
}
<PRAGMA_COMMENT>. {
    LIST;
}
<INITIAL>"//" {
    LIST;
    BEGIN SINGLE_COMMENT;
}
<INITIAL>"/*" {
    LIST;
    BEGIN MULTI_COMMENT;
}
<SINGLE_COMMENT>.*$ {
    LIST;
    BEGIN INITIAL;
}
<MULTI_COMMENT>"*/" {
    LIST;
    BEGIN INITIAL;
}
<MULTI_COMMENT>. {
    LIST;
}


 /* Token string */
<INITIAL>{delim} {
    tokenString("delim", YYText()); 
}

 /* keyword */
<INITIAL>{keywd} {
    tokenString("kw", YYText());
}

 /* operator */
<INITIAL>({add}|{sub}|{mul}|{mod}|{div}|{assign}|{compare}|{logical}) {
    tokenString("op", YYText());
}

 /* identifier */
<INITIAL>{letter}({letter}|{digit})* {
    tokenString("id", YYText());
    std::string id = std::string(YYText());
    if (id_map.find(id) == id_map.end()){
        id_vec.push_back(id);
        id_map[id] = 1;
    }
    else
        id_map[id] += 1;
}

 /* const value */
<INITIAL>0|([1-9]{digit}*) {
    tokenString("int", YYText());
} 
<INITIAL>{digit}+\.{digit}* {
    tokenString("float", YYText());
}
<INITIAL>{digit}+(\.{digit}+)?(E|e)(\+|\-)?{digit}+ {
    tokenString("sci", YYText());
}
<INITIAL>\" {
    LIST;
    BEGIN STRING;
}

<STRING>\\ {
    LIST;
    BEGIN STRING_ESCAPE;
}
<STRING>\" {
    tokenString("string", str.c_str());
    str = "";
    BEGIN INITIAL;
}
<STRING>. {
    LIST;
    str = str + std::string(YYText());
}

<STRING_ESCAPE>. {
    LIST;
    char ch = *YYText();
    if ( ch != '"' && ch != '\\')
        str = str + "\\";
    str = str + std::string(YYText());
    BEGIN STRING;
}

 /* Newline handling */
<PRAGMA_END>\n {
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf.c_str());
    ++linenum;
    buf = "";
    BEGIN INITIAL;
}

<STRING>\n {
    fprintf(stderr, "Error at line %d: %s\n", linenum, YYText());
    exit(1);
}

<PRAGMA_COMMENT>\n {
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf.c_str());
    ++linenum;
    buf = "";
    BEGIN MULTI_COMMENT;
}

\n {
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf.c_str());
    ++linenum;
    buf = "";
}

<INITIAL>{space} {
    LIST;
}

. { // Unrecognized character
    fprintf(stderr, "Error at line %d: %s\n", linenum, YYText());
    exit(1);
}

%%

#include <fstream>
int main( int argc, char **argv )
{   
    std::ifstream input;
    FlexLexer* lexer;

    if (argc > 1)
    {
        input.open(argv[1]);
        lexer = new yyFlexLexer( &input, &std::cout );
    }
    else
    {
        lexer = new yyFlexLexer();
    }

    while( lexer->yylex() != 0 );

    if (Opt_Statistic)
    {
        printf("frequencies of identifiers:\n");
        for(auto& f:id_vec){
            std::cout << f << "\t" << id_map[f] << std::endl;
        }
    }
    exit(0);
}
