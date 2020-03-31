
#include "FlareQuestGenerator.h"
#include "Flare.h"

#include "../Data/FlareResourceCatalog.h"

#include "../Economy/FlareCargoBay.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlareSimulatedSector.h"
#include "../Game/FlareSectorHelper.h"

#include "../Player/FlarePlayerController.h"


#include "FlareQuestManager.h"
#include "FlareQuestCondition.h"
#include "FlareQuest.h"
#include "FlareQuestStep.h"
#include "FlareQuestAction.h"
#include "QuestCatalog/FlareTutorialQuest.h"

#define LOCTEXT_NAMESPACE "FlareQuestGenerator"

#define MAX_QUEST_COUNT 40
#define MAX_QUEST_PER_COMPANY_COUNT 10

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareQuestGenerator::UFlareQuestGenerator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareQuestGenerator::Load(UFlareQuestManager* Parent, const FFlareQuestSave& Data)
{
	QuestManager = Parent;
	Game = Parent->GetGame();
	NextQuestIndex = Data.NextGeneratedQuestIndex;
}

void UFlareQuestGenerator::LoadQuests(const FFlareQuestSave& Data)
{
	for(const FFlareGeneratedQuestSave& QuestData : Data.GeneratedQuests) {

		UFlareQuestGenerated* Quest = NULL;
		if(QuestData.QuestClass == UFlareQuestGeneratedVipTransport::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedVipTransport>(this, UFlareQuestGeneratedVipTransport::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedResourceSale::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedResourceSale>(this, UFlareQuestGeneratedResourceSale::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedResourcePurchase::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedResourcePurchase>(this, UFlareQuestGeneratedResourcePurchase::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedResourceTrade::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedResourceTrade>(this, UFlareQuestGeneratedResourceTrade::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedStationDefense::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedStationDefense>(this, UFlareQuestGeneratedStationDefense::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedSectorDefense::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedSectorDefense>(this, UFlareQuestGeneratedSectorDefense::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedJoinAttack::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedJoinAttack>(this, UFlareQuestGeneratedJoinAttack::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedCargoHunt::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedCargoHunt>(this, UFlareQuestGeneratedCargoHunt::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedMilitaryHunt::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedMilitaryHunt>(this, UFlareQuestGeneratedMilitaryHunt::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedStationDefense2::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedStationDefense2>(this, UFlareQuestGeneratedStationDefense2::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedSectorDefense2::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedSectorDefense2>(this, UFlareQuestGeneratedSectorDefense2::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedJoinAttack2::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedJoinAttack2>(this, UFlareQuestGeneratedJoinAttack2::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedCargoHunt2::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedCargoHunt2>(this, UFlareQuestGeneratedCargoHunt2::StaticClass());
		}
		else if(QuestData.QuestClass == UFlareQuestGeneratedMilitaryHunt2::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedMilitaryHunt2>(this, UFlareQuestGeneratedMilitaryHunt2::StaticClass());
		}

		else if(QuestData.QuestClass == UFlareQuestGeneratedMeteoriteInterception::GetClass())
		{
			Quest = NewObject<UFlareQuestGeneratedMeteoriteInterception>(this, UFlareQuestGeneratedMeteoriteInterception::StaticClass());
		}
		else
		{
			FLOGV("ERROR: not loaded quest %s", *QuestData.QuestClass.ToString());
			continue;
		}

		// Attempt loading
		bool LoadSuccessful = Quest->Load(this, QuestData.Data);
		if (LoadSuccessful)
		{
			QuestManager->AddQuest(Quest);
			GeneratedQuests.Add(Quest);
		}
	}
}

void UFlareQuestGenerator::Save(FFlareQuestSave& Data)
{
	Data.NextGeneratedQuestIndex = NextQuestIndex;
	Data.GeneratedQuests.Empty();
	for(UFlareQuestGenerated* Quest : GeneratedQuests)
	{
		FFlareGeneratedQuestSave QuestData;
		QuestData.QuestClass = Quest->GetQuestClass();
		QuestData.Data = *Quest->GetInitData();
		Data.GeneratedQuests.Add(QuestData);
	}
}

/*----------------------------------------------------
	Quest generation
----------------------------------------------------*/

FText UFlareQuestGenerator::GeneratePersonName(TArray<FString> UsedNames)
{
	TArray<FText> Names;
	
	Names.Reserve(55);
	Names.Add(FText::FromString("Amos"));
	Names.Add(FText::FromString("Anette"));
	Names.Add(FText::FromString("Alex"));
	Names.Add(FText::FromString("Arya"));
	Names.Add(FText::FromString("Bender"));
	Names.Add(FText::FromString("Bertrand"));
	Names.Add(FText::FromString("Bob"));
	Names.Add(FText::FromString("Calvin"));
	Names.Add(FText::FromString("Chiku"));
	Names.Add(FText::FromString("Christian"));
	Names.Add(FText::FromString("Curtis"));
	Names.Add(FText::FromString("David"));
	Names.Add(FText::FromString("Daisy"));
	Names.Add(FText::FromString("Elisa"));
	Names.Add(FText::FromString("France"));
	Names.Add(FText::FromString("Fred"));
	Names.Add(FText::FromString("Gaius"));
	Names.Add(FText::FromString("Galiana"));
	Names.Add(FText::FromString("Geoffrey"));
	Names.Add(FText::FromString("Gwenn"));
	Names.Add(FText::FromString("Hans"));
	Names.Add(FText::FromString("Ilia")); // Revelation's space
	Names.Add(FText::FromString("Ilya")); // Nexus
	Names.Add(FText::FromString("Ivy"));
	Names.Add(FText::FromString("Jebediah"));
	Names.Add(FText::FromString("Jim"));
	Names.Add(FText::FromString("Johanna"));
	Names.Add(FText::FromString("Katia"));
	Names.Add(FText::FromString("Kaden"));
	Names.Add(FText::FromString("Lucy"));
	Names.Add(FText::FromString("Mary"));
	Names.Add(FText::FromString("Miller"));
	Names.Add(FText::FromString("Nate"));
	Names.Add(FText::FromString("Naomi"));
	Names.Add(FText::FromString("Olly"));
	Names.Add(FText::FromString("Paula"));
	Names.Add(FText::FromString("Paul"));
	Names.Add(FText::FromString("Quinn"));
	Names.Add(FText::FromString("Rangan"));
	Names.Add(FText::FromString("Robb"));
	Names.Add(FText::FromString("Saul"));
	Names.Add(FText::FromString("Sam"));
	Names.Add(FText::FromString("Scorpio"));
	Names.Add(FText::FromString("Sharon"));
	Names.Add(FText::FromString("Shu"));
	Names.Add(FText::FromString("Simone"));
	Names.Add(FText::FromString("Sunday"));
	Names.Add(FText::FromString("Tilda"));
	Names.Add(FText::FromString("Tor"));
	Names.Add(FText::FromString("Ulysse"));
	Names.Add(FText::FromString("Val"));
	Names.Add(FText::FromString("Viktor"));
	Names.Add(FText::FromString("Walter"));
	Names.Add(FText::FromString("Xavier"));
	Names.Add(FText::FromString("Yann"));
	Names.Add(FText::FromString("Zoe"));

	TArray<FText> NotUsedNames;

	for(FText Name: Names)
	{
		if(!UsedNames.Contains(Name.ToString()))
		{
			NotUsedNames.Add(Name);
		}
	}

	if(NotUsedNames.Num())
	{
		return NotUsedNames[FMath::RandHelper(NotUsedNames.Num() - 1)];
	}

	return Names[FMath::RandHelper(Names.Num() - 1)];
}

bool UFlareQuestGenerator::IsGenerationEnabled()
{
	UFlareQuest* Target = QuestManager->FindQuest("tutorial-navigation");

	if (Target)
	{
		return QuestManager->IsQuestSuccessfull(Target);
	}
	return true;
}


void UFlareQuestGenerator::GenerateIdentifer(FName QuestClass, FFlareBundle& Data)
{
	FName Identifier = FName(*(QuestClass.ToString() + "-" + FString::FromInt(NextQuestIndex++)));
	Data.PutName("identifier", Identifier);
}

float UFlareQuestGenerator::ComputeQuestProbability(UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	// Propability to give a quest is :
	// 100 % if quest count = 0
	// 0 % if quest count = MAX_QUEST_COUNT
	// With early quick drop
	// So pow(1-questCount/MAX_QUEST_COUNT, 2)
	float CompanyQuestProbability = 1.f - (float) QuestManager->GetVisibleQuestCount(Company)/ (float) MAX_QUEST_PER_COMPANY_COUNT;
	float GlobalQuestProbability = 1.f - (float) QuestManager->GetVisibleQuestCount()/ (float) MAX_QUEST_COUNT;

	FFlarePlayerSave* PlayerData = Game->GetPC()->GetPlayerData();
	int32 GameDifficulty = -1;
	if (PlayerData)
	{
		GameDifficulty = PlayerData->DifficultyId;
	}

	float DifficultyModifier = 1.0f;

	switch (GameDifficulty)
	{
	case -1: // Easy
		DifficultyModifier = 1.0f;
		break;
	case 0: // Normal
		DifficultyModifier = 1.0f;
		break;
	case 1: // Hard
		DifficultyModifier = 0.85f;
		break;
	case 2: // Very Hard
		DifficultyModifier = 0.70f;
		break;
	case 3: // Expert
		DifficultyModifier = 0.55f;
		break;
	case 4: // Unfair
		DifficultyModifier = 0.40f;
		break;
	}

	float QuestProbability = (GlobalQuestProbability * CompanyQuestProbability) * DifficultyModifier;


	/*FLOGV("CompanyQuestProbability %f", CompanyQuestProbability);
	FLOGV("GlobalQuestProbability %f", GlobalQuestProbability);
	FLOGV("QuestProbability before rep %f", QuestProbability);*/

	return QuestProbability;
}

void UFlareQuestGenerator::GenerateSectorQuest(UFlareSimulatedSector* Sector)
{
	if (!IsGenerationEnabled())
	{
		return;
	}

	TArray<UFlareCompany*> CompaniesToProcess =  Game->GetGameWorld()->GetCompanies();
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	TArray<FFlareResourceDescription*> AvailableResources;

	for (int32 SectorIndex = 0; SectorIndex < PlayerCompany->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* KnownSector = PlayerCompany->GetKnownSectors()[SectorIndex];
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats1 = SectorHelper::ComputeSectorResourceStats(KnownSector, false);

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;


			if(AvailableResources.Contains(Resource))
			{
				continue;
			}

			if(Stats1[Resource].Production > 0)
			{
				AvailableResources.Add(Resource);
				//FLOGV("%s is available in %s %d", *Resource->Identifier.ToString(), *KnownSector->GetSectorName().ToString(),
				//	Stats1[Resource].Production);
			}
		}
	}


	// For each company in random order
	while (CompaniesToProcess.Num() > 0)
	{
		UFlareQuestGenerated* Quest = NULL;

		// Get a company
		int CompanyIndex = FMath::RandRange(0, CompaniesToProcess.Num() - 1);
		UFlareCompany* Company = CompaniesToProcess[CompanyIndex];
		CompaniesToProcess.RemoveSwap(Company);
		if (Company == PlayerCompany)
		{
			continue;
		}

		// No luck, no quest this time
		if (FMath::FRand() > ComputeQuestProbability(Company))
		{
			continue;
		}

		// Generate a VIP quest
		if (FMath::FRand() < 0.15)
		{
			Quest = UFlareQuestGeneratedVipTransport::Create(this, Sector, Company);
		}

		// Trade quests
		if (Game->GetPC()->GetPlayerFleet()->GetFleetCapacity() > 0)
		{
			// Try to create a trade quest,
			// if not, try to create a Purchase quest
			// if not, try to create a Sell quest
			if (!Quest)
			{
				Quest = UFlareQuestGeneratedResourceTrade::Create(this, Sector, Company);
			}

			if (!Quest)
			{
				Quest = UFlareQuestGeneratedResourcePurchase::Create(this, Sector, Company, AvailableResources);
			}

			if (!Quest)
			{
				Quest = UFlareQuestGeneratedResourceSale::Create(this, Sector, Company);
			}
		}

		// VIP strikes again
		if (!Quest && (FMath::FRand() < 0.3 || QuestManager->GetVisibleQuestCount() == 0) )
		{
			Quest = UFlareQuestGeneratedVipTransport::Create(this, Sector, Company);
		}

		// Register quest
		RegisterQuest(Quest);
		
		if (Game->GetPC()->GetCompany()->GetCompanyValue().ArmyCurrentCombatPoints > 0)
		{
			// Compute values
			int64 TotalValue = 0;
			int64 TotalCombatValue = 0;
			CompanyValue Value = Company->GetCompanyValue();
			for (UFlareCompany* OtherCompany : GetGame()->GetGameWorld()->GetCompanies())
			{
				const CompanyValue& OtherValue = OtherCompany->GetCompanyValue();
				TotalValue += OtherValue.TotalValue;
				TotalCombatValue += OtherValue.ArmyCurrentCombatPoints;
			}

			// Hunt quests
			for (UFlareCompany* OtherCompany : GetGame()->GetGameWorld()->GetCompanies())
			{
				if (OtherCompany == PlayerCompany || Company == OtherCompany)
				{
					continue;
				}

				CompanyValue OtherValue = OtherCompany->GetCompanyValue();

				// Cargo hunt
				if (OtherValue.TotalValue > Value.TotalValue * 1.1)
				{
					// 1% per day per negative reputation point
					float ValueRatio = float(OtherValue.TotalValue) / TotalValue;

					float CargoHuntQuestProbability = FMath::Clamp(FMath::Square(ValueRatio) * 0.5f, 0.f, 1.f);

					// No luck, no quest this time
					if (FMath::FRand() > CargoHuntQuestProbability)
					{
						continue;
					}

					RegisterQuest(UFlareQuestGeneratedCargoHunt2::Create(this, Company, OtherCompany));
				}

				// Military hunt
				if (OtherValue.ArmyCurrentCombatPoints > Value.ArmyCurrentCombatPoints*1.1)
				{
					float ValueRatio = float(OtherValue.ArmyCurrentCombatPoints) / TotalCombatValue;

					// 1% per day per negative reputation point
					float MilitaryHuntQuestProbability = FMath::Clamp(FMath::Square(ValueRatio) * 0.5f, 0.f, 1.f);

					// No luck, no quest this time
					if (FMath::FRand() > MilitaryHuntQuestProbability)
					{
						continue;
					}

					RegisterQuest(UFlareQuestGeneratedMilitaryHunt2::Create(this, Company, OtherCompany));
				}
			}
		}
	}
}

void UFlareQuestGenerator::GenerateAttackQuests(UFlareCompany* AttackCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration)
{
	if(!IsGenerationEnabled())
	{
		return;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if(Target.ArmedDefenseCompanies.Num() == 1 && Target.ArmedDefenseCompanies[0] == PlayerCompany)
	{
		// Only attack the player
		return;
	}


	int64 PlayerTravelDuration = 0;
	UFlareFleet* PlayerFleet = GetGame()->GetPC()->GetPlayerFleet();

	if(PlayerFleet->IsTraveling())
	{
		PlayerTravelDuration +=	PlayerFleet->GetCurrentTravel()->GetRemainingTravelDuration();
		PlayerTravelDuration +=	UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), PlayerFleet->GetCurrentTravel()->GetDestinationSector(), Target.Sector, PlayerCompany);
	}
	else
	{
		PlayerTravelDuration +=	UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), PlayerFleet->GetCurrentSector(), Target.Sector, PlayerCompany);
	}

	if(PlayerTravelDuration > TravelDuration)
	{
		return;
	}

	// Attack quest
	if (FMath::FRand() <= ComputeQuestProbability(AttackCompany))
	{
		RegisterQuest(UFlareQuestGeneratedJoinAttack2::Create(this, AttackCompany, AttackCombatPoints, Target, TravelDuration));
	}

	// Defense quest
	for(UFlareCompany* DefenseCompany : Target.ArmedDefenseCompanies)
	{
		if(DefenseCompany == PlayerCompany)
		{
			continue;
		}

		if (FMath::FRand() <= ComputeQuestProbability(DefenseCompany))
		{
			RegisterQuest(UFlareQuestGeneratedSectorDefense2::Create(this, DefenseCompany, AttackCompany, AttackCombatPoints, Target, TravelDuration));
		}
	}
}

void UFlareQuestGenerator::GenerateMilitaryQuests()
{
	if(!IsGenerationEnabled())
	{
		return;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	// Defense quests

	for(UFlareCompany* Company : GetGame()->GetGameWorld()->GetCompanies())
	{	
		if(Company == PlayerCompany)
		{
			continue;
		}

		float QuestProbability = ComputeQuestProbability(Company);
		CompanyValue Value = Company->GetCompanyValue();


		//FLOGV("Militaty QuestProbability for %s: %f", *Company->GetCompanyName().ToString(), QuestProbability);

		// Rand
		if (FMath::FRand() > QuestProbability)
		{
			// No luck, no quest this time
			continue;
		}

		// Station defense
		for (UFlareSimulatedSpacecraft* Station : Company->GetCompanyStations())
		{
			for(UFlareCompany* HostileCompany : GetGame()->GetGameWorld()->GetCompanies())
			{
				if(HostileCompany == PlayerCompany)
				{
					continue;
				}

				if (Station->GetCapturePoint(HostileCompany) * 10 > Station->GetCapturePointThreshold())
				{
					RegisterQuest(UFlareQuestGeneratedStationDefense2::Create(this, Station->GetCurrentSector(), Station->GetCompany(), HostileCompany));
				}
			}
		}
	}
}

void UFlareQuestGenerator::GenerateMeteoriteQuest(UFlareSimulatedSpacecraft* TargetStation, float Power, int32 Count, int64 TimeBeforeImpact)
{
	if(!IsGenerationEnabled())
	{
		return;
	}

	if(TargetStation->GetCompany()->GetPlayerHostility() == EFlareHostility::Hostile)
	{
		return;
	}

	RegisterQuest(UFlareQuestGeneratedMeteoriteInterception::Create(this, TargetStation, Power, Count, TimeBeforeImpact));

}


void UFlareQuestGenerator::RegisterQuest(UFlareQuestGenerated* Quest)
{
	if(!IsGenerationEnabled())
	{
		return;
	}

	if (Quest)
	{
		QuestManager->AddQuest(Quest);
		GeneratedQuests.Add(Quest);
		QuestManager->LoadCallbacks(Quest);
		Quest->UpdateState();

		QuestManager->OnEvent(FFlareBundle().PutTag("get-contract"));
	}
}

bool UFlareQuestGenerator::FindUniqueTag(FName Tag)
{
	for(UFlareQuestGenerated* Quest : GeneratedQuests)
	{
		EFlareQuestStatus::Type Status = Quest->GetStatus();
		if (Status == EFlareQuestStatus::AVAILABLE || Status == EFlareQuestStatus::ONGOING)
		{
			if (Quest->GetInitData()->HasTag(Tag))
			{
				return true;
			}
		}
	}

	return false;
}

FName UFlareQuestGenerator::GenerateVipTag(UFlareSimulatedSpacecraft* SourceSpacecraft)
{
	return FName(*(FString("vip-")+SourceSpacecraft->GetImmatriculation().ToString()));

}

FName UFlareQuestGenerator::GenerateTradeTag(UFlareSimulatedSpacecraft* SourceSpacecraft, FFlareResourceDescription* Resource)
{
	return FName(*(FString("trade-")+SourceSpacecraft->GetImmatriculation().ToString()+"-"+Resource->Identifier.ToString()));
}

FName UFlareQuestGenerator::GenerateDefenseTag(UFlareSimulatedSector* Sector, UFlareCompany* HostileCompany)
{
	return FName(*(FString("defense-")+Sector->GetIdentifier().ToString()+"-"+HostileCompany->GetIdentifier().ToString()));
}

FName UFlareQuestGenerator::GenerateAttackTag(UFlareSimulatedSector* Sector, UFlareCompany* OwnerCompany)
{
	return FName(*(FString("attach-")+Sector->GetIdentifier().ToString()+"-"+OwnerCompany->GetIdentifier().ToString()));
}

FName UFlareQuestGenerator::GenerateHarassTag(UFlareCompany* OwnerCompany, UFlareCompany* HostileCompany)
{
	return FName(*(FString("harass-")+"-"+OwnerCompany->GetIdentifier().ToString()+"-"+HostileCompany->GetIdentifier().ToString()));
}

FName UFlareQuestGenerator::GenerateMeteoriteTag(UFlareSimulatedSector* Sector)
{
	return FName(*(FString("meteorite-")+"-"+Sector->GetIdentifier().ToString()));
}

/*----------------------------------------------------
	Generated quest
----------------------------------------------------*/

UFlareQuestGenerated::UFlareQuestGenerated(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UFlareQuestGenerated::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	LoadInternal(Parent->GetQuestManager());
	QuestGenerator = Parent;
	InitData = Data;

	return true;
}

void UFlareQuestGenerated::CreateGenericReward(FFlareBundle& Data, int64 QuestValue, UFlareCompany* Client)
{
	// TODO more reward

	bool RewardGiven = false;

	int32 ClientResearch = Client->GetResearchValue();
	int32 PlayerResearch = Client->GetGame()->GetPC()->GetCompany()->GetResearchValue();
	FFlarePlayerSave* PlayerData = Client->GetGame()->GetPC()->GetPlayerData();
	int32 GameDifficulty = -1;
	if (PlayerData) 
	{
		GameDifficulty = PlayerData->DifficultyId;
	}

	float DifficultyModifier = 1.0f;
	float PenaltyModifier = 1.0f;
	
	switch (GameDifficulty)
	{
		case -1: // Easy
			DifficultyModifier = 1.10f;
			PenaltyModifier = 1.00f;
			break;
		case 0: // Normal
			DifficultyModifier = 1.0f;
			PenaltyModifier = 1.0f;
			break;
		case 1: // Hard
			DifficultyModifier = 0.75f;
			PenaltyModifier = 0.70f;
			break;
		case 2: // Very Hard
			DifficultyModifier = 0.50f;
			PenaltyModifier = 0.45f;
			break;
		case 3: // Expert
			DifficultyModifier = 0.40f;
			PenaltyModifier = 0.30f;
			break;
		case 4: // Unfair
			DifficultyModifier = 0.30f;
			PenaltyModifier = 0.10f;
			break;
	}

	QuestValue = QuestValue * DifficultyModifier;

	if(ClientResearch > PlayerResearch)
	{
		// There is a chance to hava research reward

		float ResearchRewardProbability = 0.5f * (1.f - float(PlayerResearch) / float(ClientResearch));

		FLOGV("ResearchRewardProbability for %s : %f", *Client->GetCompanyName().ToString(), ResearchRewardProbability);

		if (FMath::FRand() < ResearchRewardProbability)
		{
			int32 MaxPossibleResearchReward = ClientResearch - PlayerResearch;
			int32 GainedResearchReward = QuestValue / 30000;

			int32 ResearchReward = FMath::Min(GainedResearchReward, MaxPossibleResearchReward);

			Data.PutInt32("reward-research", ResearchReward);
			RewardGiven = true;
		}
	}

	if(!RewardGiven)
	{
		Data.PutInt32("reward-money", QuestValue);
	}

	// Reputation reward and penalty
//	Data.PutInt32("reward-reputation", GameDifficulty);
	Data.PutInt32("reward-reputation", FMath::Sqrt(QuestValue / 10000));
	Data.PutInt32("penalty-reputation", -FMath::Sqrt(QuestValue / (1000 * PenaltyModifier)));
}

void UFlareQuestGenerated::AddGlobalFailCondition(UFlareQuestCondition* Condition)
{
	for(UFlareQuestStep* Step : GetSteps())
	{
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(Condition);
	}
}

void UFlareQuestGenerated::SetupQuestGiver(UFlareCompany* Company, bool AddWarCondition)
{
	Client = Company;
	if (AddWarCondition)
	{
		AddGlobalFailCondition(UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Company));
		Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Company));
	}
}

void UFlareQuestGenerated::SetupGenericReward(const FFlareBundle& Data)
{

	if (Data.HasInt32("reward-money"))
	{
		int64 Amount = Data.GetInt32("reward-money");
		GetSuccessActions().Add(UFlareQuestActionGiveMoney::Create(this, Client, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Amount));
	}

	if (Data.HasInt32("reward-research"))
	{
		int64 Amount = Data.GetInt32("reward-research");
		GetSuccessActions().Add(UFlareQuestActionGiveResearch::Create(this, Client, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Amount));
	}

	if (Data.HasInt32("reward-reputation"))
	{
		int64 Amount = Data.GetInt32("reward-reputation");
		GetSuccessActions().Add(UFlareQuestActionReputationChange::Create(this, Client, Amount));
	}

	if (Data.HasInt32("penalty-reputation"))
	{
		int64 Amount = Data.GetInt32("penalty-reputation");
		GetFailActions().Add(UFlareQuestActionReputationChange::Create(this, Client, Amount));
	}
}

/*----------------------------------------------------
	Generated VIP transport quest
----------------------------------------------------*/
UFlareQuestGeneratedVipTransport::UFlareQuestGeneratedVipTransport(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedVipTransport::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		if (CandidateStation->GetCompany() != Company)
		{
			continue;
		}

		if (CandidateStation->IsBeingCaptured())
		{
			continue;
		}

		// Check friendlyness
		if (CandidateStation->IsHostile(PlayerCompany) || CandidateStation->GetCompany()->GetWarState(PlayerCompany) != EFlareHostility::Neutral)
		{
			// Me or at war company or is target of a mission
			continue;
		}

		// Check if there is another distant station
		bool HasDistantSector = false;
		for(UFlareSimulatedSpacecraft* CompanyStation : CandidateStation->GetCompany()->GetCompanyStations())
		{
			if (CompanyStation->IsBeingCaptured())
			{
				continue;
			}

			if(CompanyStation->GetCurrentSector() != Sector)
			{
				HasDistantSector = true;
				break;
			}
		}

		if (!HasDistantSector)
		{
			continue;
		}

		// Check unicity
		if (Parent->FindUniqueTag(Parent->GenerateVipTag(CandidateStation)))
		{
			continue;
		}

		// It's a good
		CandidateStations.Add(CandidateStation);
	}

	if(CandidateStations.Num() == 0)
	{
		// No candidate, don't create any quest
		return NULL;
	}

	// Pick a candidate
	int32 CandidateIndex = FMath::RandRange(0, CandidateStations.Num()-1);
	UFlareSimulatedSpacecraft* Station1 = CandidateStations[CandidateIndex];

	// Find second station candidate
	TArray<UFlareSimulatedSpacecraft*> CandidateStations2;
	for(UFlareSimulatedSpacecraft* CompanyStation : Station1->GetCompany()->GetCompanyStations())
	{
		if (CompanyStation->IsBeingCaptured())
		{
			continue;
		}

		if(CompanyStation->GetCurrentSector() != Sector)
		{
			CandidateStations2.Add(CompanyStation);
		}
	}

	int32 Candidate2Index = FMath::RandRange(0, CandidateStations2.Num()-1);
	UFlareSimulatedSpacecraft* Station2 = CandidateStations2[Candidate2Index];

	// Setup reward
	int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Parent->GetGame()->GetGameWorld(), Station1->GetCurrentSector(), Station2->GetCurrentSector(), NULL);
	int64 QuestValue = 200000 + 50000 * TravelDuration;

	// Create the quest
	UFlareQuestGeneratedVipTransport* Quest = NewObject<UFlareQuestGeneratedVipTransport>(Parent, UFlareQuestGeneratedVipTransport::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedVipTransport::GetClass(), Data);


	TArray<FString> UsedNames;

	for(UFlareQuest* VisibleQuest : Parent->GetQuestManager()->GetAvailableQuests())
	{
		UFlareQuestGeneratedVipTransport* VipQuest = Cast<UFlareQuestGeneratedVipTransport>(VisibleQuest);
		if(VipQuest)
		{
			UsedNames.Add(VipQuest->GetInitData()->GetString("vip-name"));
		}
	}

	for(UFlareQuest* VisibleQuest : Parent->GetQuestManager()->GetOngoingQuests())
	{
		UFlareQuestGeneratedVipTransport* VipQuest = Cast<UFlareQuestGeneratedVipTransport>(VisibleQuest);
		if(VipQuest)
		{
			UsedNames.Add(VipQuest->GetInitData()->GetString("vip-name"));
		}
	}

	Data.PutString("vip-name", UFlareQuestGenerator::GeneratePersonName(UsedNames).ToString());
	Data.PutName("station-1", Station1->GetImmatriculation());
	Data.PutName("station-2", Station2->GetImmatriculation());
	Data.PutName("sector-1", Station1->GetCurrentSector()->GetIdentifier());
	Data.PutName("sector-2", Station2->GetCurrentSector()->GetIdentifier());
	Data.PutName("client", Station1->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateVipTag(Station1));
	CreateGenericReward(Data, QuestValue, Station1->GetCompany());

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedVipTransport::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector1 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector-1"));
	UFlareSimulatedSector* Sector2 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector-2"));
	UFlareSimulatedSpacecraft* Station1 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station-1"));
	UFlareSimulatedSpacecraft* Station2 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station-2"));

	FText VIPName = FText::FromString(Data.GetString("vip-name"));

	QuestClass = UFlareQuestGeneratedVipTransport::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = FText::Format(LOCTEXT("GeneratedVipTransportName","VIP transport : {0}"), VIPName);
	QuestDescription = FText::Format(LOCTEXT("GeneratedVipTransportDescriptionFormat","Transport {0} from {1} to {2}."), VIPName, Sector1->GetSectorName(), Sector2->GetSectorName());
	QuestCategory = EFlareQuestCategory::SECONDARY;


	FName PickUpShipId = "pick-up-ship-id";

	{
		FText Description = FText::Format(LOCTEXT("PickUpDescription", "Pick {0} up at {1} in {2}"), VIPName, UFlareGameTools::DisplaySpacecraftName(Station1), FText::FromString(Sector1->GetSectorName().ToString()));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pick-up", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station1);
		Condition->TargetShipSaveId = PickUpShipId;
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station1));

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector1));
		Steps.Add(Step);
	}

	{
		FText Description = FText::Format(LOCTEXT("Drop-offDescription", "Drop {0} off at {1} in {2}"), VIPName, UFlareGameTools::DisplaySpacecraftName(Station2), FText::FromString(Sector2->GetSectorName().ToString()));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "drop-off", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Station2);
		Condition->TargetShipMatchId = PickUpShipId;
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, NULL, PickUpShipId));
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector2));

		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station1));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));

	SetupQuestGiver(Station1->GetCompany(), true);
	SetupGenericReward(Data);

	return true;
}

/*----------------------------------------------------
	Generated resource sale quest
----------------------------------------------------*/

UFlareQuestGeneratedResourceSale::UFlareQuestGeneratedResourceSale(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedResourceSale::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		if (CandidateStation->GetCompany() != Company)
		{
			continue;
		}

		if (CandidateStation->IsBeingCaptured())
		{
			continue;
		}

		if (CandidateStation->IsUnderConstruction())
		{
			continue;
		}

		// Check friendlyness
		if (CandidateStation->IsHostile(PlayerCompany) || CandidateStation->GetCompany()->GetWarState(PlayerCompany) != EFlareHostility::Neutral)
		{
			// Me or at war company
			continue;
		}

		// Check if have too much stock there is another distant station

		for (FFlareCargo& Slot : CandidateStation->GetActiveCargoBay()->GetSlots())
		{
			if (Slot.Lock != EFlareResourceLock::Output)
			{
				// Not an output resource
				continue;
			}

			if (Slot.Quantity <= CandidateStation->GetActiveCargoBay()->GetSlotCapacity() * 0.5f)
			{
				// Not enought resources
				continue;
			}

			// Check unicity
			if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
			{
				continue;
			}

			// Check if player kwown at least one buyer of this resource
			if(!PlayerCompany->HasKnowResourceInput(Slot.Resource))
			{
				continue;
			}

			// It's a good candidate
			CandidateStations.Add(CandidateStation);
		}
	}

	if(CandidateStations.Num() == 0)
	{
		// No candidate, don't create any quest
		return NULL;
	}

	// Pick a candidate
	int32 CandidateIndex = FMath::RandRange(0, CandidateStations.Num()-1);
	UFlareSimulatedSpacecraft* Station = CandidateStations[CandidateIndex];

	// Find a resource

	int32 BestResourceExcessQuantity = 0;
	int32 BestResourceQuantity = 0;
	FFlareResourceDescription* BestResource = NULL;

	for (FFlareCargo& Slot : Station->GetActiveCargoBay()->GetSlots())
	{
		if (Slot.Lock != EFlareResourceLock::Output)
		{
			// Not an output resource
			continue;
		}


		int32 ExcessResourceQuantity = Slot.Quantity - Station->GetActiveCargoBay()->GetSlotCapacity() * 0.5f;

		if (ExcessResourceQuantity <= 0)
		{
			// Not enought resources
			continue;
		}

		// Check unicity
		if (Parent->FindUniqueTag(Parent->GenerateTradeTag(Station, Slot.Resource)))
		{
			continue;
		}

		// Check if player kwown at least one buyer of this resource
		if(!PlayerCompany->HasKnowResourceInput(Slot.Resource))
		{
			continue;
		}

		if (ExcessResourceQuantity > BestResourceExcessQuantity)
		{
			BestResourceExcessQuantity = ExcessResourceQuantity;
			BestResourceQuantity = FMath::Min(Slot.Quantity, int32(Station->GetActiveCargoBay()->GetSlotCapacity() * 0.5f));
			BestResource = Slot.Resource;
		}
	}



	int32 PlayerFleetTransportCapacity = Parent->GetGame()->GetPC()->GetPlayerFleet()->GetFleetCapacity();
	int32 PreferedCapacity = FMath::RandRange(PlayerFleetTransportCapacity / 5, PlayerFleetTransportCapacity / 2);

	int32 QuestResourceQuantity = FMath::Min(BestResourceQuantity, PreferedCapacity);

	// Setup reward
	int64 QuestValue = 200000 + 40000 * FMath::Sqrt(QuestResourceQuantity);

	// Create the quest
	UFlareQuestGeneratedResourceSale* Quest = NewObject<UFlareQuestGeneratedResourceSale>(Parent, UFlareQuestGeneratedResourceSale::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedResourceSale::GetClass(), Data);

	Data.PutName("station", Station->GetImmatriculation());
	Data.PutName("sector", Station->GetCurrentSector()->GetIdentifier());
	Data.PutName("resource", BestResource->Identifier);
	Data.PutInt32("quantity", QuestResourceQuantity);
	Data.PutName("client", Station->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateTradeTag(Station, BestResource));

	CreateGenericReward(Data, QuestValue, Station->GetCompany());

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedResourceSale::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));
	UFlareSimulatedSpacecraft* Station = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station"));
	FFlareResourceDescription* Resource = Parent->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
	int32 Quantity = InitData.GetInt32("quantity");

	QuestClass = UFlareQuestGeneratedResourceSale::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = FText::Format(LOCTEXT("GeneratedResourceSaleName","{0} destocking in {1}"), Resource->Name, Sector->GetSectorName());
	QuestDescription = FText::Format(LOCTEXT("GeneratedResourceSaleDescriptionFormat","Buy {0} {1} from {2} at {3}."),
									 FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(Station), Sector->GetSectorName());
	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuyResource"
		FText Description = FText::Format(LOCTEXT("BuyResourceDescription", "Buy {0} {1} from {2} at {3}"),
										  FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(Station), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "buy", Description);

		UFlareQuestConditionBuyAtStation* Condition = UFlareQuestConditionBuyAtStation::Create(this, "cond1", Station, Resource, Quantity);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));
		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station));

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station));

	SetupQuestGiver(Station->GetCompany(), true);
	SetupGenericReward(Data);

	return true;
}

int32 UFlareQuestGeneratedResourceSale::GetReservedQuantity(UFlareSimulatedSpacecraft* TargetStation, FFlareResourceDescription* TargetResource)
{
	UFlareSimulatedSpacecraft* Station = QuestManager->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station"));
	// TODO Cache

	if (Station == TargetStation)
	{
		FFlareResourceDescription* Resource = QuestManager->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
		if (Resource == TargetResource)
		{
			int32 Quantity = InitData.GetInt32("quantity");
			return Quantity;
		}
	}

	return 0;
}

/*----------------------------------------------------
	Generated resource purchase quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedResourcePurchase"
UFlareQuestGeneratedResourcePurchase::UFlareQuestGeneratedResourcePurchase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedResourcePurchase::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, TArray<FFlareResourceDescription*>& AvailableResources)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Find a random friendly station in sector
	TArray<UFlareSimulatedSpacecraft*> CandidateStations;
	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		if (CandidateStation->GetCompany() != Company)
		{
			continue;
		}

		if (CandidateStation->IsBeingCaptured())
		{
			continue;
		}

		if (CandidateStation->IsUnderConstruction())
		{
			continue;
		}

		// Check friendlyness
		if (CandidateStation->IsHostile(PlayerCompany) || CandidateStation->GetCompany()->GetWarState(PlayerCompany) != EFlareHostility::Neutral)
		{
			// Me or at war company
			continue;
		}

		// Check if have too low stock there is another distant station

		for (FFlareCargo& Slot : CandidateStation->GetActiveCargoBay()->GetSlots())
		{
			if (Slot.Lock != EFlareResourceLock::Input)
			{
				// Not an input resource
				continue;
			}

			if (Slot.Quantity >= CandidateStation->GetActiveCargoBay()->GetSlotCapacity() * 0.5)
			{
				// Enought resources
				continue;
			}

			// Check unicity
			if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
			{
				continue;
			}


			// Check if player kwown at least one seller of this resource
			if(!AvailableResources.Contains(Slot.Resource))
			{
				continue;
			}

			// It's a good candidate
			CandidateStations.Add(CandidateStation);
			break;
		}
	}

	if(CandidateStations.Num() == 0)
	{
		// No candidate, don't create any quest
		return NULL;
	}

	// Pick a candidate
	int32 CandidateIndex = FMath::RandRange(0, CandidateStations.Num()-1);
	UFlareSimulatedSpacecraft* Station = CandidateStations[CandidateIndex];

	// Find a resource

	int32 BestResourceMissingQuantity = 0;
	int32 BestResourceQuantity = 0;
	FFlareResourceDescription* BestResource = NULL;

	for (FFlareCargo& Slot : Station->GetActiveCargoBay()->GetSlots())
	{
		if (Slot.Lock != EFlareResourceLock::Input)
		{
			// Not an input resource
			continue;
		}


		int32 MissingResourceQuantity = Station->GetActiveCargoBay()->GetSlotCapacity() * 0.5 - Slot.Quantity;

		int32 ResourcePrice = Station->GetCurrentSector()->GetTransfertResourcePrice(nullptr, Station, Slot.Resource);
		int32 MaxAffordableQuantity = FMath::Max(0, int32(Company->GetMoney() / ResourcePrice));


		int32 MissingResourceAffordableQuantity = FMath::Min(MissingResourceQuantity, MaxAffordableQuantity);

		if (MissingResourceAffordableQuantity <= 0)
		{
			// Enought resources
			continue;
		}

		// Check unicity
		if (Parent->FindUniqueTag(Parent->GenerateTradeTag(Station, Slot.Resource)))
		{
			continue;
		}

		// Check if player kwown at least one seller of this resource
		if(!AvailableResources.Contains(Slot.Resource))
		{
			continue;
		}

		if (MissingResourceAffordableQuantity > BestResourceQuantity)
		{
			BestResourceMissingQuantity = MissingResourceAffordableQuantity;
			BestResourceQuantity = FMath::Min(Station->GetActiveCargoBay()->GetSlotCapacity() - Slot.Quantity, int32(Station->GetActiveCargoBay()->GetSlotCapacity() * 0.5));
			BestResource = Slot.Resource;
		}


	}

	if(!BestResource)
	{
		FLOG("WARNING: no best resource, inconsistent station candidate");
		return NULL;
	}


	int32 PlayerFleetTransportCapacity = Parent->GetGame()->GetPC()->GetPlayerFleet()->GetFleetCapacity();
	int32 PreferedCapacity = FMath::RandRange(PlayerFleetTransportCapacity / 5, PlayerFleetTransportCapacity / 2);


	int32 QuestResourceQuantity = FMath::Min(BestResourceQuantity, PreferedCapacity);


	// Setup reward
	int64 QuestValue = 200000 + 50000 * FMath::Sqrt(QuestResourceQuantity);

	// Create the quest
	UFlareQuestGeneratedResourcePurchase* Quest = NewObject<UFlareQuestGeneratedResourcePurchase>(Parent, UFlareQuestGeneratedResourcePurchase::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedResourcePurchase::GetClass(), Data);

	Data.PutName("station", Station->GetImmatriculation());
	Data.PutName("sector", Station->GetCurrentSector()->GetIdentifier());
	Data.PutName("resource", BestResource->Identifier);
	Data.PutInt32("quantity", QuestResourceQuantity);
	Data.PutName("client", Station->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateTradeTag(Station, BestResource));
	CreateGenericReward(Data, QuestValue, Station->GetCompany());

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedResourcePurchase::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));
	UFlareSimulatedSpacecraft* Station = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station"));
	FFlareResourceDescription* Resource = Parent->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
	int32 Quantity = InitData.GetInt32("quantity");

	QuestClass = UFlareQuestGeneratedResourcePurchase::GetClass();
	Identifier = InitData.GetName("identifier");
	QuestName = FText::Format(LOCTEXT("GeneratedResourcePurchaseName","{0} delivery in {1}"), Resource->Name, Sector->GetSectorName());
	QuestDescription = FText::Format(LOCTEXT("GeneratedResourcePurchaseDescriptionFormat","Sell {0} {1} to {2} at {3}."),
									 FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(Station), Sector->GetSectorName());
	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuyResource"
		FText Description = FText::Format(LOCTEXT("GeneratedResourcePurchaseBuyResourceDescription", "Sell {0} {1} to {2} at {3}"),
										  FText::AsNumber(Quantity), Resource->Name, UFlareGameTools::DisplaySpacecraftName(Station), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "sell", Description);

		UFlareQuestConditionSellAtStation* Condition = UFlareQuestConditionSellAtStation::Create(this, QUEST_TAG"cond1", Station, Resource, Quantity);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));
		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station));

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station));

	SetupQuestGiver(Station->GetCompany(), true);
	SetupGenericReward(Data);

	return true;
}

int32 UFlareQuestGeneratedResourcePurchase::GetReservedCapacity(UFlareSimulatedSpacecraft* TargetStation, FFlareResourceDescription* TargetResource)
{
	UFlareSimulatedSpacecraft* Station = QuestManager->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station"));
	// TODO Cache

	if (Station == TargetStation)
	{
		FFlareResourceDescription* Resource = QuestManager->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
		if (Resource == TargetResource)
		{
			int32 Quantity = InitData.GetInt32("quantity");
			return Quantity;
		}
	}

	return 0;
}

/*----------------------------------------------------
	Generated resource trade quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedResourceTrade"
UFlareQuestGeneratedResourceTrade::UFlareQuestGeneratedResourceTrade(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedResourceTrade::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Find a best friendly station in sector
	TMap<FFlareResourceDescription*, int32> BestQuantityToBuyPerResource;
	TMap<FFlareResourceDescription*, UFlareSimulatedSpacecraft*> BestStationToBuyPerResource;
	TMap<FFlareResourceDescription*, int32> BestQuantityToSellPerResource;
	TMap<FFlareResourceDescription*, UFlareSimulatedSpacecraft*> BestStationToSellPerResource;

	for (UFlareSimulatedSpacecraft* CandidateStation : Sector->GetSectorStations())
	{
		// Check friendlyness
		if (CandidateStation->IsHostile(PlayerCompany) || CandidateStation->GetCompany()->GetWarState(Company) != EFlareHostility::Owned)
		{
			continue;
		}

		if (CandidateStation->IsBeingCaptured())
		{
			continue;
		}

		if (CandidateStation->IsUnderConstruction())
		{
			continue;
		}

		// Check if have too much stock there is another distant station

		for (FFlareCargo& Slot : CandidateStation->GetActiveCargoBay()->GetSlots())
		{
			if (Slot.Lock != EFlareResourceLock::Output)
			{
				// Not an output resource
				continue;
			}

			int32 AvailableResourceQuantity = Slot.Quantity - CandidateStation->GetActiveCargoBay()->GetSlotCapacity() * 0.5;

			if (AvailableResourceQuantity <= 0)
			{
				// Not enought resources
				continue;
			}

			// Check unicity
			if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
			{
				continue;
			}

			// It's a good candidate
			if(!BestQuantityToBuyPerResource.Contains(Slot.Resource) || BestQuantityToBuyPerResource[Slot.Resource] > AvailableResourceQuantity)
			{
				// Best candidate
				BestQuantityToBuyPerResource.Add(Slot.Resource, AvailableResourceQuantity);
				BestStationToBuyPerResource.Add(Slot.Resource, CandidateStation);
			}
		}
	}

	for(UFlareCompany* Company2: Parent->GetGame()->GetGameWorld()->GetCompanies())
	{
		if (Company2 == PlayerCompany || Company2->GetWarState(Company) == EFlareHostility::Hostile)
		{
			continue;
		}

		for (UFlareSimulatedSpacecraft* CandidateStation : Company2->GetCompanyStations())
		{

			// Check if have too much stock there is another distant station
			if (CandidateStation->IsUnderConstruction())
			{
				continue;
			}

			for (FFlareCargo& Slot : CandidateStation->GetActiveCargoBay()->GetSlots())
			{
				if (Slot.Lock != EFlareResourceLock::Input)
				{
					// Not an input resource
					continue;
				}

				int32 MissingResourceQuantity = CandidateStation->GetActiveCargoBay()->GetSlotCapacity() * 0.5 - Slot.Quantity;

				int32 ResourcePrice = CandidateStation->GetCurrentSector()->GetTransfertResourcePrice(nullptr, CandidateStation, Slot.Resource);
				int32 MaxAffordableQuantity = FMath::Max(0, int32(Company->GetMoney() / ResourcePrice));


				int32 MissingResourceAffordableQuantity = FMath::Min(MissingResourceQuantity, MaxAffordableQuantity);


				if (MissingResourceAffordableQuantity <= 0)
				{
					// Enought resources
					continue;
				}

				// Check unicity
				if (Parent->FindUniqueTag(Parent->GenerateTradeTag(CandidateStation, Slot.Resource)))
				{
					continue;
				}

				// It's a good candidate
				if(!BestQuantityToSellPerResource.Contains(Slot.Resource) || BestQuantityToSellPerResource[Slot.Resource] > MissingResourceAffordableQuantity)
				{
					// Best candidate
					BestQuantityToSellPerResource.Add(Slot.Resource, MissingResourceAffordableQuantity);
					BestStationToSellPerResource.Add(Slot.Resource, CandidateStation);
				}
			}
		}
	}


	// All maps are full, now, pick the best candidate
	int32 BestNeededResourceQuantity = 0;
	FFlareResourceDescription* BestResource = NULL;
	UFlareSimulatedSpacecraft* Station1 = NULL;
	UFlareSimulatedSpacecraft* Station2 = NULL;

	for (UFlareResourceCatalogEntry* ResourcePointer : Parent->GetGame()->GetResourceCatalog()->GetResourceList())
	{
		FFlareResourceDescription* Resource = &ResourcePointer->Data;

		if (BestQuantityToBuyPerResource.Contains(Resource) && BestQuantityToSellPerResource.Contains(Resource))
		{
			int32 MinQuantity = FMath::Min(BestQuantityToBuyPerResource[Resource], BestQuantityToSellPerResource[Resource]);
			if(MinQuantity > BestNeededResourceQuantity)
			{
				BestNeededResourceQuantity = MinQuantity;
				BestResource = Resource;
			}
		}
	}
	if(BestResource == NULL)
	{
		return NULL;
	}
	Station1 = BestStationToBuyPerResource[BestResource];
	Station2 = BestStationToSellPerResource[BestResource];
	int32 BestBuyResourceQuantity = FMath::Min(Station1->GetActiveCargoBay()->GetResourceQuantity(BestResource, PlayerCompany), int32(Station1->GetActiveCargoBay()->GetSlotCapacity() * 0.5));
	int32 BestSellResourceQuantity = FMath::Min(Station2->GetActiveCargoBay()->GetSlotCapacity() - Station2->GetActiveCargoBay()->GetResourceQuantity(BestResource, PlayerCompany), int32(Station2->GetActiveCargoBay()->GetSlotCapacity() * 0.5));

	int32 BestResourceQuantity = FMath::Min(BestBuyResourceQuantity, BestSellResourceQuantity);
	int32 PlayerFleetTransportCapacity = Parent->GetGame()->GetPC()->GetPlayerFleet()->GetFleetCapacity();
	int32 PreferedCapacity = FMath::RandRange(PlayerFleetTransportCapacity / 5, PlayerFleetTransportCapacity / 2);

	int32 QuestResourceQuantity = FMath::Min(BestResourceQuantity, PreferedCapacity);



	// Setup reward
	int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Parent->GetGame()->GetGameWorld(), Station1->GetCurrentSector(), Station2->GetCurrentSector(), NULL);
	int64 QuestValue = 200000 + FMath::Sqrt(TravelDuration) * 100000 + 20000 * FMath::Sqrt(QuestResourceQuantity);

	// Create the quest
	UFlareQuestGeneratedResourceTrade* Quest = NewObject<UFlareQuestGeneratedResourceTrade>(Parent, UFlareQuestGeneratedResourceTrade::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedResourceTrade::GetClass(), Data);

	Data.PutName("station1", Station1->GetImmatriculation());
	Data.PutName("sector1", Station1->GetCurrentSector()->GetIdentifier());
	Data.PutName("station2", Station2->GetImmatriculation());
	Data.PutName("sector2", Station2->GetCurrentSector()->GetIdentifier());
	Data.PutName("resource", BestResource->Identifier);
	Data.PutInt32("quantity", QuestResourceQuantity);
	Data.PutName("client", Station1->GetCompany()->GetIdentifier());
	Data.PutTag(Parent->GenerateTradeTag(Station1, BestResource));
	Data.PutTag(Parent->GenerateTradeTag(Station2, BestResource));
	CreateGenericReward(Data, QuestValue, Station1->GetCompany());

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedResourceTrade::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareSimulatedSector* Sector1 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector1"));
	UFlareSimulatedSector* Sector2 = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector2"));
	UFlareSimulatedSpacecraft* Station1 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station1"));
	UFlareSimulatedSpacecraft* Station2 = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station2"));
	FFlareResourceDescription* Resource = Parent->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
	int32 Quantity = InitData.GetInt32("quantity");

	QuestClass = UFlareQuestGeneratedResourceTrade::GetClass();
	Identifier = InitData.GetName("identifier");
	if(Sector1 == Sector2)
	{
		QuestName = FText::Format(LOCTEXT("GeneratedResourceTradeNameLocal","{0} haulage in {1}"), Resource->Name, Sector1->GetSectorName());
		QuestDescription = FText::Format(LOCTEXT("GeneratedResourceTradeDescriptionLocalFormat","Trade {0} {1} in {2}."),
									 FText::AsNumber(Quantity), Resource->Name, Sector1->GetSectorName());
	}
	else
	{
		QuestName = FText::Format(LOCTEXT("NameDistant","{0} haulage to {1}"), Resource->Name,
								  Sector2->GetSectorName());
		QuestDescription = FText::Format(LOCTEXT("DescriptionDistantFormat","Trade {0} {1} from {2} to {3}."),
									 FText::AsNumber(Quantity), Resource->Name,
										 Sector1->GetSectorName(), Sector2->GetSectorName());
	}
	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TradeResource"
		FText Description;
		if(Sector1 == Sector2)
		{
			Description = FText::Format(LOCTEXT("TradeResourceDescriptionLocal", "Trade {0} {1} in {2}"),
										  FText::AsNumber(Quantity), Resource->Name, Sector1->GetSectorName());
		}
		else
		{
			Description = FText::Format(LOCTEXT("TradeResourceDescriptionDistant", "Trade {0} {1} from {2} to {3}"),
										  FText::AsNumber(Quantity), Resource->Name, Sector1->GetSectorName(), Sector2->GetSectorName());
		}
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "trade", Description);

		UFlareQuestConditionBuyAtStation* Condition1 = UFlareQuestConditionBuyAtStation::Create(this, QUEST_TAG"cond1", Station1, Resource, Quantity);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition1);
		UFlareQuestConditionSellAtStation* Condition2 = UFlareQuestConditionSellAtStation::Create(this, QUEST_TAG"cond2", Station2, Resource, Quantity);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition2);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector1));
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector2));
		Steps.Add(Step);
	}

	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station1));
	AddGlobalFailCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));
	if (Station1->GetCompany() != Station2->GetCompany())
	{
		AddGlobalFailCondition(UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Station2->GetCompany()));
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station1));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionSpacecraftNoMoreExist::Create(this, Station2));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), Station2->GetCompany()));

	SetupQuestGiver(Station1->GetCompany(), true);
	SetupGenericReward(Data);

	return true;
}

int32 UFlareQuestGeneratedResourceTrade::GetReservedCapacity(UFlareSimulatedSpacecraft* TargetStation, FFlareResourceDescription* TargetResource)
{
	UFlareSimulatedSpacecraft* Station2 = QuestManager->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station2"));
	// TODO Cache

	if (Station2 == TargetStation)
	{
		FFlareResourceDescription* Resource = QuestManager->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
		if (Resource == TargetResource)
		{
			int32 Quantity = InitData.GetInt32("quantity");
			return Quantity;
		}
	}

	return 0;
}

int32 UFlareQuestGeneratedResourceTrade::GetReservedQuantity(UFlareSimulatedSpacecraft* TargetStation, FFlareResourceDescription* TargetResource)
{
	UFlareSimulatedSpacecraft* Station1 = QuestManager->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("station1"));
	// TODO Cache

	if (Station1 == TargetStation)
	{
		FFlareResourceDescription* Resource = QuestManager->GetGame()->GetResourceCatalog()->Get(InitData.GetName("resource"));
		if (Resource == TargetResource)
		{
			int32 Quantity = InitData.GetInt32("quantity");
			return Quantity;
		}
	}

	return 0;
}
/*----------------------------------------------------
	Generated station defense quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedStationDefense"
UFlareQuestGeneratedStationDefense::UFlareQuestGeneratedStationDefense(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool UFlareQuestGeneratedStationDefense::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");

	QuestClass = UFlareQuestGeneratedStationDefense::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedStationDefenseNameLocal","Defend stations of {0} against {1}"), Sector->GetSectorName(), HostileCompany->GetCompanyName());
	QuestDescription = FText::Format(LOCTEXT("GeneratedStationDefenseDescriptionLocalFormat","Defend stations of {0} in {1} against {2} with at least {3} combat value."),
								 FriendlyCompany->GetCompanyName(), Sector->GetSectorName(), HostileCompany->GetCompanyName(), FText::AsNumber(ArmyCombatPoints ));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"War"
		FText Description;

		Description = FText::Format(LOCTEXT("WarDescriptionWar", "Declare war on {0}"),
										  HostileCompany->GetCompanyName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "war", Description);

		// Impose to declare war
		UFlareQuestConditionAtWar* Condition = UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), HostileCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedStationDefenseDescriptionBringForce", "Attack {0} in {1} with at least {2} combat value"),
										  HostileCompany->GetCompanyName(), Sector->GetSectorName(), FText::AsNumber(ArmyCombatPoints ));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);
		{
			// First case : the player bring forces
			UFlareQuestConditionMinArmyCombatPointsInSector* Condition = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		}

		{
			// Failed to defend
			UFlareQuestConditionStationLostInSector* FailCondition = UFlareQuestConditionStationLostInSector::Create(this, Sector, FriendlyCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);

			UFlareQuestConditionAtPeace* FailCondition2 = UFlareQuestConditionAtPeace::Create(this, PlayerCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition2);
		}

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Defend"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedStationDefenseDescriptionDefendStation", "Defend {0} in {1} against {2}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName(), HostileCompany->GetCompanyName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "defend", Description);

		// OR root
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		{
			UFlareQuestConditionNoCapturingStationInSector* Condition = UFlareQuestConditionNoCapturingStationInSector::Create(this, Sector, FriendlyCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		// OR
		{
			UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, PlayerCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition1);
		UFlareQuestConditionAtPeace* FailCondition2 = UFlareQuestConditionAtPeace::Create(this, PlayerCompany, HostileCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition2);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}


	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionStationLostInSector::Create(this,  Sector, FriendlyCompany));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionNoCapturingStationInSector::Create(this,  Sector, FriendlyCompany, HostileCompany));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}

/*----------------------------------------------------
	Generated joint attack quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedJointAttack"
UFlareQuestGeneratedJoinAttack::UFlareQuestGeneratedJoinAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UFlareQuestGeneratedJoinAttack::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	TArray<UFlareCompany*> HostileCompanies;
	TArray<FName> HostileCompanyNames = InitData.GetNameArray("hostile-companies");
	for(FName HostileCompanyName : HostileCompanyNames)
	{
		HostileCompanies.Add(Parent->GetGame()->GetGameWorld()->FindCompany(HostileCompanyName));
	}
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");
	int32 AttackDate = InitData.GetInt32("attack-date");

	QuestClass = UFlareQuestGeneratedJoinAttack::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedJoinAttackNameLocal","Attack {0} with {1}"),
		Sector->GetSectorName(),
		FriendlyCompany->GetCompanyName());

	QuestDescription = FText::Format(LOCTEXT("GeneratedJoinAttackDescriptionLocalFormat","Follow {0} in {1} with at least {2} combat value, and attack on {3}."),
		FriendlyCompany->GetCompanyName(),
		Sector->GetSectorName(),
		FText::AsNumber(ArmyCombatPoints),
		UFlareGameTools::GetDisplayDate(AttackDate + 1));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"War"
		FText Description;

		FString WarList;

		for(UFlareCompany* HostileCompany: HostileCompanies)
		{
			WarList += " ";
			WarList += HostileCompany->GetCompanyName().ToString();
		}

		Description = FText::Format(LOCTEXT("WarDescriptionWar", "Declare war on {0}"),
										  FText::FromString(WarList));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "war", Description);

		// Impose to declare war

		for(UFlareCompany* HostileCompany: HostileCompanies)
		{
			UFlareQuestConditionAtWar* Condition = UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		{
			// Failed to attack at time
			UFlareQuestConditionAfterDate* FailCondition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedJoinAttackDescriptionBringForce", "Attack {0} with at least {1} combat value on {2}"),
			Sector->GetSectorName(),
			FText::AsNumber(ArmyCombatPoints ),
			UFlareGameTools::GetDisplayDate(AttackDate + 1));

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);
		{
			// First case : the player bring forces
			UFlareQuestConditionMinArmyCombatPointsInSector* Condition = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		}

		{
			// Failed to attack at time
			UFlareQuestConditionAfterDate* FailCondition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		for(UFlareCompany* HostileCompany: HostileCompanies)
		{
			UFlareQuestConditionAtPeace* FailCondition = UFlareQuestConditionAtPeace::Create(this, PlayerCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Fight"
		FText Description;

		Description = FText::Format(LOCTEXT("FightAlongsideFormat", "Fight alongside {0} in {1}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "fight", Description);

		// OR root
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		{
			UFlareQuestConditionAfterDate* Condition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		// OR
		{
			UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, PlayerCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition1);

		for(UFlareCompany* HostileCompany: HostileCompanies)
		{
			UFlareQuestConditionAtPeace* FailCondition = UFlareQuestConditionAtPeace::Create(this, PlayerCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Steps.Add(Step);
	}


	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAfterDate::Create(this, AttackDate));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}



/*----------------------------------------------------
	Generated sector defense quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedSectorDefense"
UFlareQuestGeneratedSectorDefense::UFlareQuestGeneratedSectorDefense(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UFlareQuestGeneratedSectorDefense::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");
	int32 AttackDate = InitData.GetInt32("attack-date");

	QuestClass = UFlareQuestGeneratedSectorDefense::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedSectorDefenseNameLocal","Defend {0} against {1}'s attack"),
		Sector->GetSectorName(),
		HostileCompany->GetCompanyName());

	QuestDescription = FText::Format(LOCTEXT("GeneratedSectorDefenseDescriptionLocalFormat","Defend {0} in {1} with at least {2} combat value on {3}."),
		FriendlyCompany->GetCompanyName(),
		Sector->GetSectorName(),
		FText::AsNumber(ArmyCombatPoints),
		UFlareGameTools::GetDisplayDate(AttackDate + 1));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"War"
		FText Description;

		Description = FText::Format(LOCTEXT("WarDescriptionWar", "Declare war on {0}"),
										  HostileCompany->GetCompanyName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "war", Description);

		// Impose to declare war

		{
			UFlareQuestConditionAtWar* Condition = UFlareQuestConditionAtWar::Create(this, GetQuestManager()->GetGame()->GetPC()->GetCompany(), HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		{
			// Failed to attack at time
			UFlareQuestConditionAfterDate* FailCondition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedSectorDefenseDescriptionBringForce", "Attack {0} with at least {1} combat value on {2}"),
			Sector->GetSectorName(),
			FText::AsNumber(ArmyCombatPoints),
			UFlareGameTools::GetDisplayDate(AttackDate + 1));

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);
		{
			// First case : the player bring forces
			UFlareQuestConditionMinArmyCombatPointsInSector* Condition = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		}

		{
			// Failed to attack at time
			UFlareQuestConditionAfterDate* FailCondition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		{
			UFlareQuestConditionAtPeace* FailCondition = UFlareQuestConditionAtPeace::Create(this, PlayerCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Defend"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedSectorDefenseDescriptionDefendStation", "Defend {0} in {1}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "defend", Description);

		// OR root
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		{
			UFlareQuestConditionAfterDate* Condition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		// OR
		{
			UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, PlayerCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition1);

		{
			UFlareQuestConditionAtPeace* FailCondition = UFlareQuestConditionAtPeace::Create(this, PlayerCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAfterDate::Create(this, AttackDate));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}


/*----------------------------------------------------
	Generated cargo hunt quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedCargoHunt"
UFlareQuestGeneratedCargoHunt::UFlareQuestGeneratedCargoHunt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool UFlareQuestGeneratedCargoHunt::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 CargoCount = InitData.GetInt32("cargo-count");
	bool RequestDestroyTarget = InitData.GetInt32("destroy-cargo") > 0;
	bool TargetLargeCargo = InitData.GetInt32("large-cargo") > 0;

	QuestClass = UFlareQuestGeneratedCargoHunt::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedCargoHuntNameLocal","Attack {0}'s cargos"), HostileCompany->GetCompanyName());

	QuestDescription = FText::Format(LOCTEXT("GeneratedCargoHuntDescriptionLocalFormat","{0} {1} {2} cargos to {3}."),
								 RequestDestroyTarget? LOCTEXT("RequestDestroyTarget","Destroy") : LOCTEXT("RequestUncotrollableTarget","Make uncontrollable"),
									 FText::AsNumber(CargoCount),
									 TargetLargeCargo? LOCTEXT("TargetLargeCargo","large") : LOCTEXT("TargetSmallCargo","small"),
									 HostileCompany->GetCompanyName());

	QuestCategory = EFlareQuestCategory::SECONDARY;


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"CargoHarass"
		FText Description;

		Description = LOCTEXT("GeneratedCargoHuntHarass", "Attack unarmed, defenseless cargos");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "harass", Description);


		UFlareQuestConditionDestroySpacecraft* Condition = UFlareQuestConditionDestroySpacecraft::Create(this,
																										 QUEST_TAG"cond1",
																										 HostileCompany,
																										 CargoCount,
																										 false,
																										 TargetLargeCargo ? EFlarePartSize::L : EFlarePartSize::S,
																										 RequestDestroyTarget);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}

/*----------------------------------------------------
	Generated military hunt quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedMilitaryHunt"
UFlareQuestGeneratedMilitaryHunt::UFlareQuestGeneratedMilitaryHunt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UFlareQuestGeneratedMilitaryHunt::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 RequestedArmyCombatPoints = InitData.GetInt32("army-combat-points");
	bool RequestDestroyTarget = InitData.GetInt32("destroy") > 0;

	QuestClass = UFlareQuestGeneratedMilitaryHunt::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedMilitaryHuntNameLocal", "Attack {0}'s military ships"), HostileCompany->GetCompanyName());
	if (RequestDestroyTarget)
	{
		QuestDescription = FText::Format(LOCTEXT("GeneratedMilitaryHuntDescriptionLocalFormat1", "Destroy some of {0}'s ships to lower its combat value by {1} combats points"),
			HostileCompany->GetCompanyName(),
			FText::AsNumber(RequestedArmyCombatPoints));
	}
	else
	{
		QuestDescription = FText::Format(LOCTEXT("GeneratedMilitaryHuntDescriptionLocalFormat2", "Render some of {0}'s ships uncontrollable to lower its combat value by {1} combats points"),
			HostileCompany->GetCompanyName(),
			FText::AsNumber(RequestedArmyCombatPoints));
	}


	QuestCategory = EFlareQuestCategory::SECONDARY;


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"CargoMilitaryHarass"
		FText Description;

		Description = LOCTEXT("GeneratedMilitaryHuntHarass", "Attack military ships");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "harass", Description);


		UFlareQuestConditionDestroyCombatValue* Condition = UFlareQuestConditionDestroyCombatValue::Create(this,
																										 QUEST_TAG"cond1",
																										 HostileCompany,
																										 RequestedArmyCombatPoints,
																										 RequestDestroyTarget);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}

// New military contracts

/*----------------------------------------------------
	Generated station defense quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedStationDefense2"
UFlareQuestGeneratedStationDefense2::UFlareQuestGeneratedStationDefense2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedStationDefense2::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareCompany* HostileCompany)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateDefenseTag(Sector, HostileCompany)))
	{
		return NULL;
	}

	int64 WarPrice;

	if (HostileCompany->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		WarPrice = 0;
	}
	else
	{
		WarPrice = 200 * (HostileCompany->GetPlayerReputation() + 100);
	}

	int32 PreferredPlayerCombatPoints = int32(PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints * FMath::FRandRange(0.2,0.5));


	int32 NeedArmyCombatPoints= FMath::Max(0, SectorHelper::GetHostileArmyCombatPoints(Sector, Company, true) - SectorHelper::GetCompanyArmyCombatPoints(Sector, Company, true) /4);


	int32 RequestedArmyCombatPoints = FMath::Min(PreferredPlayerCombatPoints, NeedArmyCombatPoints);

	if(RequestedArmyCombatPoints <= 0)
	{
		return NULL;
	}

	int64 ArmyPrice = RequestedArmyCombatPoints  * 40000;

	// Setup reward
	int64 QuestValue = WarPrice + ArmyPrice;

	// Create the quest
	UFlareQuestGeneratedStationDefense2* Quest = NewObject<UFlareQuestGeneratedStationDefense2>(Parent, UFlareQuestGeneratedStationDefense2::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedStationDefense2::GetClass(), Data);
	Data.PutName("sector", Sector->GetIdentifier());
	Data.PutInt32("army-combat-points", RequestedArmyCombatPoints );
	Data.PutName("friendly-company", Company->GetIdentifier());
	Data.PutName("hostile-company", HostileCompany->GetIdentifier());
	Data.PutTag(Parent->GenerateDefenseTag(Sector, HostileCompany));
	CreateGenericReward(Data, QuestValue, Company);

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedStationDefense2::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");

	QuestClass = UFlareQuestGeneratedStationDefense2::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedStationDefenseNameLocal","Defend stations of {0} against {1}"), Sector->GetSectorName(), HostileCompany->GetCompanyName());
	QuestDescription = FText::Format(LOCTEXT("GeneratedStationDefenseDescriptionLocalFormat","Defend stations of {0} in {1} against {2} with at least {3} combat value."),
								 FriendlyCompany->GetCompanyName(), Sector->GetSectorName(), HostileCompany->GetCompanyName(), FText::AsNumber(ArmyCombatPoints ));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedStationDefenseDescriptionBringForce", "Attack {0} in {1} with at least {2} combat value"),
										  HostileCompany->GetCompanyName(), Sector->GetSectorName(), FText::AsNumber(ArmyCombatPoints ));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);
		{
			// First case : the player bring forces
			UFlareQuestConditionMinArmyCombatPointsInSector* Condition = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		}

		{
			// Failed to defend
			UFlareQuestConditionStationLostInSector* FailCondition = UFlareQuestConditionStationLostInSector::Create(this, Sector, FriendlyCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Defend"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedStationDefenseDescriptionDefendStation", "Defend {0} in {1} against {2}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName(), HostileCompany->GetCompanyName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "defend", Description);

		// OR root
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		{
			UFlareQuestConditionNoCapturingStationInSector* Condition = UFlareQuestConditionNoCapturingStationInSector::Create(this, Sector, FriendlyCompany, HostileCompany);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		// OR
		{
			UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, PlayerCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition1);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));

		Steps.Add(Step);
	}


	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionStationLostInSector::Create(this,  Sector, FriendlyCompany));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionNoCapturingStationInSector::Create(this,  Sector, FriendlyCompany, HostileCompany));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));

	AddGlobalFailCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));

	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}

/*----------------------------------------------------
	Generated joint attack quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedJointAttack2"
UFlareQuestGeneratedJoinAttack2::UFlareQuestGeneratedJoinAttack2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedJoinAttack2::Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateAttackTag(Target.Sector, Company)))
	{
		return NULL;
	}

	int64 WarPrice = 0;

	for(UFlareCompany* HostileCompany : Target.ArmedDefenseCompanies)
	{
		if (HostileCompany->GetWarState(PlayerCompany) != EFlareHostility::Hostile)
		{
			WarPrice += 200 * (HostileCompany->GetPlayerReputation() + 100);
		}
	}

	int32 PreferredPlayerCombatPoints= int32(PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints * FMath::FRandRange(0.2,0.5));


	int32 NeedArmyCombatPoints= FMath::Max(0, Target.EnemyArmyCombatPoints - AttackCombatPoints /4);


	int32 RequestedArmyCombatPoints = FMath::Min(PreferredPlayerCombatPoints, NeedArmyCombatPoints);

	if(RequestedArmyCombatPoints <= 0)
	{
		return NULL;
	}

	int64 ArmyPrice = RequestedArmyCombatPoints  * 60000;

	// Setup reward
	int64 QuestValue = WarPrice + ArmyPrice;

	// Create the quest
	UFlareQuestGeneratedJoinAttack2* Quest = NewObject<UFlareQuestGeneratedJoinAttack2>(Parent, UFlareQuestGeneratedJoinAttack2::StaticClass());

	TArray<FName> HostileCompanies;
	for(UFlareCompany* HostileCompany : Target.ArmedDefenseCompanies)
	{
		HostileCompanies.Add(HostileCompany->GetIdentifier());
	}



	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedJoinAttack2::GetClass(), Data);
	Data.PutName("sector", Target.Sector->GetIdentifier());
	Data.PutInt32("army-combat-points", RequestedArmyCombatPoints );
	Data.PutInt32("attack-date", Parent->GetGame()->GetGameWorld()->GetDate() + TravelDuration);
	Data.PutName("friendly-company", Company->GetIdentifier());
	Data.PutNameArray("hostile-companies", HostileCompanies);
	Data.PutTag(Parent->GenerateAttackTag(Target.Sector, Company));
	CreateGenericReward(Data, QuestValue, Company);

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedJoinAttack2::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	TArray<UFlareCompany*> HostileCompanies;
	TArray<FName> HostileCompanyNames = InitData.GetNameArray("hostile-companies");
	for(FName HostileCompanyName : HostileCompanyNames)
	{
		HostileCompanies.Add(Parent->GetGame()->GetGameWorld()->FindCompany(HostileCompanyName));
	}
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");
	int32 AttackDate = InitData.GetInt32("attack-date");

	QuestClass = UFlareQuestGeneratedJoinAttack2::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedJoinAttackNameLocal","Attack {0} with {1}"),
		Sector->GetSectorName(),
		FriendlyCompany->GetCompanyName());

	QuestDescription = FText::Format(LOCTEXT("GeneratedJoinAttackDescriptionLocalFormat","Follow {0} in {1} with at least {2} combat value, and attack on {3}."),
		FriendlyCompany->GetCompanyName(),
		Sector->GetSectorName(),
		FText::AsNumber(ArmyCombatPoints),
		UFlareGameTools::GetDisplayDate(AttackDate));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedJoinAttackDescriptionBringForce", "Attack {0} with at least {1} combat value on {2}"),
			Sector->GetSectorName(),
			FText::AsNumber(ArmyCombatPoints ),
			UFlareGameTools::GetDisplayDate(AttackDate));

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);
		{
			// First case : the player bring forces
			UFlareQuestConditionMinArmyCombatPointsInSector* Condition = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		}

		{
			// Failed to attack at time
			UFlareQuestConditionAfterDate* FailCondition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));


		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Fight"
		FText Description;

		Description = FText::Format(LOCTEXT("FightAlongsideFormat", "Fight alongside {0} in {1}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "fight", Description);

		// OR root
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		{
			UFlareQuestConditionAfterDate* Condition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		// OR
		{
			UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, PlayerCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition1);

		Steps.Add(Step);
	}


	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAfterDate::Create(this, AttackDate));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionPlayerTravelTooLong::Create(this, Sector, AttackDate));

	for(UFlareCompany* HostileCompany : HostileCompanies)
	{
		Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));
		AddGlobalFailCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));
	}


	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}



/*----------------------------------------------------
	Generated sector defense quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedSectorDefense2"
UFlareQuestGeneratedSectorDefense2::UFlareQuestGeneratedSectorDefense2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedSectorDefense2::Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateDefenseTag(Target.Sector, HostileCompany)))
	{
		return NULL;
	}

	int64 WarPrice = 0;


	if (HostileCompany->GetWarState(PlayerCompany) != EFlareHostility::Hostile)
	{
		WarPrice += 200 * (HostileCompany->GetPlayerReputation() + 100);
	}

	int32 PreferredPlayerCombatPoints = int32(PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints * FMath::FRandRange(0.2,0.5));


	int32 NeedArmyCombatPoints= FMath::Max(0, AttackCombatPoints - Target.EnemyArmyCombatPoints /4);


	FLOGV("PreferredPlayerCombatPoints %d", PreferredPlayerCombatPoints);
	FLOGV("Target.OwnedArmyCombatPoints %d", Target.OwnedArmyCombatPoints);
	FLOGV("Target.EnemyArmyCombatPoints %d", Target.EnemyArmyCombatPoints);
	FLOGV("NeedArmyCombatPoints %d", NeedArmyCombatPoints);

	int32 RequestedArmyCombatPoints = FMath::Min(PreferredPlayerCombatPoints, NeedArmyCombatPoints);

	FLOGV("RequestedArmyCombatPoints %d", RequestedArmyCombatPoints);

	if(RequestedArmyCombatPoints <= 0)
	{
		return NULL;
	}

	int64 ArmyPrice = RequestedArmyCombatPoints  * 60000;

	// Setup reward
	int64 QuestValue = WarPrice + ArmyPrice;

	// Create the quest
	UFlareQuestGeneratedSectorDefense2* Quest = NewObject<UFlareQuestGeneratedSectorDefense2>(Parent, UFlareQuestGeneratedSectorDefense2::StaticClass());


	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedSectorDefense2::GetClass(), Data);
	Data.PutName("sector", Target.Sector->GetIdentifier());
	Data.PutInt32("army-combat-points", RequestedArmyCombatPoints );
	Data.PutInt32("attack-date", Parent->GetGame()->GetGameWorld()->GetDate() + TravelDuration);
	Data.PutName("friendly-company", Company->GetIdentifier());
	Data.PutName("hostile-company", HostileCompany->GetIdentifier());
	Data.PutTag(Parent->GenerateDefenseTag(Target.Sector, HostileCompany));
	CreateGenericReward(Data, QuestValue, Company);

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedSectorDefense2::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();
	UFlareSimulatedSector* Sector = Parent->GetGame()->GetGameWorld()->FindSector(InitData.GetName("sector"));

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 ArmyCombatPoints = InitData.GetInt32("army-combat-points");
	int32 AttackDate = InitData.GetInt32("attack-date");

	QuestClass = UFlareQuestGeneratedSectorDefense2::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedSectorDefenseNameLocal","Defend {0} against {1}'s attack"),
		Sector->GetSectorName(),
		HostileCompany->GetCompanyName());

	QuestDescription = FText::Format(LOCTEXT("GeneratedSectorDefenseDescriptionLocalFormat","Defend {0} in {1} with at least {2} combat value on {3}."),
		FriendlyCompany->GetCompanyName(),
		Sector->GetSectorName(),
		FText::AsNumber(ArmyCombatPoints),
		UFlareGameTools::GetDisplayDate(AttackDate));

	QuestCategory = EFlareQuestCategory::SECONDARY;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Attack"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedSectorDefenseDescriptionBringForce", "Attack {0} with at least {1} combat value on {2}"),
			Sector->GetSectorName(),
			FText::AsNumber(ArmyCombatPoints),
			UFlareGameTools::GetDisplayDate(AttackDate));

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "attack", Description);
		{
			// First case : the player bring forces
			UFlareQuestConditionMinArmyCombatPointsInSector* Condition = UFlareQuestConditionMinArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, ArmyCombatPoints);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		}

		{
			// Failed to attack at time
			UFlareQuestConditionAfterDate* FailCondition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition);
		}

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector));


		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Defend"
		FText Description;

		Description = FText::Format(LOCTEXT("GeneratedSectorDefenseDescriptionDefendStation", "Defend {0} in {1}"),
										  FriendlyCompany->GetCompanyName(), Sector->GetSectorName());
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "defend", Description);

		// OR root
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		{
			UFlareQuestConditionAfterDate* Condition = UFlareQuestConditionAfterDate::Create(this, AttackDate);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		// OR
		{
			UFlareQuestConditionMaxArmyCombatPointsInSector* Condition = UFlareQuestConditionMaxArmyCombatPointsInSector::Create(this, Sector, PlayerCompany, 0);
			Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);
		}

		UFlareQuestConditionRetreatDangerousShip* FailCondition1 = UFlareQuestConditionRetreatDangerousShip::Create(this, Sector, PlayerCompany);
		Cast<UFlareQuestConditionGroup>(Step->GetFailCondition())->AddChildCondition(FailCondition1);

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAfterDate::Create(this, AttackDate));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionPlayerTravelTooLong::Create(this, Sector, AttackDate));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));
	AddGlobalFailCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));


	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);


	return true;
}


/*----------------------------------------------------
	Generated cargo hunt quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedCargoHunt2"
UFlareQuestGeneratedCargoHunt2::UFlareQuestGeneratedCargoHunt2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedCargoHunt2::Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateHarassTag(Company, HostileCompany)))
	{
		return NULL;
	}

	int64 WarPrice;

	if (HostileCompany->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		WarPrice = 0;
	}
	else
	{
		WarPrice = 200 * (HostileCompany->GetPlayerReputation() + 100);
	}

	int32 PreferredPlayerCombatPoints= int32(PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints);

	bool RequestDestroyTarget = FMath::Rand() > 0.9f;


	int32 SmallCargoCount = 0;
	int32 LargeCargoCount = 0;
	int32 SmallCargoValue = 30;
	int32 LargeCargoValue = 300;

	for(UFlareSimulatedSpacecraft* Ship : HostileCompany->GetCompanyShips())
	{
		if(RequestDestroyTarget && Ship->GetDamageSystem()->IsUncontrollable())
		{
			continue;
		}

		if(Ship->IsMilitary())
		{
			continue;
		}

		if(Ship->GetSize() == EFlarePartSize::S)
		{
			SmallCargoCount++;
		}
		else
		{
			LargeCargoCount++;
		}
	}

	int32 NeedArmyCombatPoints = (SmallCargoCount * SmallCargoValue + LargeCargoCount * LargeCargoValue) / (4 * (RequestDestroyTarget ? 2 : 1));


	int32 TheoricalRequestedArmyCombatPoints = FMath::Min(PreferredPlayerCombatPoints, NeedArmyCombatPoints);

	bool TargetLargeCargo = false;
	if (LargeCargoCount > 0 && TheoricalRequestedArmyCombatPoints > LargeCargoValue)
	{
		TargetLargeCargo = FMath::RandBool();
	}

	int32 RequestedArmyCombatPoints;
	int32 CargoCount;

	if(TargetLargeCargo)
	{
		CargoCount = TheoricalRequestedArmyCombatPoints / LargeCargoValue;
		RequestedArmyCombatPoints = LargeCargoValue * CargoCount;
	}
	else
	{
		CargoCount = FMath::Max(1,TheoricalRequestedArmyCombatPoints / SmallCargoValue);
		RequestedArmyCombatPoints = SmallCargoValue * CargoCount;
	}


	/*FLOGV("SmallCargoCount %d", SmallCargoCount);
	FLOGV("NeedArmyCombatPoints %d", NeedArmyCombatPoints);
	FLOGV("PreferredPlayerCombatPoints %d", PreferredPlayerCombatPoints);
	FLOGV("TheoricalRequestedArmyCombatPoints %d", TheoricalRequestedArmyCombatPoints);
	FLOGV("CargoCount %d", CargoCount);
	FLOGV("RequestedArmyCombatPoints %d", RequestedArmyCombatPoints);*/


	if(RequestedArmyCombatPoints <= 0)
	{
		return NULL;
	}

	int64 ArmyPrice = RequestedArmyCombatPoints  * 3000;

	// Setup reward
	int64 QuestValue = WarPrice + ArmyPrice * (RequestDestroyTarget ? 1.5f: 1.f);

	// Create the quest
	UFlareQuestGeneratedCargoHunt2* Quest = NewObject<UFlareQuestGeneratedCargoHunt2>(Parent, UFlareQuestGeneratedCargoHunt2::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedCargoHunt2::GetClass(), Data);
	Data.PutInt32("cargo-count", CargoCount );
	Data.PutInt32("destroy-cargo", RequestDestroyTarget);
	Data.PutInt32("large-cargo", TargetLargeCargo);
	Data.PutName("friendly-company", Company->GetIdentifier());
	Data.PutName("hostile-company", HostileCompany->GetIdentifier());
	Data.PutTag(Parent->GenerateHarassTag(Company, HostileCompany));
	CreateGenericReward(Data, QuestValue, Company);

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedCargoHunt2::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 CargoCount = InitData.GetInt32("cargo-count");
	bool RequestDestroyTarget = InitData.GetInt32("destroy-cargo") > 0;
	bool TargetLargeCargo = InitData.GetInt32("large-cargo") > 0;

	QuestClass = UFlareQuestGeneratedCargoHunt2::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedCargoHuntNameLocal","Attack {0}'s cargos"), HostileCompany->GetCompanyName());

	QuestDescription = FText::Format(LOCTEXT("GeneratedCargoHuntDescriptionLocalFormat","{0} {1} {2} cargos to {3}."),
								 RequestDestroyTarget? LOCTEXT("RequestDestroyTarget","Destroy") : LOCTEXT("RequestUncotrollableTarget","Make uncontrollable"),
									 FText::AsNumber(CargoCount),
									 TargetLargeCargo? LOCTEXT("TargetLargeCargo","large") : LOCTEXT("TargetSmallCargo","small"),
									 HostileCompany->GetCompanyName());

	QuestCategory = EFlareQuestCategory::SECONDARY;


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"CargoHarass"
		FText Description;

		Description = LOCTEXT("GeneratedCargoHuntHarass", "Attack unarmed, defenseless cargos");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "harass", Description);


		UFlareQuestConditionDestroySpacecraft* Condition = UFlareQuestConditionDestroySpacecraft::Create(this,
																										 QUEST_TAG"cond1",
																										 HostileCompany,
																										 CargoCount,
																										 false,
																										 TargetLargeCargo ? EFlarePartSize::L : EFlarePartSize::S,
																										 RequestDestroyTarget);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));
	AddGlobalFailCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));


	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}

/*----------------------------------------------------
	Generated military hunt quest
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "GeneratedMilitaryHunt2"
UFlareQuestGeneratedMilitaryHunt2::UFlareQuestGeneratedMilitaryHunt2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedMilitaryHunt2::Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	if (Company->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		return NULL;
	}

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateHarassTag(Company, HostileCompany)))
	{
		return NULL;
	}

	int64 WarPrice;

	if (HostileCompany->GetWarState(PlayerCompany) == EFlareHostility::Hostile)
	{
		WarPrice = 0;
	}
	else
	{
		WarPrice = 200 * (HostileCompany->GetPlayerReputation() + 100);
	}

	int32 PreferredPlayerCombatPoints= int32(PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints);

	bool RequestDestroyTarget = FMath::Rand() > 0.9;


	int32 NeedArmyCombatPoints = HostileCompany->GetCompanyValue().ArmyCurrentCombatPoints * FMath::FRandRange(0.1,0.5);

	int32 RequestedArmyCombatPoints = FMath::Min(PreferredPlayerCombatPoints, NeedArmyCombatPoints);


	FLOGV("NeedArmyCombatPoints %d", NeedArmyCombatPoints);
	FLOGV("PreferredPlayerCombatPoints %d", PreferredPlayerCombatPoints);
	FLOGV("RequestedArmyCombatPoints %d", RequestedArmyCombatPoints);


	if(RequestedArmyCombatPoints <= 0)
	{
		return NULL;
	}

	int64 ArmyPrice = RequestedArmyCombatPoints  * 20000;

	// Setup reward
	int64 QuestValue = WarPrice + ArmyPrice * (RequestDestroyTarget ? 1.5f: 1.f);

	// Create the quest
	UFlareQuestGeneratedMilitaryHunt2* Quest = NewObject<UFlareQuestGeneratedMilitaryHunt2>(Parent, UFlareQuestGeneratedMilitaryHunt2::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedMilitaryHunt2::GetClass(), Data);
	Data.PutInt32("army-combat-points", RequestedArmyCombatPoints );
	Data.PutInt32("destroy", RequestDestroyTarget);
	Data.PutName("friendly-company", Company->GetIdentifier());
	Data.PutName("hostile-company", HostileCompany->GetIdentifier());
	Data.PutTag(Parent->GenerateHarassTag(Company, HostileCompany));
	CreateGenericReward(Data, QuestValue, Company);

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedMilitaryHunt2::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	UFlareCompany* FriendlyCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("friendly-company"));
	UFlareCompany* HostileCompany = Parent->GetGame()->GetGameWorld()->FindCompany(InitData.GetName("hostile-company"));
	int32 RequestedArmyCombatPoints = InitData.GetInt32("army-combat-points");
	bool RequestDestroyTarget = InitData.GetInt32("destroy") > 0;

	QuestClass = UFlareQuestGeneratedMilitaryHunt2::GetClass();
	Identifier = InitData.GetName("identifier");

	QuestName = FText::Format(LOCTEXT("GeneratedMilitaryHuntNameLocal", "Attack {0}'s military ships"), HostileCompany->GetCompanyName());
	if (RequestDestroyTarget)
	{
		QuestDescription = FText::Format(LOCTEXT("GeneratedMilitaryHuntDescriptionLocalFormat1", "Destroy some of {0}'s ships to lower its combat value by {1} combats points"),
			HostileCompany->GetCompanyName(),
			FText::AsNumber(RequestedArmyCombatPoints));
	}
	else
	{
		QuestDescription = FText::Format(LOCTEXT("GeneratedMilitaryHuntDescriptionLocalFormat2", "Render some of {0}'s ships uncontrollable to lower its combat value by {1} combats points"),
			HostileCompany->GetCompanyName(),
			FText::AsNumber(RequestedArmyCombatPoints));
	}


	QuestCategory = EFlareQuestCategory::SECONDARY;


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"CargoMilitaryHarass"
		FText Description;

		Description = LOCTEXT("GeneratedMilitaryHuntHarass", "Attack military ships");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "harass", Description);


		UFlareQuestConditionDestroyCombatValue* Condition = UFlareQuestConditionDestroyCombatValue::Create(this,
																										 QUEST_TAG"cond1",
																										 HostileCompany,
																										 RequestedArmyCombatPoints,
																										 RequestDestroyTarget);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionTimeAfterAvailableDate::Create(this, 10));
	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));
	AddGlobalFailCondition(UFlareQuestConditionWorkFor::Create(this, HostileCompany));


	SetupQuestGiver(FriendlyCompany, true);
	SetupGenericReward(Data);

	return true;
}

/*----------------------------------------------------
	Generated meteorite destruction quest
----------------------------------------------------*/
UFlareQuestGeneratedMeteoriteInterception::UFlareQuestGeneratedMeteoriteInterception(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestGenerated* UFlareQuestGeneratedMeteoriteInterception::Create(UFlareQuestGenerator* Parent, UFlareSimulatedSpacecraft* TargetStation, float Power, int32 Count, int64 TimeBeforeImpact)
{
	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	// Check unicity
	if (Parent->FindUniqueTag(Parent->GenerateMeteoriteTag(TargetStation->GetCurrentSector())))
	{
		return NULL;
	}

	if (PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints == 0)
	{
		return NULL;
	}

	// Setup reward
	int64 QuestValue = 500000 * Power;

	// Create the quest
	UFlareQuestGeneratedMeteoriteInterception* Quest = NewObject<UFlareQuestGeneratedMeteoriteInterception>(Parent, UFlareQuestGeneratedMeteoriteInterception::StaticClass());

	FFlareBundle Data;
	Parent->GenerateIdentifer(UFlareQuestGeneratedMeteoriteInterception::GetClass(), Data);
	Data.PutName("target-station", TargetStation->GetImmatriculation());
	Data.PutInt32("intercept-date", Parent->GetGame()->GetGameWorld()->GetDate() + TimeBeforeImpact);
	Data.PutInt32("meteorite-count", Count);

	Data.PutTag(Parent->GenerateMeteoriteTag(TargetStation->GetCurrentSector()));
	CreateGenericReward(Data, QuestValue, TargetStation->GetCompany());

	Quest->Load(Parent, Data);

	return Quest;
}

bool UFlareQuestGeneratedMeteoriteInterception::Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data)
{
	UFlareQuestGenerated::Load(Parent, Data);

	UFlareCompany* PlayerCompany = Parent->GetGame()->GetPC()->GetCompany();

	UFlareSimulatedSpacecraft* TargetStation = Parent->GetGame()->GetGameWorld()->FindSpacecraft(InitData.GetName("target-station"));
	int32 InterseptDate = InitData.GetInt32("intercept-date");
	int32 MeteoriteCount = InitData.GetInt32("meteorite-count");

	QuestClass = UFlareQuestGeneratedMeteoriteInterception::GetClass();
	Identifier = InitData.GetName("identifier");

	FLOGV("TargetStation %p", TargetStation);

	if(TargetStation->IsDestroyed())
	{
		return false;
	}

	QuestName = FText::Format(LOCTEXT("GeneratedMeteoriteDestructionName", "Meteorite intercept in {0}"), TargetStation->GetCurrentSector()->GetSectorName());
	QuestDescription = FText::Format(LOCTEXT("GeneratedMeteoriteDestructionDescription", "A meteorite group will pose a threat to {0} in {1} on {2}. Intercept it and destroy it."),
		UFlareGameTools::DisplaySpacecraftName(TargetStation),
		TargetStation->GetCurrentSector()->GetSectorName(),
		UFlareGameTools::GetDisplayDate(InterseptDate + 1));



	QuestCategory = EFlareQuestCategory::SECONDARY;


	{
		FText Description;

		Description = LOCTEXT("GeneratedMeteoriteDestructionPrepareToInterceptoon", "Prepare to intercept");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "prepare", Description);


		UFlareQuestConditionAfterDate* Condition = UFlareQuestConditionAfterDate::Create(this, InterseptDate);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, TargetStation->GetCurrentSector()));

		Steps.Add(Step);
	}

	{
		FText Description;

		Description = LOCTEXT("GeneratedMeteoriteDestructionInterception", "Destroy the meteorites");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "destroy", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCounterCondition::Create(this,
																																			  [=](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("meteorite-destroyed") && Bundle.GetName("sector") == TargetStation->GetCurrentSector()->GetIdentifier())
			{
				return 1;
			}

			if(Bundle.HasTag("meteorite-miss-station") && Bundle.GetName("sector") == TargetStation->GetCurrentSector()->GetIdentifier())
			{
				return 1;
			}
			return 0;
		},
		[=]()
		{
			return FText::Format(LOCTEXT("MeteoriteDestroyedConditionLabel", "Destroy {0} meteorites in {1}"), FText::AsNumber(MeteoriteCount), TargetStation->GetCurrentSector()->GetSectorName());
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		},  "DestroyMeteoriteCond1", MeteoriteCount));

		Steps.Add(Step);
	}

	Cast<UFlareQuestConditionGroup>(ExpirationCondition)->AddChildCondition(UFlareQuestConditionAfterDate::Create(this, InterseptDate));


	AddGlobalFailCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																		  [=](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
	{
		if(Bundle.HasTag("meteorite-hit-station") && Bundle.GetName("sector") == TargetStation->GetCurrentSector()->GetIdentifier())
		{
			return true;
		}
		return false;
	},
	[]()
	{
		return LOCTEXT("MeteoriteCrashConditionLabel", "Meteorite crash on a station");
	},
	[](UFlareQuestCondition* Condition)
	{
		Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	}));


	SetupQuestGiver(TargetStation->GetCompany(), true);
	SetupGenericReward(Data);

	return true;
}

#undef LOCTEXT_NAMESPACE

