#include <iostream>

#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {

class WAWTrack : public FunctionPass {
public:
  static char ID;

  WAWTrack() : FunctionPass(ID) { }

  bool runOnFunction(Function &F) {
    if (F.getName() != "main") return false;

    bool changed = false;

    StoreInst *store = nullptr;
    for (auto &I : instructions(F)) {
      if (auto s = dyn_cast<StoreInst>(&I)) {
        store = s;
      }
    }

    auto versionFunc = "_ZN8wawtrack7versionEv";

    errs() << *store << "\n";
    auto &context = F.getContext();
    auto voidPtrType = Type::getInt8PtrTy(context);
    auto funcType = FunctionType::get(Type::getVoidTy(context), { voidPtrType }, false);
    Function::Create(funcType, GlobalValue::ExternalLinkage, versionFunc, F.getParent());
    auto callee = F.getParent()->getOrInsertFunction(versionFunc, funcType);
    auto builder = IRBuilder<>(context);
    builder.SetInsertPoint(store);
    auto call = builder.CreateCall(callee, { store->getPointerOperand() });
    errs() << *call << "\n";

    return changed;
  }

private:
};

} // namespace

// Register pass to `opt`

char WAWTrack::ID = 0;
static RegisterPass<WAWTrack> X("waw-track",
    "Tracking Write-After-Write events and stored but not loaded memory locations"
);

// Register pass to `clang`

static WAWTrack * _PassMaker = NULL;

static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
      if(!_PassMaker) { PM.add(_PassMaker = new WAWTrack()); }
    });

static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
      if(!_PassMaker){ PM.add(_PassMaker = new WAWTrack()); }
    });

