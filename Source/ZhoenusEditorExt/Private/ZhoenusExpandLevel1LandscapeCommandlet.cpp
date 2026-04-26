// Copyright 2026 Run Rong Games. All Rights Reserved.

#include "ZhoenusExpandLevel1LandscapeCommandlet.h"

#include "Editor.h"
#include "FileHelpers.h"
#include "Landscape.h"
#include "LandscapeConfigHelper.h"
#include "LandscapeEdit.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Misc/Parse.h"
#include "EngineUtils.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZhoenusExpandLevel1LandscapeCommandlet)

DEFINE_LOG_CATEGORY_STATIC(LogZhoenusExpandLevel1LandscapeCommandlet, Log, All);

namespace
{
	struct FLandscapeLayerCopySpec
	{
		FName LayerName = NAME_None;
		ULandscapeLayerInfoObject* LayerInfo = nullptr;
	};

	void LogLandscapeSummary(ALandscape* Landscape, ULandscapeInfo* LandscapeInfo, const int32 MinX, const int32 MinY, const int32 MaxX, const int32 MaxY)
	{
		const int32 ComponentSizeQuads = Landscape->ComponentSizeQuads;
		const int32 ComponentCountX = (MaxX - MinX) / ComponentSizeQuads;
		const int32 ComponentCountY = (MaxY - MinY) / ComponentSizeQuads;
		const FVector Location = Landscape->GetActorLocation();
		const FVector Scale = Landscape->GetActorScale3D();

		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log,
			TEXT("Landscape %s label=%s location=%s scale=%s"),
			*Landscape->GetName(),
			*Landscape->GetActorLabel(),
			*Location.ToString(),
			*Scale.ToString());
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log,
			TEXT("Landscape extent min=(%d,%d) max=(%d,%d) component_size_quads=%d component_count=(%d,%d)"),
			MinX,
			MinY,
			MaxX,
			MaxY,
			ComponentSizeQuads,
			ComponentCountX,
			ComponentCountY);
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log,
			TEXT("Landscape subsections=%d subsection_size_quads=%d layers=%d"),
			Landscape->NumSubsections,
			Landscape->SubsectionSizeQuads,
			LandscapeInfo->Layers.Num());

		for (const FLandscapeInfoLayerSettings& LayerSettings : LandscapeInfo->Layers)
		{
			UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log,
				TEXT("Layer name=%s info=%s"),
				*LayerSettings.GetLayerName().ToString(),
				LayerSettings.LayerInfoObj ? *LayerSettings.LayerInfoObj->GetName() : TEXT("<none>"));
		}
	}

	bool ExpandLandscapeData(
		ALandscape* Landscape,
		ULandscapeInfo* LandscapeInfo,
		const FIntRect& OldRegion,
		const FIntRect& NewRegion,
		TArray<uint16>& OutExpandedHeightData,
		TArray<FLandscapeImportLayerInfo>& OutExpandedLayerImports,
		TArray<FLandscapeLayerCopySpec>& OutLayerSpecs)
	{
		FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

		const int32 OldSizeX = OldRegion.Width() + 1;
		const int32 OldSizeY = OldRegion.Height() + 1;
		TArray<uint16> OldHeightData;
		OldHeightData.SetNumUninitialized(OldSizeX * OldSizeY);
		LandscapeEdit.GetHeightDataFast(
			OldRegion.Min.X,
			OldRegion.Min.Y,
			OldRegion.Max.X,
			OldRegion.Max.Y,
			OldHeightData.GetData(),
			0);

		FLandscapeConfigHelper::ExpandData<uint16>(OldHeightData, OutExpandedHeightData, OldRegion, NewRegion, true);

		OutExpandedLayerImports.Reset();
		OutLayerSpecs.Reset();

		for (const FLandscapeInfoLayerSettings& LayerSettings : LandscapeInfo->Layers)
		{
			FLandscapeLayerCopySpec& LayerSpec = OutLayerSpecs.AddDefaulted_GetRef();
			LayerSpec.LayerName = LayerSettings.GetLayerName();
			LayerSpec.LayerInfo = LayerSettings.LayerInfoObj;

			if (LayerSettings.LayerInfoObj == nullptr)
			{
				continue;
			}

			TArray<uint8> OldWeightData;
			OldWeightData.SetNumUninitialized(OldSizeX * OldSizeY);
			LandscapeEdit.GetWeightDataFast(
				LayerSettings.LayerInfoObj,
				OldRegion.Min.X,
				OldRegion.Min.Y,
				OldRegion.Max.X,
				OldRegion.Max.Y,
				OldWeightData.GetData(),
				0);

			TArray<uint8> ExpandedWeightData;
			FLandscapeConfigHelper::ExpandData<uint8>(OldWeightData, ExpandedWeightData, OldRegion, NewRegion, true);

			FLandscapeImportLayerInfo& ImportLayer = OutExpandedLayerImports.AddDefaulted_GetRef();
			ImportLayer.LayerName = LayerSpec.LayerName;
			ImportLayer.LayerInfo = LayerSpec.LayerInfo;
			ImportLayer.LayerData = MoveTemp(ExpandedWeightData);
		}

		return true;
	}
}

UZhoenusExpandLevel1LandscapeCommandlet::UZhoenusExpandLevel1LandscapeCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 UZhoenusExpandLevel1LandscapeCommandlet::Main(const FString& Params)
{
	check(GEditor);

	FString LevelAssetPath = TEXT("/Game/Map/Level-1");
	FString LandscapeNameFilter;
	int32 PaddingComponents = 2;
	int32 PaddingComponentsX = PaddingComponents;
	int32 PaddingComponentsY = PaddingComponents;
	int32 PaddingComponentsWest = PaddingComponentsX;
	int32 PaddingComponentsEast = PaddingComponentsX;
	int32 PaddingComponentsSouth = PaddingComponentsY;
	int32 PaddingComponentsNorth = PaddingComponentsY;

	FParse::Value(*Params, TEXT("Level="), LevelAssetPath);
	FParse::Value(*Params, TEXT("Landscape="), LandscapeNameFilter);
	FParse::Value(*Params, TEXT("PaddingComponents="), PaddingComponents);
	PaddingComponentsX = PaddingComponents;
	PaddingComponentsY = PaddingComponents;
	FParse::Value(*Params, TEXT("PaddingComponentsX="), PaddingComponentsX);
	FParse::Value(*Params, TEXT("PaddingComponentsY="), PaddingComponentsY);
	PaddingComponentsWest = PaddingComponentsX;
	PaddingComponentsEast = PaddingComponentsX;
	PaddingComponentsSouth = PaddingComponentsY;
	PaddingComponentsNorth = PaddingComponentsY;
	FParse::Value(*Params, TEXT("PaddingComponentsWest="), PaddingComponentsWest);
	FParse::Value(*Params, TEXT("PaddingComponentsEast="), PaddingComponentsEast);
	FParse::Value(*Params, TEXT("PaddingComponentsSouth="), PaddingComponentsSouth);
	FParse::Value(*Params, TEXT("PaddingComponentsNorth="), PaddingComponentsNorth);

	const bool bDryRun = FParse::Param(*Params, TEXT("DryRun"));

	if (PaddingComponentsWest < 0 || PaddingComponentsEast < 0 || PaddingComponentsSouth < 0 || PaddingComponentsNorth < 0)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error,
			TEXT("Padding components must be non-negative. Got west=%d east=%d south=%d north=%d."),
			PaddingComponentsWest,
			PaddingComponentsEast,
			PaddingComponentsSouth,
			PaddingComponentsNorth);
		return 1;
	}

	UWorld* const World = UEditorLoadingAndSavingUtils::LoadMap(LevelAssetPath);
	if (World == nullptr)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error, TEXT("Failed to load map %s."), *LevelAssetPath);
		return 1;
	}

	ALandscape* SourceLandscape = nullptr;
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		ALandscape* const Candidate = *It;
		if (!IsValid(Candidate))
		{
			continue;
		}

		if (LandscapeNameFilter.IsEmpty()
			|| Candidate->GetName() == LandscapeNameFilter
			|| Candidate->GetActorLabel() == LandscapeNameFilter)
		{
			SourceLandscape = Candidate;
			break;
		}
	}

	if (SourceLandscape == nullptr)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error,
			TEXT("No landscape found in %s matching '%s'."),
			*LevelAssetPath,
			*LandscapeNameFilter);
		return 1;
	}

	ULandscapeInfo* const SourceLandscapeInfo = SourceLandscape->GetLandscapeInfo();
	if (SourceLandscapeInfo == nullptr)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error,
			TEXT("Landscape %s has no landscape info."), *SourceLandscape->GetName());
		return 1;
	}

	int32 MinX = 0;
	int32 MinY = 0;
	int32 MaxX = 0;
	int32 MaxY = 0;
	if (!SourceLandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error,
			TEXT("Landscape %s has no valid extent."), *SourceLandscape->GetName());
		return 1;
	}

	LogLandscapeSummary(SourceLandscape, SourceLandscapeInfo, MinX, MinY, MaxX, MaxY);

	const int32 ComponentSizeQuads = SourceLandscape->ComponentSizeQuads;
	const int32 OldComponentCountX = (MaxX - MinX) / ComponentSizeQuads;
	const int32 OldComponentCountY = (MaxY - MinY) / ComponentSizeQuads;
	const int32 NewComponentCountX = OldComponentCountX + PaddingComponentsWest + PaddingComponentsEast;
	const int32 NewComponentCountY = OldComponentCountY + PaddingComponentsSouth + PaddingComponentsNorth;

	const FIntRect OldRegion(MinX, MinY, MaxX, MaxY);
	const FIntRect NewRegion(
		MinX - PaddingComponentsWest * ComponentSizeQuads,
		MinY - PaddingComponentsSouth * ComponentSizeQuads,
		MaxX + PaddingComponentsEast * ComponentSizeQuads,
		MaxY + PaddingComponentsNorth * ComponentSizeQuads);

	UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log,
		TEXT("Requested padding west=%d east=%d south=%d north=%d -> new component count=(%d,%d) new extent min=(%d,%d) max=(%d,%d)"),
		PaddingComponentsWest,
		PaddingComponentsEast,
		PaddingComponentsSouth,
		PaddingComponentsNorth,
		NewComponentCountX,
		NewComponentCountY,
		NewRegion.Min.X,
		NewRegion.Min.Y,
		NewRegion.Max.X,
		NewRegion.Max.Y);

	if (bDryRun)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log, TEXT("Dry run requested; no landscape changes applied."));
		return 0;
	}

	TArray<uint16> ExpandedHeightData;
	TArray<FLandscapeImportLayerInfo> ExpandedLayerImports;
	TArray<FLandscapeLayerCopySpec> LayerSpecs;
	if (!ExpandLandscapeData(SourceLandscape, SourceLandscapeInfo, OldRegion, NewRegion, ExpandedHeightData, ExpandedLayerImports, LayerSpecs))
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error,
			TEXT("Failed to extract and expand landscape data from %s."), *SourceLandscape->GetName());
		return 1;
	}

	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	HeightDataPerLayers.Add(FGuid(), ExpandedHeightData);

	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
	if (!ExpandedLayerImports.IsEmpty())
	{
		MaterialLayerDataPerLayers.Add(FGuid(), ExpandedLayerImports);
	}

	const FVector WorldOffset = SourceLandscape->GetTransform().TransformVector(
		FVector(NewRegion.Min.X - OldRegion.Min.X, NewRegion.Min.Y - OldRegion.Min.Y, 0.f));
	const FVector NewLandscapeLocation = SourceLandscape->GetActorLocation() + WorldOffset;
	const FString OriginalActorLabel = SourceLandscape->GetActorLabel();

	FActorSpawnParameters SpawnParams;
	ALandscape* const ExpandedLandscape = World->SpawnActor<ALandscape>(
		NewLandscapeLocation,
		SourceLandscape->GetActorRotation(),
		SpawnParams);
	if (ExpandedLandscape == nullptr)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error, TEXT("Failed to spawn expanded landscape actor."));
		return 1;
	}

	ExpandedLandscape->CopySharedProperties(SourceLandscape);
	ExpandedLandscape->SetLandscapeGuid(FGuid::NewGuid());
	ExpandedLandscape->SetActorScale3D(SourceLandscape->GetActorScale3D());

	const int32 NewSizeX = NewRegion.Width() + 1;
	const int32 NewSizeY = NewRegion.Height() + 1;
	ExpandedLandscape->Import(
		FGuid::NewGuid(),
		0,
		0,
		NewSizeX - 1,
		NewSizeY - 1,
		SourceLandscape->NumSubsections,
		SourceLandscape->SubsectionSizeQuads,
		HeightDataPerLayers,
		TEXT(""),
		MaterialLayerDataPerLayers,
		ELandscapeImportAlphamapType::Additive,
		TArrayView<const FLandscapeLayer>());

	ULandscapeInfo* const ExpandedLandscapeInfo = ExpandedLandscape->GetLandscapeInfo();
	if (ExpandedLandscapeInfo == nullptr)
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error, TEXT("Expanded landscape did not create landscape info."));
		return 1;
	}

	for (const FLandscapeLayerCopySpec& LayerSpec : LayerSpecs)
	{
		if (LayerSpec.LayerInfo != nullptr)
		{
			ExpandedLandscape->AddTargetLayer(LayerSpec.LayerName, FLandscapeTargetLayerSettings(LayerSpec.LayerInfo));
		}
		else
		{
			ExpandedLandscape->AddTargetLayer(LayerSpec.LayerName, FLandscapeTargetLayerSettings());
		}
	}
	ExpandedLandscapeInfo->UpdateLayerInfoMap(ExpandedLandscape);

	if (!World->EditorDestroyActor(SourceLandscape, true))
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Warning,
			TEXT("Failed to destroy original landscape actor %s."), *SourceLandscape->GetName());
	}

	ExpandedLandscape->SetActorLabel(OriginalActorLabel, true);

	if (!UEditorLoadingAndSavingUtils::SaveMap(World, LevelAssetPath))
	{
		UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Error, TEXT("Expanded landscape created, but saving %s failed."), *LevelAssetPath);
		return 1;
	}

	UE_LOG(LogZhoenusExpandLevel1LandscapeCommandlet, Log,
		TEXT("Expanded %s with padding west=%d east=%d south=%d north=%d and saved %s."),
		*OriginalActorLabel,
		PaddingComponentsWest,
		PaddingComponentsEast,
		PaddingComponentsSouth,
		PaddingComponentsNorth,
		*LevelAssetPath);

	return 0;
}
