#ifndef AST_HPP
#define AST_HPP


#include<string>
#include<map>
#include<vector>
#include<llvm/Support/Casting.h>
#include"APP.hpp"

/****************************************
 * AST
 * *************************************/

/**
 * Class Declaration
 */
class BaseAST;
class TranslationUnitAST;
class FunctionAST;
class PrototypeAST;
class FunctionStmtAST;
class VariableDeclAST;
class BinaryExprAST;
class NullExprAST;
class CallExprAST;
class JumpStmtAST;
class VariableAST;
class NumberAST;


/**
 * Kinds of AST
 */
enum AstID{
    BaseID,
    VariableDeclID,
    BinaryExprID,
    NullExprID,
    CallExprID,
    JumpStmtID,
    VariableID,
    NumberID
};

/**
 * Base class of AST
 */
class BaseAST{
    AstID ID;

    public:
        BaseAST(AstID id): ID(id){}
        virtual ~BaseAST(){}
        AstID getValueID() const {return ID;}
};

/**
 * AST that represents source code
 */
class TranslationUnitAST{
    std::vector<PrototypeAST*> Prototypes;
    std::vector<FunctionAST*> Functions;

    public:
        TranslationUnitAST(){}
        ~TranslationUnitAST();
        bool addPrototype(PrototypeAST *proto);
        bool addFunction(FunctionAST *func));
        bool empty();
        PrototypeAST *getPrototype(int i){
            if (i < Prototypes.size()){
                return Prototypes.at(i);
            }else{
                return NULL;
            }
        }
        FunctionAST *getFunction(int i){
            if (i < prototypes.size()){
                return Function.at(i);
            }else{
                return NULL;
            }
        }
};


/**
 * AST that represents declaration of function
 */
class PrototypeAST{
    std::string Name;
    std::vector<std::string> Params;

    public:
        PrototypeAST(const std::string &name, const std::vector<std::string> &params)
            : Name(name), Params(params){}
        std::string getName(){return Name;}
        std::string getParamName(int i){
            if (i < Params.size())
                return Params.at(i);
            return NULL;
        }
        int getParamNum(){
            return Params.size();
        }
};

/**
 * AST that represents definition of function
 */
class FunctionAST{
    PrototypeAST *Proto;
    FunctionStmtAST *Body;
    public:
        FunctionAST(PrototypeAST *proto, FunctionStmtAST *body): Proto(proto), Body(body){}
        ~FunctionAST();
        std::string getName(){
            return Proto->getName();
        }
        PrototypeAST *getPrototype(){
            return Proto;
        }
        FunctionStmtAST *getBody(){
            return Body;
        }
};

/**
 * AST that represents declaration of variable
 */
class VariableDeclAST: public BaseAST {
    public:
        typedef enum{
            param,
            local
        }DeclType;

    private:
        std::string Name;
        DeclType Type;
    public:
        VariableDeclAST(const std::string &name): BaseAST(VariableDeclID), Name(name){}
        static inline bool classof(VariableDeclAST const*){return true;}
        static inline bool classof(BaseAST const* base){
            return base->getValueID() == VariableDeclID;
        }
        ~VariableDeclAST(){}
        bool setDeclType(DeclType type){Type = type; return true;};
        std::string getName(){return Name;}
        DeclType getType(){return Type;}
};

/**
 * AST that represents binary expression
 */
class BinaryExprAST : public BaseAST{
    std::string Op;
    BaseAST *LHS, *RHS;
    public:
    BinaryExprAST(std::string op, BaseAST *lhs, BaseAST *rhs)
        : BaseAST(BinaryExprID), Op(op), LHS(lhs), RHS(rhs){
        }
    ~BinaryExprAST(){SAFE_DELETE(LHS); SAFE_DELETE(RHS);}
    static inline bool classof(BinaryExprAST const*){return true;}
    static inline bool classof(BaseAST const* base){
        return base->getValueID() == BinaryExprID;
    }
    std::string getOp(){return Op;}
    BaseAST *getLHS(){return LHS;}
    BaseAST *getRHS(){return RHS;}
};

/**
 * AST that represents ";"
 */
class NullExprAST : BaseAST{
    public:
        NullExprAST() : BaseAST(NullExprID){}
        static inline bool classof(NullExprAST const*){return true;}
        static inline bool classof((BaseAST const* base){
                return base->getValueID == NullExprID;        
        }
};

/**
 * AST that represents function call
 */
class CallExprAST : public BaseAST{
    std::string Callee;
    std::vector<BaseAST*> Args;

    public:
    CallExprAST(const std::string &callee, std::vector<BaseAST*> &args)
        : BaseAST(CallExprID), Callee(callee), Args(args){}
        ~CallExprAST();
        std::string getCallee(){return Callee;}
        BaseAST *getArgs(int i){
            if (i < Args.size()){
                return Args.at(i)
            }else{
                return NULL;
            }
        }
        static inline bool classof(CallExprAST const*){return true;}
        static inline bool classof(BaseAST const* base){
            return base->getValueID == CallExprID;
        }
};

/**
 * AST that represents jump (return)
 */
class JumpStmtAST : public BaseAST{
    BaseAST *Expr;
    public:
        JumpStmtAST(BaseAST *expr) : BaseAST(JumpStmtID), Expr(expr){
        }
        ~JumpStmtAST(){SAFE_DELETE(Expr);}
        BaseAST *getExpr(){return Expr;}
        static inline bool classof(JumpStmtAST const*){return true;}
        static inline bool classof(BaseAST const* base){
            return base->getValueID() == JumpStmtID;
        }
};

/**
 * AST that represents variable refenrence
 */
class VariableAST : public BaseAST{
    std::string Name;
    public:
        VariableAST(const std::string &name) : BaseAST(VariableID), Name(name){}
        ~VariableAST(){}
        static inline bool classof(VariableAST const*){return true;}
        static inline bool classof(BaseAST const* base){
            return base->getValueID() == VariableID;
        }
        std::string getName(){return Name;}
};

/**
 * AST that represents (integer) number
 */
class NumberAST : public BaseAST{
    int Val;
    public:
        NumberAST(int val) : BaseAST(NumberID), Val(val){};
        ~NumberAST(){}
        int getNumberValue(){return Val;}
        static inline bool classof(NumberAST const*){return true;}
        static inline bool classof(BaseAST const* base){
            return base->getValueID == NumberID;
        }
};

#endif
