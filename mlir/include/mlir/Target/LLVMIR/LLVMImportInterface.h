//===- LLVMImportInterface.h - Import from LLVM interface -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header file defines dialect interfaces for the LLVM IR import.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TARGET_LLVMIR_LLVMIMPORTINTERFACE_H
#define MLIR_TARGET_LLVMIR_LLVMIMPORTINTERFACE_H

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/DialectInterface.h"
#include "mlir/IR/Location.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/FormatVariadic.h"

namespace llvm {
class IRBuilderBase;
} // namespace llvm

namespace mlir {
namespace LLVM {
class ModuleImport;
} // namespace LLVM

/// Base class for dialect interfaces used to import LLVM IR. Dialects that can
/// be imported should provide an implementation of this interface for the
/// supported intrinsics. The interface may be implemented in a separate library
/// to avoid the "main" dialect library depending on LLVM IR. The interface can
/// be attached using the delayed registration mechanism available in
/// DialectRegistry.
class LLVMImportDialectInterface
    : public DialectInterface::Base<LLVMImportDialectInterface> {
public:
  LLVMImportDialectInterface(Dialect *dialect) : Base(dialect) {}

  /// Hook for derived dialect interfaces to implement the import of
  /// intrinsics into MLIR.
  virtual LogicalResult
  convertIntrinsic(OpBuilder &builder, llvm::CallInst *inst,
                   LLVM::ModuleImport &moduleImport) const {
    return failure();
  }

  /// Hook for derived dialect interfaces to implement the import of
  /// instructions into MLIR.
  virtual LogicalResult
  convertInstruction(OpBuilder &builder, llvm::Instruction *inst,
                     ArrayRef<llvm::Value *> llvmOperands,
                     LLVM::ModuleImport &moduleImport) const {
    return failure();
  }

  /// Hook for derived dialect interfaces to implement the import of metadata
  /// into MLIR. Attaches the converted metadata kind and node to the provided
  /// operation.
  virtual LogicalResult
  setMetadataAttrs(OpBuilder &builder, unsigned kind, llvm::MDNode *node,
                   Operation *op, LLVM::ModuleImport &moduleImport) const {
    return failure();
  }

  /// Hook for derived dialect interfaces to publish the supported intrinsics.
  /// As every LLVM IR intrinsic has a unique integer identifier, the function
  /// returns the list of supported intrinsic identifiers.
  virtual ArrayRef<unsigned> getSupportedIntrinsics() const { return {}; }

  /// Hook for derived dialect interfaces to publish the supported instructions.
  /// As every LLVM IR instruction has a unique integer identifier, the function
  /// returns the list of supported instruction identifiers. These identifiers
  /// will then be used to match LLVM instructions to the appropriate import
  /// interface and `convertInstruction` method. It is an error to have multiple
  /// interfaces overriding the same instruction.
  virtual ArrayRef<unsigned> getSupportedInstructions() const { return {}; }

  /// Hook for derived dialect interfaces to publish the supported metadata
  /// kinds. As every metadata kind has a unique integer identifier, the
  /// function returns the list of supported metadata identifiers. `ctx` can be
  /// used to obtain IDs of metadata kinds that do not have a fixed static one.
  virtual ArrayRef<unsigned>
  getSupportedMetadata(llvm::LLVMContext &ctx) const {
    return {};
  }
};

/// Interface collection for the import of LLVM IR that dispatches to a concrete
/// dialect interface implementation. Queries the dialect interfaces to obtain a
/// list of the supported LLVM IR constructs and then builds a mapping for the
/// efficient dispatch.
class LLVMImportInterface
    : public DialectInterfaceCollection<LLVMImportDialectInterface> {
public:
  using Base::Base;

  /// Queries all registered dialect interfaces for the supported LLVM IR
  /// intrinsic and metadata kinds and builds the dispatch tables for the
  /// conversion. Returns failure if multiple dialect interfaces translate the
  /// same LLVM IR intrinsic.
  LogicalResult initializeImport(llvm::LLVMContext &llvmContext) {
    for (const LLVMImportDialectInterface &iface : *this) {
      // Verify the supported intrinsics have not been mapped before.
      const auto *intrinsicIt =
          llvm::find_if(iface.getSupportedIntrinsics(), [&](unsigned id) {
            return intrinsicToDialect.count(id);
          });
      if (intrinsicIt != iface.getSupportedIntrinsics().end()) {
        return emitError(
            UnknownLoc::get(iface.getContext()),
            llvm::formatv(
                "expected unique conversion for intrinsic ({0}), but "
                "got conflicting {1} and {2} conversions",
                *intrinsicIt, iface.getDialect()->getNamespace(),
                intrinsicToDialect.lookup(*intrinsicIt)->getNamespace()));
      }
      const auto *instructionIt =
          llvm::find_if(iface.getSupportedInstructions(), [&](unsigned id) {
            return instructionToDialect.count(id);
          });
      if (instructionIt != iface.getSupportedInstructions().end()) {
        return emitError(
            UnknownLoc::get(iface.getContext()),
            llvm::formatv(
                "expected unique conversion for instruction ({0}), but "
                "got conflicting {1} and {2} conversions",
                *intrinsicIt, iface.getDialect()->getNamespace(),
                instructionToDialect.lookup(*intrinsicIt)
                    ->getDialect()
                    ->getNamespace()));
      }
      // Add a mapping for all supported intrinsic identifiers.
      for (unsigned id : iface.getSupportedIntrinsics())
        intrinsicToDialect[id] = iface.getDialect();
      // Add a mapping for all supported instruction identifiers.
      for (unsigned id : iface.getSupportedInstructions())
        instructionToDialect[id] = &iface;
      // Add a mapping for all supported metadata kinds.
      for (unsigned kind : iface.getSupportedMetadata(llvmContext))
        metadataToDialect[kind].push_back(iface.getDialect());
    }

    return success();
  }

  /// Converts the LLVM intrinsic to an MLIR operation if a conversion exists.
  /// Returns failure otherwise.
  LogicalResult convertIntrinsic(OpBuilder &builder, llvm::CallInst *inst,
                                 LLVM::ModuleImport &moduleImport) const;

  /// Returns true if the given LLVM IR intrinsic is convertible to an MLIR
  /// operation.
  bool isConvertibleIntrinsic(llvm::Intrinsic::ID id) {
    return intrinsicToDialect.count(id);
  }

  /// Converts the LLVM instruction to an MLIR operation if a conversion exists.
  /// Returns failure otherwise.
  LogicalResult convertInstruction(OpBuilder &builder, llvm::Instruction *inst,
                                   ArrayRef<llvm::Value *> llvmOperands,
                                   LLVM::ModuleImport &moduleImport) const {
    // Lookup the dialect interface for the given instruction.
    const LLVMImportDialectInterface *iface =
        instructionToDialect.lookup(inst->getOpcode());
    if (!iface)
      return failure();

    return iface->convertInstruction(builder, inst, llvmOperands, moduleImport);
  }

  /// Returns true if the given LLVM IR instruction is convertible to an MLIR
  /// operation.
  bool isConvertibleInstruction(unsigned id) {
    return instructionToDialect.count(id);
  }

  /// Attaches the given LLVM metadata to the imported operation if a conversion
  /// to one or more MLIR dialect attributes exists and succeeds. Returns
  /// success if at least one of the conversions is successful and failure if
  /// all of them fail.
  LogicalResult setMetadataAttrs(OpBuilder &builder, unsigned kind,
                                 llvm::MDNode *node, Operation *op,
                                 LLVM::ModuleImport &moduleImport) const {
    // Lookup the dialect interfaces for the given metadata.
    auto it = metadataToDialect.find(kind);
    if (it == metadataToDialect.end())
      return failure();

    // Dispatch the conversion to the dialect interfaces.
    bool isSuccess = false;
    for (Dialect *dialect : it->getSecond()) {
      const LLVMImportDialectInterface *iface = getInterfaceFor(dialect);
      assert(iface && "expected to find a dialect interface");
      if (succeeded(
              iface->setMetadataAttrs(builder, kind, node, op, moduleImport)))
        isSuccess = true;
    }

    // Returns failure if all conversions fail.
    return success(isSuccess);
  }

  /// Returns true if the given LLVM IR metadata is convertible to an MLIR
  /// attribute.
  bool isConvertibleMetadata(unsigned kind) {
    return metadataToDialect.count(kind);
  }

private:
  /// Generate llvm.call_intrinsic when no supporting dialect available.
  static LogicalResult
  convertUnregisteredIntrinsic(OpBuilder &builder, llvm::CallInst *inst,
                               LLVM::ModuleImport &moduleImport);

  DenseMap<unsigned, Dialect *> intrinsicToDialect;
  DenseMap<unsigned, const LLVMImportDialectInterface *> instructionToDialect;
  DenseMap<unsigned, SmallVector<Dialect *, 1>> metadataToDialect;
};

} // namespace mlir

#endif // MLIR_TARGET_LLVMIR_LLVMIMPORTINTERFACE_H
