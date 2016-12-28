#include "parser.hpp"


/**
 * Constructor
 */
Parser::Parser(std::string filename){
    Tokens=LexicalAnalysis(filename);
}


/**
 * Do parsing
 * @return success: true fail: false
 */
bool Parser::doParser(){
    if (!Tokens){
        fprint(stderr, "error ar lexer\n");
        return false;
    }else{
        return visitTranslationUnit();
    }
}

/**
 * Get AST
 * @return Reference to TranslationUnit
 */
TranslationUnitAST &Parser::getAST(){
    if (TU){
        return *TU;
    }else{
        return *(new TranslationUnitAST());
    }
}

/**
 * Method of parsing for TranslationUnit
 * @return success: true fail: false
 */
bool Parser::visitTranslationUnit(){
    TU = new TranslationUnitAST();
    std::vector<std::string> param_list;
    param_list.push_back("i");
    TU->addPrototype(new PrototypeAST("printnum", param_list));
    PrototypeTable["printnum"] = 1;

    //ExternalDecl
    while (true){
        if (!visitExternalDeclaration(TU)){
            SAFE_DELETE(TU);
            return false;
        }
        if (Tokens->getCurType() == TOK_EOF)
            break;
    }
    return true;
}

/**
 * Class of parsing for ExternalDeclaration
 * Add parsed PrototypeAST and FunctionAST to TranslationUnit
 * @param TranslationUnitAST
 * @return true
 */
bool Parser::visitExternalDeclaration(TranslationUnitAST *tunit){
    //FunctionDeclaration
    PrototypeAST *proto = visitFunctionDeclaration();
    if (proto){
        tunit->addPrototype(proto);
        return true;
    }

    //FunctionDefinition
    FunctionAST *func_def = visitFunctionDefinition();
    if (func_def){
        tunit->addFunction(func_def);
        return true;
    }

    return false;
}

/**
 * Parsing method for FunctionDeclaration
 * @return success: PrototypeAST fail: NULL
 */
PrototypeAST *Parser::visitFunctionDeclaration(){
    int bkup = Tokens->getCurIndex();
    PrototypeAST *proto = visitPrototype();
    if (!proto){
        return NULL;
    }

    //prototype
    if (Tokens->getCurString() == ";"){
        if (PrototypeTable.find(proto->getName()) != PrototypeTable.end() ||
                (FunctionTable.find(proto->getName()) != FunctionTable.end() && 
                 FunctionTable[proto->getName()] != proto->getParamNum())){
            fprint(stderr, "Function : %s is redefined", proto->getName().c_str());
            SAFE_DELETE(proto);
            return NULL;
        }
        PrototypeTable[proto->getName()] = proto->getParamNum();
        Tokens->getNextToken();
        return proto;
    }else{
        SAFE_DELETE(proto);
        Tokens->applyTokenIndex(bkup);
        return NULL;
    }
}

/**
 * Parsing method for FunctionDefinition
 * @return success: FunctionAST fail: NULL
 */
FunctionAST *Parser::visitFunctionDefinition(){
    int bkup = Tokens->getCurIndex();

    PrototypeAST *proto = visitPrototype();
    if (!proto){
        return NULL;
    }else if ((PrototypeTable.find(proto->getName()) != PrototypeTable.end() &&
                PrototypeTable[proto->getName()] != proto->getParamNum()) ||
            FunctionTable.find(proto->getName() != FunctionTable.end())){
        fprint(stderr, "Function : %s is redefined", proto->getName().c_str());
        SAFE_DELETE(proto);
        return NULL;
    }

    VariableTable.clear();
    FunctionStmtAST *func_stmt = visitFunctionStatement(proto);
    if (func_stmt){
        FunctionTable[proto->getName()] = proto->getParamNum();
        return new FunctionAST(proto, func_stmt);
    }else{
        SAFE_DELETE(proto);
        Tokens->applyTokenIndex(bkup);
        return NULL;
    }
}

/**
 * Parsing method for Prototype
 * @return success: PrototypeAST fail: NULL
 */
PrototypeAST *Parser::visitPrototype(){
    std::string func_name;

    int bkup = Tokens->getCurIndex();

    //type_specifier
    if (Tokens->getCurType() == TOK_INT){
        Tokens->getNextToken();
    }else{
        return NULL;
    }

    //IDENTIFIER
    if (Tokens->getCurType() == TOK_IDENTIFIER){
        func_name = Tokens->getCurString();
        Tokens->getNextToken();
    }else{
        Tokens->ungetToken(1);
        return NULL;
    }

    //'('
    if (Tokens->getCurString() == "("){
        Tokens->getNextToken();
    }else{
        Tokens->ungetToken(2);
        return NULL;
    }

    //parameter_list
    std::vector<std::string> param_list;
    bool is_first_param = true;
    while (true){
        //','
        if (!is_first_param && Tokens->getCurType() == TOK_SYMBOL && Tokens->getCurString() == ","){
            Tokens->getNextToken();
        }
        if (Tokens->getCurType() == TOK_INT){
            Tokens->getNextToken();
        }else{
            break;
        }

        if (Tokens->getCurType() == TOK_IDENTIFIER){
            if (std::find(param_list.begin(), param_list.end(), Tokens->getCurString()) != param_list.end()){
                Tokens->applyTokenIndex(bkup);
                return NULL;
            }
            param_list.push_back(Tokens->getCurString());
            Tokens->getNextToken();
        }else{
            Tokens->applyTokenIndex(bkup);
            return NULL;
        }
        is_first_param = false;
    }

    //')'
    if (Tokens->getCurString() == ")"){
        Tokens->getNextToken();
        return new PrototypeAST(func_name, param_list);
    }else{
        Tokens->applyTokenIndex(bkup);
        return NULL;
    }

}

/**
 * Parsing method for FunctionStatement
 * @param Instance of Prototype class which has function name and arguments
 * @return success: FunctionStmtAST fail: NULL
 */
FunctionStmtAST *Parser::visitFunctionStatement(PrototypeAST *proto){
    int bkup = Tokens->getCurIndex();

    //{
    if (Tokens->getCurString() == "{"){
        Tokens->getNextToken();
    }else{
        return NULL;
    }

    //Create FunctionStatement
    FunctionStmtAST *func_stmt = new FunctionStmtAST();

    //Add parameter to FunctionStatement
    for (int i=0; i<proto->getParamNum(); i++){
        VariableDeclAST *vdecl = new VariableDeclAST(proto->getParamName(i));
        vdecl->setDeclType(VariableDeclAST::param);
        func_stmt->addVariableDeclaration(vdecl);
        VariableTable.push_back(vdecl->getName());
    }

    VariableDeclAST *var_decl;
    BaseAST *stmt;
    BaseAST *last_stmt;

    //{statement_list}
    if (stmt = visitStatement()){
        while (stmt){
            last_stmt = stmt;
            func_stmt->addStatement(stmt);
            stmt = visitStatement();
        }

    //variable_declaration_list
    }else if (var_decl = visitVariableDeclaration()){
        while (var_decl){
            var_decl->setDeclType(VariableDeclAST::local);
            if (std::find(VariableTable.begin(), VariableTable.end(), var_decl->getName()) != VariableTable.end()){
                SAFE_DELETE(var_decl);
                SAFE_DELETE(func_stmt);
                return NULL;
            }
            func_stmt->addVariableDeclaration(var_decl);
            VariableTable.push_back(var_decl->getName());
            //Parse VariableDeclaration
            var_decl = visitVariableDeclaration();
        }

        if (stmt = visitStatement()){
            while (stmt){
                last_stmt = stmt;
                func_stmt->addStatement(stmt);
                stmt = visitStatement();
            }
        }


    //other
    }else{
        SAFE_DELETE(func_stmt);
        Tokens->applyTokenIndex(bkup);
        return NULL;
    }

    //check if last statement is jump_statement
    if (!last_stmt || !llvm::isa<JumpStmtAST>(last_stmt)){
        SAFE_DELETE(func_stmt);
        Tokens->applyTokenIndex(bkup);
        return NULL;
    }

    //}
    if (Tokens->getCurString() == "}"){
        Tokens->getNextToken();
        return func_stmt;
    }else{
        SAFE_DELETE(func_stmt);
        return NULL;
    }
}


/**
 * Parsing method for VariableDeclaration
 * @return success: VariableDeclAST fail: NULL
 */
VariableDeclAST *Parser::visitVariableDeclaration(){
    std::string name;

    //INT
    if (Tokens->getCurType() == TOK_INT){
        Tokens->getNextToken();
    }else{
        return NULL;
    }

    //IDENTIFIER
    if (Tokens->getCurType() == TOK_IDENTIFIER){
        name = Tokens->getCurString();
        Tokens->getNextToken();
    }else{
        Tokens->ungetToken(1);
        return NULL;
    }

    //';'
    if (Tokens->getCurString() == ";"){
        Tokens->getNextToken();
        return new VariableDeclAST(name);
    }else{
        Tokens->ungetToken(2);
        return NULL;
    }
}


