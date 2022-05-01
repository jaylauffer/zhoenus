// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
struct FOnlineServiceStatus;
enum class EImprobableAccountStatus : uint8;
#ifdef ONLINESERVICEAUTH_OnlineServiceAuthBPLibrary_generated_h
#error "OnlineServiceAuthBPLibrary.generated.h already included, missing '#pragma once' in OnlineServiceAuthBPLibrary.h"
#endif
#define ONLINESERVICEAUTH_OnlineServiceAuthBPLibrary_generated_h

#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_93_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FOnlineServiceStatus_Statics; \
	ONLINESERVICEAUTH_API static class UScriptStruct* StaticStruct();


template<> ONLINESERVICEAUTH_API UScriptStruct* StaticStruct<struct FOnlineServiceStatus>();

#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_48_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FDeployment_Statics; \
	ONLINESERVICEAUTH_API static class UScriptStruct* StaticStruct();


template<> ONLINESERVICEAUTH_API UScriptStruct* StaticStruct<struct FDeployment>();

#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_33_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FPlayerIdentityTokenDetails_Statics; \
	ONLINESERVICEAUTH_API static class UScriptStruct* StaticStruct();


template<> ONLINESERVICEAUTH_API UScriptStruct* StaticStruct<struct FPlayerIdentityTokenDetails>();

#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_115_DELEGATE \
struct _Script_OnlineServiceAuth_eventOnAccountLoginWithImprobableBP_Parms \
{ \
	FString AccountId; \
	FString PlayerIdentityToken; \
	FOnlineServiceStatus Status; \
	EImprobableAccountStatus AccountStatus; \
}; \
static inline void FOnAccountLoginWithImprobableBP_DelegateWrapper(const FScriptDelegate& OnAccountLoginWithImprobableBP, const FString& AccountId, const FString& PlayerIdentityToken, FOnlineServiceStatus const& Status, EImprobableAccountStatus const& AccountStatus) \
{ \
	_Script_OnlineServiceAuth_eventOnAccountLoginWithImprobableBP_Parms Parms; \
	Parms.AccountId=AccountId; \
	Parms.PlayerIdentityToken=PlayerIdentityToken; \
	Parms.Status=Status; \
	Parms.AccountStatus=AccountStatus; \
	OnAccountLoginWithImprobableBP.ProcessDelegate<UObject>(&Parms); \
}


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_SPARSE_DATA
#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execAccountLoginWithImprobableBP);


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execAccountLoginWithImprobableBP);


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUOnlineServiceAuthBPLibrary(); \
	friend struct Z_Construct_UClass_UOnlineServiceAuthBPLibrary_Statics; \
public: \
	DECLARE_CLASS(UOnlineServiceAuthBPLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/OnlineServiceAuth"), NO_API) \
	DECLARE_SERIALIZER(UOnlineServiceAuthBPLibrary)


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_INCLASS \
private: \
	static void StaticRegisterNativesUOnlineServiceAuthBPLibrary(); \
	friend struct Z_Construct_UClass_UOnlineServiceAuthBPLibrary_Statics; \
public: \
	DECLARE_CLASS(UOnlineServiceAuthBPLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/OnlineServiceAuth"), NO_API) \
	DECLARE_SERIALIZER(UOnlineServiceAuthBPLibrary)


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UOnlineServiceAuthBPLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UOnlineServiceAuthBPLibrary) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UOnlineServiceAuthBPLibrary); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UOnlineServiceAuthBPLibrary); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UOnlineServiceAuthBPLibrary(UOnlineServiceAuthBPLibrary&&); \
	NO_API UOnlineServiceAuthBPLibrary(const UOnlineServiceAuthBPLibrary&); \
public:


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UOnlineServiceAuthBPLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UOnlineServiceAuthBPLibrary(UOnlineServiceAuthBPLibrary&&); \
	NO_API UOnlineServiceAuthBPLibrary(const UOnlineServiceAuthBPLibrary&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UOnlineServiceAuthBPLibrary); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UOnlineServiceAuthBPLibrary); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UOnlineServiceAuthBPLibrary)


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_PRIVATE_PROPERTY_OFFSET
#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_119_PROLOG
#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_PRIVATE_PROPERTY_OFFSET \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_SPARSE_DATA \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_RPC_WRAPPERS \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_INCLASS \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_PRIVATE_PROPERTY_OFFSET \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_SPARSE_DATA \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_RPC_WRAPPERS_NO_PURE_DECLS \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_INCLASS_NO_PURE_DECLS \
	Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h_122_ENHANCED_CONSTRUCTORS \
static_assert(false, "Unknown access specifier for GENERATED_BODY() macro in class OnlineServiceAuthBPLibrary."); \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> ONLINESERVICEAUTH_API UClass* StaticClass<class UOnlineServiceAuthBPLibrary>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Zhoenus_Plugins_OnlineServiceAuth_OnlineServiceAuth_Source_OnlineServiceAuth_Public_OnlineServiceAuthBPLibrary_h


#define FOREACH_ENUM_EIMPROBABLEACCOUNTSTATUS(op) \
	op(EImprobableAccountStatus::Unknown) \
	op(EImprobableAccountStatus::Headless) \
	op(EImprobableAccountStatus::Unverified) \
	op(EImprobableAccountStatus::Verified) 

enum class EImprobableAccountStatus : uint8;
template<> ONLINESERVICEAUTH_API UEnum* StaticEnum<EImprobableAccountStatus>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
