
#include "FlareSaveWriter.h"
#include "../../Flare.h"
#include "../FlareSaveGame.h"
#include "Game/FlareGameTools.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSaveWriter::UFlareSaveWriter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveGame(UFlareSaveGame* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// General stuff
	JsonObject->SetStringField("Game", "Helium Rain");
	JsonObject->SetStringField("SaveFormat", FormatInt32(1));
	JsonObject->SetBoolField("AutoSave", Data->AutoSave);

	// Game data
	JsonObject->SetObjectField("Player", SavePlayer(&Data->PlayerData));
	JsonObject->SetObjectField("PlayerCompanyDescription", SaveCompanyDescription(&Data->PlayerCompanyDescription));
	JsonObject->SetStringField("CurrentImmatriculationIndex", FormatInt32(Data->CurrentImmatriculationIndex));
	JsonObject->SetStringField("CurrentIdentifierIndex", FormatInt32(Data->CurrentIdentifierIndex));
	JsonObject->SetObjectField("World", SaveWorld(&Data->WorldData));


	return JsonObject;
}

/*----------------------------------------------------
	Generator
----------------------------------------------------*/

TSharedRef<FJsonObject> UFlareSaveWriter::SavePlayer(FFlarePlayerSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("UUID", Data->UUID.ToString());
	JsonObject->SetStringField("ScenarioId", FormatInt32(Data->ScenarioId));
	JsonObject->SetStringField("DifficultyId", FormatInt32(Data->DifficultyId));
	JsonObject->SetBoolField("AICheats", Data->AICheats);

	JsonObject->SetStringField("PlayerEmblemIndex", FormatInt32(Data->PlayerEmblemIndex));
	JsonObject->SetStringField("CompanyIdentifier", Data->CompanyIdentifier.ToString());
	JsonObject->SetStringField("PlayerFleetIdentifier", Data->PlayerFleetIdentifier.ToString());
	JsonObject->SetStringField("LastFlownShipIdentifier", Data->LastFlownShipIdentifier.ToString());
	JsonObject->SetObjectField("Quest", SaveQuest(&Data->QuestData));

	TArray< TSharedPtr<FJsonValue> > UnlockedScannables;
	UnlockedScannables.Reserve(Data->UnlockedScannables.Num());
	for (int i = 0; i < Data->UnlockedScannables.Num(); i++)
	{
		UnlockedScannables.Add(MakeShareable(new FJsonValueString(Data->UnlockedScannables[i].ToString())));
	}
	JsonObject->SetArrayField("UnlockedScannables", UnlockedScannables);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveQuest(FFlareQuestSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("SelectedQuest", Data->SelectedQuest.ToString());
	JsonObject->SetBoolField("PlayTutorial", Data->PlayTutorial);
	JsonObject->SetBoolField("PlayStory", Data->PlayStory);
	JsonObject->SetStringField("NextGeneratedQuestIndex", FormatInt64(Data->NextGeneratedQuestIndex));

	TArray< TSharedPtr<FJsonValue> > QuestProgresses;
	QuestProgresses.Reserve(Data->QuestProgresses.Num());
	for(int i = 0; i < Data->QuestProgresses.Num(); i++)
	{
		QuestProgresses.Add(MakeShareable(new FJsonValueObject(SaveQuestProgress(&Data->QuestProgresses[i]))));
	}
	JsonObject->SetArrayField("QuestProgresses", QuestProgresses);

	TArray< TSharedPtr<FJsonValue> > SuccessfulQuests;
	SuccessfulQuests.Reserve(Data->SuccessfulQuests.Num());
	for(int i = 0; i < Data->SuccessfulQuests.Num(); i++)
	{
		SuccessfulQuests.Add(MakeShareable(new FJsonValueString(Data->SuccessfulQuests[i].ToString())));
	}
	JsonObject->SetArrayField("SuccessfulQuests", SuccessfulQuests);

	TArray< TSharedPtr<FJsonValue> > AbandonedQuests;
	AbandonedQuests.Reserve(Data->AbandonedQuests.Num());
	for(int i = 0; i < Data->AbandonedQuests.Num(); i++)
	{
		AbandonedQuests.Add(MakeShareable(new FJsonValueString(Data->AbandonedQuests[i].ToString())));
	}
	JsonObject->SetArrayField("AbandonedQuests", AbandonedQuests);

	TArray< TSharedPtr<FJsonValue> > FailedQuests;
	FailedQuests.Reserve(Data->FailedQuests.Num());
	for(int i = 0; i < Data->FailedQuests.Num(); i++)
	{
		FailedQuests.Add(MakeShareable(new FJsonValueString(Data->FailedQuests[i].ToString())));
	}
	JsonObject->SetArrayField("FailedQuests", FailedQuests);

	TArray< TSharedPtr<FJsonValue> > GeneratedQuests;
	GeneratedQuests.Reserve(Data->GeneratedQuests.Num());
	for(int i = 0; i < Data->GeneratedQuests.Num(); i++)
	{
		GeneratedQuests.Add(MakeShareable(new FJsonValueObject(SaveGeneratedQuest(&Data->GeneratedQuests[i]))));
	}
	JsonObject->SetArrayField("GeneratedQuests", GeneratedQuests);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveQuestProgress(FFlareQuestProgressSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("QuestIdentifier", Data->QuestIdentifier.ToString());
	JsonObject->SetStringField("Status", FormatEnum<EFlareQuestStatus::Type>("EFlareQuestStatus",Data->Status));

	JsonObject->SetStringField("AvailableDate", FormatInt64(Data->AvailableDate));
	JsonObject->SetStringField("AcceptationDate", FormatInt64(Data->AcceptationDate));

	JsonObject->SetObjectField("Data", SaveBundle(&Data->Data));

	TArray< TSharedPtr<FJsonValue> > SuccessfullSteps;
	SuccessfullSteps.Reserve(Data->SuccessfullSteps.Num());
	for(int i = 0; i < Data->SuccessfullSteps.Num(); i++)
	{
		SuccessfullSteps.Add(MakeShareable(new FJsonValueString(Data->SuccessfullSteps[i].ToString())));
	}
	JsonObject->SetArrayField("SuccessfullSteps", SuccessfullSteps);

	TArray< TSharedPtr<FJsonValue> > CurrentStepProgress;
	CurrentStepProgress.Reserve(Data->CurrentStepProgress.Num());
	for(int i = 0; i < Data->CurrentStepProgress.Num(); i++)
	{
		CurrentStepProgress.Add(MakeShareable(new FJsonValueObject(SaveQuestStepProgress(&Data->CurrentStepProgress[i]))));
	}
	JsonObject->SetArrayField("CurrentStepProgress", CurrentStepProgress);

	TArray< TSharedPtr<FJsonValue> > TriggerConditionsSave;
	TriggerConditionsSave.Reserve(Data->TriggerConditionsSave.Num());
	for(int i = 0; i < Data->TriggerConditionsSave.Num(); i++)
	{
		TriggerConditionsSave.Add(MakeShareable(new FJsonValueObject(SaveQuestStepProgress(&Data->TriggerConditionsSave[i]))));
	}
	JsonObject->SetArrayField("TriggerConditionsSave", TriggerConditionsSave);

	TArray< TSharedPtr<FJsonValue> > ExpirationConditionsSave;
	ExpirationConditionsSave.Reserve(Data->ExpirationConditionsSave.Num());
	for(int i = 0; i < Data->ExpirationConditionsSave.Num(); i++)
	{
		ExpirationConditionsSave.Add(MakeShareable(new FJsonValueObject(SaveQuestStepProgress(&Data->ExpirationConditionsSave[i]))));
	}
	JsonObject->SetArrayField("ExpirationConditionsSave", ExpirationConditionsSave);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveGeneratedQuest(FFlareGeneratedQuestSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("QuestClass", Data->QuestClass.ToString());
	JsonObject->SetObjectField("Data", SaveBundle(&Data->Data));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveQuestStepProgress(FFlareQuestConditionSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ConditionIdentifier", Data->ConditionIdentifier.ToString());
	JsonObject->SetObjectField("Data", SaveBundle(&Data->Data));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyDescription(FFlareCompanyDescription* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("ShortName", Data->ShortName.ToString());
	JsonObject->SetStringField("Description", Data->Description.ToString());

	JsonObject->SetStringField("CustomizationBasePaintColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationBasePaintColor)));
	JsonObject->SetStringField("CustomizationPaintColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationPaintColor)));
	JsonObject->SetStringField("CustomizationOverlayColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationOverlayColor)));
	JsonObject->SetStringField("CustomizationLightColor", FormatVector(UFlareGameTools::ColorToVector(Data->CustomizationLightColor)));
	JsonObject->SetStringField("CustomizationPatternIndex", FormatInt32(Data->CustomizationPatternIndex));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveWorld(FFlareWorldSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Date", FormatInt64(Data->Date));
	JsonObject->SetStringField("EventDate_GlobalWar", FormatInt64(Data->EventDate_GlobalWar));
	
	TArray< TSharedPtr<FJsonValue> > Companies;
	Companies.Reserve(Data->CompanyData.Num());
	for(int i = 0; i < Data->CompanyData.Num(); i++)
	{
		Companies.Add(MakeShareable(new FJsonValueObject(SaveCompany(&Data->CompanyData[i]))));
	}
	JsonObject->SetArrayField("Companies", Companies);

	TArray< TSharedPtr<FJsonValue> > Sectors;
	Sectors.Reserve(Data->SectorData.Num());
	for(int i = 0; i < Data->SectorData.Num(); i++)
	{
		Sectors.Add(MakeShareable(new FJsonValueObject(SaveSector(&Data->SectorData[i]))));
	}
	JsonObject->SetArrayField("Sectors", Sectors);

	TArray< TSharedPtr<FJsonValue> > Travels;
	Travels.Reserve(Data->TravelData.Num());
	for(int i = 0; i < Data->TravelData.Num(); i++)
	{
		Travels.Add(MakeShareable(new FJsonValueObject(SaveTravel(&Data->TravelData[i]))));
	}
	JsonObject->SetArrayField("Travels", Travels);

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompany(FFlareCompanySave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("CatalogIdentifier", FormatInt32(Data->CatalogIdentifier));
	JsonObject->SetStringField("Money", FormatInt64(Data->Money));
	JsonObject->SetStringField("CompanyValue", FormatInt64(Data->CompanyValue));
	JsonObject->SetStringField("PlayerLastPeaceDate", FormatInt64(Data->PlayerLastPeaceDate));
	JsonObject->SetStringField("PlayerLastWarDate", FormatInt64(Data->PlayerLastWarDate));
	JsonObject->SetStringField("PlayerLastTributeDate", FormatInt64(Data->PlayerLastTributeDate));
	JsonObject->SetStringField("FleetImmatriculationIndex", FormatInt32(Data->FleetImmatriculationIndex));
	JsonObject->SetStringField("TradeRouteImmatriculationIndex", FormatInt32(Data->TradeRouteImmatriculationIndex));
	JsonObject->SetStringField("WhiteListImmatriculationIndex", FormatInt32(Data->WhiteListImmatriculationIndex));
	JsonObject->SetStringField("DefaultWhiteListIdentifier", Data->DefaultWhiteListIdentifier.ToString());
	JsonObject->SetStringField("ResearchAmount", FormatInt32(Data->ResearchAmount));
	JsonObject->SetStringField("ResearchSpent", FormatInt32(Data->ResearchSpent));
	JsonObject->SetObjectField("AI", SaveCompanyAI(&Data->AI));
	JsonObject->SetObjectField("Licenses", SaveCompanyLicenses(&Data->Licenses));

	SaveFloat(JsonObject,"ResearchRatio", Data->ResearchRatio);
	SaveFloat(JsonObject,"Retaliation", Data->Retaliation);

	TArray< TSharedPtr<FJsonValue> > UnlockedTechnologies;
	UnlockedTechnologies.Reserve(Data->UnlockedTechnologies.Num());
	for (int i = 0; i < Data->UnlockedTechnologies.Num(); i++)
	{
		UnlockedTechnologies.Add(MakeShareable(new FJsonValueString(Data->UnlockedTechnologies[i].ToString())));
	}
	JsonObject->SetArrayField("UnlockedTechnologies", UnlockedTechnologies);

	TArray< TSharedPtr<FJsonValue> > CaptureOrders;
	CaptureOrders.Reserve(Data->CaptureOrders.Num());
	for (int i = 0; i < Data->CaptureOrders.Num(); i++)
	{
		CaptureOrders.Add(MakeShareable(new FJsonValueString(Data->CaptureOrders[i].ToString())));
	}
	JsonObject->SetArrayField("CaptureOrders", CaptureOrders);


	TArray< TSharedPtr<FJsonValue> > HostileCompanies;
	HostileCompanies.Reserve(Data->HostileCompanies.Num());
	for(int i = 0; i < Data->HostileCompanies.Num(); i++)
	{
		HostileCompanies.Add(MakeShareable(new FJsonValueString(Data->HostileCompanies[i].ToString())));
	}
	JsonObject->SetArrayField("HostileCompanies", HostileCompanies);

	TArray< TSharedPtr<FJsonValue> > Ships;
	Ships.Reserve(Data->ShipData.Num());
	for(int i = 0; i < Data->ShipData.Num(); i++)
	{
		Ships.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->ShipData[i]))));
	}
	JsonObject->SetArrayField("Ships", Ships);

	TArray< TSharedPtr<FJsonValue> > ChildStations;
	ChildStations.Reserve(Data->ChildStationData.Num());
	for(int i = 0; i < Data->ChildStationData.Num(); i++)
	{
		ChildStations.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->ChildStationData[i]))));
	}
	JsonObject->SetArrayField("ChildStations", ChildStations);

	TArray< TSharedPtr<FJsonValue> > Stations;
	Stations.Reserve(Data->StationData.Num());
	for(int i = 0; i < Data->StationData.Num(); i++)
	{
		Stations.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->StationData[i]))));
	}
	JsonObject->SetArrayField("Stations", Stations);

	TArray< TSharedPtr<FJsonValue> > DestroyedSpacecrafts;
	DestroyedSpacecrafts.Reserve(Data->DestroyedSpacecraftData.Num());
	for(int i = 0; i < Data->DestroyedSpacecraftData.Num(); i++)
	{
		DestroyedSpacecrafts.Add(MakeShareable(new FJsonValueObject(SaveSpacecraft(&Data->DestroyedSpacecraftData[i]))));
	}
	JsonObject->SetArrayField("DestroyedSpacecrafts", DestroyedSpacecrafts);

	TArray< TSharedPtr<FJsonValue> > Fleets;
	Fleets.Reserve(Data->Fleets.Num());
	for(int i = 0; i < Data->Fleets.Num(); i++)
	{
		Fleets.Add(MakeShareable(new FJsonValueObject(SaveFleet(&Data->Fleets[i]))));
	}
	JsonObject->SetArrayField("Fleets", Fleets);

	TArray< TSharedPtr<FJsonValue> > TradeRoutes;
	TradeRoutes.Reserve(Data->TradeRoutes.Num());
	for(int i = 0; i < Data->TradeRoutes.Num(); i++)
	{
		TradeRoutes.Add(MakeShareable(new FJsonValueObject(SaveTradeRoute(&Data->TradeRoutes[i]))));
	}
	JsonObject->SetArrayField("TradeRoutes", TradeRoutes);

	TArray< TSharedPtr<FJsonValue> > WhiteLists;
	TradeRoutes.Reserve(Data->WhiteLists.Num());
	for (int i = 0; i < Data->WhiteLists.Num(); i++)
	{
		WhiteLists.Add(MakeShareable(new FJsonValueObject(SaveWhiteList(&Data->WhiteLists[i]))));
	}
	JsonObject->SetArrayField("WhiteLists", WhiteLists);

	TArray< TSharedPtr<FJsonValue> > SectorsKnowledge;
	SectorsKnowledge.Reserve(Data->SectorsKnowledge.Num());
	for(int i = 0; i < Data->SectorsKnowledge.Num(); i++)
	{
		SectorsKnowledge.Add(MakeShareable(new FJsonValueObject(SaveSectorKnowledge(&Data->SectorsKnowledge[i]))));
	}
	JsonObject->SetArrayField("SectorsKnowledge", SectorsKnowledge);

	SaveFloat(JsonObject,"PlayerReputation", Data->PlayerReputation);

	TArray< TSharedPtr<FJsonValue> > TransactionLog;
	TransactionLog.Reserve(Data->TransactionLog.Num());
	for(int i = 0; i < Data->TransactionLog.Num(); i++)
	{
		TransactionLog.Add(MakeShareable(new FJsonValueObject(SaveTransactionLogEntry(&Data->TransactionLog[i]))));
	}
	JsonObject->SetArrayField("TransactionLog", TransactionLog);


	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraft(FFlareSpacecraftSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// TODO light save if destroyed
	JsonObject->SetBoolField("IsDestroyed", Data->IsDestroyed);
	JsonObject->SetBoolField("IsUnderConstruction", Data->IsUnderConstruction);
	JsonObject->SetStringField("Immatriculation", Data->Immatriculation.ToString());
	JsonObject->SetStringField("NickName", Data->NickName.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("CompanyIdentifier", Data->CompanyIdentifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("SpawnMode", FormatEnum<EFlareSpawnMode::Type>("EFlareSpawnMode",Data->SpawnMode));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetBoolField("WantUndockInternalShips", Data->WantUndockInternalShips);
	JsonObject->SetStringField("DockedTo", Data->DockedTo.ToString());
	JsonObject->SetStringField("DockedAt", FormatInt32(Data->DockedAt));
	JsonObject->SetStringField("DockedAtInternally", Data->DockedAtInternally.ToString());
	JsonObject->SetStringField("DefaultWhiteListIdentifier", Data->DefaultWhiteListIdentifier.ToString());
	SaveFloat(JsonObject,"DockedAngle", Data->DockedAngle);
	SaveFloat(JsonObject,"Heat", Data->Heat);
	SaveFloat(JsonObject,"PowerOutageDelay", Data->PowerOutageDelay);
	SaveFloat(JsonObject,"PowerOutageAcculumator", Data->PowerOutageAcculumator);
	JsonObject->SetStringField("DynamicComponentStateIdentifier", Data->DynamicComponentStateIdentifier.ToString());
	SaveFloat(JsonObject,"DynamicComponentStateProgress", Data->DynamicComponentStateProgress);
	JsonObject->SetStringField("Level", FormatInt32(Data->Level));
	JsonObject->SetStringField("TradingReason", FormatInt32(Data->TradingReason));
	JsonObject->SetBoolField("IsTrading", Data->IsTrading);
	JsonObject->SetBoolField("IsIntercepted", Data->IsIntercepted);
	SaveFloat(JsonObject,"RefillStock", Data->RefillStock);
	SaveFloat(JsonObject,"RepairStock", Data->RepairStock);
	JsonObject->SetBoolField("IsReserve", Data->IsReserve);
	JsonObject->SetBoolField("AllowExternalOrder", Data->AllowExternalOrder);
	JsonObject->SetBoolField("AllowAutoConstruction", Data->AllowAutoConstruction);
	JsonObject->SetObjectField("Pilot", SavePilot(&Data->Pilot));
	JsonObject->SetObjectField("Asteroid", SaveAsteroid(&Data->AsteroidData));
	JsonObject->SetStringField("HarpoonCompany", Data->HarpoonCompany.ToString());
	JsonObject->SetStringField("OwnerShipName", Data->OwnerShipName.ToString());
	JsonObject->SetStringField("AttachActorName", Data->AttachActorName.ToString());
	JsonObject->SetStringField("AttachComplexStationName", Data->AttachComplexStationName.ToString());
	JsonObject->SetStringField("AttachComplexConnectorName", Data->AttachComplexConnectorName.ToString());

	TArray< TSharedPtr<FJsonValue> > Components;
	Components.Reserve(Data->Components.Num());
	for(int i = 0; i < Data->Components.Num(); i++)
	{
		Components.Add(MakeShareable(new FJsonValueObject(SaveSpacecraftComponent(&Data->Components[i]))));
	}
	JsonObject->SetArrayField("Components", Components);

	TArray< TSharedPtr<FJsonValue> > ConstructionCargoBay;
	ConstructionCargoBay.Reserve(Data->ConstructionCargoBay.Num());
	for(int i = 0; i < Data->ConstructionCargoBay.Num(); i++)
	{
		ConstructionCargoBay.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->ConstructionCargoBay[i]))));
	}
	JsonObject->SetArrayField("ConstructionCargoBay", ConstructionCargoBay);

	TArray< TSharedPtr<FJsonValue> > ProductionCargoBay;
	ProductionCargoBay.Reserve(Data->ProductionCargoBay.Num());
	for(int i = 0; i < Data->ProductionCargoBay.Num(); i++)
	{
		ProductionCargoBay.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->ProductionCargoBay[i]))));
	}
	JsonObject->SetArrayField("ProductionCargoBay", ProductionCargoBay);

	TArray< TSharedPtr<FJsonValue> > FactoryStates;
	FactoryStates.Reserve(Data->FactoryStates.Num());
	for(int i = 0; i < Data->FactoryStates.Num(); i++)
	{
		FactoryStates.Add(MakeShareable(new FJsonValueObject(SaveFactory(&Data->FactoryStates[i]))));
	}
	JsonObject->SetArrayField("FactoryStates", FactoryStates);

	TArray< TSharedPtr<FJsonValue> > ShipyardOrderQueue;
	ShipyardOrderQueue.Reserve(Data->ShipyardOrderQueue.Num());
	for(int i = 0; i < Data->ShipyardOrderQueue.Num(); i++)
	{
		ShipyardOrderQueue.Add(MakeShareable(new FJsonValueObject(SaveShipyardOrderQueue(&Data->ShipyardOrderQueue[i]))));
	}
	JsonObject->SetArrayField("ShipyardOrderQueue", ShipyardOrderQueue);

	TArray< TSharedPtr<FJsonValue> > SalesExcludedResources;
	SalesExcludedResources.Reserve(Data->SalesExcludedResources.Num());
	for(int i = 0; i < Data->SalesExcludedResources.Num(); i++)
	{
		SalesExcludedResources.Add(MakeShareable(new FJsonValueString(Data->SalesExcludedResources[i].ToString())));
	}
	JsonObject->SetArrayField("SalesExcludedResources", SalesExcludedResources);

	TArray< TSharedPtr<FJsonValue> > ShipyardOrderExternalConfig;
	ShipyardOrderExternalConfig.Reserve(Data->ShipyardOrderExternalConfig.Num());
	for (int i = 0; i < Data->ShipyardOrderExternalConfig.Num(); i++)
	{
		ShipyardOrderExternalConfig.Add(MakeShareable(new FJsonValueString(Data->ShipyardOrderExternalConfig[i].ToString())));
	}
	JsonObject->SetArrayField("ShipyardOrderExternalConfig", ShipyardOrderExternalConfig);

	TArray< TSharedPtr<FJsonValue> > OwnedShipNames;
	OwnedShipNames.Reserve(Data->OwnedShipNames.Num());
	for (int i = 0; i < Data->OwnedShipNames.Num(); i++)
	{
		OwnedShipNames.Add(MakeShareable(new FJsonValueString(Data->OwnedShipNames[i].ToString())));
	}
	JsonObject->SetArrayField("OwnedShipNames", OwnedShipNames);

	TArray< TSharedPtr<FJsonValue> > ConnectedStations;
	ConnectedStations.Reserve(Data->ConnectedStations.Num());
	for (int i = 0; i < Data->ConnectedStations.Num(); i++)
	{
		ConnectedStations.Add(MakeShareable(new FJsonValueObject(SaveStationConnection(&Data->ConnectedStations[i]))));
	}
	JsonObject->SetArrayField("ConnectedStations", ConnectedStations);

	TArray<FName> CapturePointCompanies;
	Data->CapturePoints.GetKeys(CapturePointCompanies);
	TArray< TSharedPtr<FJsonValue> > CapturePoints;
	CapturePoints.Reserve(Data->CapturePoints.Num());
	for(int i = 0; i < Data->CapturePoints.Num(); i++)
	{
		FName Company = CapturePointCompanies[i];
		int32 Points = Data->CapturePoints[Company];

		TSharedRef<FJsonObject> JsonChildObject = MakeShareable(new FJsonObject());

		JsonChildObject->SetStringField("Company", Company.ToString());
		JsonChildObject->SetStringField("Points", FormatInt32(Points));

		CapturePoints.Add(MakeShareable(new FJsonValueObject(JsonChildObject)));
	}
	JsonObject->SetArrayField("CapturePoints", CapturePoints);
	JsonObject->SetStringField("SaveVersion", FormatInt32(Data->SaveVersion));
	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SavePilot(FFlareShipPilotSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Name", Data->Name);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveAsteroid(FFlareAsteroidSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("Scale", FormatVector(Data->Scale));
	JsonObject->SetStringField("AsteroidMeshID", FormatInt32(Data->AsteroidMeshID));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveMeteorite(FFlareMeteoriteSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	//JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("TargetOffset", FormatVector(Data->TargetOffset));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("MeteoriteMeshID", FormatInt32(Data->MeteoriteMeshID));
	JsonObject->SetStringField("IsMetal", FormatInt32(Data->IsMetal));
	SaveFloat(JsonObject,"Damage", Data->Damage);
	SaveFloat(JsonObject,"BrokenDamage", Data->BrokenDamage);
	JsonObject->SetStringField("TargetStation", Data->TargetStation.ToString());
	JsonObject->SetStringField("HasMissed", FormatInt32(Data->HasMissed));
	JsonObject->SetStringField("DaysBeforeImpact", FormatInt32(Data->DaysBeforeImpact));



	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraftComponent(FFlareSpacecraftComponentSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ComponentIdentifier", Data->ComponentIdentifier.ToString());
	JsonObject->SetStringField("ShipSlotIdentifier", Data->ShipSlotIdentifier.ToString());
	SaveFloat(JsonObject,"Damage", Data->Damage);
	JsonObject->SetObjectField("Turret", SaveSpacecraftComponentTurret(&Data->Turret));
	JsonObject->SetObjectField("Weapon", SaveSpacecraftComponentWeapon(&Data->Weapon));
	JsonObject->SetObjectField("Pilot", SaveTurretPilot(&Data->Pilot));
	
	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraftComponentTurret(FFlareSpacecraftComponentTurretSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	SaveFloat(JsonObject,"TurretAngle", Data->TurretAngle);
	SaveFloat(JsonObject,"BarrelsAngle", Data->BarrelsAngle);

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSpacecraftComponentWeapon(FFlareSpacecraftComponentWeaponSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("FiredAmmo", FormatInt32(Data->FiredAmmo));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveTurretPilot(FFlareTurretPilotSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Name", Data->Name);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTradeOperation(FFlareTradeRouteSectorOperationSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ResourceIdentifier", Data->ResourceIdentifier.ToString());
	JsonObject->SetStringField("GotoSectorIndex", FormatInt32(Data->GotoSectorIndex));
	JsonObject->SetStringField("GotoOperationIndex", FormatInt32(Data->GotoOperationIndex));

	JsonObject->SetStringField("MaxQuantity", FormatInt32(Data->MaxQuantity));
	JsonObject->SetStringField("InventoryLimit", FormatInt32(Data->InventoryLimit));
	JsonObject->SetStringField("MaxWait", FormatInt32(Data->MaxWait));
	JsonObject->SetStringField("Type", FormatEnum<EFlareTradeRouteOperation::Type>("EFlareTradeRouteOperation",Data->Type));

	SaveFloat(JsonObject, "LoadUnloadPriority", Data->LoadUnloadPriority);
	SaveFloat(JsonObject, "BuySellPriority", Data->BuySellPriority);
	JsonObject->SetBoolField("CanTradeWithStorages", Data->CanTradeWithStorages);
	JsonObject->SetBoolField("CanDonate", Data->CanDonate);

	TArray< TSharedPtr<FJsonValue> > Conditions;
	Conditions.Reserve(Data->OperationConditions.Num());
	for (int i = 0; i < Data->OperationConditions.Num(); i++)
	{
		Conditions.Add(MakeShareable(new FJsonValueObject(SaveOperationCondition(&Data->OperationConditions[i]))));
	}

	JsonObject->SetArrayField("Conditions", Conditions);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveOperationCondition(FFlareTradeRouteOperationConditionSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("Type", FormatEnum<EFlareTradeRouteOperationConditions::Type>("EFlareTradeRouteOperationConditions", Data->ConditionRequirement));
	SaveFloat(JsonObject, "ConditionPercentage", Data->ConditionPercentage);
	JsonObject->SetBoolField("SkipOnConditionFail", Data->SkipOnConditionFail);
	JsonObject->SetBoolField("BooleanOne", Data->BooleanOne);
	JsonObject->SetBoolField("BooleanTwo", Data->BooleanTwo);
	JsonObject->SetBoolField("BooleanThree", Data->BooleanThree);
	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCargo(FFlareCargoSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ResourceIdentifier", Data->ResourceIdentifier.ToString());
	JsonObject->SetStringField("Quantity", FormatInt32(Data->Quantity));
	JsonObject->SetStringField("Lock", FormatEnum<EFlareResourceLock::Type>("EFlareResourceLock",Data->Lock));
	JsonObject->SetStringField("Restriction", FormatEnum<EFlareResourceRestriction::Type>("EFlareResourceRestriction",Data->Restriction));

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveShipyardOrderQueue(FFlareShipyardOrderSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Company", Data->Company.ToString());
	JsonObject->SetStringField("ShipClass", Data->ShipClass.ToString());
	JsonObject->SetStringField("AdvancePayment", FormatInt32(Data->AdvancePayment));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveStationConnection(FFlareConnectionSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ConnectorName", Data->ConnectorName.ToString());
	JsonObject->SetStringField("StationIdentifier", Data->StationIdentifier.ToString());

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveFactory(FFlareFactorySave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetBoolField("Active", Data->Active);
	JsonObject->SetStringField("CostReserved", FormatInt32(Data->CostReserved));
	JsonObject->SetStringField("ProductedDuration", FormatInt64(Data->ProductedDuration));
	JsonObject->SetBoolField("InfiniteCycle", Data->InfiniteCycle);
	JsonObject->SetStringField("CycleCount", FormatInt32(Data->CycleCount));
	JsonObject->SetStringField("TargetShipClass", Data->TargetShipClass.ToString());
	JsonObject->SetStringField("TargetShipCompany", Data->TargetShipCompany.ToString());

	TArray< TSharedPtr<FJsonValue> > ResourceReserved;
	ResourceReserved.Reserve(Data->ResourceReserved.Num());
	for(int i = 0; i < Data->ResourceReserved.Num(); i++)
	{
		ResourceReserved.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->ResourceReserved[i]))));
	}
	JsonObject->SetArrayField("ResourceReserved", ResourceReserved);

	TArray< TSharedPtr<FJsonValue> > OutputCargoLimit;
	OutputCargoLimit.Reserve(Data->OutputCargoLimit.Num());
	for(int i = 0; i < Data->OutputCargoLimit.Num(); i++)
	{
		OutputCargoLimit.Add(MakeShareable(new FJsonValueObject(SaveCargo(&Data->OutputCargoLimit[i]))));
	}
	JsonObject->SetArrayField("OutputCargoLimit", OutputCargoLimit);

	return JsonObject;
}

//////////////////

TSharedRef<FJsonObject> UFlareSaveWriter::SaveFleet(FFlareFleetSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("DefaultWhiteListIdentifier", Data->DefaultWhiteListIdentifier.ToString());

	TArray< TSharedPtr<FJsonValue> > ShipImmatriculations;
	ShipImmatriculations.Reserve(Data->ShipImmatriculations.Num());
	for(int i = 0; i < Data->ShipImmatriculations.Num(); i++)
	{
		ShipImmatriculations.Add(MakeShareable(new FJsonValueString(Data->ShipImmatriculations[i].ToString())));
	}
	JsonObject->SetArrayField("ShipImmatriculations", ShipImmatriculations);
	JsonObject->SetStringField("FleetColor", FormatVector(UFlareGameTools::ColorToVector(Data->FleetColor)));
	JsonObject->SetBoolField("AutoTrade", Data->AutoTrade);
	JsonObject->SetBoolField("HideTravelList", Data->HideTravelList);

	JsonObject->SetStringField("AutoTradeStatsDays", FormatInt32(Data->AutoTradeStatsDays));
	JsonObject->SetStringField("AutoTradeStatsLoadResources", FormatInt32(Data->AutoTradeStatsLoadResources));
	JsonObject->SetStringField("AutoTradeStatsUnloadResources", FormatInt32(Data->AutoTradeStatsUnloadResources));
	JsonObject->SetStringField("AutoTradeStatsMoneySell", FormatInt64(Data->AutoTradeStatsMoneySell));
	JsonObject->SetStringField("AutoTradeStatsMoneyBuy", FormatInt64(Data->AutoTradeStatsMoneyBuy));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveWhiteList(FFlareWhiteListSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	TArray< TSharedPtr<FJsonValue> > CompanyData;
	CompanyData.Reserve(Data->CompanyData.Num());
	for (int i = 0; i < Data->CompanyData.Num(); i++)
	{
		CompanyData.Add(MakeShareable(new FJsonValueObject(SaveWhiteListCompanyData(&Data->CompanyData[i]))));
	}

	JsonObject->SetArrayField("CompanyData", CompanyData);
	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveWhiteListCompanyData(FFlareWhiteListCompanyDataSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetBoolField("CanTradeTo", Data->CanTradeTo);
	JsonObject->SetBoolField("CanTradeFrom", Data->CanTradeFrom);

	TArray< TSharedPtr<FJsonValue> > ResourcesTradeFrom;
	ResourcesTradeFrom.Reserve(Data->ResourcesTradeFrom.Num());
	for (int i = 0; i < Data->ResourcesTradeFrom.Num(); i++)
	{
		ResourcesTradeFrom.Add(MakeShareable(new FJsonValueString(Data->ResourcesTradeFrom[i].ToString())));
	}
	JsonObject->SetArrayField("ResourcesTradeFrom", ResourcesTradeFrom);

	TArray< TSharedPtr<FJsonValue> > ResourcesTradeTo;
	ResourcesTradeTo.Reserve(Data->ResourcesTradeTo.Num());
	for (int i = 0; i < Data->ResourcesTradeTo.Num(); i++)
	{
		ResourcesTradeTo.Add(MakeShareable(new FJsonValueString(Data->ResourcesTradeTo[i].ToString())));
	}
	JsonObject->SetArrayField("ResourcesTradeTo", ResourcesTradeTo);
	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTradeRoute(FFlareTradeRouteSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Name", Data->Name.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("FleetIdentifier", Data->FleetIdentifier.ToString());
	JsonObject->SetStringField("TargetSectorIdentifier", Data->TargetSectorIdentifier.ToString());
	JsonObject->SetStringField("CurrentOperationIndex", FormatInt32(Data->CurrentOperationIndex));
	JsonObject->SetStringField("CurrentOperationProgress", FormatInt32(Data->CurrentOperationProgress));
	JsonObject->SetStringField("CurrentOperationDuration", FormatInt32(Data->CurrentOperationDuration));
	JsonObject->SetBoolField("IsPaused", Data->IsPaused);


	// Stats
	JsonObject->SetStringField("StatsDays", FormatInt32(Data->StatsDays));
	JsonObject->SetStringField("StatsLoadResources", FormatInt32(Data->StatsLoadResources));
	JsonObject->SetStringField("StatsUnloadResources", FormatInt32(Data->StatsUnloadResources));
	JsonObject->SetStringField("StatsMoneySell", FormatInt64(Data->StatsMoneySell));
	JsonObject->SetStringField("StatsMoneyBuy", FormatInt64(Data->StatsMoneyBuy));
	JsonObject->SetStringField("StatsOperationSuccessCount", FormatInt32(Data->StatsOperationSuccessCount));
	JsonObject->SetStringField("StatsOperationFailCount", FormatInt32(Data->StatsOperationFailCount));

	TArray< TSharedPtr<FJsonValue> > Sectors;
	Sectors.Reserve(Data->Sectors.Num());
	for(int i = 0; i < Data->Sectors.Num(); i++)
	{
		Sectors.Add(MakeShareable(new FJsonValueObject(SaveTradeRouteSector(&Data->Sectors[i]))));
	}
	JsonObject->SetArrayField("Sectors", Sectors);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTradeRouteSector(FFlareTradeRouteSectorSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("SectorIdentifier", Data->SectorIdentifier.ToString());


	TArray< TSharedPtr<FJsonValue> > Operations;
	Operations.Reserve(Data->Operations.Num());
	for(int i = 0; i < Data->Operations.Num(); i++)
	{
		Operations.Add(MakeShareable(new FJsonValueObject(SaveTradeOperation(&Data->Operations[i]))));
	}
	JsonObject->SetArrayField("Operations", Operations);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveSectorKnowledge(FFlareCompanySectorKnowledge* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("SectorIdentifier", Data->SectorIdentifier.ToString());
	JsonObject->SetStringField("Knowledge", FormatEnum<EFlareSectorKnowledge::Type>("EFlareSectorKnowledge",Data->Knowledge));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTransactionLogEntry(FFlareTransactionLogEntry* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Date", FormatInt64(Data->Date));
	JsonObject->SetStringField("Amount", FormatInt64(Data->Amount));
	JsonObject->SetStringField("Type", FormatEnum<EFlareTransactionLogEntry::Type>("EFlareTransactionLogEntry",Data->Type));
	JsonObject->SetStringField("Spacecraft", Data->Spacecraft.ToString());
	JsonObject->SetStringField("Sector", Data->Sector.ToString());
	JsonObject->SetStringField("OtherCompany", Data->OtherCompany.ToString());
	JsonObject->SetStringField("OtherSpacecraft", Data->OtherSpacecraft.ToString());
	JsonObject->SetStringField("Resource", Data->Resource.ToString());
	JsonObject->SetStringField("Spacecraft", Data->Spacecraft.ToString());
	JsonObject->SetStringField("ResourceQuantity", FormatInt32(Data->ResourceQuantity));
	JsonObject->SetStringField("ExtraIdentifier1", Data->ExtraIdentifier1.ToString());
	JsonObject->SetStringField("ExtraIdentifier2", Data->ExtraIdentifier2.ToString());

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyAI(FFlareCompanyAISave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("BudgetMilitary", FormatInt64(Data->BudgetMilitary));
	JsonObject->SetStringField("BudgetStation", FormatInt64(Data->BudgetStation));
	JsonObject->SetStringField("BudgetTechnology", FormatInt64(Data->BudgetTechnology));
	JsonObject->SetStringField("BudgetTrade", FormatInt64(Data->BudgetTrade));
	JsonObject->SetStringField("DateBoughtLicense", FormatInt64(Data->DateBoughtLicense));
	SaveFloat(JsonObject,"Caution", Data->Caution);
	SaveFloat(JsonObject,"Pacifism", Data->Pacifism);
	JsonObject->SetStringField("ResearchProject", Data->ResearchProject.ToString());
	JsonObject->SetStringField("DesiredStationLicense", Data->DesiredStationLicense.ToString());
	JsonObject->SetBoolField("CalculatedDefaultBudget", Data->CalculatedDefaultBudget);
	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyLicenses(FFlareCompanyLicensesSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	TArray< TSharedPtr<FJsonValue> > LicenseBuilding;
	LicenseBuilding.Reserve(Data->LicenseBuilding.Num());
	for (int i = 0; i < Data->LicenseBuilding.Num(); i++)
	{
		LicenseBuilding.Add(MakeShareable(new FJsonValueString(Data->LicenseBuilding[i].ToString())));
	}

	JsonObject->SetArrayField("LicenseBuilding", LicenseBuilding);
	JsonObject->SetBoolField("HasRecievedStartingLicenses", Data->HasRecievedStartingLicenses);
	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveCompanyReputation(FFlareCompanyReputationSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("CompanyIdentifier", Data->CompanyIdentifier.ToString());
	SaveFloat(JsonObject,"Reputation", Data->Reputation);

	return JsonObject;
}


TSharedRef<FJsonObject> UFlareSaveWriter::SaveSector(FFlareSectorSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("GivenName", Data->GivenName.ToString());
	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("LocalTime", FormatInt64(Data->LocalTime));
	JsonObject->SetObjectField("People", SavePeople(&Data->PeopleData));


	TArray< TSharedPtr<FJsonValue> > Bombs;
	Bombs.Reserve(Data->BombData.Num());
	for(int i = 0; i < Data->BombData.Num(); i++)
	{
		Bombs.Add(MakeShareable(new FJsonValueObject(SaveBomb(&Data->BombData[i]))));
	}
	JsonObject->SetArrayField("Bombs", Bombs);


	TArray< TSharedPtr<FJsonValue> > Asteroids;
	Asteroids.Reserve(Data->AsteroidData.Num());
	for(int i = 0; i < Data->AsteroidData.Num(); i++)
	{
		Asteroids.Add(MakeShareable(new FJsonValueObject(SaveAsteroid(&Data->AsteroidData[i]))));
	}
	JsonObject->SetArrayField("Asteroids", Asteroids);

	TArray< TSharedPtr<FJsonValue> > Meteorites;
	Meteorites.Reserve(Data->MeteoriteData.Num());
	for(int i = 0; i < Data->MeteoriteData.Num(); i++)
	{
		Meteorites.Add(MakeShareable(new FJsonValueObject(SaveMeteorite(&Data->MeteoriteData[i]))));
	}
	JsonObject->SetArrayField("Meteorites", Meteorites);

	TArray< TSharedPtr<FJsonValue> > FleetIdentifiers;
	FleetIdentifiers.Reserve(Data->FleetIdentifiers.Num());
	for(int i = 0; i < Data->FleetIdentifiers.Num(); i++)
	{
		FleetIdentifiers.Add(MakeShareable(new FJsonValueString(Data->FleetIdentifiers[i].ToString())));
	}
	JsonObject->SetArrayField("FleetIdentifiers", FleetIdentifiers);

	TArray< TSharedPtr<FJsonValue> > SpacecraftIdentifiers;
	SpacecraftIdentifiers.Reserve(Data->SpacecraftIdentifiers.Num());
	for(int i = 0; i < Data->SpacecraftIdentifiers.Num(); i++)
	{
		SpacecraftIdentifiers.Add(MakeShareable(new FJsonValueString(Data->SpacecraftIdentifiers[i].ToString())));
	}
	JsonObject->SetArrayField("SpacecraftIdentifiers", SpacecraftIdentifiers);


	TArray< TSharedPtr<FJsonValue> > ResourcePrices;
	ResourcePrices.Reserve(Data->ResourcePrices.Num());
	for(int i = 0; i < Data->ResourcePrices.Num(); i++)
	{
		ResourcePrices.Add(MakeShareable(new FJsonValueObject(SaveResourcePrice(&Data->ResourcePrices[i]))));
	}
	JsonObject->SetArrayField("ResourcePrices", ResourcePrices);

	JsonObject->SetBoolField("IsTravelSector", Data->IsTravelSector);

	JsonObject->SetObjectField("FleetSupplyConsumptionStats", SaveFloatBuffer(&Data->FleetSupplyConsumptionStats));
	JsonObject->SetStringField("DailyFleetSupplyConsumption", FormatInt32(Data->DailyFleetSupplyConsumption));

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SavePeople(FFlarePeopleSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Population", FormatInt32(Data->Population));
	JsonObject->SetStringField("FoodStock", FormatInt32(Data->FoodStock));
	JsonObject->SetStringField("FuelStock", FormatInt32(Data->FuelStock));
	JsonObject->SetStringField("ToolStock", FormatInt32(Data->ToolStock));
	JsonObject->SetStringField("TechStock", FormatInt32(Data->TechStock));
	SaveFloat(JsonObject,"FoodConsumption", Data->FoodConsumption);
	SaveFloat(JsonObject,"FuelConsumption", Data->FuelConsumption);
	SaveFloat(JsonObject,"ToolConsumption", Data->ToolConsumption);
	SaveFloat(JsonObject,"TechConsumption", Data->TechConsumption);
	JsonObject->SetStringField("Money", FormatInt32(Data->Money));
	JsonObject->SetStringField("Dept", FormatInt32(Data->Dept));
	JsonObject->SetStringField("BirthPoint", FormatInt32(Data->BirthPoint));
	JsonObject->SetStringField("DeathPoint", FormatInt32(Data->DeathPoint));
	JsonObject->SetStringField("HungerPoint", FormatInt32(Data->HungerPoint));
	JsonObject->SetStringField("HappinessPoint", FormatInt32(Data->HappinessPoint));

	TArray< TSharedPtr<FJsonValue> > CompanyReputations;
	CompanyReputations.Reserve(Data->CompanyReputations.Num());
	for(int i = 0; i < Data->CompanyReputations.Num(); i++)
	{
		CompanyReputations.Add(MakeShareable(new FJsonValueObject(SaveCompanyReputation(&Data->CompanyReputations[i]))));
	}
	JsonObject->SetArrayField("CompanyReputations", CompanyReputations);


	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveBomb(FFlareBombSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("Identifier", Data->Identifier.ToString());
	JsonObject->SetStringField("Location", FormatVector(Data->Location));
	JsonObject->SetStringField("Rotation", FormatRotator(Data->Rotation));
	JsonObject->SetStringField("LinearVelocity", FormatVector(Data->LinearVelocity));
	JsonObject->SetStringField("AngularVelocity", FormatVector(Data->AngularVelocity));
	JsonObject->SetStringField("WeaponSlotIdentifier", Data->WeaponSlotIdentifier.ToString());
	JsonObject->SetStringField("AimTargetSpacecraft", Data->AimTargetSpacecraft.ToString());
	JsonObject->SetStringField("ParentSpacecraft", Data->ParentSpacecraft.ToString());
	JsonObject->SetStringField("AttachTarget", Data->AttachTarget.ToString());
	JsonObject->SetBoolField("Activated", Data->Activated);
	JsonObject->SetBoolField("Dropped", Data->Dropped);
	JsonObject->SetBoolField("Locked", Data->Locked);
	SaveFloat(JsonObject,"DropParentDistance", Data->DropParentDistance);
	SaveFloat(JsonObject,"LifeTime", Data->LifeTime);
	SaveFloat(JsonObject,"BurnDuration", Data->BurnDuration);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveResourcePrice(FFFlareResourcePrice* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("ResourceIdentifier", Data->ResourceIdentifier.ToString());
	SaveFloat(JsonObject,"Price", Data->Price);
	JsonObject->SetObjectField("Prices", SaveFloatBuffer(&Data->Prices));


	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveFloatBuffer(FFlareFloatBuffer* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("MaxSize", FormatInt32(Data->MaxSize));
	JsonObject->SetStringField("WriteIndex", FormatInt32(Data->WriteIndex));



	TArray< TSharedPtr<FJsonValue> > Values;
	Values.Reserve(Data->Values.Num());
	for(int i = 0; i < Data->Values.Num(); i++)
	{
		Values.Add(MakeShareable(new FJsonValueNumber(Data->Values[i])));
	}
	JsonObject->SetArrayField("Values", Values);

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveBundle(FFlareBundle* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if(Data->FloatValues.Num() > 0)
	{
		TSharedRef<FJsonObject> FloatObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->FloatValues)
		{
			FloatObject->SetNumberField(Pair.Key.ToString(), FixFloat(Pair.Value));
		}
		JsonObject->SetObjectField("FloatValues", FloatObject);
	}

	if(Data->Int32Values.Num() > 0)
	{
		TSharedRef<FJsonObject> Int32Object = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->Int32Values)
		{
			Int32Object->SetStringField(Pair.Key.ToString(), FormatInt32(Pair.Value));
		}
		JsonObject->SetObjectField("Int32Values", Int32Object);
	}

	if(Data->TransformValues.Num() > 0)
	{
		TSharedRef<FJsonObject> TransformObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->TransformValues)
		{
			TransformObject->SetStringField(Pair.Key.ToString(), FormatTransform(Pair.Value));
		}
		JsonObject->SetObjectField("TransformValues", TransformObject);
	}

	if(Data->VectorArrayValues.Num() > 0)
	{
		TSharedRef<FJsonObject> TransformObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->VectorArrayValues)
		{
			TArray< TSharedPtr<FJsonValue> > VectorArray;
			for(FVector Vector: Pair.Value.Entries)
			{
				VectorArray.Add(MakeShareable(new FJsonValueString(FormatVector(Vector))));
			}

			TransformObject->SetArrayField(Pair.Key.ToString(), VectorArray);
		}
		JsonObject->SetObjectField("VectorArrayValues", TransformObject);
	}

	if(Data->NameValues.Num() > 0)
	{
		TSharedRef<FJsonObject> Int32Object = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->NameValues)
		{
			Int32Object->SetStringField(Pair.Key.ToString(), Pair.Value.ToString());
		}
		JsonObject->SetObjectField("NameValues", Int32Object);
	}
	
	if(Data->NameArrayValues.Num() > 0)
	{
		TSharedRef<FJsonObject> TransformObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->NameArrayValues)
		{
			TArray< TSharedPtr<FJsonValue> > NameArray;
			for(FName Name: Pair.Value.Entries)
			{
				NameArray.Add(MakeShareable(new FJsonValueString(Name.ToString())));
			}

			TransformObject->SetArrayField(Pair.Key.ToString(), NameArray);
		}
		JsonObject->SetObjectField("NameArrayValues", TransformObject);
	}

	if (Data->StringValues.Num() > 0)
	{
		TSharedRef<FJsonObject> StringObject = MakeShareable(new FJsonObject());
		for (auto& Pair : Data->StringValues)
		{
			StringObject->SetStringField(Pair.Key.ToString(), Pair.Value);
		}
		JsonObject->SetObjectField("StringValues", StringObject);
	}

	if (Data->Tags.Num() > 0)
	{
		TArray< TSharedPtr<FJsonValue> > Tags;
		Tags.Reserve(Data->Tags.Num());
		for(int i = 0; i < Data->Tags.Num(); i++)
		{
			Tags.Add(MakeShareable(new FJsonValueString(Data->Tags[i].ToString())));
		}
		JsonObject->SetArrayField("Tags", Tags);
	}

	return JsonObject;
}

TSharedRef<FJsonObject> UFlareSaveWriter::SaveTravel(FFlareTravelSave* Data)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("FleetIdentifier", Data->FleetIdentifier.ToString());
	JsonObject->SetStringField("OriginSectorIdentifier", Data->OriginSectorIdentifier.ToString());
	JsonObject->SetStringField("DestinationSectorIdentifier", Data->DestinationSectorIdentifier.ToString());
	JsonObject->SetStringField("DepartureDate", FormatInt64(Data->DepartureDate));

	JsonObject->SetObjectField("SectorData", SaveSector(&Data->SectorData));

	return JsonObject;
}

void UFlareSaveWriter::SaveFloat(TSharedPtr< FJsonObject > Object, FString Key, float Data)
{
	Object->SetNumberField(Key, FixFloat(Data));
}
