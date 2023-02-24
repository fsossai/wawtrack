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

  std::set<LoadInst*> gatherLoads(Function &F) {
    std::set<LoadInst*> loads;

    for (auto &I : instructions(F)) {
      if (auto l = dyn_cast<StoreInst>(&I)) {
        loads.insert(l);
      }
    }

    return loads;
  }

  std::set<StoreInst*> gatherStores(Function &F) {
    std::set<StoreInst*> stores;

    for (auto &I : instructions(F)) {
      if (auto s = dyn_cast<StoreInst>(&I)) {
        stores.insert(s);
      }
    }

    return stores;
  }

  bool runOnFunction(Function &F) {
    bool changed = false;

    auto stores = gatherStores(F);
    auto loads = gatherLoads(F);

    auto trackerStoreFuncName = "_ZN8wawtrack5storeEPv";
    auto trackerLoadFuncName = "_ZN8wawtrack4loadEPv";
    auto trackerDumpFuncName = "_ZN8wawtrack4dumpEv";

    auto &context = F.getContext();

    auto voidPtrType = Type::getInt8PtrTy(context);
    auto eventFuncType = FunctionType::get(Type::getVoidTy(context), { voidPtrType }, false);
    auto dumpFuncType = FunctionType::get(Type::getVoidTy(context), { }, false);

    Function::Create(eventFuncType, GlobalValue::ExternalLinkage, trackerStoreFuncName, F.getParent());
    Function::Create(eventFuncType, GlobalValue::ExternalLinkage, trackerLoadFuncName, F.getParent());
    Function::Create(dumpFuncType, GlobalValue::ExternalLinkage, trackerDumpFuncName, F.getParent());

    auto builder = IRBuilder<>(context);

    auto callee1 = F.getParent()->getOrInsertFunction(trackerDumpFuncName, dumpFuncType);
    auto callee2 = F.getParent()->getOrInsertFunction(trackerLoadFuncName, eventFuncType);
    auto callee3 = F.getParent()->getOrInsertFunction(trackerStoreFuncName, eventFuncType);

    builder.SetInsertPoint(store);
    auto call1 = builder.CreateCall(callee1, { });
    builder.SetInsertPoint(call1);
    auto call2 = builder.CreateCall(callee2, { store->getPointerOperand() });
    builder.SetInsertPoint(call2);
    auto call3 = builder.CreateCall(callee3, { store->getPointerOperand() });
    builder.SetInsertPoint(call3);
    auto call4 = builder.CreateCall(callee3, { store->getPointerOperand() });

    errs() << *call1 << "\n";
    errs() << *call2 << "\n";

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

