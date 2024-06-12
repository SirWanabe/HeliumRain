#include "FlareSpacecraftOrderOverlayInfo.h"
#include "FlareSpacecraftOrderOverlay.h"
#include "../FlareUITypes.h"
#include "../../Flare.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSpacecraftTypes.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Data/FlareFactoryCatalogEntry.h"
#include "../../Data/FlareSpaceCraftCatalog.h"
#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftOrderOverlayInfo"

/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSpaceCraftOverlayInfo::Construct(const FArguments& InArgs)
{
	// Data
	VerboseInformation = InArgs._VerboseInformation;
	MenuManager = InArgs._MenuManager;
	TargetSector = InArgs._TargetSector;
	OrderIsShip = InArgs._OrderIsShip;
	Desc = InArgs._Desc;
	TargetShipyard = InArgs._TargetShipyard;
	TargetSkirmish = InArgs._TargetSkirmish;
	OrderIsConfig = InArgs._OrderIsConfig;

	if (VerboseInformation)
	{
		SetSpacecraftInfoVerbose();
	}
	else
	{
		VerboseInfoLeft = FText();
		VerboseInfoRight = FText();
	}

	if (TargetShipyard)
	{
		SpaceCraftData = TargetShipyard->GetData();
	}

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetShipyard || TargetSkirmish)
	{
		// Regular ship-buying (no skirmish)
		if (TargetShipyard)
		{
			if (!OrderIsConfig)
			{
				// Production time
				ProductionTime = TargetShipyard->GetShipProductionTime(Desc->Identifier);
				ProductionTime += TargetShipyard->GetEstimatedQueueAndProductionDuration(Desc->Identifier, -1);
			}
		}
	}
	// Station-building
	else
	{
		ProductionTime = Desc->CycleCost.ProductionTime;
	}

	//create the layout
	ChildSlot
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Picture
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(&Desc->MeshPreviewBrush)
		]

	// Main infos
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.Text(Desc->Name)
//			.Text(FText::FromName(Desc->Identifier))
//			.Text(Desc->ImmatriculationCode)
			.TextStyle(&Theme.NameFont)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
		[
			SNew(STextBlock)
			.Text(Desc->Description)
		.TextStyle(&Theme.TextFont)
		.WrapTextAt(Theme.ContentWidth)
		]
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.Text(this, &SFlareSpaceCraftOverlayInfo::GetSpacecraftInfo)
		.TextStyle(&Theme.TextFont)
		]
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.Text(this, &SFlareSpaceCraftOverlayInfo::GetSpacecraftInfoVerboseRight)
		.TextStyle(&Theme.TextFont)
		]
		]
		]
		]

	// Costs
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ProductionCost", "Production cost & duration"))
		.Visibility(!OrderIsConfig ? EVisibility::Visible : EVisibility::Collapsed)
		.TextStyle(&Theme.NameFont)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(TextBlock, STextBlock)
			//			SNew(STextBlock)
		.Text(this, &SFlareSpaceCraftOverlayInfo::GetProductionText)

		.WrapTextAt(0.65 * Theme.ContentWidth)
		.TextStyle(&Theme.TextFont)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(this, &SFlareSpaceCraftOverlayInfo::GetProductionTime)
		.Visibility(this, &SFlareSpaceCraftOverlayInfo::GetProductionTimeVisibility)
		.WrapTextAt(0.65 * Theme.ContentWidth)
		.TextStyle(&Theme.TextFont)
		]
		]
		];
}

void SFlareSpaceCraftOverlayInfo::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (TargetShipyard)
	{
		if (OrderIsConfig)
		{
			SpaceCraftData = TargetShipyard->GetData();
		}
	}
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

EVisibility SFlareSpaceCraftOverlayInfo::GetProductionTimeVisibility() const
{
	if (IsEnabled() && ProductionTime > 0)
	{
		if (!TargetSkirmish && !OrderIsConfig)
		{
			return EVisibility::Visible;
		}
	}
	return EVisibility::Collapsed;
}

FText SFlareSpaceCraftOverlayInfo::GetSpacecraftShipInfo() const
{
	uint32 CargoBayCount = Desc->CargoBayCount;
	uint32 CargoBayCapacity = Desc->CargoBayCapacity;

	FText ShipDesignation;
	FText Drones;
	FText GunsandTurrets;
	FText CargoInfo;
	
	bool GunsAndTurretsSet = false;
	bool CargoSet = false;

	if (Desc->DroneMaximum)
	{
		Drones = FText::Format(LOCTEXT("Drones", "{0} drones, "),
		FText::AsNumber(Desc->DroneMaximum));
	}

	if (!Desc->IsStation())
	{
		if (Desc->GunSlots.Num() || Desc->TurretSlots.Num())
		{
			ShipDesignation = LOCTEXT("MilitaryShip", "Military Ship");
		}
		else if (CargoBayCount > 0 || CargoBayCapacity > 0)
		{
			ShipDesignation = LOCTEXT("TradeShip", "Trade Ship");
		}
	}
	else
	{
		if (Desc->GunSlots.Num() || Desc->TurretSlots.Num())
		{
			ShipDesignation = LOCTEXT("MilitaryStation", "Militarized Station");
		}
		else
		{
			ShipDesignation = LOCTEXT("Station", "Station");
		}
	}

	if (Desc->GunSlots.Num() || Desc->TurretSlots.Num())
	{
		GunsAndTurretsSet = true;
		if (Desc->GunSlots.Num() && Desc->TurretSlots.Num())
		{
			GunsandTurrets = FText::Format(LOCTEXT("GunsAndTurrets", "{0}{1} guns, {2} turrets, {3} combat value"),
			Drones,
			FText::AsNumber(Desc->GunSlots.Num()),
			FText::AsNumber(Desc->TurretSlots.Num()),
			FText::AsNumber(Desc->CombatPoints));
		}
		else if (Desc->GunSlots.Num())
		{
			GunsandTurrets = FText::Format(LOCTEXT("GunSlots", "{0}{1} gun slots, {2} combat value"),
			Drones,
			FText::AsNumber(Desc->GunSlots.Num()),
			FText::AsNumber(Desc->CombatPoints));
		}
		else
		{
			GunsandTurrets = FText::Format(LOCTEXT("GunSlots", "{0}{1} turrets, {2} combat value"),
			Drones,
			FText::AsNumber(Desc->TurretSlots.Num()),
			FText::AsNumber(Desc->CombatPoints));
		}
	}

	if (CargoBayCount > 0 || CargoBayCapacity > 0)
	{
		if (GunsAndTurretsSet)
		{
			GunsandTurrets = FText::Format(LOCTEXT("GunsAndTurretsFormatCargo", "{0}, "),
			GunsandTurrets);
		}
		CargoSet = true;
		CargoInfo = FText::Format(LOCTEXT("CargoInformation", "{0}x{1} cargo units"),
		FText::AsNumber(CargoBayCount),
		FText::AsNumber(CargoBayCapacity));
	}

	if (!GunsAndTurretsSet && !CargoSet)
	{
		return FText::Format(LOCTEXT("SpaceShipInfo", "{0}"),
		ShipDesignation);
	}
	return FText::Format(LOCTEXT("SpaceShipInfo", "{0} ({1}{2})"),
	ShipDesignation,
	GunsandTurrets,
	CargoInfo);
}

FText SFlareSpaceCraftOverlayInfo::GetSpacecraftInfoVerboseLeft() const
{
	return VerboseInfoLeft;
}
FText SFlareSpaceCraftOverlayInfo::GetSpacecraftInfoVerboseRight() const
{
	return VerboseInfoRight;
}

void SFlareSpaceCraftOverlayInfo::SetSpacecraftInfoVerbose()
{
	//	FText BasicInfo = GetSpacecraftShipInfo();

	FText Engines;
	FText EnginesRight;
	FText Mass;
	FText MassRight;
	FText HeatCapacity;
	FText HeatCapacityRight;
	FText RCSCount;
	FText RCSCountRight;
	FText DroneMax;
	FText DroneMaxRight;
	FText AngularMaxVelocity;
	FText AngularMaxVelocityRight;
	FText LinearMaxVelocity;
	FText LinearMaxVelocityRight;

	FText InternalComponents;
	FText InternalComponentsRight;

	//internal component values:
	// HP
	float HitPoints = 0;
	// Armor in %
	float ArmourPoints = 0;
	// Engine thrust in KN
	int32 TotalEnginePower = 0;
	//  Value represents global rcs system initial capabilities. Angular acceleration un °/s^2
	float AngularAccelerationRate = 0.f;
	// Heat radiation surface in m^2
	float Heatsink = 0.f;
	// Heat production on usage in KiloWatt on max usage
	float HeatProduction = 0.f;
	// Heat production on usage in KiloWatt on max usage
	float HeatProductionWeapons = 0.f;
	// Heat production on usage in KiloWatt on max usage
	float HeatProductionEngines = 0.f;
	// Heat production on usage in KiloWatt on max usage
	float HeatProductionRCS = 0.f;
	// Cargo volume un m^3
	float ComponentCargo = 0.f;
	/** Weapon ammo max capacity */
	int32 TotalAmmoCapacity = 0;

	UFlareSpacecraftComponentsCatalog* Catalog = MenuManager->GetGame()->GetShipPartsCatalog();
	FName RCSIdentifier;
	FName OrbitalEngineIdentifier;
	FName DefaultGunIdentifier = FName("weapon-eradicator");
	FName DefaultTurretIdentifier = FName("weapon-artemis");

	if (Desc->Size == EFlarePartSize::S)
	{
		RCSIdentifier = FName("rcs-coral");
		OrbitalEngineIdentifier = FName("engine-thresher");
	}
	else if (Desc->Size == EFlarePartSize::L)
	{
		RCSIdentifier = FName("rcs-rift");
		OrbitalEngineIdentifier = FName("pod-thera");
	}

	if (Desc->DefaultRCS != NAME_None)
	{
		RCSIdentifier = Desc->DefaultRCS;
	}

	if (Desc->DefaultEngine != NAME_None)
	{
		OrbitalEngineIdentifier = Desc->DefaultEngine;
	}

	if (Desc->RCSCount > 0)
	{
		for (int32 i = 0; i < Desc->RCSCount; i++)
		{
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(RCSIdentifier);
			if (ComponentDescription)
			{
				HitPoints += ComponentDescription->HitPoints;
				ArmourPoints += ComponentDescription->Armor;
				Heatsink += ComponentDescription->GeneralCharacteristics.HeatSink;
				HeatProductionRCS += ComponentDescription->GeneralCharacteristics.HeatProduction;
				ComponentCargo += ComponentDescription->GeneralCharacteristics.Cargo;

				TotalEnginePower += ComponentDescription->EngineCharacteristics.EnginePower;
				AngularAccelerationRate += ComponentDescription->EngineCharacteristics.AngularAccelerationRate;

				TotalAmmoCapacity += ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			}
		}
	}

	if (Desc->OrbitalEngineCount > 0)
	{
		for (int32 i = 0; i < Desc->OrbitalEngineCount; i++)
		{
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(OrbitalEngineIdentifier);
			if (ComponentDescription)
			{
				HitPoints += ComponentDescription->HitPoints;
				ArmourPoints += ComponentDescription->Armor;
				Heatsink += ComponentDescription->GeneralCharacteristics.HeatSink;
				HeatProductionEngines += ComponentDescription->GeneralCharacteristics.HeatProduction;
				ComponentCargo += ComponentDescription->GeneralCharacteristics.Cargo;

				TotalEnginePower += ComponentDescription->EngineCharacteristics.EnginePower;
				AngularAccelerationRate += ComponentDescription->EngineCharacteristics.AngularAccelerationRate;

				TotalAmmoCapacity += ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			}
		}
	}

//	FString InternalCompString;
//	FString InternalCompRightString;

	if (Desc->InternalComponentSlots.Num() > 0)
	{
		for (int32 SlotIndex = 0; SlotIndex < Desc->InternalComponentSlots.Num(); SlotIndex++)
		{
			FFlareSpacecraftSlotDescription* SlotDescription = &Desc->InternalComponentSlots[SlotIndex];
			if (SlotDescription)
			{
				FName ComponentIdentifier = SlotDescription->ComponentIdentifier;
				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentIdentifier);
				if (ComponentDescription)
				{
/*
					if (InternalCompString.Len())
					{
						InternalCompString += "\n";
					}

					InternalCompString += FString::Printf(TEXT("%s - %s"), *SlotDescription->SlotIdentifier.ToString(), *SlotDescription->ComponentIdentifier.ToString());
					if (SlotDescription->PoweredComponents.Num() > 0)
					{
						InternalCompRightString += FString::Printf(TEXT("\n \n %s\n \n"), *SlotDescription->SlotIdentifier.ToString());
						for (int32 PowerSlotIndex = 0; PowerSlotIndex < SlotDescription->PoweredComponents.Num(); PowerSlotIndex++)
						{
							FName PoweredSlot = SlotDescription->PoweredComponents[PowerSlotIndex];
							InternalCompRightString += FString::Printf(TEXT("%s\n"), *PoweredSlot.ToString());
						}
					}
*/
					HitPoints += ComponentDescription->HitPoints;
					ArmourPoints += ComponentDescription->Armor;
					Heatsink += ComponentDescription->GeneralCharacteristics.HeatSink;
					HeatProduction += ComponentDescription->GeneralCharacteristics.HeatProduction;
					ComponentCargo += ComponentDescription->GeneralCharacteristics.Cargo;

					TotalEnginePower += ComponentDescription->EngineCharacteristics.EnginePower;
					AngularAccelerationRate += ComponentDescription->EngineCharacteristics.AngularAccelerationRate;

					TotalAmmoCapacity += ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				}
			}
		}
	}
	if (Desc->GunSlots.Num() > 0)
	{
		for (int32 SlotIndex = 0; SlotIndex < Desc->GunSlots.Num(); SlotIndex++)
		{
			FFlareSpacecraftSlotDescription* SlotDescription = &Desc->GunSlots[SlotIndex];
			if (SlotDescription)
			{
				FName ComponentIdentifier = SlotDescription->ComponentIdentifier;
				if (ComponentIdentifier == NAME_None)
				{
					ComponentIdentifier = DefaultGunIdentifier;
				}

				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentIdentifier);
				if (ComponentDescription)
				{
					HitPoints += ComponentDescription->HitPoints;
					ArmourPoints += ComponentDescription->Armor;
					Heatsink += ComponentDescription->GeneralCharacteristics.HeatSink;
					HeatProductionWeapons += ComponentDescription->GeneralCharacteristics.HeatProduction;
					ComponentCargo += ComponentDescription->GeneralCharacteristics.Cargo;

					TotalEnginePower += ComponentDescription->EngineCharacteristics.EnginePower;
					AngularAccelerationRate += ComponentDescription->EngineCharacteristics.AngularAccelerationRate;

					TotalAmmoCapacity += ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				}
			}
		}
	}

//	FString TurretAngleStrings = "";

	if (Desc->TurretSlots.Num() > 0)
	{
		for (int32 SlotIndex = 0; SlotIndex < Desc->TurretSlots.Num(); SlotIndex++)
		{
			FFlareSpacecraftSlotDescription* SlotDescription = &Desc->TurretSlots[SlotIndex];
			if (SlotDescription)
			{
				FName ComponentIdentifier = SlotDescription->ComponentIdentifier;
				if (ComponentIdentifier == NAME_None)
				{
					ComponentIdentifier = DefaultTurretIdentifier;
				}
/*
				if (TurretAngleStrings.Len())
				{
					TurretAngleStrings += "\n";
				}

				TurretAngleStrings += FString::Printf(TEXT("\n%s\n"), *SlotDescription->SlotIdentifier.ToString());

				for (int32 BarrelAngles = 0; BarrelAngles < SlotDescription->TurretBarrelsAngleLimit.Num(); BarrelAngles++)
				{
					float Value = SlotDescription->TurretBarrelsAngleLimit[BarrelAngles];			
					TurretAngleStrings += FString::Printf(TEXT("%f\n"), Value);
				}
*/
				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentIdentifier);
				if (ComponentDescription)
				{
					HitPoints += ComponentDescription->HitPoints;
					ArmourPoints += ComponentDescription->Armor;
					Heatsink += ComponentDescription->GeneralCharacteristics.HeatSink;
					HeatProductionWeapons += ComponentDescription->GeneralCharacteristics.HeatProduction;
					ComponentCargo += ComponentDescription->GeneralCharacteristics.Cargo;

					TotalEnginePower += ComponentDescription->EngineCharacteristics.EnginePower;
					AngularAccelerationRate += ComponentDescription->EngineCharacteristics.AngularAccelerationRate;

					TotalAmmoCapacity += ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				}
			}
		}
	}

	FText HitPointsText;
	FText HitPointsTextRight;
	FText ArmourPointsText;
	FText ArmourPointsTextRight;

	FText TotalEnginePowerText;
	FText TotalEnginePowerTextRight;
	FText AngularAccelerationRateText;
	FText AngularAccelerationRateTextRight;

	FText HeatsinkText;
	FText HeatsinkTextRight;
	FText HeatProductionText;
	FText HeatProductionTextRight;
	FText HeatProductionWEPText;
	FText HeatProductionWEPTextRight;
	FText HeatProductionEnginesText;
	FText HeatProductionEnginesTextRight;
	FText HeatProductionRCSText;
	FText HeatProductionRCSTextRight;
	FText ComponentCargoText;
	FText ComponentCargoTextRight;
	FText TotalAmmoCapacityText;
	FText TotalAmmoCapacityTextRight;

//	FText TurretAngles = FText::FromString(TurretAngleStrings);
//	FText InternalCompsLeft = FText::FromString(InternalCompString);
//	FText InternalCompsRight = FText::FromString(InternalCompRightString);

	if (HitPoints > 0)
	{
		HitPointsText = LOCTEXT("HullIntegrity", "Hull Integrity:\n");
		HitPointsTextRight = FText::Format(LOCTEXT("HullIntegrityValue", "{0}\n"),
			FText::AsNumber(HitPoints));
	}

	if (ArmourPoints > 0)
	{
		ArmourPointsText = LOCTEXT("Armor", "Armor:\n");
		ArmourPointsTextRight = FText::Format(LOCTEXT("ArmorValue", "{0}\n"),
			FText::AsNumber(ArmourPoints * 100));
	}

	if (TotalEnginePower > 0)
	{
		TotalEnginePowerText = LOCTEXT("EnginePower", "Engine Power:\n");
		TotalEnginePowerTextRight = FText::Format(LOCTEXT("EnginePowerValue", "{0}KN\n"),
			FText::AsNumber(TotalEnginePower));
	}

	if (AngularAccelerationRate > 0)
	{
		AngularAccelerationRateText = LOCTEXT("AngularAccelerationRate", "Angular Acceleration Rate:\n");
		AngularAccelerationRateTextRight = FText::Format(LOCTEXT("AngularAccelerationRateValue", "{0} °/s^2\n"),
			FText::AsNumber(AngularAccelerationRate));
	}

	if (Heatsink > 0)
	{
		HeatsinkText = LOCTEXT("Heatsink", "Heatsink:\n");
		HeatsinkTextRight = FText::Format(LOCTEXT("HeatsinkValue", "{0} M^2\n"),
			FText::AsNumber(Heatsink));
	}

	if (HeatProduction > 0)
	{
		HeatProductionText = LOCTEXT("HeatProduction", "Heat Production:\n");
		HeatProductionTextRight = FText::Format(LOCTEXT("HeatProductionValue", "{0} K/W\n"),
			FText::AsNumber(HeatProduction));
	}

	if (HeatProductionWeapons > 0)
	{
		HeatProductionWEPText = LOCTEXT("HeatProductionWep", "\u2022	WEAPON:\n");
		HeatProductionWEPTextRight = FText::Format(LOCTEXT("HeatProductionWepValue", "{0} K/W\n"),
			FText::AsNumber(HeatProductionWeapons));
	}

	if (HeatProductionEngines > 0)
	{
		HeatProductionEnginesText = LOCTEXT("HeatProductionEngine", "\u2022	ENGINE:\n");
		HeatProductionEnginesTextRight = FText::Format(LOCTEXT("HeatProductionEngineValue", "{0} K/W\n"),
			FText::AsNumber(HeatProductionEngines));
	}

	if (HeatProductionRCS > 0)
	{
		HeatProductionRCSText = LOCTEXT("HeatProductionRCS", "\u2022	RCS:\n");
		HeatProductionRCSTextRight = FText::Format(LOCTEXT("HeatProductionRCSValue", "{0} K/W\n"),
			FText::AsNumber(HeatProductionRCS));
	}


	if (ComponentCargo > 0)
	{
		ComponentCargoText = LOCTEXT("Cargo", "Cargo:\n");
		ComponentCargoTextRight = FText::Format(LOCTEXT("CargoValue", "{0} M^3\n"),
			FText::AsNumber(ComponentCargo));
	}

	if (TotalAmmoCapacity > 0)
	{
		TotalAmmoCapacityText = LOCTEXT("AmmoCapacity", "Ammo Capacity:\n");
		TotalAmmoCapacityTextRight = FText::Format(LOCTEXT("AmmoCapacityValue", "{0}\n"),
			FText::AsNumber(TotalAmmoCapacity));
	}

	InternalComponents = FText::Format(LOCTEXT("DefaultInternalComponentsSummary", "\nDefault Component Configuration:\n{0}{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}"),
		HitPointsText,
		ArmourPointsText,
		TotalEnginePowerText,
		AngularAccelerationRateText,
		HeatsinkText,
		HeatProductionText,
		HeatProductionWEPText,
		HeatProductionEnginesText,
		HeatProductionRCSText,
		TotalAmmoCapacityText,
		ComponentCargoText);

	InternalComponentsRight = FText::Format(LOCTEXT("DefaultInternalComponentsRightSummary", "\n\n{0}{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}"),
		HitPointsTextRight,
		ArmourPointsTextRight,
		TotalEnginePowerTextRight,
		AngularAccelerationRateTextRight,
		HeatsinkTextRight,
		HeatProductionTextRight,
		HeatProductionWEPTextRight,
		HeatProductionEnginesTextRight,
		HeatProductionRCSTextRight,
		TotalAmmoCapacityTextRight,
		ComponentCargoTextRight);

	if (Desc->Mass > 0)
	{
		Mass = LOCTEXT("Mass", "Mass:\n");
		MassRight = FText::Format(LOCTEXT("MassValue", "{0}-Tons\n"),
			FText::AsNumber(Desc->Mass / 1000));
	}

	if (Desc->HeatCapacity > 0)
	{
		HeatCapacity = LOCTEXT("HeatCapacity", "Heat Capacity:\n");
		HeatCapacityRight = FText::Format(LOCTEXT("HeatCapacityValue", "{0} KJ/K\n"),
			FText::AsNumber(Desc->HeatCapacity));
	}

	if (Desc->OrbitalEngineCount > 0)
	{
		Engines = LOCTEXT("Engines", "Engines:\n");
		EnginesRight = FText::Format(LOCTEXT("EnginesValue", "{0}\n"),
			FText::AsNumber(Desc->OrbitalEngineCount));
	}

	if (Desc->RCSCount > 0)
	{
		RCSCount = LOCTEXT("RCSCount", "RCS:\n");
		RCSCountRight = FText::Format(LOCTEXT("RCSCountValue", "{0}\n"),
			FText::AsNumber(Desc->RCSCount));
	}

	if (Desc->AngularMaxVelocity > 0)
	{
		AngularMaxVelocity = LOCTEXT("AngularMaxVelocity", "Maximum Angular Velocity:\n");
		AngularMaxVelocityRight = FText::Format(LOCTEXT("AngularMaxVelocityValue", "{0} degree/s\n"),
			FText::AsNumber(Desc->AngularMaxVelocity));
	}

	if (Desc->LinearMaxVelocity > 0)
	{
		LinearMaxVelocity = LOCTEXT("LinearMaxVelocity", "Maximum Linear Velocity:\n");
		LinearMaxVelocityRight = FText::Format(LOCTEXT("LinearMaxVelocityValue", "{0}\n"),
			FText::AsNumber(Desc->LinearMaxVelocity));
	}

	if (Desc->DroneMaximum > 0)
	{
		DroneMax = LOCTEXT("DroneMaximum", "Maximum Drones:\n");
		DroneMaxRight = FText::Format(LOCTEXT("DroneMaximumValue", "{0}\n"),
			FText::AsNumber(Desc->DroneMaximum));
	}

	VerboseInfoLeft = FText::Format(LOCTEXT("SpacecraftInfoVerboseHelperLeft", "{0}{1}{2}{3}{4}{5}{6}{7}"),
		Mass,
		HeatCapacity,
		Engines,
		RCSCount,
		AngularMaxVelocity,
		LinearMaxVelocity,
		DroneMax,
		InternalComponents);
//		InternalCompsLeft,
//		TurretAngles);

	VerboseInfoRight = FText::Format(LOCTEXT("SpacecraftInfoVerboseHelperRight", "\n\n{0}{1}{2}{3}{4}{5}{6}{7}"),
		MassRight,
		HeatCapacityRight,
		EnginesRight,
		RCSCountRight,
		AngularMaxVelocityRight,
		LinearMaxVelocityRight,
		DroneMaxRight,
		InternalComponentsRight);
//		InternalCompsRight);
}

FText SFlareSpaceCraftOverlayInfo::GetSpacecraftInfo() const
{
	// Ship-building
	if (TargetShipyard || TargetSkirmish)
	{
		// Station
		if (Desc->IsStation())
		{
			return FText::Format(LOCTEXT("FactoryStationFormat", "(Station, {0} factories)"),
				FText::AsNumber(Desc->Factories.Num()));
		}
		return GetSpacecraftShipInfo();
	}

	// Station-building
	else
	{
		//		FCHECK(TargetSector);

		if (VerboseInformation)
		{
			if (Desc->Factories.Num() > 0)
			{
				FText VerboseBasicInfo = GetSpacecraftInfoVerboseLeft();
				FText LeftBasicInfo = GetSpacecraftShipInfo();
				FText BasicInfo = FText::Format(LOCTEXT("SpacecraftInfoVerbose", "{0}\n{1}"),
					LeftBasicInfo,
					VerboseBasicInfo);

				FString FactoryString;

				for (int32 FactoryIndex = 0; FactoryIndex < Desc->Factories.Num(); FactoryIndex++)
				{
					FFlareFactoryDescription* FactoryDescription = &Desc->Factories[FactoryIndex]->Data;
					if (FactoryDescription == nullptr || FactoryDescription == NULL)
					{
						continue;
					}

					FFlareProductionData* FactoryCycleCost = &FactoryDescription->CycleCost;

					FactoryString += "\nFactory: ";

					FactoryString += FString::Printf(TEXT("%s"), *FactoryDescription->Name.ToString());
					bool DoneResource = false;

					FactoryString += "\nProduction Cost: ";

					if (FactoryCycleCost->ProductionCost > 0)
					{
						FString FormattedNumber = FString::FormatAsNumber(UFlareGameTools::DisplayMoney(FactoryCycleCost->ProductionCost));
						FactoryString += FString::Printf(TEXT("%s"), *FormattedNumber);
						FactoryString += " credits";
					}
					else
					{
						FactoryString += "0 credits";
					}

					FactoryString += ", Time: ";

					if (FactoryCycleCost->ProductionTime > 0)
					{
						FactoryString += FString::Printf(TEXT("%u"), FactoryCycleCost->ProductionTime);
						if (FactoryCycleCost->ProductionTime == 1)
						{
							FactoryString += " day";
						}
						else
						{
							FactoryString += " days";
						}
					}
					else
					{
						FactoryString += "instantly";
					}


					if (FactoryCycleCost->InputResources.Num() > 0)
					{
						FactoryString += "\nInputs\n";
						for (FFlareFactoryResource InputResource : FactoryCycleCost->InputResources)
						{
							if (DoneResource)
							{
								FactoryString += ", ";
							}
							FactoryString += FString::Printf(TEXT("%u %s"), InputResource.Quantity, *InputResource.Resource->Data.Name.ToString());
							DoneResource = true;
						}
					}

					if (FactoryCycleCost->OutputResources.Num() > 0)
					{
						FactoryString += "\nOutputs\n";
						DoneResource = false;
						for (FFlareFactoryResource OutputResource : FactoryCycleCost->OutputResources)
						{
							if (DoneResource)
							{
								FactoryString += ", ";
							}

							FactoryString += FString::Printf(TEXT("%u %s"), OutputResource.Quantity, *OutputResource.Resource->Data.Name.ToString());
							DoneResource = true;
						}
					}

					if (FactoryDescription->OutputActions.Num() > 0)
					{
						FactoryString += "\nSpecial Actions:\n";
						DoneResource = false;
						for (FFlareFactoryAction OutputAction : FactoryDescription->OutputActions)
						{
							if (DoneResource)
							{
								FactoryString += ", ";
							}

							if (OutputAction.Action == EFlareFactoryAction::BuildStation)
							{
								FactoryString += "Build Station";
							}
							else if (OutputAction.Action == EFlareFactoryAction::CreateShip)
							{
								FactoryString += "Create Ship";
							}
							else if (OutputAction.Action == EFlareFactoryAction::DiscoverSector)
							{
								FactoryString += "Discover Sector";
							}
							else if (OutputAction.Action == EFlareFactoryAction::GainResearch)
							{
								FactoryString += FString::Printf(TEXT("Gain %u Research"), OutputAction.Quantity);
							}
							DoneResource = true;
						}
					}

					if (FactoryIndex < Desc->Factories.Num())
					{
						FactoryString += "\n";
					}
				}

				if (Desc->IsStation())
				{
					return FText::Format(LOCTEXT("StationInfoFormatVerbose", "(Station, {0} factories)\n{1}\n{2}"),
						FText::AsNumber(Desc->Factories.Num()),
						BasicInfo,
						FText::FromString(FactoryString));
				}
				else
				{
					return FText::Format(LOCTEXT("ShipStationInfoFormatVerbose", "({0} factories)\n{1}\n{2}"),
						FText::AsNumber(Desc->Factories.Num()),
						BasicInfo,
						FText::FromString(FactoryString));
				}
			}
			else if (Desc->IsStation())
			{
				FText VerboseBasicInfo = GetSpacecraftInfoVerboseLeft();
				FText LeftBasicInfo = GetSpacecraftShipInfo();
				FText BasicInfo = FText::Format(LOCTEXT("StationInfoVerbose", "(Station, 0 factories)\n{0}\n{1}"),
					LeftBasicInfo,
					VerboseBasicInfo);
				return BasicInfo;
			}
			else
			{
				FText VerboseBasicInfo = GetSpacecraftInfoVerboseLeft();
				FText LeftBasicInfo = GetSpacecraftShipInfo();
				FText BasicInfo = FText::Format(LOCTEXT("InfoVerbose", "\n{0}\n{1}"),
					LeftBasicInfo,
					VerboseBasicInfo);
				return BasicInfo;
			}
		}
		else
		{
			return FText::Format(LOCTEXT("StationInfoFormat", "(Station, {0} factories)"), FText::AsNumber(Desc->Factories.Num()));
		}
	}
}

FText SFlareSpaceCraftOverlayInfo::GetProductionTime() const
{
	// Ship-building

	return FText::Format(LOCTEXT("ProductionTimeFormat", "\u2022 {0} days"),
		FText::AsNumber(ProductionTime));
}

FText SFlareSpaceCraftOverlayInfo::GetProductionText() const
{

	FText ProductionCost;

	// Ship-building
	if (TargetShipyard || TargetSkirmish)
	{
		// Regular ship-buying (no skirmish)
		if (TargetShipyard)
		{
			if (OrderIsConfig)
			{
				if (SpaceCraftData.ShipyardOrderExternalConfig.Find(Desc->Identifier) != INDEX_NONE)
				{
					ProductionCost = LOCTEXT("ConfigEnabled", "Enabled");
				}
				else
				{
					ProductionCost = LOCTEXT("ConfigDisabled", "Disabled");
				}
			}
			else
			{
				// Production cost
				if (MenuManager->GetPC()->GetCompany() == TargetShipyard->GetCompany())
				{
					ProductionCost = FText::Format(LOCTEXT("FactoryProductionResourcesFormat", "\u2022 {0}"), TargetShipyard->GetShipCost(Desc->Identifier));
				}
				else
				{
					int32 CycleProductionCost = UFlareGameTools::ComputeSpacecraftPrice(Desc->Identifier, TargetShipyard->GetCurrentSector(), true,false,true,MenuManager->GetPC()->GetCompany(),TargetShipyard->GetCompany());
					ProductionCost = FText::Format(LOCTEXT("FactoryProductionCostFormat", "\u2022 {0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(CycleProductionCost)));
				}
			}
		}

		// Skirmish
		else
		{
			ProductionCost = FText::Format(LOCTEXT("SkirmishCombatCost", "\u2022 {0}"), FText::AsNumber(Desc->CombatPoints));
		}
	}

	// Station-building
	else
	{
		// Add resources
		FString ResourcesString;
		for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			FFlareFactoryResource* FactoryResource = &Desc->CycleCost.InputResources[ResourceIndex];
			if (FactoryResource)
			{
				if (ResourcesString.Len())
				{
					ResourcesString += ", ";
				}

//			FString FormattedQuantity = FString::FormatAsNumber(FactoryResource->Quantity);
				ResourcesString += FString::Printf(TEXT("%u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Name.ToString()); // FString needed here
			}
		}

		// Constraints
		FString ConstraintString;
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::FreeAsteroid))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("AsteroidNeeded", "a free asteroid").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::SunExposure))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("SunNeeded", "good sun exposure").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::GeostationaryOrbit))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("GeostationaryNeeded", "a geostationary orbit").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::HideOnIce))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("NonIcyNeeded", "a non-icy sector").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::HideOnNoIce))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("IcyNeeded", "an icy sector").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::NoComplex))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("NoComplexNeeded", "regular sector building").ToString();
		}
		if (Desc->BuildConstraint.Contains(EFlareBuildConstraint::SpecialSlotInComplex))
		{
			if (ConstraintString.Len())
			{
				ConstraintString += ", ";
			}
			ConstraintString += LOCTEXT("SpecialComplexNeeded", "a central complex slot").ToString();
		}

		if (ConstraintString.Len())
		{
			ConstraintString = LOCTEXT("ConstructioNRequirement", "\n\u2022 Requires").ToString() + " " + ConstraintString;
		}

		if (TargetSector)
		{
			int32 CompanyStationCountInSector = 0;
			for (UFlareSimulatedSpacecraft* Station : TargetSector->GetSectorStations())
			{
				if (Station->GetCompany() == MenuManager->GetPC()->GetCompany())
				{
					++CompanyStationCountInSector;
				}
			}
			ProductionCost = FText::Format(LOCTEXT("StationCostFormatNoSect", "\u2022 Costs {0} credits ({1} stations in this sector) {3}\n\u2022 Completion requires {2}"),
				FText::AsNumber(UFlareGameTools::DisplayMoney(TargetSector->GetStationConstructionFee(Desc->CycleCost.ProductionCost, MenuManager->GetPC()->GetCompany()))),
				FText::AsNumber(CompanyStationCountInSector),
				FText::FromString(ResourcesString),
				FText::FromString(ConstraintString));
		}
		else
		{
			// Final text
			if (VerboseInformation)
			{
				FString FlagsString;

				if (Desc->IsDroneCarrier)
				{
					FlagsString += LOCTEXT("DroneCarrier", "Drone Carrier").ToString();
				}

				if (Desc->IsDroneShip)
				{
					if (FlagsString.Len())
					{
						FlagsString += ", ";
					}
					FlagsString += LOCTEXT("DroneShip", "Drone").ToString();
				}

				FlagsString = LOCTEXT("FlagStringConstructed", "\n\u2022").ToString() + " " + FlagsString;
				FText TechnologyRequired = GetTechnologyRequirementsText(Desc);
				if (!TechnologyRequired.IsEmpty())
				{
					ProductionCost = FText::Format(LOCTEXT("StationCostFormat", "\u2022 Costs {0} credits\n\u2022{1}\n\u2022 Requires {2}{3}"),
						UFlareGameTools::DisplayMoney(Desc->CycleCost.ProductionCost),
						FText::FromString(ResourcesString),
						TechnologyRequired,
						FText::FromString(ConstraintString),
						FText::FromString(FlagsString));
				}
				else
				{
					ProductionCost = FText::Format(LOCTEXT("StationCostFormat", "\u2022 Costs {0} credits\n\u2022 {1}{2}"),
						UFlareGameTools::DisplayMoney(Desc->CycleCost.ProductionCost),
						FText::FromString(ResourcesString),
						FText::FromString(ConstraintString),
						FText::FromString(FlagsString));
				}
			}
			else
			{
				ProductionCost = FText::Format(LOCTEXT("StationCostFormat", "\u2022 Costs {0} credits\n\u2022 {1}{2}"),
					UFlareGameTools::DisplayMoney(Desc->CycleCost.ProductionCost),
					FText::FromString(ResourcesString),
					FText::FromString(ConstraintString));
			}
		}
	}

	return ProductionCost;
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

FText SFlareSpaceCraftOverlayInfo::GetTechnologyRequirementsText(const FFlareSpacecraftDescription* Description) const
{
	const FFlareTechnologyDescription* Technology = nullptr;
	FText TechnologyRequired = FText();
	FName Identifier = Description->Identifier;

	if (Identifier == "station-habitation" ||
		Identifier == "station-outpost" ||
		Identifier == "station-solar-plant")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("stations");
	}
	else if (Identifier == "station-ice-mine" ||
		Identifier == "station-silica-mine" ||
		Identifier == "station-iron-mine")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("mining");
	}
	else if (Identifier == "station-carbon-refinery" ||
		Identifier == "station-farm" ||
		Identifier == "station-plastics-refinery")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("chemicals");
	}
	else if (Identifier == "station-ch4-pump" ||
		Identifier == "station-he3-pump" ||
		Identifier == "station-h2-pump")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("orbital-pumps");
	}
	else if (Identifier == "station-steelworks" ||
		Identifier == "station-tool-factory" ||
		Identifier == "station-arsenal")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("metallurgy");
	}
	else if (Identifier == "station-shipyard")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("shipyard-station");
	}
	else if (Identifier == "station-tokamak" ||
		Identifier == "station-hub" ||
		Identifier == "station-complex" ||
		Identifier == "station-foundry")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("advanced-stations");
	}
	else if (Identifier == "station-telescope" ||
		Identifier == "station-research")
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get("science");
	}
	else if(Description->RequiredTechnologies.Num() > 0)
	{
		Technology = MenuManager->GetGame()->GetTechnologyCatalog()->Get(Description->RequiredTechnologies[0]);
	}

	if (Technology)
	{
		TechnologyRequired = Technology->Name;
	}

	return TechnologyRequired;
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

#undef LOCTEXT_NAMESPACE
