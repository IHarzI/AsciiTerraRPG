#pragma once

#include "spNetCommon.h"
#include "Containers/DynamirArray.h"
#include "spNetMath.h"

using namespace spnet::Containers;

const uint32 InvalidID = 0xFFFFFFFF;

struct GameWorld;

const uint32 MaxNameSize = 75;
const uint32 MaxInventorySize = 20;
using CoordVec = matharz::template_vec3<int16>;

struct AttackTarget
{
	bool valid = false;
	std::vector<uint32> playerIDs;
	std::vector<uint32> EnemyIDs;
};

enum class ECustomMetaMSG : uint32
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	ClientAccepted,
	ClientRegisterOnServer,
	ClientRegisterFromServer,
	ClientUpdateMap,
	ClientInitEnemy,
	ClientAddPlayer,
	ClientJustCheck,
	GameRemovePlayer,
	GameUpdatePlayer,
	GamePlayerActionValidated,
	GamePlayerActionAborted,
	GameAITurnAction,
	GamePlayerTurn,
	GameEndPlayerTurn,
	GameNextPlayerTurn,
	ServerGamePlayerActionApprove,
	ServerShyConnectionCheck,
	MessageAll,
	ServerMessage
};

enum ItemType
{
	None,
	Arrow,
	Bow,
	Sword,
	Apple
};

struct Item
{
	FixedString<75> ItemName;
	ItemType type{None};
	// Can be differeent for different items, aka for arrows - quantity, for bow/seord could be like "healtg/max usage, etc"
	uint8 usage{0};
	uint8 Power{0};
};

struct ItemSlot
{
	bool empty{true};
	Item Item{};
	uint8 Mods{};
	uint32 quantity{};
};

const char* PlayerActionsNames[]
{
	"MOVE",
	"ATTACK"
};

enum EPlayerActionType : uint8
{
	Move = 0,
	Attack,
	EndTurn
};

struct PlayerAction
{
	EPlayerActionType ActionType;
	uint32 PlayerID;
	StaticArray<int16, 4> ActionInfo;
};

enum ETileType : uint8
{
	Grass = 0,
	Forest,
	Water,
	Stone,
	Hill,
	MAXCount
};

struct Tile
{
	uint8 Type;
	uint8 Mods;
	char Appereance;
};

const Tile TilesTable[] =
{
	{Grass, 0, ';'},
	{Forest, 0, 'F'},
	{Water, 0, 'v'},
	{Stone, 0, 'o'},
	{Hill, 0, 'M'}
};

const char* TilesNames[] =
{
	"Grass",
	"Forest",
	"Water",
	"Stone"
	"Hill",
};

struct Map
{
	static constexpr uint32 Width = 40;
	static constexpr uint32 Height = 40;
	StaticArray<uint8, Width*Height> MapTiles;
};

const Item ItemTable[] =
{
	{"Master Keiju Sword", Sword, 100, 5},
	{"Villager Bow", Bow, 27, 1 },
	{"Apple", Apple, 3, 1},
	{"Just Arrow", Arrow, 1, 1},
	{"Poisoned Arrow", Arrow, 7, 3},
	{"Iron Sword", Sword, 27, 3}
};

struct Inventory
{
	Inventory(std::initializer_list<Item> initList)
	{
		for (Item item : initList)
		{
			AddItem(item);
		}
	};

	Inventory(std::initializer_list<ItemSlot> initList)
	{
		uint32 i = 0;
		for (ItemSlot item : initList)
		{
			Storage[i++] = item;
		}
	};

	Inventory() {};

	Inventory(Inventory& otherInv) 
	{
		Storage = otherInv.Storage;
	}

	Inventory& operator=(Inventory& Other)
	{
		Storage = Other.Storage;
		return *this;
	}

	Inventory& operator=(Inventory&& Other)
	{
		std::swap(Storage, Other.Storage);
		return *this;
	}

	Inventory(Inventory&& otherInv)
	{
		Storage = otherInv.Storage;
	}

	Inventory(const Inventory& otherInv)
	{
		Storage = otherInv.Storage;
	}

	StaticArray<ItemSlot,MaxInventorySize> Storage;

	void AddItem(Item item) {
		ItemSlot ItemToAdd{ false, item, item.type == Sword || item.type == Bow ? item.usage : (uint8)0, item.type == Sword || item.type == Bow ? (uint8)1 : item.usage };

		for (auto& StoredItem : Storage)
		{
			if (StoredItem.empty)
			{
				StoredItem = ItemToAdd;
				return;
			}
		}
	};
	void RemoveItem(Item item) {
		for (auto& StoredItem: Storage)
		{
			if (std::strcmp((const char*)StoredItem.Item.ItemName.data(), (const char*)item.ItemName.data()) == 0)
			{
				StoredItem = {};
				return;
			};
		}
	};

	uint32 FindItem(ItemType itemType)
	{
		uint32 ItemID = 0;
		for (auto& StoredItem : Storage)
		{
			if (!StoredItem.empty && StoredItem.Item.type == itemType)
			{
				return ItemID;
			}
			ItemID++;
		};

		return InvalidID;
	}

};

struct Player
{
	char Name[MaxNameSize];
	uint32 ID{0};
	int32 HP{0};
	uint32 EquipedItemID{0};
	Inventory Inventory{};
	CoordVec Coords{};
	
	uint8 MaxTurns : 4;
	uint8 CurrentTurns : 4;

	uint8 GetMaxTurnActions() const { return MaxTurns; };
	uint8 GetCurrentTurnActions() const { return CurrentTurns; };
	uint8 SetCurrentTurnActions(uint32 TurnActions) { assert(TurnActions <= 16); return CurrentTurns = TurnActions; };
	uint8 SetMaxTurnActions(uint32 TurnActions) { assert(TurnActions <= 16); return MaxTurns = TurnActions; };

	bool TakeDamage(uint32 Damage) 
	{ 
#ifdef BUILD_CLIENT
		std::cout << "Player [" << (const char*)Name << "] took damage[" << Damage << "] lefted HP[" << HP << "]\n";
#endif // BUILD_CLIENT
		if (Damage >= HP) HP = 0; else HP -= Damage; return HP > 0;
	}
	void MakeAttack(AttackTarget target, GameWorld& gameWorld);

 };

enum EEnemyType : uint8
{
	Zombie = 0,
	Bandit,
	EnemyTypeMaxCount
};

const char* EnemiesNames[]
{
	"Zombie",
	"Bandit"
};

enum EEnemyActionType : uint8
{
	EnemyMove,
	EnemyBaseAttack
};

const char* EnemiesActionsNames[]
{
	"MOVE",
	"BASE ATTACK"
};

struct EnemyAction
{
	EEnemyActionType actionType;
	uint32 EnemyID;
	StaticArray<int16, 4> ActionInfo;
};

struct Enemy
{
	EEnemyType type;
	uint32 faction;
	int32 HP;
	uint32 MaxActions;
	uint32 EquipedItemID{ 0 };
	Inventory inventory;
	CoordVec Coords{};

	bool TakeDamage(uint32 Damage) 
	{ 
#ifdef BUILD_CLIENT
		std::cout << "[" << EnemiesNames[type] << "] took damage[" << Damage << "] lefted HP[" << HP << "]\n";
#endif // BUILD_CLIENT

		if (Damage >= HP) HP = 0; else HP -= Damage; return HP > 0; 
	}

	void MakeAttack(AttackTarget target, GameWorld& gameWorld);
};

const Enemy EnemiesTable[] =
{
	{Zombie,0, 17, 3, 0,{{false, ItemTable[5], 0, 1}}},
	{Bandit,1, 23, 2, 0,{{false, ItemTable[1], 0, 1}, {false, ItemTable[3], 0, 50}}}
};

struct GameWorld
{
	std::unordered_map<uint32, Player> PlayersList{};
	std::vector<Enemy> EnemyList;
	Map GameWorldMap;
	void* OwnerPtr = nullptr;
	// bool IsPlayer, uint32 ID
	bool (*KillCallback)(void*, bool, uint32) = nullptr;

	void MakeApprovedAction(Player& Player, PlayerAction action) 
	{
		switch (action.ActionType)
		{
		case Move:
			Player.Coords += CoordVec{ action.ActionInfo[1], action.ActionInfo[2], action.ActionInfo[3] };
		case Attack:
		{
			auto AttackTargets = PlayerFindAttackTarget(Player, action);
			Player.MakeAttack(AttackTargets, *this);
		}
		default:
			break;
		}
		Player.SetCurrentTurnActions(Player.GetCurrentTurnActions() - 1);
	}
	
	void MakeEnemyAction(Enemy& enemy, EnemyAction action)
	{
		switch (action.actionType)
		{
		case EnemyMove:
		{
			enemy.Coords += CoordVec{ action.ActionInfo[1], action.ActionInfo[2], action.ActionInfo[3] };
			break;
		};
		case EnemyBaseAttack:
		{
			auto AttackTargets = EnemyFindAttackTarget(enemy, action);
			enemy.MakeAttack(AttackTargets, *this);

			break;
		};
		default:
			break;
		};
	
	};

	AttackTarget PlayerFindAttackTarget(Player& player, PlayerAction action)
	{
		CoordVec AttackDirection{ action.ActionInfo[1],action.ActionInfo[2],action.ActionInfo[3] };
		AttackTarget atkTarget;
		for (auto& playerInList : PlayersList)
		{
			if (playerInList.second.ID != player.ID)
			{
				if (playerInList.second.Coords == player.Coords + AttackDirection)
				{
					atkTarget.valid = true;
					atkTarget.playerIDs.push_back(playerInList.second.ID);
				}
			}
		}
		uint32 EnemyID = 0;
		for (auto& Enemy : EnemyList)
		{
			if (Enemy.Coords == player.Coords + AttackDirection)
			{
				atkTarget.valid = true;
				atkTarget.EnemyIDs.push_back(EnemyID);
			};
			EnemyID++;
		}
		return atkTarget;
	};

	AttackTarget EnemyFindAttackTarget(Enemy& enemy, EnemyAction action)
	{
		CoordVec AttackDirection{ action.ActionInfo[1],action.ActionInfo[2],action.ActionInfo[3] };

		AttackTarget atkTarget;
		for (auto& playerInList : PlayersList)
		{
			if (playerInList.second.Coords == enemy.Coords + AttackDirection)
			{
				atkTarget.valid = true;
				atkTarget.playerIDs.push_back(playerInList.second.ID);
			}
		}

		uint32 EnemyID = 0;
		for (auto& Enemy : EnemyList)
		{
			if (Enemy.faction != enemy.faction)
			{
				if (Enemy.Coords == enemy.Coords + AttackDirection)
				{
					atkTarget.valid = true;
					atkTarget.EnemyIDs.push_back(EnemyID);
				};
			};
			EnemyID++;
		}

		return atkTarget;
	};
};

void Enemy::MakeAttack(AttackTarget target, GameWorld& gameWorld)
{
	if (!inventory.Storage[EquipedItemID].empty)
	{
		uint32 AttackDamage = 0;
		switch (inventory.Storage[EquipedItemID].Item.type)
		{
		case Sword:
		{
			AttackDamage += inventory.Storage[EquipedItemID].Item.Power;

			break;
		}
		case Bow:
		{
			uint32  ArrowID = inventory.FindItem(Arrow);
			if (ArrowID != InvalidID)
			{
				AttackDamage += inventory.Storage[ArrowID].Item.Power;
				inventory.Storage[ArrowID].quantity--;
				if (inventory.Storage[ArrowID].quantity == 0)
					inventory.Storage[ArrowID].empty = true;
			}
			break;
		}
		default:
			AttackDamage = 1;
			break;
		};

		for (auto& PlayerID : target.playerIDs)
		{
			gameWorld.PlayersList[PlayerID].TakeDamage(AttackDamage);
			if (gameWorld.KillCallback)
				gameWorld.KillCallback(gameWorld.OwnerPtr, true, PlayerID);
		}

		for (auto& EnemyID : target.EnemyIDs)
		{
			gameWorld.EnemyList[EnemyID].TakeDamage(AttackDamage);
			if (gameWorld.KillCallback)
				gameWorld.KillCallback(gameWorld.OwnerPtr, false, EnemyID);
		}

	}
}

void Player::MakeAttack(AttackTarget target, GameWorld& gameWorld)
{
	if (!Inventory.Storage[EquipedItemID].empty)
	{
		uint32 AttackDamage = 0;
		switch (Inventory.Storage[EquipedItemID].Item.type)
		{
		case Sword:
		{
			AttackDamage += Inventory.Storage[EquipedItemID].Item.Power;

			break;
		}
		case Bow:
		{
			uint32  ArrowID = Inventory.FindItem(Arrow);
			if (ArrowID != InvalidID)
			{
				AttackDamage += Inventory.Storage[ArrowID].Item.Power;
				Inventory.Storage[ArrowID].quantity--;
				if (Inventory.Storage[ArrowID].quantity == 0)
					Inventory.Storage[ArrowID].empty = true;
			}
			break;
		}
		default:
			AttackDamage = 1;
			break;
		};

		for (auto& PlayerID : target.playerIDs)
		{
			gameWorld.PlayersList[PlayerID].TakeDamage(AttackDamage);
			if (gameWorld.KillCallback)
				gameWorld.KillCallback(gameWorld.OwnerPtr, true, PlayerID);
		}

		for (auto& EnemyID : target.EnemyIDs)
		{
			gameWorld.EnemyList[EnemyID].TakeDamage(AttackDamage);
			if (gameWorld.KillCallback)
				gameWorld.KillCallback(gameWorld.OwnerPtr, false, EnemyID);
		}

	}
};