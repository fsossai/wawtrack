#include <set>
#include <string>
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

  static std::set<LoadInst*> gatherLoads(Function &F) {
    std::set<LoadInst*> loads;

    for (auto &I : instructions(F)) {
      if (auto l = dyn_cast<LoadInst>(&I)) {
        loads.insert(l);
      }
    }

    return loads;
  }

  static std::set<StoreInst*> gatherStores(Function &F) {
    std::set<StoreInst*> stores;

    for (auto &I : instructions(F)) {
      if (auto s = dyn_cast<StoreInst>(&I)) {
        stores.insert(s);
      }
    }

    return stores;
  }

  static bool injectLoadTrackers(Function &F, const std::set<LoadInst*> &loads) {
    if (loads.size() == 0) {
      return false;
    }

    auto &context = F.getContext();
    auto voidPtrType = Type::getInt8PtrTy(context);
    auto funcName = "_ZN8wawtrack4loadEPv";
    auto funcType = FunctionType::get(Type::getVoidTy(context), { voidPtrType }, false);
    auto callee = F.getParent()->getOrInsertFunction(funcName, funcType);
    auto builder = IRBuilder<>(context);

    Function::Create(funcType, GlobalValue::ExternalLinkage, funcName, F.getParent());

    for (auto load : loads) {
      builder.SetInsertPoint(load);
      auto bitcast = builder.CreateBitCast(load->getPointerOperand(), voidPtrType);
      builder.CreateCall(callee, { bitcast });
    }

    return true;
  }

  static bool injectStoreTrackers(Function &F, const std::set<StoreInst*> &stores) {
    if (stores.size() == 0) {
      return false;
    }

    errs() << "wawtrack: " << F.getName() << "\n";

    auto &context = F.getContext();
    auto voidPtrType = Type::getInt8PtrTy(context);
    auto funcName = "_ZN8wawtrack5storeEPv";
    auto funcType = FunctionType::get(Type::getVoidTy(context), { voidPtrType }, false);
    auto callee = F.getParent()->getOrInsertFunction(funcName, funcType);
    auto builder = IRBuilder<>(context);

    Function::Create(funcType, GlobalValue::ExternalLinkage, funcName, F.getParent());

    for (auto store : stores) {
      builder.SetInsertPoint(store);
      auto bitcast = builder.CreateBitCast(store->getPointerOperand(), voidPtrType);
      builder.CreateCall(callee, { bitcast });
    }

    return true;
  }

  bool runOnFunction(Function &F) {
    bool changed = false;

    auto stores = WAWTrack::gatherStores(F);
    auto loads = WAWTrack::gatherLoads(F);

    changed |= WAWTrack::injectStoreTrackers(F, stores);
    changed |= WAWTrack::injectLoadTrackers(F, loads);

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

