#include "backend/codegen.hpp"
#include "ast/ast.hpp"

#include <filesystem>
#include <iostream>
#include <cstdlib>

#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Scalar/SROA.h>
#include <llvm/Transforms/Scalar/SCCP.h>
#include <llvm/Transforms/Scalar/LoopUnrollPass.h>
#include <llvm/Transforms/Scalar/ADCE.h>

namespace CodeGen {
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    
    std::unique_ptr<llvm::FunctionPassManager> TheFPM;
    std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
    std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
    std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
    std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
    std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC;
    std::unique_ptr<llvm::StandardInstrumentations> TheSI;

    BackendSymbolTable backendST;
    
    llvm::Type* i8 = nullptr;
    llvm::Type* i32 = nullptr;
    llvm::Type* r64 = nullptr;
    llvm::PointerType* ptr = nullptr;

    llvm::Function *TheWriteInteger = nullptr;
    llvm::Function *TheWriteBoolean = nullptr;
    llvm::Function *TheWriteChar = nullptr;
    llvm::Function *TheWriteReal = nullptr;
    llvm::Function *TheWriteString = nullptr;
    llvm::Function *TheReadInteger = nullptr;
    llvm::Function *TheReadBoolean = nullptr;
    llvm::Function *TheReadChar = nullptr;
    llvm::Function *TheReadReal = nullptr;
    llvm::Function *TheReadString = nullptr;
    llvm::Function *TheAbs = nullptr;
    llvm::Function *TheFAbs = nullptr;
    llvm::Function *TheSqrt = nullptr;
    llvm::Function *TheSin = nullptr;
    llvm::Function *TheCos = nullptr;
    llvm::Function *TheTan = nullptr;
    llvm::Function *TheArcTan = nullptr;
    llvm::Function *TheExp = nullptr;
    llvm::Function *TheLn = nullptr;
    llvm::Function *ThePi = nullptr;
    llvm::Function *TheTrunc = nullptr;
    llvm::Function *TheRound = nullptr;
    llvm::Function *TheOrd = nullptr;
    llvm::Function *TheChr = nullptr;
    
    llvm::ConstantInt* c32(int n) {
        return llvm::ConstantInt::get(*TheContext, llvm::APInt(32, n, true));
    }
    
    llvm::ConstantFP* f64(double d) {
        return llvm::ConstantFP::get(*TheContext, llvm::APFloat(d));
    }

    void declareLibFunctions() {
        // IO
        llvm::FunctionType *procIntArgType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), {i32}, false);
        llvm::FunctionType *procByteArgType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), {i8}, false);
        llvm::FunctionType *procRealArgType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), {r64}, false);
        llvm::FunctionType *procStringType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), {ptr}, false);

        TheWriteInteger =
            llvm::Function::Create(procIntArgType, llvm::Function::ExternalLinkage,
                                   "writeInteger", TheModule.get());
        TheWriteBoolean =
            llvm::Function::Create(procByteArgType, llvm::Function::ExternalLinkage,
                                   "writeBoolean", TheModule.get());
        TheWriteChar =
            llvm::Function::Create(procByteArgType, llvm::Function::ExternalLinkage,
                                   "writeChar", TheModule.get());
        TheWriteReal =
            llvm::Function::Create(procRealArgType, llvm::Function::ExternalLinkage,
                                   "writeReal", TheModule.get());
        TheWriteString =
            llvm::Function::Create(procStringType, llvm::Function::ExternalLinkage,
                                   "writeString", TheModule.get());


        llvm::FunctionType *funcIntResType =
            llvm::FunctionType::get(i32, {}, false);
        llvm::FunctionType *funcByteResType =
            llvm::FunctionType::get(i8, {}, false);
        llvm::FunctionType *funcRealResType =
            llvm::FunctionType::get(r64, {}, false);
        std::vector<llvm::Type*> IntCharArrayRef = {i32, ptr};
        llvm::FunctionType *procIntStringRefType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext),
                                   IntCharArrayRef, false);

        TheReadInteger =
            llvm::Function::Create(funcIntResType, llvm::Function::ExternalLinkage,
                                   "readInteger", TheModule.get());
        TheReadBoolean =
            llvm::Function::Create(funcByteResType, llvm::Function::ExternalLinkage,
                                   "readBoolean", TheModule.get());
        TheReadChar =
            llvm::Function::Create(funcByteResType, llvm::Function::ExternalLinkage,
                                   "readChar", TheModule.get());
        TheReadReal =
            llvm::Function::Create(funcRealResType, llvm::Function::ExternalLinkage,
                                   "readReal", TheModule.get());
        TheReadString =
            llvm::Function::Create(procIntStringRefType, llvm::Function::ExternalLinkage,
                                   "readString", TheModule.get());

        // Math
        llvm::FunctionType *funcIntResIntArgType =
            llvm::FunctionType::get(i32, {i32}, false);
        llvm::FunctionType *funcRealResRealArg =
            llvm::FunctionType::get(r64, {r64}, false);
        llvm::FunctionType *funcIntResRealArgType =
            llvm::FunctionType::get(i32, {r64}, false);

        TheAbs =
            llvm::Function::Create(funcIntResIntArgType, llvm::Function::ExternalLinkage,
                                   "abs", TheModule.get());
        TheFAbs =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "fabs", TheModule.get());
       TheSqrt =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "sqrt", TheModule.get());
       TheSin =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "sin", TheModule.get());
       TheCos =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "cos", TheModule.get());
       TheTan =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "tan", TheModule.get());
       TheArcTan =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "arctan", TheModule.get());
       TheExp =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "exp", TheModule.get());
       TheLn =
            llvm::Function::Create(funcRealResRealArg, llvm::Function::ExternalLinkage,
                                   "ln", TheModule.get());
       ThePi =
           llvm::Function::Create(funcRealResType, llvm::Function::ExternalLinkage,
                                  "pi", TheModule.get());
       TheTrunc =
           llvm::Function::Create(funcIntResRealArgType, llvm::Function::ExternalLinkage,
                                  "trunc", TheModule.get());
       TheRound =
           llvm::Function::Create(funcIntResRealArgType, llvm::Function::ExternalLinkage,
                                  "round", TheModule.get());
       // Conversion
       llvm::FunctionType *funcIntResByteArgType =
            llvm::FunctionType::get(i32, {i8}, false);
       llvm::FunctionType *funcByteResIntArgType =
           llvm::FunctionType::get(i8, {i32}, false);

       TheOrd =
           llvm::Function::Create(funcIntResByteArgType, llvm::Function::ExternalLinkage,
                                  "ord", TheModule.get());

       TheChr =
           llvm::Function::Create(funcByteResIntArgType, llvm::Function::ExternalLinkage,
                                  "chr", TheModule.get());
    }
    
    void initialize(const std::string& module_name) {
        TheContext = std::make_unique<llvm::LLVMContext>();
        TheModule = std::make_unique<llvm::Module>(module_name, *TheContext);
        Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

        TheFPM = std::make_unique<llvm::FunctionPassManager>();
        TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
        TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
        TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
        TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
        ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
        TheSI = std::make_unique<llvm::StandardInstrumentations>(*TheContext, true);
        TheSI->registerCallbacks(*ThePIC, TheMAM.get());
        
        i8  = llvm::IntegerType::get(*TheContext, 8);
        i32 = llvm::IntegerType::get(*TheContext, 32);
        r64 = llvm::Type::getDoubleTy(*TheContext);
        ptr = llvm::PointerType::getUnqual(*TheContext);

        declareLibFunctions();
    }
    
    static std::unique_ptr<llvm::TargetMachine> setupTargetMachine(const std::string& filename) {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
        
        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        TheModule->setTargetTriple(TargetTriple);

        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
        if (!Target) {
            llvm::errs() << Error;
            std::exit(1);
        }

        auto targetMachine = std::unique_ptr<llvm::TargetMachine>(
            Target->createTargetMachine(TargetTriple, "generic", "", llvm::TargetOptions(), std::optional<llvm::Reloc::Model>()));
            
        TheModule->setDataLayout(targetMachine->createDataLayout());
        TheModule->setSourceFileName(filename);
        
        return targetMachine;
    }

    static void emitIR(const CompilerOptions& co) {
        if (co.imm_code_stdout) {
            TheModule->print(llvm::outs(), nullptr);
            return;
        }
        std::filesystem::path p = co.filename;
        p.replace_extension(".imm");
        std::error_code EC;
        llvm::raw_fd_ostream dest(p.string(), EC, llvm::sys::fs::OF_None);
        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message() << "\n";
            std::exit(1);
        }
        TheModule->print(dest, nullptr);
    }

    static void emitAssembly(const CompilerOptions& co, llvm::TargetMachine* TargetMachine) {
        std::filesystem::path p = co.filename;
        p.replace_extension(".asm");
        std::error_code EC;
        std::unique_ptr<llvm::raw_fd_ostream> file_stream;
        
        if (!co.final_code_stdout) {
            file_stream = std::make_unique<llvm::raw_fd_ostream>(p.string(), EC, llvm::sys::fs::OF_None);
            if (EC) {
                llvm::errs() << "Could not open file: " << EC.message() << "\n";
                std::exit(1);
            }
        }
        llvm::raw_pwrite_stream& dest = co.final_code_stdout ? (llvm::raw_pwrite_stream&)llvm::outs() : *file_stream;
        llvm::legacy::PassManager pass;
        if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
            llvm::errs() << "TargetMachine can't emit a file of this type\n";
            std::exit(1);
        }
        pass.run(*TheModule);
    }

    static void linkExecutable(const CompilerOptions& co) {
        std::filesystem::path p = co.filename;
        p.replace_extension(".asm");
        std::string asm_filename = p.string();
        std::string exec_name = co.ex_name.empty() ? (p.parent_path() / "a.out").string() : co.ex_name;
        
        std::string cmd = "gcc -no-pie -x assembler " + asm_filename + " -x none build/libpcl.a -lm -o " + exec_name;
        if (std::system(cmd.c_str())) {
            std::cerr << "Error: Linking failed!" << std::endl;
            std::exit(1);
        }
    }

    void generateOutput(const CompilerOptions& co) {
        auto TargetMachine = setupTargetMachine(co.filename);
        emitIR(co);
        emitAssembly(co, TargetMachine.get());
        if (!co.final_code_stdout && !co.imm_code_stdout) {
            linkExecutable(co);
        }
    }

    void optimize() {
        TheFPM->addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
        TheFPM->addPass(llvm::InstCombinePass());
        TheFPM->addPass(llvm::ReassociatePass());
        TheFPM->addPass(llvm::GVNPass());
        TheFPM->addPass(llvm::SCCPPass());
        TheFPM->addPass(llvm::SimplifyCFGPass());
        TheFPM->addPass(llvm::LoopUnrollPass());
        TheFPM->addPass(llvm::ADCEPass());
        
        llvm::PassBuilder PB;
        PB.registerModuleAnalyses(*TheMAM);
        PB.registerFunctionAnalyses(*TheFAM);
        PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);

        for (auto &Func : *TheModule) {
            if (!Func.isDeclaration()) {
                TheFPM->run(Func, *TheFAM);
            }
        }
    }

    void BackendSymbolTable::enterScope() {
        NamedValues.push_back({});
    }

    void BackendSymbolTable::exitScope() {
        NamedValues.pop_back();
    }

    void BackendSymbolTable::insert(const std::string& name, llvm::AllocaInst* inst) {
        NamedValues.back()[name] = inst;
    }
    
    llvm::AllocaInst* BackendSymbolTable::lookup(const std::string& name) const {
        for (auto nv = NamedValues.rbegin(); nv != NamedValues.rend(); ++nv) {
            auto symbol = nv->find(name);
            if (symbol!=nv->end()) return symbol->second;
        }
        return nullptr;
    }
}
