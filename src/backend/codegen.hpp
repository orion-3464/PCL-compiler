#ifndef __CODEGEN_HPP__
#define __CODEGEN_HPP__

// Modeled after: https://github.com/JuliaLang/julia/blob/master/src/llvm-codegen-shared.h

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/APFloat.h>

#include <map>
#include <memory>
#include <vector>

struct CompilerOptions;

namespace CodeGen {
    extern std::unique_ptr<llvm::LLVMContext> TheContext;
    extern std::unique_ptr<llvm::Module> TheModule;
    extern std::unique_ptr<llvm::IRBuilder<>> Builder;

    extern std::unique_ptr<llvm::FunctionPassManager> TheFPM;
    extern std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
    extern std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
    extern std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
    extern std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
    extern std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC;
    extern std::unique_ptr<llvm::StandardInstrumentations> TheSI;

    extern llvm::Type *i8;
    extern llvm::Type *i32;
    extern llvm::Type *r64;
    extern llvm::PointerType* ptr;

    extern llvm::Function *TheWriteInteger;
    extern llvm::Function *TheWriteBoolean;
    extern llvm::Function *TheWriteChar;
    extern llvm::Function *TheWriteReal;
    extern llvm::Function *TheWriteString;
    extern llvm::Function *TheReadInteger;
    extern llvm::Function *TheReadBoolean;
    extern llvm::Function *TheReadChar;
    extern llvm::Function *TheReadReal;
    extern llvm::Function *TheReadString;
    extern llvm::Function *TheAbs;
    extern llvm::Function *TheFAbs;
    extern llvm::Function *TheSqrt;
    extern llvm::Function *TheSin;
    extern llvm::Function *TheCos;
    extern llvm::Function *TheTan;
    extern llvm::Function *TheArcTan;
    extern llvm::Function *TheExp;
    extern llvm::Function *TheLn;
    extern llvm::Function *ThePi;
    extern llvm::Function *TheTrunc;
    extern llvm::Function *TheRound;
    extern llvm::Function *TheOrd;
    extern llvm::Function *TheChr;
    
    llvm::ConstantInt* c32(int n);
    llvm::ConstantFP* f64(double d);

    void declareLibFunctions();
    void initialize(const std::string& module_name);
    void generateOutput(const CompilerOptions& co);
    void optimize();

    class BackendSymbolTable {
    public:
        BackendSymbolTable() = default;

        void enterScope();
        void exitScope();
        void insert(const std::string& name, llvm::AllocaInst* inst);
        llvm::AllocaInst* lookup(const std::string& name) const;
    private:
        // Kaleidoscope suggests only map.
        // We add a stack on top to handle scoping
        std::vector<std::map<std::string, llvm::AllocaInst*>> NamedValues;
    };

    extern BackendSymbolTable backendST;
}

#endif
