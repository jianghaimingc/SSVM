// SPDX-License-Identifier: Apache-2.0
#include <vector>

#include "api/ssvm.h"

#include "common/errcode.h"
#include "common/log.h"
#include "common/span.h"
#include "common/statistics.h"
#include "common/value.h"

#include "ast/module.h"
#include "host/wasi/wasimodule.h"
#include "interpreter/interpreter.h"
#include "loader/loader.h"
#include "runtime/storemgr.h"
#include "validator/validator.h"
#include "vm/vm.h"

/// SSVM_ConfigureContext implementation.
struct SSVM_ConfigureContext {
  SSVM::Configure Conf;
};

/// SSVM_ASTModuleContext implementation.
struct SSVM_ASTModuleContext {
  SSVM_ASTModuleContext(std::unique_ptr<SSVM::AST::Module> Mod)
      : Module(std::move(Mod)) {}
  std::unique_ptr<SSVM::AST::Module> Module;
};

/// SSVM_LoaderContext implementation.
struct SSVM_LoaderContext {
  SSVM_LoaderContext(const SSVM::Configure &Conf) : Load(Conf) {}
  SSVM::Loader::Loader Load;
};

/// SSVM_ValidatorContext implementation.
struct SSVM_ValidatorContext {
  SSVM_ValidatorContext(const SSVM::Configure &Conf) : Valid(Conf) {}
  SSVM::Validator::Validator Valid;
};

/// SSVM_InterpreterContext implementation.
struct SSVM_InterpreterContext {
  SSVM_InterpreterContext(const SSVM::Configure &Conf,
                          SSVM::Statistics::Statistics *S = nullptr)
      : Interp(Conf, S) {}
  SSVM::Interpreter::Interpreter Interp;
};

/// SSVM_VMContext implementation.
struct SSVM_VMContext {
  template <typename... Args>
  SSVM_VMContext(Args &&... Vals) : VM(std::forward<Args>(Vals)...) {}
  SSVM::VM::VM VM;
};

namespace {

using namespace SSVM;

/// Helper function for returning a SSVM_Result by error code.
inline constexpr SSVM_Result genSSVM_Result(ErrCode Code) {
  return SSVM_Result{.Code = static_cast<uint8_t>(Code)};
}

/// Helper functions for returning a SSVM_Value by various values.
template <typename T> inline SSVM_Value genSSVM_Value(T Val) {
  return SSVM_Value{
      .Value = retrieveValue<unsigned __int128>(ValVariant(toUnsigned(Val)))};
}
template <> inline constexpr SSVM_Value genSSVM_Value(__int128 Val) {
  return SSVM_Value{.Value = static_cast<unsigned __int128>(Val)};
}
template <> inline SSVM_Value genSSVM_Value(ValVariant Val) {
  return SSVM_Value{.Value = retrieveValue<unsigned __int128>(Val)};
}
template <> inline SSVM_Value genSSVM_Value(RefVariant Val) {
  return genSSVM_Value(ValVariant(Val));
}

/// Helper function for converting a SSVM_Value array to a ValVariant vector.
inline std::vector<ValVariant> genValVariantVec(const SSVM_Value *Val,
                                                const uint32_t Len) {
  std::vector<ValVariant> Vec(Len, 0U);
  for (uint32_t I = 0; I < Len && Val; I++) {
    Vec[I] = Val[I].Value;
  }
  return Vec;
}

/// Helper function for making a Span to a uint8_t array.
template <typename T>
inline constexpr Span<const T> genSpan(const T *Buf, const uint32_t Len) {
  return Span<const T>(Buf, Len);
}

/// Helper functions for converting a ValVariant vector to a SSVM_Value array.
inline constexpr void fillSSVM_ValueArr(Span<const ValVariant> Vec,
                                        SSVM_Value *Val, const uint32_t Len) {
  if (Val == nullptr) {
    return;
  }
  for (uint32_t I = 0; I < Len && I < Vec.size(); I++) {
    Val[I] = genSSVM_Value(Vec[I]);
  }
}

/// Helper template to run and return result.
auto EmptyThen = [](auto &&Res) {};
template <typename T> inline bool isContext(T *Cxt) { return (Cxt != nullptr); }
template <typename T, typename... Args>
inline bool isContext(T *Cxt, Args *... Cxts) {
  return isContext(Cxt) && isContext(Cxts...);
}
template <typename T, typename U, typename... CxtT>
inline SSVM_Result wrap(T &&Proc, U &&Then, CxtT *... Cxts) {
  if (isContext(Cxts...)) {
    if (auto Res = Proc()) {
      Then(Res);
      return genSSVM_Result(ErrCode::Success);
    } else {
      return genSSVM_Result(Res.error());
    }
  } else {
    return genSSVM_Result(ErrCode::WrongVMWorkflow);
  }
}

/// Helper function for deletion.
template <typename T> inline constexpr void deleteIf(T *Cxt) {
  if (Cxt) {
    delete Cxt;
  }
}

/// Helper functions of context conversions.
#define CONVTO(SIMP, INST, NAME, QUANT)                                        \
  inline QUANT auto *to##SIMP##Cxt(QUANT INST *Cxt) {                          \
    return reinterpret_cast<QUANT SSVM_##NAME##Context *>(Cxt);                \
  }
CONVTO(Stat, Statistics::Statistics, Statistics, )
CONVTO(Store, Runtime::StoreManager, Store, )
CONVTO(Mod, Runtime::Instance::ModuleInstance, ModuleInstance, )
CONVTO(FType, Runtime::Instance::FType, FunctionType, )
CONVTO(FType, Runtime::Instance::FType, FunctionType, const)
CONVTO(Func, Runtime::Instance::FunctionInstance, FunctionInstance, )
CONVTO(Tab, Runtime::Instance::TableInstance, TableInstance, )
CONVTO(Mem, Runtime::Instance::MemoryInstance, MemoryInstance, )
CONVTO(Glob, Runtime::Instance::GlobalInstance, GlobalInstance, )
CONVTO(ImpObj, Runtime::ImportObject, ImportObject, )
#undef CONVTO

#define CONVFROM(SIMP, INST, NAME, QUANT)                                      \
  inline QUANT auto *from##SIMP##Cxt(QUANT SSVM_##NAME##Context *Cxt) {        \
    return reinterpret_cast<QUANT INST *>(Cxt);                                \
  }
CONVFROM(Stat, Statistics::Statistics, Statistics, )
CONVFROM(Stat, Statistics::Statistics, Statistics, const)
CONVFROM(Store, Runtime::StoreManager, Store, )
CONVFROM(Store, Runtime::StoreManager, Store, const)
CONVFROM(Mod, Runtime::Instance::ModuleInstance, ModuleInstance, const)
CONVFROM(FType, Runtime::Instance::FType, FunctionType, const)
CONVFROM(Func, Runtime::Instance::FunctionInstance, FunctionInstance, const)
CONVFROM(Tab, Runtime::Instance::TableInstance, TableInstance, )
CONVFROM(Tab, Runtime::Instance::TableInstance, TableInstance, const)
CONVFROM(Mem, Runtime::Instance::MemoryInstance, MemoryInstance, )
CONVFROM(Mem, Runtime::Instance::MemoryInstance, MemoryInstance, const)
CONVFROM(Glob, Runtime::Instance::GlobalInstance, GlobalInstance, )
CONVFROM(Glob, Runtime::Instance::GlobalInstance, GlobalInstance, const)
CONVFROM(ImpObj, Runtime::ImportObject, ImportObject, )
CONVFROM(ImpObj, Runtime::ImportObject, ImportObject, const)
#undef CONVFROM

/// Helper function of retrieving exported maps.
inline uint32_t fillMap(const std::map<std::string, uint32_t, std::less<>> &Map,
                        char **Names, SSVM_InstanceAddress *FuncAddrs,
                        const uint32_t Len) {
  if (Names && FuncAddrs) {
    uint32_t I = 0;
    for (auto &&Pair : Map) {
      if (I > Len) {
        break;
      }
      size_t NameLen = Pair.first.length();
      Names[I] = new char[NameLen + 1];
      std::strncpy(Names[I], Pair.first.c_str(), NameLen);
      Names[I][NameLen] = '\0';
      FuncAddrs[I].Addr = Pair.second;
      I++;
    }
  }
  return Map.size();
}

} // namespace

#ifdef __cplusplus
extern "C" {
#endif

/// >>>>>>>> SSVM version functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

const char *SSVM_VersionGet() { return SSVM_VERSION; }

uint32_t SSVM_VersionGetMajor() { return SSVM_VERSION_MAJOR; }

uint32_t SSVM_VersionGetMinor() { return SSVM_VERSION_MINOR; }

uint32_t SSVM_VersionGetPatch() { return SSVM_VERSION_PATCH; }

/// <<<<<<<< SSVM version functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM logging functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void SSVM_LogSetErrorLevel() { SSVM::Log::setErrorLoggingLevel(); }

void SSVM_LogSetDebugLevel() { SSVM::Log::setDebugLoggingLevel(); }

/// <<<<<<<< SSVM logging functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM value functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_Value SSVM_ValueGenI32(const int32_t Val) { return genSSVM_Value(Val); }

SSVM_Value SSVM_ValueGenI64(const int64_t Val) { return genSSVM_Value(Val); }

SSVM_Value SSVM_ValueGenF32(const float Val) { return genSSVM_Value(Val); }

SSVM_Value SSVM_ValueGenF64(const double Val) { return genSSVM_Value(Val); }

SSVM_Value SSVM_ValueGenV128(const __int128 Val) { return genSSVM_Value(Val); }

SSVM_Value SSVM_ValueGenNullRef(const SSVM_RefType T) {
  return genSSVM_Value(SSVM::genNullRef(static_cast<SSVM::RefType>(T)));
}

SSVM_Value SSVM_ValueGenFuncRef(const uint32_t Index) {
  return genSSVM_Value(SSVM::genFuncRef(Index));
}

SSVM_Value SSVM_ValueGenExternRef(void *Ref) {
  return genSSVM_Value(SSVM::genExternRef(Ref));
}

int32_t SSVM_ValueGetI32(const SSVM_Value Val) {
  return SSVM::retrieveValue<int32_t>(SSVM::ValVariant(Val.Value));
}

int64_t SSVM_ValueGetI64(const SSVM_Value Val) {
  return SSVM::retrieveValue<int64_t>(SSVM::ValVariant(Val.Value));
}

float SSVM_ValueGetF32(const SSVM_Value Val) {
  return SSVM::retrieveValue<float>(SSVM::ValVariant(Val.Value));
}

double SSVM_ValueGetF64(const SSVM_Value Val) {
  return SSVM::retrieveValue<double>(SSVM::ValVariant(Val.Value));
}

__int128 SSVM_ValueGetV128(const SSVM_Value Val) {
  return SSVM::retrieveValue<__int128>(SSVM::ValVariant(Val.Value));
}

uint32_t SSVM_ValueGetFuncIdx(const SSVM_Value Val) {
  return SSVM::retrieveFuncIdx(SSVM::ValVariant(Val.Value));
}

void *SSVM_ValueGetExternRef(const SSVM_Value Val) {
  return &SSVM::retrieveExternRef<uint32_t>(SSVM::ValVariant(Val.Value));
}

/// <<<<<<<< SSVM value functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM result functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

bool SSVM_ResultOK(const SSVM_Result Res) {
  if (static_cast<SSVM::ErrCode>(Res.Code) == SSVM::ErrCode::Success ||
      static_cast<SSVM::ErrCode>(Res.Code) == SSVM::ErrCode::Terminated) {
    return true;
  } else {
    return false;
  }
}

uint32_t SSVM_ResultGetCode(const SSVM_Result Res) {
  return static_cast<uint32_t>(Res.Code);
}

const char *SSVM_ResultGetMessage(const SSVM_Result Res) {
  return SSVM::ErrCodeStr[static_cast<SSVM::ErrCode>(Res.Code)].c_str();
}

/// <<<<<<<< SSVM result functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM configure functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_ConfigureContext *SSVM_ConfigureCreate() {
  return new SSVM_ConfigureContext;
}

void SSVM_ConfigureAddProposal(SSVM_ConfigureContext *Cxt,
                               const SSVM_Proposal Prop) {
  if (Cxt) {
    Cxt->Conf.addProposal(static_cast<SSVM::Proposal>(Prop));
  }
}

void SSVM_ConfigureRemoveProposal(SSVM_ConfigureContext *Cxt,
                                  const SSVM_Proposal Prop) {
  if (Cxt) {
    Cxt->Conf.removeProposal(static_cast<SSVM::Proposal>(Prop));
  }
}

bool SSVM_ConfigureHasProposal(const SSVM_ConfigureContext *Cxt,
                               const SSVM_Proposal Prop) {
  if (Cxt) {
    return Cxt->Conf.hasProposal(static_cast<SSVM::Proposal>(Prop));
  }
  return false;
}

void SSVM_ConfigureAddHostRegistration(SSVM_ConfigureContext *Cxt,
                                       const SSVM_HostRegistration Host) {
  if (Cxt) {
    Cxt->Conf.addHostRegistration(static_cast<SSVM::HostRegistration>(Host));
  }
}

void SSVM_ConfigureRemoveHostRegistration(SSVM_ConfigureContext *Cxt,
                                          const SSVM_HostRegistration Host) {
  if (Cxt) {
    Cxt->Conf.removeHostRegistration(static_cast<SSVM::HostRegistration>(Host));
  }
}

bool SSVM_ConfigureHasHostRegistration(const SSVM_ConfigureContext *Cxt,
                                       const SSVM_HostRegistration Host) {
  if (Cxt) {
    return Cxt->Conf.hasHostRegistration(
        static_cast<SSVM::HostRegistration>(Host));
  }
  return false;
}

void SSVM_ConfigureSetMaxMemoryPage(SSVM_ConfigureContext *Cxt,
                                    const uint32_t Page) {
  if (Cxt) {
    Cxt->Conf.setMaxMemoryPage(Page);
  }
}

uint32_t SSVM_ConfigureGetMaxMemoryPage(const SSVM_ConfigureContext *Cxt) {
  if (Cxt) {
    return Cxt->Conf.getMaxMemoryPage();
  }
  return 0;
}

void SSVM_ConfigureDelete(SSVM_ConfigureContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM configure functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM statistics functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_StatisticsContext *SSVM_StatisticsCreate() {
  return toStatCxt(new SSVM::Statistics::Statistics);
}

uint64_t SSVM_StatisticsGetInstrCount(const SSVM_StatisticsContext *Cxt) {
  return fromStatCxt(Cxt)->getInstrCount();
}

double SSVM_StatisticsGetInstrPerSecond(const SSVM_StatisticsContext *Cxt) {
  return fromStatCxt(Cxt)->getInstrPerSecond();
}

uint64_t SSVM_StatisticsGetTotalCost(const SSVM_StatisticsContext *Cxt) {
  return fromStatCxt(Cxt)->getTotalCost();
}

void SSVM_StatisticsSetCostTable(SSVM_StatisticsContext *Cxt, uint64_t *CostArr,
                                 const uint32_t Len) {
  fromStatCxt(Cxt)->setCostTable(genSpan(CostArr, Len));
}

void SSVM_StatisticsSetCostLimit(SSVM_StatisticsContext *Cxt,
                                 const uint64_t Limit) {
  fromStatCxt(Cxt)->setCostLimit(Limit);
}

void SSVM_StatisticsDelete(SSVM_StatisticsContext *Cxt) {
  deleteIf(fromStatCxt(Cxt));
}

/// <<<<<<<< SSVM statistics functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM AST module functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void SSVM_ASTModuleDelete(SSVM_ASTModuleContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM AST module functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM loader functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_LoaderContext *SSVM_LoaderCreate(const SSVM_ConfigureContext *ConfCxt) {
  if (ConfCxt) {
    return new SSVM_LoaderContext(ConfCxt->Conf);
  } else {
    return new SSVM_LoaderContext(SSVM::Configure());
  }
}

SSVM_Result SSVM_LoaderParseFromFile(SSVM_LoaderContext *Cxt,
                                     SSVM_ASTModuleContext **Module,
                                     const char *Path) {
  return wrap(
      [&]() { return Cxt->Load.parseModule(Path); },
      [&](auto &&Res) { *Module = new SSVM_ASTModuleContext(std::move(*Res)); },
      Cxt);
}

SSVM_Result SSVM_LoaderParseFromBuffer(SSVM_LoaderContext *Cxt,
                                       SSVM_ASTModuleContext **Module,
                                       const uint8_t *Buf,
                                       const uint32_t BufLen) {
  return wrap(
      [&]() { return Cxt->Load.parseModule(genSpan(Buf, BufLen)); },
      [&](auto &&Res) { *Module = new SSVM_ASTModuleContext(std::move(*Res)); },
      Cxt);
}

void SSVM_LoaderDelete(SSVM_LoaderContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM loader functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM validator functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_ValidatorContext *
SSVM_ValidatorCreate(const SSVM_ConfigureContext *ConfCxt) {
  if (ConfCxt) {
    return new SSVM_ValidatorContext(ConfCxt->Conf);
  } else {
    return new SSVM_ValidatorContext(SSVM::Configure());
  }
}

SSVM_Result SSVM_ValidatorValidate(SSVM_ValidatorContext *Cxt,
                                   const SSVM_ASTModuleContext *ModuleCxt) {
  return wrap([&]() { return Cxt->Valid.validate(*ModuleCxt->Module.get()); },
              EmptyThen, Cxt);
}

void SSVM_ValidatorDelete(SSVM_ValidatorContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM validator functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM interpreter functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_InterpreterContext *
SSVM_InterpreterCreate(const SSVM_ConfigureContext *ConfCxt,
                       SSVM_StatisticsContext *StatCxt) {
  if (ConfCxt) {
    if (StatCxt) {
      return new SSVM_InterpreterContext(ConfCxt->Conf, fromStatCxt(StatCxt));
    } else {
      return new SSVM_InterpreterContext(ConfCxt->Conf);
    }
  } else {
    if (StatCxt) {
      return new SSVM_InterpreterContext(SSVM::Configure(),
                                         fromStatCxt(StatCxt));
    } else {
      return new SSVM_InterpreterContext(SSVM::Configure());
    }
  }
}

SSVM_Result SSVM_InterpreterInstantiate(SSVM_InterpreterContext *Cxt,
                                        SSVM_StoreContext *StoreCxt,
                                        const SSVM_ASTModuleContext *ASTCxt) {
  return wrap(
      [&]() {
        return Cxt->Interp.instantiateModule(*fromStoreCxt(StoreCxt),
                                             *ASTCxt->Module.get());
      },
      EmptyThen, Cxt, StoreCxt, ASTCxt);
}

SSVM_Result
SSVM_InterpreterRegisterImport(SSVM_InterpreterContext *Cxt,
                               SSVM_StoreContext *StoreCxt,
                               const SSVM_ImportObjectContext *ImportCxt) {
  return wrap(
      [&]() {
        return Cxt->Interp.registerModule(*fromStoreCxt(StoreCxt),
                                          *fromImpObjCxt(ImportCxt));
      },
      EmptyThen, Cxt, StoreCxt, ImportCxt);
}

SSVM_Result SSVM_InterpreterRegisterModule(SSVM_InterpreterContext *Cxt,
                                           SSVM_StoreContext *StoreCxt,
                                           const SSVM_ASTModuleContext *ASTCxt,
                                           const char *ModuleName) {
  return wrap(
      [&]() {
        return Cxt->Interp.registerModule(*fromStoreCxt(StoreCxt),
                                          *ASTCxt->Module.get(), ModuleName);
      },
      EmptyThen, Cxt, StoreCxt, ASTCxt);
}

SSVM_Result SSVM_InterpreterInvoke(SSVM_InterpreterContext *Cxt,
                                   SSVM_StoreContext *StoreCxt,
                                   const char *FuncName,
                                   const SSVM_Value *Params,
                                   const uint32_t ParamLen, SSVM_Value *Returns,
                                   const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap(
      [&]() -> SSVM::Expect<std::vector<SSVM::ValVariant>> {
        /// Check exports for finding function address.
        const auto FuncExp = fromStoreCxt(StoreCxt)->getFuncExports();
        const auto FuncIter = FuncExp.find(FuncName);
        if (FuncIter == FuncExp.cend()) {
          LOG(ERROR) << SSVM::ErrCode::FuncNotFound;
          LOG(ERROR) << SSVM::ErrInfo::InfoExecuting("", FuncName);
          return Unexpect(SSVM::ErrCode::FuncNotFound);
        }
        return Cxt->Interp.invoke(*fromStoreCxt(StoreCxt), FuncIter->second,
                                  ParamVec);
      },
      [&](auto &&Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); }, Cxt,
      StoreCxt);
}

SSVM_Result SSVM_InterpreterInvokeRegistered(
    SSVM_InterpreterContext *Cxt, SSVM_StoreContext *StoreCxt,
    const char *ModuleName, const char *FuncName, const SSVM_Value *Params,
    const uint32_t ParamLen, SSVM_Value *Returns, const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap(
      [&]() -> SSVM::Expect<std::vector<SSVM::ValVariant>> {
        /// Get module instance.
        SSVM::Runtime::Instance::ModuleInstance *ModInst;
        if (auto Res = fromStoreCxt(StoreCxt)->findModule(ModuleName)) {
          ModInst = *Res;
        } else {
          LOG(ERROR) << Res.error();
          LOG(ERROR) << SSVM::ErrInfo::InfoExecuting(ModuleName, FuncName);
          return Unexpect(Res);
        }

        /// Get exports and find function.
        const auto FuncExp = ModInst->getFuncExports();
        const auto FuncIter = FuncExp.find(FuncName);
        if (FuncIter == FuncExp.cend()) {
          LOG(ERROR) << SSVM::ErrCode::FuncNotFound;
          LOG(ERROR) << SSVM::ErrInfo::InfoExecuting(ModuleName, FuncName);
          return Unexpect(SSVM::ErrCode::FuncNotFound);
        }
        return Cxt->Interp.invoke(*fromStoreCxt(StoreCxt), FuncIter->second,
                                  ParamVec);
      },
      [&](auto &&Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); }, Cxt,
      StoreCxt);
}

void SSVM_InterpreterDelete(SSVM_InterpreterContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM interpreter functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM store functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_StoreContext *SSVM_StoreCreate() {
  return toStoreCxt(new SSVM::Runtime::StoreManager);
}

SSVM_ModuleInstanceContext *
SSVM_StoreGetModule(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr) {
  if (Cxt) {
    if (auto Res = fromStoreCxt(Cxt)->getModule(Addr.Addr)) {
      return toModCxt(*Res);
    }
  }
  return nullptr;
}

SSVM_FunctionInstanceContext *
SSVM_StoreGetFunction(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr) {
  if (Cxt) {
    if (auto Res = fromStoreCxt(Cxt)->getFunction(Addr.Addr)) {
      return toFuncCxt(*Res);
    }
  }
  return nullptr;
}

SSVM_TableInstanceContext *SSVM_StoreGetTable(SSVM_StoreContext *Cxt,
                                              const SSVM_InstanceAddress Addr) {
  if (Cxt) {
    if (auto Res = fromStoreCxt(Cxt)->getTable(Addr.Addr)) {
      return toTabCxt(*Res);
    }
  }
  return nullptr;
}

SSVM_MemoryInstanceContext *
SSVM_StoreGetMemory(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr) {
  if (Cxt) {
    if (auto Res = fromStoreCxt(Cxt)->getMemory(Addr.Addr)) {
      return toMemCxt(*Res);
    }
  }
  return nullptr;
}

SSVM_GlobalInstanceContext *
SSVM_StoreGetGlobal(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr) {
  if (Cxt) {
    if (auto Res = fromStoreCxt(Cxt)->getGlobal(Addr.Addr)) {
      return toGlobCxt(*Res);
    }
  }
  return nullptr;
}

uint32_t SSVM_StoreGetFunctionExportsLength(const SSVM_StoreContext *Cxt) {
  if (Cxt) {
    return fromStoreCxt(Cxt)->getFuncExports().size();
  }
  return 0;
}

uint32_t SSVM_StoreGetFunctionExports(const SSVM_StoreContext *Cxt,
                                      char **Names,
                                      SSVM_InstanceAddress *FuncAddrs,
                                      const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromStoreCxt(Cxt)->getFuncExports(), Names, FuncAddrs, Len);
  }
  return 0;
}

uint32_t SSVM_StoreGetTableExportsLength(const SSVM_StoreContext *Cxt) {
  if (Cxt) {
    return fromStoreCxt(Cxt)->getTableExports().size();
  }
  return 0;
}

uint32_t SSVM_StoreGetTableExports(const SSVM_StoreContext *Cxt, char **Names,
                                   SSVM_InstanceAddress *TableAddrs,
                                   const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromStoreCxt(Cxt)->getTableExports(), Names, TableAddrs,
                   Len);
  }
  return 0;
}

uint32_t SSVM_StoreGetMemoryExportsLength(const SSVM_StoreContext *Cxt) {
  if (Cxt) {
    return fromStoreCxt(Cxt)->getMemExports().size();
  }
  return 0;
}

uint32_t SSVM_StoreGetMemoryExports(const SSVM_StoreContext *Cxt, char **Names,
                                    SSVM_InstanceAddress *MemoryAddrs,
                                    const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromStoreCxt(Cxt)->getMemExports(), Names, MemoryAddrs, Len);
  }
  return 0;
}

uint32_t SSVM_StoreGetGlobalExportsLength(const SSVM_StoreContext *Cxt) {
  if (Cxt) {
    return fromStoreCxt(Cxt)->getGlobalExports().size();
  }
  return 0;
}

uint32_t SSVM_StoreGetGlobalExports(const SSVM_StoreContext *Cxt, char **Names,
                                    SSVM_InstanceAddress *GlobalAddrs,
                                    const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromStoreCxt(Cxt)->getGlobalExports(), Names, GlobalAddrs,
                   Len);
  }
  return 0;
}

SSVM_ModuleInstanceContext *SSVM_StoreFindModule(SSVM_StoreContext *Cxt,
                                                 const char *Name) {
  if (Cxt) {
    if (auto Res = fromStoreCxt(Cxt)->findModule(Name)) {
      return toModCxt(*Res);
    }
  }
  return nullptr;
}

void SSVM_StoreDelete(SSVM_StoreContext *Cxt) { deleteIf(fromStoreCxt(Cxt)); }

/// <<<<<<<< SSVM store functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM module instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

uint32_t SSVM_ModuleInstanceGetFunctionExportsLength(
    const SSVM_ModuleInstanceContext *Cxt) {
  if (Cxt) {
    return fromModCxt(Cxt)->getFuncExports().size();
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetFunctionExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *FuncAddrs, const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromModCxt(Cxt)->getFuncExports(), Names, FuncAddrs, Len);
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetTableExportsLength(
    const SSVM_ModuleInstanceContext *Cxt) {
  if (Cxt) {
    return fromModCxt(Cxt)->getTableExports().size();
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetTableExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *TableAddrs, const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromModCxt(Cxt)->getTableExports(), Names, TableAddrs, Len);
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetMemoryExportsLength(
    const SSVM_ModuleInstanceContext *Cxt) {
  if (Cxt) {
    return fromModCxt(Cxt)->getMemExports().size();
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetMemoryExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *MemoryAddrs, const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromModCxt(Cxt)->getMemExports(), Names, MemoryAddrs, Len);
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetGlobalExportsLength(
    const SSVM_ModuleInstanceContext *Cxt) {
  if (Cxt) {
    return fromModCxt(Cxt)->getGlobalExports().size();
  }
  return 0;
}

uint32_t SSVM_ModuleInstanceGetGlobalExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *GlobalAddrs, const uint32_t Len) {
  if (Cxt) {
    return fillMap(fromModCxt(Cxt)->getGlobalExports(), Names, GlobalAddrs,
                   Len);
  }
  return 0;
}

/// <<<<<<<< SSVM module instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM function type functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_FunctionTypeContext *SSVM_FunctionTypeCreate(
    const enum SSVM_ValType *ParamList, const uint32_t ParamLen,
    const enum SSVM_ValType *ReturnList, const uint32_t ReturnLen) {
  auto *Cxt = new SSVM::Runtime::Instance::FType;
  if (ParamLen > 0) {
    Cxt->Params.resize(ParamLen);
  }
  for (uint32_t I = 0; I < ParamLen; I++) {
    Cxt->Params[I] = static_cast<SSVM::ValType>(ParamList[I]);
  }
  if (ReturnLen > 0) {
    Cxt->Returns.resize(ReturnLen);
  }
  for (uint32_t I = 0; I < ReturnLen; I++) {
    Cxt->Params[I] = static_cast<SSVM::ValType>(ReturnList[I]);
  }
  return toFTypeCxt(Cxt);
}

uint32_t
SSVM_FunctionTypeGetParametersLength(const SSVM_FunctionTypeContext *Cxt) {
  if (Cxt) {
    return fromFTypeCxt(Cxt)->Params.size();
  }
  return 0;
}

uint32_t SSVM_FunctionTypeGetParameters(const SSVM_FunctionTypeContext *Cxt,
                                        SSVM_ValType *List,
                                        const uint32_t Len) {
  if (Cxt) {
    for (uint32_t I = 0; I < fromFTypeCxt(Cxt)->Params.size() && I < Len; I++) {
      List[I] = static_cast<SSVM_ValType>(fromFTypeCxt(Cxt)->Params[I]);
    }
    return fromFTypeCxt(Cxt)->Params.size();
  }
  return 0;
}

uint32_t
SSVM_FunctionTypeGetReturnsLength(const SSVM_FunctionTypeContext *Cxt) {
  if (Cxt) {
    return fromFTypeCxt(Cxt)->Returns.size();
  }
  return 0;
}

uint32_t SSVM_FunctionTypeGetReturns(const SSVM_FunctionTypeContext *Cxt,
                                     SSVM_ValType *List, const uint32_t Len) {
  if (Cxt) {
    for (uint32_t I = 0; I < fromFTypeCxt(Cxt)->Returns.size() && I < Len;
         I++) {
      List[I] = static_cast<SSVM_ValType>(fromFTypeCxt(Cxt)->Returns[I]);
    }
    return fromFTypeCxt(Cxt)->Returns.size();
  }
  return 0;
}

void SSVM_FunctionTypeDelete(SSVM_FunctionTypeContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM function type functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM function instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

const SSVM_FunctionTypeContext *
SSVM_FunctionInstanceGetFunctionType(const SSVM_FunctionInstanceContext *Cxt) {
  if (Cxt) {
    return toFTypeCxt(&fromFuncCxt(Cxt)->getFuncType());
  }
  return nullptr;
}

/// <<<<<<<< SSVM function instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM table instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_TableInstanceContext *
SSVM_TableInstanceCreate(const enum SSVM_RefType RefType,
                         const SSVM_Limit Limit) {
  SSVM::RefType Type = static_cast<SSVM::RefType>(RefType);
  if (Limit.HasMax) {
    return toTabCxt(new SSVM::Runtime::Instance::TableInstance(
        Type, SSVM::AST::Limit(Limit.Min, Limit.Max)));
  } else {
    return toTabCxt(new SSVM::Runtime::Instance::TableInstance(
        Type, SSVM::AST::Limit(Limit.Min)));
  }
}

enum SSVM_RefType
SSVM_TableInstanceGetRefType(const SSVM_TableInstanceContext *Cxt) {
  if (Cxt) {
    return static_cast<SSVM_RefType>(fromTabCxt(Cxt)->getReferenceType());
  }
  return SSVM_RefType_FuncRef;
}

SSVM_Result SSVM_TableInstanceGetData(const SSVM_TableInstanceContext *Cxt,
                                      SSVM_Value *Data, const uint32_t Offset) {
  return wrap([&]() { return fromTabCxt(Cxt)->getRefAddr(Offset); },
              [&](auto &&Res) { *Data = genSSVM_Value(*Res); }, Cxt, Data);
}

SSVM_Result SSVM_TableInstanceSetData(SSVM_TableInstanceContext *Cxt,
                                      SSVM_Value Data, const uint32_t Offset) {
  return wrap(
      [&]() {
        return fromTabCxt(Cxt)->setRefAddr(
            Offset, std::get<SSVM::RefVariant>(SSVM::ValVariant(Data.Value)));
      },
      EmptyThen, Cxt);
}

uint32_t SSVM_TableInstanceGetSize(const SSVM_TableInstanceContext *Cxt) {
  if (Cxt) {
    return fromTabCxt(Cxt)->getSize();
  }
  return 0;
}

SSVM_Result SSVM_TableInstanceGrow(SSVM_TableInstanceContext *Cxt,
                                   const uint32_t Size) {
  return wrap(
      [&]() -> SSVM::Expect<void> {
        if (fromTabCxt(Cxt)->growTable(Size)) {
          return {};
        } else {
          return SSVM::Unexpect(SSVM::ErrCode::TableOutOfBounds);
        }
      },
      EmptyThen, Cxt);
}

void SSVM_TableInstanceDelete(SSVM_TableInstanceContext *Cxt) {
  deleteIf(fromTabCxt(Cxt));
}

/// <<<<<<<< SSVM table instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM memory instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_MemoryInstanceContext *SSVM_MemoryInstanceCreate(const SSVM_Limit Limit) {
  if (Limit.HasMax) {
    return toMemCxt(new SSVM::Runtime::Instance::MemoryInstance(
        SSVM::AST::Limit(Limit.Min, Limit.Max)));
  } else {
    return toMemCxt(new SSVM::Runtime::Instance::MemoryInstance(
        SSVM::AST::Limit(Limit.Min)));
  }
}

SSVM_Result SSVM_MemoryInstanceGetData(const SSVM_MemoryInstanceContext *Cxt,
                                       uint8_t **Data, const uint32_t Offset,
                                       const uint32_t Length) {
  return wrap([&]() { return fromMemCxt(Cxt)->getBytes(Offset, Length); },
              [&](auto &&Res) { *Data = &(*Res)[0]; }, Cxt, Data);
}

SSVM_Result SSVM_MemoryInstanceSetData(SSVM_MemoryInstanceContext *Cxt,
                                       uint8_t *Data, const uint32_t Offset,
                                       const uint32_t Length) {

  return wrap(
      [&]() {
        return fromMemCxt(Cxt)->setBytes(genSpan(Data, Length), Offset, 0,
                                         Length);
      },
      EmptyThen, Cxt, Data);
}

uint32_t SSVM_MemoryInstanceGetPageSize(const SSVM_MemoryInstanceContext *Cxt) {
  if (Cxt) {
    return fromMemCxt(Cxt)->getDataPageSize();
  }
  return 0;
}

SSVM_Result SSVM_MemoryInstanceGrowPage(SSVM_MemoryInstanceContext *Cxt,
                                        const uint32_t Page) {

  return wrap(
      [&]() -> SSVM::Expect<void> {
        if (fromMemCxt(Cxt)->growPage(Page)) {
          return {};
        } else {
          return SSVM::Unexpect(SSVM::ErrCode::MemoryOutOfBounds);
        }
      },
      EmptyThen, Cxt);
}

void SSVM_MemoryInstanceDelete(SSVM_MemoryInstanceContext *Cxt) {
  deleteIf(fromMemCxt(Cxt));
}

/// <<<<<<<< SSVM memory instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM global instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_GlobalInstanceContext *
SSVM_GlobalInstanceCreate(const enum SSVM_ValType ValType,
                          const enum SSVM_Mutability Mut,
                          const SSVM_Value Value) {
  return toGlobCxt(new SSVM::Runtime::Instance::GlobalInstance(
      static_cast<SSVM::ValType>(ValType), static_cast<SSVM::ValMut>(Mut),
      Value.Value));
}

enum SSVM_ValType
SSVM_GlobalInstanceGetValType(const SSVM_GlobalInstanceContext *Cxt) {
  if (Cxt) {
    return static_cast<SSVM_ValType>(fromGlobCxt(Cxt)->getValType());
  }
  return SSVM_ValType_I32;
}

enum SSVM_Mutability
SSVM_GlobalInstanceGetMutability(const SSVM_GlobalInstanceContext *Cxt) {
  if (Cxt) {
    return static_cast<SSVM_Mutability>(fromGlobCxt(Cxt)->getValMut());
  }
  return SSVM_Mutability_Const;
}

SSVM_Value SSVM_GlobalInstanceGetValue(const SSVM_GlobalInstanceContext *Cxt) {
  if (Cxt) {
    return genSSVM_Value(fromGlobCxt(Cxt)->getValue());
  }
  return genSSVM_Value(0);
}

void SSVM_GlobalInstanceSetValue(SSVM_GlobalInstanceContext *Cxt,
                                 const SSVM_Value Value) {
  if (Cxt && fromGlobCxt(Cxt)->getValMut() == SSVM::ValMut::Var) {
    fromGlobCxt(Cxt)->getValue() = Value.Value;
  }
}

void SSVM_GlobalInstanceDelete(SSVM_GlobalInstanceContext *Cxt) {
  deleteIf(fromGlobCxt(Cxt));
}

/// <<<<<<<< SSVM global instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// <<<<<<<< SSVM import object functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SSVM_ImportObjectContext *SSVM_ImportObjectCreate(const char *ModuleName) {
  return toImpObjCxt(new SSVM::Runtime::ImportObject(ModuleName));
}

SSVM_ImportObjectContext *SSVM_ImportObjectCreateWASI(
    const char *const *Args, const uint32_t ArgLen, const char *const *Envs,
    const uint32_t EnvLen, const char *const *Dirs, const uint32_t DirLen,
    const char *const *Preopens, const uint32_t PreopenLen) {
  std::vector<std::string> ArgVec, EnvVec, DirVec;
  std::string ProgName;
  if (Args) {
    if (ArgLen > 0) {
      ProgName = Args[0];
    }
    for (uint32_t I = 1; I < ArgLen; I++) {
      ArgVec.emplace_back(Args[I]);
    }
  }
  if (Envs) {
    for (uint32_t I = 0; I < EnvLen; I++) {
      EnvVec.emplace_back(Envs[I]);
    }
  }
  if (Dirs) {
    for (uint32_t I = 0; I < DirLen; I++) {
      DirVec.emplace_back(Dirs[I]);
    }
  }
  if (Preopens) {
    for (uint32_t I = 0; I < PreopenLen; I++) {
      DirVec.emplace_back(std::string(Preopens[I]) + ":" +
                          std::string(Preopens[I]));
    }
  }
  auto *WasiMod = new SSVM::Host::WasiModule();
  auto &WasiEnv = WasiMod->getEnv();
  WasiEnv.init(DirVec, ProgName, ArgVec, EnvVec);
  return toImpObjCxt(WasiMod);
}

void SSVM_ImportObjectAddTable(SSVM_ImportObjectContext *Cxt, const char *Name,
                               SSVM_TableInstanceContext *TableCxt) {
  if (Cxt && TableCxt) {
    fromImpObjCxt(Cxt)->addHostTable(
        Name, std::unique_ptr<SSVM::Runtime::Instance::TableInstance>(
                  fromTabCxt(TableCxt)));
  }
}

void SSVM_ImportObjectAddMemory(SSVM_ImportObjectContext *Cxt, const char *Name,
                                SSVM_MemoryInstanceContext *MemoryCxt) {
  if (Cxt && MemoryCxt) {
    fromImpObjCxt(Cxt)->addHostMemory(
        Name, std::unique_ptr<SSVM::Runtime::Instance::MemoryInstance>(
                  fromMemCxt(MemoryCxt)));
  }
}

void SSVM_ImportObjectAddGlobal(SSVM_ImportObjectContext *Cxt, const char *Name,
                                SSVM_GlobalInstanceContext *GlobalCxt) {
  if (Cxt && GlobalCxt) {
    fromImpObjCxt(Cxt)->addHostGlobal(
        Name, std::unique_ptr<SSVM::Runtime::Instance::GlobalInstance>(
                  fromGlobCxt(GlobalCxt)));
  }
}

void SSVM_ImportObjectDelete(SSVM_ImportObjectContext *Cxt) {
  deleteIf(fromImpObjCxt(Cxt));
}

/// >>>>>>>> SSVM import object functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// >>>>>>>> SSVM VM functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SSVM_VMContext *SSVM_VMCreate(const SSVM_ConfigureContext *ConfCxt,
                              SSVM_StoreContext *StoreCxt) {
  if (ConfCxt) {
    if (StoreCxt) {
      return new SSVM_VMContext(ConfCxt->Conf, *fromStoreCxt(StoreCxt));
    } else {
      return new SSVM_VMContext(ConfCxt->Conf);
    }
  } else {
    if (StoreCxt) {
      return new SSVM_VMContext(SSVM::Configure(), *fromStoreCxt(StoreCxt));
    } else {
      return new SSVM_VMContext(SSVM::Configure());
    }
  }
}

SSVM_Result SSVM_VMRegisterModuleFromFile(SSVM_VMContext *Cxt,
                                          const char *ModuleName,
                                          const char *Path) {
  return wrap([&]() { return Cxt->VM.registerModule(ModuleName, Path); },
              EmptyThen, Cxt);
}

SSVM_Result SSVM_VMRegisterModuleFromBuffer(SSVM_VMContext *Cxt,
                                            const char *ModuleName,
                                            const uint8_t *Buf,
                                            const uint32_t BufLen) {
  return wrap(
      [&]() {
        return Cxt->VM.registerModule(ModuleName, genSpan(Buf, BufLen));
      },
      EmptyThen, Cxt);
}

SSVM_Result
SSVM_VMRegisterModuleFromImport(SSVM_VMContext *Cxt,
                                const SSVM_ImportObjectContext *ImportCxt) {
  return wrap(
      [&]() { return Cxt->VM.registerModule(*fromImpObjCxt(ImportCxt)); },
      EmptyThen, Cxt, ImportCxt);
}

SSVM_Result
SSVM_VMRegisterModuleFromASTModule(SSVM_VMContext *Cxt, const char *ModuleName,
                                   const SSVM_ASTModuleContext *ASTCxt) {
  return wrap(
      [&]() {
        return Cxt->VM.registerModule(ModuleName, *ASTCxt->Module.get());
      },
      EmptyThen, Cxt, ASTCxt);
}

SSVM_Result SSVM_VMRunWasmFromFile(SSVM_VMContext *Cxt, const char *Path,
                                   const char *FuncName,
                                   const SSVM_Value *Params,
                                   const uint32_t ParamLen, SSVM_Value *Returns,
                                   const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap([&]() { return Cxt->VM.runWasmFile(Path, FuncName, ParamVec); },
              [&](auto Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); },
              Cxt);
}

SSVM_Result
SSVM_VMRunWasmFromBuffer(SSVM_VMContext *Cxt, const uint8_t *Buf,
                         const uint32_t BufLen, const char *FuncName,
                         const SSVM_Value *Params, const uint32_t ParamLen,
                         SSVM_Value *Returns, const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap(
      [&]() {
        return Cxt->VM.runWasmFile(genSpan(Buf, BufLen), FuncName, ParamVec);
      },
      [&](auto &&Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); }, Cxt);
}

SSVM_Result SSVM_VMRunWasmFromASTModule(
    SSVM_VMContext *Cxt, const SSVM_ASTModuleContext *ASTCxt,
    const char *FuncName, const SSVM_Value *Params, const uint32_t ParamLen,
    SSVM_Value *Returns, const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap(
      [&]() {
        return Cxt->VM.runWasmFile(*ASTCxt->Module.get(), FuncName, ParamVec);
      },
      [&](auto &&Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); }, Cxt,
      ASTCxt);
}

SSVM_Result SSVM_VMLoadWasmFromFile(SSVM_VMContext *Cxt, const char *Path) {
  return wrap([&]() { return Cxt->VM.loadWasm(Path); }, EmptyThen, Cxt);
}

SSVM_Result SSVM_VMLoadWasmFromBuffer(SSVM_VMContext *Cxt, const uint8_t *Buf,
                                      const uint32_t BufLen) {
  return wrap([&]() { return Cxt->VM.loadWasm(genSpan(Buf, BufLen)); },
              EmptyThen, Cxt);
}

SSVM_Result SSVM_VMLoadWasmFromASTModule(SSVM_VMContext *Cxt,
                                         const SSVM_ASTModuleContext *ASTCxt) {
  return wrap([&]() { return Cxt->VM.loadWasm(*ASTCxt->Module.get()); },
              EmptyThen, Cxt, ASTCxt);
}

SSVM_Result SSVM_VMValidate(SSVM_VMContext *Cxt) {
  return wrap([&]() { return Cxt->VM.validate(); }, EmptyThen, Cxt);
}

SSVM_Result SSVM_VMInstantiate(SSVM_VMContext *Cxt) {
  return wrap([&]() { return Cxt->VM.instantiate(); }, EmptyThen, Cxt);
}

SSVM_Result SSVM_VMExecute(SSVM_VMContext *Cxt, const char *FuncName,
                           const SSVM_Value *Params, const uint32_t ParamLen,
                           SSVM_Value *Returns, const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap([&]() { return Cxt->VM.execute(FuncName, ParamVec); },
              [&](auto &&Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); },
              Cxt);
}

SSVM_Result
SSVM_VMExecuteRegistered(SSVM_VMContext *Cxt, const char *ModuleName,
                         const char *FuncName, const SSVM_Value *Params,
                         const uint32_t ParamLen, SSVM_Value *Returns,
                         const uint32_t ReturnLen) {
  auto ParamVec = genValVariantVec(Params, ParamLen);
  return wrap([&]() { return Cxt->VM.execute(ModuleName, FuncName, ParamVec); },
              [&](auto &&Res) { fillSSVM_ValueArr(*Res, Returns, ReturnLen); },
              Cxt);
}

void SSVM_VMCleanup(SSVM_VMContext *Cxt) {
  if (Cxt) {
    Cxt->VM.cleanup();
  }
}

uint32_t SSVM_VMGetFunctionListLength(SSVM_VMContext *Cxt) {
  if (Cxt) {
    return Cxt->VM.getFunctionList().size();
  }
  return 0;
}

uint32_t SSVM_VMGetFunctionList(SSVM_VMContext *Cxt, char **Names,
                                SSVM_FunctionTypeContext **FuncTypes,
                                const uint32_t Len) {
  if (Cxt) {
    auto FuncList = Cxt->VM.getFunctionList();
    if (Names && FuncTypes) {
      for (uint32_t I = 0; I < Len && I < FuncList.size(); I++) {
        size_t NameLen = FuncList[I].first.length();
        Names[I] = new char[Len + 1];
        std::strncpy(Names[I], FuncList[I].first.c_str(), NameLen);
        Names[I][NameLen] = '\0';
        FuncTypes[I] =
            toFTypeCxt(new SSVM::Runtime::Instance::FType(FuncList[I].second));
      }
    }
    return FuncList.size();
  }
  return 0;
}

SSVM_ImportObjectContext *
SSVM_VMGetImportModuleContext(SSVM_VMContext *Cxt,
                              const enum SSVM_HostRegistration Reg) {
  if (Cxt) {
    return toImpObjCxt(
        Cxt->VM.getImportModule(static_cast<SSVM::HostRegistration>(Reg)));
  }
  return nullptr;
}

SSVM_StoreContext *SSVM_VMGetStoreContext(SSVM_VMContext *Cxt) {
  if (Cxt) {
    return toStoreCxt(&Cxt->VM.getStoreManager());
  }
  return nullptr;
}

SSVM_StatisticsContext *SSVM_VMGetStatisticsContext(SSVM_VMContext *Cxt) {
  if (Cxt) {
    return toStatCxt(&Cxt->VM.getStatistics());
  }
  return nullptr;
}

void SSVM_VMDelete(SSVM_VMContext *Cxt) { deleteIf(Cxt); }

/// <<<<<<<< SSVM VM functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef __cplusplus
} /// extern "C"
#endif
