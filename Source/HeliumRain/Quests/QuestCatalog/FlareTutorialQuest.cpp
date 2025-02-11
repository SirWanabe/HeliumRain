#include "FlareTutorialQuest.h"

#include "Flare.h"
#include "../../Game/FlareGame.h"
#include "../FlareQuestCondition.h"
#include "../FlareQuestAction.h"
#include "../FlareQuestStep.h"
#include "../FlareQuest.h"
#include "../../Game/FlareSaveGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../../Spacecrafts/FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareTutorialQuest"

static FName TUTORIAL_CURRENT_PROGRESSION_TAG("current-progression");
static FName TUTORIAL_LAST_TARGET_CHANGE_TS("last-target-change-ts");


/*----------------------------------------------------
	Tutorial Flying
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialFlying"
UFlareQuestTutorialFlying::UFlareQuestTutorialFlying(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialFlying::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialFlying* Quest = NewObject<UFlareQuestTutorialFlying>(Parent, UFlareQuestTutorialFlying::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialFlying::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-flying";
	QuestName = LOCTEXT("TutorialFlyingName","Spaceflight tutorial");
	QuestDescription = LOCTEXT("TutorialFlyingDescription","Let's learn how to fly this thing !");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	// Common condition
	UFlareQuestCondition* FlyShip = UFlareQuestConditionFlyingShipClass::Create(this, NAME_None);

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GoForward"
		FText Description = LOCTEXT("GoForwardDescription","Spaceships feature small maneuvering engines called the RCS, and large orbital engines for accelerating. To move forward, press <input-axis:NormalThrustInput,1.0> slightly. You can modify the key binding in the settings menu (<input-action:SettingsMenu>).");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "go-forward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 30));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GoBackward"
		FText Description = LOCTEXT("GoBackwardDescription","There is no air to brake in space. Your ship will keep its velocity and direction if you don't brake with your RCS. Press <input-axis:NormalThrustInput,-1.0> to brake.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "go-backward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxCollinearVelocity::Create(this, QUEST_TAG"cond1", -20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PitchUp"
		FText Description = LOCTEXT("PitchUpDescription","Move your cursor to the top of the screen to pitch up.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pitch-up", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,1,0) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"PitchDown"
		FText Description = LOCTEXT("PitchDownDescription","Move your cursor to the bottom of the screen to pitch down.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "pitch-down", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,1,0) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"YawLeft"
		FText Description = LOCTEXT("YawLeftDescription","Move your cursor to the left of the screen to turn left.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "yaw-left", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(0,0,1) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"YawRight"
		FText Description = LOCTEXT("YawRightDescription","Move your cursor to the right of the screen to turn right.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "yaw-right", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(0,0,1) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"RollLeft"
		FText Description = LOCTEXT("RollLeftDescription","Roll your spacescraft left with <input-axis:NormalRollInput,-1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "roll-left", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinRotationVelocity::Create(this, FVector(1,0,0) , 35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"RollRight"
		FText Description = LOCTEXT("RollRightDescription","Roll your spacescraft right with <input-axis:NormalRollInput,1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "roll-right", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxRotationVelocity::Create(this, FVector(1,0,0) , -35));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Up"
		FText Description = LOCTEXT("UpDescription","Your spacecraft can move along any axis - up, down, left, right... Try moving up with <input-axis:MoveVerticalInput,1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "up", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinVerticalVelocity::Create(this, QUEST_TAG"cond1", 20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Down"
		FText Description = LOCTEXT("DownDescription","Move your spacecraft down with <input-axis:MoveVerticalInput,-1.0>.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "down", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMaxVerticalVelocity::Create(this, QUEST_TAG"cond1", -20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Forward"
		FText Description = LOCTEXT("ForwardDescription","Almost there ! Move your ship forward again before we try some real flying.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "forward", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionMinCollinearVelocity::Create(this, QUEST_TAG"cond1", 20));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FollowAdvancedPath"
		FText Description = LOCTEXT("FollowAdvancedPathDescription","Keep a constant velocity and aim for your target, your ship's engine controller will automatically align your velocity with your ship's orientation.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "follow-advanced-path", Description);


		// TODO force first light


		Cast<UFlareQuestConditionGroup>(Step->GetEnableCondition())->AddChildCondition(FlyShip);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionFollowRandomWaypoints::Create(this, QUEST_TAG"cond1", false));
		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial Navigation
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialNavigation"
UFlareQuestTutorialNavigation::UFlareQuestTutorialNavigation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialNavigation::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialNavigation* Quest = NewObject<UFlareQuestTutorialNavigation>(Parent, UFlareQuestTutorialNavigation::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

static float InverseRamp(float Input, float Target, float StepInput, float StepOutput)
{
	float B = Target * (1-StepOutput) / (StepOutput * (StepInput - Target));
	float A = Target * (1- B);

	return FMath::Clamp(Target / (A + B * Input), 0.f, 1.f);
}

void UFlareQuestTutorialNavigation::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-navigation";
	QuestName = LOCTEXT("TutorialNavigationName","Orbital navigation tutorial");
	QuestDescription = LOCTEXT("TutorialNavigationDescription","Learn how to travel from one sector to another.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-flying"));

	FName PickUpShipId = "dock-at-ship-id";
	UFlareSimulatedSector* Sector = FindSector("the-depths");
	UFlareSimulatedSector* FirstLightSector = FindSector("first-light");
	if (!Sector)
	{
		return;
	}

	UFlareSimulatedSpacecraft* StationA = Sector->GetSectorStations().Num() > 0 ? Sector->GetSectorStations()[0] : NULL;
	UFlareSimulatedSpacecraft* StationB= Sector->GetSectorStations().Num() > 1 ? Sector->GetSectorStations()[1] : NULL;
	UFlareSimulatedSpacecraft* StationC = Sector->GetSectorStations().Num() > 2 ? Sector->GetSectorStations()[2] : NULL;

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OpenTravelMenu"
		FText Description = LOCTEXT("TravelWheelDescription", "You need to learn how to travel between sectors. To initiate a travel with your personal fleet open the wheel menu with (<input-action:Wheel>). Highlight \"Travel\" and use (<input-action:StartFire>) to select. Drill further into the travel menu to initiate a travel to \"The Depths\".");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "initiate-travel", Description);
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
			[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if (Bundle.HasTag("travel-begin") && Bundle.GetName("fleet") == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetIdentifier())
			{
				return true;
			}
			return false;
		},
			[]()
		{
			return LOCTEXT("TravelBegin", "Initiate Travel to \"The Depths\"");
		},
			[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));


		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG"WheelTravel"
		FText Description = LOCTEXT("WheelTravelDescription", "Travel can take multiple days. To view the current progress of your travel look at the top screen element. To progress a day press (<input-action:Simulate>) until you arrive at \"The Depths\"");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "wheeltravel", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorVisited::Create(this, Sector));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"Activate"
		FText Description = LOCTEXT("ActivateDescription","Your ship arrived at destination !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "activate", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorActive::Create(this, Sector));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TargetA"
		FText Description = FText::Format(LOCTEXT("TargetADescription", "You can use the targeting system to interact with objects. Use (<input-action:NextTarget>) and (<input-action:PreviousTarget>) to select {0}."),
			UFlareGameTools::DisplaySpacecraftName(StationA));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-station-a", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTargetSpacecraft::Create(this, QUEST_TAG"cond1", StationA, 3.0));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TargetB"
		FText Description = FText::Format(LOCTEXT("TargetBDescription", "Use (<input-action:NextTarget>) and (<input-action:PreviousTarget>) to select {0}. The targets are sorted by distance to your nose."),
			UFlareGameTools::DisplaySpacecraftName(StationB));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-station-b", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTargetSpacecraft::Create(this, QUEST_TAG"cond1", StationB, 3.0));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TargetC"
		FText Description = FText::Format(LOCTEXT("TargetCDescription", "Use (<input-action:NextTarget>) and (<input-action:PreviousTarget>) to select {0}."),
			UFlareGameTools::DisplaySpacecraftName(StationC));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-station-C", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTargetSpacecraft::Create(this, QUEST_TAG"cond1", StationC, 3.0));
		Steps.Add(Step);
	}

	// Manual Docking
	{
		int const Distance = 300;
		int const Speed = 2;

		FText Description = FText::Format(LOCTEXT("Approach1", "Approach and stop at less than {0}m away from the station."), FText::AsNumber(Distance));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "approach-1", Description);

		UFlareQuestConditionTutorialGenericStateCondition* DistanceCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, Distance](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();
			if (PlayerShip && StationC->IsActive())
			{
				bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
				if (AlreadyDocked)
				{
					return true;
				}

				FVector PlayerLocation = PlayerShip->GetActorLocation();
				FVector StationLocation = StationC->GetActive()->GetActorLocation();

				if ((PlayerLocation - StationLocation).SizeSquared() < (Distance * Distance * 10000))
				{
					return true;
				}
			}

			return false;
		},
		[StationC, Distance]()
		{
			return FText::Format(LOCTEXT("Approache1Label", "Approach at less than {0}m away from {1}"),FText::AsNumber(Distance), UFlareGameTools::DisplaySpacecraftName(StationC));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		DistanceCondition->AddConditionObjectivesFunc = [this, StationC, DistanceCondition, Distance](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = DistanceCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip && StationC->IsActive())
			{
				bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
				if (AlreadyDocked)
				{
					ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
				}
				else
				{
					FVector PlayerLocation = PlayerShip->GetActorLocation();
					FVector StationLocation = StationC->GetActive()->GetActorLocation();

					float CurrentDistance = (PlayerLocation - StationLocation).Size();

					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("Approach1DistanceLabel", "{0}m"), FText::AsNumber(FMath::RoundToInt(CurrentDistance / 100)));
					ObjectiveCondition.Progress = InverseRamp(CurrentDistance, 100 * Distance, 100000, 0.5);
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(DistanceCondition);


		UFlareQuestConditionTutorialGenericStateCondition* SpeedCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, Speed](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();
			if (PlayerShip)
			{
				bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
				if (AlreadyDocked)
				{
					return true;
				}

				FVector PlayerVelocity = PlayerShip->GetLinearVelocity();
				FVector StationLocation = StationC->GetActive()->GetActorLocation();

				if (PlayerVelocity.SizeSquared() < Speed * Speed)
				{
					return true;
				}
			}

			return false;
		},
		[Speed]()
		{
			return FText::Format(LOCTEXT("Stop1Label", "Stop (< {0} m/s)"), FText::AsNumber(Speed));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		SpeedCondition->AddConditionObjectivesFunc = [this, StationC, SpeedCondition, Speed](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = SpeedCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip)
			{
				bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
				if (AlreadyDocked)
				{
					ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
				}
				else
				{
					float CurrentVelocity = PlayerShip->GetLinearVelocity().Size();

					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("Approach1SpeedLabel", "{0} m/s"), FText::AsNumber(FMath::RoundToInt(CurrentVelocity)));
					ObjectiveCondition.Progress = InverseRamp(CurrentVelocity, Speed, 50, 0.5);
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(SpeedCondition);

		Steps.Add(Step);
	}

	{
		int const Distance = 75;
		int const Speed = 2;

		FText Description = FText::Format(LOCTEXT("FindDockingPortDescription", "Stations have multiple docking ports for light and heavy ships. Turn around the station to find an available docking port - your docking computer will automatically start.\nApproach a docking port at less than {0}m with your docking computer and stop."), FText::AsNumber(Distance));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "find-docking port", Description);


		UFlareQuestConditionTutorialGenericStateCondition* DistanceCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, Distance](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			if (!PlayerShip || !StationC->IsActive())
			{
				return false;
			}

			bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
			if (AlreadyDocked)
			{
				return true;
			}

			FText DockInfo;
			AFlareSpacecraft* DockSpacecraft = NULL;
			FFlareDockingParameters DockParameters;
			bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

			if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
			{
				return DockParameters.DockToDockDistance < Distance * 100;
			}

			return false;
		},
		[StationC, Distance]()
		{
			return FText::Format(LOCTEXT("Approache2Label", "Approach a docking port of {1} at less than {0}m"),FText::AsNumber(Distance), UFlareGameTools::DisplaySpacecraftName(StationC));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		DistanceCondition->AddConditionObjectivesFunc = [this, StationC, DistanceCondition, Distance](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = DistanceCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip && StationC->IsActive())
			{
				FVector PlayerLocation = PlayerShip->GetActorLocation();
				AFlareSpacecraft* DockSpacecraft = NULL;

				FText DockInfo;
				FFlareDockingParameters DockParameters;
				bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

				if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
				{
					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("Approach2DistanceLabel", "{0}m"), FText::AsNumber(FMath::RoundToInt(DockParameters.DockToDockDistance /100)));
					ObjectiveCondition.Progress = InverseRamp(DockParameters.DockToDockDistance, 100 * Distance, 10000, 0.5);
				}
				else
				{
					bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
					if (AlreadyDocked)
					{
						ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
					}
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(DistanceCondition);

		UFlareQuestConditionTutorialGenericStateCondition* SpeedCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, Speed](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();
			if (PlayerShip)
			{
				bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
				if (AlreadyDocked)
				{
					return true;
				}

				FVector PlayerVelocity = PlayerShip->GetLinearVelocity();
				FVector StationLocation = StationC->GetActive()->GetActorLocation();

				if (PlayerVelocity.SizeSquared() < Speed * Speed)
				{
					return true;
				}
			}

			return false;
		},
		[Speed]()
		{
			return FText::Format(LOCTEXT("Stop2Label", "Stop (< {0} m/s)"), FText::AsNumber(Speed));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		SpeedCondition->AddConditionObjectivesFunc = [this, StationC, SpeedCondition, Speed](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = SpeedCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip)
			{
				bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
				if (AlreadyDocked)
				{
					ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
				}
				else
				{
					float CurrentVelocity = PlayerShip->GetLinearVelocity().Size();

					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("Approach2SpeedLabel", "{0} m/s"), FText::AsNumber(FMath::RoundToInt(CurrentVelocity)));
					ObjectiveCondition.Progress = InverseRamp(CurrentVelocity, Speed, 50, 0.5);
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(SpeedCondition);


		Steps.Add(Step);
	}

	{
		int const LateralError = 5;


		FText Description = FText::Format(LOCTEXT("ReduceLateralErrorDescription", "Bring your ship in front of the docking port and reduce the lateral error displayed on your docking computer to less than {0}m.\nThe easiest way is to aim for the center of the small circle in the HUD and translate with <input-axis:MoveVerticalInput,1.0>, <input-axis:MoveVerticalInput,-1.0>, <input-axis:MoveHorizontalInput,1.0> and <input-axis:MoveHorizontalInput,-1.0> to bring the small circle in the HUD at the center of the bigger circle. Keep your velocity low during the manoeuver."), FText::AsNumber(LateralError));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "reduce-lateral-error", Description);


		UFlareQuestConditionTutorialGenericStateCondition* LateralErrorCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, LateralError](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			if (!PlayerShip || !StationC->IsActive())
			{
				return false;
			}

			bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
			if (AlreadyDocked)
			{
				return true;
			}

			FText DockInfo;
			AFlareSpacecraft* DockSpacecraft = NULL;
			FFlareDockingParameters DockParameters;
			bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

			if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
			{
				float LinearError = FVector::VectorPlaneProject(DockParameters.DockToDockDeltaLocation, DockParameters.StationDockAxis).Size() / 100;

				return LinearError < LateralError;
			}

			return false;
		},
		[StationC, LateralError]()
		{
			return FText::Format(LOCTEXT("LateralErrorLabel", "Reduce lateral error to less than {0}m"),FText::AsNumber(LateralError));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		LateralErrorCondition->AddConditionObjectivesFunc = [this, StationC, LateralErrorCondition, LateralError](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = LateralErrorCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip && StationC->IsActive())
			{
				AFlareSpacecraft* DockSpacecraft = NULL;

				FText DockInfo;
				FFlareDockingParameters DockParameters;
				bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

				if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
				{
					float LinearError = FVector::VectorPlaneProject(DockParameters.DockToDockDeltaLocation, DockParameters.StationDockAxis).Size() / 100;

					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("LateralErrorDistanceLabel", "{0}m"), FText::AsNumber(FMath::RoundToInt(LinearError)));
					ObjectiveCondition.Progress = InverseRamp(LinearError, LateralError, 15, 0.5);
				}
				else
				{
					bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
					if (AlreadyDocked)
					{
						ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
					}
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(LateralErrorCondition);

		Steps.Add(Step);
	}


	{
		int const RollErrorTarget = 5;


		FText Description = FText::Format(LOCTEXT("ReduceRollErrorDescription", "While keeping both speed and lateral error low, correct the roll error displayed on your docking computer to less than {0}\u00B0.\nOnce roll is correct, the two lines in the HUD must be opposite. Use <input-axis:NormalRollInput,-1.0> and <input-axis:NormalRollInput,1.0> to roll."), FText::AsNumber(RollErrorTarget));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "reduce-roll-error", Description);


		UFlareQuestConditionTutorialGenericStateCondition* RollErrorCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, RollErrorTarget](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			if (!PlayerShip || !StationC->IsActive())
			{
				return false;
			}

			bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
			if (AlreadyDocked)
			{
				return true;
			}

			FText DockInfo;
			AFlareSpacecraft* DockSpacecraft = NULL;
			FFlareDockingParameters DockParameters;
			bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

			if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
			{
				// Roll error
				FVector StationTopAxis = DockParameters.StationDockTopAxis;
				FVector TopAxis = PlayerShip->GetActorRotation().RotateVector(FVector(0, 0, 1));
				FVector LocalTopAxis = FVector::VectorPlaneProject(TopAxis, DockParameters.StationDockAxis).GetSafeNormal();
				float RollDot = FVector::DotProduct(StationTopAxis, LocalTopAxis);
				float RollError = FMath::RadiansToDegrees(FMath::Acos(RollDot));

				return RollError < RollErrorTarget;
			}

			return false;
		},
		[StationC, RollErrorTarget]()
		{
			return FText::Format(LOCTEXT("RollErrorLabel", "Reduce roll error to less than {0}\u00B0"), FText::AsNumber(RollErrorTarget));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		RollErrorCondition->AddConditionObjectivesFunc = [this, StationC, RollErrorCondition, RollErrorTarget](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = RollErrorCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip && StationC->IsActive())
			{
				AFlareSpacecraft* DockSpacecraft = NULL;

				FText DockInfo;
				FFlareDockingParameters DockParameters;
				bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

				if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
				{
					// Roll error
					FVector StationTopAxis = DockParameters.StationDockTopAxis;
					FVector TopAxis = PlayerShip->GetActorRotation().RotateVector(FVector(0, 0, 1));
					FVector LocalTopAxis = FVector::VectorPlaneProject(TopAxis, DockParameters.StationDockAxis).GetSafeNormal();
					float RollDot = FVector::DotProduct(StationTopAxis, LocalTopAxis);
					float RollError = FMath::RadiansToDegrees(FMath::Acos(RollDot));

					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("RollErrorDistanceLabel", "{0}\u00B0"), FText::AsNumber(FMath::RoundToInt(RollError)));
					ObjectiveCondition.Progress = InverseRamp(RollErrorTarget, RollError, 20, 0.5);
				}
				else
				{
					bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
					if (AlreadyDocked)
					{
						ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
					}
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(RollErrorCondition);

		Steps.Add(Step);
	}


	{
		int const AlignmentErrorTarget = 3;


		FText Description = FText::Format(LOCTEXT("ReduceAlignementErrorDescription", "Then correct your alignment error to less than {0}\u00B0.\nAim for the center of the small circle in the HUD when the roll and lateral error are low to correct your alignment."), FText::AsNumber(AlignmentErrorTarget));
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "reduce-alignment-error", Description);


		UFlareQuestConditionTutorialGenericStateCondition* AlignmentErrorCondition = UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this, StationC, AlignmentErrorTarget](UFlareQuestCondition* Condition)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			if (!PlayerShip || !StationC->IsActive())
			{
				return false;
			}

			bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
			if (AlreadyDocked)
			{
				return true;
			}

			FText DockInfo;
			AFlareSpacecraft* DockSpacecraft = NULL;
			FFlareDockingParameters DockParameters;
			bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

			if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
			{
				float AngularDot = FVector::DotProduct(DockParameters.ShipDockAxis.GetSafeNormal(), -DockParameters.StationDockAxis.GetSafeNormal());
				float AngularError = FMath::RadiansToDegrees(FMath::Acos(AngularDot));

				return AngularError < AlignmentErrorTarget;
			}

			return false;
		},
		[StationC, AlignmentErrorTarget]()
		{
			return FText::Format(LOCTEXT("AngularErrorLabel", "Reduce alignment error to less than {0}\u00B0"), FText::AsNumber(AlignmentErrorTarget));
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		});

		AlignmentErrorCondition->AddConditionObjectivesFunc = [this, StationC, AlignmentErrorCondition, AlignmentErrorTarget](FFlarePlayerObjectiveData* ObjectiveData)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			FFlarePlayerObjectiveCondition ObjectiveCondition;
			ObjectiveCondition.InitialLabel = AlignmentErrorCondition->GetInitialLabel();
			ObjectiveCondition.TerminalLabel = FText();
			ObjectiveCondition.Progress = 0;
			ObjectiveCondition.MaxProgress = 1;
			ObjectiveCondition.Counter = 0;
			ObjectiveCondition.MaxCounter = 0;

			if (PlayerShip && StationC->IsActive())
			{
				AFlareSpacecraft* DockSpacecraft = NULL;

				FText DockInfo;
				FFlareDockingParameters DockParameters;
				bool DockingInProgress = PlayerShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);

				if (DockSpacecraft && DockingInProgress && DockSpacecraft == StationC->GetActive())
				{
					float AngularDot = FVector::DotProduct(DockParameters.ShipDockAxis.GetSafeNormal(), -DockParameters.StationDockAxis.GetSafeNormal());
					float AngularError = FMath::RadiansToDegrees(FMath::Acos(AngularDot));


					ObjectiveCondition.TerminalLabel = FText::Format(LOCTEXT("AngularErrorDistanceLabel", "{0}\u00B0"), FText::AsNumber(FMath::RoundToInt(AngularError)));
					ObjectiveCondition.Progress = InverseRamp(AlignmentErrorTarget, AngularError, 20, 0.5);
				}
				else
				{
					bool AlreadyDocked = PlayerShip->GetNavigationSystem()->IsDocked();
					if (AlreadyDocked)
					{
						ObjectiveCondition.Progress = ObjectiveCondition.MaxProgress;
					}
				}
			}

			ObjectiveData->ConditionList.Add(ObjectiveCondition);
			ObjectiveData->AddTargetSpacecraft(StationC);
		};

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(AlignmentErrorCondition);

		Steps.Add(Step);
	}

{
		FText Description = LOCTEXT("NavigationDockAtDescription", "Once all the docking parameters are good, slowly speed up to reduce the distance, keeping all the error values as low as you can. Once you're 3 meters away, the magnetic grab will snap your ship to the docking port. For your first docking, try staying below 5m/s and don't hesitate to stop to correct your alignment.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "dock-at", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, StationC);
		Condition->TargetShipSaveId = PickUpShipId;

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("CommandUndockDescription", "Congratulations, you are docked ! If you want, you can make docking easier by unlocking autodocking in the technology menu.\nOpen the wheel menu with (<input-action:Wheel>) to select the undocking option.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "command-undock", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
		[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			AFlareSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetShipPawn();

			if (PlayerShip && Bundle.HasTag("undock") && Bundle.GetName("target") == PlayerShip->GetImmatriculation())
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("CommandUndockLabel", "Undock");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}


	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG"OpenTravelMenuBackToFirstLight"
		FText Description = LOCTEXT("OpenTravelMenuBackToFirstLightDescription", "There are other methods of initiating travel besides just the wheel menu. Open the menu bar with <input - action:ToggleOverlay> and click on the orbital map.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-travel-menu", Description);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Orbit));
		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG"Travel"
		FText Description = LOCTEXT("TravelFleetPanelDescription","On the left side of the screen Select \"Trade Fleet 1\" and then select \"First Light\". Use the \"Fast Forward\" button to wait for the end of the travel.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorVisited::Create(this, FirstLightSector));
		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG "Activate"
		FText Description = LOCTEXT("ActivateDescription", "Your ship arrived at destination !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "activate", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorActive::Create(this, FirstLightSector));
		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG "Travel"
		FText Description = LOCTEXT("TravelDescription", "Select the sector \"The Depths\" and click \"Travel\". Then, on the orbital map, use the \"Fast Forward\" button to wait for the end of the travel.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorVisited::Create(this, Sector));
		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG "Activate"
		FText Description = LOCTEXT("ActivateDescription", "Your ship arrived at destination !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "activate", Description);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionSectorActive::Create(this, Sector));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial Contracts
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialContracts"
UFlareQuestTutorialContracts::UFlareQuestTutorialContracts(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialContracts::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialContracts* Quest = NewObject<UFlareQuestTutorialContracts>(Parent, UFlareQuestTutorialContracts::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialContracts::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-contracts";
	QuestName = LOCTEXT("TutorialContractsName","Contracts tutorial");
	QuestDescription = LOCTEXT("TutorialContractsDescription","Learn how to complete contracts for other companies.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-navigation"));


	UFlareSimulatedSector* Sector = FindSector("blue-heart");

	if (!Sector)
	{
		return;
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FindContract"
		FText Description = LOCTEXT("FindContractDescription","Other companies in this system can give you contracts to carry out. You can ignore them, but they are a very good way to earn money or research, or to discover new sectors. Keep traveling between sectors to find contracts.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "find-contract", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGetContrat::Create(this));
		Steps.Add(Step);

		Sector = FindSector("blue-heart");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Sector = FindSector("the-spire");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Sector = FindSector("outpost");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Sector = FindSector("miners-home");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Sector = FindSector("nights-home");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Sector = FindSector("farm");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		Sector = FindSector("lighthouse");
		Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));

		if (!Parent->IsPlayStory())
		{
			Sector = FindSector("pendulum");
			Step->GetInitActions().Add(UFlareQuestActionDiscoverSector::Create(this, Sector, false));
		}
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OpenQuestMenu"
		FText Description = LOCTEXT("OpenQuestMenuDescription","A company has made a new contract available to you.\nOpen the contracts menu with (<input-action:QuestMenu>), or from the menu bar with (<input-action:ToggleOverlay>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-quest-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Quest));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"AcceptQuest"
		FText Description = LOCTEXT("AcceptQuestDescription","You can manage contracts here. You have only a few days to accept a contract, but once you do, you can usually take your time to finish it. Accept your first contract now !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "accept-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialAcceptQuest::Create(this));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"TrackQuest"
		FText Description = LOCTEXT("TrackQuestDescription","Tracking a contract makes its instructions visible permanently. Track a contract now - you can return to this menu to track the tutorial contract at any time if you are lost.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "track-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTrackQuest::Create(this));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FinishQuest"
		FText Description = LOCTEXT("FinishQuestDescription","Complete five contracts to start filling your company's coffers.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "complete-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialFinishQuest::Create(this, QUEST_TAG"cond1", 5));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial technology
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialTechnology"
UFlareQuestTutorialTechnology::UFlareQuestTutorialTechnology(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialTechnology::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialTechnology* Quest = NewObject<UFlareQuestTutorialTechnology>(Parent, UFlareQuestTutorialTechnology::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialTechnology::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-technology";
	QuestName = LOCTEXT("TutorialTechnologyName","Technology tutorial");
	QuestDescription = LOCTEXT("TutorialTechnologyDescription","Learn how to use technologies.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));

	int32 AutoPilotResearchCost = GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetTechnologyCostFromID("auto-docking");
	if (!AutoPilotResearchCost)
	{
		AutoPilotResearchCost = 25;
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GainResearchPoint"
		FText Description = LOCTEXT("GainResearchPointDescription","You can develop technologies to increase your capabilities and your company's performance. To develop a technology, you need research points. Companies with a high research value sometimes offer you contracts with research points as a reward. You will learn a more efficient way to get research points in the future !\nGain some research points to move forward.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "gain-research-points", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialResearchValue::Create(this, AutoPilotResearchCost));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OpenTechnologyMenu"
		FText Description = LOCTEXT("OpenTechnologyMenuDescription","You can research new technologies with the technology menu. Open it with (<input-action:TechnologyMenu>), or from the menu bar with (<input-action:ToggleOverlay>) to spend your research points.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-technology-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Technology));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchTechnology"
		FText Description = LOCTEXT("ResearchTechnologyDescription","You can manage research in the technology menu. Select a technology and research it !\nChoose wisely, for the price of all technology is increased after each research. You may consider researching the 'Science' technology to build a research station quickly.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-technology", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialResearchTechnology::Create(this, 1));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ReachLevelTechnology3"
		FText Description = LOCTEXT("ReachLevelTechnology3Description","High level technology requires an increased technology level. Your technology level is incremented every new technology.\nResearch more technologies to unlock level 3 technologies.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "track-quest", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialTechnologyLevel::Create(this, 3));
		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial build ship
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialBuildShip"
UFlareQuestTutorialBuildShip::UFlareQuestTutorialBuildShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialBuildShip::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialBuildShip* Quest = NewObject<UFlareQuestTutorialBuildShip>(Parent, UFlareQuestTutorialBuildShip::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialBuildShip::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-build-ship";
	QuestName = LOCTEXT("TutorialBuildShipName","Ship-building tutorial");
	QuestDescription = LOCTEXT("TutorialBuildShipDescription","Learn how to build ships.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	UFlareSimulatedSector* Sector = FindSector("blue-heart");

	if (!Sector)
	{
		return;
	}

	FName PickUpShipId = "dock-at-ship-id";
	UFlareSimulatedSpacecraft* Shipyard = NULL;
	for(UFlareSimulatedSpacecraft* Station: Sector->GetSectorStations())
	{
		if(Station->IsShipyard())
		{
			Shipyard = Station;
			break;
		}
	}

	if (!Shipyard)
	{
		return;
	}

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"GainMoneyBuildShip"
		FText Description = LOCTEXT("GainMoneyBuildShipDescription","You can buy ships to increase your trading force or to defend your estate. Ships are expensive, save some money first !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "gain-money", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialMoney::Create(this, 3000000));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"DockAt"
		FText Description = LOCTEXT("BuildShipDockAtDescription", "You can order ships at shipyards. Dock your ship at the Blue Heart shipyard.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "dock", Description);

		UFlareQuestConditionDockAt* Condition = UFlareQuestConditionDockAt::Create(this, Shipyard);
		Condition->TargetShipSaveId = PickUpShipId;

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(Condition);

		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"OrderShip"
		FText Description = LOCTEXT("OrderShipDescription","Open the shipyard details, either with the wheel menu or through the sector menu. You will have the details of production lines.\nPick a production line for small ships and order a freighter of your choice.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "order-ship", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOrderFreigther::Create(this, EFlarePartSize::S));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuildShip"
		FText Description = LOCTEXT("BuildShipDescription","Ship construction may be long, and may require additional resources to start if they are not already in stock at the shipyard. Make sure production has started and continue playing until your ship is ready.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "build-ship", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialBuildShip::Create(this, EFlarePartSize::S));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial build station
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialBuildStation"
UFlareQuestTutorialBuildStation::UFlareQuestTutorialBuildStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialBuildStation::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialBuildStation* Quest = NewObject<UFlareQuestTutorialBuildStation>(Parent, UFlareQuestTutorialBuildStation::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialBuildStation::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-build-station";
	QuestName = LOCTEXT("TutorialBuildStationName","Station-building tutorial");
	QuestDescription = LOCTEXT("TutorialBuildStationDescription","Learn how to build stations.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	int32 AutoPilotResearchCost = GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetTechnologyCostFromID("auto-docking");
	if (!AutoPilotResearchCost)
	{
		AutoPilotResearchCost = 25;
	}

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionTutorialResearchValue::Create(this, AutoPilotResearchCost));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-build-ship"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchStationTechnology"
		FText Description = LOCTEXT("BuildStationResearchStationTechnologyDescription","Stations are important to gain long-term economic power. Different stations have different technological requirements. Research a technology that unlocks contruction of stations.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-station-technology", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockStation::Create(this));
		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG"GainMoneyBuildStation"
		FText Description = LOCTEXT("GainMoneyBuildStationDescription", "Stations and their required licensing is expensive, save some money first !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "gain-money", Description);
		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialMoney::Create(this, 10000000));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"SectorMenu"
		FText Description = LOCTEXT("SectorMenuDescription", "You have now unlocked some station-building capabilities. You can build stations in any sector, but there are various limitations depending on the kind of station you want to build, and the sector itself. You can't build solar stations in clouds of dust !\nOpen the current sector menu (<input-action:SectorMenu>), or another known sector from the orbital menu.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "dock", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Sector));
		Steps.Add(Step);
	}

	{
#undef QUEST_STEP_TAG
#define QUEST_STEP_TAG QUEST_TAG"BuyStationLicense"
		FText Description = LOCTEXT("BuyStationLicenseDescription", "Before being allowed to build a station in a sector you must purchase its station license. The cost of station licenses can vary based on various factors.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "buy-station-license", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
			[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if (Bundle.HasTag("station-license-bought"))
			{
				return true;
			}
			return false;
		},
			[]()
		{
			return LOCTEXT("BuyStationLicense", "Buy a station building license");
		},
			[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));


		Steps.Add(Step);
	}
	
	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"StartConstuction"
		FText Description = LOCTEXT("StartConstuctionDescription", "Open the station-building menu using the button on the bottom left part of the sector menu to build a station. Starting a station construction will cost you fees then you will have to bring construction resources.");

		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "order-station", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialStartStationConstruction::Create(this, false));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuildStation"
		FText Description = LOCTEXT("BuildStationDescription","Your station is now under construction. Bring the missing construction resources to finish the construction ! Other companies can sell resources directly to the station too.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "build-station", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialHaveStation::Create(this, false));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"StartUpgrade"
		FText Description = LOCTEXT("StartUpgradeDescription","Your station is now ready, you can inspect it to see its production needs. You can upgrade stations to increase their productivity and cargo bay. Upgrading a station will shut down production until you bring the missing resources.\nUpgrade a station using the upgrade button in the station details");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "start-upgrade", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialStartStationConstruction::Create(this, true));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"FinishUpgrade"
		FText Description = LOCTEXT("FinishUpgradeDescription","Bring missing construction resources to finish the upgrade.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "finish-upgrade", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialHaveStation::Create(this, true));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial research station
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialResearchStation"
UFlareQuestTutorialResearchStation::UFlareQuestTutorialResearchStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialResearchStation::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialResearchStation* Quest = NewObject<UFlareQuestTutorialResearchStation>(Parent, UFlareQuestTutorialResearchStation::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialResearchStation::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-research-station";
	QuestName = LOCTEXT("TutorialResearchStationName","Research station tutorial");
	QuestDescription = LOCTEXT("TutorialResearchStationDescription","Learn how to build and use research stations.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-technology"));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-build-station"));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ResearchStationTechnology"
		FText Description = LOCTEXT("ResearchStationTechnologyDescription","The fastest way to research technology is by building research stations. First, unlock the technology to build research stations.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "research-station-technology", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialUnlockTechnology::Create(this, "science"));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"BuildReasearchStation"
		FText Description = LOCTEXT("BuildReasearchStationDescription", "You now have the technology to build research stations. Build one anywhere you'd like.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "build-research-station", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialHaveStation::Create(this, false, "station-research"));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"ProduceResearch"
		FText Description = LOCTEXT("ProduceResearchDescription","Your research station is now ready. Make sure you provide it with the input resources it needs to produce research points !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "produce-research", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialProduceResearch::Create(this, QUEST_TAG"cond1", 50));
		Steps.Add(Step);
	}
}

/*----------------------------------------------------
	Tutorial repair ship
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialRepairShip"
UFlareQuestTutorialRepairShip::UFlareQuestTutorialRepairShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialRepairShip::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialRepairShip* Quest = NewObject<UFlareQuestTutorialRepairShip>(Parent, UFlareQuestTutorialRepairShip::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialRepairShip::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-repair-ship";
	QuestName = LOCTEXT("TutorialRepairShipName","Ship repair tutorial");
	QuestDescription = LOCTEXT("TutorialRepairShipDescription","Learn how to repair a ship.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionTutorialShipNeedFs::Create(this, false));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"StartRepair"
		FText Description = LOCTEXT("StartRepairDescription","You have a damaged ship, you need to repair it. Go to the sector menu (<input-action:SectorMenu>) and click on the repair button, on the left of the menu.\nRepairing requires Fleet Supply in a station or cargo in the sector.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "start-repair", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialRepairRefill::Create(this, false, false));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"EndRepair"
		FText Description = LOCTEXT("EndRepairDescription", "Repairs can take a few days, depending on the damage. Wait for the end of the repairs.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "end-repair", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialRepairRefill::Create(this, false, true));
		Steps.Add(Step);
	}
}



/*----------------------------------------------------
	Tutorial refill ship
----------------------------------------------------*/
#undef QUEST_TAG
#define QUEST_TAG "TutorialRefillShip"
UFlareQuestTutorialRefillShip::UFlareQuestTutorialRefillShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialRefillShip::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialRefillShip* Quest = NewObject<UFlareQuestTutorialRefillShip>(Parent, UFlareQuestTutorialRefillShip::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialRefillShip::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-refill-ship";
	QuestName = LOCTEXT("TutorialRefillShipName","Ship refill tutorial");
	QuestDescription = LOCTEXT("TutorialRefillShipDescription","Learn how to refill ships.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-contracts"));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionTutorialShipNeedFs::Create(this, true));

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"StartRefill"
		FText Description = LOCTEXT("StartRefillDescription","One of your ships used ammunition and needs to refill.\nGo to the sector menu (<input-action:SectorMenu>) and click on the refill button on the left of the menu. Refilling requires Fleet Supply in a station or cargo in the same sector.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "start-refill", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialRepairRefill::Create(this, true, false));
		Steps.Add(Step);
	}

	{
		#undef QUEST_STEP_TAG
		#define QUEST_STEP_TAG QUEST_TAG"EndRefill"
		FText Description = LOCTEXT("EndRefillDescription", "The refilling can take a few days, depending on the ammo depletion. Wait for the end of the refilling.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "end-refill", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialRepairRefill::Create(this, true, true));
		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial fighter
----------------------------------------------------*/

UFlareQuestTutorialFighter::UFlareQuestTutorialFighter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialFighter::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialFighter* Quest = NewObject<UFlareQuestTutorialFighter>(Parent, UFlareQuestTutorialFighter::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialFighter::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-fighter";
	QuestName = LOCTEXT("TutorialFighterShipName","Combat tutorial");
	QuestDescription = LOCTEXT("TutorialFighterDescription","Learn how to use armed spacecraft.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-build-ship"));

	{
		FText Description = LOCTEXT("HaveMilitaryShipDescription","It's been said that you should prepare for war while you pray for peace. Have a fighter built for you at a shipyard.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "have-military", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){
			for(auto Ship: GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetCompanyShips())
			{
				if(Ship->IsMilitary() && Ship->GetSize() == EFlarePartSize::S)
				{
					return true;
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("HaveMilitaryShipConditionLabel", "Have a fighter");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("FlyMilitaryShipDescription", "Fly your new fighter. Merge the fighter fleet into your player fleet. After the merge use the fly button in the sector menu, or (<input-action:QuickSwitch>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "fly-military", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){

			UFlareSimulatedSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetPlayerShip();

			if(PlayerShip && PlayerShip->IsMilitary() && PlayerShip->GetSize() == EFlarePartSize::S)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("FlyMilitaryShipConditionLabel", "Fly a fighter");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("WeaponToggleOnDescription", "You can enable or disable weapons with the combat key (<input-action:ToggleCombat>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "toggle-on-weapon", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("toggle-combat") && Bundle.GetInt32("new-state") == 1)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("WeaponToggleOnConditionLabel", "Toggle on your weapons");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("WeaponToggleOffDescription", "To disable your weapons, press the combat key again (<input-action:ToggleCombat>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "toggle-off-weapon", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("toggle-combat") && Bundle.GetInt32("new-state") == 0)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("WeaponToggleOffConditionLabel", "Toggle off your weapons");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("ActivateWeaponDescription", "You can also activate one of your weapons directly with (<input-action:SpacecraftKey2>) and following keys. You can have multiple differents weapons in a ship.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "activate-weapon", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("activate-weapon") && Bundle.GetInt32("index") == 0)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("ActivateWeaponConditionLabel", "Activate your first weapon group");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("DeactivateWeaponDescription", "Finally, you can directly disable your weapons with (<input-action:SpacecraftKey1>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "deactivate-weapon", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("deactivate-weapon"))
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("DeactivateWeaponConditionLabel", "Deactivate your weapons");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("FireWeaponDescription", "Activate a weapon again, and try to fire with <LeftMouseButton>. Be careful not to harm anyone !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "fire-weapon", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCounterCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("fire-gun"))
			{
				return 1;
			}
			return 0;
		},
		[]()
		{
			return LOCTEXT("FireWeaponConditionLabel", "Fire 10 bullets.");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		},  "FireWeaponcond1", 10));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("FlyToAsteroidDescription", "Let's try hitting a target. Fly to a sector with asteroids.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "fly-to-asteroid", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){

			UFlareSimulatedSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetPlayerShip();

			if(PlayerShip && PlayerShip->GetCurrentSector() && PlayerShip->GetCurrentSector()->GetData()->AsteroidData.Num() > 0)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("FlyToAsteroidConditionLabel", "Fly to a sector with asteroids");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("HitAsteroidDescription", "Make sure your are less than 2km away from a big, un-mined asteroid, and try to hit it.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "hit-asteroid", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCounterCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("hit-asteroid"))
			{
				return 1;
			}
			return 0;
		},
		[]()
		{
			return LOCTEXT("HitAsteroidConditionLabel", "Hit an asteroid 20 times.");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		},  "HitAsteroidcond1", 20));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("TargetCargoDescription", "Asteroids don't move, so it's easy, but a moving targest will require you to anticipate its movement. Your onboard computer can help you.\nTarget one of your freighters and approach at less than 1km");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "target-cargo", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){

			UFlareSimulatedSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetPlayerShip();

			if(PlayerShip && PlayerShip->IsActive())
			{
				AFlareSpacecraft* Target = PlayerShip->GetActive()->GetCurrentTarget().SpacecraftTarget;

				if(Target && Target->GetCompany() == PlayerShip->GetCompany() && !Target->IsMilitary())
				{
					if((Target->GetActorLocation() - PlayerShip->GetActive()->GetActorLocation()).SizeSquared() < 100000.f * 100000.f)
					{
						return true;
					}
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("TargetCargoConditionLabel", "Target one of your freighters and approach at less than 1km.");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
		}));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("ZoomDescription", "If you look at your freighter, you will see a rhombus : the aim indicator. It appears when the target is in weapon range.\nIf the target keeps its current velocity, shooting the aim indicator will hit the center of the target.\nYou can have a better view by holding the zoom key (<input-action:CombatZoom>). It can be useful to aim precisely from a distance.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "zoom-cargo	", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("full-zoom"))
			{
				UFlareSimulatedSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetPlayerShip();

				if(PlayerShip && PlayerShip->IsActive())
				{
					AFlareSpacecraft* Target = PlayerShip->GetActive()->GetCurrentTarget().SpacecraftTarget;

					if(Target)
					{
						FVector TargetAxis = (Target->GetActorLocation() - PlayerShip->GetActive()->GetActorLocation()).GetUnsafeNormal();
						FVector NoseAxis = PlayerShip->GetActive()->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));

						if(FVector::DotProduct(TargetAxis, NoseAxis) > 0.993)
						{
							return true;
						}
					}
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("ZoomConditionLabel", "Zoom at your target");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("HitCargoDescription", "Fire a few bullets at your ship. BE CAREFUL ! You can disable or destroy cargo ships with only a few shots. Try hitting it only once !");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "hit-cargo", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("hit-ship"))
			{
				UFlareSimulatedSpacecraft* PlayerShip = GetQuestManager()->GetGame()->GetPC()->GetPlayerShip();

				if(PlayerShip && PlayerShip->IsActive())
				{
					AFlareSpacecraft* Target = PlayerShip->GetActive()->GetCurrentTarget().SpacecraftTarget;

					if(Target && Target->GetCompany() == PlayerShip->GetCompany() && !Target->IsMilitary())
					{
						if(Bundle.GetName("immatriculation") == Target->GetImmatriculation())
						{
							return true;
						}
					}
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("HitCargoConditionLabel", "Hit the target freighter");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("Have2MilitaryShipDescription","Before fighting real enemies, we will learn how to fight with multiple ships. Build a second fighter and make sure it is in your fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "have-2-military", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){

			int Count = 0;
			for(auto Ship: GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetShips())
			{
				if(Ship->IsMilitary() && Ship->GetSize() == EFlarePartSize::S)
				{
					Count++;
				}

				if(Count >=2)
				{
					return true;
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("Have2MilitaryShipConditionLabel", "Have 2 fighters in your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("MultipleQuickSwitchDescription", "When you have multiple ships in a battle, all the ships you're not piloting are still fighting autonomously for you. You can fly other ships in your fleet at any time, should yours be damaged.\nThe (<input-action:QuickSwitch>) key allows you to quickly jump to another ship. Press this key a few times.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "multiple-quick-switch", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCounterCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("quick-switch"))
			{
				return 1;
			}
			return 0;
		},
		[]()
		{
			return LOCTEXT("MultipleQuickSwitchConditionLabel", "Press quick switch 5 times.");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		},  "MultipleQuickSwitchcond1", 5));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("BattleSectorDescription", "Wait for someone to attack you, or start a battle. You can accept combat contracts using the contracts menu (<input-action:QuestMenu>) or you can declare war using the diplomatic menu (<input-action:LeaderboardMenu>).\nOnce you are at war, you can see sectors with hostile ships in red in the orbital map. Find one with enemy fighters.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "battle-sector", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){

			UFlareCompany* PlayerCompany = GetQuestManager()->GetGame()->GetPC()->GetCompany();
			if(GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet() && GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetCurrentSector()->GetSectorBattleState(PlayerCompany).InFight)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("BattleSectorConditionLabel", "Go into a fight");
		},
		[](UFlareQuestCondition* Condition)
		{
//			Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
//			Condition->Callbacks.AddUnique(EFlareQuestCallback::WAR_STATE_CHANGED);
			Condition->Callbacks.AddUnique(EFlareQuestCallback::PLAYER_BATTLESTATE_CHANGED);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("MakeFighterUncontrollableDescription", "Shoot down an enemy fighter. Ships disabled by a wingman don't count, you need the training ! Don't fire at disabled ships - they are prisoners of war.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "disable-enemy", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
		[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if (Bundle.HasTag("enemy-uncontrollable"))
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("MakeFighterUncontrollableConditionLabel", "Make one enemy fighter uncontrollable by yourself");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial split fleet
----------------------------------------------------*/

UFlareQuestTutorialSplitFleet::UFlareQuestTutorialSplitFleet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialSplitFleet::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialSplitFleet* Quest = NewObject<UFlareQuestTutorialSplitFleet>(Parent, UFlareQuestTutorialSplitFleet::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialSplitFleet::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-split-fleet";
	QuestName = LOCTEXT("TutorialSplitFleetName","Fleet management tutorial");
	QuestDescription = LOCTEXT("TutorialSplitFleetDescription","Learn how to split fleets.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(
				UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this](UFlareQuestCondition* Condition){

					if(GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetShipCount() > 1)
					{
						return true;
					}
					return false;
				},
		[](){
			return FText();
		},
		[](UFlareQuestCondition* Condition)
		{
		 Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
		 Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));


	{
		FText Description = LOCTEXT("TravelLargeFleetDescription","You have multiple ships in your fleet. All the ships in a fleet will travel together. Travel to another sector with your fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "travel-large-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("travel-end") && GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetShipCount() > 1)
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("FinishLargeFleetTravelLabel", "Finish a travel with at least 2 ships in your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("FleetMenuDescription","You can split your fleet to manage your ships separately. Open the fleet menu with (<input-action:FleetMenu>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-fleet-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Fleet));
		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("SelectPlayerFleetDescription","Select your own fleet in the left colunm.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "select-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("fleet-selected") && Bundle.GetName("fleet") == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetIdentifier())
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("SelectFleetLabel", "Select your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("EditPlayerFleetDescription","Edit your fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "edit-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("fleet-edited") && Bundle.GetName("fleet") == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetIdentifier())
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("EditFleetLabel", "Edit your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("SplitFleetDescription", "Now remove all the ships from your fleet, except the one you are flying.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "battle-sector", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){
				if(GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetShipCount() == 1)
				{
					return true;
				}
				return false;
		},
		[]()
		{
			return LOCTEXT("SplitFleetLabel", "Keep only one ship in your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}
}


/*----------------------------------------------------
	Tutorial remote fleet
----------------------------------------------------*/

UFlareQuestTutorialDistantFleet::UFlareQuestTutorialDistantFleet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialDistantFleet::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialDistantFleet* Quest = NewObject<UFlareQuestTutorialDistantFleet>(Parent, UFlareQuestTutorialDistantFleet::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialDistantFleet::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-distant-fleet";
	QuestName = LOCTEXT("TutorialDistantFleetName","Remote fleets tutorial");
	QuestDescription = LOCTEXT("TutorialDistantFleetDescription","Learn how manage remote fleets.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(
				UFlareQuestConditionTutorialGenericStateCondition::Create(this,
		[this](UFlareQuestCondition* Condition){

					if(GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetCompanyFleets().Num() > 1)
					{
						return true;
					}
					return false;
				},
		[](){
			return FText();
		},
		[](UFlareQuestCondition* Condition)
		{
		 Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
		 Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));


	{
		FText Description = LOCTEXT("CompanyMenuDescription","You have multiple fleets. You can see your ship's locations and fleet assignments in the company menu. Open the company menu with (<input-action:CompanyMenu>) and go to the Property tab.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-company-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Company));
		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("OpenSectorMenuDescription","You can also trade and travel with remote fleets. To send a remote fleet to a sector, open a sector by clicking on the sector in the orbital map.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-sector-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
				[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if (Bundle.HasTag("open-menu") && Bundle.GetInt32("menu") == (EFlareMenu::MENU_Sector+0))
			{
				FFlareMenuParameterData* MenuData = (FFlareMenuParameterData*) Bundle.GetPtr("data");
				if (MenuData && MenuData->Sector)
				{
					for (UFlareFleet* Fleet: MenuData->Sector->GetSectorFleets())
					{
						if (Fleet->GetFleetCompany() == GetQuestManager()->GetGame()->GetPC()->GetCompany())
						{
							return false;
						}
					}
					return true;
				}
			}
			return false;

		},
		[]()
		{
			return LOCTEXT("OpenDistantSectorLabel", "Open a sector with none of your fleets");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("StartDistantTravelDescription","At the left of the travel button, select a remote fleet before starting a travel.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "start-distant-travel", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,  [this](UFlareQuestCondition* Condition)
			{
				UFlareCompany* PlayerCompany = GetQuestManager()->GetGame()->GetPC()->GetCompany();

				for(UFlareFleet* Fleet : PlayerCompany->GetCompanyFleets())
				{
					if (Fleet->IsTraveling() && Fleet != GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet())
					{
						return true;
					}
				}
				return false;
			},
			[]()
			{
				return LOCTEXT("StartTravelWithDistantShip", "Start a travel with a remote fleet");
			},
			[](UFlareQuestCondition* Condition)
			{
				Condition->Callbacks.AddUnique(EFlareQuestCallback::TRAVEL_STARTED);
			}));
		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("WaitDistantTravelEndDescription", "While a remote fleet is traveling, you can still play with your personal fleet. Wait for the arrival of the remote fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "wait-distant-travel-end", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
			[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if (Bundle.HasTag("travel-end") && Bundle.GetName("fleet") != GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetIdentifier())
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("WaitDistantTravelEndLabel", "Wait for your fleet to arrive");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("OpenTradeSectorMenu", "If a remote fleet is in a sector with stations, you can trade there. Send a fleet to a sector with stations, then open the sector menu.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-trade-sector", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
			[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if (Bundle.HasTag("open-menu") && Bundle.GetInt32("menu") == (EFlareMenu::MENU_Sector+0))
			{
				FFlareMenuParameterData* MenuData = (FFlareMenuParameterData*) Bundle.GetPtr("data");

				if (MenuData)
				{
					UFlareSimulatedSector* Sector = MenuData->Sector;
					if (!Sector)
					{
						if (GetQuestManager()->GetGame()->GetActiveSector())
						{
							Sector = GetQuestManager()->GetGame()->GetActiveSector()->GetSimulatedSector();
						}
						else if (GetQuestManager()->GetGame()->GetPC()->GetPlayerShip())
						{
							Sector = GetQuestManager()->GetGame()->GetPC()->GetPlayerShip()->GetCurrentSector();
						}
					}

					if (Sector && Sector->GetSectorStations().Num() > 0)
					{
						for (UFlareFleet* Fleet: Sector->GetSectorFleets())
						{
							if (Fleet->GetFleetCompany() == GetQuestManager()->GetGame()->GetPC()->GetCompany()
							&& Fleet != GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet())
							{
								return true;
							}
						}
					}
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("OpenTradeSectorLabel", "Open the sector menu with a remote fleet near stations");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("DistantTradeDescription","If you click on a remote ship, the trade button should be available. Trade something with a station.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "distant-trade", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
			[this](UFlareQuestCondition* Condition)
		{
			for (UFlareSimulatedSpacecraft* Ship: GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetCompanyShips())
			{
				if(Ship->IsTrading())
				{
					return true;
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("DistantTradeLabel", "Trade with a remote fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::TRADE_DONE);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("WaitTradeEndDescription","Remote ships don't need to dock at stations, but trades take one day. If you look at your remote ship's status, your will see whether it is trading. Wait for the end of the transaction by waiting a day with (<input-action:Simulate>), or the skip button on the orbital map.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "trade-end", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
			[this](UFlareQuestCondition* Condition)
		{
			for (UFlareSimulatedSpacecraft* Ship: GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetCompanyShips())
			{
				if(Ship->IsTrading())
				{
					return false;
				}
			}
			return true;
		},
		[]()
		{
			return LOCTEXT("WaitTradeEndLabel", "Wait for the trade to end");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
		}));

		Steps.Add(Step);
	}

}




/*----------------------------------------------------
	Tutorial merge fleet
----------------------------------------------------*/

UFlareQuestTutorialMergeFleet::UFlareQuestTutorialMergeFleet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialMergeFleet::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialMergeFleet* Quest = NewObject<UFlareQuestTutorialMergeFleet>(Parent, UFlareQuestTutorialMergeFleet::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialMergeFleet::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-merge-fleet";
	QuestName = LOCTEXT("TutorialMergeFleetName","Fleet merging tutorial");
	QuestDescription = LOCTEXT("TutorialMergeFleetDescription","Learn how to merge different fleets.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-distant-fleet"));

	{
		FText Description = LOCTEXT("JoinFleets","You can merge fleets together. Both fleets need to be in the same sector. Move a remote fleet to your current sector or travel to a remote fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "join-fleets", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
			[this](UFlareQuestCondition* Condition)
		{
			UFlareCompany* PlayerCompany = GetQuestManager()->GetGame()->GetPC()->GetCompany();

			for(UFlareFleet* Fleet : PlayerCompany->GetCompanyFleets())
			{
				if (Fleet != GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()
				&& Fleet->GetCurrentSector() == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetCurrentSector())
				{
					return true;
				}
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("JoinFleetsLabel", "Have another fleet in your sector");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
		}));



		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("FleetMenuMergeDescription","Open the fleet menu with (<input-action:FleetMenu>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-fleet-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Fleet));
		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("SelectPlayerFleet2Description","Select your fleet in the left colunm.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "select-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
			[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("fleet-selected") && Bundle.GetName("fleet") == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetIdentifier())
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("SelectFleetLabel", "Select your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("EditPlayerFleetDescription","Edit your fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "edit-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
			[this](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("fleet-edited") && Bundle.GetName("fleet") == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetIdentifier())
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("EditFleetLabel", "Edit your fleet");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("MergeFleetDescription", "Select fleets in the left columns and merge all local fleets into your fleet.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "merge-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){
					UFlareCompany* PlayerCompany = GetQuestManager()->GetGame()->GetPC()->GetCompany();

					for(UFlareFleet* Fleet : PlayerCompany->GetCompanyFleets())
					{
						if (Fleet != GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()
						&& Fleet->GetCurrentSector() == GetQuestManager()->GetGame()->GetPC()->GetPlayerFleet()->GetCurrentSector())
						{
							return false;
						}
					}
					return true;
		},
		[]()
		{
			return LOCTEXT("MergeFleetLabel", "Merge all local fleets");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}
}



/*----------------------------------------------------
	Tutorial trade route
----------------------------------------------------*/

UFlareQuestTutorialTradeRoute::UFlareQuestTutorialTradeRoute(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuest* UFlareQuestTutorialTradeRoute::Create(UFlareQuestManager* Parent)
{
	UFlareQuestTutorialTradeRoute* Quest = NewObject<UFlareQuestTutorialTradeRoute>(Parent, UFlareQuestTutorialTradeRoute::StaticClass());
	Quest->Load(Parent);
	return Quest;
}

void UFlareQuestTutorialTradeRoute::Load(UFlareQuestManager* Parent)
{
	LoadInternal(Parent);

	Identifier = "tutorial-trade-route";
	QuestName = LOCTEXT("TutorialTradeRouteName","Trade route tutorial");
	QuestDescription = LOCTEXT("TutorialTradeRouteFleetDescription","Learn how manage trade routes.");
	QuestCategory = EFlareQuestCategory::TUTORIAL;

	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-merge-fleet"));
	Cast<UFlareQuestConditionGroup>(TriggerCondition)->AddChildCondition(UFlareQuestConditionQuestSuccessful::Create(this, "tutorial-split-fleet"));

	{
		FText Description = LOCTEXT("OpenWorldEconomyMenuDescription","Trade routes are a tool to automate trading through different sectors. Browse resources in the economy menu and search for pairs of producer and consumer sectors.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-world-economy-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_WorldEconomy));
		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("Have2FleetsDescription", "Make sure you have a remote fleet. Split your own fleet if necessary.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "have-2-fleets", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericStateCondition::Create(this,
																																			  [this](UFlareQuestCondition* Condition){


						  if(GetQuestManager()->GetGame()->GetPC()->GetCompany()->GetCompanyFleets().Num() > 1)
						  {
							  return true;
						  }
						  return false;
					  },
		[]()
		{
			return LOCTEXT("Have2FleetsLabel", "Have at least 2 fleets");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("OpenCompanyMenuDescription","Open the company menu (<input-action:CompanyMenu>)");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-company-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_Company));
		Steps.Add(Step);
	}


	{
		FText Description = LOCTEXT("OpenTradeRouteMenuDescription","Click on the 'Add new trade route' button.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "open-trade-route-menu", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialOpenMenu::Create(this, EFlareMenu::MENU_TradeRoute));
		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("AssignFleetDescription","Assign a remote fleet to your trade route. You can add more ships to this fleet later, including military ships to protect the convoy.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "assign-fleet", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("assign-fleet"))
			{
				return true;
			}
			return false;
		},
		[]()
		{
			return LOCTEXT("AssignFleetLabel", "Assign a fleet to a trade route");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		}));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("AddTradeRouteSectorDescription", "Add two sectors of your choice to the trade route.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "add-trade-route-sector", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCounterCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("trade-route-sector-add"))
			{
				return 1;
			}
			return 0;
		},
		[]()
		{
			return LOCTEXT("AddTradeRouteSectorConditionLabel", "Add two sectors to the trade route");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		},  "AddTradeRouteSectorcond1", 2));

		Steps.Add(Step);
	}

	{
		FText Description = LOCTEXT("TradeRouteProfitsDescription", "Try configuring your trade route with buying and selling operation to earn money.");
		UFlareQuestStep* Step = UFlareQuestStep::Create(this, "add-trade-route-sector", Description);

		Cast<UFlareQuestConditionGroup>(Step->GetEndCondition())->AddChildCondition(UFlareQuestConditionTutorialGenericEventCounterCondition::Create(this,
																																			  [](UFlareQuestCondition* Condition, FFlareBundle& Bundle)
		{
			if(Bundle.HasTag("trade-route-transaction"))
			{
				FLOGV("trade-route-transaction %d", Bundle.GetInt32("money-variation"));

				return Bundle.GetInt32("money-variation") / 100;
			}
			return 0;
		},
		[]()
		{
			return LOCTEXT("TradeRouteProfitsLabel", "Gain 5000 credits with trade routes");
		},
		[](UFlareQuestCondition* Condition)
		{
			Condition->Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
		},  "TradeRouteProfitscond1", 5000));

		Steps.Add(Step);
	}

}


/*----------------------------------------------------
	Tutorial get contrat condition
----------------------------------------------------*/
UFlareQuestConditionTutorialGetContrat::UFlareQuestConditionTutorialGetContrat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialGetContrat* UFlareQuestConditionTutorialGetContrat::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialGetContrat* Condition = NewObject<UFlareQuestConditionTutorialGetContrat>(ParentQuest, UFlareQuestConditionTutorialGetContrat::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialGetContrat::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	InitialLabel = LOCTEXT("TutorialGetContrat", "Travel to other sectors and accept a contract");
}

void UFlareQuestConditionTutorialGetContrat::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("get-contract"))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialGetContrat::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialGetContrat::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial open menu condition
----------------------------------------------------*/
UFlareQuestConditionTutorialOpenMenu::UFlareQuestConditionTutorialOpenMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialOpenMenu* UFlareQuestConditionTutorialOpenMenu::Create(UFlareQuest* ParentQuest, EFlareMenu::Type Menu)
{
	UFlareQuestConditionTutorialOpenMenu* Condition = NewObject<UFlareQuestConditionTutorialOpenMenu>(ParentQuest, UFlareQuestConditionTutorialOpenMenu::StaticClass());
	Condition->Load(ParentQuest, Menu);
	return Condition;
}

void UFlareQuestConditionTutorialOpenMenu::Load(UFlareQuest* ParentQuest, EFlareMenu::Type Menu)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	MenuType = Menu;
	
	switch (MenuType) {
	case EFlareMenu::MENU_FlyShip:
		InitialLabel = LOCTEXT("MenuNone", "Close the menu");
		break;
	case EFlareMenu::MENU_Orbit:
		InitialLabel = LOCTEXT("MenuOrbit", "Open the orbital map");
		break;
	case EFlareMenu::MENU_Company:
		InitialLabel =  LOCTEXT("MenuQuest", "Open the company menu");
		break;
	case EFlareMenu::MENU_Fleet:
		InitialLabel =  LOCTEXT("MenuFleet", "Open the fleets menu");
		break;
	case EFlareMenu::MENU_Technology:
		InitialLabel =  LOCTEXT("MenuTechnology", "Open the technology menu");
		break;
	case EFlareMenu::MENU_Sector:
		InitialLabel =  LOCTEXT("MenuSector", "Open the sector menu");
		break;
	case EFlareMenu::MENU_WorldEconomy:
		InitialLabel =  LOCTEXT("MenuWorldEconomy", "Open the economy menu in the orbital map");
		break;
	case EFlareMenu::MENU_Quest:
		InitialLabel =  LOCTEXT("MenuContracts", "Open the contracts menu");
		break;
	case EFlareMenu::MENU_TradeRoute:
		InitialLabel =  LOCTEXT("MenuTradeRoute", "Create a new trade route");
		break;
	case EFlareMenu::MENU_Leaderboard:
		InitialLabel =  LOCTEXT("MenuLeaderboard", "Open the diplomaty menu");
		break;

	default:
		FLOGV("WARNING: no label for UFlareQuestConditionTutorialOpenMenu with MenuType=%d", (int) MenuType);
		break;
	}
}

void UFlareQuestConditionTutorialOpenMenu::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("open-menu") && Bundle.GetInt32("menu") == (MenuType+0))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialOpenMenu::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialOpenMenu::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Accept quest condition
----------------------------------------------------*/
UFlareQuestConditionTutorialAcceptQuest::UFlareQuestConditionTutorialAcceptQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialAcceptQuest* UFlareQuestConditionTutorialAcceptQuest::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialAcceptQuest* Condition = NewObject<UFlareQuestConditionTutorialAcceptQuest>(ParentQuest, UFlareQuestConditionTutorialAcceptQuest::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialAcceptQuest::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	InitialLabel = LOCTEXT("TutorialAcceptContrat", "Accept a contract");
}

void UFlareQuestConditionTutorialAcceptQuest::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("accept-contract"))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialAcceptQuest::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialAcceptQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Track quest condition
----------------------------------------------------*/
UFlareQuestConditionTutorialTrackQuest::UFlareQuestConditionTutorialTrackQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialTrackQuest* UFlareQuestConditionTutorialTrackQuest::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialTrackQuest* Condition = NewObject<UFlareQuestConditionTutorialTrackQuest>(ParentQuest, UFlareQuestConditionTutorialTrackQuest::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialTrackQuest::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	InitialLabel = LOCTEXT("TutorialTrackContrat", "Track a contract");
}

void UFlareQuestConditionTutorialTrackQuest::OnEvent(FFlareBundle& Bundle)
{
	if(Completed)
	{
		return;
	}

	if (Bundle.HasTag("track-contract"))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialTrackQuest::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialTrackQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress =IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Finish quest condition
----------------------------------------------------*/

UFlareQuestConditionTutorialFinishQuest::UFlareQuestConditionTutorialFinishQuest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialFinishQuest* UFlareQuestConditionTutorialFinishQuest::Create(UFlareQuest* ParentQuest,
																					 FName ConditionIdentifierParam,
																					 int32 Count)
{
	UFlareQuestConditionTutorialFinishQuest*Condition = NewObject<UFlareQuestConditionTutorialFinishQuest>(ParentQuest, UFlareQuestConditionTutorialFinishQuest::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, Count);
	return Condition;
}

void UFlareQuestConditionTutorialFinishQuest::Load(UFlareQuest* ParentQuest,
												 FName ConditionIdentifierParam,
												 int32 Count)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionTutorialFinishQuest need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);

	QuestCount = Count;

	InitialLabel = FText::Format(LOCTEXT("FinishQuestLabel","Complete {0} contracts"),
									 FText::AsNumber(QuestCount));
}

void UFlareQuestConditionTutorialFinishQuest::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionTutorialFinishQuest::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(TUTORIAL_CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionTutorialFinishQuest::IsCompleted()
{
	return CurrentProgression >= QuestCount;
}

void UFlareQuestConditionTutorialFinishQuest::OnEvent(FFlareBundle& Bundle)
{
	if (Bundle.HasTag("success-contract"))
	{
		CurrentProgression++;
	}
}

void UFlareQuestConditionTutorialFinishQuest::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = QuestCount;
	ObjectiveCondition.MaxProgress = QuestCount;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}



/*----------------------------------------------------
Tutorial command dock condition
----------------------------------------------------*/
UFlareQuestConditionTutorialCommandDock::UFlareQuestConditionTutorialCommandDock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialCommandDock* UFlareQuestConditionTutorialCommandDock::Create(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* SpacecraftParam)
{
	UFlareQuestConditionTutorialCommandDock* Condition = NewObject<UFlareQuestConditionTutorialCommandDock>(ParentQuest, UFlareQuestConditionTutorialCommandDock::StaticClass());
	Condition->Load(ParentQuest, SpacecraftParam);
	return Condition;
}

void UFlareQuestConditionTutorialCommandDock::Load(UFlareQuest* ParentQuest, UFlareSimulatedSpacecraft* SpacecraftParam)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	Target = SpacecraftParam;

	InitialLabel = FText::Format(LOCTEXT("StartDocking", "Start docking at {0} with the wheel menu"), UFlareGameTools::DisplaySpacecraftName(Target));
}

void UFlareQuestConditionTutorialCommandDock::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("start-docking") && Bundle.GetName("target") == Target->GetImmatriculation())
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialCommandDock::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialCommandDock::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
	if (Target)
	{
		ObjectiveData->AddTargetSpacecraft(Target);
	}
}

/*----------------------------------------------------
Target spacecraftcondition
----------------------------------------------------*/

UFlareQuestConditionTutorialTargetSpacecraft::UFlareQuestConditionTutorialTargetSpacecraft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialTargetSpacecraft* UFlareQuestConditionTutorialTargetSpacecraft::Create(UFlareQuest* ParentQuest,
	FName ConditionIdentifierParam,
	UFlareSimulatedSpacecraft* Spacecraft,
	float Duration)
{
	UFlareQuestConditionTutorialTargetSpacecraft*Condition = NewObject<UFlareQuestConditionTutorialTargetSpacecraft>(ParentQuest, UFlareQuestConditionTutorialTargetSpacecraft::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, Spacecraft, Duration);
	return Condition;
}

void UFlareQuestConditionTutorialTargetSpacecraft::Load(UFlareQuest* ParentQuest,
	FName ConditionIdentifierParam,
	UFlareSimulatedSpacecraft* Spacecraft,
	float Duration)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionTutorialTargetSpacecraft need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);

	TargetDuration = Duration;
	TargetSpacecraft = Spacecraft;

	InitialLabel = FText::Format(LOCTEXT("TargetSpacecraftLabel", "Target {0} for {1} seconds"),
		UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft),
		FText::AsNumber(int32(TargetDuration)));
}

void UFlareQuestConditionTutorialTargetSpacecraft::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if (Bundle)
	{
		HasSave &= Bundle->HasFloat(TUTORIAL_LAST_TARGET_CHANGE_TS);
	}
	else
	{
		HasSave = false;
	}

	if (HasSave)
	{
		LastTargetChangeTimestamp = Bundle->GetFloat(TUTORIAL_LAST_TARGET_CHANGE_TS);
	}
	else
	{
		LastTargetChangeTimestamp = FPlatformTime::Seconds();
	}

}

void UFlareQuestConditionTutorialTargetSpacecraft::Save(FFlareBundle* Bundle)
{
	Bundle->PutFloat(TUTORIAL_LAST_TARGET_CHANGE_TS, LastTargetChangeTimestamp);
}


bool UFlareQuestConditionTutorialTargetSpacecraft::IsCompleted()
{
	AFlareSpacecraft* SpacecraftActive = GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetCurrentTarget().SpacecraftTarget;
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftActive ? SpacecraftActive->GetParent() : NULL;

	if (TargetSpacecraft == Spacecraft)
	{
		double CurrentDuration =  FPlatformTime::Seconds() - LastTargetChangeTimestamp;
		if (CurrentDuration > TargetDuration)
		{
			return true;
		}
	}
	return false;
}

void UFlareQuestConditionTutorialTargetSpacecraft::OnEvent(FFlareBundle& Bundle)
{
	if (Bundle.HasTag("target-changed"))
	{
		LastTargetChangeTimestamp = FPlatformTime::Seconds();
	}
}

void UFlareQuestConditionTutorialTargetSpacecraft::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxProgress = TargetDuration;
	ObjectiveCondition.MaxCounter = 0;
	ObjectiveCondition.Counter = 0;

	AFlareSpacecraft* SpacecraftActive = GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetCurrentTarget().SpacecraftTarget;
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftActive ? SpacecraftActive->GetParent() : NULL;


	if (TargetSpacecraft == Spacecraft)
	{
		double CurrentDuration = FPlatformTime::Seconds() - LastTargetChangeTimestamp;
		if (CurrentDuration > TargetDuration)
		{
			ObjectiveCondition.Progress = TargetDuration;
		}
		else
		{
			ObjectiveCondition.Progress = CurrentDuration;
		}
	}
	else
	{
		ObjectiveCondition.Progress = 0;
	}
	
	ObjectiveData->ConditionList.Add(ObjectiveCondition);

	if (TargetSpacecraft)
	{
		ObjectiveData->AddTargetSpacecraft(TargetSpacecraft);
	}
}

/*----------------------------------------------------
	Tutorial research value condition
----------------------------------------------------*/
UFlareQuestConditionTutorialResearchValue::UFlareQuestConditionTutorialResearchValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialResearchValue* UFlareQuestConditionTutorialResearchValue::Create(UFlareQuest* ParentQuest, int32 Count)
{
	UFlareQuestConditionTutorialResearchValue* Condition = NewObject<UFlareQuestConditionTutorialResearchValue>(ParentQuest, UFlareQuestConditionTutorialResearchValue::StaticClass());
	Condition->Load(ParentQuest, Count);
	return Condition;
}

void UFlareQuestConditionTutorialResearchValue::Load(UFlareQuest* ParentQuest, int32 Count)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetResearchPoints = Count;

	FText InitialLabelText = LOCTEXT("PlayerResearchPoints", "Gain {0} research points");
	InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(TargetResearchPoints));
}

bool UFlareQuestConditionTutorialResearchValue::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	return PlayerCompany->GetResearchValue() >= TargetResearchPoints;
}

void UFlareQuestConditionTutorialResearchValue::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = PlayerCompany->GetResearchValue();
	ObjectiveCondition.MaxCounter = TargetResearchPoints;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial technology level condition
----------------------------------------------------*/
UFlareQuestConditionTutorialTechnologyLevel::UFlareQuestConditionTutorialTechnologyLevel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialTechnologyLevel* UFlareQuestConditionTutorialTechnologyLevel::Create(UFlareQuest* ParentQuest, int32 Level)
{
	UFlareQuestConditionTutorialTechnologyLevel* Condition = NewObject<UFlareQuestConditionTutorialTechnologyLevel>(ParentQuest, UFlareQuestConditionTutorialTechnologyLevel::StaticClass());
	Condition->Load(ParentQuest, Level);
	return Condition;
}

void UFlareQuestConditionTutorialTechnologyLevel::Load(UFlareQuest* ParentQuest, int32 Level)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetLevel = Level;

	FText InitialLabelText = LOCTEXT("PlayerTechnologyLevel", "Reach technology level {0}");
	InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(TargetLevel));
}

bool UFlareQuestConditionTutorialTechnologyLevel::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	return PlayerCompany->GetTechnologyLevel() >= TargetLevel;
}

void UFlareQuestConditionTutorialTechnologyLevel::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = PlayerCompany->GetTechnologyLevel();
	ObjectiveCondition.MaxCounter = TargetLevel;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial research technology condition
----------------------------------------------------*/
UFlareQuestConditionTutorialResearchTechnology::UFlareQuestConditionTutorialResearchTechnology(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialResearchTechnology* UFlareQuestConditionTutorialResearchTechnology::Create(UFlareQuest* ParentQuest, int32 MinLevel)
{
	UFlareQuestConditionTutorialResearchTechnology* Condition = NewObject<UFlareQuestConditionTutorialResearchTechnology>(ParentQuest, UFlareQuestConditionTutorialResearchTechnology::StaticClass());
	Condition->Load(ParentQuest, MinLevel);
	return Condition;
}

void UFlareQuestConditionTutorialResearchTechnology::Load(UFlareQuest* ParentQuest, int32 MinLevel)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetMinLevel = MinLevel;

	if(MinLevel <= 1)
	{
		InitialLabel = LOCTEXT("ResearchTechnologyLowLevel", "Research a technology");
	}
	else
	{
		InitialLabel = FText::Format(LOCTEXT("ResearchTechnologyHighLevel", "Research a technology of level {0} or more"), TargetMinLevel);
	}
}

void UFlareQuestConditionTutorialResearchTechnology::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("unlock-technology") && Bundle.GetInt32("level") >= TargetMinLevel)
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialResearchTechnology::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialResearchTechnology::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial money value condition
----------------------------------------------------*/
UFlareQuestConditionTutorialMoney::UFlareQuestConditionTutorialMoney(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialMoney* UFlareQuestConditionTutorialMoney::Create(UFlareQuest* ParentQuest, int32 Count)
{
	UFlareQuestConditionTutorialMoney* Condition = NewObject<UFlareQuestConditionTutorialMoney>(ParentQuest, UFlareQuestConditionTutorialMoney::StaticClass());
	Condition->Load(ParentQuest, Count);
	return Condition;
}

void UFlareQuestConditionTutorialMoney::Load(UFlareQuest* ParentQuest, int32 Count)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetMoney = Count;

	FText InitialLabelText = LOCTEXT("PlayerMoney", "Save {0} credits");
	InitialLabel = FText::Format(InitialLabelText, FText::AsNumber(UFlareGameTools::DisplayMoney(TargetMoney)));
}

bool UFlareQuestConditionTutorialMoney::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	return PlayerCompany->GetMoney() >= TargetMoney;
}

void UFlareQuestConditionTutorialMoney::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = UFlareGameTools::DisplayMoney(PlayerCompany->GetMoney());
	ObjectiveCondition.MaxCounter = UFlareGameTools::DisplayMoney(TargetMoney);

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial order ship condition
----------------------------------------------------*/
UFlareQuestConditionTutorialOrderFreigther::UFlareQuestConditionTutorialOrderFreigther(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialOrderFreigther* UFlareQuestConditionTutorialOrderFreigther::Create(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	UFlareQuestConditionTutorialOrderFreigther* Condition = NewObject<UFlareQuestConditionTutorialOrderFreigther>(ParentQuest, UFlareQuestConditionTutorialOrderFreigther::StaticClass());
	Condition->Load(ParentQuest, Size);
	return Condition;
}

void UFlareQuestConditionTutorialOrderFreigther::Load(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetSize = Size;

	if(Size == EFlarePartSize::S)
	{
		InitialLabel = LOCTEXT("OrderSmallFreighter", "Order a small freighter");
	}
	else
	{
		InitialLabel = LOCTEXT("OrderLargeFreighter", "Order a large freighter");
	}
}

void UFlareQuestConditionTutorialOrderFreigther::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("order-ship") && Bundle.GetInt32("size") == int32(TargetSize) && Bundle.GetInt32("military") == 0)
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialOrderFreigther::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialOrderFreigther::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial build ship condition
----------------------------------------------------*/
UFlareQuestConditionTutorialBuildShip::UFlareQuestConditionTutorialBuildShip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialBuildShip* UFlareQuestConditionTutorialBuildShip::Create(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	UFlareQuestConditionTutorialBuildShip* Condition = NewObject<UFlareQuestConditionTutorialBuildShip>(ParentQuest, UFlareQuestConditionTutorialBuildShip::StaticClass());
	Condition->Load(ParentQuest, Size);
	return Condition;
}

void UFlareQuestConditionTutorialBuildShip::Load(UFlareQuest* ParentQuest, EFlarePartSize::Type Size)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetSize = Size;

	if(Size == EFlarePartSize::S)
	{
		InitialLabel = LOCTEXT("BuildSmallShip", "Build a small ship");
	}
	else
	{
		InitialLabel = LOCTEXT("BuildLargeShip", "Build a large ship");
	}
}

void UFlareQuestConditionTutorialBuildShip::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("build-ship") && Bundle.GetInt32("size") == int32(TargetSize))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialBuildShip::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialBuildShip::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial unlock station condition
----------------------------------------------------*/
UFlareQuestConditionTutorialUnlockStation::UFlareQuestConditionTutorialUnlockStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialUnlockStation* UFlareQuestConditionTutorialUnlockStation::Create(UFlareQuest* ParentQuest)
{
	UFlareQuestConditionTutorialUnlockStation* Condition = NewObject<UFlareQuestConditionTutorialUnlockStation>(ParentQuest, UFlareQuestConditionTutorialUnlockStation::StaticClass());
	Condition->Load(ParentQuest);
	return Condition;
}

void UFlareQuestConditionTutorialUnlockStation::Load(UFlareQuest* ParentQuest)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);

	FText InitialLabelText = LOCTEXT("PlayerUnloackStationTech", "Unlock at least one station  technology");
	InitialLabel = InitialLabelText;
}

bool UFlareQuestConditionTutorialUnlockStation::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	return PlayerCompany->HasStationTechnologyUnlocked();
}

void UFlareQuestConditionTutorialUnlockStation::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial start station construction condition
----------------------------------------------------*/
UFlareQuestConditionTutorialStartStationConstruction::UFlareQuestConditionTutorialStartStationConstruction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialStartStationConstruction* UFlareQuestConditionTutorialStartStationConstruction::Create(UFlareQuest* ParentQuest, bool Upgrade)
{
	UFlareQuestConditionTutorialStartStationConstruction* Condition = NewObject<UFlareQuestConditionTutorialStartStationConstruction>(ParentQuest, UFlareQuestConditionTutorialStartStationConstruction::StaticClass());
	Condition->Load(ParentQuest, Upgrade);
	return Condition;
}

void UFlareQuestConditionTutorialStartStationConstruction::Load(UFlareQuest* ParentQuest, bool Upgrade)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetUpgrade = Upgrade;

	if(!Upgrade)
	{
		InitialLabel = LOCTEXT("StartStationConstructionNew", "Start a station construction");
	}
	else
	{
		InitialLabel = LOCTEXT("StartStationConstructionUprade", "Start a station upgrade");
	}
}

void UFlareQuestConditionTutorialStartStationConstruction::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if (Bundle.HasTag("start-station-construction") && Bundle.GetInt32("upgrade") == int32(TargetUpgrade))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialStartStationConstruction::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialStartStationConstruction::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
Tutorial build station condition
----------------------------------------------------*/
UFlareQuestConditionTutorialHaveStation::UFlareQuestConditionTutorialHaveStation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialHaveStation* UFlareQuestConditionTutorialHaveStation::Create(UFlareQuest* ParentQuest, bool Upgrade, FName StationIdentifier, UFlareSimulatedSector* Sector)
{
	UFlareQuestConditionTutorialHaveStation* Condition = NewObject<UFlareQuestConditionTutorialHaveStation>(ParentQuest, UFlareQuestConditionTutorialHaveStation::StaticClass());
	Condition->Load(ParentQuest, Upgrade, StationIdentifier, Sector);
	return Condition;
}

void UFlareQuestConditionTutorialHaveStation::Load(UFlareQuest* ParentQuest, bool Upgrade, FName StationIdentifier, UFlareSimulatedSector* Sector)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetUpgrade = Upgrade;
	TargetStationIdentifier = StationIdentifier;
	TargetSector = Sector;

	FFlareSpacecraftDescription* Desc = GetGame()->GetSpacecraftCatalog()->Get(TargetStationIdentifier);

	if(TargetSector)
	{
		if(!Upgrade)
		{
			if(Desc)
			{
				InitialLabel = FText::Format(LOCTEXT("FinishSpecificStationConstructionNewInSector", "Build a new {0} in {1}"), Desc->Name, TargetSector->GetSectorName());
			}
			else
			{
				InitialLabel = FText::Format(LOCTEXT("FinishStationConstructionNewInSector", "Finish a station construction in {0}"),  TargetSector->GetSectorName());
			}
		}
		else
		{
			if(Desc)
			{
					InitialLabel = FText::Format(LOCTEXT("FinishSpecificStationConstructionUpgradeInSector", "Upgrade {0} in {1}"), Desc->Name, TargetSector->GetSectorName());
			}
			else
			{
				InitialLabel = FText::Format(LOCTEXT("FinishStationConstructionUpgradeInSector", "Finish a station upgrade in {0}"),  TargetSector->GetSectorName());
			}
		}
	}
	else
	{
		if(!Upgrade)
		{
			if(Desc)
			{
				InitialLabel = FText::Format(LOCTEXT("FinishSpecificStationConstructionNew", "Build {0}"), Desc->Name);
			}
			else
			{
				InitialLabel = LOCTEXT("FinishStationConstructionNew", "Finish a station construction");
			}
		}
		else
		{
			if(Desc)
			{
					InitialLabel = FText::Format(LOCTEXT("FinishSpecificStationConstructionUpgrade", "Upgrade {0}"), Desc->Name);
			}
			else
			{
				InitialLabel = LOCTEXT("FinishStationConstructionUpgrade", "Fishish a station upgrade");
			}
		}
	}
}

void UFlareQuestConditionTutorialHaveStation::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();
	TArray<UFlareSimulatedSpacecraft*> StationsArray;
	if (TargetSector && TargetSector->GetIdentifier() != NAME_None)
	{
		UFlareSimulatedSector* SpecificSector = GetGame()->GetGameWorld()->FindSector(TargetSector->GetIdentifier());
		if (SpecificSector)
		{
			StationsArray = PlayerCompany->GetCompanySectorStations(SpecificSector);
		}
		else
		{
			StationsArray = PlayerCompany->GetCompanyStations();
		}
	}
	else
	{
		StationsArray = PlayerCompany->GetCompanyStations();
	}

	for (UFlareSimulatedSpacecraft* Station : StationsArray)
	{
		if (!Station->IsUnderConstruction()
			&& (!TargetUpgrade || (Station->GetLevel() > 1))
			&& (TargetSector == NULL || TargetSector->GetIdentifier() == Station->GetCurrentSector()->GetIdentifier()))
		{

			if (TargetStationIdentifier == NAME_None || TargetStationIdentifier == Station->GetDescription()->Identifier)
			{
				Completed = true;
			}
			else if (Station->IsComplex())
			{
				for (UFlareSimulatedSpacecraft* Child : Station->GetComplexChildren())
				{
					if (TargetStationIdentifier == Child->GetDescription()->Identifier)
					{
						Completed = true;
						break;
					}
				}
			}
		}
	}
}

bool UFlareQuestConditionTutorialHaveStation::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialHaveStation::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


/*----------------------------------------------------
	Tutorial unlock technology condition
----------------------------------------------------*/
UFlareQuestConditionTutorialUnlockTechnology::UFlareQuestConditionTutorialUnlockTechnology(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialUnlockTechnology* UFlareQuestConditionTutorialUnlockTechnology::Create(UFlareQuest* ParentQuest, FName Identifier)
{
	UFlareQuestConditionTutorialUnlockTechnology* Condition = NewObject<UFlareQuestConditionTutorialUnlockTechnology>(ParentQuest, UFlareQuestConditionTutorialUnlockTechnology::StaticClass());
	Condition->Load(ParentQuest, Identifier);
	return Condition;
}

void UFlareQuestConditionTutorialUnlockTechnology::Load(UFlareQuest* ParentQuest, FName TechIdentifier)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	TargetIdentifier = TechIdentifier;

	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(TargetIdentifier);

	if(Technology)
	{
		FText InitialLabelText = LOCTEXT("PlayerUnlockTech", "Unlock '{0}' technology");
		InitialLabel = FText::Format(InitialLabelText, Technology->Name);
	}
}

bool UFlareQuestConditionTutorialUnlockTechnology::IsCompleted()
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	return PlayerCompany->IsTechnologyUnlocked(TargetIdentifier);
}

void UFlareQuestConditionTutorialUnlockTechnology::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}


/*----------------------------------------------------
	Buy at station condition
----------------------------------------------------*/

UFlareQuestConditionTutorialProduceResearch::UFlareQuestConditionTutorialProduceResearch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialProduceResearch* UFlareQuestConditionTutorialProduceResearch::Create(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 QuantityParam)
{
	UFlareQuestConditionTutorialProduceResearch*Condition = NewObject<UFlareQuestConditionTutorialProduceResearch>(ParentQuest, UFlareQuestConditionTutorialProduceResearch::StaticClass());
	Condition->Load(ParentQuest, ConditionIdentifierParam, QuantityParam);
	return Condition;
}

void UFlareQuestConditionTutorialProduceResearch::Load(UFlareQuest* ParentQuest, FName ConditionIdentifierParam, int32 QuantityParam)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionBuyAtStation need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Quantity = QuantityParam;


	InitialLabel = FText::Format(LOCTEXT("BuyAtStationDestroyed", "Produce {0} research points"),
								 FText::AsNumber(Quantity));
}

void UFlareQuestConditionTutorialProduceResearch::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionTutorialProduceResearch::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(TUTORIAL_CURRENT_PROGRESSION_TAG, CurrentProgression);
}


bool UFlareQuestConditionTutorialProduceResearch::IsCompleted()
{
	return CurrentProgression >= Quantity;
}

void UFlareQuestConditionTutorialProduceResearch::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = Quantity;
	ObjectiveCondition.MaxProgress = Quantity;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

void UFlareQuestConditionTutorialProduceResearch::OnEvent(FFlareBundle& Bundle)
{
	if (Bundle.HasTag("produce-research"))
	{
		CurrentProgression+=Bundle.GetInt32("amount");
	}

	if (CurrentProgression >= Quantity)
	{
		CurrentProgression = Quantity;
	}
}


/*----------------------------------------------------
Tutorial repair or refill condition
----------------------------------------------------*/
UFlareQuestConditionTutorialRepairRefill::UFlareQuestConditionTutorialRepairRefill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialRepairRefill* UFlareQuestConditionTutorialRepairRefill::Create(UFlareQuest* ParentQuest, bool Refill, bool End)
{
	UFlareQuestConditionTutorialRepairRefill* Condition = NewObject<UFlareQuestConditionTutorialRepairRefill>(ParentQuest, UFlareQuestConditionTutorialRepairRefill::StaticClass());
	Condition->Load(ParentQuest, Refill, End);
	return Condition;
}

void UFlareQuestConditionTutorialRepairRefill::Load(UFlareQuest* ParentQuest, bool Refill, bool End)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::QUEST_EVENT);
	Completed = false;
	TargetRefill = Refill;
	TargetEnd = End;

	if(TargetRefill)
	{
		if(TargetEnd)
		{
			InitialLabel = LOCTEXT("EndShipRefill", "Finish to refill ");
		}
		else
		{
			InitialLabel = LOCTEXT("StartShipRefill", "Start to refill ");
		}
	}
	else
	{
		if(TargetEnd)
		{
			InitialLabel = LOCTEXT("EndShipRepair", "Finish to repair ");
		}
		else
		{
			InitialLabel = LOCTEXT("StartShipRepair", "Start to repair ");
		}
	}
}

void UFlareQuestConditionTutorialRepairRefill::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	if((TargetRefill && TargetEnd && Bundle.HasTag("refill-end"))
			|| (TargetRefill && !TargetEnd && Bundle.HasTag("refill-start"))
			|| (!TargetRefill && TargetEnd && Bundle.HasTag("repair-end"))
			|| (!TargetRefill && !TargetEnd && Bundle.HasTag("repair-start")))
	{
		Completed = true;
	}
}

bool UFlareQuestConditionTutorialRepairRefill::IsCompleted()
{
	return Completed;
}

void UFlareQuestConditionTutorialRepairRefill::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = InitialLabel;
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = 1;
	ObjectiveCondition.MaxProgress = 1;
	ObjectiveCondition.Counter = IsCompleted() ? 1 : 0;
	ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Ship need fs condition
----------------------------------------------------*/
UFlareQuestConditionTutorialShipNeedFs::UFlareQuestConditionTutorialShipNeedFs(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialShipNeedFs* UFlareQuestConditionTutorialShipNeedFs::Create(UFlareQuest* ParentQuest, bool Refill)
{
	UFlareQuestConditionTutorialShipNeedFs* Condition = NewObject<UFlareQuestConditionTutorialShipNeedFs>(ParentQuest, UFlareQuestConditionTutorialShipNeedFs::StaticClass());
	Condition->Load(ParentQuest, Refill);
	return Condition;
}

void UFlareQuestConditionTutorialShipNeedFs::Load(UFlareQuest* ParentQuest, bool Refill)
{
	LoadInternal(ParentQuest);
	Callbacks.AddUnique(EFlareQuestCallback::TICK_FLYING);
	Callbacks.AddUnique(EFlareQuestCallback::NEXT_DAY);
	TargetRefill = Refill;
}

bool UFlareQuestConditionTutorialShipNeedFs::IsCompleted()
{
	for(UFlareSimulatedSpacecraft* Ship : GetGame()->GetPC()->GetCompany()->GetCompanyShips())
	{
		if(TargetRefill && Ship->NeedRefill())
		{
			return true;
		}

		if(!TargetRefill && Ship->GetDamageSystem()->GetGlobalDamageRatio() < 1.f)
		{
			return true;
		}
	}

	return false;
}

/*----------------------------------------------------
	Tutorial generic check condition
----------------------------------------------------*/
UFlareQuestConditionTutorialGenericStateCondition::UFlareQuestConditionTutorialGenericStateCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialGenericStateCondition* UFlareQuestConditionTutorialGenericStateCondition::Create(UFlareQuest* ParentQuest,
																											 std::function<bool (UFlareQuestCondition*)> IsCompletedParam,
																											 std::function<FText ()> GetInitalLabelParam,
																											 std::function<void (UFlareQuestCondition* Condition)> InitParam)
{
	UFlareQuestConditionTutorialGenericStateCondition* Condition = NewObject<UFlareQuestConditionTutorialGenericStateCondition>(ParentQuest, UFlareQuestConditionTutorialGenericStateCondition::StaticClass());
	Condition->Load(ParentQuest, IsCompletedParam, GetInitalLabelParam, InitParam);
	return Condition;
}

void UFlareQuestConditionTutorialGenericStateCondition::Load(UFlareQuest* ParentQuest,
															 std::function<bool (UFlareQuestCondition*)> IsCompletedParam,
															 std::function<FText ()> GetInitalLabelParam,
															 std::function<void (UFlareQuestCondition* Condition)> InitParam)
{
	LoadInternal(ParentQuest);
	InitParam(this);
	IsCompletedFunc = IsCompletedParam;
	GetInitalLabelFunc = GetInitalLabelParam;
}

bool UFlareQuestConditionTutorialGenericStateCondition::IsCompleted()
{
	return IsCompletedFunc(this);
}

FText UFlareQuestConditionTutorialGenericStateCondition::GetInitialLabel()
{
	return GetInitalLabelFunc();
}

void UFlareQuestConditionTutorialGenericStateCondition::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{

	if(AddConditionObjectivesFunc)
	{
		AddConditionObjectivesFunc(ObjectiveData);
	}
	else
	{
		FFlarePlayerObjectiveCondition ObjectiveCondition;
		ObjectiveCondition.InitialLabel = GetInitialLabel();
		ObjectiveCondition.TerminalLabel = FText();
		ObjectiveCondition.Progress = IsCompleted() ? 1 : 0;
		ObjectiveCondition.MaxProgress = 1;
		ObjectiveCondition.Counter = 0;
		ObjectiveCondition.MaxCounter = 0;

		ObjectiveData->ConditionList.Add(ObjectiveCondition);
	}
}

/*----------------------------------------------------
	Tutorial generic event condition
----------------------------------------------------*/
UFlareQuestConditionTutorialGenericEventCondition::UFlareQuestConditionTutorialGenericEventCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialGenericEventCondition* UFlareQuestConditionTutorialGenericEventCondition::Create(UFlareQuest* ParentQuest,
																											 std::function<bool (UFlareQuestCondition*, FFlareBundle& Bundle)> IsCompletedParam,
																											 std::function<FText ()> GetInitalLabelParam,
																											 std::function<void (UFlareQuestCondition* Condition)> InitParam)
{
	UFlareQuestConditionTutorialGenericEventCondition* Condition = NewObject<UFlareQuestConditionTutorialGenericEventCondition>(ParentQuest, UFlareQuestConditionTutorialGenericEventCondition::StaticClass());
	Condition->Load(ParentQuest, IsCompletedParam, GetInitalLabelParam, InitParam);
	return Condition;
}

void UFlareQuestConditionTutorialGenericEventCondition::Load(UFlareQuest* ParentQuest,
															 std::function<bool (UFlareQuestCondition*, FFlareBundle& Bundle)> IsCompletedParam,
															 std::function<FText ()> GetInitalLabelParam,
															 std::function<void (UFlareQuestCondition* Condition)> InitParam)
{
	LoadInternal(ParentQuest);
	InitParam(this);
	Completed = false;
	IsCompletedFunc = IsCompletedParam;
	GetInitalLabelFunc = GetInitalLabelParam;
}

void UFlareQuestConditionTutorialGenericEventCondition::OnEvent(FFlareBundle& Bundle)
{
	if (Completed)
	{
		return;
	}

	Completed = IsCompletedFunc(this, Bundle);
}

bool UFlareQuestConditionTutorialGenericEventCondition::IsCompleted()
{
	return Completed;
}

FText UFlareQuestConditionTutorialGenericEventCondition::GetInitialLabel()
{
	return GetInitalLabelFunc();
}

void UFlareQuestConditionTutorialGenericEventCondition::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{

	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText();
	ObjectiveCondition.Progress = 0;
	ObjectiveCondition.MaxProgress = 0;
	ObjectiveCondition.Counter = 0;
	ObjectiveCondition.MaxCounter = 0;

	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

/*----------------------------------------------------
	Tutorial generic event counter condition
----------------------------------------------------*/
UFlareQuestConditionTutorialGenericEventCounterCondition::UFlareQuestConditionTutorialGenericEventCounterCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlareQuestConditionTutorialGenericEventCounterCondition* UFlareQuestConditionTutorialGenericEventCounterCondition::Create(UFlareQuest* ParentQuest,
																											 std::function<int32 (UFlareQuestCondition*, FFlareBundle& Bundle)> IsCompletedParam,
																											 std::function<FText ()> GetInitalLabelParam,
																											 std::function<void (UFlareQuestCondition* Condition)> InitParam,
																											 FName ConditionIdentifierParam,
																											 int32 Counter)
{
	UFlareQuestConditionTutorialGenericEventCounterCondition* Condition = NewObject<UFlareQuestConditionTutorialGenericEventCounterCondition>(ParentQuest, UFlareQuestConditionTutorialGenericEventCounterCondition::StaticClass());
	Condition->Load(ParentQuest, IsCompletedParam, GetInitalLabelParam, InitParam, ConditionIdentifierParam, Counter);
	return Condition;
}

void UFlareQuestConditionTutorialGenericEventCounterCondition::Load(UFlareQuest* ParentQuest,
															 std::function<int32 (UFlareQuestCondition*, FFlareBundle& Bundle)> IsCompletedParam,
															 std::function<FText ()> GetInitalLabelParam,
															 std::function<void (UFlareQuestCondition* Condition)> InitParam,
															 FName ConditionIdentifierParam,
															 int32 Counter)
{
	if (ConditionIdentifierParam == NAME_None)
	{
		FLOG("WARNING: UFlareQuestConditionTutorialFinishQuest need identifier for state saving");
	}
	LoadInternal(ParentQuest, ConditionIdentifierParam);
	InitParam(this);
	IsCompletedFunc = IsCompletedParam;
	GetInitalLabelFunc = GetInitalLabelParam;
	TargetCounter = Counter;
}

void UFlareQuestConditionTutorialGenericEventCounterCondition::OnEvent(FFlareBundle& Bundle)
{
	CurrentProgression += IsCompletedFunc(this, Bundle);
}

bool UFlareQuestConditionTutorialGenericEventCounterCondition::IsCompleted()
{
	return CurrentProgression >= TargetCounter;
}


FText UFlareQuestConditionTutorialGenericEventCounterCondition::GetInitialLabel()
{
	return GetInitalLabelFunc();
}

void UFlareQuestConditionTutorialGenericEventCounterCondition::Restore(const FFlareBundle* Bundle)
{
	bool HasSave = true;
	if(Bundle)
	{
		HasSave &= Bundle->HasInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		HasSave = false;
	}

	if(HasSave)
	{
		CurrentProgression = Bundle->GetInt32(TUTORIAL_CURRENT_PROGRESSION_TAG);
	}
	else
	{
		CurrentProgression = 0;
	}

}

void UFlareQuestConditionTutorialGenericEventCounterCondition::Save(FFlareBundle* Bundle)
{
	Bundle->PutInt32(TUTORIAL_CURRENT_PROGRESSION_TAG, CurrentProgression);
}

void UFlareQuestConditionTutorialGenericEventCounterCondition::AddConditionObjectives(FFlarePlayerObjectiveData* ObjectiveData)
{
	FFlarePlayerObjectiveCondition ObjectiveCondition;
	ObjectiveCondition.InitialLabel = GetInitialLabel();
	ObjectiveCondition.TerminalLabel = FText::GetEmpty();
	ObjectiveCondition.MaxCounter = TargetCounter;
	ObjectiveCondition.MaxProgress = TargetCounter;
	ObjectiveCondition.Counter = CurrentProgression;
	ObjectiveCondition.Progress = CurrentProgression;
	ObjectiveData->ConditionList.Add(ObjectiveCondition);
}

#undef LOCTEXT_NAMESPACE
