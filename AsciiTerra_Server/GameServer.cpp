#include <iostream>
#include <spiderNet.h>
#include "spNetMETA.h"
#include "Timer.h"

class CustomServer : public spnet::server_interface<ECustomMetaMSG>
{
public:
	CustomServer(uint16_t nPort) : spnet::server_interface<ECustomMetaMSG>(nPort)
	{
		GenerateMap();
		GenerateEnemies();
	}
protected:
	virtual bool OnClientConnect(std::shared_ptr<spnet::connection<ECustomMetaMSG>> client)
	{
		spnet::message<ECustomMetaMSG> msg;
		msg.header.id = ECustomMetaMSG::ServerAccept;
		client->Send(msg);
		return true;
	}

	//will modify playerInfo
	void ClientOnRegister(Player& playerInfo, std::shared_ptr<spnet::connection<ECustomMetaMSG>> client)
	{
		playerInfo.ID = client->GetID();
		bool FoundSpawnPlace = false;

		while (!FoundSpawnPlace)
		{
			CoordVec randVec{
				(short)(rand() % gameWorld.GameWorldMap.Width - 2 + 1),
				(short)(rand() % gameWorld.GameWorldMap.Height - 2 + 1),
				0
			};
			for (auto& enemy : gameWorld.EnemyList)
			{
				if (enemy.Coords == randVec)
					continue;
			}
			for (auto& playerInList : gameWorld.PlayersList)
			{
				if (playerInList.second.Coords == randVec)
					continue;
			}
			FoundSpawnPlace = true;
			playerInfo.Coords = randVec;
		};
		const uint32 BaseTurnActions = 4;
		playerInfo.SetMaxTurnActions(BaseTurnActions);
		playerInfo.SetCurrentTurnActions(BaseTurnActions);

		playerInfo.HP = 45;
		for (uint32 i = 0; i < rand() % 5; i++)
		{
			playerInfo.Inventory.AddItem(ItemTable[rand() % 5]);
		};

	}

	void ChooseNextPlayerForTurn(std::shared_ptr<spnet::connection<ECustomMetaMSG>> client)
	{
		if (!gameWorld.PlayersList.size())
		{
			SERVER_ERROR("No Next players to play!");
			return;
		};

		if (!client)
		{
			CurrentPlayerID = gameWorld.PlayersList.begin()->second.ID;
		}
		else
		{
			auto nextPlayer = gameWorld.PlayersList.find(client->GetID());
			if (++nextPlayer == gameWorld.PlayersList.end())
			{
				MakeAITurn();

				CurrentPlayerID = gameWorld.PlayersList.begin()->second.ID;
			}
			else
			{
				CurrentPlayerID = nextPlayer->second.ID;
			};
		};

		if (client && CurrentPlayerID == client->GetID() && !client->IsConnected())
		{
			SERVER_ERROR("No Next players to play!");
		}

		spnet::message<ECustomMetaMSG> msgNewTurn;
		msgNewTurn.header.id = ECustomMetaMSG::GamePlayerTurn;
		bool StartedNewTurn = false;
		for (auto& clientInDeq : m_deqConnection)
		{
			if (clientInDeq->GetID() == CurrentPlayerID)
			{
				MessageClient(clientInDeq, msgNewTurn);
				StartedNewTurn = true;
				break;
			};
		};

		if (!StartedNewTurn)
			SERVER_ERROR("Can't start new turn. something wrong!");

		gameWorld.PlayersList.at(CurrentPlayerID).SetCurrentTurnActions(gameWorld.PlayersList.at(CurrentPlayerID).GetMaxTurnActions());
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<spnet::connection<ECustomMetaMSG>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";

		if (client)
		{
			if (gameWorld.PlayersList.find(client->GetID()) != gameWorld.PlayersList.end())
			{
				auto& pd = gameWorld.PlayersList[client->GetID()];
				SERVER_MESSAGE("Removing disconnected client!" << " Client id:" << pd.ID << " Name: " << pd.Name);
				if (client->GetID() == CurrentPlayerID)
				{
					ChooseNextPlayerForTurn(client);
				}

				gameWorld.PlayersList.erase(client->GetID());
				GarbageIDs.push_back(client->GetID());
			}
		}

	}

	void MakeAITurn()
	{
		std::vector<EnemyAction> enemyActions;
		uint32 ActiveEnemyID = 0;
		bool EnemyListDirt = false;
		for (auto& Enemy : gameWorld.EnemyList)
		{
			if (Enemy.HP == 0)
			{
				EnemyListDirt = true;
				continue;
			};
			// try to equip any weapon in inventory, if an empty handed
			if (Enemy.EquipedItemID == InvalidID || Enemy.inventory.Storage[Enemy.EquipedItemID].empty)
			{
				auto SwordID = Enemy.inventory.FindItem(Sword);
				auto BowID = Enemy.inventory.FindItem(Bow);
				auto ArrowID = Enemy.inventory.FindItem(Arrow);

				Enemy.EquipedItemID = SwordID != InvalidID ? SwordID : BowID != InvalidID ? ArrowID != InvalidID ? BowID : InvalidID : InvalidID;
			}

			uint32 BestAttackCandidate = InvalidID;
			int16 DistanceToTarget = gameWorld.GameWorldMap.Width * gameWorld.GameWorldMap.Height;
			CoordVec TargetCoords{};
			bool IsItPlayer = false;
			for (auto& Player : gameWorld.PlayersList)
			{
				auto Distance = (Player.second.Coords - Enemy.Coords).Lenght();
				if (Distance < DistanceToTarget)
				{
					DistanceToTarget = Distance;
					IsItPlayer = true;
					BestAttackCandidate = Player.second.ID;
					TargetCoords = Player.second.Coords;
				}
			}

			if (!IsItPlayer || DistanceToTarget > 5)
			{
				uint32 OtherEnemyID = 0;
				for (auto& OtherEnemy : gameWorld.EnemyList)
				{
					if (Enemy.faction != OtherEnemy.faction)
					{
						auto Distance = (OtherEnemy.Coords - Enemy.Coords).Lenght();
						if (Distance < DistanceToTarget)
						{
							DistanceToTarget = Distance;
							IsItPlayer = false;

							BestAttackCandidate = OtherEnemyID;
							TargetCoords = OtherEnemy.Coords;
						};
					}
					OtherEnemyID++;
				};
			}

			uint32 LeftedAction = Enemy.MaxActions;

			while (LeftedAction != 0)
			{
				EnemyAction action{};
				action.EnemyID = ActiveEnemyID;
				if (BestAttackCandidate != InvalidID)
				{
					DistanceToTarget = (TargetCoords - Enemy.Coords).Lenght();
					auto xDisplacement = TargetCoords.x - Enemy.Coords.x;
					auto yDisplacement = TargetCoords.y - Enemy.Coords.y;
					// Move to Target
					if (DistanceToTarget > 1)
					{
						if (matharz::abs(xDisplacement))
						{
							action.actionType = EnemyMove;
							action.EnemyID = ActiveEnemyID;
							action.ActionInfo[1] = 1 * xDisplacement > 0 ? 1 : -1;
							TargetCoords -= action.ActionInfo[1];
							gameWorld.MakeEnemyAction(Enemy, action);
							enemyActions.push_back(action);
							LeftedAction--;
							continue;
						}
						else if (matharz::abs(yDisplacement))
						{
							action.actionType = EnemyMove;
							action.EnemyID = ActiveEnemyID;
							action.ActionInfo[2] = 1 * yDisplacement > 0 ? 1 : -1;
							TargetCoords -= action.ActionInfo[2];
							gameWorld.MakeEnemyAction(Enemy, action);
							enemyActions.push_back(action);
							LeftedAction--;
							continue;
						}
						else
						{
							// Z component, no actions for now
							// as there are no "levels of ground layer" on map yet
						}
					}
					// Make Attack
					else
					{
						action.actionType = EnemyBaseAttack;
						action.EnemyID = ActiveEnemyID;
						action.ActionInfo[1] = xDisplacement;
						action.ActionInfo[2] = yDisplacement;
						gameWorld.MakeEnemyAction(Enemy, action);
						enemyActions.push_back(action);
						LeftedAction--;

					}
				}
				else
				{
					// Nothing to do, no more players/enemies to attack
					// will procrastinate...
					break;
				}
			}

			ActiveEnemyID++;		
		}
		
		// Send all ai actions to clients
		{
			uint32 actionsCount = enemyActions.size();
			for (auto& action : enemyActions)
			{
				spnet::message<ECustomMetaMSG> msg;
				msg.header.id = ECustomMetaMSG::GameAITurnAction;
				msg << action;
				MessageAllClients(msg);
			}
		};

		if (EnemyListDirt)
		{
			auto oldEnemyList = gameWorld.EnemyList;
			gameWorld.EnemyList.clear();
			for (auto& Enemy : oldEnemyList)
			{
				if (Enemy.HP > 0)
					gameWorld.EnemyList.push_back(Enemy);
			}
		};

	}

	void ShyCheckAllPlayers()
	{
		spnet::message<ECustomMetaMSG> msg;
		msg.header.id = ECustomMetaMSG::ServerShyConnectionCheck;

		MessageAllClients(msg);
	};

	bool CheckPlayerAction(PlayerAction action)
	{
		if (gameWorld.PlayersList.at(action.PlayerID).HP <= 0)
			return false;

		switch (action.ActionType)
		{
			case Attack:
			{
				auto target = gameWorld.PlayerFindAttackTarget(gameWorld.PlayersList.at(action.PlayerID), action);
				return target.valid;
				break;
			}
			case Move:
			{
				auto& Player = gameWorld.PlayersList.at(action.PlayerID);
				auto NewCoords = Player.Coords + CoordVec{ action.ActionInfo[1],action.ActionInfo[2],action.ActionInfo[3] };
				if (NewCoords.x < 0 || NewCoords.x > gameWorld.GameWorldMap.Width ||
					NewCoords.y < 0 || NewCoords.y > gameWorld.GameWorldMap.Height ||
					gameWorld.GameWorldMap.MapTiles[NewCoords.x + NewCoords.y * gameWorld.GameWorldMap.Width] == Hill)
					return false;
				return true;
				break;
			}
			default:
				return false;
				break;
		}
	}

public:

	void Run()
	{
		tt_Timer Timer;
		auto timeStart = Timer.Count();
		while (true)
		{
			if (tt_calcDuration(timeStart, Timer.GetTimeNowHRC()) > 1000)
			{
				ShyCheckAllPlayers();
				Timer.Stop();
				timeStart = Timer.Count();
			};

			Update(-1, true);
		}
	}

protected:

	// Gameplay
	void GenerateMap()
	{
		for (uint32 height = 0; height < gameWorld.GameWorldMap.Height; height++)
		{
			for (uint32 width = 0; width < gameWorld.GameWorldMap.Width; width++)
			{
				auto& Tile = gameWorld.GameWorldMap.MapTiles[width + height * gameWorld.GameWorldMap.Width];
				if (width == 0 || width == gameWorld.GameWorldMap.Width - 1 || height == 0 || height == gameWorld.GameWorldMap.Height-1)
				{
					Tile = ETileType::Hill;
					continue;
				}

				Tile = rand() % Hill;

			}
		}
	}

	void GenerateEnemies()
	{
		const uint32 EnemiesCount = rand() % 25;
		for (uint32 i = 0; i < EnemiesCount; i++)
		{
			gameWorld.EnemyList.push_back(EnemiesTable[rand() % EnemyTypeMaxCount]);
			gameWorld.EnemyList.back().Coords = CoordVec{ (int16)(rand() % 25 + 1), (int16)(rand() % 25 + 1), 0 };
		}
	}

	// -----

public:

	void OnClientValidated(std::shared_ptr<spnet::connection<ECustomMetaMSG>> client) override
	{
		// Client passed validation check, so send them a message informing
		// them they can continue to communicate
		spnet::message<ECustomMetaMSG> msg;
		msg.header.id = ECustomMetaMSG::ClientAccepted;
		client->Send(msg);
	}
protected:
	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<spnet::connection<ECustomMetaMSG>> client, spnet::message<ECustomMetaMSG>& msg)
	{
		if (!GarbageIDs.empty())
		{
			for (auto pid : GarbageIDs)
			{
				spnet::message<ECustomMetaMSG> m;
				m.header.id = ECustomMetaMSG::GameRemovePlayer;
				m << pid;
				SERVER_MESSAGE("Removing disconnected client from other clients!" << " Client id:" << pid);
				MessageAllClients(m);
				
			}
			GarbageIDs.clear();
		}

		switch (msg.header.id)
		{
			case ECustomMetaMSG::ClientRegisterOnServer:
			{
				Player playerDesc{};
				msg >> playerDesc;
				ClientOnRegister(playerDesc, client);

				gameWorld.PlayersList.insert_or_assign(playerDesc.ID, playerDesc);

				spnet::message<ECustomMetaMSG> msgSendID;
				msgSendID.header.id = ECustomMetaMSG::ClientRegisterFromServer;
				msgSendID << playerDesc;

				spnet::message<ECustomMetaMSG> msgInitClientMap;
				msgInitClientMap.header.id = ECustomMetaMSG::ClientUpdateMap;
				msgInitClientMap << gameWorld.GameWorldMap;

				MessageClient(client, msgSendID);
				MessageClient(client, msgInitClientMap);
				uint32 EnemiesCount = 0;
				EnemiesCount = gameWorld.EnemyList.size();
				for (uint32 i = 0; i < EnemiesCount; i++)
				{
					spnet::message<ECustomMetaMSG> msgInitEnemies;
					msgInitEnemies.header.id = ECustomMetaMSG::ClientInitEnemy;
					msgInitEnemies << gameWorld.EnemyList[i];
					MessageClient(client, msgInitEnemies);
				} 

				spnet::message<ECustomMetaMSG> msgAddPlayer;
				msgAddPlayer.header.id = ECustomMetaMSG::ClientAddPlayer;
				msgAddPlayer << playerDesc;
				MessageAllClients(msgAddPlayer, client);

				for (const auto& player : gameWorld.PlayersList)
				{
					if(player.second.ID == playerDesc.ID)
					{
						continue;
					}
					spnet::message<ECustomMetaMSG> msgAddOtherPlayers;
					msgAddOtherPlayers.header.id = ECustomMetaMSG::ClientAddPlayer;
					msgAddOtherPlayers << player.second;
					MessageClient(client, msgAddOtherPlayers);
				}

				if (CurrentPlayerID == 0)
				{
					ChooseNextPlayerForTurn(nullptr);
				}

				break;
			}
			break;

			case ECustomMetaMSG::ClientJustCheck:
				break;

			case ECustomMetaMSG::ServerGamePlayerActionApprove:
			{
				PlayerAction action;
				msg >> action;
				if (client->GetID() == CurrentPlayerID)
				{
					auto ClientPlayer = gameWorld.PlayersList.find(client->GetID());
					if (ClientPlayer != gameWorld.PlayersList.end())
					{
						uint32 CurrentActions = ClientPlayer->second.GetCurrentTurnActions();
						if (CurrentActions > 0)
						{
							if (CheckPlayerAction(action))
							{
								gameWorld.MakeApprovedAction(ClientPlayer->second, action);
								action.PlayerID = client->GetID();
								spnet::message<ECustomMetaMSG> msgForOtherPlayers;
								msgForOtherPlayers.header.id = ECustomMetaMSG::GamePlayerActionValidated;
								msgForOtherPlayers << action;

								MessageAllClients(msgForOtherPlayers, nullptr);

								if (ClientPlayer->second.GetCurrentTurnActions() <= 0)
								{
									spnet::message<ECustomMetaMSG> msgEndTurn;
									msgEndTurn.header.id = ECustomMetaMSG::GameEndPlayerTurn;
									MessageClient(client, msgEndTurn);
									ChooseNextPlayerForTurn(client);

									spnet::message<ECustomMetaMSG> msgNewPlayerTurn;
									msgNewPlayerTurn.header.id = ECustomMetaMSG::GameNextPlayerTurn;
									msgNewPlayerTurn << CurrentPlayerID;
									MessageAllClients(msgNewPlayerTurn, nullptr);
								}

								return;
							};
						};
					};
				};

				spnet::message<ECustomMetaMSG> abortedAction;
				abortedAction.header.id = ECustomMetaMSG::GamePlayerActionAborted;
				abortedAction << action;
				MessageClient(client, abortedAction);
				break;
			}

			case ECustomMetaMSG::GameUpdatePlayer:
			{
				// Simply bounce update to everyone except incoming client
				MessageAllClients(msg, client);
				break;
			}

			case ECustomMetaMSG::ServerPing:
			{
				std::cout << "[" << client->GetID() << "]: Server Ping\n";

				// Simply bounce message back to client
				client->Send(msg);
			}
				break;
			case ECustomMetaMSG::MessageAll:
			{
				std::cout << "[" << gameWorld.PlayersList.at(client->GetID()).Name << "]: Message All\n";

				// Construct a new message and send it to all clients
				spnet::message<ECustomMetaMSG> msg;
				msg.header.id = ECustomMetaMSG::ServerMessage;
				msg << client->GetID();
				MessageAllClients(msg, client);
			}
				break;
		}
	}

private:
	GameWorld gameWorld;
	std::vector<uint32> GarbageIDs;
	uint32 CurrentPlayerID = 0;
};

int main()
{
	CustomServer server(60000);
	server.Start();

	server.Run();

	return 0;
}