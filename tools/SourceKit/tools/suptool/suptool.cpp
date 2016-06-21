//===--- sourcekitd-repl.cpp ----------------------------------------------===//
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

#include "SourceKit/Core/Context.h"
#include "SourceKit/Core/LangSupport.h"
#include "SourceKit/Core/NotificationCenter.h"
#include "SourceKit/Support/Concurrency.h"
#include "SourceKit/Support/Logging.h"
#include "SourceKit/Support/UIdent.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Mutex.h"
#include <unistd.h>
#include <histedit.h>

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Threading.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/YAMLParser.h"
#include <mutex>


#include "llvm/Support/Path.h"

// FIXME: Portability ?
#include <Block.h>
#include <dispatch/dispatch.h>

#ifdef LLVM_ON_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif
using namespace llvm;
using namespace SourceKit;

void initialize();

std::string getRuntimeLibPath() {
  // FIXME: Move to an LLVM API. Note that libclang does the same thing.
#ifdef LLVM_ON_WIN32
#error Not implemented
#else
  // This silly cast below avoids a C++ warning.
  Dl_info info;
  if (dladdr((void *)(uintptr_t)initialize, &info) == 0)
    llvm_unreachable("Call to dladdr() failed");

  // We now have the path to the shared lib, move to the parent 'lib' path.
  return llvm::sys::path::parent_path(info.dli_fname);
#endif
}

static SourceKit::Context *GlobalCtx = nullptr;

void initialize() {
  GlobalCtx = new SourceKit::Context(getRuntimeLibPath());
  //GlobalCtx->getNotificationCenter().addDocumentUpdateNotificationReceiver(
  //  onDocumentUpdateNotification);
}

void shutdown() {
  delete GlobalCtx;
  GlobalCtx = nullptr;
}

static SourceKit::Context &getTheGlobalContext() {
  assert(GlobalCtx);
  return *GlobalCtx;
}

int main() {
    initialize();
    LangSupport &Lang = getTheGlobalContext().getSwiftLangSupport();
    //Lang.getDocInfo(InputBuf, ModuleName, Args, DocConsumer);
    
    // Get stuff into memory buffer
    //llvm::outs() << InputBuf->getBuffer();
    
    Lang.doMyStuff();

    //CompilerInstance CI;
    //// Display diagnostics to stderr.
    //PrintingDiagnosticConsumer PrintDiags;
    //CI.addDiagnosticConsumer(&PrintDiags);

    //CompilerInvocation Invocation;
    //std::string Error;
    //bool Failed = getASTManager().initCompilerInvocation(Invocation, Args,
    //                                                   CI.getDiags(),
    //                                                   StringRef(),
    //                                                   Error);
    //if (Failed) {
    //    llvm::outs() << "FAILED"; 
    //    return 1;
    //}
    //Invocation.getClangImporterOptions().ImportForwardDeclarations = true;


    shutdown();
    return 0;
}

