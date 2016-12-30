#include "codegen.hpp"

/**
 * Constructor
 */
CodeGen::CodeGen(){
    Builder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    Mod = NULL;
}

/**
 * Destructor
 */
CodeGen::~CodeGen(){
    SAFE_DELETE(Builder);
    SAFE_DELETE(Mod);
}

/**
 * Implement code generation
 * @param TranslationUnitAST Module name
 */
bool CodeGen::doCodeGen(TranslationUnitAST &tunit, std::string name,
        std::string link_file, bool with_jit=false){

    if (!generateTranslationUnit(tunit, name)){
        return false;
    }

    //Link module if linkfile is indicated
    if (!link_file.empty() && !linkModule(Mod, link_file)){
        return false;
    }

    //Do JIT if JIT flag is set true
    if (with_jit){
        llvm::ExecutionEngine *EE = llvm::EngineBuilder(Mod).create();
        llvm::EngineBuilder(Mod).create();
        llvm::Function *F;
        if (!(F = Mod->getFunction("main"))){
            return false;
        }

        int (*fp)() = (int (*)())EE->getPointerToFunction(F);
        fprintf(stderr, "%d\n", fp());
    }

    return true;
}

/**
 * Get Module
 */
llvm::Module &CodeGen::getModule(){
    if (Mod){
        return *Mod;
    }else{
        return *(new llvm::Module("null", llvm::getGlobalContext()));
    }
}

/**
 * Method of Module generarion
 */
bool CodeGen::generateTranslationUnit(TranslationUnitAST &tunit, std::string name){
    Mod = new llvm::Module(name, llvm::getGlobalContext());

    //Function declaration
    for (int i=0; ; i++){
        PrototypeAST *proto = tunit.getPrototype(i);
        if (!proto){
            break;
        }else if (!generatePrototype(proto, Mod)){
            SAFE_DELETE(Mod);
            return false;
        }
    }

    //Function definition
    for (int i=0; ; i++){
        FunctionAST *func = tunit.getFunction(i);
        if (!func){
            break;
        }else if (!generateFunctionDefinition(func, Mod)){
            SAFE_DELETE(Mod);
            return false;
        }
    }

    return true;
}

/**
 * Method of function definition
 * @param FunctionAST Module
 * @return Pointer of generated Function
 */
llvm::Function *CodeGen::generateFunctionDefinition(FunctionAST *func_ast, llvm::Module *mod){
    llvm::Function *func = generatePrototype(func_ast->getPrototype(), mod);
    if (!func){
        return NULL;
    }
    CurFunc = func;
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);
    Builder->SetInsertPoint(bblock);
    generateFunctionStatement(func_ast->getBody());

    return func;
}

/**
 * Method of function declaration
 */
llvm::Function *CodeGen::generatePrototype(PrototypeAST *proto, llvm::Module *mod){
    //Already declared?
    llvm::Function *func = mod->getFunction(proto->getName());
    if (func){
        if (func->arg_size() == proto->getParamNum() && func->empty()){
            return func;
        }else{
            fprintf(stderr, "error::function %s if redefined", proto->getName().c_str());
            return NULL;
        }
    }

    //Create arg_types
    std::vector<llvm::Type*> int_types(proto->getParamNum(), llvm::Type::getInt32Ty(llvm::getGlobalContext()));

    //Create func_type
    llvm::FunctionType *func_type = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(llvm::getGlobalContext()),
            int_types, false
            );

    //Create function
    func = llvm::Function::Create(func_type,
            llvm::Function::ExternalLinkage,
            proto->getName(),
            mod
            );

    //Set names
    llvm::Function::arg_iterator arg_iter = func->arg_begin();
    for (int i=0; i<proto->getParamNum(); i++){
        arg_iter->setName(proto->getParamName(i).append("_arg"));
        ++arg_iter;
    }

    return func;
}

/**
 * Method of generating function
 * Declare variable => Statement
 * @param FunctionStmtAST
 * @return Last generated pointer of Value
 */
llvm::Value *CodeGen::generateFunctionStatement(FunctionStmtAST *func_stmt){
    //insert variable decls
    VariableDeclAST *vdecl;
    llvm::Value *v = NULL;
    for (int i=0; ; i++){
        if (!func_stmt->getVariableDecl(i)){
            break;
        }

        //Create alloca
        vdecl = llvm::dyn_cast<VariableDeclAST>(func_stmt->getVariableDecl(i));
        v = generateVariableDeclaration(vdecl);
    }

    //insert expr statement
    BaseAST *stmt;
    for (int i=0; ; i++){
        stmt = func_stmt->getStatement(i);
        if (!stmt){
            break;
        }else if (!llvm::isa<NullExprAST>(stmt)){
            v = generateStatement(stmt);
        }
    }
    return v;
}

/**
 * Method of generating variable declaration
 */
llvm::Value *CodeGen::generateVariableDeclaration(VariableDeclAST *vdecl){
    //Create alloca
    llvm::AllocaInst *alloca = Builder->CreateAlloca(
            llvm::Type::getInt32Ty(llvm::getGlobalContext()),
            0,
            vdecl->getName()
            );

    //If args alloca
    if (vdecl->getType() == VariableDeclAST::param){
        //store args
        llvm::ValueSymbolTable &vs_table = CurFunc->getValueSymbolTable();
        Builder->CreateStore(vs_table.lookup(vdecl->getName().append("_arg")), alloca);
    }
    
    return alloca;
}

/**
 * Method of generating statement
 * @param JumpStmtAST
 */
llvm::Value *CodeGen::generateStatement(BaseAST *stmt){
    if (llvm::isa<BinaryExprAST>(stmt)){
        return generateBinaryExpression(llvm::dyn_cast<BinaryExprAST>(stmt));
    }else if (llvm::isa<CallExprAST>(stmt)){
        return generateCallExpression(llvm::dyn_cast<CallExprAST>(stmt));
    }else if (llvm::isa<JumpStmtAST>(stmt)){
        return generateJumpStatement(llvm::dyn_cast<JumpStmtAST>(stmt));
    }else{
        return NULL;
    }
}

/**
 * Binary Expression
 */
llvm::Value *CodeGen::generateBinaryExpression(BinaryExprAST *bin_expr){
    BaseAST *lhs = bin_expr->getLHS();
    BaseAST *rhs = bin_expr->getRHS();

    llvm::Value *lhs_v;
    llvm::Value *rhs_v;

    //assignment
    if (bin_expr->getOp() == "="){
        //lhs is variable
        VariableAST *lhs_var = llvm::dyn_cast<VariableAST>(lhs);
        llvm::ValueSymbolTable &vs_table = CurFunc->getValueSymbolTable();
        lhs_v = vs_table.lookup(lhs_var->getName());
    
    //Other operand
    }else{
        //lhs is
        //Binary?
        if (llvm::isa<BinaryExprAST>(lhs)){
            lhs_v = generateBinaryExpression(llvm::dyn_cast<BinaryExprAST>(lhs));

        //Variable?
        }else if (llvm::isa<VariableAST>(lhs)){
            lhs_v = generateVariable(llvm::dyn_cast<VariableAST>(lhs));

        //Number?
        }else if (llvm::isa<NumberAST>(lhs)){
            NumberAST *num = llvm::dyn_cast<NumberAST>(lhs);
            lhs_v = generateNumber(num->getNumberValue());
        }
    }

    //Create rhs value
    if (llvm::isa<BinaryExprAST>(rhs)){
        rhs_v = generateBinaryExpression(llvm::dyn_cast<BinaryExprAST>(rhs));

    //CallExpr
    }else if (llvm::isa<CallExprAST>(rhs)){
        rhs_v = generateCallExpression(llvm::dyn_cast<CallExprAST>(rhs));

    //Variable
    }else if (llvm::isa<VariableAST>(rhs)){
        rhs_v = generateVariable(llvm::dyn_cast<VariableAST>(rhs));

    //Number
    }else if (llvm::isa<NumberAST>(rhs)){
        NumberAST *num = llvm::dyn_cast<NumberAST>(rhs);
        rhs_v = generateNumber(num->getNumberValue());
    }

    if (bin_expr->getOp() == "="){
        //store
        return Builder->CreateStore(rhs_v, lhs_v);
    }else if (bin_expr->getOp() == "+"){
        return Builder->CreateAdd(lhs_v, rhs_v, "add_tmp");
    }else if (bin_expr->getOp() == "-"){
        return Builder->CreateSub(lhs_v, rhs_v, "sub_tmp");
    }else if (bin_expr->getOp() == "*"){
        return Builder->CreateMul(lhs_v, rhs_v, "mul_tmp");
    }else if (bin_expr->getOp() == "/"){
        return Builder->CreateSDiv(lhs_v, rhs_v, "div_tmp");
    }
}

/**
 * Method of generating Function Call
 */
llvm::Value *CodeGen::generateCallExpression(CallExprAST *call_expr){
    std::vector<llvm::Value*> arg_vec;
    BaseAST *arg;
    llvm::Value *arg_v;
    llvm::ValueSymbolTable &vs_table = CurFunc->getValueSymbolTable();
    for (int i=0; ; i++){
        if (!(arg = call_expr->getArgs(i))){
            break;
        }

        //isCall
        if (llvm::isa<CallExprAST>(arg)){
            arg_v = generateCallExpression(llvm::dyn_cast<CallExprAST>(arg));
        }

        //isBinaryExpr
        else if (llvm::isa<BinaryExprAST>(arg)){
            BinaryExprAST *bin_expr = llvm::dyn_cast<BinaryExprAST>(arg);

            arg_v = generateBinaryExpression(llvm::dyn_cast<BinaryExprAST>(arg));

            if (bin_expr->getOp() == "="){
                VariableAST *var = llvm::dyn_cast<VariableAST>(bin_expr->getLHS());
                arg_v = Builder->CreateLoad(vs_table.lookup(var->getName()), "arg_val");
            }
        }

        //isVar
        else if (llvm::isa<VariableAST>(arg)){
            arg_v = generateVariable(llvm::dyn_cast<VariableAST>(arg));
        }

        //isNumber
        else if (llvm::isa<NumberAST>(arg)){
            NumberAST *num = llvm::dyn_cast<NumberAST>(arg);
            arg_v = generateNumber(num->getNumberValue());
        }

        arg_vec.push_back(arg_v);

    }

    return Builder->CreateCall(Mod->getFunction(call_expr->getCallee()),
            arg_vec, "call_tmp");
}

/**
 * Generating Jump
 */
llvm::Value *CodeGen::generateJumpStatement(JumpStmtAST *jump_stmt){
    BaseAST *expr = jump_stmt->getExpr();
    llvm::Value *ret_v;
    if (llvm::isa<BinaryExprAST>(expr)){
        ret_v = generateBinaryExpression(llvm::dyn_cast<BinaryExprAST>(expr));
    }else if (llvm::isa<VariableAST>(expr)){
        VariableAST *var = llvm::dyn_cast<VariableAST>(expr);
        ret_v = generateVariable(var);
    }else if (llvm::isa<NumberAST>(expr)){
        NumberAST *num = llvm::dyn_cast<NumberAST>(expr);
        ret_v = generateNumber(num->getNumberValue());
    }

    Builder->CreateRet(ret_v);
}

/**
 * Generate Reference of Variable
 */
llvm::Value *CodeGen::generateVariable(VariableAST *var){
    llvm::ValueSymbolTable &vs_table = CurFunc->getValueSymbolTable();
    return Builder->CreateLoad(vs_table.lookup(var->getName()), "var_tmp");
}

llvm::Value *CodeGen::generateNumber(int value){
    return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(llvm::getGlobalContext()),
            value
            );
}

bool CodeGen::linkModule(llvm::Module *dest, std::string file_name){
    llvm::SMDiagnostic err;
    llvm::Module *link_mod = llvm::ParseIRFile(file_name, err, llvm::getGlobalContext());
    if (!link_mod){
        return false;
    }

    std::string err_msg;
    if (llvm::Linker::LinkModules(dest, link_mod, llvm::Linker::DestroySource, &err_msg)){
        return false;
    }

    SAFE_DELETE(link_mod);

    return true;
}
