// SPDX-License-Identifier: Apache-2.0
//===-- ssvm/api/ssvm.h - SSVM C API --------------------------------------===//
//
// Part of the SSVM Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the function declarations of SSVM C API.
///
//===----------------------------------------------------------------------===//

#ifndef __SSVM_C_API_H__
#define __SSVM_C_API_H__

#if defined(_WIN32)
#ifdef SSVM_COMPILE_LIBRARY
#define SSVM_CAPI_EXPORT __declspec(dllexport)
#else
#define SSVM_CAPI_EXPORT __declspec(dllimport)
#endif /// SSVM_COMPILE_LIBRARY
#else
#define SSVM_CAPI_EXPORT __attribute__((visibility("default")))
#endif /// _WIN32

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/// SSVM version.
#define SSVM_VERSION "0.7.3"
#define SSVM_VERSION_MAJOR 0
#define SSVM_VERSION_MINOR 7
#define SSVM_VERSION_PATCH 3

/// Opaque struct of WASM value.
typedef struct SSVM_Value {
  unsigned __int128 Value;
} SSVM_Value;

/// WASM Value type enumeration.
enum SSVM_ValType
#ifdef __cplusplus
    : uint32_t
#endif // __cplusplus
{ SSVM_ValType_I32 = 0x7FU,
  SSVM_ValType_I64 = 0x7EU,
  SSVM_ValType_F32 = 0x7DU,
  SSVM_ValType_F64 = 0x7CU,
  SSVM_ValType_V128 = 0x7BU,
  SSVM_ValType_FuncRef = 0x70U,
  SSVM_ValType_ExternRef = 0x6FU };

/// WASM Reference type enumeration.
enum SSVM_RefType
#ifdef __cplusplus
    : uint32_t
#endif // __cplusplus
{ SSVM_RefType_FuncRef = 0x70U,
  SSVM_RefType_ExternRef = 0x6FU };

/// WASM Mutability enumeration.
enum SSVM_Mutability
#ifdef __cplusplus
    : uint32_t
#endif // __cplusplus
{ SSVM_Mutability_Const = 0x00U,
  SSVM_Mutability_Var = 0x01U };

/// Opaque struct of WASM execution result.
typedef struct SSVM_Result {
  uint8_t Code;
} SSVM_Result;

/// Struct of WASM limit.
typedef struct SSVM_Limit {
  /// Boolean to describe has max value or not.
  bool HasMax;
  /// Minimum value.
  uint32_t Min;
  /// Maximum value. Will be ignored if the `HasMax` is false.
  uint32_t Max;
} SSVM_Limit;

/// Opaque struct of SSVM instance address.
typedef struct SSVM_InstanceAddress {
  uint32_t Addr;
} SSVM_InstanceAddress;

/// WASM Proposal enumeration.
enum SSVM_Proposal
#ifdef __cplusplus
    : uint32_t
#endif // __cplusplus
{ SSVM_Proposal_Annotations = 0,
  SSVM_Proposal_BulkMemoryOperations,
  SSVM_Proposal_ExceptionHandling,
  SSVM_Proposal_FunctionReferences,
  SSVM_Proposal_Memory64,
  SSVM_Proposal_ReferenceTypes,
  SSVM_Proposal_SIMD,
  SSVM_Proposal_TailCall,
  SSVM_Proposal_Threads,
  SSVM_Proposal_Max };

/// Host Module Registration enumeration.
enum SSVM_HostRegistration
#ifdef __cplusplus
    : uint32_t
#endif // __cplusplus
{ SSVM_HostRegistration_Wasi = 0,
  SSVM_HostRegistration_SSVM_Process,
  SSVM_HostRegistration_Max };

/// Opaque struct of SSVM configure.
typedef struct SSVM_ConfigureContext SSVM_ConfigureContext;

/// Opaque struct of SSVM statistics.
typedef struct SSVM_StatisticsContext {
} SSVM_StatisticsContext;

/// Opaque struct of SSVM AST module.
typedef struct SSVM_ASTModuleContext SSVM_ASTModuleContext;

/// Opaque struct of SSVM loader.
typedef struct SSVM_LoaderContext SSVM_LoaderContext;

/// Opaque struct of SSVM validator.
typedef struct SSVM_ValidatorContext SSVM_ValidatorContext;

/// Opaque struct of SSVM interpreter.
typedef struct SSVM_InterpreterContext SSVM_InterpreterContext;

/// Opaque struct of SSVM store.
typedef struct SSVM_StoreContext {
} SSVM_StoreContext;

/// Opaque struct of SSVM module instance.
typedef struct SSVM_ModuleInstanceContext {
} SSVM_ModuleInstanceContext;

/// Opaque struct of SSVM function type.
typedef struct SSVM_FunctionTypeContext {
} SSVM_FunctionTypeContext;

/// Opaque struct of SSVM function instance.
typedef struct SSVM_FunctionInstanceContext {
} SSVM_FunctionInstanceContext;

/// Opaque struct of SSVM table instance.
typedef struct SSVM_TableInstanceContext {
} SSVM_TableInstanceContext;

/// Opaque struct of SSVM memory instance.
typedef struct SSVM_MemoryInstanceContext {
} SSVM_MemoryInstanceContext;

/// Opaque struct of SSVM global instance.
typedef struct SSVM_GlobalInstanceContext {
} SSVM_GlobalInstanceContext;

/// Opaque struct of SSVM import object.
typedef struct SSVM_ImportObjectContext {
} SSVM_ImportObjectContext;

/// Opaque struct of SSVM VM.
typedef struct SSVM_VMContext SSVM_VMContext;

#ifdef __cplusplus
extern "C" {
#endif

/// >>>>>>>> SSVM version functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Get the version string of the SSVM C API.
///
/// The returned string must NOT be freed.
///
/// \returns NULL-terminated C string of version.
SSVM_CAPI_EXPORT extern const char *SSVM_VersionGet();

/// Get the major version value of the SSVM C API.
///
/// \returns Value of the major version.
SSVM_CAPI_EXPORT extern uint32_t SSVM_VersionGetMajor();

/// Get the minor version value of the SSVM C API.
///
/// \returns Value of the minor version.
SSVM_CAPI_EXPORT extern uint32_t SSVM_VersionGetMinor();

/// Get the patch version value of the SSVM C API.
///
/// \returns Value of the patch version.
SSVM_CAPI_EXPORT extern uint32_t SSVM_VersionGetPatch();

/// <<<<<<<< SSVM version functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM logging functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Set the logging system to filter to error level.
SSVM_CAPI_EXPORT extern void SSVM_LogSetErrorLevel();

/// Set the logging system to filter to debug level.
SSVM_CAPI_EXPORT extern void SSVM_LogSetDebugLevel();

/// <<<<<<<< SSVM logging functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM value functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Generate the I32 WASM value.
///
/// \param Val the I32 value.
///
/// \returns SSVM_Value struct with the I32 value.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenI32(const int32_t Val);

/// Generate the I64 WASM value.
///
/// \param Val the I64 value.
///
/// \returns SSVM_Value struct with the I64 value.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenI64(const int64_t Val);

/// Generate the F32 WASM value.
///
/// \param Val the F32 value.
///
/// \returns SSVM_Value struct with the F32 value.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenF32(const float Val);

/// Generate the F64 WASM value.
///
/// \param Val the F64 value.
///
/// \returns SSVM_Value struct with the F64 value.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenF64(const double Val);

/// Generate the V128 WASM value.
///
/// \param Val the V128 value.
///
/// \returns SSVM_Value struct with the V128 value.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenV128(const __int128 Val);

/// Generate the NULL reference WASM value.
///
/// The values generated by this function are only meaningful when the
/// `SSVM_Proposal_BulkMemoryOperations` or the `SSVM_Proposal_ReferenceTypes`
/// turns on in configuration.
///
/// \param T the reference type.
///
/// \returns SSVM_Value struct with the NULL reference.
SSVM_CAPI_EXPORT extern SSVM_Value
SSVM_ValueGenNullRef(const enum SSVM_RefType T);

/// Generate the function reference WASM value.
///
/// The values generated by this function are only meaningful when the
/// `SSVM_Proposal_BulkMemoryOperations` or the `SSVM_Proposal_ReferenceTypes`
/// turns on in configuration.
///
/// \param Index the function index.
///
/// \returns SSVM_Value struct with the function reference.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenFuncRef(const uint32_t Index);

/// Generate the function reference WASM value.
///
/// The values generated by this function are only meaningful when the
/// `SSVM_Proposal_ReferenceTypes` turns on in configuration.
///
/// \param Ref the reference to the external object.
///
/// \returns SSVM_Value struct with the external reference.
SSVM_CAPI_EXPORT extern SSVM_Value SSVM_ValueGenExternRef(void *Ref);

/// Retrieve the I32 value from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns I32 value in the input struct.
SSVM_CAPI_EXPORT extern int32_t SSVM_ValueGetI32(const SSVM_Value Val);

/// Retrieve the I64 value from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns I64 value in the input struct.
SSVM_CAPI_EXPORT extern int64_t SSVM_ValueGetI64(const SSVM_Value Val);

/// Retrieve the F32 value from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns F32 value in the input struct.
SSVM_CAPI_EXPORT extern float SSVM_ValueGetF32(const SSVM_Value Val);

/// Retrieve the F64 value from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns F64 value in the input struct.
SSVM_CAPI_EXPORT extern double SSVM_ValueGetF64(const SSVM_Value Val);

/// Retrieve the V128 value from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns V128 value in the input struct.
SSVM_CAPI_EXPORT extern __int128 SSVM_ValueGetV128(const SSVM_Value Val);

/// Retrieve the function index from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns function index in the input struct.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ValueGetFuncIdx(const SSVM_Value Val);

/// Retrieve the external reference from the WASM value.
///
/// \param Val the SSVM_Value struct.
///
/// \returns external reference in the input struct.
SSVM_CAPI_EXPORT extern void *SSVM_ValueGetExternRef(const SSVM_Value Val);

/// <<<<<<<< SSVM value functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM result functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Check the result is a success or not.
///
/// \param Res the SSVM_Result struct.
///
/// \returns true if the error code is ErrCode::Success or ErrCode::Terminated,
/// false for others.
SSVM_CAPI_EXPORT extern bool SSVM_ResultOK(const SSVM_Result Res);

/// Get the result code.
///
/// \param Res the SSVM_Result struct.
///
/// \returns corresponding result code.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ResultGetCode(const SSVM_Result Res);

/// Get the result message.
///
/// The returned string must NOT be freed.
///
/// \param Res the SSVM_Result struct.
///
/// \returns NULL-terminated C string of the corresponding error message.
SSVM_CAPI_EXPORT extern const char *
SSVM_ResultGetMessage(const SSVM_Result Res);

/// <<<<<<<< SSVM result functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM configure functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_ConfigureContext.
///
/// The caller owns the object and should call `SSVM_ConfigureDelete` to free
/// it.
///
/// \returns pointer to the context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_ConfigureContext *SSVM_ConfigureCreate();

/// Add a proposal setting into the SSVM_ConfigureContext.
///
/// For turning on a specific WASM proposal in SSVM_VMContext, you can set the
/// proposal value into the SSVM_ConfigureContext and create VM with this
/// context.
/// ```c
/// SSVM_ConfigureContext *Conf = SSVM_ConfigureCreate();
/// SSVM_ConfigureAddProposal(Conf, SSVM_Proposal_BulkMemoryOperations);
/// SSVM_ConfigureAddProposal(Conf, SSVM_Proposal_ReferenceTypes);
/// SSVM_ConfigureAddProposal(Conf, SSVM_Proposal_SIMD);
/// SSVM_VMContext *VM = SSVM_VMCreate(Conf);
/// ```
///
/// \param Cxt the SSVM_ConfigureContext to add the proposal value.
/// \param Prop the proposal value.
SSVM_CAPI_EXPORT extern void
SSVM_ConfigureAddProposal(SSVM_ConfigureContext *Cxt,
                          const enum SSVM_Proposal Prop);

/// Remove a proposal setting in the SSVM_ConfigureContext.
///
/// \param Cxt the SSVM_ConfigureContext to remove the proposal.
/// \param Prop the proposal value.
SSVM_CAPI_EXPORT extern void
SSVM_ConfigureRemoveProposal(SSVM_ConfigureContext *Cxt,
                             const enum SSVM_Proposal Prop);

/// Check if a proposal setting exists in the SSVM_ConfigureContext or not.
///
/// \param Cxt the SSVM_ConfigureContext to check the proposal value.
/// \param Prop the proposal value.
///
/// \returns true if the proposal setting exists, false if not.
SSVM_CAPI_EXPORT extern bool
SSVM_ConfigureHasProposal(const SSVM_ConfigureContext *Cxt,
                          const enum SSVM_Proposal Prop);

/// Add a host pre-registration setting into SSVM_ConfigureContext.
///
/// For turning on the Wasi support in SSVM_VMContext, you can set the host
/// pre-registration value into the SSVM_ConfigureContext and create VM with
/// this context.
/// ```c
/// SSVM_ConfigureContext *Conf = SSVM_ConfigureCreate();
/// SSVM_ConfigureAddHostRegistration(Conf, SSVM_HostRegistration_Wasi);
/// SSVM_VMContext *VM = SSVM_VMCreate(Conf);
/// ```
///
/// \param Cxt the SSVM_ConfigureContext to add host pre-registration.
/// \param Host the host pre-registration value.
SSVM_CAPI_EXPORT extern void
SSVM_ConfigureAddHostRegistration(SSVM_ConfigureContext *Cxt,
                                  const enum SSVM_HostRegistration Host);

/// Remove a host pre-registration setting in the SSVM_ConfigureContext.
///
/// \param Cxt the SSVM_ConfigureContext to remove the host pre-registration.
/// \param Prop the host pre-registration value.
SSVM_CAPI_EXPORT extern void
SSVM_ConfigureRemoveHostRegistration(SSVM_ConfigureContext *Cxt,
                                     const enum SSVM_HostRegistration Host);

/// Check if a host pre-registration setting exists in the SSVM_ConfigureContext
/// or not.
///
/// \param Cxt the SSVM_ConfigureContext to check the host pre-registration.
/// \param Prop the host pre-registration value.
///
/// \returns true if the host pre-registration setting exists, false if not.
SSVM_CAPI_EXPORT extern bool
SSVM_ConfigureHasHostRegistration(const SSVM_ConfigureContext *Cxt,
                                  const enum SSVM_HostRegistration Host);

/// Set the page limit of memory instances.
///
/// Limit the page count (64KiB per page) in memory instances.
///
/// \param Cxt the SSVM_ConfigureContext to set the maximum page count.
/// \param Page the maximum page count.
SSVM_CAPI_EXPORT extern void
SSVM_ConfigureSetMaxMemoryPage(SSVM_ConfigureContext *Cxt, const uint32_t Page);

/// Get the page limit of memory instances.
///
/// \param Cxt the SSVM_ConfigureContext to get the maximum page count setting.
///
/// \returns the page count limitation value.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_ConfigureGetMaxMemoryPage(const SSVM_ConfigureContext *Cxt);

/// Deletion of the SSVM_ConfigureContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_ConfigureContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_ConfigureDelete(SSVM_ConfigureContext *Cxt);

/// <<<<<<<< SSVM configure functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM statistics functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_StatisticsContext.
///
/// The caller owns the object and should call `SSVM_StatisticsDelete` to free
/// it.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_StatisticsContext *SSVM_StatisticsCreate();

/// Get the instruction count in execution.
///
/// \param Cxt the SSVM_StatisticsContext to get data.
///
/// \returns the instruction count in total execution.
SSVM_CAPI_EXPORT extern uint64_t
SSVM_StatisticsGetInstrCount(const SSVM_StatisticsContext *Cxt);

/// Get the instruction count per second in execution.
///
/// \param Cxt the SSVM_StatisticsContext to get data.
///
/// \returns the instruction count per second.
SSVM_CAPI_EXPORT extern double
SSVM_StatisticsGetInstrPerSecond(const SSVM_StatisticsContext *Cxt);

/// Get the total cost in execution.
///
/// \param Cxt the SSVM_StatisticsContext to get data.
///
/// \returns the total cost.
SSVM_CAPI_EXPORT extern uint64_t
SSVM_StatisticsGetTotalCost(const SSVM_StatisticsContext *Cxt);

/// Set the costs of instructions.
///
/// \param Cxt the SSVM_StatisticsContext to set the cost table.
/// \param CostArr the cost table array.
/// \param Len the length of the cost table array.
SSVM_CAPI_EXPORT extern void
SSVM_StatisticsSetCostTable(SSVM_StatisticsContext *Cxt, uint64_t *CostArr,
                            const uint32_t Len);

/// Set the cost limit in execution.
///
/// The WASM execution will be aborted if the instruction costs exceeded the
/// limit and the ErrCode::CostLimitExceeded will be returned.
///
/// \param Cxt the SSVM_StatisticsContext to set the cost table.
/// \param Limit the cost limit.
SSVM_CAPI_EXPORT extern void
SSVM_StatisticsSetCostLimit(SSVM_StatisticsContext *Cxt, const uint64_t Limit);

/// Deletion of the SSVM_StatisticsContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_StatisticsContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_StatisticsDelete(SSVM_StatisticsContext *Cxt);

/// <<<<<<<< SSVM statistics functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM AST module functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Deletion of the SSVM_ASTModuleContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_ASTModuleContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_ASTModuleDelete(SSVM_ASTModuleContext *Cxt);

/// <<<<<<<< SSVM AST module functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM loader functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_LoaderContext.
///
/// The caller owns the object and should call `SSVM_LoaderDelete` to free it.
///
/// \param ConfCxt the SSVM_ConfigureContext as the configuration of Loader.
/// NULL for the default configuration.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_LoaderContext *
SSVM_LoaderCreate(const SSVM_ConfigureContext *ConfCxt);

/// Load and parse the WASM module from a WASM file into SSVM_ASTModuleContext.
///
/// Load and parse the WASM module from the file path, and return a
/// SSVM_ASTModuleContext as result. The caller owns the SSVM_ASTModuleContext
/// object and should call `SSVM_ASTModuleDelete` to free it.
///
/// \param Cxt the SSVM_LoaderContext.
/// \param [out] Module the output SSVM_ASTModuleContext if succeeded.
/// \param Path the NULL-terminated C string of the WASM file path.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_LoaderParseFromFile(SSVM_LoaderContext *Cxt,
                         SSVM_ASTModuleContext **Module, const char *Path);

/// Load and parse the WASM module from a buffer into SSVM_ASTModuleContext.
///
/// Load and parse the WASM module from a buffer, and return a
/// SSVM_ASTModuleContext as result. The caller owns the SSVM_ASTModuleContext
/// object and should call `SSVM_ASTModuleDelete` to free it.
///
/// \param Cxt the SSVM_LoaderContext.
/// \param [out] Module the output SSVM_ASTModuleContext if succeeded.
/// \param Buf the buffer of WASM binary.
/// \param BufLen the length of the buffer.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_LoaderParseFromBuffer(SSVM_LoaderContext *Cxt,
                           SSVM_ASTModuleContext **Module, const uint8_t *Buf,
                           const uint32_t BufLen);

/// Deletion of the SSVM_LoaderContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_LoaderContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_LoaderDelete(SSVM_LoaderContext *Cxt);

/// <<<<<<<< SSVM loader functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM validator functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_ValidatorContext.
///
/// The caller owns the object and should call `SSVM_ValidatorDelete` to free
/// it.
///
/// \param ConfCxt the SSVM_ConfigureContext as the configuration of Validator.
/// NULL for the default configuration.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_ValidatorContext *
SSVM_ValidatorCreate(const SSVM_ConfigureContext *ConfCxt);

/// Validate the SSVM AST Module.
///
/// \param Cxt the SSVM_ValidatorContext.
/// \param ModuleCxt the SSVM_ASTModuleContext to validate.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_ValidatorValidate(SSVM_ValidatorContext *Cxt,
                       const SSVM_ASTModuleContext *ModuleCxt);

/// Deletion of the SSVM_ValidatorContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_ValidatorContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_ValidatorDelete(SSVM_ValidatorContext *Cxt);

/// <<<<<<<< SSVM validator functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM interpreter functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_InterpreterContext.
///
/// The caller owns the object and should call `SSVM_InterpreterDelete` to free
/// it.
///
/// \param ConfCxt the SSVM_ConfigureContext as the configuration of
/// Interpreter. NULL for the default configuration.
/// \param StatCxt the SSVM_StatisticsContext as the statistics object set into
/// Interpreter. The statistics will refer to this context, and the life cycle
/// should be ensured until the interpreter context is deleted. NULL for not
/// doing the statistics.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_InterpreterContext *
SSVM_InterpreterCreate(const SSVM_ConfigureContext *ConfCxt,
                       SSVM_StatisticsContext *StatCxt);

/// Instantiate SSVM AST Module into a store.
///
/// Instantiate the SSVM AST Module as an active anonymous module in store. You
/// can call `SSVM_StoreGetActiveModule` to retrieve the active module instance.
///
/// \param Cxt the SSVM_InterpreterContext to instantiate the module.
/// \param StoreCxt the SSVM_StoreContext to store the instantiated module.
/// \param ASTCxt the SSVM AST Module context generated by loader or compiler.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_InterpreterInstantiate(SSVM_InterpreterContext *Cxt,
                            SSVM_StoreContext *StoreCxt,
                            const SSVM_ASTModuleContext *ASTCxt);

/// Register and instantiate SSVM import object into a store.
///
/// Instantiate the instances in SSVM import object context and register into a
/// store with their exported name and the host module name.
///
/// \param Cxt the SSVM_InterpreterContext to instantiate the module.
/// \param StoreCxt the SSVM_StoreContext to store the instantiated module.
/// \param ImportCxt the SSVM_ImportObjectContext to register.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_InterpreterRegisterImport(SSVM_InterpreterContext *Cxt,
                               SSVM_StoreContext *StoreCxt,
                               const SSVM_ImportObjectContext *ImportCxt);

/// Register and instantiate SSVM AST Module into a store.
///
/// Instantiate the instances in SSVM AST Module and register into a store with
/// their exported name and module name.
///
/// \param Cxt the SSVM_InterpreterContext to instantiate the module.
/// \param StoreCxt the SSVM_StoreContext to store the instantiated module.
/// \param ASTCxt the SSVM AST Module context generated by loader or compiler.
/// \param ModuleName the NULL-terminated C string of module name for all
/// exported instances.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result SSVM_InterpreterRegisterModule(
    SSVM_InterpreterContext *Cxt, SSVM_StoreContext *StoreCxt,
    const SSVM_ASTModuleContext *ASTCxt, const char *ModuleName);

/// Invoke a WASM function by name.
///
/// After instantiating a WASM module, the WASM module is registered as the
/// anonymous module in the store context. Then you can repeatedly call this
/// function to invoke exported WASM functions by their names until the store
/// context is reset or a new WASM module is registered or instantiated. For
/// calling the functions in registered WASM modules with names in store, please
/// use `SSVM_InterpreterInvokeRegistered` instead.
///
/// \param Cxt the SSVM_InterpreterContext.
/// \param StoreCxt the SSVM_StoreContext which the module instantiated in.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_InterpreterInvoke(SSVM_InterpreterContext *Cxt,
                       SSVM_StoreContext *StoreCxt, const char *FuncName,
                       const SSVM_Value *Params, const uint32_t ParamLen,
                       SSVM_Value *Returns, const uint32_t ReturnLen);

/// Invoke a WASM function by its module name and function name.
///
/// After registering a WASM module, the WASM module is registered with its name
/// in the store context. Then you can repeatedly call this function to invoke
/// exported WASM functions by their module names and function names until the
/// store context is reset.
///
/// \param Cxt the SSVM_InterpreterContext.
/// \param StoreCxt the SSVM_StoreContext which the module instantiated in.
/// \param ModuleName the NULL-terminated C string of module name.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result SSVM_InterpreterInvokeRegistered(
    SSVM_InterpreterContext *Cxt, SSVM_StoreContext *StoreCxt,
    const char *ModuleName, const char *FuncName, const SSVM_Value *Params,
    const uint32_t ParamLen, SSVM_Value *Returns, const uint32_t ReturnLen);

/// Deletion of the SSVM_InterpreterContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_InterpreterContext to delete.
SSVM_CAPI_EXPORT extern void
SSVM_InterpreterDelete(SSVM_InterpreterContext *Cxt);

/// <<<<<<<< SSVM interpreter functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM store functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_StoreContext.
///
/// The caller owns the object and should call `SSVM_StoreDelete` to free
/// it.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_StoreContext *SSVM_StoreCreate();

/// Get the module instance context by the instance address.
///
/// The module instance context links to the module instance in the store
/// context and owned by the store context.
///
/// \param Cxt the SSVM_StoreContext.
/// \param Addr the module instance address in the store context.
///
/// \returns pointer to the module instance context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_ModuleInstanceContext *
SSVM_StoreGetModule(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr);

/// Get the function instance context by the instance address.
///
/// The function instance context links to the function instance in the store
/// context and owned by the store context.
///
/// \param Cxt the SSVM_StoreContext.
/// \param Addr the function instance address in the store context.
///
/// \returns pointer to the function instance context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_FunctionInstanceContext *
SSVM_StoreGetFunction(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr);

/// Get the table instance context by the instance address.
///
/// The table instance context links to the table instance in the store
/// context and owned by the store context.
///
/// \param Cxt the SSVM_StoreContext.
/// \param Addr the table instance address in the store context.
///
/// \returns pointer to the table instance context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_TableInstanceContext *
SSVM_StoreGetTable(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr);

/// Get the memory instance context by the instance address.
///
/// The memory instance context links to the memory instance in the store
/// context and owned by the store context.
///
/// \param Cxt the SSVM_StoreContext.
/// \param Addr the memory instance address in the store context.
///
/// \returns pointer to the memory instance context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_MemoryInstanceContext *
SSVM_StoreGetMemory(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr);

/// Get the global instance context by the instance address.
///
/// The global instance context links to the global instance in the store
/// context and owned by the store context.
///
/// \param Cxt the SSVM_StoreContext.
/// \param Addr the global instance address in the store context.
///
/// \returns pointer to the global instance context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_GlobalInstanceContext *
SSVM_StoreGetGlobal(SSVM_StoreContext *Cxt, const SSVM_InstanceAddress Addr);

/// Get the length of exported function list in store.
///
/// \param Cxt the SSVM_StoreContext.
///
/// \returns length of exported function list.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetFunctionExportsLength(const SSVM_StoreContext *Cxt);

/// Get the exported function list and the function address.
///
/// After instantiating a WASM module, the WASM module is registered into the
/// store context as an anonymous module. Then you can call this function to get
/// the exported function list of the anonymous module. If you want to get the
/// function list of registered named modules in store context, please call
/// `SSVM_StoreFindModule` to get the module instance context and then call
/// `SSVM_ModuleInstanceGetFunctionExports` instead. The returned function names
/// are allocated and stored in `Names` array, and the caller should call `free`
/// to free them. If the `Names` and `FuncAddrs` buffer lengths are smaller than
/// the result of the exported functions list size, the overflowed return values
/// will be discarded.
///
/// \param Cxt the SSVM_StoreContext.
/// \param [out] Names the output names buffer of exported functions.
/// \param [out] FuncAddrs the instance addresses buffer of function addresses.
/// \param Len the buffer length.
///
/// \returns actual exported function list size.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetFunctionExports(const SSVM_StoreContext *Cxt, char **Names,
                             SSVM_InstanceAddress *FuncAddrs,
                             const uint32_t Len);

/// Get the length of exported table list in store.
///
/// \param Cxt the SSVM_StoreContext.
///
/// \returns length of exported table list.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetTableExportsLength(const SSVM_StoreContext *Cxt);

/// Get the exported table list and the table address.
///
/// After instantiating a WASM module, the WASM module is registered into the
/// store context as an anonymous module. Then you can call this function to get
/// the exported table list of the anonymous module. If you want to get the
/// table list of registered named modules in store context, please call
/// `SSVM_StoreFindModule` to get the module instance context and then call
/// `SSVM_ModuleInstanceGetTableExports` instead. The returned table names are
/// allocated and stored in `Names` array, and the caller should call `free` to
/// free them. If the `Names` and `TableAddrs` buffer lengths are smaller than
/// the result of the exported table list size, the overflowed return values
/// will be discarded.
///
/// \param Cxt the SSVM_StoreContext.
/// \param [out] Names the output names buffer of exported tables.
/// \param [out] TableAddrs the instance addresses buffer of table addresses.
/// \param Len the buffer length.
///
/// \returns actual exported table list size.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetTableExports(const SSVM_StoreContext *Cxt, char **Names,
                          SSVM_InstanceAddress *TableAddrs, const uint32_t Len);

/// Get the length of exported memory list in store.
///
/// \param Cxt the SSVM_StoreContext.
///
/// \returns length of exported memory list.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetMemoryExportsLength(const SSVM_StoreContext *Cxt);

/// Get the exported memory list and the memory address.
///
/// After instantiating a WASM module, the WASM module is registered into the
/// store context as an anonymous module. Then you can call this function to get
/// the exported memory list of the anonymous module. If you want to get the
/// memory list of registered named modules in store context, please call
/// `SSVM_StoreFindModule` to get the module instance context and then call
/// `SSVM_ModuleInstanceGetMemoryExports` instead. The returned memory names are
/// allocated and stored in `Names` array, and the caller should call `free` to
/// free them. If the `Names` and `MemoryAddrs` buffer lengths are smaller than
/// the result of the exported memory list size, the overflowed return values
/// will be discarded.
///
/// \param Cxt the SSVM_StoreContext.
/// \param [out] Names the output names buffer of exported memories.
/// \param [out] MemoryAddrs the instance addresses buffer of memory addresses.
/// \param Len the buffer length.
///
/// \returns actual exported memory list size.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetMemoryExports(const SSVM_StoreContext *Cxt, char **Names,
                           SSVM_InstanceAddress *MemoryAddrs,
                           const uint32_t Len);

/// Get the length of exported global list in store.
///
/// \param Cxt the SSVM_StoreContext.
///
/// \returns length of exported global list.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetGlobalExportsLength(const SSVM_StoreContext *Cxt);

/// Get the exported global list and the global address.
///
/// After instantiating a WASM module, the WASM module is registered into the
/// store context as an anonymous module. Then you can call this function to get
/// the exported global list of the anonymous module. If you want to get the
/// global list of registered named modules in store context, please call
/// `SSVM_StoreFindModule` to get the module instance context and then call
/// `SSVM_ModuleInstanceGetGlobalExports` instead. The returned global names are
/// allocated and stored in `Names` array, and the caller should call `free` to
/// free them. If the `Names`, and `GlobalAddrs` buffer lengths are smaller than
/// the result of the exported global list size, the overflowed return values
/// will be discarded.
///
/// \param Cxt the SSVM_StoreContext.
/// \param [out] Names the output names buffer of exported globals.
/// \param [out] GlobalAddrs the instance addresses buffer of global addresses.
/// \param Len the buffer length.
///
/// \returns actual exported global list size.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_StoreGetGlobalExports(const SSVM_StoreContext *Cxt, char **Names,
                           SSVM_InstanceAddress *GlobalAddrs,
                           const uint32_t Len);

/// Find the module instance context by the module name.
///
/// The module instance context links to the module instance in the store
/// context and owned by the store context.
///
/// \param Cxt the SSVM_StoreContext.
/// \param Name the module name registered in the store context.
///
/// \returns pointer to the module instance context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_ModuleInstanceContext *
SSVM_StoreFindModule(SSVM_StoreContext *Cxt, const char *Name);

/// Deletion of the SSVM_StoreContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_StoreContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_StoreDelete(SSVM_StoreContext *Cxt);

/// <<<<<<<< SSVM store functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM module instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Get the length of exported function list in the module.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
///
/// \returns length of exported function list in the module.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetFunctionExportsLength(
    const SSVM_ModuleInstanceContext *Cxt);

/// Get the exported function list and the function address.
///
/// The returned function names are allocated and stored in `Names` array, and
/// the caller should call `free` to free them. If the `Names` and `FuncAddrs`
/// buffer lengths are smaller than the result of the exported functions list
/// size, the overflowed return values will be discarded.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
/// \param [out] Names the output names buffer of exported functions.
/// \param [out] FuncAddrs the instance addresses buffer of function addresses.
/// \param Len the buffer length.
///
/// \returns actual exported function list size.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetFunctionExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *FuncAddrs, const uint32_t Len);

/// Get the length of exported table list in the module.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
///
/// \returns length of exported table list in the module.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_ModuleInstanceGetTableExportsLength(const SSVM_ModuleInstanceContext *Cxt);

/// Get the exported table list and the table address.
///
/// The returned table names are allocated and stored in `Names` array, and the
/// caller should call `free` to free them. If the `Names` and `TableAddrs`
/// buffer lengths are smaller than the result of the exported table list size,
/// the overflowed return values will be discarded.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
/// \param [out] Names the output names buffer of exported tables.
/// \param [out] TableAddrs the instance addresses buffer of table addresses.
/// \param Len the buffer length.
///
/// \returns actual exported table list size.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetTableExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *TableAddrs, const uint32_t Len);

/// Get the length of exported memory list in the module.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
///
/// \returns length of exported memory list in the module.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetMemoryExportsLength(
    const SSVM_ModuleInstanceContext *Cxt);

/// Get the exported memory list and the memory address.
///
/// The returned memory names are allocated and stored in `Names` array, and the
/// caller should call `free` to free them. If the `Names` and `MemoryAddrs`
/// buffer lengths are smaller than the result of the exported memory list size,
/// the overflowed return values will be discarded.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
/// \param [out] Names the output names buffer of exported memories.
/// \param [out] MemoryAddrs the instance addresses buffer of memory addresses.
/// \param Len the buffer length.
///
/// \returns actual exported memory list size.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetMemoryExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *MemoryAddrs, const uint32_t Len);

/// Get the length of exported global list in in the module.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
///
/// \returns length of exported global list in the module.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetGlobalExportsLength(
    const SSVM_ModuleInstanceContext *Cxt);

/// Get the exported global list and the global address.
///
/// The returned global names are allocated and stored in `Names` array, and the
/// caller should call `free` to free them. If the `Names`, and `GlobalAddrs`
/// buffer lengths are smaller than the result of the exported global list size,
/// the overflowed return values will be discarded.
///
/// \param Cxt the SSVM_ModuleInstanceContext.
/// \param [out] Names the output names buffer of exported globals.
/// \param [out] GlobalAddrs the instance addresses buffer of global addresses.
/// \param Len the buffer length.
///
/// \returns actual exported global list size.
SSVM_CAPI_EXPORT extern uint32_t SSVM_ModuleInstanceGetGlobalExports(
    const SSVM_ModuleInstanceContext *Cxt, char **Names,
    SSVM_InstanceAddress *GlobalAddrs, const uint32_t Len);

/// <<<<<<<< SSVM module instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM function type functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_FunctionTypeContext.
///
/// The caller owns the object and should call `SSVM_FunctionTypeDelete` to free
/// it.
///
/// \param ParamList the value types list of parameters. NULL if the length is
/// 0.
/// \param ParamLen the ParamList buffer length.
/// \param ReturnList the value types list of returns. NULL if the length is 0.
/// \param ReturnLen the ReturnList buffer length.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_FunctionTypeContext *SSVM_FunctionTypeCreate(
    const enum SSVM_ValType *ParamList, const uint32_t ParamLen,
    const enum SSVM_ValType *ReturnList, const uint32_t ReturnLen);

/// Get the parameter types list length from the SSVM_FunctionTypeContext.
///
/// \param Cxt the SSVM_FunctionTypeContext.
///
/// \returns the parameter types list length.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_FunctionTypeGetParametersLength(const SSVM_FunctionTypeContext *Cxt);

/// Get the parameter types list from the SSVM_FunctionTypeContext.
///
/// If the `List` buffer length is smaller than the parameter types list's
/// length, the overflowed values will be discarded.
///
/// \param Cxt the SSVM_FunctionTypeContext.
/// \param [out] List the SSVM_ValType buffer to fill the parameter value types.
/// \param Len the value type buffer length.
///
/// \returns the actual parameter types list length.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_FunctionTypeGetParameters(const SSVM_FunctionTypeContext *Cxt,
                               enum SSVM_ValType *List, const uint32_t Len);

/// Get the return types list length from the SSVM_FunctionTypeContext.
///
/// \param Cxt the SSVM_FunctionTypeContext.
///
/// \returns the return types list length.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_FunctionTypeGetReturnsLength(const SSVM_FunctionTypeContext *Cxt);

/// Get the return types list from the SSVM_FunctionTypeContext.
///
/// If the `List` buffer length is smaller than the return types list's length,
/// the overflowed values will be discarded.
///
/// \param Cxt the SSVM_FunctionTypeContext.
/// \param [out] List the SSVM_ValType buffer to fill the return value types.
/// \param Len the value type buffer length.
///
/// \returns the actual return types list length.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_FunctionTypeGetReturns(const SSVM_FunctionTypeContext *Cxt,
                            enum SSVM_ValType *List, const uint32_t Len);

/// Deletion of the SSVM_FunctionTypeContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_FunctionTypeContext to delete.
SSVM_CAPI_EXPORT extern void
SSVM_FunctionTypeDelete(SSVM_FunctionTypeContext *Cxt);

/// <<<<<<<< SSVM function type functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM function instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Get the function type context of the function instance.
///
/// The function type context links to the function type in the function
/// instance context and owned by the context. The caller should NOT call the
/// `SSVM_FunctionTypeDelete`.
///
/// \param Cxt the SSVM_FunctionInstanceContext.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern const SSVM_FunctionTypeContext *
SSVM_FunctionInstanceGetFunctionType(const SSVM_FunctionInstanceContext *Cxt);

/// <<<<<<<< SSVM function instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM table instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_TableInstanceContext.
///
/// The caller owns the object and should call `SSVM_TableInstanceDelete` to
/// free it.
///
/// \param RefType the reference type of the table instance context.
/// \param Limit the limit struct to initialize the table instance context.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_TableInstanceContext *
SSVM_TableInstanceCreate(const enum SSVM_RefType RefType,
                         const SSVM_Limit Limit);

/// Get the reference type from a table instance.
///
/// \param Cxt the SSVM_TableInstanceContext.
///
/// \returns the reference type of the table instance.
SSVM_CAPI_EXPORT extern enum SSVM_RefType
SSVM_TableInstanceGetRefType(const SSVM_TableInstanceContext *Cxt);

/// Get the reference value in a table instance.
///
/// \param Cxt the SSVM_TableInstanceContext.
/// \param [out] Data the result reference value.
/// \param Offset the reference value offset (index) in the table instance.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_TableInstanceGetData(const SSVM_TableInstanceContext *Cxt,
                          SSVM_Value *Data, const uint32_t Offset);

/// Set the reference value into a table instance.
///
/// \param Cxt the SSVM_TableInstanceContext.
/// \param Data the reference value to set into the table instance.
/// \param Offset the reference value offset (index) in the table instance.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_TableInstanceSetData(SSVM_TableInstanceContext *Cxt, SSVM_Value Data,
                          const uint32_t Offset);

/// Get the size of a table instance.
///
/// \param Cxt the SSVM_TableInstanceContext.
///
/// \returns the size of the table instance.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_TableInstanceGetSize(const SSVM_TableInstanceContext *Cxt);

/// Grow a table instance with a size.
///
/// \param Cxt the SSVM_TableInstanceContext.
/// \param Size the count of reference values to grow in the table instance.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_TableInstanceGrow(SSVM_TableInstanceContext *Cxt, const uint32_t Size);

/// Deletion of the SSVM_TableInstanceContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_TableInstanceContext to delete.
SSVM_CAPI_EXPORT extern void
SSVM_TableInstanceDelete(SSVM_TableInstanceContext *Cxt);

/// <<<<<<<< SSVM table instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM memory instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_MemoryInstanceContext.
///
/// The caller owns the object and should call `SSVM_MemoryInstanceDelete` to
/// free it.
///
/// \param Limit the limit struct to initialize the memory instance context.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_MemoryInstanceContext *
SSVM_MemoryInstanceCreate(const SSVM_Limit Limit);

/// Get the data pointer in a memory instance.
///
/// \param Cxt the SSVM_MemoryInstanceContext.
/// \param [out] Data the result data pointer to the data with the offset.
/// \param Offset the data start offset in the memory instance.
/// \param Length the requested data length. If the `Offset + Length` is larger
/// than the data size in the memory instance, this function will failed.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_MemoryInstanceGetData(const SSVM_MemoryInstanceContext *Cxt,
                           uint8_t **Data, const uint32_t Offset,
                           const uint32_t Length);

/// Copy and set data into a memory instance.
///
/// \param Cxt the SSVM_MemoryInstanceContext.
/// \param Data the data buffer to copy.
/// \param Offset the data start offset in the memory instance.
/// \param Length the data buffer length. If the `Offset + Length` is larger
/// than the data size in the memory instance, this function will failed.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_MemoryInstanceSetData(SSVM_MemoryInstanceContext *Cxt, uint8_t *Data,
                           const uint32_t Offset, const uint32_t Length);

/// Get the current page size (64 KiB of each page) of a memory instance.
///
/// \param Cxt the SSVM_MemoryInstanceContext.
///
/// \returns the page size of the memory instance.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_MemoryInstanceGetPageSize(const SSVM_MemoryInstanceContext *Cxt);

/// Grow a memory instance with a page size.
///
/// \param Cxt the SSVM_MemoryInstanceContext.
/// \param Page the page count to grow in the memory instance.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_MemoryInstanceGrowPage(SSVM_MemoryInstanceContext *Cxt,
                            const uint32_t Page);

/// Deletion of the SSVM_MemoryInstanceContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_MemoryInstanceContext to delete.
SSVM_CAPI_EXPORT extern void
SSVM_MemoryInstanceDelete(SSVM_MemoryInstanceContext *Cxt);

/// <<<<<<<< SSVM memory instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// >>>>>>>> SSVM global instance functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_GlobalInstanceContext.
///
/// The caller owns the object and should call `SSVM_GlobalInstanceDelete` to
/// free it.
///
/// \param ValType the value type of the global instance.
/// \param Mut the mutation of the global instance.
/// \param Value the initial value of the global instance.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_GlobalInstanceContext *
SSVM_GlobalInstanceCreate(const enum SSVM_ValType ValType,
                          const enum SSVM_Mutability Mut,
                          const SSVM_Value Value);

/// Get the value type from a global instance.
///
/// \param Cxt the SSVM_GlobalInstanceContext.
///
/// \returns the value type of the global instance.
SSVM_CAPI_EXPORT extern enum SSVM_ValType
SSVM_GlobalInstanceGetValType(const SSVM_GlobalInstanceContext *Cxt);

/// Get the mutability from a global instance.
///
/// \param Cxt the SSVM_GlobalInstanceContext.
///
/// \returns the mutability of the global instance.
SSVM_CAPI_EXPORT extern enum SSVM_Mutability
SSVM_GlobalInstanceGetMutability(const SSVM_GlobalInstanceContext *Cxt);

/// Get the value from a global instance.
///
/// \param Cxt the SSVM_GlobalInstanceContext.
///
/// \returns the current value of the global instance.
SSVM_CAPI_EXPORT extern SSVM_Value
SSVM_GlobalInstanceGetValue(const SSVM_GlobalInstanceContext *Cxt);

/// Set the value from a global instance.
///
/// This function will do nothing if the global context is set as the `Const`
/// mutation.
///
/// \param Cxt the SSVM_GlobalInstanceContext.
/// \param Value the value to set into the global context.
SSVM_CAPI_EXPORT extern void
SSVM_GlobalInstanceSetValue(SSVM_GlobalInstanceContext *Cxt,
                            const SSVM_Value Value);

/// Deletion of the SSVM_GlobalInstanceContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_GlobalInstanceContext to delete.
SSVM_CAPI_EXPORT extern void
SSVM_GlobalInstanceDelete(SSVM_GlobalInstanceContext *Cxt);

/// <<<<<<<< SSVM global instance functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// <<<<<<<< SSVM import object functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/// Creation of the SSVM_ImportObjectContext.
///
/// The caller owns the object and should call `SSVM_ImportObjectDelete` to free
/// it.
///
/// \param ModuleName the module name of this host module to import.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_ImportObjectContext *
SSVM_ImportObjectCreate(const char *ModuleName);

/// Creation of the SSVM_ImportObjectContext for the WASI specification.
///
/// This function will create a WASI host module that contains the WASI host
/// functions and initialize it. The caller owns the object and should call
/// `SSVM_ImportObjectDelete` to free it.
///
/// \param Args the command line arguments. The first argument suggests being
/// the program name. NULL if the length is 0.
/// \param ArgLen the length of the command line arguments.
/// \param Envs the environment variables in the format `ENV=VALUE`. NULL if the
/// length is 0.
/// \param EnvLen the length of the environment variables.
/// \param Dirs the directory mappings in the format `PATH1:PATH2`. NULL if the
/// length is 0.
/// \param DirLen the length of the directory mappings.
/// \param Preopens the directory paths to preopen. NULL if the length is 0.
/// \param PreopenLen the length of the directory paths to preopen.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_ImportObjectContext *SSVM_ImportObjectCreateWASI(
    const char *const *Args, const uint32_t ArgLen, const char *const *Envs,
    const uint32_t EnvLen, const char *const *Dirs, const uint32_t DirLen,
    const char *const *Preopens, const uint32_t PreopenLen);

/// Add a table instance context into a SSVM_ImportObjectContext.
///
/// Move the table instance context into the import object. The caller should
/// NOT access or delete the table instance context after calling this function.
///
/// \param Cxt the SSVM_ImportObjectContext to add the table instance.
/// \param Name the export table name.
/// \param TableCxt the SSVM_TableInstanceContext to add.
SSVM_CAPI_EXPORT extern void
SSVM_ImportObjectAddTable(SSVM_ImportObjectContext *Cxt, const char *Name,
                          SSVM_TableInstanceContext *TableCxt);

/// Add a memory instance context into a SSVM_ImportObjectContext.
///
/// Move the memory instance context into the import object. The caller should
/// NOT access or delete the memory instance context after calling this
/// function.
///
/// \param Cxt the SSVM_ImportObjectContext to add the memory instance.
/// \param Name the export memory name.
/// \param MemoryCxt the SSVM_MemoryInstanceContext to add.
SSVM_CAPI_EXPORT extern void
SSVM_ImportObjectAddMemory(SSVM_ImportObjectContext *Cxt, const char *Name,
                           SSVM_MemoryInstanceContext *MemoryCxt);

/// Add a global instance context into a SSVM_ImportObjectContext.
///
/// Move the global instance context into the import object. The caller should
/// NOT access or delete the global instance context after calling this
/// function.
///
/// \param Cxt the SSVM_ImportObjectContext to add the global instance.
/// \param Name the export global name.
/// \param GlobalCxt the SSVM_GlobalInstanceContext to add.
SSVM_CAPI_EXPORT extern void
SSVM_ImportObjectAddGlobal(SSVM_ImportObjectContext *Cxt, const char *Name,
                           SSVM_GlobalInstanceContext *GlobalCxt);

/// Deletion of the SSVM_ImportObjectContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_ImportObjectContext to delete.
SSVM_CAPI_EXPORT extern void
SSVM_ImportObjectDelete(SSVM_ImportObjectContext *Cxt);

/// >>>>>>>> SSVM import object functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// >>>>>>>> SSVM VM functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/// Creation of the SSVM_VMContext.
///
/// The caller owns the object and should call `SSVM_VMDelete` to free it.
///
/// \param ConfCxt the SSVM_ConfigureContext as the configuration of VM. NULL
/// for the default configuration.
/// \param StoreCxt the SSVM_StoreContext as the external WASM store of VM. The
/// instantiation and execution will refer to this store context, and the life
/// cycle should be ensured until the VM context is deleted. NULL for the
/// default store owned by SSVM_VMContext.
///
/// \returns pointer to context, NULL if failed.
SSVM_CAPI_EXPORT extern SSVM_VMContext *
SSVM_VMCreate(const SSVM_ConfigureContext *ConfCxt,
              SSVM_StoreContext *StoreCxt);

/// Register and instantiate WASM into the store in VM from a WASM file.
///
/// Load a WASM file from the path, and register all exported instances and
/// instantiate them into the store in VM with their exported name and module
/// name.
///
/// \param Cxt the SSVM_VMContext which contains the store.
/// \param ModuleName the NULL-terminated C string of module name for all
/// exported instances.
/// \param Path the NULL-terminated C string of the WASM
/// file path.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMRegisterModuleFromFile(SSVM_VMContext *Cxt, const char *ModuleName,
                              const char *Path);

/// Register and instantiate WASM into the store in VM from a buffer.
///
/// Load a WASM module from a buffer, and register all exported instances and
/// instantiate them into the store in VM with their exported name and module
/// name.
///
/// \param Cxt the SSVM_VMContext which contains the store.
/// \param ModuleName the NULL-terminated C string of module name for all
/// exported instances.
/// \param Buf the buffer of WASM binary.
/// \param BufLen the length of the buffer.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMRegisterModuleFromBuffer(SSVM_VMContext *Cxt, const char *ModuleName,
                                const uint8_t *Buf, const uint32_t BufLen);

/// Register and instantiate SSVM import object into the store in VM.
///
/// Instantiate the instances in SSVM import object context and register them
/// into the store in VM with their exported name and the host module name.
///
/// \param Cxt the SSVM_VMContext which contains the store.
/// \param ImportCxt the SSVM_ImportObjectContext to register.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMRegisterModuleFromImport(SSVM_VMContext *Cxt,
                                const SSVM_ImportObjectContext *ImportCxt);

/// Register and instantiate WASM into the store in VM from a SSVM AST Module.
///
/// Load from the SSVM AST Module, and register all exported instances and
/// instantiate them into the store in VM with their exported name and module
/// name.
///
/// \param Cxt the SSVM_VMContext which contains the store.
/// \param ModuleName the NULL-terminated C string of module name for all
/// exported instances.
/// \param ASTCxt the SSVM AST Module context generated by loader or compiler.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMRegisterModuleFromASTModule(SSVM_VMContext *Cxt, const char *ModuleName,
                                   const SSVM_ASTModuleContext *ASTCxt);

/// Instantiate the WASM module from a WASM file and invoke a function by name.
///
/// This is the function to invoke a WASM function rapidly.
/// Load and instantiate the WASM module from the file path, and then invoke a
/// function by name and parameters. If the `Returns` buffer length is smaller
/// than the arity of the function, the overflowed return values will be
/// discarded.
///
/// \param Cxt the SSVM_VMContext.
/// \param Path the NULL-terminated C string of the WASM file path.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMRunWasmFromFile(SSVM_VMContext *Cxt, const char *Path,
                       const char *FuncName, const SSVM_Value *Params,
                       const uint32_t ParamLen, SSVM_Value *Returns,
                       const uint32_t ReturnLen);

/// Instantiate the WASM module from a buffer and invoke a function by name.
///
/// This is the function to invoke a WASM function rapidly.
/// Load and instantiate the WASM module from a buffer, and then invoke a
/// function by name and parameters. If the `Returns` buffer length is smaller
/// than the arity of the function, the overflowed return values will be
/// discarded.
///
/// \param Cxt the SSVM_VMContext.
/// \param Buf the buffer of WASM binary.
/// \param BufLen the length of the buffer.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMRunWasmFromBuffer(SSVM_VMContext *Cxt, const uint8_t *Buf,
                         const uint32_t BufLen, const char *FuncName,
                         const SSVM_Value *Params, const uint32_t ParamLen,
                         SSVM_Value *Returns, const uint32_t ReturnLen);

/// Instantiate the WASM module from a SSVM AST Module and invoke a function by
/// name.
///
/// This is the function to invoke a WASM function rapidly.
/// Load and instantiate the WASM module from the SSVM AST Module, and then
/// invoke the function by name and parameters. If the `Returns` buffer length
/// is smaller than the arity of the function, the overflowed return values will
/// be discarded.
///
/// \param Cxt the SSVM_VMContext.
/// \param ASTCxt the SSVM AST Module context generated by loader or compiler.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result SSVM_VMRunWasmFromASTModule(
    SSVM_VMContext *Cxt, const SSVM_ASTModuleContext *ASTCxt,
    const char *FuncName, const SSVM_Value *Params, const uint32_t ParamLen,
    SSVM_Value *Returns, const uint32_t ReturnLen);

/// Load the WASM module from a WASM file.
///
/// This is the first step to invoke a WASM function step by step.
/// Load and parse the WASM module from the file path. You can then call
/// `SSVM_VMValidate` for the next step.
///
/// \param Cxt the SSVM_VMContext.
/// \param Path the NULL-terminated C string of the WASM file path.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result SSVM_VMLoadWasmFromFile(SSVM_VMContext *Cxt,
                                                            const char *Path);

/// Load the WASM module from a buffer.
///
/// This is the first step to invoke a WASM function step by step.
/// Load and parse the WASM module from a buffer. You can then call
/// `SSVM_VMValidate` for the next step.
///
/// \param Cxt the SSVM_VMContext.
/// \param Buf the buffer of WASM binary.
/// \param BufLen the length of the buffer.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMLoadWasmFromBuffer(SSVM_VMContext *Cxt, const uint8_t *Buf,
                          const uint32_t BufLen);

/// Load the WASM module from loaded SSVM AST Module.
///
/// This is the first step to invoke a WASM function step by step.
/// Copy the loaded SSVM AST Module context into VM. The VM context has no
/// dependency on the input AST Module context. You can then call
/// `SSVM_VMValidate` for the next step.
///
/// \param Cxt the SSVM_VMContext.
/// \param ASTCxt the SSVM AST Module context generated by loader or compiler.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMLoadWasmFromASTModule(SSVM_VMContext *Cxt,
                             const SSVM_ASTModuleContext *ASTCxt);

/// Validate the WASM module loaded into the VM context.
///
/// This is the second step to invoke a WASM function step by step.
/// After loading a WASM module into VM context, You can call this function to
/// validate it. And you can then call `SSVM_VMInstantiate` for the next step.
/// Note that only validated WASM modules can be instantiated in the VM context.
///
/// \param Cxt the SSVM_VMContext.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result SSVM_VMValidate(SSVM_VMContext *Cxt);

/// Instantiate the validated WASM module in the VM context.
///
/// This is the third step to invoke a WASM function step by step.
/// After validating a WASM module in the VM context, You can call this function
/// to instantiate it. And you can then call `SSVM_VMExecute` for invoking the
/// exported function in this WASM module.
///
/// \param Cxt the SSVM_VMContext.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result SSVM_VMInstantiate(SSVM_VMContext *Cxt);

/// Invoke a WASM function by name.
///
/// This is the final step to invoke a WASM function step by step.
/// After instantiating a WASM module in the VM context, the WASM module is
/// registered into the store in the VM context as an anonymous module. Then you
/// can repeatedly call this function to invoke the exported WASM functions by
/// their names until the VM context is reset or a new WASM module is registered
/// or loaded. For calling the functions in registered WASM modules with module
/// names, please use `SSVM_VMExecuteRegistered` instead. If the `Returns`
/// buffer length is smaller than the arity of the function, the overflowed
/// return values will be discarded.
///
/// \param Cxt the SSVM_VMContext.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMExecute(SSVM_VMContext *Cxt, const char *FuncName,
               const SSVM_Value *Params, const uint32_t ParamLen,
               SSVM_Value *Returns, const uint32_t ReturnLen);

/// Invoke a WASM function by its module name and function name.
///
/// After registering a WASM module in the VM context, you can repeatedly call
/// this function to invoke exported WASM functions by their module names and
/// function names until the VM context is reset. If the `Returns` buffer length
/// is smaller than the arity of the function, the overflowed return values will
/// be discarded.
///
/// \param Cxt the SSVM_VMContext.
/// \param ModuleName the NULL-terminated C string of module name.
/// \param FuncName the NULL-terminated C string of function name.
/// \param Params the SSVM_Value buffer with the parameter values.
/// \param ParamLen the parameter buffer length.
/// \param [out] Returns the SSVM_Value buffer to fill the return values.
/// \param ReturnLen the return buffer length.
///
/// \returns SSVM_Result. Call `SSVM_ResultGetMessage` for the error message.
SSVM_CAPI_EXPORT extern SSVM_Result
SSVM_VMExecuteRegistered(SSVM_VMContext *Cxt, const char *ModuleName,
                         const char *FuncName, const SSVM_Value *Params,
                         const uint32_t ParamLen, SSVM_Value *Returns,
                         const uint32_t ReturnLen);

/// Reset of SSVM_VMContext.
///
/// After calling this function, the statistics, loaded module, and the
/// instances in the store except registered instances will be cleared.
///
/// \param Cxt the SSVM_VMContext to reset.
SSVM_CAPI_EXPORT extern void SSVM_VMCleanup(SSVM_VMContext *Cxt);

/// Get the length of exported function list.
///
/// \param Cxt the SSVM_VMContext.
///
/// \returns length of exported function list.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_VMGetFunctionListLength(SSVM_VMContext *Cxt);

/// Get the exported function list.
///
/// The returned function names are allocated and stored in `Names` array, and
/// the caller should call `free` to free them. The function type contexts of
/// the corresponding function names are allocated and stored in `FuncTypes`
/// array, and the caller should call `SSVM_FunctionTypeDelete` to free them. If
/// the `Names` and `FuncTypes` buffer lengths are smaller than the result of
/// the exported function list size, the overflowed return values will be
/// discarded.
///
/// \param Cxt the SSVM_VMContext.
/// \param [out] Names the output names buffer of exported functions.
/// \param [out] FuncTypes the function type contexts buffer.
/// \param Len the buffer length.
///
/// \returns actual exported function list size.
SSVM_CAPI_EXPORT extern uint32_t
SSVM_VMGetFunctionList(SSVM_VMContext *Cxt, char **Names,
                       SSVM_FunctionTypeContext **FuncTypes,
                       const uint32_t Len);

/// Get the import object corresponding to the SSVM_HostRegistration settings.
///
/// When creating the VM context with configuration, the host module will be
/// registered according to the SSVM_HostRegistration settings added into the
/// SSVM_ConfigureContext. You can call this function to get the
/// SSVM_ImportObjectContext corresponding to the settings. The import object
/// context links to the context owned by the VM context. The caller should NOT
/// call the `SSVM_ImportObjectDelete`.
///
/// ```c
/// SSVM_ConfigureContext *Conf = SSVM_ConfigureCreate();
/// SSVM_ConfigureAddHostRegistration(Conf, SSVM_HostRegistration_Wasi);
/// SSVM_ConfigureAddHostRegistration(Conf, SSVM_HostRegistration_SSVM_Process);
/// SSVM_VMContext *VM = SSVM_VMCreate(Conf);
/// SSVM_ImportObjectContext *WasiMod =
///     SSVM_VMGetImportModuleContext(VM, SSVM_HostRegistration_Wasi);
/// SSVM_ImportObjectContext *ProcessMod =
///     SSVM_VMGetImportModuleContext(VM, SSVM_HostRegistration_SSVM_Process);
/// ```
///
/// \param Cxt the SSVM_VMContext.
/// \param Reg the host registration value to get the import module.
///
/// \returns pointer to the import module context. NULL if not found.
SSVM_CAPI_EXPORT extern SSVM_ImportObjectContext *
SSVM_VMGetImportModuleContext(SSVM_VMContext *Cxt,
                              const enum SSVM_HostRegistration Reg);

/// Get the store context used in the SSVM_VMContext.
///
/// The store context links to the store in the VM context and owned by the VM
/// context. The caller should NOT call the `SSVM_StoreDelete`.
///
/// \param Cxt the SSVM_VMContext.
///
/// \returns pointer to the store context.
SSVM_CAPI_EXPORT extern SSVM_StoreContext *
SSVM_VMGetStoreContext(SSVM_VMContext *Cxt);

/// Get the statistics context used in the SSVM_VMContext.
///
/// The statistics context links to the statistics in the VM context and owned
/// by the VM context. The caller should NOT call the `SSVM_StatisticsDelete`.
///
/// \param Cxt the SSVM_VMContext.
///
/// \returns pointer to the statistics context.
SSVM_CAPI_EXPORT extern SSVM_StatisticsContext *
SSVM_VMGetStatisticsContext(SSVM_VMContext *Cxt);

/// Deletion of the SSVM_VMContext.
///
/// After calling this function, the context will be freed and should NOT be
/// used.
///
/// \param Cxt the SSVM_VMContext to delete.
SSVM_CAPI_EXPORT extern void SSVM_VMDelete(SSVM_VMContext *Cxt);

/// <<<<<<<< SSVM VM functions <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef __cplusplus
} /// extern "C"
#endif

#endif /// __SSVM_C_API_H__
