#include <iostream>
#include <spiderNet.h>
#include "spNetMETA.h"
#include "Timer.h"

using namespace spnet::Containers;

static std::string repeat(unsigned times, char c) {
	std::string result;
	for (; times > 0; --times)
		result += c;

	return result;
}

std::string ActionToString(Player& player, uint32 ClientID, PlayerAction action)
{
	std::string ActionSTR;

	char MsgBuff[150];
	if (player.ID == ClientID)
	{
		ActionSTR.append("You");
	}
	else
	{
		ActionSTR.append((const char*)player.Name);
	};

	switch (action.ActionType)
	{
	case Attack:
		ActionSTR.append("attacked.");
		break;
	case Move:
		ActionSTR.append(" moved ");
		ActionSTR.append(action.ActionInfo[1] != 0 ?
			action.ActionInfo[1] > 0 ? " south" : " north" :
			action.ActionInfo[2] != 0 ? action.ActionInfo[2] > 0 ? " east" : " west" :
			action.ActionInfo[2] > 0 ? " uphill" : " down");
		break;
	default:
		break;
	};
	ActionSTR.append(1, '\n');
	return ActionSTR;
}

class CustomClient : public spnet::client_interface<ECustomMetaMSG>
{
private:
	GameWorld gameWorld;
	uint32 PlayerID = 0;
	bool bWaitingForConnection = true;

	bool IsEnemyListDirt = false;
	bool IsPlayerDead = false;
	bool key[11] = {false};
	bool old_key[11] = {false};
public:
	// META Server
	void PingServer()
	{
		spnet::message<ECustomMetaMSG> msg;
		msg.header.id = ECustomMetaMSG::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	bool ClientKillCallback(bool IsPlayer, uint32 ID)
	{
		if (IsPlayer)
		{
			if (ID == PlayerID)
				IsPlayerDead = true;
		}
		else
		{
			IsEnemyListDirt = true;
		}
		return true;
	}

	static bool KillCallback(void* OwnerPtr, bool IsPlayer, uint32 id)
	{
		CustomClient* owner = reinterpret_cast<CustomClient*>(OwnerPtr);
		return owner->ClientKillCallback(IsPlayer, id);
	};

	void MessageAll()
	{
		spnet::message<ECustomMetaMSG> msg;
		msg.header.id = ECustomMetaMSG::MessageAll;
		Send(msg);
	}
	// -----
	// Gameplay 
	
	void MakeActionApprove(PlayerAction action)
	{
		spnet::message<ECustomMetaMSG> msg;
		switch (action.ActionType)
		{
		case Move:
			msg.header.id = ECustomMetaMSG::ServerGamePlayerActionApprove;
			msg << action;
			Send(msg);
			break;
		case Attack:

			break;
		default:
			break;
		}
	}

	// ----
	// UI 
	void ShowInventory()
	{
		std::cout << "\n---------Inventory---------\n";
		const uint32 MaxCellWidth = 30;
		const char EmptySpace = '`';
		const char Border = '|';
		std::string charBuff;
		charBuff.reserve(20 * MaxCellWidth + 200);
		auto printInventoryRow = [MaxCellWidth, EmptySpace, Border](uint32 row, Inventory& inv, std::string& charBuff, uint32 offset)-> void
		{
			if (row > 4)
				return;
			for (uint32 i = 0; i < 5; i++)
			{
				uint32 CellOffset = offset + i * MaxCellWidth;

				auto& Item = inv.Storage[i + row * 5];
				std::string cellStr{};
				if (i == 0)
					cellStr.append(1, Border);

				if (Item.empty)
				{
					cellStr.append(repeat(MaxCellWidth - 2, EmptySpace));
					cellStr.append(1, Border);
					charBuff.append(cellStr);
					continue;
				}

				uint32 itemNameLen = std::strlen((const char*)Item.Item.ItemName.data());
				uint32 itemFillingEmptyThreshold = itemNameLen > MaxCellWidth - 2 ? 0xFFFFFFFF : ((MaxCellWidth - 2) - itemNameLen) / 2;
				if (itemFillingEmptyThreshold == 0xFFFFFFFF)
				{
					charBuff.append((const char*)Item.Item.ItemName.data(), MaxCellWidth - 2);
					charBuff.append(1, Border);
				}
				else
				{
					cellStr.append(repeat(itemFillingEmptyThreshold, EmptySpace));
					cellStr.append((const char*)Item.Item.ItemName.data());
					cellStr.append(repeat(itemFillingEmptyThreshold, EmptySpace));
					cellStr.append(1, Border);
					charBuff.append(cellStr);
				}
			};
			charBuff.append(1, '\n');
		};
		for (uint32 i = 0; i < 4; i++)
		{
			printInventoryRow(i, gameWorld.PlayersList.at(PlayerID).Inventory, charBuff, i * (MaxCellWidth * 5 + 1));
		};
		charBuff[4 * 5 * MaxCellWidth + 1] = '\0';
		charBuff.append(MaxCellWidth * 5, EmptySpace);
		std::cout << charBuff;
		std::cout << "\n------------------------------------\n";
	}

	void ShowPlayers()
	{
		std::cout << "\n---------PLAYERS TABLE---------\n";
		std::string PrintString;
		for (auto playerInList : gameWorld.PlayersList)
		{
			char Buffer[500];
			if (playerInList.second.ID == PlayerID)
			{
				PrintString.append("[YOU]");
			};
				sprintf(Buffer, " Player Stats: Name{%s} ID{%i} Health{%i} Coords{%i %i %i} \0", playerInList.second.Name,
					playerInList.second.ID, playerInList.second.HP, playerInList.second.Coords.x, playerInList.second.Coords.y, playerInList.second.Coords.z);

				PrintString.append(Buffer);
			
			PrintString.append("\n");
		}
		std::cout << PrintString;
		std::cout << "\n------------------------------\n";
	}

	void ShowMap()
	{
		std::cout << "\n---------Map---------\n";
		std::string PrintString;
		
		{
			PrintString.append("Map legend: \n");
			std::string GuideMapStr;
			GuideMapStr.reserve(500);
			uint32 Offset = 0;
			for (uint32 i = 0; i < ETileType::MAXCount; i++)
			{
				char GuideCharsBuff[200];
				const uint32 spaceForTyleType = 15;
				sprintf(GuideCharsBuff, "Tile type: %s %s| Tyle appereance: [%c]\n", TilesNames[i], repeat(spaceForTyleType - strlen(TilesNames[i]), ' ').c_str(), TilesTable[i].Appereance);
				GuideMapStr.append(GuideCharsBuff);
			}
			PrintString.append(GuideMapStr);
			PrintString.append("Character apperance on map - [C]\n");
			PrintString.append("Enemy apperance on map - [Q]\n");
			PrintString.append("----------\n");
		}

		for (int8 height = 0; height < gameWorld.GameWorldMap.Height; height++)
		{
			StaticArray<uint32, 99> PlayersOnLine{};
			StaticArray<uint32, 99> EnemiesOnLine{};
			uint32 EnemiesOnLineCount = 0;
			uint32 PlayersOnLineCount = 0;
			for (int8 width = 0; width < gameWorld.GameWorldMap.Width; width++)
			{
				bool IsCharacterOnTile = false;
				bool IsEnemyOnTile = false;
				for (auto& PlayerInList : gameWorld.PlayersList)
				{
					if (height == PlayerInList.second.Coords.y && width == PlayerInList.second.Coords.x)
					{
						PrintString.append(1, 'C');
						IsCharacterOnTile = true;
						PlayersOnLine[PlayersOnLineCount] = PlayerInList.second.ID;
						PlayersOnLineCount++;
					}
				}
				uint32 EnemiListID = 0;
				for (auto& EnemyInList : gameWorld.EnemyList)
				{
					if (height == EnemyInList.Coords.y && width == EnemyInList.Coords.x)
					{
						PrintString.append(1, 'Q');
						IsEnemyOnTile = true;
						EnemiesOnLine[PlayersOnLineCount] = EnemiListID;
						EnemiesOnLineCount++;
					}
					EnemiListID++;
				}

				if(!IsCharacterOnTile && !IsEnemyOnTile)
				{
					uint8 Tile = gameWorld.GameWorldMap.MapTiles[width + height * gameWorld.GameWorldMap.Width];
					PrintString.append(1, TilesTable[Tile].Appereance);
				};
			}

			if (PlayersOnLineCount)
			{
				for (uint32 i = 0; i < PlayersOnLineCount; i++)
				{
					PrintString.append(repeat(gameWorld.PlayersList.at(PlayersOnLine[i]).Coords.x, '-'));
					if (PlayersOnLine[i] == PlayerID)
						PrintString.append(" Here is your Character ");
					else
					{
						PrintString.append(gameWorld.PlayersList.at(PlayersOnLine[i]).Name);
						PrintString.append(" character ");
					};
				};
			}

			if (EnemiesOnLineCount)
			{
				for (uint32 i = 0; i < EnemiesOnLineCount; i++)
				{
					PrintString.append(repeat(gameWorld.EnemyList.at(EnemiesOnLine[i]).Coords.x, '-'));
					{
						PrintString.append(EnemiesNames[gameWorld.EnemyList.at(EnemiesOnLine[i]).type]);
					};
				};
			}
			PrintString.append(1, '\n');
		};

		std::cout << PrintString;
		std::cout << "\n------------------------------\n";
	}

	// -----

	// Net Machinery
	void CreatePlayer()
	{

		if (!Connect("127.0.0.1", 60000))
			CLIENT_ERROR("Can't connect to server!");

		gameWorld.KillCallback = &KillCallback;
		gameWorld.OwnerPtr = this;

	}

	void ParseCommand(std::string Command)
	{
		if (Command == "ShowInventory")
		{
			CLIENT_MESSAGE("Your inventory: "); ShowPlayers();
		}
		else if(Command == "ShowPlayer")
		{
			CLIENT_MESSAGE("Current players:"); ShowInventory();
		};
	}

	void UpdatePlayer()
	{
		if (IsConnected())
		{

			if (GetForegroundWindow() == GetConsoleWindow())
			{
				key[0] = GetAsyncKeyState('1') & 0x8000;
				key[1] = GetAsyncKeyState('2') & 0x8000;
				key[2] = GetAsyncKeyState('3') & 0x8000;
				key[3] = GetAsyncKeyState('I') & 0x8000;
				key[4] = GetAsyncKeyState('P') & 0x8000;

				key[5] = GetAsyncKeyState('W') & 0x8000;
				key[6] = GetAsyncKeyState('S') & 0x8000;
				key[7] = GetAsyncKeyState('A') & 0x8000;
				key[8] = GetAsyncKeyState('D') & 0x8000;

				key[9] = GetAsyncKeyState('?') & 0x8000;
				key[10] = GetAsyncKeyState('M') & 0x8000;
			}

			if (key[0] && !old_key[0]) { PingServer(); CLIENT_MESSAGE("PingButton pressed");		};
			if (key[1] && !old_key[1]) { MessageAll(); CLIENT_MESSAGE("Sending message to all other clients"); }
			if (key[2] && !old_key[2]) { Disconnect(); CLIENT_MESSAGE("ExitButton pressed");		};
			if (key[3] && !old_key[3]) { CLIENT_MESSAGE("Your inventory: "); ShowInventory();		};
			if (key[4] && !old_key[4]) { CLIENT_MESSAGE("Current players:"); ShowPlayers();			};
			if (key[10] && !old_key[10]) { CLIENT_MESSAGE("Map: "); ShowMap(); };
			if (key[9] && !old_key[9]) { std::string Command; std::cin >> Command; ParseCommand(Command);};


			if (key[5] && !old_key[5]) MakeActionApprove({ Move, PlayerID, {0,0,-1,0} });
			if (key[6] && !old_key[6]) MakeActionApprove({ Move, PlayerID, {0,0,1,0} });
			if (key[7] && !old_key[7]) MakeActionApprove({ Move, PlayerID, {0,-1,0,0} });
			if (key[8] && !old_key[8]) MakeActionApprove({ Move, PlayerID, {0,1,0,0} });

			for (int i = 0; i < 10; i++) old_key[i] = key[i];

			if (IsConnected())
			{
				spnet::message<ECustomMetaMSG> msg;
				msg.header.id = ECustomMetaMSG::ClientJustCheck;
				Send(msg);

				if (!Incoming().empty())
				{

					auto msg = Incoming().pop_front().msg;

					switch (msg.header.id)
					{
						case ECustomMetaMSG::ServerAccept:
						{
							// Server has responded to a ping request				
							std::cout << "Server Accepted Connection\n";
							break;
						}

						case(ECustomMetaMSG::ClientAccepted):
						{
							std::cout << "Server accepted client - you're in!\n";
							std::cout << "Write your name: \n";
							std::string Name{};
							std::cin >> Name;
							Player player{};
							std::memcpy(player.Name, Name.c_str(), MaxNameSize);
							spnet::message<ECustomMetaMSG> msg;
							msg.header.id = ECustomMetaMSG::ClientRegisterOnServer;
							msg << player;
							Send(msg);
							break;
						}

						case (ECustomMetaMSG::ClientRegisterFromServer):
						{
							Player RegisteredPlayer{};
							msg >> RegisteredPlayer;
							std::cout << "Assigned Client ID = " << RegisteredPlayer.ID << "\n";
							PlayerID = RegisteredPlayer.ID;
							gameWorld.PlayersList.insert_or_assign(RegisteredPlayer.ID, RegisteredPlayer);
							std::cout << "Server registered client. Name: " << gameWorld.PlayersList.at(PlayerID).Name << " ID: " << gameWorld.PlayersList.at(PlayerID).ID << '\n';
							break;
						}

						case (ECustomMetaMSG::ClientUpdateMap):
						{
							msg >> gameWorld.GameWorldMap;
							
							break;
						}
						case (ECustomMetaMSG::ClientInitEnemy):
						{
							Enemy enemy;
							msg >> enemy;
							gameWorld.EnemyList.emplace_back(std::move(enemy));
						
							break;
						}

						case(ECustomMetaMSG::ClientAddPlayer):
						{
							Player desc;
							msg >> desc;
							gameWorld.PlayersList.insert_or_assign(desc.ID, desc);

							break;
						}

						case(ECustomMetaMSG::GameRemovePlayer):
						{
							uint32_t nRemovalID = 0;
							msg >> nRemovalID;
							gameWorld.PlayersList.erase(nRemovalID);
							break;
						}

						case (ECustomMetaMSG::GamePlayerTurn):
						{
							auto& currentPlayer = gameWorld.PlayersList[PlayerID];
							currentPlayer.SetCurrentTurnActions(currentPlayer.GetMaxTurnActions());
							CLIENT_MESSAGE("Your TURN!");
							break;
						}

						case (ECustomMetaMSG::GameEndPlayerTurn):
						{
							CLIENT_MESSAGE("End of your TURN!");
							break;
						}

						case (ECustomMetaMSG::GameNextPlayerTurn):
						{
							uint32 NextPlayerID{};
							msg >> NextPlayerID;
							if (NextPlayerID == PlayerID)
								break;

							std::string NextPlayerTurnSTR{};
							NextPlayerTurnSTR.append(1, '[');
							NextPlayerTurnSTR.append((const char*)gameWorld.PlayersList.at(NextPlayerID).Name);
							NextPlayerTurnSTR.append("] TURN");
							CLIENT_MESSAGE(NextPlayerTurnSTR);

							break;
						}

						case(ECustomMetaMSG::GameUpdatePlayer):
						{
							Player desc;
							msg >> desc;
							gameWorld.PlayersList.insert_or_assign(desc.ID, desc);
							break;
						}

						case(ECustomMetaMSG::GamePlayerActionValidated):
						{
							PlayerAction action;
							msg >> action;

							auto player = gameWorld.PlayersList.find(action.PlayerID);
							if (player == gameWorld.PlayersList.end())
							{
								CLIENT_ERROR("Missed Player action!");
								return;
							}

							gameWorld.MakeApprovedAction(player->second, action);

							std::cout << ActionToString(player->second, PlayerID, action);

							char MsgBuff[150];
							if(player->second.ID == PlayerID)
								sprintf(MsgBuff, "You made action, left actions: %i \0", player->second.GetCurrentTurnActions());
							else
								sprintf(MsgBuff, "%s maded action, left actions: %i \0", player->second.Name, player->second.GetCurrentTurnActions());

							CLIENT_MESSAGE((const char*)MsgBuff);
							break;
						}

						case(ECustomMetaMSG::GamePlayerActionAborted):
						{
							PlayerAction action;
							msg >> action;
							char ActionBuff[100];
							sprintf(ActionBuff, "Action {%s, %i, %i, %i, %i} Aborted\0", PlayerActionsNames[action.ActionType],
								action.ActionInfo[0], action.ActionInfo[1], action.ActionInfo[2], action.ActionInfo[3]);
							CLIENT_MESSAGE((const char*)ActionBuff);
							break;
						}

						case ECustomMetaMSG::ServerDeny:
						{
							// Server has responded to a ping request				
							std::cout << "Server Deny Connection\n";
						}
						break;

						case ECustomMetaMSG::ServerPing:
						{
							// Server has responded to a ping request
							std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
							std::chrono::system_clock::time_point timeThen;
							msg >> timeThen;
							std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
						}
						break;

						case ECustomMetaMSG::ServerMessage:
						{
							// Server has responded to a ping request	
							uint32_t clientID;
							msg >> clientID;
							std::cout << "Hello from [" << gameWorld.PlayersList.find(clientID)->second.Name << "]\n";
						}
						break;

						case ECustomMetaMSG::GameAITurnAction:
						{
							EnemyAction action;
							msg >> action;
							std::string ActionSTR{};
							ActionSTR.append("{ENEMY ACTION] {");
							ActionSTR.append(EnemiesNames[gameWorld.EnemyList[action.EnemyID].type]);
							ActionSTR.append("] [");
							ActionSTR.append(EnemiesActionsNames[action.actionType]);
							if (action.actionType == EnemyMove || action.actionType == EnemyBaseAttack)
							{
								ActionSTR.append(" to :");
								char buff[150];
								sprintf(buff, "[%i %i %i]\0", action.ActionInfo[1], action.ActionInfo[2], action.ActionInfo[3]);
								ActionSTR.append((const char*)buff);
							}
							ActionSTR.append(1, '\n');
							std::cout << ActionSTR;
							gameWorld.MakeEnemyAction(gameWorld.EnemyList.at(action.EnemyID), action);
							break;
						}
					}
				}
			}
		};
	};
	// ------
	public:
	void run()
	{
		CreatePlayer();
		
		tt_Timer Timer;
		auto timeStart = Timer.Count();
		srand(timeStart.time_since_epoch().count());
		while (IsConnected())
		{
			if (tt_calcDuration(timeStart, Timer.GetTimeNowHRC()) > 100)
			{
				UpdatePlayer();
				Timer.Stop();
				timeStart = Timer.Count();
			};
		};
	}

};

int main()
{
	CustomClient c;
	c.run();

	return 0;
}
