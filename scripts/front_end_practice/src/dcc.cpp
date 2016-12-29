#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetSelect.h"
#include "lexer.hpp"
#include "AST.hpp"
#include "parser.hpp"
#include "codegen.hpp"


/**
 * Option parser
 */
class OptionParser{
    private:
        std::string InputFileName;
        std::string OutputFileName;
        std::string LinkFileName;
        bool WithJit;
        int Argc;
        char **Argv;

    public:
        OptionParser(int argc, char **argv): Argc(argc), Argv(argv), WithJit(false){}
        void printHelp();
        std::string getInputFileName(){return InputFileName;}
        std::string getOutputFileName(){return OutputFileName;}
        std::string getLinKFileName(){return LinkFileName;}
        bool getWithJit(){return WithJit;}
        bool parseOption();

};

/**
 * Help
 */
void OptionParser::printHelp(){
    fprintf(stdout, "Compiler for DummyC...\n");
    fprintf(stdout, "LLVM 3.5 for MacOS/X")
}

/**
 * Parse Option
 */
bool OptionParser::parseOption(){
    if (Argc < 2){
        fprintf(stderr, "Arguments is not enough\n");
        return false;
    }

    for (int i=0; i<Argc; i++){
        if (Argv[i][0] == '-' && Argv[i][1] == 'o' && Argv[i][2] == '\0'){
            OutputFileName.assign(Argv[++i]);
        }else if (Argv[i][0] == '-' && Argv[i][1] == 'h' && Argv[i][2] == '\0'){
            printHelp();
            return false;
        }else if (Argv[i][0] == '-' && Argv[i][1] == 'l' && Argv[i][2] == '\0'){
            LinkFileName.assign(Argv[++i]);
        }else if (Argv[i][0] == '-' && Argv[i][1] == 'j' && Argv[i][2] == 'i' && Argv[i][3] == 't' && Argv[i][4] == '\0'){
            WithJit = true;
        }else if (Argv[i][0] == '-'){
            fprintf(stderr, "%s is unknown option\n", Argv[i]);
            return false;
        }else{
            InputFileName.assign(Argv[i]);
        }
    }

    //OutputFileName
    std::string ifn = InputFileName;
    int len = ifn.length();
    if (OutputFileName.empty() && (len > 2) && ifn[len-3] == '.' && ((ifn[len-2] == 'd' && ifn[len-1] == 'c'))){
        OutputFileName = std::string(ifn.begin(), ifn.end() - 3);
        OutputFileName += ".s";
    }else if (OutputFileName.empty()){
        OutputFileName = ifn;
        OutputFileName += ".s";
    }

    return true;
}

/**
 * main function
 */
int main(int argc, char **argv){
    llvm::InitializeNativeTarget();
    llvm::sys::PrintStackTraceOnErrorSignal();
    llvm::PrettyStackTraceProgram X(argc, argv)

    llvm::EnableDebugBuffering = true;

    OptionParser opt(argc, argv);
    if (!opt.parseOption()){
        exit(1);
    }

    if (opt.getInputFileName().length() == 0){
        fprintf(stderr, "InputFileName not exists\n");
        exit(1);
    }

    Parser *parser = new Parser(opt.getInputFileName());
    if (!parser->doParse()){
        fprintf(stderr, "Error at parser or lexer\n");
        SAFE_DELETE(parser);
        exit(1);
    }

    TranslationUnitAST &tunit = parser->getAST();
    if (tunit.empty()){
        fprintf(stderr, "TranslationUnit is empty\n");
        SAFE_DELETE(parser);
        exit(1);
    }

    CodeGene *codegen = new CodeGen();
    if (!codegen->doCodeGen(tunit, opt.getInputFileName(), opt.getLinkFileName(), opt.getWithJit())){
        fprintf(stderr, "Error at codegen\n");
        SAFE_DELETE(parser);
        SAFE_DELETE(codegen);
        exit(1);
    }

    llvm::Module &mod = codegen->getModule();
    if (mod.empty()){
        fprintf(stderr, "Module is empty\n");
        SAFE_DELETE(parser);
        SAFE_DELETE(codegen);
        exit(1);
    }

    llvm::PassManager pm;

    //SSA
    pm.add(llvm::createPromoteMemoryToRegisterPass());

    //Output
    std::string  error;
    llvm::raw_fd_ostream raw_stream(opt.getOutputFileName().c_str(), error);
    pm.add(createPrintModulePass(&raw_stream));
    pm.run(mod);
    raw_stream.close();

    //delete
    SAFE_DELETE(parser);
    SAFE_DELETE(codegen);

    return 0;

}


