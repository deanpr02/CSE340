/*
 * Copyright (C) Rida Bazzi
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR","REALNUM", "BASE08NUM", "BASE16NUM" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;

    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        //When we are here, the token is either a 0 or some number string ex: 13492
        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        tmp.token_type = NUM;
        tmp.line_no = line_no;

        //Get the next input if we aren't empty
        if(!input.EndOfInput()){
            input.GetChar(c);
        }else{
            return tmp;
        }

        Token new_t = tmp;
        char hold;
        char hold2;
        char hold3;
        //REALNUM check
        //If we have a dot, we want to begin to build a token however, in case the next digits do not match the expression we want to return our old value
        if(c == '.'){
            int count = 0;
            hold = c;
            new_t.lexeme += c;
            if(!input.EndOfInput()){
                input.GetChar(c);
                while(!input.EndOfInput() && isdigit(c)){
                    count += 1;
                    new_t.lexeme += c;
                    input.GetChar(c);
                }
                if(!input.EndOfInput()){
                    input.UngetChar(c);
                }
                if(count >= 1){
                    new_t.token_type = REALNUM;
                    new_t.line_no = line_no;
                    return new_t;
                }
                else{
                    input.UngetChar(hold);
                }
            }
        }
        //Checking for base08 or pt1. base16
        //else if
        else if(c == 'x'){
           //has x in it
           hold = c;
           new_t.lexeme += c;
            if(!input.EndOfInput()){
                input.GetChar(c);
                hold2 = c;
                new_t.lexeme += c;
                if(!input.EndOfInput()){
                    input.GetChar(c);
                    new_t.lexeme += c;
                    string test = "";
                    test += hold2;
                    test += c;
                    if(test == "08"){
                        new_t.token_type = BASE08NUM;
                        new_t.line_no = line_no;
                        return new_t;
                    }
                    else if (test == "16"){
                        new_t.token_type = BASE16NUM;
                        new_t.line_no = line_no;
                        return new_t;
                    }
                    input.UngetChar(c);
                    input.UngetChar(hold2);
                    input.UngetChar(hold);
                }
                else{
                    input.UngetChar(c);
                    input.UngetChar(hold);
                    return tmp;
                }
            }
            else{
                input.UngetChar(c);
                return tmp;
            }
        }
        
    //Check for outlier base16nums such as those that have ABCDEF before the x
    else if(isxdigit(c) && isupper(c)){
        vector<char> store;
        store.push_back(c);
        new_t.lexeme += c;
        if(!input.EndOfInput()){
            input.GetChar(c);
        }
        while(!input.EndOfInput() && isxdigit(c) && isupper(c) || !input.EndOfInput() && isdigit(c)){
            store.push_back(c);
            new_t.lexeme += c;
            input.GetChar(c);
        }
        if(!input.EndOfInput() && c == 'x'){
            hold = c;
            new_t.lexeme += c;
            if(!input.EndOfInput()){
                input.GetChar(c);
                hold2 = c;
                new_t.lexeme += c;
                if(!input.EndOfInput()){
                    input.GetChar(c);
                    new_t.lexeme += c;
                    string test = "";
                    test += hold2;
                    test += c;
                    if(test == "16"){
                        new_t.token_type = BASE16NUM;
                        new_t.line_no = line_no;
                        return new_t;
                    }
                    input.UngetChar(c);
                    input.UngetChar(hold2);
                    input.UngetChar(hold);
                    for(int i = store.size() - 1; i >= 0; i--){
                        input.UngetChar(store[i]);
            }
            store.clear();
                }
                else{
                    input.UngetChar(c);
                    input.UngetChar(hold);
                    for(int i = store.size() - 1; i >= 0; i--){
                        input.UngetChar(store[i]);
            }
                    return tmp;
                }
            }
            else{
                input.UngetChar(c);
                for(int i = store.size() - 1; i >= 0; i--){
                input.UngetChar(store[i]);
            }
                return tmp;
            }
        }
        else{
            input.UngetChar(c);
            for(int i = store.size() - 1; i >= 0; i--){
                input.UngetChar(store[i]);
            }
            return tmp;
        }
    }




        else{
            input.UngetChar(c);
        }


        return tmp;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

//WE ARE ASSUMING THAT A HEX DIGIT STARTS WITH A NUMBER EVEN THOUGH THAT IS WRONG
Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
