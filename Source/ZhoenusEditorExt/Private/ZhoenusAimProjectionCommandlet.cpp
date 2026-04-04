// Copyright 2026 Run Rong Games. All Rights Reserved.

#include "ZhoenusAimProjectionCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "MaterialEditingLibrary.h"
#include "Misc/PackageName.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Modules/ModuleManager.h"
#include "NiagaraEditorUtilities.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemFactoryNew.h"
#include "Engine/Texture.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZhoenusAimProjectionCommandlet)

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusAimProjectionCommandlet, Log, All);

namespace
{
	const TCHAR* const DefaultSystemObjectPath = TEXT("/Game/Effects/NS_AimProjection.NS_AimProjection");
	const TCHAR* const DefaultSystemPackagePath = TEXT("/Game/Effects/NS_AimProjection");
	const TCHAR* const DefaultMaterialObjectPath = TEXT("/Game/Textures/M_AimProjector.M_AimProjector");
	const TCHAR* const DefaultTextureObjectPath = TEXT("/Game/Textures/M_triangulation.M_triangulation");
	const TCHAR* const DefaultEmitterTemplatePath = TEXT("/Niagara/DefaultAssets/Templates/Emitters/SingleLoopingParticle.SingleLoopingParticle");

	bool SaveAssetPackage(UObject* Asset)
	{
		if (Asset == nullptr)
		{
			return false;
		}

		UPackage* const Package = Asset->GetOutermost();
		if (Package == nullptr)
		{
			return false;
		}

		const FString PackageName = Package->GetName();
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.Error = GError;

		return UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);
	}

	bool ConfigureReticleMaterial(UMaterial* Material, UTexture* ReticleTexture)
	{
		if (Material == nullptr || ReticleTexture == nullptr)
		{
			return false;
		}

		Material->Modify();
		if (UMaterialEditorOnlyData* const EditorOnlyData = Material->GetEditorOnlyData())
		{
			EditorOnlyData->Modify();
		}

		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);

		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Translucent;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;
		Material->bDisableDepthTest = false;

		bool bNeedsRecompile = false;
		UMaterialEditingLibrary::SetMaterialUsage(Material, MATUSAGE_NiagaraSprites, bNeedsRecompile);

		UMaterialExpressionTextureSample* const TextureSample = Cast<UMaterialExpressionTextureSample>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTextureSample::StaticClass(), -320, 0));
		if (TextureSample == nullptr)
		{
			return false;
		}

		TextureSample->Modify();
		TextureSample->Texture = ReticleTexture;
		TextureSample->AutoSetSampleType();

		UMaterialEditorOnlyData* const EditorOnlyData = Material->GetEditorOnlyData();
		if (EditorOnlyData == nullptr)
		{
			return false;
		}

		EditorOnlyData->BaseColor.Expression = nullptr;
		EditorOnlyData->Metallic.Expression = nullptr;
		EditorOnlyData->Specular.Expression = nullptr;
		EditorOnlyData->Roughness.Expression = nullptr;
		EditorOnlyData->Normal.Expression = nullptr;
		EditorOnlyData->EmissiveColor.Expression = TextureSample;
		EditorOnlyData->Opacity.Expression = TextureSample;
		EditorOnlyData->OpacityMask.Expression = nullptr;

		const TArray<FExpressionOutput> Outputs = TextureSample->GetOutputs();
		if (!Outputs.IsEmpty())
		{
			const FExpressionOutput& DefaultOutput = Outputs[0];
			EditorOnlyData->EmissiveColor.Mask = DefaultOutput.Mask;
			EditorOnlyData->EmissiveColor.MaskR = DefaultOutput.MaskR;
			EditorOnlyData->EmissiveColor.MaskG = DefaultOutput.MaskG;
			EditorOnlyData->EmissiveColor.MaskB = DefaultOutput.MaskB;
			EditorOnlyData->EmissiveColor.MaskA = DefaultOutput.MaskA;

			EditorOnlyData->Opacity.Mask = DefaultOutput.Mask;
			EditorOnlyData->Opacity.MaskR = 0;
			EditorOnlyData->Opacity.MaskG = 0;
			EditorOnlyData->Opacity.MaskB = 0;
			EditorOnlyData->Opacity.MaskA = 1;
		}

		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->MarkPackageDirty();
		return true;
	}
}

UZhoenusAimProjectionCommandlet::UZhoenusAimProjectionCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 UZhoenusAimProjectionCommandlet::Main(const FString& Params)
{
	check(GEditor);

	FString SystemObjectPath = DefaultSystemObjectPath;
	FString SystemPackagePath = DefaultSystemPackagePath;
	FString MaterialObjectPath = DefaultMaterialObjectPath;
	FString TextureObjectPath = DefaultTextureObjectPath;
	FString TemplateEmitterPath = DefaultEmitterTemplatePath;

	FParse::Value(*Params, TEXT("System="), SystemObjectPath);
	FParse::Value(*Params, TEXT("Package="), SystemPackagePath);
	FParse::Value(*Params, TEXT("Material="), MaterialObjectPath);
	FParse::Value(*Params, TEXT("Texture="), TextureObjectPath);
	FParse::Value(*Params, TEXT("Template="), TemplateEmitterPath);

	UE_LOG(LogZhoenusAimProjectionCommandlet, Log, TEXT("Preparing aim projection system %s using material %s."),
		*SystemObjectPath, *MaterialObjectPath);

	UMaterial* const AimMaterial = LoadObject<UMaterial>(nullptr, *MaterialObjectPath);
	if (AimMaterial == nullptr)
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to load reticle material %s."), *MaterialObjectPath);
		return 1;
	}

	UTexture* const ReticleTexture = LoadObject<UTexture>(nullptr, *TextureObjectPath);
	if (ReticleTexture == nullptr)
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to load reticle texture %s."), *TextureObjectPath);
		return 1;
	}

	if (!ConfigureReticleMaterial(AimMaterial, ReticleTexture))
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to configure reticle material %s."), *MaterialObjectPath);
		return 1;
	}

	if (!AimMaterial->CheckMaterialUsage(MATUSAGE_NiagaraSprites))
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Material %s is not valid for Niagara sprites."), *MaterialObjectPath);
		return 1;
	}

	UNiagaraEmitter* const TemplateEmitter = LoadObject<UNiagaraEmitter>(nullptr, *TemplateEmitterPath);
	if (TemplateEmitter == nullptr)
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to load Niagara emitter template %s."), *TemplateEmitterPath);
		return 1;
	}

	UPackage* const Package = CreatePackage(*SystemPackagePath);
	if (Package == nullptr)
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to create package %s."), *SystemPackagePath);
		return 1;
	}

	Package->FullyLoad();

	const FString AssetName = FPackageName::GetLongPackageAssetName(SystemPackagePath);
	if (AssetName.IsEmpty())
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to derive an asset name from package %s."), *SystemPackagePath);
		return 1;
	}

	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemObjectPath);
	const bool bCreatedNewAsset = (System == nullptr);
	if (System == nullptr)
	{
		System = NewObject<UNiagaraSystem>(Package, FName(*AssetName), RF_Public | RF_Standalone | RF_Transactional);
		if (System == nullptr)
		{
			UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Unable to create Niagara system asset."));
			return 1;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(System, true);
		FAssetRegistryModule::AssetCreated(System);
	}

	System->Modify();
	while (System->GetEmitterHandles().Num() > 0)
	{
		const FNiagaraEmitterHandle HandleToRemove = System->GetEmitterHandles()[0];
		System->RemoveEmitterHandle(HandleToRemove);
	}

	FNiagaraEditorUtilities::AddEmitterToSystem(*System, *TemplateEmitter, TemplateEmitter->GetExposedVersion().VersionGuid);

	bool bPatchedRenderer = false;
	for (FNiagaraEmitterHandle& EmitterHandle : System->GetEmitterHandles())
	{
		if (FVersionedNiagaraEmitterData* const EmitterData = EmitterHandle.GetEmitterData())
		{
			EmitterData->bLocalSpace = false;

			for (UNiagaraRendererProperties* const Renderer : EmitterData->GetRenderers())
			{
				UNiagaraSpriteRendererProperties* const SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderer);
				if (SpriteRenderer == nullptr)
				{
					continue;
				}

				SpriteRenderer->Modify();
				SpriteRenderer->Material = AimMaterial;
				SpriteRenderer->MaterialUserParamBinding = FNiagaraUserParameterBinding();
				SpriteRenderer->Alignment = ENiagaraSpriteAlignment::Unaligned;
				SpriteRenderer->FacingMode = ENiagaraSpriteFacingMode::FaceCameraPlane;
				SpriteRenderer->PivotInUVSpace = FVector2D(0.5f, 0.5f);
				bPatchedRenderer = true;
			}
		}
	}

	if (!bPatchedRenderer)
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("No Niagara sprite renderer was found to patch in %s."), *SystemObjectPath);
		return 1;
	}

	System->RequestCompile(false);
	System->WaitForCompilationComplete();
	System->MarkPackageDirty();
	Package->MarkPackageDirty();

	if (!SaveAssetPackage(System))
	{
		UE_LOG(LogZhoenusAimProjectionCommandlet, Error, TEXT("Failed to save %s."), *SystemObjectPath);
		return 1;
	}

	if (AimMaterial->GetOutermost() != nullptr && AimMaterial->GetOutermost()->IsDirty())
	{
		if (!SaveAssetPackage(AimMaterial))
		{
			UE_LOG(LogZhoenusAimProjectionCommandlet, Warning, TEXT("Failed to save updated material usage on %s."), *MaterialObjectPath);
		}
	}

	UE_LOG(LogZhoenusAimProjectionCommandlet, Log, TEXT("%s aim projection asset %s."),
		bCreatedNewAsset ? TEXT("Created") : TEXT("Updated"), *SystemObjectPath);

	return 0;
}
