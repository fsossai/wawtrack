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
    bool changed = false;
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

