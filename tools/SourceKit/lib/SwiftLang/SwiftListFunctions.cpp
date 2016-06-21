//===--- SwiftDocSupport.cpp ----------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "SwiftASTManager.h"
#include "SwiftEditorDiagConsumer.h"
#include "SwiftLangSupport.h"
#include "SourceKit/Support/UIdent.h"

#include "swift/AST/ASTPrinter.h"
#include "swift/AST/ASTWalker.h"
#include "swift/AST/SourceEntityWalker.h"
#include "swift/Frontend/Frontend.h"
#include "swift/Frontend/PrintingDiagnosticConsumer.h"
#include "swift/IDE/CommentConversion.h"
#include "swift/IDE/ModuleInterfacePrinting.h"
#include "swift/IDE/SyntaxModel.h"
// This is included only for createLazyResolver(). Move to different header ?
#include "swift/Sema/IDETypeChecking.h"
#include "swift/Config.h"

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"

using namespace SourceKit;
using namespace swift;
using namespace ide;

class SourceDocASTWalker : public SourceEntityWalker {
public:
  SourceManager &SM;
  unsigned BufferID;

  SourceDocASTWalker(SourceManager &SM, unsigned BufferID)
    : SM(SM), BufferID(BufferID) {}

  ~SourceDocASTWalker() {
  }

  bool walkToDeclPre(Decl *D, CharSourceRange Range) override {
    return true;
  }

  bool walkToDeclPost(Decl *D) override {
    if (isa<FuncDecl>(D)) {
        FuncDecl *FD = dyn_cast<FuncDecl>(D);
        llvm::outs() <<"\n" << "function: " << FD->getNameStr();
        for (auto pl: FD->getParameterLists()) {
            llvm::outs() << " Params: ";
            for (auto p: *pl) {
                llvm::outs() << p->getArgumentName().str() << " ";
                llvm::outs() << p->getTypeLoc().getType().getString() << " ";
                p->getTypeLoc().getSourceRange().print(llvm::outs(), SM);
            }
        }
        llvm::outs() << "\n";
        //FD->dump(llvm::outs());
    }
    return true;
  }

  bool visitDeclReference(ValueDecl *D, CharSourceRange Range,
                          TypeDecl *CtorTyRef, Type Ty) override {
    return true;
  }

  bool visitSubscriptReference(ValueDecl *D, CharSourceRange Range,
                               bool IsOpenBracket) override {
    // Treat both open and close brackets equally
    return visitDeclReference(D, Range, nullptr, Type());
  }

  bool isLocal(Decl *D) const {
    return isa<ParamDecl>(D) || D->getDeclContext()->getLocalContext();
  }

  unsigned getOffset(SourceLoc Loc) const {
    return SM.getLocOffsetInBuffer(Loc, BufferID);
  }

};

void SwiftLangSupport::listFunctions() {
  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  CompilerInvocation Invocation;
  std::string Error;

  SmallVector<const char *, 8> Args;
  bool Failed = getASTManager().initCompilerInvocation(Invocation, Args,
                                                       CI.getDiags(),
                                                       StringRef(),
                                                       Error);
  llvm::outs() << Error;
  if (Failed) {
      llvm::outs() << "Failed to init compiler";
    return;
  }
  Invocation.getClangImporterOptions().ImportForwardDeclarations = true;
  llvm::outs() << "init compiler";

  std::unique_ptr<llvm::MemoryBuffer> InputBuf;
  std::string str = "func abc(){}\n"
      "func cool(a: Type){}\n"
      "func aFunction(a: Int, b: Double, c: () -> Void) -> Int { return 1 }"
      ;
  InputBuf = llvm::MemoryBuffer::getMemBuffer(str, "<input>");
  Invocation.addInputBuffer(InputBuf.get());
   if (CI.setup(Invocation)) {
       llvm::outs() << "ERRROR";
       return;
   }
 
   ASTContext &Ctx = CI.getASTContext();
   CloseClangModuleFiles scopedCloseFiles(*Ctx.getClangModuleLoader());
   CI.performSema();
 
   // Setup a typechecker for protocol conformance resolving.
   OwnedResolver TypeResolver = createLazyResolver(Ctx);
   SourceManager &SM = CI.getSourceMgr();
   unsigned BufID = CI.getInputBufferIDs().back();

   SourceDocASTWalker Walker(SM, BufID);
   Walker.walk(*CI.getMainModule());
}
