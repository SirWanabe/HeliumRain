
#include "FlareHelpMenu.h"
#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareTabView.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Game/FlareGame.h"

#include "../Components/FlareTechnologyInfo.h"
#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCompanyCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"

#include "../Components/FlarePartInfo.h"
#include "../Components/FlareCompanyHelpInfo.h"
#include "../Components/FlareSpacecraftOrderOverlayInfo.h"

#define LOCTEXT_NAMESPACE "FlareHelpMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareHelpMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
//	AFlarePlayerController* PC = MenuManager->GetPC();
	Game = MenuManager->GetGame();
	double TextWrappingBig = 1.75 * Theme.ContentWidth;
	TArray<UFlareResourceCatalogEntry*> ResourceList = MenuManager->GetGame()->GetResourceCatalog()->Resources;

	// Build structure
	ChildSlot
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
		[
			SNew(SFlareTabView)

			+ SFlareTabView::Slot()
			.Header(LOCTEXT("GameplayTab", "Gameplay Information"))
			.HeaderHelp(LOCTEXT("GameplayInfo", "View general gameplay information within Helium Rain"))
			[
				SNew(SFlareTabView)
				+ SFlareTabView::Slot()
				.Header(LOCTEXT("OverviewTab", "Overview"))
				.HeaderHelp(LOCTEXT("Overviewinfo", "View overview of Helium Rain"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("OverviewTitle", "Helium Rain"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("OverviewMain", "\
Inside the giant planets, deep under the surface, helium from the atmosphere can condense due to the pression: there is an helium rain over the metallic ocean. As our Giant planet Nema is the hearth of the world, the game is named after this phenomenon.\n\n\
Helium Rain is a realistic space simulation game."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("OverviewTopTitle", "\nA space sim that doesn't cheat"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("OverviewTopMain", "\
The massive gas giant Nema is your new home. Surrounded by rings of metal and rock, the planet is a near-infinite source of fuel. Helium Rain gives you an empire to build and defend against your competitors. It's up to you to decide if you will trade resources, avoid trouble, or start wars.\n\n\
    \u2022Fly any ship of your fleet, including fighters, freighters or capital ships.\n\
	\u2022Build stations, colonize asteroids, explore new areas.\n\
	\u2022Customize your ship, change your equipment, buy new gear.\n\
	\u2022Do as you please, whether you want to become a wealthy merchant or a fierce pilot.\n\n\
Helium Rain is a game where everything is tied together. Ships and stations are built with resources that need to be mined, transported and traded. The steel you sell to a company will become a ship or a gun. The cargo you're blowing up will create a shortage. The blockade you're imposing will raise the prices. Helium Rain simulates a complex economy where all actions have consequences."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("OverviewSpaceshipsTitle", "\nSpaceships"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("OverviewSpaceshipsMain", "\
All ships have generators, radiators, weapons, engines, cargo bays or life support systems - they all work together. Every thruster on your ship plays a role. Aim for engines to pin down your enemies, or send them spinning around by destroying power stations. Most of the ships have more than thirty individual parts to target. You can customize the most important components - your main engines, attitude control thrusters and weapons. You can choose your colors, emblem, and of course, you can fly every ship in the game.\n\n\
The physics of the game are highly detailed. Power-hungry components will glow red hot when cooling fails, ships and stations can be pushed or thrown out of place, and gun shells have complex trajectories."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("OverviewComputerSpecsTitle", "\nAbout your computer"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("OverviewComputerSpecsMain", "\
Helium Rain is built for high-end Windows and Linux computers. We recommend the following:\n\n\
    \u2022A high-end video card such as the nVidia GTX970 or the AMD R290;\n\
	\u2022A quad - core processor;\n\
	\u20228GB of RAM;\n\
	\u2022A modern 64 - bits operating system(Windows 7 or Ubuntu 14.04).\n\n\
Helium Rain does not support Mac OS X or other platforms."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("OverviewOpenDevelopmentTitle", "\nOpen-source development"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("OverviewOpenDevelopmentMain", "The game's full source code is available on Github under the MIT license, found at https://github.com/arbonagw/HeliumRain. Feel free to read the code to look for interesting stuff for your own game."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("OverviewAboutTeamTitle", "\nAbout the team"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("OverviewAboutTeamMain", "\
Helium Rain was made by:\n\n\
    \u2022Gwennaël ARBONA 'Stranger' - Art, UI & Game design\n\
	\u2022Frédéric BERTOLUS 'Niavok' - Gameplay & Game design\n\
	\u2022Daisy HERBAUT - Music\n\n\
We'd also like to thank Jérôme MILLION-ROUSSEAU for the game logo.\n\
Some of us are employed by another company on top of their work in this project. This game is our own personal work and is not supported by any of our employers in any way."))
										]
									]
								]
							]
						]
					]
				]
				+ SFlareTabView::Slot()
				.Header(LOCTEXT("HowtoplayTab", "How To Play"))
				.HeaderHelp(LOCTEXT("Howtoplaynfo", "View basic information about playing Helium Rain"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayMainTitle", "How To Play Helium Rain"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayTopMain", "\
This is a basic guide on how to How To Play Helium Rain.\n\n\
In Helium Rain you play a new company in a small colony living around a giant planet Nema.\n\n\
Your goal is to extend your company and make it an empire. For this, you will need to use a smart combination of commercial and military options. You can achieve the goal you gave yourself(be the richest, the most powerful... or the only one) just by fighting, or without ever fighting, but in some cases both will be required.\n\n\
In any case you will need to make money. The simplest method to make money is trading."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayShipsTitle", "\nShips"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayShipsMain", "\
    \u2022You begin with one ship and you can buy or build other ships during the game.\n\
    \u2022You can manage ships with fleets, with as many fleets as you want, and as many ships per fleet (it's possible to mix military and trading ships)\n\
	\u2022There is a special fleet, the personal fleet: it contains the ship you control. It's possible to switch the control to other ships in your personal fleet at any time.\n\
	\u2022It's possible to give orders to other fleets, to travel or to trade, but they will manage their battles alone.\n\
	\u2022Military ships left in a sector will defend that sector against every threat.\n\
	\u2022You can automate remote fleets with a powerful trade route system. This trade route system is flexible and can be used to organize patrols."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayStationsTitle", "\nStations"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayStationsMain", "\
The most lucrative and safe way to make money is to build stations. Stations are very expensive but can ensure continued and secure revenues. Be careful, your neighbors can be friendly as long as you are small and peaceful, but being more aggressive or ambitious can lead to your fall.\n\n\
	\u2022Stations and ships are built from resources.Resources are gathered from mines or pumps or produced in other stations from primary resources.\n\
	\u2022In the early game, other already existing companies will own stations to allow the economy to work.\n\
	\u2022During the game, you will build or capture stations / mines / pumps to produce and sell resources and ships yourself.\n\
	\u2022In end game, you can become partially or totally autonomous for resources and ships production."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayPiracyTitle", "\nPiracy"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayPiracyMain", "You can also try piracy, but a good ending is not a guarantee."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayDiplomacyTitle", "\nDiplomacy"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayDiplomacyMain", "\
If your dare it, war can be a very good way to make good deals. The Helium Rain economy is responsive to your action, so destroying a competitor can make you the sole provider of a resource.\n\
To wage war, you can build various military ships to destroy enemy stations or ships."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayPurchasingspaceshipsTitle", "\nPurchasing spaceships"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayPurchasingspaceshipsMain", "\
Spaceships can be found at the shipyard, a huge station encountered in some busy sectors. Players must be at peace with the shipyard's owner to do any purchases. To do the purchase, one must go to the \"Buy Ship\" menu and click one of the buttons to order either small or large ships. Once the production price has been paid, the player must wait for their construction to end. The progress can be seen through the same menu.\n\
If there is currently a ship of the same production line in production, one must wait for that task to end before their ship's production begins.\n\
There are two other reasons a ship production might not start:\n\n\
    \u2022If the company owning the shipyard has not enough money (this happens only to ships it builds for itself, not ships ordered by a third party)\n\
	\u2022If the shipyard doesn't have the resources needed to build the ship in its cargo bay (one can help it to get them by selling missing resources to the station)\n\n\
Building ships becomes more straightforward once the player aquires their own shipyard."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayPiratesHarassTitle", "\nPirates harass me, what can I do?"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayPiratesHarassMain", "\
	\u2022Increase your military power: they won't attack you if you are stronger than them\n\
	\u2022Increase your military power : you will win battles if you are stronger than them\n\
	\u2022Pay tribute : you can always pay Pirates for peace"))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayMoveRemoteShipTitle", "\nHow can I move a remote ship without trade route?"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayMoveRemoteShipMain", "First your ship must be ready to travel: not stranded or uncontrollable, not trading, not intercepted Then to the sector menu of your destination sector. Next to the Travel button, you have a drop down to select your remote fleet (you can split the fleet if your want to move only one ship). Then click on the travel button."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayRedIconsTitle", "\nWhat is the meaning of the 3 red icons over ships?"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayRedIconsMain", "\
The icons indicate the status of the 3 main system on a ship: \n\n\
	\u2022The red big engine indicate the orbital engine are too damaged to travel, the ship is stranded\n\
	\u2022The red small engine indicate the RCS system is broken, the ship is uncontrollable\n\
	\u2022The red bullet indicate the ship's weapon system is broken or out of ammo: the ship is out of combat"))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayShipDamagedTitle", "\nMy ship is damaged, what can I do?"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayShipDamagedMain", "\
Open the sector menu of the sector were your damaged ships are. You will find a repair button on the top right part of the menu. To repair: \n\n\
	\u2022Make sure your are not in battle\n\
	\u2022It must be fleet supply in the sector. If not:\n\n\
		1. wait for another company to bring fleet supply\n\
		2. buy fleet supply in another sector and bring it where your damaged ship is\n\n\
	\u2022If you don't own the fleet supply, be sure:\n\n\
		1. to have enough money to buy it\n\
		2. that you are not at war with the owner"))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HowToPlayFindOrderedShipTitle", "\nI ordered a ship but I can't find it"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HowToPlayFindOrderedShipMain", "\
It may take many days to build a ship. You can follow the construction progress in the left part of the orbital map in the \"events\" section.\n\
Some events may delay the completion:\n\n\
	\u2022The shipyard have to finish the construction of the previous ship in the production line\n\
	\u2022The shipyard can be in outage of a resource. In this case, you can bring the missing resource or wait someone else do that\n\n\
If you don't see build ship status in orbital map, it can mean: \n\n\
	\u2022Your ship is ready : look at the sector menu of the shipyard, or in your company menu (there should of been a notification)\n\
	\u2022Your have been at war the owner of the shipyard, your order is lost\n\
	\u2022The shipyard has been captured by a company in war with you, your order is lost\n\
	\u2022Your ship has been destroyed(there should of been a notification)"))
										]
									]
								]
							]
						]
					]
				]
				+ SFlareTabView::Slot()
				.Header(LOCTEXT("TradingTab", "Trading"))
				.HeaderHelp(LOCTEXT("TradingInfo", "View information about trading"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TradingTopMainTitle", "Trading"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TradingTopMain", "Trading in Helium Rain allows companies to exchange resources between each other in a living world. Resources are produced and consumed by stations and can be transported with cargos."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("ManualTradingTitle", "\nManual trading"))
											.TextStyle(&Theme.NameFontBold)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("ManualTradingMain", "\
Manual trading goes as follows:\n\n\
	\u2022Travel to a sector\n\
    \u2022Dock with a station using a cargo ship\n\
    \u2022Buy resources (or load them if the station is owned)\n\
    \u2022Optionally travel to another sector\n\
    \u2022Dock with another station with the cargo\n\
    \u2022Sell resources (or unload, if the station is owned)\n\n\
With the \"Auto-Docking\" technology researched the above order of events are simplified; simply select the \"trade\" option on the ship or station, setup the trade deal and click \"confirm transfer\". The trade ships will then set out to do the transaction.\n\n\
Note that in most cases, a station will only sell resources it produces and buy resources it consumes. One exception to this is the Supply outpost that buys and sells fleet supply. When selling supply, the outpost will apply a significant margin to any ship or station that used it to refill or repair.\n\
Distant fleets do not need to dock with stations, but each trade (each single exchange) will take them a full day to complete."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TradeRoutesTitle", "\nTrade routes"))
											.TextStyle(&Theme.NameFontBold)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TradeRoutesMain", "\
One may automatize this process using trade routes.\n\n\
    1. Trade routes are created by clicking the \"Add new trade route\" button on the right side of the Orbital Menu.\n\n\
    2. Once in the trade route creation menu, one may first draw a list of up to 5 sectors by selecting them in the drop-down menu in the center of the screen and clicking on \"Add (X / 5)\", to the right of the drop-down menu (where X is the current number of sectors in the trade route).\n\n\
    3. Once all desired sectors have been selected this way, a small diagram appears underneath, showing each sector in order, with arrows depicting which direction the trade route will take. Sectors can be rearranged using the \"Move\" buttons under each sector's column.\n\n\
    4. Each column includes a list of Operations to be performed in that sector. Initially, only one default operation is listed per sector. Operations are added by clicking the \" * Add\" button. Each operation can be modified by clicking the \"Edit\" button; the operation's edition tab is then shown on the left side of the screen, where the type of operation, the resource involved, and the cargos' behavior can all be set. Typically the first operation of the first sector is to Buy (first drop-down menu) a Resource A (second drop-down menu). A limit in max quantity and time can both be set as well, by clicking the corresponding buttons and setting the slider that appears under each of them upon clicking. The max quantity limit will prevent cargos from loading more than a specified amount of resources. The time limit will force the cargos to start the next operation after a specified amount of days, even if they didn't fill their cargo bays.\n\n\
    5. If one sector has more than one operation, they can be ordered using the \"Move up\" and \"Move down\" buttons in each operation's setting panel. Their current order is shown in the sector's column in the form of a numbered list; that order is essential, as each operation will be performed one after the other.\n\n\
    6. \"Load\" and \"Unload\" are the primary trade operations. Each of these can be configured on how much they prioritize using resources from your own assets or from foreign assets (if at all). One may set a trade route that will buy Fuel somewhere and unload it in a sector where they have an Iron Mine; and then a second trade route that will load the Iron Mine's production, and sell it to a foreign Steelworks Station. \n\n\
	7. \"Goto Operation\" enables the ability to choose a different operation within the operations list to become the currently active operation. Useful for more complicated trade routes.\n\n\
	8. \"Maintenance\" operation allows ships within the trade route to repair and/or rearm if they need to do so.\n\n\
	9. Each operation in the trade route can have their own set of conditions added and then configured.\n\n\
    10. Once an operation is set, clicking the \"Done\" button brings up options to rename the trade route, assign a fleet to it, skip the current operation manually, pause it, and reset its stats. Stats are shown there as well once a fleet has been assigned to this route. They only become informative after several cycles.\n\n\
Assigned fleets may include military vessels for defense."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TradeGoodDealsTitle", "\nFinding good deals"))
											.TextStyle(&Theme.NameFontBold)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TradeGoodDealsMain", "\
There are several ways to find worthwhile deals or trade routes.\n\n\
    \u2022Clicking on a station (from your current sector, or from a sector previously selected through the Orbital Map) shows their current stocks, and what resources they buy/sell (Input/Output).\n\
	\u2022The Economy menu (accessed from the Orbital Map) displays a given resource's supply and demand in each (already visited) sector. Typically, setting a trade route that will buy a resource where it's heavily produced and then selling it in a sector where it's heavily consumed is the way to go.\n\
	\u2022Contracts arise from real needs. If the same delivery contracts keep coming from the same sector, a trade route may work well there as well."))
										]
									]
								]
							]
						]
					]
				]
				+ SFlareTabView::Slot()
				.Header(LOCTEXT("TechnologyTab", "Technology"))
				.HeaderHelp(LOCTEXT("Technologyinfo", "View information about technology"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologiesTopTitle", "Technologies"))
											.TextStyle(&Theme.NameFontBold)
										]	
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TechnologiesTopMain", "You can improve your company by researching technologies."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologiesUnlockTitle", "\nUnlock technology"))
											.TextStyle(&Theme.NameFontBold)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TechnologiesUnlockMain", "\
Technologies can be unlocked permanently in the technology menu. To unlock a technology, you need to pay the displayed cost. You can only unlock technology with a level inferior or equal to your current technology level.\n\n\
The cost of a technology depends on its level and your technology inflation ratio.\n\n\
Cost = BaseCost * Level * InflationRatio\n\n\
The base cost is 20 research points.\n\n\
The inflation ratio is 1 and is multiplied by 1.3 each time your technology level increase (or 1.22 if you have the instruments technology)\n\n\
Example:\n\n\
    \u2022Mining is a technology level 1. At the start of the game its cost is 20 ( 20 * 1 * 1). Unlock it multiply the inflation ratio by 1.3 to 1.3\n\
	\u2022Quick repair is a technology level 2. After buying a technology, its cost is 52 (20 * 2 * 1.3).The inflation ratio is increase to 1.69 (1.3 * 1.3)\n\
	\u2022I now unlock Metallurgy(level 2).Its cost is 67 (20 * 2 * 1.69).The inflation ratio is increase to 2.197\n\
	\u2022To purchase the level 4 technology Bombing, I need 175 research points(20 * 4 * 2.197)"))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologiesLevelTitle", "\nTechnology level"))
											.TextStyle(&Theme.NameFontBold)
										]	
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(1.50 * Theme.ContentWidth)
											.Text(LOCTEXT("TechnologiesLevelMain", "Your technology level increase by one each time you buy a technology.When your technology level increases, your Inflation Ratio is also increased. "))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologiesRPTitle", "\nResearch points"))
											.TextStyle(&Theme.NameFontBold)
										]	
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TechnologiesRPMain", "\
The research points needed to unlock technology can be gained in multiple ways:\n\n\
    \u2022Contract Reward\n\
    \u2022Research Station\n\
    \u2022Capture ships and stations"))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologiesListTitle", "\nTechnologies list"))
											.TextStyle(&Theme.NameFontBold)
										]	
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologieGeneralTitle", "\n	General"))
											.TextStyle(&Theme.NameFontBold)
										]	

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TechnologiesGeneralMain", "\
	\n\
		\u2022Instruments\n\
			\u2022Effect: 0.08 lower research multiplier\n\
		\u2022Quick Repair\n\
			\u2022Effect: 50% faster repairs\n\
		\u2022Diplomacy\n\
			\u2022Effect: 50% lower reputation penalties\n\
		\u2022Fast Travel\n\
			\u2022Effect: 50 % faster travel speed"))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologieEconomicTitle", "\n	Economic"))
											.TextStyle(&Theme.NameFontBold)
										]	

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TechnologiesEconomicMain", "\
	\n\
		\u2022Science\n\
			\u2022Unlock: Research Station, Orbital Telescope\n\
		\u2022Logistics\n\
			\u2022Unlock: Solar Power Plant, Habitation Center, Repair Outpost\n\
		\u2022Mining\n\
			\u2022Unlock: Ice Mine, Iron Mine, Silica Mine\n\
		\u2022Chemicals\n\
			\u2022Unlock: Plasic Production Plant, Carbon Refinery, Farm\n\
		\u2022Metallurgy\n\
			\u2022Unlock: Arsenal, Steelworks, Tool Factory\n\
		\u2022Shipyard\n\
			\u2022Unlock: Shipyard\n\
		\u2022Orbital Pumps\n\
			\u2022Unlock: Methane Pumping Station, Hydrogen Pump Station, Helium Pump Station\n\
		\u2022 Advanced stations\n\
			\u2022Unlock: Station Complex, Trading Hub, Foundry, Fusion Power Plant"))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("TechnologieMilitaryTitle", "\n	Military"))
											.TextStyle(&Theme.NameFontBold)
										]	

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("TechnologiesMilitaryMain", "\
	\n\
		\u2022Early warning\n\
			\u2022Effect: Enable notifications of incoming dangers\n\
		\u2022Advanced radar\n\
			\u2022Effect: Upgraded Early warning with more specific information\n\
		\u2022Pirate Tech\n\
			\u2022Effect: Unlock ship capture harpoons and increase station capture speed by up to 100%\n\
		\u2022Negotiations\n\
			\u2022Effect: 50% faster enemy station capture speed, 50% slower owned station capture speed\n\
		\u2022Flak\n\
			\u2022Effect: Unlock proximity-detonation weapons for use against fighters\n\
		\u2022Bombing\n\
			\u2022Effect: Unlock high-explosive bombs"))
										]
									]
								]
							]
						]
					]
				]
			]

			+ SFlareTabView::Slot()
			.Header(LOCTEXT("StatisticalTab", "Statistical Information"))
			.HeaderHelp(LOCTEXT("StatisticalInfo", "View various statistical information within Helium Rain"))
			[
				SNew(SFlareTabView)

				+ SFlareTabView::Slot()
				.Header(LOCTEXT("SpaceshipsTab", "Spaceships"))
				.HeaderHelp(LOCTEXT("SpaceshipsInfo", "View information about spaceships"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)


								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("SpaceshipsTabTitle", "Spaceships"))
											.TextStyle(&Theme.NameFontBold)
										]	

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("SpaceshipsTopMain", "\
Spaceships are the vessels piloted by the players in Helium Rain. Ships have generators, radiators, weapons, engines, cargo bays or life support systems - they all work together. Every thruster on a ship plays a role. Most of the ships have more than thirty individual parts to target. Players can customize their ship to enhance their performance or change their visual look.\n\
There are 2 categories of spaceships: the class S (for small) ships like fighter or bomber and the class L (for large) ships like cruiser or destroyer. Each category have a distinct list of upgrade (engine, weapons, etc), a separate production line in shipyards and different controls.\n\
Spaceships may be civilian or military. Civilian spaceships have a cargo bay and military spaceship have weapon slot (for S) or turret slot (for L)."))
										]

										+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
												.Text(LOCTEXT("SpaceshipsListTitle", "Spaceship Statistics"))
											.TextStyle(&Theme.TitleFont)
											]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(ShipSelector, SListView<TSharedPtr<FInterfaceContainer>>)
											.ListItemsSource(&ShipList)
											.SelectionMode(ESelectionMode::None)
											.OnGenerateRow(this, &SFlareHelpMenu::OnGenerateSpacecraftLine, true)
										]
									]
								]
							]
						]
					]
				]

				+ SFlareTabView::Slot()
				.Header(LOCTEXT("StationsTab", "Stations"))
				.HeaderHelp(LOCTEXT("StationsInfo", "View information about stations"))

				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("StationsTabTitle", "Stations"))
											.TextStyle(&Theme.NameFontBold)
										]	

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("StationsTopMain", "Stations in Helium Rain are places where players may trade resources, upgrade their ships and get contracts."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("StationsListTitle", "Station Statistics"))
											.TextStyle(&Theme.TitleFont)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(StationSelector, SListView<TSharedPtr<FInterfaceContainer>>)
											.ListItemsSource(&StationList)
											.SelectionMode(ESelectionMode::None)
											.OnGenerateRow(this, &SFlareHelpMenu::OnGenerateSpacecraftLine,false)
										]
									]
								]
							]
						]
					]
				]

				+ SFlareTabView::Slot()
				.Header(LOCTEXT("ComponentsTab", "Components"))
				.HeaderHelp(LOCTEXT("ComponentsInfo", "View information about ship components"))

				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							// Upgrades
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ComponentsListTitle", "Component Statistics"))
								.TextStyle(&Theme.TitleFont)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Top)
							.Padding(Theme.TitlePadding)
							[
								SNew(SHorizontalBox)
								// Engine
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SBox)
									.WidthOverride(0.50 * Theme.ContentWidth)
									[
										SNew(SVerticalBox)
										// Section title
										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(Theme.TitlePadding)
										[
											SNew(STextBlock)
											.Text(LOCTEXT("EngineUpgradeTitle", "Orbital engines"))
											.TextStyle(&Theme.SubTitleFont)
										]

										// Ship part picker
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(EngineList, SListView< TSharedPtr<FInterfaceContainer> >)
											.ListItemsSource(&EngineListData)
											.SelectionMode(ESelectionMode::None)
											.OnGenerateRow(this, &SFlareHelpMenu::GeneratePartInfo,5)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(Theme.TitlePadding)
										[
											SNew(STextBlock)
											.Text(LOCTEXT("RCSUpgradeTitle", "RCS"))
											.TextStyle(&Theme.SubTitleFont)
										]

										// Ship part picker
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(RCSList, SListView< TSharedPtr<FInterfaceContainer> >)
											.ListItemsSource(&RCSListData)
											.SelectionMode(ESelectionMode::None)
											.OnGenerateRow(this, &SFlareHelpMenu::GeneratePartInfo,5)
										]
									]
								]


								// Weapons
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Top)
									.HAlign(HAlign_Left)
									[
										SNew(SBox)
										.HAlign(HAlign_Center)
										.WidthOverride(1.25 * Theme.ContentWidth)
										[
											SNew(SVerticalBox)
											// Section title
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(Theme.TitlePadding)
											[
												SNew(STextBlock)
												.Text(LOCTEXT("WeaponsUpgradeTitle", "Weapons"))
												.TextStyle(&Theme.SubTitleFont)
											]

											// Ship part picker
											+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SAssignNew(WeaponList, SListView< TSharedPtr<FInterfaceContainer> >)
												.ListItemsSource(&WeaponListData)
												.SelectionMode(ESelectionMode::None)
												.OnGenerateRow(this, &SFlareHelpMenu::GeneratePartInfo,16)
											]
										]
									]
								]
							]
						]
					]

					+ SFlareTabView::Slot()
				.Header(LOCTEXT("CompaniesTab", "Companies"))
				.HeaderHelp(LOCTEXT("CompaniesInfo", "View information about the various companies"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)


								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("CompaniesTabTitle", "Companies"))
											.TextStyle(&Theme.NameFontBold)
										]	

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("CompaniesTopMain", "\
Companies are the factions which control the entire environment in Helium Rain. All stations and spaceships are controlled and constructed by individual companies. Companies all start the game with a differing amount of stations and ships but will expand the size of their empires. Each company has unique behaviors which effect what resources they prefer to trade and stations they prefer to build as well as how they distribute their efforts. Companies also each have a relationship with the player which effects diplomatic interactions with the company."))
										]

										// Ship part picker
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(CompanyList, SListView< TSharedPtr<FInterfaceContainer> >)
											.ListItemsSource(&CompanyListData)
											.SelectionMode(ESelectionMode::None)
											.OnGenerateRow(this, &SFlareHelpMenu::OnGenerateCompanyLine)
										]
									]
								]
							]
						]
					]
				]

						+ SFlareTabView::Slot()
						.Header(LOCTEXT("ResourcesTab", "Resources"))
						.HeaderHelp(LOCTEXT("Resourcesnfo", "View information about resources"))
						[
							SNew(SBox)
								.WidthOverride(2.2 * Theme.ContentWidth)
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Fill)
								[
									SNew(SHorizontalBox)
										// Info block
										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Left)
										.VAlign(VAlign_Top)
										.AutoWidth()
										[
											// Data
											SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(Theme.ContentPadding)
												[
													SNew(SScrollBox)
														.Style(&Theme.ScrollBoxStyle)
														.ScrollBarStyle(&Theme.ScrollBarStyle)

														+ SScrollBox::Slot()
														.Padding(Theme.ContentPadding)
														[
															SNew(SHorizontalBox)
															+ SHorizontalBox::Slot()
															.Padding(Theme.ContentPadding)
															.AutoWidth()
															.HAlign(HAlign_Left)
															.VAlign(VAlign_Top)
															[
																SNew(SVerticalBox)
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SNew(STextBlock)
																		.Text(LOCTEXT("ResourcesTopTitle", "Resources"))
																		.TextStyle(&Theme.NameFontBold)
																]
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SNew(STextBlock)
																	.TextStyle(&Theme.TextFont)
																	.WrapTextAt(TextWrappingBig)
																	.Text(FText::Format(LOCTEXT("ResourcesTopMain", "There are {0} resources in Helium Rain. They all have different uses but they are all produced and consumed in stations, except for fleet supply."),
																	ResourceList.Num()))
																]

																+SVerticalBox::Slot()
																.AutoHeight()
																[
																	SAssignNew(ResourcesBox, SVerticalBox)
																]
															]
														]
												]
										]
								]
						]

			]

			+ SFlareTabView::Slot()
			.Header(LOCTEXT("LoreTab", "Universe Lore"))
			.HeaderHelp(LOCTEXT("LoreTabInfo", "View the Helium Rain background lore"))
			[
				SNew(SFlareTabView)

				+ SFlareTabView::Slot()
				.Header(LOCTEXT("HeliumRainSolarObjects", "Solar Objects"))
				.HeaderHelp(LOCTEXT("HeliumRainSolarObjectsInfo", "View information about the Solar Objects in Helium Rain"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
												.Text(LOCTEXT("BhydriTitle", "\u2022Bhydri"))
												.TextStyle(&Theme.NameFontBold)
											]

										+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
												.TextStyle(&Theme.TextFont)
												.WrapTextAt(TextWrappingBig)
												.Text(LOCTEXT("BhydriMain", "\
	The star B Hydri is visible as the sun of Helium Rain.\n\n\
		Physical characteristics\n\n\
			\u2022Mass : 2.472 - 10 ^ 30 kg\n\
			\u2022Radius: 739 886 km\n\
			\u2022Surface : 6.879 - 10 ^ 12 km²\n\
			\u2022Surface temperature : 5916 K\n\
			\u2022Solar surface power : 69 MW / m²\n\
			\u2022Solar power : 4.777 - 10 ^ 26 W"))
											]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("NemaTitle", "\n\u2022Nema"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("NemaMain", "\
	Nema is the heart of the planetary system in Helium Rain. It is a gas giant orbiting B Hydri.\n\n\
	Physical characteristics\n\n\
		\u2022Mass : 8.421 - 10 ^ 26 kg(141 Earths)\n\
		\u2022Radius : 69 586 km\n\
		\u2022Flattening : 0.0316\n\
		\u2022Equatorial radius : 70 706 km\n\
		\u2022Polar radius : 68 465 km\n\
		\u2022Surface gravity : 11.60 m / s²(1.18 g)\n\
		\u2022Axial tilt : 3.886 °\n\
		\u2022Sidereal rotation period : 26 h 50 min 33 s\n\
		\u2022Volume : 1.411 - 10 ^ 24 m³\n\
		\u2022Mean density : 596.635 kg / m³\n\
		\u2022Albedo : 0.313\n\
		\u2022Solar constant : 3.094 KW / m²\n\n\
	Orbital characteristics\n\n\
		\u2022Orbital period : 207.95 days\n\
		\u2022Semi - major axis : 110 491 584 km\n\
		\u2022Geostationary altitude : 167304 km(236890 km radius)"))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("AnkaTitle", "\n\u2022Anka"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("AnkaMain", "\
	Anka is one of the moons orbiting Nema. \n\n\
"))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("HelaTitle", "\n\u2022Hela"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("HelaMain", "\
	Hela is one of the moons orbiting Nema. \n\n\
"))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("AstaTitle", "\n\u2022Asta"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("AstaMain", "\
	Asta is one of the moons orbiting Nema. \n\n\
"))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("AdenaTitle", "\u2022Adena"))
											.TextStyle(&Theme.NameFontBold)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("AdenaMain", "\
	Asta is one of the moons orbiting Nema. \n\n\
"))
										]
									]
								]
							]
						]
					]
				]
			]

			+ SFlareTabView::Slot()
			.Header(LOCTEXT("TechnicalTab", "Technical"))
			.HeaderHelp(LOCTEXT("TechnicalTabInfo", "Technical Information"))
			[
				SNew(SFlareTabView)

				+ SFlareTabView::Slot()
				.Header(LOCTEXT("HRFMVersionTab", "HRFM Version history"))
				.HeaderHelp(LOCTEXT("HRFMVersionInfo", "Helium Rain HRFM version history"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)
					// Info block
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.AutoWidth()
				[
					// Data
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("VersionHRFM", "HRFM Version History"))
				.TextStyle(&Theme.SubTitleFont)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.WrapTextAt(TextWrappingBig)
				.TextStyle(&Theme.TextFont)
				.Text(LOCTEXT("VersionHistoryHRFM", "Created by Wanabe\n\
https://github.com/SirWanabe/HeliumRain\n\
\n\
1.4.0\n\n\
1.3.9\n\n\
(Active Simulation) Companies can now initiate refill and repairs for their ships in the actively simulated sector\n\
(Active Simulation) Companies can now periodically tell a local trade - ship to buy or sell resources within the sector\n\
(Active Simulation) Ships which are repairing or rearming will dock to a station and stay docked unless combat occurs\n\
(Active Simulation) Re - Enabled Flak detonation(I had disabled it sometime ago.Probably due to their heavy FPS cost)\n\
(Active Simulation) Improved in sector explosion effect\n\
(Active Simulation) Small debris can sometimes break off ship components when components are destroyed\n\
\n\
(UI) Orbital Sectors menu now has toggles to turn on / off events / trade routes / fleets information\n\
(UI) Orbital Sectors menu can now show a list of fleets fit for travel.Click a fleet and then click a sector to send them.\n\
(UI) Fleets now have a new toggle option to hide them from the travel lists.\n\
(UI) Fleets assigned to un - paused trade routes and Fleets which are travelling but can't change their destination are hidden from sector travel lists.\n\
(UI) Selected Shipyards in station menu lists will show the quantity of ships in production and queued for production\n\
(UI) Viable trade filter will now filter out stations which have full cargo for a particular resource when assessing sell stations\n\
(UI) The move up / down buttons in the Trade Route screen become disabled if they aren't useful\n\
(UI) Fleet menu Remove&Add button can now remove the last ship of a fleet\n\
(UI) Skirmish components helptext now shows the description of the component\n\
\n\
(New Game Menu) Added in Starting Scenarios to the New Game screen.By default there are two basic ones included, the Trader start and the Fighter start.Helium Rain workshop mods can expand further with their own game start options.\n\
(New Game Menu) Added in 'AI Cheats' toggle to new game menu.Used to enable / disable some artificial cheats the AI can receive on difficulties above or equal to Hard\n\
\n\
(Performance) Moved sector Shell cache to a more generic cache system.Added in Bomb, Debris and Asteroid cache.\n\
(Performance) Shells, Bomb, Asteroid, ShipPilot, TurretAI tick speeds have been reduced.Optimisations to some of their processes\n\
(Performance) Set EPSCPoolMethod::AutoRelease for various types of particles, enabling UE to automatically pool / fetch them\n\
(Performance) FlareSectorButtons no longer have a tick override\n\
(Performance) Spacecraft tick moved over into a new sector tick method\n\
(Performance) Turrets no longer look for targets if the ship is not engaged in battle\n\
(Performance) Spacecraft Damage System no longer checks controllable / alive / recovery every tick, instead triggered when damaged\n\
(Performance) The spacecraft used for display previews can be reused rather than recreated / deleted each time\n\
(Performance) GetSectorBattleState() can now remember the last result on a per company basis and will only do a full refresh at specific points.\n\
(Performance) Added Shadow quality setting in graphics options.\n\
(Performance) Replaced 2X Supersampling toggle with a resolution render slider which ranges from 30%-200%.\n\
\n\
(Modding) Technology, Resource, Ships / Stations with duplicate identifiers can have 'newer' versions remove the older one from the game.This means modifications packed as a DLC can be used to override default entities without duplicates.\n\
(Modding) Added in UnlockItems to FFlareTechnologyDescription.Array of what this technology unlocks when researched.Hooks into the already existing RequiredTechnologies arrays for Ships / Stations and ship components.\n\
(Modding) The game now detects what the maximum technology level is rather than setting it to a fixed value of 5.\n\
(Modding) Added Data / FlareCompanyCataLogEntry, giving a method of properly adding new companies.However, if using this method all the original companies will be completely ignored\n\
(Modding) Added IsDisabledOverrideStats to FFlareSpacecraftDescription.If enabled along with IsDisabled will copy certain stats over to the original version of the description.Useful for overriding some stats of an existing ship if you don't have the model data etc to go along with it. Also doubles as an useful method of improving mod compatibility\n\
(Modding) Added ShipyardFabBonus to TechnologyDesc.Acts as a multiplier which lowers the resource costs of building ships.Half of the bonus is utilised by external companies when considering the cost, effectively increasing the profit margin\n\
(Modding) Added RestrictedWeapons to FFlareSpacecraftSlotGroupDescription.An array of names to limit the allowable choices of weapons for that specific slot.Also RestrictedEngines and RestrictedRCS\n\
(Modding) Added DefaultWeapon, DefaultRCS and DefaultEngine.If defined this ship will use those respective items as their defaults.\n\
(Modding) A variety of changes to improve how Drones and their Carriers work\n\
(Modding) Added SectorLimits variable to sectorDesc\n\
(Modding / Other) Added Launch / Retrieve Drone keybind and an option in the mousewheel menu\n\
\n\
Fixed(HRFanMod): Stations were running the CargoAI and could sometimes try to dock with each other which would lead to(hilarious) immersion breaking physics interactions\n\
Fixed(HRFanMod): Fixed simulation crash in UpdateReserveShips() routine\n\
Fixed(HRFanMod): Player owned fighters would not properly select targets to engage\n\
Fixed(HRFanMod): External Info Configuration / Allow External orders buttons weren't properly hiding themselves when looking at a player ship after looking at a shipyard\n\
Fixed(HRFanMod): If reentering a sector quickly(such as skipping a single day) Shipyards could collide with their previous counterpart which hadn't yet been deleted and gain collision damage along with spinning out of control\n\
Fixed(HRFanMod): Sometimes the auto-docking system would fail.\n\
Fixed(Vanilla): Station menu 'Docked Ships' section can now see the ships docked within attached complex elements\n\
Fixed(Vanilla): sudden frame rate drop in situations when turrets are trying to target during a battle where bombs are involved\n\
Fixed(Vanilla): sudden frame rate drop in situations involving Flak - weaponry.Also spread the flak damage process across multiple ticks, averaging out the fps impact in heavy situations.\n\
Fixed(Vanilla): AI cargo ships trying to dock in sector could not find a friendly station to dock at\n\
Fixed(Vanilla): Damaged stations constantly heat up over time, and as such their temperature could increase to multiple thousands of Kelvin which caused them to emit extremely bright, blinding lights.As a work around the maximum temperature value for the visual effect now has an upper limit.\n\
Fixed(Vanilla): contracts / quests can now see station complex children for completion conditions\n\
Fixed(Vanilla): If a company declares war on a player while in the Orbital Map Menu the sector buttons would not immediately update their display to show the Red war state\n\
\n\
1.3.8\n\
'lots of stuff'"))
				]]]]]]]

				+ SFlareTabView::Slot()
				.Header(LOCTEXT("VersionTab", "Version history"))
				.HeaderHelp(LOCTEXT("VersionTabInfo", "Helium Rain version history"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("VersionHistory", "Release version history:"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.WrapTextAt(TextWrappingBig)
											.TextStyle(&Theme.TextFont)
											.Text(LOCTEXT("VersionHistoryMain", "\
1.3.7\n\n\
	\u2022Fix mod launcher failing to load with Steam in Linux\n\n\
1.3.6\n\n\
	Bugs Fixed\n\
		\u2022Fix research points increasing too quickly with Instruments tech\n\
		\u2022Fix people consuming resource during construction\n\n\
1.3.5\n\
	\u2022GOG support\n\n\
1.3.4\n\n\
	\u2022Fix OpenGL3 support\n\n\
1.3.3\n\n\
	Bugs fixed\n\
		\u2022Fix station scrapping not detecting all cargos\n\
		\u2022Fix free money exploit on contracts\n\
		\u2022Cleanup unwanted files from client\n\n\
1.3.2\n\n\
	Bugs fixed\n\
		\u2022Fix crash when clicking end-of-construction notification for complexes\n\n\
1.3.1\n\n\
	Bugs fixed\n\
		\u2022Fix Linux support\n\
		\u2022Fix failure to launch mod launcher offline\n\n\
1.3.0\n\n\
	Gameplay\n\
		\u2022Add new military contracts\n\
		\u2022Add technical support for modding, including a new launcher tool\n\n\
	UI\n\
		\u2022Add Russian localization\n\
		\u2022Add mod list to the main menu\n\n\
	Bugs fixed\n\
		\u2022Fix contract locking only working for trade contract\n\
		\u2022Fix ships trading in empty sectors\n\n\
1.2.1\n\n\
	UI\n\
		\u2022HUD is now kept at a unique scale from 1080p and higher resolutions so that it's still readable\n\n\
	Bugs Fixed\n\
		\u2022Fix autopilot not disengaged with joystick inputs\n\
		\u2022Fix joystick throttle leading to extreme thrust values\n\n\
1.2.0\n\n\
	Gameplay\n\
		\u2022New flight control model that allows for concurrent inputs\n\
		\u2022Meteorite contracts have reduced meteor counts, higher intercept time, lower probability of spawning\n\
		\u2022Queued spacecraft orders are now cancelled when disabling external shipyard orders\n\
		\u2022Player-fleet ships escort the player around the sector\n\
		\u2022Add sound effect on docking\n\
		\u2022Trading quests are more worthwile\n\n\
	UI\n\
		\u2022Improve docking HUD to make it easier to understand\n\
		\u2022HUD is slightly sharper across the board\n\
		\u2022Add option to disable mouse input\n\
		\u2022Add option to show lateral velocity on the HUD\n\
		\u2022Artifact scanner now blinks to attract player attention\n\n\
	Bugs Fixed\n\
		\u2022Fix auto-trading ships ignoring the \"trade\" flag of cargo bays\n\
		\u2022Fix ships appearing in the trade menu as possible trading targets (ship-to-ship trading isn't supported)\n\
		\u2022Fix joystick throttle cancelling autopilot\n\
		\u2022Fix jerky motion of joystick controls\n\
		\u2022Fix inverted credit and debit columns\n\
		\u2022Fix speed indicator not scaling with resolution\n\
		\u2022Fix complex stations showing unusable upgrade button\n\
		\u2022Fix electronics requested at contracts while none is produced\n\
		\u2022Fix useless and confusing fleet filter appearing on the spacecraft list in the fleet menu\n\
		\u2022Fix some cases of game crashes while simulating many days quickly\n\
		\u2022Fix game crash on hotkey press in some conditions\n\
		\u2022Fix some collision issues at Blue Heart\n\n\
1.1.5\n\n\
	Bugs Fixed\n\
		\u2022Attempt to fix some crashes\n\n\
1.1.4\n\n\
	Bugs Fixed\n\
		\u2022Fix collision issues in Night's Home\n\
		\u2022Fix complex cargo bays not allowing more than half hold for trade routes\n\n\
1.1.3\n\n\
	Bugs Fixed\n\
		\u2022Fix complex stations not allowing construction of shipyards\n\n\
1.1.2\n\n\
	Gameplay\n\
		\u2022Reduce migrations of populations between sectors\n\n\
	Bugs Fixed\n\
		\u2022Fix complex cargo bays locking up\n\
		\u2022Fix miscellaneous trading issues\n\
		\u2022Fix notifications of scrapping by AI companies\n\
		\u2022Fix auto-trading fleets not handling fleet supplies\n\
		\u2022Fix auto-trading fleets not selling\n\
		\u2022Fix auto-trading still enabled after assigning a fleet to a trade route\n\
		\u2022Fix blocked quest step on the Pendulum contract\n\
		\u2022Fix station construction limits in sector blocking substation construction\n\
		\u2022Fix contract generation while a fleet is stranded at the end of the travel\n\
		\u2022Fix trade route sliders acting up\n\
		\u2022Fix typos, contracts wording, bad tooltips\n\n\
1.1.1\n\n\
	Gameplay\n\
		\u2022Trade routes don't default to trading with hubs anymore\n\
		\u2022Balance Fusion Power Plant\n\n\
	Graphics & UI\n\
		\u2022Add \"Trade with Hubs\" options to trading steps in the Trade Route menu\n\
		\u2022Add arrow icons on \"Move Up\" & \"Move Down\" buttons in the Trade Route menu\n\
		\u2022Add \"Include Hubs\" option to the World Economy menu\n\
		\u2022Add sorting buttons to the World Economy & Resource Prices menu\n\
		\u2022F4 now the default key binding for the World Economy menu, moving other menus to higher function keys (main menu is F10, settings F12, etc)\n\
		\u2022Improve performance in Night's Home\n\n\
	Bugs fixed\n\
		\u2022Fix automated trading not evaluating sales correctly\n\
		\u2022Fix automated trading stopping sector search on the first hostile sector\n\n\
1.0.2\n\n\
	Gameplay\n\
		\u2022Make artifacts more rewarding in end-game\n\
		\u2022Remove time limits on VIP contracts\n\n\
	Bugs Fixed\n\
		\u2022Fix fleet controls available on AI fleets\n\
		\u2022Fix auto-trade not actually requiring the matching technology\n\
		\u2022Fix high memory usage and performance issues in company log\n\
		\u2022Fix typos in texts\n\
		\u2022Fix \"buying\" transactions not defaulting to the contract-bound amount\n\
		\u2022Fix crash on sector reload\n\n\
1.0.1\n\n\
	Gameplay\n\
		\u2022Reduce artifact research gain from 100 to 40\n\n\
	Bugs fixed\n\
		\u2022Rename \"Travels\" title to \"Events\" for clarity\n\n\
1.0.0\n\n\
	Gameplay\n\
		\u2022Add artifact system to earn research points through exploration\n\
		\u2022Add Anomaly & Pharos sectors\n\
		\u2022Rework Trading Hub that now act as a resource buffer, renamed to Storage Hub, can no longer be built at complexes\n\
		\u2022Rework economy model\n\
		\u2022Day simulations are faster to process\n\
		\u2022Trade routes now skip inactive sectors\n\
		\u2022Trade routes now have two additional trade limit types\n\
		\u2022Ambient spacecraft power sound updated to be less annoying\n\
		\u2022Lighthouse is now a known sector on new games\n\n\
	Balancing\n\
		\u2022AI factions now produce less smaller ships in end-game\n\
		\u2022Hammer gun deals less damage and has reduced accuracy\n\
		\u2022Mjolnir gun deals more damage\n\n\
	Graphics & UI\n\
		\u2022Upgrade the side panel in the orbital menu\n\
		\u2022Upgrade menu background animation\n\
		\u2022Remove Early Access branding\n\n\
	Bugs fixed\n\
		\u2022Fix impossible contracts\n\
		\u2022Fix global AI nerf system\n\
		\u2022Fix shipyard tutorial not explaining the potential requirement of resource gathering\n\
		\u2022Fix unclear trade route menu when no fleet is assigned\n\
		\u2022Fix collision issues in some sectors\n\
		\u2022Fix crash when entering the skirmish setup menu too quickly\n\
		\u2022Fix crashes caused by back key operation after game unload\n\
		\u2022Fix crash on game load when nickname of a ship is empty\n\
		\u2022Remove dead achievement"))
										]
										+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
												.Text(LOCTEXT("VersionEAHistory", "\nEarly Access version history:"))
											.TextStyle(&Theme.SubTitleFont)
											]

										+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
												.WrapTextAt(TextWrappingBig)
												.TextStyle(&Theme.TextFont)
												.Text(LOCTEXT("VersionHistoryEAMain", "\
Early Access 10/06/08\n\n\
	Bugs fixed\n\
		\u2022Fix crash on game load in some cases\n\n\
Early Access 21/05/08\n\n\
	Graphics & UI\n\
		\u2022Add transaction log menu\n\
		\u2022Add accounting menu\n\
		\u2022Split tutorials from contracts in the contract menu\n\
		\u2022Make blur effects nicer-looking\n\n\
	Bugs Fixed\n\
		\u2022Fix crash on trade route edition\n\
		\u2022Fix unusable UI on some Linux devices\n\
		\u2022Fix autopilot stuck on the Farm\n\n\
Early Access 25/03/18\n\n\
	Gameplay Changes\n\
		\u2022Add new sector \"The Farm\" with capital station\n\
		\u2022Add retry feature to the skirmish score menu\n\
		\u2022Remove collision damage from ships to stations\n\n\
	Graphics & UI\n\
		\u2022Add backlighting from planets\n\
		\u2022Fix lighting artifacts on Nema's rings\n\
		\u2022Upgrade Nema with a nicer texture\n\
		\u2022Upgrade rendering quality in various places\n\n\
	Bugs fixed\n\
		\u2022Fix crash on travel\n\
		\u2022Fix notification freeze at the end of a travel\n\
		\u2022Fix some skirmishes not counted as a win despite victory\n\
		\u2022Fix incorrect additional binding for thrust input\n\
		\u2022Fix AI stations upgrading stations beyond max level\n\
		\u2022Fix company logo covering some contract descriptions\n\
		\u2022Fix useless cargo bays in complexes\n\
		\u2022Fix cargo bay lock system\n\
		\u2022Fix complex's shipyard not always starting production\n\
		\u2022Fix random crash on opening the sector menu\n\
		\u2022Fix some cases of scrapping not going back from the menu\n\n\
	Other changes\n\
		\u2022New Unreal Engine version 4.19\n\
		\u2022Save files are now compressed for smaller size\n\n\
Early Access 26/01/18\n\n\
	Gameplay Changes\n\
		\u2022Reduce Hammer gun damage by 33%\n\
		\u2022Reduce Artemis accuracy by 30%\n\n\
	Bugs fixed\n\
		\u2022Fix crash on combat toggle\n\n\
Early Access 25/01/18\n\n\
	Gameplay Changes\n\
		\u2022Add Skirmish mode to create custom battles against AI opponents\n\
		\u2022Rework gun controls on capital ships\n\
		\u2022Reduce diplomatic penalties on station capture\n\
		\u2022Turrets and AI ships now target missiles and meteorites\n\
		\u2022Tweak overall balance for all weapons and ships\n\
		\u2022Increase turret coverage on Anubis cruiser\n\
		\u2022Increase turret coverage on Dragon corvette\n\
		\u2022Move Leviathan camera to the front turret\n\n\
	Graphics & UI\n\
		\u2022New Dragon model\n\
		\u2022Upgrade farm model\n\
		\u2022Upgrade Blue Heart, Night's Home stations models\n\
		\u2022Upgrade weapon sounds\n\
		\u2022Upgrade alarm sounds\n\
		\u2022Add new fleet icon for military fleets\n\n\
	Bugs Fixed\n\
		\u2022Fix flak shells not working on missiles and meteorites\n\
		\u2022Fix flak shells fired with incorrect fuse setting from player ships\n\
		\u2022Fix AI ships not correctly using multiple weapon slots\n\
		\u2022Fix missiles braking away from high-velocity ships\n\
		\u2022Fix shipyard queue not transferred on capture\n\
		\u2022Fix ability to fly ships from a different fleet when using the context menu\n\
		\u2022Fix crash on hitting the \"4\" key while in menus\n\
		\u2022Fix sound cuts while switching ships\n\
		\u2022Fix effect sounds fading away while moving\n\n\
	Other Changes\n\
		\u2022New Unreal Engine version with Vulkan support\n\
		\u2022Remove APEX physics for meteorites\n\
		\u2022Add Steam trading cards\n\n\
Early Access 02/12/17\n\n\
	Graphics & UI\n\
		\u2022Add an option to list spacecrafts as fleets in object lists\n\
		\u2022Add pause button on trade routes\n\
		\u2022Add clean black bars to the story pages\n\
		\u2022Sort resource slots by input/output status\n\
		\u2022Reduce cockpit blur\n\n\
	Bugs fixed\n\
		\u2022Fix Pendulum quest not taking resources\n\
		\u2022Fix fleet tutorial\n\
		\u2022Fix docking at complexes with large ships\n\
		\u2022Fix crash on fast forward with high numbers of stations\n\
		\u2022Fix crash in the trade route menu\n\
		\u2022Fix complexes not listed in the trade menu\n\
		\u2022Fix display issues on the orbital map\n\
		\u2022Fix fleets not ordered in a sane way\n\
		\u2022Fix spacecraft hotkeys not ignored while typing\n\
		\u2022Fix cracks in RCS audio\n\n\
Early Access 23/11/17\n\n\
	Bugs fixed\n\
		\u2022Fixed ships stuck waiting on a station that doesn't trade\n\
		\u2022Fixed cargo bay locked by incorrect resources when upgrading stations\n\
		\u2022Fixed complex shipyard not offering production options on slot 8\n\
		\u2022Fixed aspect ratio of trade route menu\n\
		\u2022[tentative] Fixed crashes in the trade route menu\n\n\
Early Access 09/11/17\n\n\
	Gameplay changes\n\
		\u2022New station complex system, allowing multiple stations to work together and share resources\n\
		\u2022New station cost & limitation system, making stations more affordable\n\
		\u2022Station capture is now a manual process to avoid batch capture of unwanted stations\n\
		\u2022Stations can now be scrapped for their resources\n\
		\u2022Increase station spawn distance to avoid collisions\n\
		\u2022Add very small guidance on harpoons\n\
		\u2022Automatically fill the construction cargo bay with the production cargo bay if possible\n\
		\u2022Add a new gamepad profile for left-handed players\n\n\
	Graphics & UI\n\
		\u2022New Unreal Engine version, adding Linux support to AMD users with LLVM5\n\
		\u2022UI is now 15% denser overall to allow for more information on screen\n\
		\u2022New company menu with company renaming, emblem selection\n\
		\u2022New orbital map menu with prettier graphics & fleet locations\n\
		\u2022New sector menu layout\n\
		\u2022New technology menu layout\n\
		\u2022Rendering is now slightly sharper overall\n\
		\u2022Add button to unbind joystick axis mappings\n\
		\u2022Add sector selector to the local economy menu\n\n\
	Bugs fixed\n\
		\u2022Fix teleportation on docked ship upgrade\n\
		\u2022Fix crash when rebinding keys\n\
		\u2022Fix military tactics control unavailable outside battles\n\
		\u2022Fix arrow keys being un-rebindable\n\
		\u2022Fix manual docking failing with joysticks\n\
		\u2022Fix manual docking near Blue Heart\n\n\
Early Access 05/09/17\n\n\
	Gameplay changes\n\
		\u2022Allow manual docking with anticollision enabled\n\
		\u2022Add colinearity test on manual docking\n\
	Bugs fixed\n\
		\u2022Fix cut text on cockpit screens\n\
		\u2022Fix anticollision while docking\n\
		\u2022Fix UI cut in square or vertical resolutions\n\n\
Early Access 03/09/17\n\n\
	Graphics & UI\n\
		\u2022All stations now feature a larger, improved design\n\
		\u2022Improve cockpit fonts\n\
		\u2022Improve rendering quality to minimize aliasing\n\
		\u2022Improve rendering quality of the sun\n\n\
	Gameplay changes\n\
		\u2022Reduce turn rate of heavy AI ships to avoid to disturbing turret aiming\n\
		\u2022Make Miner's home visible at game start to allow access to steel\n\
		\u2022Make the player start with one free technology\n\
		\u2022Allow order of one heavy ship and one light ship at shipyards\n\
		\u2022Clarify contract expirations\n\
		\u2022AI player ships now only attack stations with the \"attack stations\" tactic\n\
		\u2022Telescopes now warn when no sector is left to be found\n\n\
	Bugs fixed\n\
		\u2022Fix many cases of indirect reputation loss\n\
		\u2022Fix trade route loading operation using only one ship\n\
		\u2022Fix contract buyer unable to buy the contracted resources\n\
		\u2022Fix anticollision issues in Blue Heart\n\
		\u2022Fix menu deadlock when building a station in the current sector with ship in reserve\n\
		\u2022Fix weapon swap unauthorized for barely damaged ships\n\
		\u2022Fix attack notification with 0 fleet\n\
		\u2022Fix AI upgrading stations during contracts on these stations\n\
		\u2022Fix building rate of habitations by AI\n\
		\u2022Fix resource consumption estimate of shipyards\n\
		\u2022Fix reputation shame mechanism\n\
		\u2022Fix company combat value acting up\n\
		\u2022Fix bad Thera thrust value\n\
		\u2022Fix immediate sector exit when flying a repaired ship\n\
		\u2022Fix incorrect construction values for Tokamaks\n\
		\u2022Fix crash when researching sectors with a telescope\n\
		\u2022Fix negative population\n\
		\u2022Fix AI trade bug leading to economy freeze\n\n\
"))
										]
										+ SVerticalBox::Slot()
											.AutoHeight()
											[
												//splitting text, at string limit
												SNew(STextBlock)
												.WrapTextAt(TextWrappingBig)
												.TextStyle(&Theme.TextFont)
												.Text(LOCTEXT("VersionHistoryEASecondary", "\
Early Access 27/08/17\n\n\
	Gameplay changes\n\
		\u2022Complete overhaul of shipyards to add a production queue. Spacecrafts are now built in the order they arrive so that expensive ships can be built.\n\
		\u2022Prevent end - game alliances against the player.Building up a large military force is still subject to slow loss of reputation, but AIs are no longer compelled to attack the player with no provocation.AI companies can still ally against another AI company that places itself in a dominant position.\n\
		\u2022Manual docking has much more speed control, more accurate docking computer, lower grabbing threshold on L ships\n\
		\u2022Trade route deals will now only offer actual deals that make some amount of sense\n\
		\u2022Prevent generation of trade quests if the player has zero cargo space\n\
		\u2022Make meteorites spawn closer to reduce chance of being outside sector\n\
		\u2022Reduce combat points of weapons to better reflect the impact of upgrades\n\
		\u2022Reduce nerf ratio of companies so that they handle more of the economy\n\n\
	Bugs fixed\n\
		\u2022Fix crash when AI docks early in the loading with no player ship spawned\n\
		\u2022Fix Tokamak station not producing fuel\n\
		\u2022Fix \"max quantity\" slider acting up in the trade route menu\n\
		\u2022Fix research level appearing as more than what exists\n\
		\u2022Fix joystick keys not appearing for the bindings they've been set to\n\
		\u2022Fix color picker resetting to \"round\" colors\n\
		\u2022Fix docking helpers on L ships\n\
		\u2022Fix meteorite quests displaying empty when the station is destroyed\n\
		\u2022Fix out - of - screen in some resolutions\n\
		\u2022Fix stations pushing bank account to negative values\n\
		\u2022Fix a collection of trade bugs\n\
		\u2022Fix cut text on technology menu, trade route menu\n\
		\u2022Fix bad station count in the sector menu\n\
		\u2022Fix fighting tutorial\n\
		\u2022Fix research station tutorial\n\n\
	Graphics & UI\n\
		\u2022Headlights now more powerful\n\
		\u2022Fix heat artifacts on Anubis FP view\n\
		\u2022Make debris densities slightly higher\n\
		\u2022Make some menus work better in very wide apect ratios\n\
		\u2022Make speed reticles more visible on HUD\n\
		\u2022Make stations show up as stations in the spacecraft menu\n\
		\u2022Make nose icons a bit larger\n\
		\u2022Better confirm travel text\n\
		\u2022Fix lots of typos and translation issues\n\n\
Early Access 25/08/17\n\n\
	Gameplay changes\n\
		\u2022Nerf meteorite damage to stations\n\
		\u2022Make autopilot dock at target if a station is selected\n\
	Bugs fixed\n\
		\u2022Fix water not in drop list in the trade route menu\n\
		\u2022Fix fleet selection in the sector menu\n\
		\u2022Fix joystick buttons unusable as bindings\n\
		\u2022Fix joystick bindings not saved\n\
		\u2022Fix horizontal hat of joystick ignored\n\
		\u2022Fix virtual scroll registering as a real key in settings menu\n\
		\u2022Fix docking guides disappearing\n\
		\u2022Fix spread between Solen and docks\n\
		\u2022Fix Orca cockpit cut at high FOV\n\
		\u2022Fix Atlas freighter having damaged cooling at full health\n\
		\u2022Fix French translation\n\
		\u2022Fix wrong tech for research stations in tutorial\n\
		\u2022Fix vertical artifacts in drop lists in the color panel\n\
		\u2022[tentative] Fix rare crash when selecting docking option\n\n\
	Graphics & UI\n\
		\u2022Rework sector exiting to clarify what happens\n\
		\u2022Better resource & economy menus\n\
		\u2022Show where ships are built on the orbital map\n\
		\u2022Add station, ship count to trade routes\n\
		\u2022Add gamma settings, input sensitivity setting\n\
		\u2022Make RCS effects more agressive in reaction times\n\
		\u2022Make the sky slightly dynamic\n\
		\u2022Make Hela slightly better - looking\n\n\
Early Access 22/08/17\n\n\
	New features & content\n\
		\u2022Special characteristics of stations are now shown in UI & HUD : shipyard, station in construction, contract objective, and so on.\n\
		\u2022New trade route menu that features hints on where to buy or sell resources\n\
		\u2022New alternative gamepad bindings to turn with the left stick and move with the right one\n\n\
	Gameplay changes\n\
		\u2022Change gamepad bindings : shoulder pads now mapped to speed, left stick to lateral movement, thumbsticks to roll.\n\
		\u2022Don't show the docking computer if the dock isn't on screen\n\
		\u2022Remove research level 3 technology from technology tutorial\n\
		\u2022Enable fleet orders from the wheel menu on all military ships\n\
		\u2022Add BH habitations to target objective during pendulum contract\n\
		\u2022Highlight the mine to dock at during the navigation tutorial\n\n\
	Bugs fixed\n\
		\u2022Fix crash on shipyard building\n\
		\u2022Fix trade route not perfoming operations correctly when there is more than one ship in the assigned fleet\n\
		\u2022Fix some drop lists always showing the first item\n\
		\u2022Fix a number of typo errors\n\
		\u2022Fix Blue Heart collision boxes\n\n\
	Graphics & UI\n\
		\u2022Upgrade company icons & colors\n\
		\u2022Tweak lighting to reduce artifacts on large stations\n\
		\u2022Remove atifacts at night on Hela\n\
		\u2022Add tabs to the settings menu\n\
		\u2022Show reduction of production in spacecraft info\n\
		\u2022Center mines in the inspect menu\n\
		\u2022Remove resource price list from the trade menu as it was confusing\n\
		\u2022Hide notification counter while flying\n\n\
Early Access 20/08/17\n\n\
	New features & content\n\
		\u2022Full support for gamepad & joystick\n\
		\u2022UI scaling at high resolutions\n\
		\u2022Field - of - view setting\n\
		\u2022Key bindings to cut speed, start a trade\n\n\
	Gameplay changes\n\
		\u2022Nerf the collision alarm to a more aggressive threshold\n\
		\u2022Show scanning screen instead of docking computer when both are active\n\
		\u2022Reduce the number of waypoints in tutorial\n\
		\u2022Undocking now gives the player control much faster\n\n\
	Bugs fixed\n\
		\u2022Fix blinding effects on the Solen freighter\n\
		\u2022Fix notification counter\n\
		\u2022Fix conflict with Steam VR\n\
		\u2022Fix some conflicting inputs\n\
		\u2022Fix collision risk when AI ships undock\n\
		\u2022Fix trade route quantity limit\n\
		\u2022[tentative] Fix multiple joysticks merged as one\n\
		\u2022[tentative] Fix some crashes\n\n\
	Graphics & UI\n\
		\u2022The rings of Nema are now slightly bluer\n\
		\u2022Make company icon selection easier to understand\n\
		\u2022Add gamepad input map to the settings menu\n\n\
Early Access 18/08/17\n\
This is the first public release in Early Access. Enjoy the game !\n\n\
	Gameplay changes\n\
		\u2022Add notification if a trade route is locked by a danger in the destination sector\n\n\
	Bugs fixed\n\
		\u2022Fix recovery to avoid being stranded\n\
		\u2022Fix missing step in tutorial\n\n\
	Graphics & UI\n\
		\u2022New design for the Atlas freighter\n\
		\u2022Upgrade rendering quality for the Solen freighter & Orca bomber\n\
		\u2022Upgrade hull detail with a nicer pattern\n\
		\u2022Change Fleet Supply icon to remove non - localized text"))
											]
									]
								]
							]
						]
					]
				]
				+ SFlareTabView::Slot()
				.Header(LOCTEXT("VersionBetaTab", "Version history (beta)"))
				.HeaderHelp(LOCTEXT("VersionBetaTabInfo", "Helium Rain version history(beta)"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("VersionHistoryBeta", "Beta version history:"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.WrapTextAt(TextWrappingBig)
											.TextStyle(&Theme.TextFont)
											.Text(LOCTEXT("VersionHistoryBetaMain", "\
Early Access 17/08/17\n\n\
	New features & content\n\
		\u2022The player fleet can now be renamed\n\
		\u2022Add basic mouse emulation for joysticks\n\
		\u2022Add option to invert Y - axis\n\n\
	Gameplay changes\n\
		\u2022Add reputation penalty when shooting incapacited ships\n\
		\u2022Balance pirates\n\
		\u2022Balance station cost\n\
		\u2022Reduce ship construction time\n\n\
	Bugs fixed\n\
		\u2022Fix 'Request peace' button wording if other company want peace\n\
		\u2022Fix docking bug where the docking state seemed corrupted\n\
		\u2022Fix z - fighting on Outpost stations\n\
		\u2022Fix strange dock layout on farms\n\
		\u2022Fix artifacts on Fuji exhaust\n\n\
	Graphics & UI\n\
		\u2022Upgrade design of the Anubis cruiser\n\
		\u2022Upgrade metal rendering\n\
		\u2022Add 'move' buttons to sectors in the trade route editor\n\
		\u2022Add warning when abandoning a contract\n\
		\u2022Add info that military contracts require military ships\n\n\
Early Access 03 / 08 / 17\n\
	Gameplay changes\n\
		\u2022Make light cockpits more resistant to have a chance to survive Hammer hits\n\
		\u2022Balance trade contracts to allow multiple contracts\n\
		\u2022Make AI take prisoners after battle and guard them\n\n\
	Bugs fixed\n\
		\u2022Fix auto - docking technology not unlocking auto - docking option in menus\n\
		\u2022Fix crash and game state corruption on game over while docked\n\
		\u2022Remove debugging shortcut\n\
		\u2022Wording and translation fixes\n\n\
Early Access 30 / 07 / 17\n\
	New features & content\n\
		\u2022New manual docking gameplay and tutorial\n\
		\u2022New music for some sectors\n\
		\u2022New high - resolution screenshots shortcut on R key\n\n\
	Gameplay changes\n\
		\u2022Move Hub station to end - game because it's a common mistake to buy one early\n\
		\u2022Remove support for \"no-cockpit\" HUD because it's too much work to maintain\n\n\
	Graphics & UI\n\
		\u2022Improve farming stations\n\
		\u2022Improve manual docking HUD\n\
		\u2022Make docking rings blink every second\n\
		\u2022New hotkeys for spacecraft actions\n\
		\u2022Input bindings duplicates can be automatically replaced\n\
		\u2022Scrapping of ships is now confirmed\n\
		\u2022Disable button to add trade routes if no fleet is available\n\
		\u2022Remove default binding for \"face backward\"\n\n\
	Bugs fixed\n\
		\u2022Fix fast forward button state when canceling confirmation popup\n\
		\u2022Fix transaction quantity limited by its money even for trade with its ours stations\n\
		\u2022Fix station destruction bug leading to corrupted save\n\n\
Beta - 230717\n\
	New features & content\n\
		\u2022Add Steam achievements\n\
		\u2022Add new scanner mini - game for inspection in Pendulum contract\n\n\
	Gameplay changes\n\
		\u2022No more enable pilot in fire director mode for heavy ships\n\
		\u2022Allow docking with menu during navigation tutorial\n\n\
	Graphics & UI\n\
		\u2022Improve all planet and moon render\n\
		\u2022Use progress bar instead of percentages in cockpits\n\
		\u2022Make fleet to travel selection more persistent\n\
		\u2022Update game icon\n\
		\u2022Rework ship info buttons layout\n\
		\u2022Disable no - cockpit mode\n\
		\u2022Use company customization pattern as background pattern in UI\n\
		\u2022Improve quality of weapon impacts\n\
		\u2022Make the sky slightly brighter\n\
		\u2022A lot of small UI and wording improvements\n\n\
	Bugs fixed\n\
		\u2022UI : fix too long joystick button and names\n\
		\u2022UI : fix dragging issues in menus causing unexpected rotation on background assets\n\
		\u2022UI : fix HUD distance rounding issue\n\
		\u2022UI : fix selection of ships with light during piloting\n\
		\u2022UI : fix wrong warning when targeting the tracked station\n\
		\u2022General : a lot of french translation fixes\n\n\
Beta - 100717\n\
	New features & content\n\
		\u2022Add a meteorites intercept contract\n\
		\u2022Add trade route statistics\n\
		\u2022Add experimental French translation(some layout issues)\n\n\
	Gameplay changes\n\
		\u2022Don't generate military contracts if the player doesn't have military ships\n\
		\u2022Use interception time instead of distance to show aim indicators at a more accurate range\n\n\
	Graphics\n\
		\u2022Upgrade shadowing quality to reduce artifacts\n\
		\u2022Upgrade resolution of shadows on Nema's rings\n\
		\u2022Reduce performance cost of Eradicator impact effects\n\
		\u2022Update splash screen\n\n\
	Bugs fixed\n\
		\u2022General : fix ships ejected from trade route if not all ship were trading\n\
		\u2022General : fix research station upgrade effect\n\
		\u2022General : fix telescope station upgrade effect\n\
		\u2022General : make the player ship not fully destructible instead of the last alive ship to avoir recovery bug\n\
		\u2022UI : fix funny key change warning when removing a key assignment\n\
		\u2022UI : fix trade slider init with sell to a station for a contract\n\
		\u2022UI : fix lock trade checkbox that stop the production\n\
Beta - 300617\n\
	General\n\
		\u2022pass the game to qwerty by default\n\
		\u2022move default binding of 'lock direction' to capslock and 'combat zoom' to space\n\n\
	Gameplay\n\
		\u2022no more allow station construction in a sector in danger\n\n\
	AI\n\
		\u2022make AI able to declare join war againt enemy more powerful than every company induvidialy\n\
		\u2022make AI able to setup attack from multiple company to allow multiple company to win against one powerful ennemy\n\n\
	UI\n\
		\u2022rename \"Travel and Order\" in \"Incoming Events\"\n\
		\u2022enable notification masking\n\
		\u2022show again trade routes in orbital map\n\
		\u2022add current battle in incoming events list\n\
		\u2022show warning in settings menu when trying to assign an already assigned key\n\
		\u2022set default quantity for trade to the right value when the tracked contract have a sell condition\n\n\
	Graphics\n\
		\u2022add iridescence effect to metals\n\
		\u2022improve reflection quality\n\n\
	Balance\n\
		\u2022move default ship count in sector to 30\n\n\
	Bugs\n\
		\u2022general : fix Boneyard collision mode\n\
		\u2022general : fix crash if trying to upgrade a ship with weapon that are not unlock by technology(after ship capture)\n\
		\u2022ui : fix external camera and menu camera for Windows\n\
		\u2022ui : fix trading status text\n\
		\u2022ui : fix fly interdiction text\n\
		\u2022contract : fix duplicate fail condition when a trade contract have 2 stations owned by the same company\n\
		\u2022contract : various contract text fixes\n\n\
Beta - 190617\n\
	General\n\
		\u2022switch to Unreal Engine 4.16\n\
		\u2022add Deimos Games logo and info\n\n\
	Gameplay\n\
		\u2022new more understandable recovery system : you don't have a new free ship but the old one is partially repair and you have a fine.\n\
		\u2022don't generate trade contracts with stations being captured\n\n\
	Tutorials\n\
		\u2022add fleet management tutorials\n\
		\u2022add remote fleet tutorial\n\
		\u2022add trade route tutorial\n\
		\u2022force to buy a freigther in \"buy ship\" tutorial\n\n\
	UI\n\
		\u2022rework EULA and credit menu\n\
		\u2022auto track the last contract(instead of the first) by defaut when a contract is finished to allow chained quest to be smooth\n\n\
	Graphics\n\
		\u2022assure that planet texture quality is always good\n\
		\u2022add boneyard icons to its substations\n\n\
	Balance\n\
		\u2022make stations a lot more affordable for the player in early game\n\
		\u2022the price now depend only of the player station count in its company and in the sector\n\
		\u2022Increase trade contract reward :\n\
		\u2022trade contracts :\n\
			\u2022from 1000 + 500 sqrt(travel_duration) + 50 sqrt(resource_quantity)\n\
			\u2022to 1000 + 1000 sqrt(travel_duration) + 100 sqrt(resource_quantity)\n\
		\u2022buy and sell contracts :\n\
			\u2022from 500 + 50 * sqrt(resource_quantity)\n\
			\u2022to 500 + 100 * sqrt(resource_quantity)\n\
		\u2022make trade contract volume more variable if capacity transport of the player vary\n\
		\u2022make axis reputation very stable as they are neutral\n\n\
	Bugfix\n\
		\u2022ui : multiple wording fixes\n\
		\u2022contracts : fix contract unicity\n\
		\u2022contracts : avoid contract to fail at acceptation making them not available as soon as the failed conditions are reached\n\n\
Beta - 120617\n\
	Gameplay\n\
		\u2022Quick patch to fix risk of economy collapse that a lock a game, with others fixes and improvements.\n\
		\u2022increase anticollision system precision and make it adaptative to ships breaking capabilities\n\n\
	UI\n\
		\u2022show repair and refill deadline in travel sector\n\
		\u2022make player confirm fast forward if the is no incoming events\n\
		\u2022make notification slower to disappear(7s -> 10s)\n\n\
	Balance\n\
		\u2022tutorial : reduce research needs for technology tutorial from 50 to 30, to match with the new technology base price\n\n\
	Bugs\n\
		\u2022economy : avoid economy collapse due to people not spending enough money\n\
		\u2022autopilot : fix exit avoidance system using incoherent center of sector\n\
		\u2022ui : fix sector menu for travel sectors\n\
		\u2022graphics : fix planetarium for travel sectors after sector exit"))
										]
										// splitting up the string...too many characters
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.WrapTextAt(TextWrappingBig)
											.TextStyle(&Theme.TextFont)
											.Text(LOCTEXT("VersionHistoryBetaSecondary", "\
\nBeta - 090617\n\
	General\n\
		\u2022Update legals informations\n\n\
	Gameplay\n\
		\u2022make bombs and missiles destructible by the player\n\
		\u2022no more make ships assigned to a trade route travel automatically to a dangerous sector\n\n\
	UI\n\
		\u2022move trade route selection to company menu to make more space to incoming events\n\
		\u2022refactor incoming attack list and notifications\n\
		\u2022make new contracts notifications no more stop fast forward\n\
		\u2022add more details about shipyard state in the shipyard details\n\
		\u2022make harder to accidentally dump precious resources\n\
		\u2022better texts when trade operation is not possible\n\
		\u2022add notification when fleet repair or refill end\n\
		\u2022add information about station under construction in trade menu\n\
		\u2022rename \"World prices\" menu to \"Economy\"\n\n\
	Graphics\n\
		\u2022move planetarium to follow travel progress\n\
		\u2022improve engine render\n\
		\u2022improve flight effects\n\
		\u2022improve asteroid rendering quality\n\n\
	Balance\n\
		\u2022reduce technology base cost from 50 to 30\n\
		\u2022refactor station technologies : the new level 1 \"science\" technology allow to build research station and telescope, the pumping technology move from level 1 to level 3, the foundry is now unlock with advanced stations\n\
		\u2022change the Solen to give it a cargo bay of 2x50 instead of 1x100\n\
		\u2022make Hammer gun more precise but a lot more expensive\n\
		\u2022reduce Omen mass from 32t to 16t\n\
		\u2022force the player to manually make peace after an AI declare war to him\n\
		\u2022make pacifism move more slowly : pacifism increase base rate : 2.0 -> 0.8, pacifism decrease base rate : 1.0 -> 0.6, give reputation when ordering a ship to a company(ship price / 1000)\n\
		\u2022increase inter company UBI contribution from 0.001 to 0.002\n\n\
	Bugs\n\
		\u2022ui : fix crash when editing a trade route operation if no fleet was assigned\n\
		\u2022ui : fix tool - tip for when the \"Remove ship\" button in edit fleet menu was disabled\n\
		\u2022ui : make failed contracts due to expiration date not display \"1 day left\"\n\
		\u2022ui : fix minor interface glitch in not visited sectors\n\
		\u2022ui : fix rotation of HR logo in menus\n\
		\u2022ui : fix The Forge sector description\n\
		\u2022graphics : fix AA settings bug\n\
		\u2022graphics : adapt resolution when enabling full screen\n\
		\u2022graphics : remove tesselation from Colossus to ensure Linux support\n\
		\u2022general : fix repair not correctly ending bug\n\
		\u2022general : check recovery at the end of each day and not only at the end of fast forward\n\n\
Beta - 020617\n\
	General\n\
		\u2022add support of unidirectional joystick thrust\n\
		\u2022add support for joystick dead zones\n\n\
	UI\n\
		\u2022show \"missing resources\" when a ship construction is lock because of a resource outage\n\
		\u2022show incoming threats in red in the travel list\n\
		\u2022ensure long joystick names can be shown\n\
		\u2022hide precise light ratio as it's not used for now\n\
		\u2022make transport fee a lot more visible\n\
		\u2022allow to set trade route quantity limit precisely\n\n\
	Graphics\n\
		\u2022reduce sun intensity\n\
		\u2022minor change on dust effects\n\n\
	Balance\n\
		\u2022allow shipyard owners to be in dept to build ships order by others companies\n\n\
	Bugs\n\
		\u2022contract : fix combat tutorial\n\
		\u2022ui : fix menu looping bug\n\
		\u2022ui : fix multiple typo in contracts\n\
		\u2022ui : fix highlight box not updated in HUD\n\
		\u2022graphics : fix Nema rings LOD\n\
		\u2022economy : fix crash when canceling ship construction\n\
		\u2022general : fix bad file name Windows firewall\n\n\
Beta - 290517\n\
	Gameplay\n\
		\u2022never destroy a ship completely because of a collision\n\
		\u2022allow control of ship rotation in addition to ship translation from the external camera(space bar by default)\n\
		\u2022make large ships unafraid of hostile small ships to avoid destabilizing turrets\n\n\
	Contracts\n\
		\u2022add lots of VIP names and avoid reusing the same names for multiple contracts\n\
		\u2022add a 30 days delay in the Pendulum contract after the pirate step\n\
		\u2022improve wording in contract tutorial\n\n\
	Tutorials\n\
		\u2022add combat tutorial\n\
		\u2022add ships repair tutorial\n\
		\u2022add ships refilling tutorial\n\n\
	UI\n\
		\u2022aggregate all secondary contract notifications in one\n\
		\u2022hide expiration conditions : they were misleading and taken as failed conditions\n\
		\u2022show more details about ship state in ship lists\n\
		\u2022don't notify pointless \"battle lost\" notification, and don't lock fast forward in these cases\n\
		\u2022give detailed infos about abandoned ships when traveling\n\
		\u2022treat ships in paused trade route as regular ships\n\
		\u2022show trade routes status everywhere\n\
		\u2022forbid abandon of tutorial and story contracts\n\n\
	Graphics\n\
		\u2022change the look of pumping stations, research stations and outposts to differentiate them\n\
		\u2022improve dust effects\n\
		\u2022improve firing effects\n\
		\u2022improve visualization of damage on destroyed ship parts\n\
		\u2022move Orca camera\n\
		\u2022add very small chromatic aberration\n\
		\u2022fix planetary texture LOD\n\n\
	Balance\n\
		\u2022technology : increase research point reward by 60 %\n\
		\u2022diplomacy : divide rank based reputation loose by 30\n\
		\u2022military : divide large ship reparation speed by 5\n\
		\u2022military : slightly improve Artemis\n\n\
	Bugs\n\
		\u2022ui : fix trade quantity when the company has debt\n\
		\u2022ui : fix line wrapping bug in trade menu\n\
		\u2022ui : fix \"No objects\" misleading in sector menu\n\
		\u2022combat : fix AI ship and turret pilot aim precision\n\
		\u2022combat : fix Kami turret angles and collision mesh\n\
		\u2022combat : fix collision issue in The Spire\n\
		\u2022general : fix AA quality settings not modifiable\n\
		\u2022general : fix sector defense contract not loaded correctly\n\
		\u2022general : fix crash if a station collide something different from a ship\n\n\
Beta - 220517\n\
	Gameplay\n\
		\u2022revamp diplomacy system\n\n\
	Contracts\n\
		\u2022add Pendulum history contract\n\n\
	Tutorials\n\
		\u2022add technology and research station tutorial\n\
		\u2022add buy ship tutorial\n\
		\u2022add station construction and upgrade tutorial\n\
		\u2022add targeting tutorial\n\n\
	UI\n\
		\u2022allow T key to select the contract objectif if not in battle\n\
		\u2022show contract fail conditions in contracts menu\n\
		\u2022add needs columns in ecomomy menu\n\
		\u2022show sector population\n\n\
	Graphics\n\
		\u2022redesign boneyard sector\n\
		\u2022better metal, exhaust and bloom effects\n\n\
	Balance\n\
		\u2022economy : increase the \"nerf ratio\" but apply it only to level 1 cargo bay size of station\n\
		\u2022economy : increase arsenal production\n\
		\u2022economy : increase plastic factory production\n\
		\u2022economy : increase station cargo bay size\n\
		\u2022economy : balance Kami ship cost\n\
		\u2022economy : limit high end consumer resource consumption if they are expensive\n\
		\u2022contracts : generate hunt contract only on travel end\n\
		\u2022contracts : add duration fail condition for VIP contracts\n\
		\u2022technology : move shipyard construction unlock in a specific technology\n\
		\u2022technology : now, only one technology is need to increase its technology level(2 before)\n\
		\u2022technology : reduce research point production cost\n\
		\u2022world : a lot of balance in the game creation setup\n\n\
	Bugs\n\
		\u2022flying : fix station bad placement if they are capture before being in an active sector\n\
		\u2022flying : fix ship disappearing randomly if stopped\n\
		\u2022flying : fix weapon firing empty weapon first on load game\n\
		\u2022economy : fix fleet supply consumption bug\n\
		\u2022economy : fix shipyard consumption estimation\n\
		\u2022contract : fix missing diplomatic fail and expiration conditions in generated quests\n\
		\u2022ui : fix trading permission display\n\
		\u2022ui : fix external and menu camera control\n\
		\u2022ui : station list order in sector menu\n\
		\u2022ai : fix a lot of AI bug\n\
		\u2022general : various performance improvements"))
										]
									]
								]
							]
						]
					]
				]
			+ SFlareTabView::Slot()
				.Header(LOCTEXT("ModdingTab", "Modding"))
				.HeaderHelp(LOCTEXT("ModdingTabInfo", "Help for modding Helium Rain"))
				[
					SNew(SBox)
					.WidthOverride(2.2 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
						// Info block
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							// Data
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(SScrollBox)
								.Style(&Theme.ScrollBoxStyle)
								.ScrollBarStyle(&Theme.ScrollBarStyle)

								+ SScrollBox::Slot()
								.Padding(Theme.ContentPadding)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.Padding(Theme.ContentPadding)
									.AutoWidth()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("ModToolsOverviewHelp", "Modding Helium Rain - overview"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("ModToolsOverviewMain", "\
Modding support in Helium Rain can enable new ships, stations, resources, sectors, or ship upgrades. They cannot add new quests, story, or new C++ functionality - if you want to modify the game sources, please follow the instructions on our public Git repository, found at https://github.com/arbonagw/HeliumRain. Source mods can't be distributed through Steamworks, and by design, a source code mod would behave as an entirely separate game - two source mods can't be combined.\n\n\
Creating mods will require the Helium Rain Mod Kit, available through the Epic Games Launcher for free. No other software is required. The Helium Rain Mod Kit doesn't provide all game assets, though about half of them are provided for reference. Helium Rain mods can then be distributed through Steamworks and applied automatically to users, but developers can make them available elsewhere to be applied manually on GOG & itch.io releases."))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("SourcesCreating", "\nCreating a mod"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("SourcesCreatingMain", "\
To start the modding process, you will need to download the Epic Games Launcher, register, and find the Helium Rain Mod Kit at the bottom of the Store tab. Install it and make sure to remember where the mod kit is installed on your computer, as you will be working from there to set it up. Once installed, the modding SDK should have a few folders - one named \"UE_4.20\", which is the engine, and one named \"HeliumRainModKit\" - everything will happen in that second folder.\n\n\
Modding will work by creating a plugin for the game. We'll create a mod named ExampleMod for reference - make sure to always use the same name everywhere, without spaces or special characters. This mod should already exist - make sure to adapt this tutorial to your own mod, or try first with the example.\n\
A plugin has to be created in the /HeliumRainModKit/Mods folder of the SDK, and has three mandatory components : \n\n\
	\u2022A plugin file, ExampleMod.uplugin, created manually\n\
	\u2022A preview image for Steamworks, Preview.png, created manually\n\
	\u2022A content directory, that will be created through the editor\n\n\
The syntax of the plugin file is quite easy. Here is an example.\n\n\
		{\n\
			\"FileVersion\" : 3,\n\
			\"FriendlyName\" : \"Example Mod\",\n\
			\"Version\" : 1,\n\
			\"VersionName\" : \"1.0\",\n\
			\"CreatedBy\" : \"Deimos Games\",\n\
			\"CreatedByURL\" : \"http://helium-rain.com\",\n\
			\"EngineVersion\" : \"4.20.0\",\n\
			\"Description\" : \"Example mod for the Helium Rain Mod Kit\",\n\
			\"Category\" : \"User Mod\",\n\
			\"EnabledByDefault\" : true,\n\
			\"Modules\" :\n\
			[\n\
			],\n\
			\"CanContainContent\" : true\n\
		}\n\n\
Once this is done, you can launch the Unreal Engine editor right away. You don't need to decide on a great description or a kick-ass preview - you can edit both later and update the mod on Steam."))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("SourcesContentHelp", "\nCreating content"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("SourcesContentHelpMain", "\
This part assumes you are familiar with Unreal Engine 4. We won't provide our own guide, or detailed guides on how the game assets are structured.\n\
You will work a lot with the Content Browser window. Make sure a folder list panel is visible on the left side of that window - toggle it with the small button below \"Add New\" on the top left. Your mod folder should appear at this point, this is where you will work from ! You can only add content here - changes elsewhere will not be applied. You can't remove existing content or update a material - you can however copy a material to the mod folder, change it, and use it for your content.\n\
Here are some details on Helium Rain's architecture.\n\n\
	\u2022All of our gameplay data is created through what we call DataAssets. You can find many of them in the /Gameplay/Catalog folder of the Content Browser as examples - look at how they're built, and learn from that.\n\
	\u2022Ships or stations have an additional Templatefile, built with UE4's Blueprint tool. Templates let us actually design the ship and place parts. These are found in /Ships/Templates and /Stations/Templates, and are referenced from the DataAssets.\n\
	\u2022A new ship would require both a new data asset and a new template.A modified ship could only use a new data asset, and keep the base template.\n\
	\u2022A new weapon or engine would require a data asset and a StaticMesh.\n\
	\u2022New sectors only require a data asset, but can also feature an entire new level, if you add a sub - level to the Space map, and provide its name in the data asset.\n\n\
Talk with other modders on the Steam forums, or our Reddit forum, to get answers ! The source code for the game is also available if you've got questions. "))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("SourcesBuildingHelp", "\nBuilding a mod"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("SourcesBuildingMain", "\
To build a mod, you will need to open the Project Launcher window in Unreal Engine. Add a new profile using the small \"plus\" sign on the bottom right, or use the provided \"Mod\" profile.\n\
We will create a profile that \"cooks\" a \"Shipping\" \"DLC\", using a \"PAK file\" with compression, based on a previous release of the game. Let's explain all of this.\n\n\
	\u2022Cooking is the process to create a usable mod file for a packaged game. This file will be a .pak file, a compressed archive of your mod.\n\
	\u2022Make sure you pick the \"HeliumRain\" project by browsing for the HeliumRainModKit / HeliumRain.uproject file.\n\
	\u2022Make sure to set how you would like to cook to \"By the book\"\n\
	\u2022We will cook for WindowsNoEditor, and LinuxNoEditor. Please cook for the \"en\" culture.\n\
	\u2022What is called DLC here is your mod - use the name of your own mod.\n\
	\u2022The release is the name of the folder inside / Releases / -currently \"1.3.4-modkit\".\n\
	\u2022Make sure Build DLC and Include Engine Content is enabled\n\
	\u2022\"Compress Content\", \"Save packages without versions\", \"Store all content in a single file (UnrealPak)\" and \"Don't include editor content in build\" should be selected"))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("SourcesUploadingHelp", "\nUploading a mod"))
											.TextStyle(&Theme.SubTitleFont)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
											.WrapTextAt(TextWrappingBig)
											.Text(LOCTEXT("SourcesUploadingHelpMain", "\
This is the easy part. Open a console command (cmd), go to the mod kit folder (\"cd / D \"C:\\...\\HeliumRainModKit\\\"), and run the following command : \n\n\
	\u2022Boiler --game HeliumRain --mod ExampleMod\n\n\
Your mod will be uploaded as a hidden new project on the Steamworks page for Helium Rain. Look for a menu near your user name to find your uploaded mods, complete the settings, and make it visible. Players can now subscribe to the mod, enabling them next time the game runs. Enabled mods are listed on the bottom of the main menu."))
										]
									]
								]
							]
						]
					]
				]
			]
		];

	// Init arrays

	StationList.Empty();
	ShipList.Empty();

	CompanyListData.Empty();
	CompanyList->RequestListRefresh();

	EngineListData.Empty();
	EngineList->RequestListRefresh();
	RCSListData.Empty();
	RCSList->RequestListRefresh();
	WeaponListData.Empty();
	WeaponList->RequestListRefresh();
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHelpMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareHelpMenu::Enter()
{
	FLOG("SFlareHelpMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareSpacecraftComponentsCatalog* Catalog = MenuManager->GetGame()->GetShipPartsCatalog();
	UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();
	UFlareCompanyCatalog* CompanyCatalog = MenuManager->GetGame()->GetCompanyCatalog();

	if (!StationList.Num())
	{
		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->StationCatalog.Num(); SpacecraftIndex++)
		{
			// Candidate have to be not a substation and available in tech
			FFlareSpacecraftDescription* Description = &SpacecraftCatalog->StationCatalog[SpacecraftIndex]->Data;
			if (!Description->IsSubstation)
			{
				UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->StationCatalog[SpacecraftIndex];
				StationList.AddUnique(FInterfaceContainer::New(&Entry->Data));
			}
		}
	}

	if (!ShipList.Num())
	{
		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->ShipCatalog.Num(); SpacecraftIndex++)
		{
			// Candidate have to be not a substation and available in tech
			UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->ShipCatalog[SpacecraftIndex];
			ShipList.AddUnique(FInterfaceContainer::New(&Entry->Data));
		}
	}

	if (!CompanyListData.Num())
	{
		/*
		for (int CompanyIndex = 0; CompanyIndex < CompanyCatalog->Companies.Num(); CompanyIndex++)
		{
			FFlareCompanyDescription* Entry = CompanyCatalog->Companies[CompanyIndex];
			CompanyList.AddUnique(FInterfaceContainer::New(Entry));
		}
		*/

		TArray<FFlareCompanyDescription*> CompanyWorkingList;
		MenuManager->GetGame()->GetCompanyCatalog()->GetCompanyList(CompanyWorkingList);

		for (FFlareCompanyDescription* Company : CompanyWorkingList)
		{
			CompanyListData.AddUnique(FInterfaceContainer::New(Company));
		}
		CompanyList->RequestListRefresh();
	}

	if (!EngineListData.Num())
	{
		TArray<FFlareSpacecraftComponentDescription*> PartList;

		// Engines
		PartList.Empty();
		Catalog->GetEngineList(PartList, EFlarePartSize::S, NULL, NULL);
		Catalog->GetEngineList(PartList, EFlarePartSize::L, NULL, NULL);

		for (FFlareSpacecraftComponentDescription* Part : PartList)
		{
			EngineListData.AddUnique(FInterfaceContainer::New(Part));
		}
		EngineList->RequestListRefresh();
	}

	if (!RCSListData.Num())
	{
		TArray<FFlareSpacecraftComponentDescription*> PartList;

		// Engines
		PartList.Empty();
		Catalog->GetRCSList(PartList, EFlarePartSize::S, NULL, NULL);
		Catalog->GetRCSList(PartList, EFlarePartSize::L, NULL, NULL);

		for (FFlareSpacecraftComponentDescription* Part : PartList)
		{
			RCSListData.AddUnique(FInterfaceContainer::New(Part));
		}
		RCSList->RequestListRefresh();
	}

	if (!WeaponListData.Num())
	{
		TArray<FFlareSpacecraftComponentDescription*> PartList;

		// Engines
		PartList.Empty();
		Catalog->GetWeaponList(PartList, EFlarePartSize::S, NULL, NULL);
		Catalog->GetWeaponList(PartList, EFlarePartSize::L, NULL, NULL);

		for (FFlareSpacecraftComponentDescription* Part : PartList)
		{
			WeaponListData.AddUnique(FInterfaceContainer::New(Part));
		}
		WeaponList->RequestListRefresh();
	}

	//	

	if (!SetupResources)
	{
		TArray<UFlareResourceCatalogEntry*> ResourceList = MenuManager->GetGame()->GetResourceCatalog()->Resources;
		TArray<FFlareResourceDescription*> ConsumerResources;
		TArray<FFlareResourceDescription*> MaintenanceResources;
		TArray<FFlareResourceDescription*> RawResources;
		TArray<FFlareResourceDescription*> ManufacturingResources;
		TArray<FFlareResourceDescription*> UncategorizedResources;

		//because there's no useful categorization data by default in HR.
		TArray<FString> HardcodedRawResources;
		TArray<FString> HardcodedManufacturingResources;

		HardcodedRawResources.Reserve(6);
		HardcodedRawResources.Add(TEXT("Helium 3"));
		HardcodedRawResources.Add(TEXT("Dihydrogen"));
		HardcodedRawResources.Add(TEXT("Methane"));
		HardcodedRawResources.Add(TEXT("Iron Oxyde"));
		HardcodedRawResources.Add(TEXT("Silica"));
		HardcodedRawResources.Add(TEXT("Water"));

		HardcodedManufacturingResources.Reserve(3);
		HardcodedManufacturingResources.Add(TEXT("Carbon"));
		HardcodedManufacturingResources.Add(TEXT("Steel"));
		HardcodedManufacturingResources.Add(TEXT("Plastics"));

		for (int ResourceIndex = 0; ResourceIndex < ResourceList.Num(); ResourceIndex++)
		{
			UFlareResourceCatalogEntry* Resource = ResourceList[ResourceIndex];
			if (Resource)
			{
				FFlareResourceDescription* Data = &Resource->Data;
				if (Data)
				{
					if (Data->IsConsumerResource)
					{
						ConsumerResources.AddUnique(Data);
					}
					else if (Data->IsMaintenanceResource)
					{
						MaintenanceResources.AddUnique(Data);
					}
					else if (Data->IsRawResource || HardcodedRawResources.Find(Data->Name.ToString()) != INDEX_NONE)
					{
						RawResources.AddUnique(Data);
					}

					else if (Data->IsManufacturingResource || HardcodedManufacturingResources.Find(Data->Name.ToString()) != INDEX_NONE)
					{
						ManufacturingResources.AddUnique(Data);
					}
					else
					{
						UncategorizedResources.AddUnique(Data);
					}
				}
			}
		}

		SetupResourceDisplay(ResourcesBox, ConsumerResources, FText(LOCTEXT("ConsumerResources", "Consumer Resources")), FText(LOCTEXT("ResourcesConsumerMain", "These resources can be bought by the population, but they may have other uses.")));
		SetupResourceDisplay(ResourcesBox, RawResources, FText(LOCTEXT("RawResources", "Raw Resources")), FText(LOCTEXT("ResourcesRawMaterialsMain", "These resources are produced directly from the environment.")));
		SetupResourceDisplay(ResourcesBox, ManufacturingResources, FText(LOCTEXT("ManufacturingResources", "Manufacturing Resources")), FText(LOCTEXT("ResourcesManufacturingMain", "These resources are produced from raw materials and can be used as construction resources or to build consumer resources.")));
		SetupResourceDisplay(ResourcesBox, MaintenanceResources, FText(LOCTEXT("MaintenanceResources", "Maintenance Resources")), FText(LOCTEXT("ResourcesFleetSupplyMain", "These resources are used to repair or refill spacecrafts.")));
		SetupResourceDisplay(ResourcesBox, UncategorizedResources, FText(LOCTEXT("UncategorizedResources", "Uncategorized Resources")), FText());
		SetupResources = true;
	}
}
void SFlareHelpMenu::SetupResourceDisplay(TSharedPtr<SVerticalBox> HudBox, TArray<FFlareResourceDescription*> ResourcesData, FText CategoryName, FText CategorySubText)
{
	if (ResourcesData.Num() > 0)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		double TextWrappingResourceInfo = 1.25 * Theme.ContentWidth;

		if (!CategoryName.IsEmpty())
		{
			HudBox->AddSlot()
			.AutoHeight()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("ResourceCategoryTitle", "\n{0}"),
					CategoryName))
					.TextStyle(&Theme.NameFontBold)
				]
			];
			if (!CategorySubText.IsEmpty())
			{
				HudBox->AddSlot()
				.AutoHeight()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("ResourceCategorySubText", "\n{0}\n"),
						CategorySubText))
						.TextStyle(&Theme.TextFont)
						.WrapTextAt(TextWrappingResourceInfo)
					]
				];
			}
		}

		for (int ResourceIndex = 0; ResourceIndex < ResourcesData.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* ResourceData = ResourcesData[ResourceIndex];
			HudBox->AddSlot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.AutoWidth()
				.Padding(1,4)
				[
					SNew(SBorder)
					.Padding(FMargin(0))
					.BorderImage(&ResourceData->Icon)
					[
						SNew(SBox)
						.WidthOverride(Theme.ResourceWidth)
						.HeightOverride(Theme.ResourceHeight)
						[
							SNew(SVerticalBox)
							// Resource name
							+ SVerticalBox::Slot()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(ResourceData->Acronym)
							]

							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.SmallFont)
								.Text(FText::Format(LOCTEXT("ResourcePriceInfoInsideBoxCentre", "{0}-{1}"),
								FText::AsNumber(ResourceData->MinPrice / 100),
								FText::AsNumber(ResourceData->MaxPrice / 100)
								))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Bottom)
							.HAlign(HAlign_Right)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.SmallFont)
								.Text(FText::Format(LOCTEXT("ResourcePriceInfoInsideBoxBottom", "{0}"),
								FText::AsNumber(ResourceData->TransportFee / 100)
								))
							]
						]
					]
				]

				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(Theme.ResourceHeight)
					.Padding(FMargin(1,4))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SmallFont)
							.WrapTextAt(TextWrappingResourceInfo)
							.Text(ResourceData->Description)
						]
	
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Bottom)
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SmallFont)
							.Text(FText::Format(LOCTEXT("ResourcePriceInfo", "\n\n\u2022{0} - Price {1}-{2}.CR. Transport fee {3}.CR"),
							ResourceData->Name,
							FText::AsNumber(ResourceData->MinPrice / 100),
							FText::AsNumber(ResourceData->MaxPrice / 100),
							FText::AsNumber(ResourceData->TransportFee / 100)
							))
						]
					]
				]
			];
		}
	}
}



TSharedRef<ITableRow> SFlareHelpMenu::OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable, bool OrderIsShipValue)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FFlareSpacecraftDescription* Desc = Item->SpacecraftDescriptionPtr;

	uint32 Width = 32;

	// Structure
	return SNew(SFlareListItem, OwnerTable)
	.Width(Width)
	.Height(2)
	.Content()
	[
		SNew(SFlareSpaceCraftOverlayInfo)
		.MenuManager(MenuManager)
		.Desc(Desc)
		.VerboseInformation(true)
		.OrderIsShip(OrderIsShipValue)
		.TargetShipyard(NULL)
		.TargetSkirmish(NULL)
		.TargetSector(NULL)
	];
}

TSharedRef<ITableRow> SFlareHelpMenu::OnGenerateCompanyLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{ 
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FFlareCompanyDescription* Desc = Item->CompanyDescriptionPtr;

	uint32 Width = 32;

	// Structure
	return SNew(SFlareListItem, OwnerTable)
		.Width(Width)
		.Height(2)
		.Content()
		[
			SNew(SFlareCompanyHelpInfo)
			.CompanyDescription(Desc)
			.MenuManager(MenuManager)
		];
}

TSharedRef<ITableRow> SFlareHelpMenu::GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable, int32 Width)
{
	TSharedPtr<SFlarePartInfo> Temp;
	TSharedPtr<SFlareListItem> TempWidget;

	// Create the row
	TSharedRef<ITableRow> res = SAssignNew(TempWidget, SFlareListItem, OwnerTable)
	.Width(Width)
	.Height(2)
	.Content()
	[
		SAssignNew(Temp, SFlarePartInfo)
		.Description(Item->PartDescription)
		.Verbose(true)
		.ShowOwnershipInfo(true)
	];
	return res;
}

void SFlareHelpMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

//garbage collector doesn't like reusing the company list. garbage collector likes to have its way with it.
//	CompanyListData.Empty();
//	CompanyList->RequestListRefresh();
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

#undef LOCTEXT_NAMESPACE