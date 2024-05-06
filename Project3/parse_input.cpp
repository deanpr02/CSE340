#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include <iostream>
#include "parse_input.h"
#include <map>
#include <vector>

using namespace std;

LexicalAnalyzer lexer = LexicalAnalyzer();
Token tmp;
map<string,int> var_list = map<string,int>();
vector<struct InstructionNode*> instructions = vector<struct InstructionNode*>();
int instruction_count = 0;
int previous = 0;
vector<int> case_jumps = vector<int>();

void connectInstructions(){
    if(instruction_count > 0){
        instructions[instruction_count-1]->next = instructions[instruction_count];
    }
}

void connectInstructions(int src, int dst){
    instructions[src]->next = instructions[dst];
}

void addConstant(string num){
    if(var_list.count(num) > 0){
        return;
    }
    else{
        var_list[num] = next_available;
        mem[next_available] = stoi(num);
        next_available++;
    }
}


//parse program
struct InstructionNode * parse_generate_intermediate_representation()
{
//parse var_section
if(lexer.peek(1).token_type == ID){
    parse_var_section();
}
//parse body
if(lexer.peek(1).token_type == LBRACE){
    parse_body();
}
//parse inputs
if(lexer.peek(1).token_type == NUM){
    parse_inputs();
}
return instructions[0]; 
}

void parse_var_section(){
if(lexer.peek(1).token_type == ID){
    parse_id_list();
}
//semicolon
if(lexer.peek(1).token_type == SEMICOLON){
    lexer.GetToken();
}
}

void parse_id_list(){
    //ID
    if(lexer.peek(1).token_type == ID){
        tmp = lexer.GetToken();
        //assign variables to 0
        var_list[tmp.lexeme] = next_available;
        mem[next_available] = 0;
        next_available++;
        if(lexer.peek(1).token_type == COMMA){
            lexer.GetToken();
            parse_id_list();
        }
    }
}

void parse_body(){
if(lexer.peek(1).token_type == LBRACE){
    lexer.GetToken();
}
parse_stmt_list();
if(lexer.peek(1).token_type == RBRACE){
    lexer.GetToken();
}
}

void parse_stmt_list(){
parse_stmt();
if(lexer.peek(1).token_type != RBRACE){
    parse_stmt_list();
}

}

void parse_stmt(){
TokenType test = lexer.peek(1).token_type;
if(test == ID){
    parse_assign_stmt();
}
if(test == WHILE){
    parse_while_stmt();
}
if(test == IF){
    parse_if_stmt();
}
if(test == SWITCH){
    parse_switch_stmt();
}
if(test == FOR){
    parse_for_stmt();
}
if(test == OUTPUT){
    parse_output_stmt();
}
if(test == INPUT){
    parse_input_stmt();
}
}

void parse_assign_stmt(){
    //ID
    tmp = lexer.GetToken();
    instructions.push_back(new InstructionNode);
    instructions[instruction_count]->type = ASSIGN;
    instructions[instruction_count]->next = NULL;
    instructions[instruction_count]->assign_inst.left_hand_side_index = var_list[tmp.lexeme];
    lexer.GetToken();
    if(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM){
        parse_primary();
    }
    TokenType t = lexer.peek(1).token_type;
    if(t == PLUS || t == MINUS || t == MULT || t == DIV){
        lexer.UngetToken(1);
        parse_expr();
        connectInstructions();
        instruction_count++;
    }
    //we are only assigning a value not an expression
    else{
        if(tmp.token_type == NUM){
            addConstant(tmp.lexeme);
        }
            instructions[instruction_count]->assign_inst.op = OPERATOR_NONE;
            instructions[instruction_count]->assign_inst.operand1_index = var_list[tmp.lexeme];
            connectInstructions();
            instruction_count++;
    }
    //semicolon
    lexer.GetToken();
}

struct InstructionNode* parse_for_assign_stmt(){
    //ID
    tmp = lexer.GetToken();
    struct InstructionNode* for_assign = new InstructionNode;
    for_assign->type = ASSIGN;
    for_assign->next = NULL;
    for_assign->assign_inst.left_hand_side_index = var_list[tmp.lexeme];
    //EQUAL - dont need
    lexer.GetToken();
    if(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM){
        parse_primary();
    }
    TokenType t = lexer.peek(1).token_type;
    if(t == PLUS || t == MINUS || t == MULT || t == DIV){
        lexer.UngetToken(1);
        parse_for_expr(for_assign);
    }
    //we are only assigning a value not an expression
    else{
        if(tmp.token_type == NUM){
            addConstant(tmp.lexeme);
        }
            instructions[instruction_count]->assign_inst.op = OPERATOR_NONE;
            instructions[instruction_count]->assign_inst.operand1_index = var_list[tmp.lexeme];
    }
    //semicolon
    lexer.GetToken();
    return for_assign;
}

void parse_expr(){
    parse_primary();
    if(tmp.token_type == NUM){
        addConstant(tmp.lexeme);
    }
    instructions[instruction_count]->assign_inst.operand1_index = var_list[tmp.lexeme];
    parse_op();
    parse_primary();
    if(tmp.token_type == NUM){
        addConstant(tmp.lexeme);
    }
    instructions[instruction_count]->assign_inst.operand2_index = var_list[tmp.lexeme];
}

void parse_for_expr(struct InstructionNode* node){
    parse_primary();
    if(tmp.token_type == NUM){
        addConstant(tmp.lexeme);
    }
    node->assign_inst.operand1_index = var_list[tmp.lexeme];
    parse_for_op(node);
    parse_primary();
    if(tmp.token_type == NUM){
        addConstant(tmp.lexeme);
    }
    node->assign_inst.operand2_index = var_list[tmp.lexeme];
}

void parse_primary(){
    if(lexer.peek(1).token_type == ID){
        //get ID
        tmp = lexer.GetToken();
        return;
    }
    if(lexer.peek(1).token_type == NUM){
        tmp = lexer.GetToken();
    }
}

void parse_op(){
    TokenType t = lexer.peek(1).token_type;
    if(t == PLUS){
        tmp = lexer.GetToken();
        instructions[instruction_count]->assign_inst.op = OPERATOR_PLUS;
        return;
    }
    if(t == MINUS){
        tmp = lexer.GetToken();
        instructions[instruction_count]->assign_inst.op = OPERATOR_MINUS;
        return;
    }
    if(t == MULT){
        tmp = lexer.GetToken();
        instructions[instruction_count]->assign_inst.op = OPERATOR_MULT;
        return;
    }
    if(t == DIV){
        tmp = lexer.GetToken();
        instructions[instruction_count]->assign_inst.op = OPERATOR_DIV;
    }
}

void parse_for_op(struct InstructionNode* node){
    TokenType t = lexer.peek(1).token_type;
    if(t == PLUS){
        tmp = lexer.GetToken();
        node->assign_inst.op = OPERATOR_PLUS;
        return;
    }
    if(t == MINUS){
        tmp = lexer.GetToken();
        node->assign_inst.op = OPERATOR_MINUS;
        return;
    }
    if(t == MULT){
        tmp = lexer.GetToken();
        node->assign_inst.op = OPERATOR_MULT;
        return;
    }
    if(t == DIV){
        tmp = lexer.GetToken();
        node->assign_inst.op = OPERATOR_DIV;
    }
}

void parse_output_stmt(){
    //output keyword
    lexer.GetToken();
    instructions.push_back(new InstructionNode);
    instructions[instruction_count]->type = OUT;
    instructions[instruction_count]->next = NULL;
    //ID
    tmp = lexer.GetToken();
    instructions[instruction_count]->output_inst.var_index = var_list[tmp.lexeme];
    connectInstructions();
    instruction_count++;
    lexer.GetToken();

}

void parse_input_stmt(){
    //input keyword
    lexer.GetToken();
    instructions.push_back(new InstructionNode);
    instructions[instruction_count]->type = IN;
    instructions[instruction_count]->next = NULL;
    //ID
    tmp = lexer.GetToken();
    instructions[instruction_count]->input_inst.var_index = var_list[tmp.lexeme];
    connectInstructions();
    instruction_count++;
    lexer.GetToken();
}

void parse_while_stmt(){
    //while keyword
    //while noop for end of while statement
    struct InstructionNode* nope = new InstructionNode;
    struct InstructionNode* jmp = new InstructionNode;
    nope->type = NOOP;
    nope->next = NULL;
    jmp->type = JMP;
    jmp->next = NULL;
    lexer.GetToken();
    jmp->jmp_inst.target = parse_condition(nope);
    parse_body();
    //push noop at end of instructions for while loop
    instructions.push_back(jmp);
    connectInstructions();
    instruction_count++;
    instructions.push_back(nope);
    connectInstructions();
    instruction_count++;

}

void parse_if_stmt(){
    //if keyword
    lexer.GetToken();
    struct InstructionNode* nope = new InstructionNode;
    nope->next = NULL;
    nope->type = NOOP;
    parse_condition(nope);
    parse_body();
    //NOOP instruction - added after condition instructions are added but we connected it to condiiton
    instructions.push_back(nope);
    connectInstructions();
    instruction_count++;
}

//nope instruction node is our corresponding noop for each condition which we will connect to if
//our condition is invalid
struct InstructionNode* parse_condition(struct InstructionNode* nope){
    parse_primary();
    struct InstructionNode* condition = new InstructionNode;
    instructions.push_back(condition);
    condition->type = CJMP;
    condition->next = NULL;
    if(tmp.token_type == NUM){
        addConstant(tmp.lexeme);
    }
    condition->cjmp_inst.operand1_index = var_list[tmp.lexeme];
    parse_relop();
    parse_primary();
    if(tmp.token_type == NUM){
        addConstant(tmp.lexeme);
    }
    condition->cjmp_inst.operand2_index = var_list[tmp.lexeme];
    condition->cjmp_inst.target = nope;
    connectInstructions();
    instruction_count++;
    return condition;
}

void parse_relop(){
    if(lexer.peek(1).token_type == GREATER){
        tmp = lexer.GetToken();
        instructions[instruction_count]->cjmp_inst.condition_op = CONDITION_GREATER;
        return;
    }
    if(lexer.peek(1).token_type == LESS){
        tmp = lexer.GetToken();
        instructions[instruction_count]->cjmp_inst.condition_op = CONDITION_LESS;
        return;
    }
    if(lexer.peek(1).token_type == NOTEQUAL){
        tmp = lexer.GetToken();
        instructions[instruction_count]->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
        return;
    }
}

void parse_switch_stmt(){
    string var_name;
    struct InstructionNode* nope = new InstructionNode;
    nope->type = NOOP;
    nope->next = NULL;
    //switch keyword
    lexer.GetToken();
    //ID
    tmp = lexer.GetToken();
    var_name = tmp.lexeme;
    lexer.GetToken();
    parse_case_list(var_name);
    if(lexer.peek(1).token_type == RBRACE){
        lexer.GetToken();
    }
    else{
    parse_default_case();
    lexer.GetToken();
    }

    for(int i = 0; i < case_jumps.size(); ++i){
        instructions[case_jumps[i]]->jmp_inst.target = nope;
    }
    instructions.push_back(nope);
    connectInstructions();
    instruction_count++;

}

void parse_for_stmt(){
    struct InstructionNode* jmp = new InstructionNode;
    struct InstructionNode* nope = new InstructionNode;
    struct InstructionNode* for_assign;
    nope->type = NOOP;
    nope->next = NULL;
    jmp->type = JMP;
    jmp->next = NULL;
    //for keyword
    lexer.GetToken();
    //lparen
    lexer.GetToken();
    parse_assign_stmt();
    jmp->jmp_inst.target = parse_condition(nope);
    //semicolon
    lexer.GetToken();
    for_assign = parse_for_assign_stmt();
    //rparen
    lexer.GetToken();
    parse_body();
    instructions.push_back(for_assign);
    connectInstructions();
    instruction_count++;
    instructions.push_back(jmp);
    connectInstructions();
    instruction_count++;
    instructions.push_back(nope);
    connectInstructions();
    instruction_count++;
}

void parse_case_list(string var){
    parse_case(var);
    if(lexer.peek(1).token_type != RBRACE && lexer.peek(1).token_type != DEFAULT){
        parse_case_list(var);
    }
}


void parse_case(string var){
    struct InstructionNode* condition = new InstructionNode;
    struct InstructionNode* nope = new InstructionNode;
    struct InstructionNode* jmp = new InstructionNode;
    struct InstructionNode* case_jmp = new InstructionNode;
    struct InstructionNode* test = new InstructionNode;
    int cond_index = instruction_count;
    condition->type = CJMP;
    condition->next = NULL;
    nope->type = NOOP;
    nope->next = NULL;
    jmp->type = JMP;
    jmp->next = NULL;
    case_jmp->type = JMP;
    case_jmp->next = NULL;
    test->type = NOOP;
    test->next = NULL;
    //case keyword
    lexer.GetToken();
    //need to check if var == num
    //num
    tmp = lexer.GetToken();
    //add condition instruction
    addConstant(tmp.lexeme);
    condition->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    condition->cjmp_inst.operand1_index = var_list[var];
    condition->cjmp_inst.operand2_index = var_list[tmp.lexeme];
    instructions.push_back(condition);
    connectInstructions();
    instruction_count++;
    jmp->jmp_inst.target = nope;
    instructions.push_back(jmp);
    connectInstructions();
    instruction_count++;
    //body of condition is a jump to next condition aka end of condition NOOP
    //NOOP will be after body code for that statement
    //need to generate if state
    instructions.push_back(test);
    connectInstructions();
    instruction_count++;
    instructions[cond_index]->cjmp_inst.target = instructions[instruction_count-1];
    lexer.GetToken();
    parse_body();
    //this is the cases body so we need to set conditions jump target to this.
    //instructions[cond_index]->cjmp_inst.target = instructions[cond_index+2];
    instructions.push_back(case_jmp);
    connectInstructions();
    case_jumps.push_back(instruction_count);
    instruction_count++;
    //need to update this at end of switch so we know where to jump to after finishing a statement
    //case_jumps.push_back(case_jmp);
    instructions.push_back(nope);
    connectInstructions();
    instruction_count++;
}

void parse_default_case(){
    //default keyword
    lexer.GetToken();
    lexer.GetToken();
    parse_body();
}

void parse_inputs(){
    parse_num_list();
}

void parse_num_list(){
    tmp = lexer.GetToken();
    inputs.push_back(stoi(tmp.lexeme));
    if(lexer.peek(1).token_type == NUM){
        parse_num_list();
    }
}

void parse_condition(){
    return;
}




