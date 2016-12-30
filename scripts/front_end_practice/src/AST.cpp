#include "AST.hpp"

/**
 * Destructor
 */
TranslationUnitAST::~TranslationUnitAST(){
    for (int i=0; i<Prototypes.size(); i++){
        SAFE_DELETE(Prototypes[i]);
    }
    Prototypes.clear();

    for (int i=0; i<Functions.size(); i++){
        SAFE_DELETE(Functions[i]);
    }
    Functions.clear();
}

/**
 * Method for PrototypeAST
 */
bool TranslationUnitAST::addPrototype(PrototypeAST *proto){
    Prototypes.push_back(proto);
    return true;
}

/**
 * Method for FunctionAST
 */
bool TranslationUnitAST::addFunction(FunctionAST *func){
    Functions.push_back(func);
    return true;
}

bool TranslationUnitAST::empty(){
    if (Prototypes.size() == 0 && Functions.size() == 0){
        return true;
    }else{
        return false;
    }
}

/**
 * Destructor
 */
FunctionAST::~FunctionAST(){
    SAFE_DELETE(Proto);
    SAFE_DELETE(Body);
}

FunctionStmtAST::~FunctionStmtAST(){
    for (int i=0; i<VariableDecls.size(); i++){
        SAFE_DELETE(VariableDecls[i]);
    }
    VariableDecls.clear();

    for (int i=0; i<StmtLists.size(); i++){
        SAFE_DELETE(StmtLists[i]);
    }
    StmtLists.clear();
}


/**
 * Method for VariableDeclAST
 */
bool FunctionStmtAST::addVariableDeclaration(VariableDeclAST *vdecl){
    VariableDecls.push_back(vdecl);
    return true;
}

/**
 * Destructor
 */
CallExprAST::~CallExprAST(){
    for (int i=0; i<Args.size(); i++){
        SAFE_DELETE(Args[i]);
    }
}
