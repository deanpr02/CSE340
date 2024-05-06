#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include "lexer.h"
#include <utility>

using namespace std;

//utility struct for grammar data structure
struct node{
    string term;
    node* next;
};

//global lexer for parse functions
LexicalAnalyzer lex;

//if our grammar we read from input is invalid, then print out syntax error
void syntax_error(){
    cout<<"SYNTAX ERROR!!!\n";
    exit(0);
}

//If we reach a non-terminal that has multiple rules, store a "|" in grammar linked list for seperation
node* addRule(node* head){
    node* temp = head;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = new node();
    temp = temp->next;
    temp->term = "|";
    return temp;
}

//Utility function to check if a symbol is in a particular 
bool isIn(vector<string> first,string target){
    for(int k = 0; k < first.size(); ++k){
        if(first[k] == target){
            return true;
        }
    }
    return false;
}

//remove terminals from ordered list
map<string,int> extractTerminals(vector<string>ordered_terminals,vector<string>non_terminals,vector<string>& ordered_non_terminals){
    map<string,int> newterms = map<string,int>();
    for(int i = 0; i < ordered_terminals.size(); ++i){
        if(!isIn(non_terminals,ordered_terminals[i])){
            newterms[ordered_terminals[i]] = newterms.size()+1;
        }
        else{
            ordered_non_terminals.push_back(ordered_terminals[i]);
        }
    }
    return newterms;
}

void removeNonTerminals(vector<string>ordered_terminals,vector<string>non_terminals,vector<string>& onlyTerms){
    for(int i = 0; i < ordered_terminals.size(); ++i){
        if(!isIn(non_terminals,ordered_terminals[i])){
            onlyTerms.push_back(ordered_terminals[i]);
        }
    }
}

//Pass by reference so i modify the map, not a copy of the map
void initializeSets(map<string,vector<string>>&mp,vector<node*> grammar){
    vector<string> tmp = vector<string>();
    for(int i = 0; i<grammar.size(); ++i){
        mp[grammar[i]->term] = tmp;
    }
}

void addToFirstInOrder(vector<string>& firstSet,string terminal,map<string,int>terminals){
    int i = 0;
    if(terminal == "#"){
        firstSet.insert(firstSet.begin(),terminal);
        return;
    }
    if(i > 0 && firstSet[0] == "#"){
        ++i;
    }
    int key = terminals[terminal];
    for(i; i < firstSet.size(); ++i){
        if(key < terminals[firstSet[i]]){
            firstSet.insert(firstSet.begin()+i,terminal);
            return;
        }
        }
        firstSet.push_back(terminal);
    }

bool addToFollowInOrder(vector<string>& followSet, vector<string> terms, map<string,int>terminals, bool& change){
    bool epsilon = false;
    bool check;
    int key;
    for(int i = 0; i < terms.size(); ++i){
        check = true;
        //If epsilon is true then we will want to move on to the next var
        if(terms[i] == "#"){
            epsilon = true;
            continue;
        }
        if(!isIn(followSet,terms[i])){
            if(terms[i] == "$"){
                followSet.insert(followSet.begin(),terms[i]);
                continue;
            }
            key = terminals[terms[i]];
            for(int j = 0; j < followSet.size(); ++j){
            if(key < terminals[followSet[j]]){
                followSet.insert(followSet.begin()+j,terms[i]);
                check = false;
                change = true;
                break;
            }
            }
            if(check){
                followSet.push_back(terms[i]);
                change = true;
            }
        }

    }
    return epsilon;
}


void checkAllRules(node* ruleSet, vector<string> non_terminals, vector<string>& firstSet,bool& change, map<string,vector<string>> allSets,map<string,int>terminals){
    node* temp = ruleSet;
    while(temp != NULL){
        if(temp->term == "|"){
            temp = temp->next;
        }
        int isNonTerminal = count(non_terminals.begin(),non_terminals.end(),temp->term);
        if(!isNonTerminal){
            int isFound = count(firstSet.begin(),firstSet.end(),temp->term);
            if(isFound == 0){
                addToFirstInOrder(firstSet,temp->term,terminals);
                change = true;
            }
                //if we find a terminal we want to end or go to next rule
                while(temp != NULL && temp->term != "|"){
                    temp = temp->next;
                }
        }
        //if we find a non-terminal
        else{
            //get the first set of non-terminal
            //if epsillon is in it then recursively call
            vector<string> tempSet = allSets[temp->term];
            int check;
            for(int j = 0; j < tempSet.size(); ++j){
                check = count(firstSet.begin(),firstSet.end(),tempSet[j]);
                if(tempSet[j] == "#"){
                    if(temp->next != NULL && temp->next->term != "|"){
                        continue;
                    }
                }
                if(check == 0){
                    addToFirstInOrder(firstSet,tempSet[j],terminals);
                    change = true;
                }
        }
        if(!isIn(tempSet,"#")){
            while(temp != NULL && temp->term != "|"){
                    temp = temp->next;
                }
        }
        else{
            temp = temp->next;
        }

    }
}
}

void printFirstSets(map<string,vector<string>>firstSets,vector<string>ordered_non_terminals){
    for(int k = 0; k < ordered_non_terminals.size(); ++k){
        cout<<"FIRST("<< ordered_non_terminals[k] << ") = { ";
        for(int z = 0; z < firstSets[ordered_non_terminals[k]].size();++z){
            cout<< firstSets[ordered_non_terminals[k]][z];
            if(z == firstSets[ordered_non_terminals[k]].size()-1){
                cout<< " ";
            }
            else{
                cout<<", ";
            }
    }
    cout<<"}\n";
}
}

void printFollowSets(map<string,vector<string>>followSets,vector<string>ordered_non_terminals){
    for(int k = 0; k < ordered_non_terminals.size(); ++k){
        cout<<"FOLLOW("<< ordered_non_terminals[k] << ") = { ";
        for(int z = 0; z < followSets[ordered_non_terminals[k]].size();++z){
            cout<< followSets[ordered_non_terminals[k]][z];
            if(z == followSets[ordered_non_terminals[k]].size()-1){
                cout<< " ";
            }
            else{
                cout<<", ";
            }
    }
    cout<<"}\n";
}
}

map<string,vector<string>> calculateFirstSets(vector<node*> grammar,map<string,int>& terminals, vector<string>& non_terminals,vector<string>ordered_non_terminals){
    map<string,vector<string>> firstSets;
    //We know have empty sets for each non-terminal
    initializeSets(firstSets,grammar);
    node* rule;
    bool change = true;
    while(change){
        change = false;
        for(int i = 0; i < grammar.size(); ++i){
            checkAllRules(grammar[i]->next,non_terminals,firstSets[grammar[i]->term],change,firstSets,terminals);
        }
    }
    return firstSets;

}

int findGrammarSpot(vector<node*> grammar, string target){
    for(int i = 0; i < grammar.size(); ++i){
        if(grammar[i]->term == target){
            return i;
        }
    }
    return -1;
}

node* getNextRule(node* head){
    while(head != NULL && head->term != "|"){
        head = head->next;
    }
    return head;
}

map<string,vector<string>> calculateFollowSets(map<string,vector<string>>firstSets,vector<string>ordered_non_terminals,vector<node*>grammar,map<string,int>& terminals){
    map<string,vector<string>> followSets;
    initializeSets(followSets,grammar);
    //Initialize start variable with EOF terminal
    followSets[grammar[0]->term].push_back("$");
    int index;
    bool change = true;
    bool epsilon = false;
    node* head;
    node* temp;
    vector<string> firstSet;
    vector<string> terms;
    while(change){
        change = false;
    for(int i = 0; i < ordered_non_terminals.size(); ++i){
        index = findGrammarSpot(grammar,ordered_non_terminals[i]);
        head = grammar[index];
        temp = head;
        temp = temp->next;
        while(temp != NULL){
            if(temp->term == "|"){
                temp = temp->next;
            }
            //If its a terminal, add it and go to next rule;
            if(!isIn(ordered_non_terminals,temp->term)){
                temp = temp->next;
                continue;
            }
            epsilon = true;
            node* temp_n = temp;
            while(epsilon){
                epsilon = false;
                //If we reach end of a rule add follow of non_terminal to follow of non-terminal
                if(temp_n->next == NULL || temp_n->next != NULL && temp_n->next->term == "|"){
                    addToFollowInOrder(followSets[temp->term],followSets[head->term],terminals,change);
                    break;
                }
                else if(temp_n->next != NULL && !isIn(ordered_non_terminals,temp_n->next->term)){
                    terms.push_back(temp_n->next->term);
                    addToFollowInOrder(followSets[temp->term],terms,terminals,change);
                    terms.clear();
                    //temp = getNextRule(temp);
                    break;
                }
                else{
                    if(temp_n->next != NULL && temp_n->next->term != "|"){
                        firstSet = firstSets[temp_n->next->term];
                        epsilon = addToFollowInOrder(followSets[temp->term],firstSet,terminals,change);
                    }
                    else{
                        addToFollowInOrder(followSets[temp->term],followSets[head->term],terminals,change);
                        break;
                    }
                }
                temp_n = temp_n->next;
            }
            if(temp != NULL){
                temp = temp->next;
            }
            terms.clear();
        }
    }
    }
    return followSets;

}

bool checkVectors(vector<vector<string>>rules, vector<string>target_rule){
    for(int i =0; i < rules.size(); ++i){
        if(rules[i] == target_rule){
            return true;
        }
    }
    return false;
}

bool hasEpsilonRule(vector<vector<string>>rules,string bruh){
    vector<string> temp = vector<string>();
    temp.push_back("#");
    for(int i =0; i < rules.size(); ++i){
            if(rules[i] == temp){
                return true;
            }
    }
    return false;
}

bool checkEpsilon(node* rule){
    node* temp = rule;
    while(temp != NULL){
        if(temp->term == "#"){
            return true;
        }
        temp = temp->next;
    }
    return false;
}

vector<string> grabRule(node*& temp){
    vector<string> rule = vector<string>();
    while(temp != NULL && temp->term != "|"){
        rule.push_back(temp->term);
        temp = temp->next;
    }
    if(temp!= NULL && temp->term == "|"){
        temp = temp->next;
    }
    return rule;
}

void printRemovedRules(map<string,vector<vector<string>>> ruleSet, vector<string>reachable,vector<node*>grammar,vector<pair<string,vector<string>>>ordered_rules){
    node* head;
    node* temp;
    int index;
    vector<string> rule;
    for(int i = 0; i < ordered_rules.size();++i){
        if(isIn(reachable,ordered_rules[i].first)){
            rule = ordered_rules[i].second;
            if(checkVectors(ruleSet[ordered_rules[i].first],rule)){
                cout<<ordered_rules[i].first<< " -> ";
                for(int z = 0; z < rule.size();++z){
                    cout<<rule[z]<< " ";
                }
                cout<<"\n";
                }
            }
        }
    }


void removeUnreachable(map<string,vector<vector<string>>>&ruleSet,vector<node*> grammar,vector<string>terminals,vector<pair<string,vector<string>>>ordered_rules,vector<string>& reachable,int task){
    terminals.push_back("#");
    bool change = true;
    reachable.push_back(grammar[0]->term);
    while(change){
        change = false;
        if(ruleSet[grammar[0]->term].size() == 0){
            break;
        }
        for(int i = 0; i < grammar.size(); ++i){
            if(isIn(reachable,grammar[i]->term)){
                for(int j = 0; j < ruleSet[grammar[i]->term].size();++j){
                    for(int k = 0; k < ruleSet[grammar[i]->term][j].size();++k){
                        if(!isIn(terminals,ruleSet[grammar[i]->term][j][k])){
                            if(!isIn(reachable,ruleSet[grammar[i]->term][j][k])){
                            reachable.push_back(ruleSet[grammar[i]->term][j][k]);
                            change = true;
                        }
                    }
                        }
                    }
                }
            }
        }
        if(task == 2){
        printRemovedRules(ruleSet,reachable,grammar,ordered_rules);
        }
    }


void removeNonGenerating(map<string,vector<vector<string>>>&ruleSet, vector<string> terminals, vector<node*> grammar){
    vector<string> generating = vector<string>();
    vector<string> keptRule;
    generating.push_back("#");
    for(int i = 0; i < terminals.size(); ++i){
        generating.push_back(terminals[i]);
    }
    node* rule;
    node* temp;
    bool change = true;
    int index;
    while(change){
        change = false;
        for(int i = 0; i < grammar.size();++i){
        rule = grammar[i];
        temp = rule->next;
            while(temp != NULL){
                keptRule = grabRule(temp);
                for(int j = 0; j < keptRule.size();++j){
                    if(!isIn(generating,keptRule[j])){
                        index = findGrammarSpot(grammar,keptRule[j]);
                        if(!checkEpsilon(grammar[index])){
                            keptRule.clear();
                        }
                    }
                }
                if(keptRule.size() > 0){
                    if(!isIn(generating,grammar[i]->term)){
                    generating.push_back(grammar[i]->term);
                    change = true;
                    }
                    if(!checkVectors(ruleSet[grammar[i]->term],keptRule)){
                    ruleSet[grammar[i]->term].push_back(keptRule);
                    change = true;
                    }
                }
                keptRule.clear();

            }
        }
    }
}


map<string,vector<vector<string>>> removeAllUseless(vector<node*> grammar, vector<string> terminals,vector<pair<string,vector<string>>>ordered_rules,vector<string>& reachable,int task){
    map<string,vector<vector<string>>> ruleSet = map<string,vector<vector<string>>>();
    for(int i = 0; i < grammar.size(); ++i){
        ruleSet[grammar[i]->term] = vector<vector<string>>();
    }
    removeNonGenerating(ruleSet,terminals,grammar);
    removeUnreachable(ruleSet,grammar,terminals,ordered_rules,reachable,task);
    return ruleSet;

}


void printTerminalsAndNoneTerminals(vector<string> ordered_terminals, vector<string> non_terminals){
    vector<string> queue = vector<string>();
    for(int i = 0; i < ordered_terminals.size(); ++i){
        if(!isIn(non_terminals,ordered_terminals[i])){
            cout<<ordered_terminals[i]<<" ";
        }
        else{
            queue.push_back(ordered_terminals[i]);
        }
    }
    for(int j = 0; j < queue.size(); ++j){
        cout<<queue[j]<< " ";
    }
}


void checkIfPredictiveParser(vector<node*> grammar, vector<string> terminals,vector<pair<string,vector<string>>>ordered_rules,vector<string>& reachable, int task, int grammar_size){
    map<string,vector<vector<string>>> rules;
    vector<string> rule;
    vector<string> seen = vector<string>();
    terminals.push_back("#");
    rules = removeAllUseless(grammar,terminals,ordered_rules,reachable,task);
    int count = 0;
    for(int i = 0; i < ordered_rules.size(); ++i){
            if(isIn(reachable,ordered_rules[i].first)){
                rule = ordered_rules[i].second;
            if(checkVectors(rules[ordered_rules[i].first],rule)){
                ++count;
                }
            }
    }
    if(count != grammar_size){
        cout<<"NO";
        return;
    }
    else{
        cout<<"YES";
        return;
    }
}

void checkOrder(vector<string>& ordered_terminals,string target){
    if(!isIn(ordered_terminals,target)){
        ordered_terminals.push_back(target);
    }
}

void parse_idlist(vector<node*>& grammar, int index,pair<string,vector<string>>p,vector<pair<string,vector<string>>>& ordered_rules,vector<string>& ordered_terminals){
    Token t;
    node* head;
    node* temp;
    t = lex.peek(1);
    if(t.token_type != ID){
        syntax_error();
    }
    head = grammar[index];
    temp = head;
    if(temp->next != NULL){
        temp = addRule(temp);
    }
    while(lex.peek(1).token_type == ID){
    temp->next = new node();
    temp = temp->next;
    t = lex.GetToken();
    temp->term = t.lexeme;
    p.second.push_back(temp->term);
    checkOrder(ordered_terminals,t.lexeme);
    }
    ordered_rules.push_back(p);
    p.second.clear();

}

void parse_righthand(vector<node*>& grammar,string nonterm,pair<string,vector<string>>p,vector<pair<string,vector<string>>>& ordered_rules,vector<string>& ordered_terminals){
    Token t;
    int index;
    node* temp;
    node* head;
    index = findGrammarSpot(grammar,nonterm);
    t = lex.peek(1);
    if(t.token_type == ID){
        parse_idlist(grammar,index,p,ordered_rules,ordered_terminals);
    }
    //Add epsilon rule if no id list
    else if(t.token_type == STAR){
        head = grammar[index];
        temp = head;
        if(grammar[index]->next != NULL){
            temp = addRule(temp);
        }
        temp->next = new node();
        temp = temp->next;
        temp->term = "#";
        p.second.push_back("#");
        ordered_rules.push_back(p);
        p.second.clear();

    }
    else{
        syntax_error();
    }

}

void parse_rule(vector<node*>&grammar,vector<string>& non_terminals,vector<pair<string,vector<string>>>& ordered_rules,vector<string>& ordered_terminals){
    Token t;
    pair<string,vector<string>> p;
    node* temp;
    t = lex.peek(1);
    if(t.token_type != ID){
        syntax_error();
    }
    t = lex.GetToken();
    temp = new node();
    checkOrder(ordered_terminals,t.lexeme);
    temp->term = t.lexeme;
    p.first = temp->term;
    if(findGrammarSpot(grammar,temp->term) == -1){
    grammar.push_back(temp);
    //if we reach a new left side, and it is not already in non_terminals, then we add it
    if(!isIn(non_terminals,temp->term)){
        non_terminals.push_back(temp->term);
    }
    }
    if(lex.peek(1).token_type != ARROW){
        syntax_error();
    }
    lex.GetToken();
    parse_righthand(grammar,temp->term,p,ordered_rules,ordered_terminals);
    if(lex.peek(1).token_type != STAR){
        syntax_error();
    }
    lex.GetToken();
}

void parse_rulelist(vector<node*>& grammar,vector<string>& non_terminals,vector<string>& ordered_terminals,vector<pair<string,vector<string>>>& ordered_rules){
    Token t;
    parse_rule(grammar,non_terminals,ordered_rules,ordered_terminals);
    t = lex.peek(1);
    if(t.token_type != HASH){
        parse_rule(grammar,non_terminals,ordered_rules,ordered_terminals);
    }

}

void GetGrammar(vector<node*>& grammar,vector<string>& non_terminals,vector<string>& ordered_terminals,vector<pair<string,vector<string>>>& ordered_rules){
    while(lex.peek(1).token_type != HASH){
    parse_rulelist(grammar,non_terminals,ordered_terminals,ordered_rules);
    }
    lex.GetToken();
}

int main(int argc, char* argv[]){
    string input;
    string temp;
    int index;
    int count;

    vector<node*> grammar = vector<node*>();
    map<string,int> terminals = map<string,int>();
    vector<string> non_terminals = vector<string>();
    vector<string> ordered_terminals = vector<string>();
    vector<string> ordered_non_terminals = vector<string>();
    map<string,vector<string>> firstSets;
    map<string,vector<string>> followSets;
    vector<pair<string,vector<string>>>ordered_rules = vector<pair<string,vector<string>>>();
    vector<string> onlyTerms;
    vector<string> reachable;

    struct node* rule;
    int task;

    if(argc < 2){
        cout<<"Error: missing argument\n";
        return 1;
    }

    task = atoi(argv[1]);
    GetGrammar(grammar,non_terminals,ordered_terminals,ordered_rules);

    switch (task) {

        //Print terminals and non-terminals in order
        case 1: 
            printTerminalsAndNoneTerminals(ordered_terminals,non_terminals);
            break;

        //Remove useless symbols
        case 2: 
            removeNonTerminals(ordered_terminals,non_terminals,onlyTerms);
            removeAllUseless(grammar,onlyTerms,ordered_rules,reachable,task);
            break;

        //Calculate the first sets
        case 3: 
            terminals = extractTerminals(ordered_terminals,non_terminals,ordered_non_terminals);
            firstSets = calculateFirstSets(grammar,terminals,non_terminals,ordered_non_terminals);
            printFirstSets(firstSets,ordered_non_terminals);
            break;

        //Calculate the follow sets
        case 4: 
            terminals = extractTerminals(ordered_terminals,non_terminals,ordered_non_terminals);
            firstSets = calculateFirstSets(grammar,terminals,non_terminals,ordered_non_terminals);
            followSets = calculateFollowSets(firstSets,ordered_non_terminals,grammar,terminals);
            printFollowSets(followSets,ordered_non_terminals);
            break;

        //Check if parser is predictive
        case 5: 
            removeNonTerminals(ordered_terminals,non_terminals,onlyTerms);
            checkIfPredictiveParser(grammar,onlyTerms,ordered_rules,reachable,task,ordered_rules.size());
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
    
}
