
#include "FlareHistoryQuest.h"
#include "Flare.h"

#include "Data/FlareResourceCatalog.h"
#include "Data/FlareSpacecraftCatalog.h"

#include "Economy/FlareCargoBay.h"

#include "Game/FlareGame.h"
#include "Game/FlareGameTools.h"
#include "Game/FlareScenarioTools.h"

#include "Player/FlarePlayerController.h"

#include "../FlareQuestCondition.h"
#include "../FlareQuestStep.h"
#include "../FlareQuestAction.h"

#include "FlareTutorialQuest.h"


#define LOCTEXT_NAMESPACE "FlareHistotyQuest"

static FName HISTORY_CURRENT_PROGRESSION_TAG("current-progression");
static FName HISTORY_START_DATE_TAG("start-date");


/*----------------------------------------------------
	Quest pendulum
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "Pendulum"
UFlareQuestPendulum::UFlareQuestPendulum(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestPendulum::Create(UFlareQuestManager* Parent)
{
	UFlareQuestPendulum* Quest = NewObject<UFlareQuestPendulum>(Parent, UFlareQuestPendulum::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestPendulum::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "pendulum";
	QuestName = LOCTEXT("PendulumName","Pendulum");
	QuestDescription = LOCTEXT("PendulumDescription","Help has been requested to all companies around Nema.");
	QuestCategory = EFlareQuestCategory::HISTORY;

	UFlareSimulatedSector* Pendulum = FindSector("pendulum");
	if (!Pendulum)
	{
		return;
	}
	UFlareSimulatedSector* BlueHeart = FindSector("blue-heart");
	if (!BlueHeart)
	{
		return;
	}
	UFlareSimulatedSector* TheSpire = FindSector("the-spire");
	if (!TheSpire)
	{
		return;
	}

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestMinSectorStationCount::Create(this, TheSpire, 10));



	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = LOCTEXT("DockAtDescription", "You are summoned by the Nema Colonial Administration to a meeting on the Pendulum project. Please dock at one of the habitation centers of the Blue Heart capital station.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "meeting", Description);

		UFlareQuestConditionDockAtType* Condition = UFlareQuestConditionDockAtType::Create(this, BlueHeart, "station-bh-habitation");

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TravelToSpire"
		FText Description = LOCTEXT("TravelToSpireDescription","Thanks for attending this meeting. As you may have observed, the Spire, our only source of gas, can't match our needs anymore. We need to build a new orbital extractor. \nHowever, the Spire was built before the war and all the knowledge disappeared when the Daedalus carrier was shot to pieces. Your help is needed to build a new one.\nPlease start reverse-engineering the Spire so that we can learn more.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel-to-spire", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionVisitSector::Create(this, TheSpire));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Inspect"
		FText Description = LOCTEXT("InspectDescription","Inspect The Spire in detail to gather information about its materials and construction methods. Our agent attached transmitter beacons on the structure to help you locating valuable data.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "inspect-the-spire", Description);

		TArray<FVector> Waypoints;
		TArray<FText> WaypointTexts;
		TArray<FText> CustomInitialLabels;

		Waypoints.Add(FVector(0,0,-211084));
		Waypoints.Add(FVector(0,0,-98414));
		Waypoints.Add(FVector(-34858.507812, 27965.390625, 291240.156250));
		Waypoints.Add(FVector(-6500.073242, -43279.824219, 290588.531250));
		Waypoints.Add(FVector(-1783.690918, 2219.517334, 79133.492188));
		Waypoints.Add(FVector(0,0,12563.443359));
		Waypoints.Add(FVector(3321.726562, 37.321136, 3937.916504));
		Waypoints.Add(FVector(901.953857, 174.950699, -12089.154297));

		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel1", "Inspect the pipe at the reference point A in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel2", "Inspect the pipe at the reference point B in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel3", "Inspect the counterweight attachement #1 in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel4", "Inspect the counterweight attachement #3 in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel5", "Inspect the counterweight cables in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel6", "Inspect the counterweight station attachement in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel7", "Inspect the extraction module in {0}"), TheSpire->GetSectorName()));
		CustomInitialLabels.Add(FText::Format(LOCTEXT("InspectLabel8", "Inspect the pipe attachement in {0}"), TheSpire->GetSectorName()));

		WaypointTexts.Add(LOCTEXT("WaypointText1", "Pipe inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText2", "Pipe inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText3", "Attachement inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText4", "Attachement inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText5", "Cables inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText6", "Attachement inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText7", "Module inspected, {0} left"));
		WaypointTexts.Add(LOCTEXT("WaypointText8", "Attachement inspected. Good job !"));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionWaypoints::Create(this, QUEST_TAG"cond1", TheSpire, Waypoints, 16000, true, CustomInitialLabels, WaypointTexts));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DataReturn"
		FText Description = LOCTEXT("DataReturnDescription", "This is great data ! Come back to our scientists in Blue Heart to see what they make of it.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "data-return", Description);

		UFlareQuestConditionDockAtType* Condition = UFlareQuestConditionDockAtType::Create(this, BlueHeart, "station-bh-habitation");

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"research"
		FText Description = LOCTEXT("ResearchDescription","The data you brought back will help us understand how the Spire was built. We need you to transform this knowledge into a usable technology to build a new space elevator.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockTechnology::Create(this, "orbital-pumps"));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockTechnology::Create(this, "science"));

		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"telescope"
		FText Description = LOCTEXT("telescopeDescription","Congratulations, you've just acquired the blueprints for a new orbital extractor. This is huge ! Two things are still needed : construction resources, and a massive counterweight for the space elevator. Build a telescope station to help finding a good sector.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "telescope", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestStationCount::Create(this, "station-telescope", 1));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"resources"
		FText Description = LOCTEXT("resourcesDescription","How many freighters do you own ? We need you to bring ships loaded with construction resources in Blue Heart.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "resources", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "steel", 1000));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "plastics", 1000));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "tech", 500));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestBringResource::Create(this, BlueHeart, "tools", 500));

		Step->GetEndActions().Add(UFlareQuestActionTakeResources::Create(this, BlueHeart, "steel", 1000));
		Step->GetEndActions().Add(UFlareQuestActionTakeResources::Create(this, BlueHeart, "plastics", 1000));
		Step->GetEndActions().Add(UFlareQuestActionTakeResources::Create(this, BlueHeart, "tech", 500));
		Step->GetEndActions().Add(UFlareQuestActionTakeResources::Create(this, BlueHeart, "tools", 500));

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"peace"
		FText Description = LOCTEXT("peaceDescription","We've started construction for the Pendulum elevator. You need to be cautious of pirate activity, since we gathered a lot of resources here.\nReduce pirates to silence for a while. You should use your orbital telescope to locate their Boneyard base.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pirates", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareCompanyMaxCombatValue::Create(this, GetQuestManager()->GetGame()->GetScenarioTools()->Pirates, 0));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"wait"
		FText Description = LOCTEXT("waitDescription","Good job. We'll get this up and running now.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "wait", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionWait::Create(this, QUEST_TAG"cond1", 30));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TravelToPendulum"
		FText Description = LOCTEXT("TravelToPendulumDescription","The construction of Pendulum construction is finished, come take a look !");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel-to-pendulum", Description);

		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Pendulum));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorVisited::Create(this, Pendulum));
		Steps.Add(Step);
	}


	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"pumps"
		FText Description = LOCTEXT("pumpsDescription","Welcome to the Pendulum. Other companies will come here too, but you helped build this extractor, so you've earned a timed exclusivity. Build some gas terminals while the administrative fees are low !");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pumps", Description);
		Step->SetEndCondition(UFlareQuestConditionOrGroup::Create(this, true));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialHaveStation::Create(this, false, "station-ch4-pump", Pendulum));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialHaveStation::Create(this, false, "station-h2-pump", Pendulum));
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialHaveStation::Create(this, false, "station-he3-pump", Pendulum));

		Steps.Add(Step);
	}

	GetSuccessActions().Add(UFlareQuestActionGeneric::Create(this,
															 [this](){
		GetQuestManager()->GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_PENDULUM", 1);
	}));

}


/*----------------------------------------------------
	Quest dock at type condition
----------------------------------------------------*/
UFlareQuestConditionDockAtType::UFlareQuestConditionDockAtType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionDockAtType* UFlareQuestConditionDockAtType::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName StationType)
{
	UFlareQuestConditionDockAtType* Condition = NewObject<UFlareQuestConditionDockAtType>(ParentQuest, UFlareQuestConditionDockAtType::StaticClass());
	Condition->Load(ParentQuest, Sector, StationType);
	return Condition;
}

void UFlareQuestConditionDockAtType::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName StationType)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::SHIP_DOCKED);
	TargetStationType = StationType;
	TargetSector = Sector;
	Completed = false;

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(TargetStationType);

	if(Desc)
	{
		InitialLabel = FText::Format(LOCTEXT("DockAtTypeFormat", "Dock at a {0} in {1}"),
													Desc->Name,
													TargetSector->GetSectorName());
	}

}

bool UFlareQuestConditionDockAtType::IsCompleted()
{
	if (Completed)
	{
		return true;
	}

	if (GetGame()->GetPC()->GetPlayerShip() && GetGame()->GetPC()->GetPlayerShip()->IsActive())
	{
		AFlareSpacecraft* PlayerShip = GetGame()->GetPC()->GetPlayerShip()->GetActive();

		if(PlayerShip->GetNavigationSystem()->GetDockStation())
		{
		if (PlayerShip->GetNavigationSystem()->GetDockStation()->GetDescription()->Identifier == TargetStationType)
			{
				Completed = true;
				return true;
			}
		}
	}

	return false;
}

void UFlareQuestConditionDockAtType::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);

	for (UFlareSimulatedSpacecraft* Station : TargetSector->GetSectorStations())
	{
		if (Station->GetDescription()->Identifier == TargetStationType)
		{
			ObjectiveData->AddTargetSpacecraft(Station);
		}
	}
}

/*----------------------------------------------------
	Visit sector condition
----------------------------------------------------*/
UFlareQuestConditionVisitSector::UFlareQuestConditionVisitSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionVisitSector* UFlareQuestConditionVisitSector::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector)
{
	UFlareQuestConditionVisitSector* Condition = NewObject<UFlareQuestConditionVisitSector>(ParentQuest, UFlareQuestConditionVisitSector::StaticClass());
	Condition->Load(ParentQuest, Sector);
	return Condition;
}

void UFlareQuestConditionVisitSector::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetSector = Sector;
	InitialLabel = FText::Format(LOCTEXT("VisitSector", "Visit {0}"), TargetSector->GetSectorName());
}

void UFlareQuestConditionVisitSector::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("travel-end") && Bundle.GetName("sector") == TargetSector->GetIdentifier())
	{
		Completed = true;
	}
}

bool UFlareQuestConditionVisitSector::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionVisitSector::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}

/*----------------------------------------------------
	Follow absolute waypoints condition
----------------------------------------------------*/

UFlareQuestConditionWaypoints::UFlareQuestConditionWaypoints(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionWaypoints* UFlareQuestConditionWaypoints::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, UFlareSimulatedSector* Sector, TArray<FVector> VectorListParam, float Radius, bool RequiresScan, TArray<FText> CustomInitialLabelsParam, TArray<FText> CustomWaypointTextsParam)
{
	UFlareQuestConditionWaypoints*Condition = NewObject<UFlareQuestConditionWaypoints>(ParentQuest, UFlareQuestConditionWaypoints::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, Sector, VectorListParam, Radius, RequiresScan, CustomInitialLabelsParam, CustomWaypointTextsParam);
	return Condition;
}

void UFlareQuestConditionWaypoints::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, UFlareSimulatedSector* Sector, TArray<FVector> VectorListParam, float Radius, bool RequiresScan, TArray<FText> CustomInitialLabelsParam, TArray<FText> CustomWaypointTextsParam)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionWaypoints need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	VectorList = VectorListParam;
	TargetSector = Sector;
	TargetRequiresScan = RequiresScan;
	CustomInitialLabels = CustomInitialLabelsParam;
	CustomWaypointTexts = CustomWaypointTextsParam;
	TargetRadius = Radius;


	if (RequiresScan)
	{
		InitialLabel = FText::Format(LOCTEXT("ScanWaypointsInSector", "Analyze signals in {0}"), TargetSector->GetSectorName());
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("FollowWaypointsInSector", "Fly to waypoints in {0}"), TargetSector->GetSectorName());
	}
}

void UFlareQuestConditionWaypoints::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(HISTORY_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		IsInit = true;
		CurrentProgression = Bundle->GetInt32(HISTORY_CURRENT_PROGRESSION_TAG);
		UpdateInitalLabel();
	}
	else
	{
		IsInit = false;
	}

}

void UFlareQuestConditionWaypoints::Save(FFlareBundle* Bundle)
{
	if (IsInit)
	{
		Bundle->PutInt32(HISTORY_CURRENT_PROGRESSION_TAG, CurrentProgression);
	}
}

void UFlareQuestConditionWaypoints::Init()
{
	if(IsInit)
	{
		return;
	}

	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();

	if (Spacecraft)
	{
		IsInit = true;
		CurrentProgression = 0;
		UpdateInitalLabel();
	}
}

void UFlareQuestConditionWaypoints::UpdateInitalLabel()
{
	if(CurrentProgression < CustomInitialLabels.Num())
	{
		InitialLabel = CustomInitialLabels[CurrentProgression];
	}
}

bool UFlareQuestConditionWaypoints::IsCompleted()
{
	AFlareSpacecraft* Spacecraft = GetPC()->GetShipPawn();
	if (Spacecraft)
	{
		Init();
		FVector WorldTargetLocation = VectorList[CurrentProgression];
		float MaxDistance = TargetRadius;
		
		if (Spacecraft->GetParent()->GetCurrentSector() == TargetSector)
		{
			// Waypoint completion logic
			bool HasCompletedWaypoint = false;
			if (TargetRequiresScan)
			{
				if (Spacecraft->IsWaypointScanningFinished())
				{
					HasCompletedWaypoint = true;
				}
			}
			else if (FVector::Dist(Spacecraft->GetActorLocation(), WorldTargetLocation) < MaxDistance)
			{
				HasCompletedWaypoint = true;
			}

			if (HasCompletedWaypoint)
			{
				// Nearing the target
				if (CurrentProgression + 2 <= VectorList.Num())
				{
					CurrentProgression++;
					UpdateInitalLabel();

					FText WaypointText;

					if(CurrentProgression-1 < CustomWaypointTexts.Num())
					{
						WaypointText = CustomWaypointTexts[CurrentProgression-1];
					}
					else if (TargetRequiresScan)
					{
						WaypointText = LOCTEXT("ScanProgress", "Signal analyzed, {0} left");
					}
					else
					{
						WaypointText = LOCTEXT("WaypointProgress", "Waypoint reached, {0} left");
					}

					Quest->SendQuestNotification(FText::Format(WaypointText, FText::AsNumber(VectorList.Num() - CurrentProgression)),
						FName(*(FString("quest-")+GetIdentifier().ToString()+"-step-progress")),
						false);
				}
				else
				{
					// All waypoints reached
					return true;
				}
			}
		}
	}
	return false;
}

void UFlareQuestConditionWaypoints::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	Init();
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = VectorList.Num();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = VectorList.Num();
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	for (int TargetIndex = 0; TargetIndex < VectorList.Num(); TargetIndex++)
	{
		if (TargetIndex < CurrentProgression)
		{
			// Don't show old target
			continue;
		}
		FFlarePlayerObjectiveTarget ObjectiveTarget;
		ObjectiveTarget.RequiresScan = TargetRequiresScan;
		ObjectiveTarget.Actor = NULL;
		ObjectiveTarget.Active = (CurrentProgression == TargetIndex);
		ObjectiveTarget.Radius = TargetRadius;

		FVector WorldTargetLocation = VectorList[TargetIndex]; // In cm

		ObjectiveTarget.Location = WorldTargetLocation;
		ObjectiveData->TargetList.Add(ObjectiveTarget);
	}
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}


/*----------------------------------------------------
Station count condition
----------------------------------------------------*/
UFlareQuestStationCount::UFlareQuestStationCount(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestStationCount* UFlareQuestStationCount::Create(UFlareQuest* ParentQuest, FName StationIdentifier, int32 StationCount)
{
	UFlareQuestStationCount* Condition = NewObject<UFlareQuestStationCount>(ParentQuest, UFlareQuestStationCount::StaticClass());
	Condition->Load(ParentQuest, StationIdentifier, StationCount);
	return Condition;
}

void UFlareQuestStationCount::Load(UFlareQuest* ParentQuest, FName StationIdentifier, int32 StationCount)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetStationCount = StationCount;
	TargetStationIdentifier = StationIdentifier;

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(TargetStationIdentifier);



	if(TargetStationCount == 1)
	{
		InitialLabel = FText::Format(LOCTEXT("HaveOneSpecificStation", "Build a {0} "), Desc->Name);
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("HaveMultipleSpecificStation", "Build {0} {1} "), FText::AsNumber(StationCount), Desc->Name);
	}
}

int32 UFlareQuestStationCount::GetStationCount()
{
	int StationCount = 0;
	for(UFlareSimulatedSpacecraft* Station : GetGame()->GetPC()->GetCompany()->GetCompanyStations())
	{
		if(Station->GetDescription()->Identifier == TargetStationIdentifier && !Station->IsUnderConstruction())
		{
			StationCount++;
		}
	}
	return StationCount;
}

bool UFlareQuestStationCount::IsCompleted()
{
	return GetStationCount() >= TargetStationCount;
}

void UFlareQuestStationCount::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = TargetStationCount;
	ObjectiveCondition.MaxProgress = TargetStationCount;
	ObjectiveCondition.Counter = GetStationCount();
	ObjectiveCondition.Progress = GetStationCount();
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Bring resource condition
----------------------------------------------------*/
UFlareQuestBringResource::UFlareQuestBringResource(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestBringResource* UFlareQuestBringResource::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName ResourceIdentifier, int32 Count)
{
	UFlareQuestBringResource* Condition = NewObject<UFlareQuestBringResource>(ParentQuest, UFlareQuestBringResource::StaticClass());
	Condition->Load(ParentQuest, Sector, ResourceIdentifier, Count);
	return Condition;
}

void UFlareQuestBringResource::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, FName ResourceIdentifier, int32 Count)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetResourceCount = Count;
	TargetResourceIdentifier = ResourceIdentifier;
	TargetSector = Sector;

	FFlareResourceDescription* Desc = GetGame()->GetResourceCatalog()->Get(TargetResourceIdentifier);


	if(Desc)
	{
		InitialLabel = FText::Format(LOCTEXT("BringResource", "Bring ships loaded with {0} {1} at {2}"),
									 FText::AsNumber(TargetResourceCount), Desc->Name, TargetSector->GetSectorName());
	}
}

int32 UFlareQuestBringResource::GetResourceCount()
{
	int ResourceCount = 0;

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(TargetResourceIdentifier);
	if(!Resource)
	{
		return 0;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	for(UFlareSimulatedSpacecraft* Ship: TargetSector->GetSectorShips())
	{
		if(Ship->GetCompany() != PlayerCompany)
		{
			continue;
		}

		ResourceCount += Ship->GetActiveCargoBay()->GetResourceQuantity(Resource, PlayerCompany);
	}

	return ResourceCount;
}

bool UFlareQuestBringResource::IsCompleted()
{
	return GetResourceCount() >= TargetResourceCount;
}

void UFlareQuestBringResource::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = TargetResourceCount;
	ObjectiveCondition.MaxProgress = TargetResourceCount;
	ObjectiveCondition.Counter = GetResourceCount();
	ObjectiveCondition.Progress = GetResourceCount();
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	ObjectiveData->TargetSectors.Add(TargetSector);
}



/*----------------------------------------------------
	Max army in company condition
----------------------------------------------------*/
UFlareCompanyMaxCombatValue::UFlareCompanyMaxCombatValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareCompanyMaxCombatValue* UFlareCompanyMaxCombatValue::Create(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	UFlareCompanyMaxCombatValue* Condition = NewObject<UFlareCompanyMaxCombatValue>(ParentQuest, UFlareCompanyMaxCombatValue::StaticClass());
	Condition->Load(ParentQuest, TargetCompanyParam, TargetArmyPointsParam);
	return Condition;
}

void UFlareCompanyMaxCombatValue::Load(UFlareQuest* ParentQuest, UFlareCompany* TargetCompanyParam, int32 TargetArmyPointsParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetCompany = TargetCompanyParam;
	TargetArmyPoints = TargetArmyPointsParam;


	FText InitialLabelText = LOCTEXT("CompanyMaxArmyCombatPoints", "{0} must have at most {1} combat value");
	InitialLabel = FText::Format(InitialLabelText, TargetCompany->GetCompanyName(), FText::AsNumber(TargetArmyPoints));
}

bool UFlareCompanyMaxCombatValue::IsCompleted()
{
	return TargetCompany->GetCompanyValue().ArmyCurrentCombatPoints <= TargetArmyPoints;
}

void UFlareCompanyMaxCombatValue::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = TargetCompany->GetCompanyValue().ArmyCurrentCombatPoints;
	ObjectiveCondition.MaxCounter = TargetArmyPoints;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


/*----------------------------------------------------
Min station count in sector condition
----------------------------------------------------*/
UFlareQuestMinSectorStationCount::UFlareQuestMinSectorStationCount(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestMinSectorStationCount* UFlareQuestMinSectorStationCount::Create(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, int32 StationCount)
{
	UFlareQuestMinSectorStationCount* Condition = NewObject<UFlareQuestMinSectorStationCount>(ParentQuest, UFlareQuestMinSectorStationCount::StaticClass());
	Condition->Load(ParentQuest, Sector, StationCount);
	return Condition;
}

void UFlareQuestMinSectorStationCount::Load(UFlareQuest* ParentQuest, UFlareSimulatedSector* Sector, int32 StationCount)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetStationCount = StationCount;
	TargetSector = Sector;

	InitialLabel = FText::Format(LOCTEXT("HaveStationCountInSector", "It must be at least {0} station in {1}"),
								 FText::AsNumber(StationCount),
								 TargetSector->GetSectorName());
}


bool UFlareQuestMinSectorStationCount::IsCompleted()
{
	return TargetSector->GetSectorStations().Num() >= TargetStationCount;
}

void UFlareQuestMinSectorStationCount::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = TargetStationCount;
	ObjectiveCondition.MaxProgress = TargetStationCount;
	ObjectiveCondition.Counter = TargetSector->GetSectorStations().Num();
	ObjectiveCondition.Progress = TargetSector->GetSectorStations().Num();
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


/*----------------------------------------------------
	Wait condition
----------------------------------------------------*/

UFlareQuestConditionWait::UFlareQuestConditionWait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionWait* UFlareQuestConditionWait::Create(UFlareQuest* ParentQuest, FName ConditionIdentifier, int32 Duration)
{
	UFlareQuestConditionWait*Condition = NewObject<UFlareQuestConditionWait>(ParentQuest, UFlareQuestConditionWait::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifier, Duration);
	return Condition;
}

void UFlareQuestConditionWait::Load(UFlareQuest* ParentQuest, FName ConditionIdentifier, int32 Duration)
{
	if (ConditionIdentifier == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionWait need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifier);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetDuration = Duration;
}


FText UFlareQuestConditionWait::GetInitialLabel()
{
	int64 DateLimit = StartDate + TargetDuration;

	int64 RemainingDuration = DateLimit - GetGame()->GetGameWorld()->GetDate();

	return FText::Format(LOCTEXT("RemainingDurationWaitFormat", "{0} remaining"), UFlareGameTools::FormatDate(RemainingDuration, 2));
}


void UFlareQuestConditionWait::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(HISTORY_START_DATE_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		IsInit = true;
		StartDate = Bundle->GetInt32(HISTORY_START_DATE_TAG);
	}
	else
	{
		IsInit = false;
	}

}

void UFlareQuestConditionWait::Save(FFlareBundle* Bundle)
{
	if (IsInit)
	{
		Bundle->PutInt32(HISTORY_START_DATE_TAG, StartDate);
	}
}

void UFlareQuestConditionWait::Init()
{
	if(!IsInit)
	{
		StartDate = GetGame()->GetGameWorld()->GetDate();
		IsInit = true;
	}
}

bool UFlareQuestConditionWait::IsCompleted()
{

	Init();
	int64 DateLimit = StartDate + TargetDuration;
	int64 RemainingDuration = DateLimit - GetGame()->GetGameWorld()->GetDate();


	return RemainingDuration <= 0;
}

void UFlareQuestConditionWait::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	Init();

	int64 CurrentProgression = GetGame()->GetGameWorld()->GetDate() - StartDate;

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = TargetDuration;
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = TargetDuration;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


#undef LOCTEXT_NAMESPACE
