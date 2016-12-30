#ifndef PARSER_HPP
#define PARSER_HPP

#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include "APP.hpp"
#include "AST.hpp"
#include "lexer.hpp"


/**
 * Class of parser and semantic analysis
 */
typedef class Parser{
    public:

    private:
        TokenStream *Tokens;
        TranslationUnitAST *TU;

        //Identifier table for semantic analysis
        std::vector<std::string> VariableTable;
        std::map<std::string, int> PrototypeTable;
        std::map<std::string, int> FunctionTable;

    protected:

    public:
        Parser(std::string filename);
        ~Parser(){SAFE_DELETE(TU); SAFE_DELETE(Tokens);}
        bool doParse();
        TranslationUnitAST &getAST();

    private:
        /**
         * Methods of parsing
         */
        bool visitTranslationUnit();
        bool visitExternalDeclaration(TranslationUnitAST *tunit);
        PrototypeAST *visitFunctionDeclaration();
        FunctionAST *visitFunctionDefinition();
        PrototypeAST *visitPrototype();
        FunctionStmtAST *visitFunctionStatement(PrototypeAST *proto);
        VariableDeclAST *visitVariableDeclaration();
        BaseAST *visitStatement();
        BaseAST *visitExpressionStatement();
        BaseAST *visitJumpStatement();
        BaseAST *visitAssignmentExpression();
        BaseAST *visitAdditiveExpression(BaseAST *lhs);
        BaseAST *visitMultiplicativeExpression(BaseAST *lhs);
        BaseAST *visitPostfixExpression();
        BaseAST *visitPrimaryExpression();

    protected:

}Parser;

#endif
