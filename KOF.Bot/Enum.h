#pragma once

enum PacketHeader
{
	READY,
	LOGIN,
	CONFIGURATION,
	POINTER,
	INJECTION,
	PING,
	CAPTCHA,
	UPDATE,
	ROUTE
};

enum LoginType
{
	GENERIC,
	TOKEN,
};

enum PlatformType
{
	USKO,
	CNKO,
	JPKO,
	KOKO,
	STKO,
};

enum ConfigurationRequestType
{
	LOAD,
	SAVE,
};

enum InjectionRequestType
{
	REQUEST,
	REPORT,
};

enum Pipe
{
	PIPE_LOAD_POINTER = 1,
	PIPE_BASIC_ATTACK,
	PIPE_USE_SKILL,
	PIPE_SEND_PACKET,
	PIPE_LOGIN,
	PIPE_PUSH_PHASE,
	PIPE_SELECT_CHARACTER_SKIP,
	PIPE_SELECT_CHARACTER_LEFT,
	PIPE_SELECT_CHARACTER_RIGHT,
	PIPE_SELECT_CHARACTER_ENTER,
	PIPE_LOAD_SERVER_LIST,
	PIPE_SELECT_SERVER,
	PIPE_SHOW_CHANNEL,
	PIPE_SELECT_CHANNEL,
	PIPE_CONNECT_SERVER,
	PIPE_SAVE_CPU,
	PIPE_PROXY,
	PIPE_VIP_GET_OUT,
};

enum StateMove
{
	PSM_STOP = 0,
	PSM_WALK,
	PSM_RUN,
	PSM_WALK_BACKWARD,
	PSM_COUNT
};

enum StateAction
{
	PSA_BASIC = 0,
	PSA_ATTACK,
	PSA_GUARD,
	PSA_STRUCK,
	PSA_DYING,
	PSA_DEATH,
	PSA_SPELLMAGIC,
	PSA_SITDOWN,
	PSA_COUNT
};

enum AttackResult
{
	ATTACK_FAIL = 0,
	ATTACK_SUCCESS = 1,
	ATTACK_TARGET_DEAD = 2,
	ATTACK_TARGET_DEAD_OK = 3,
	MAGIC_ATTACK_TARGET_DEAD = 4
};

enum SkillTargetType
{
	TARGET_SELF = 1,
	TARGET_FRIEND_WITHME = 2,
	TARGET_FRIEND_ONLY = 3,
	TARGET_PARTY = 4,
	TARGET_NPC_ONLY = 5,
	TARGET_PARTY_ALL = 6,
	TARGET_ENEMY_ONLY = 7,
	TARGET_ALL = 8,
	TARGET_AREA_ENEMY = 10,
	TARGET_AREA_FRIEND = 11,
	TARGET_AREA_ALL = 12,
	TARGET_AREA = 13,
	TARGET_DEAD_FRIEND_ONLY = 25,
	TARGET_UNKNOWN = 0xffffffff
};

enum LoginError
{
	AUTH_SUCCESS = 1,
	AUTH_NOT_FOUND = 2,
	AUTH_INVALID = 3,
	AUTH_BANNED = 4,
	AUTH_IN_GAME = 5,
	AUTH_ERROR = 6,
	AUTH_AGREEMENT = 15,
	AUTH_OTP = 16,
	AUTH_FAILED = 255
};

enum PlayerType
{
	PLAYER_BASE = 0, PLAYER_NPC = 1, PLAYER_OTHER = 2, PLAYER_MYSELF = 3
};

enum Race
{
	RACE_ALL = 0,
	RACE_KA_ARKTUAREK = 1, RACE_KA_TUAREK = 2, RACE_KA_WRINKLETUAREK = 3, RACE_KA_PURITUAREK = 4,
	RACE_EL_BABARIAN = 11, RACE_EL_MAN = 12, RACE_EL_WOMEN = 13,
	//RACE_KA_NORMAL = 11, RACE_KA_WARRIOR = 12, RACE_KA_ROGUE = 13, RACE_KA_MAGE = 14,
	RACE_NPC = 100,
	RACE_UNKNOWN = 0xffffffff
};

enum Class
{
	CLASS_KINDOF_WARRIOR = 1, CLASS_KINDOF_ROGUE, CLASS_KINDOF_WIZARD, CLASS_KINDOF_PRIEST,
	CLASS_KINDOF_ATTACK_WARRIOR, CLASS_KINDOF_DEFEND_WARRIOR, CLASS_KINDOF_ARCHER, CLASS_KINDOF_ASSASSIN,
	CLASS_KINDOF_ATTACK_WIZARD, CLASS_KINDOF_PET_WIZARD, CLASS_KINDOF_HEAL_PRIEST, CLASS_KINDOF_CURSE_PRIEST,

	CLASS_KA_WARRIOR = 101, CLASS_KA_ROGUE, CLASS_KA_WIZARD, CLASS_KA_PRIEST,
	CLASS_KA_BERSERKER = 105, CLASS_KA_GUARDIAN, CLASS_KA_HUNTER = 107, CLASS_KA_PENETRATOR,
	CLASS_KA_SORCERER = 109, CLASS_KA_NECROMANCER, CLASS_KA_SHAMAN = 111, CLASS_KA_DARKPRIEST,

	CLASS_EL_WARRIOR = 201, CLASS_EL_ROGUE, CLASS_EL_WIZARD, CLASS_EL_PRIEST,
	CLASS_EL_BLADE = 205, CLASS_EL_PROTECTOR, CLASS_EL_RANGER = 207, CLASS_EL_ASSASIN,
	CLASS_EL_MAGE = 209, CLASS_EL_ENCHANTER, CLASS_EL_CLERIC = 211, CLASS_EL_DRUID,

	CLASS_UNKNOWN = 0xffffffff
};

enum Nation
{
	NATION_NOTSELECTED = 0, NATION_KARUS, NATION_ELMORAD, NATION_UNKNOWN = 0xffffffff
};

enum KnightsDuty
{
	KNIGHTS_DUTY_UNKNOWN = 0,
	KNIGHTS_DUTY_CHIEF = 1,
	KNIGHTS_DUTY_VICECHIEF = 2,
	KNIGHTS_DUTY_PUNISH = 3,
	KNIGHTS_DUTY_TRAINEE = 4,
	KNIGHTS_DUTY_KNIGHT = 5,
	KNIGHTS_DUTY_OFFICER = 6
};

enum InOut
{
	INOUT_IN = 1,
	INOUT_OUT = 2,
	INOUT_RESPAWN = 3,
	INOUT_WARP = 4,
	INOUT_SUMMON = 5
};

enum LoyaltyType
{
	LOYALTY_NATIONAL_POINTS = 1,
	LOYALTY_MANNER_POINTS = 2
};

enum SubPacketClassChangeMain
{
	N3_SP_CLASS_CHANGE_PURE = 0x01,
	N3_SP_CLASS_CHANGE_REQ = 0x02,
	N3_SP_CLASS_ALL_POINT = 0x03,
	N3_SP_CLASS_SKILL_POINT = 0x04,
	N3_SP_CLASS_POINT_CHANGE_PRICE_QUERY = 0x05,
	N3_SP_CLASS_PROMOTION = 0x06
};

enum SubPacketClassChange
{
	N3_SP_CLASS_CHANGE_SUCCESS = 0x01,
	N3_SP_CLASS_CHANGE_NOT_YET = 0x02,
	N3_SP_CLASS_CHANGE_ALREADY = 0x03,
	N3_SP_CLASS_CHANGE_ITEM_IN_SLOT = 0x04,
	N3_SP_CLASS_CHANGE_FAILURE = 0x00
};

enum SkillMagicType
{
	SKILL_MAGIC_TYPE_CASTING = 1,
	SKILL_MAGIC_TYPE_FLYING = 2,
	SKILL_MAGIC_TYPE_EFFECTING = 3,
	SKILL_MAGIC_TYPE_FAIL = 4,
	SKILL_MAGIC_TYPE_BUFF = 5,
	SKILL_MAGIC_TYPE_CANCEL = 6
};

enum SkillMagicFailMsg
{
	SKILLMAGIC_FAIL_CASTING = -100,	// "Casting failed."
	SKILLMAGIC_FAIL_KILLFLYING = -101,
	SKILLMAGIC_FAIL_ENDCOMBO = -102,
	SKILLMAGIC_FAIL_NOEFFECT = -103,	// "<skill name> failed"
	SKILLMAGIC_FAIL_ATTACKZERO = -104,	// "<skill name> missed"
	SKILLMAGIC_FAIL_UNKNOWN = 0xffffffff
};

enum ShoppingMallType
{
	STORE_OPEN = 1,
	STORE_CLOSE = 2,
	STORE_BUY = 3,
	STORE_MINI = 4,
	STORE_PROCESS = 5,
	STORE_LETTER = 6
};

enum PartyUpdateType
{
	PARTY_UPDATE_CREATE = 1,             // PARTY GROUP CREATE
	PARTY_UPDATE_INSERT = 2,             // PARTY INSERT PERMIT
	PARTY_UPDATE_JOINED = 3,             // PARTY MEMBER INSERT
	PARTY_UPDATE_LEAVE = 4,              // PARTY MEMBER REMOVE
	PARTY_UPDATE_DISMISSED = 5,          // PARTY GROUP DELETE
	PARTY_UPDATE_HEALTH_MANA_CHANGE = 6,   // PARTY MEMBER HPMP CHANGE
	PARTY_UPDATE_LEVEL_CHANGE = 7,        // PARTY MEMBER LEVEL CHANGE
	PARTY_UPDATE_CLASS_CHANGE = 8,        // PARTY MEMBER CLASS CHANGE
	PARTY_UPDATE_STATUS_CHANGE = 9,       // PARTY MEMBER STATUS ( DISASTER, POISON ) CHANGE
	PARTY_UPDATE_REGISTER = 10,          // PARTY MESSAGE BOARD REGISTER
	PARTY_UPDATE_REPORT = 11,            // PARTY REQUEST MESSAGE BOARD MESSAGES
	PARTY_UPDATE_PROMOTE = 28,           // PROMOTES USER TO PARTY LEADER
	PARTY_UPDATE_COMMAND_CHANGE = 30,
	PARTY_UPDATE_EFFECT_TARGET = 31,
	PARTY_UPDATE_ALERT_ENEMY = 32
};

enum BuffType
{
	BUFF_TYPE_NONE = 0,
	BUFF_TYPE_HP_MP = 1,
	BUFF_TYPE_AC = 2,
	BUFF_TYPE_SIZE = 3,
	BUFF_TYPE_DAMAGE = 4,
	BUFF_TYPE_ATTACK_SPEED = 5,
	BUFF_TYPE_SPEED = 6,
	BUFF_TYPE_STATS = 7,
	BUFF_TYPE_RESISTANCES = 8,
	BUFF_TYPE_ACCURACY = 9,
	BUFF_TYPE_MAGIC_POWER = 10,
	BUFF_TYPE_EXPERIENCE = 11,
	BUFF_TYPE_WEIGHT = 12,
	BUFF_TYPE_WEAPON_DAMAGE = 13,
	BUFF_TYPE_WEAPON_AC = 14,
	BUFF_TYPE_LOYALTY = 15,
	BUFF_TYPE_NOAH_BONUS = 16,
	BUFF_TYPE_PREMIUM_MERCHANT = 17,
	BUFF_TYPE_ATTACK_SPEED_ARMOR = 18,
	BUFF_TYPE_DAMAGE_DOUBLE = 19,
	BUFF_TYPE_DISABLE_TARGETING = 20,
	BUFF_TYPE_BLIND = 21,
	BUFF_TYPE_FREEZE = 22,
	BUFF_TYPE_INSTANT_MAGIC = 23,
	BUFF_TYPE_DECREASE_RESIST = 24,
	BUFF_TYPE_MAGE_ARMOR = 25,
	BUFF_TYPE_PROHIBIT_INVIS = 26,
	BUFF_TYPE_RESIS_AND_MAGIC_DMG = 27,
	BUFF_TYPE_TRIPLEAC_HALFSPEED = 28,
	BUFF_TYPE_BLOCK_CURSE = 29,
	BUFF_TYPE_BLOCK_CURSE_REFLECT = 30,
	BUFF_TYPE_MANA_ABSORB = 31,
	BUFF_TYPE_IGNORE_WEAPON = 32,
	BUFF_TYPE_VARIOUS_EFFECTS = 33,
	BUFF_TYPE_PASSION_OF_SOUL = 35,
	BUFF_TYPE_FIRM_DETERMINATION = 36,
	BUFF_TYPE_SPEED2 = 40,
	BUFF_TYPE_UNK_EXPERIENCE = 42,
	BUFF_TYPE_ATTACK_RANGE_ARMOR = 43,
	BUFF_TYPE_MIRROR_DAMAGE_PARTY = 44,
	BUFF_TYPE_DAGGER_BOW_DEFENSE = 45,
	BUFF_TYPE_STUN = 47,
	BUFF_TYPE_LOYALTY_AMOUNT = 55,
	BUFF_TYPE_NO_RECALL = 150,
	BUFF_TYPE_REDUCE_TARGET = 151,
	BUFF_TYPE_SILENCE_TARGET = 152,
	BUFF_TYPE_NO_POTIONS = 153,
	BUFF_TYPE_KAUL_TRANSFORMATION = 154,
	BUFF_TYPE_UNDEAD = 155,
	BUFF_TYPE_UNSIGHT = 156,
	BUFF_TYPE_BLOCK_PHYSICAL_DAMAGE = 157,
	BUFF_TYPE_BLOCK_MAGICAL_DAMAGE = 158,
	BUFF_TYPE_UNK_POTION = 159,
	BUFF_TYPE_SLEEP = 160,
	BUFF_TYPE_INVISIBILITY_POTION = 163,
	BUFF_TYPE_GODS_BLESSING = 164,
	BUFF_TYPE_HELP_COMPENSATION = 165,
	BUFF_TYPE_HYPER_NOAH = 169,
};

enum TradeSubPacket
{
	TRADE_REQUEST = 1,
	TRADE_AGREE = 2,
	TRADE_ADD = 3,
	TRADE_OTHER_ADD = 4,
	TRADE_DECIDE = 5,
	TRADE_OTHER_DECIDE = 6,
	TRADE_DONE = 7,
	TRADE_CANCEL = 8
};

enum ItemClass
{
	ITEM_CLASS_DAGGER = 11,
	ITEM_CLASS_SWORD = 21,
	ITEM_CLASS_SWORD_2H = 22,
	ITEM_CLASS_AXE = 31,
	ITEM_CLASS_AXE_2H = 32,
	ITEM_CLASS_MACE = 41,
	ITEM_CLASS_MACE_2H = 42,
	ITEM_CLASS_SPEAR = 51,
	ITEM_CLASS_POLEARM = 52,

	ITEM_CLASS_SHIELD = 60,

	ITEM_CLASS_BOW = 70,
	ITEM_CLASS_BOW_CROSS = 71,
	ITEM_CLASS_BOW_LONG = 80,

	ITEM_CLASS_EARRING = 91,
	ITEM_CLASS_AMULET = 92,
	ITEM_CLASS_RING = 93,
	ITEM_CLASS_BELT = 94,
	ITEM_CLASS_CHARM = 95,
	ITEM_CLASS_JEWEL = 96,
	ITEM_CLASS_POTION = 97,
	ITEM_CLASS_SCROLL = 98,

	ITEM_CLASS_LAUNCHER = 100,

	ITEM_CLASS_STAFF = 110,
	ITEM_CLASS_ARROW = 120,
	ITEM_CLASS_JAVELIN = 130,

	ITEM_CLASS_ARMOR_WARRIOR = 210,
	ITEM_CLASS_ARMOR_ROGUE = 220,
	ITEM_CLASS_ARMOR_MAGE = 230,
	ITEM_CLASS_ARMOR_PRIEST = 240,

	ITEM_CLASS_ETC = 251,

	ITEM_CLASS_UNKNOWN = 0xffffffff
};

enum ItemMovementType
{
	ITEM_INVEN_SLOT = 1,
	ITEM_SLOT_INVEN = 2,
	ITEM_INVEN_INVEN = 3,
	ITEM_SLOT_SLOT = 4,
	ITEM_INVEN_ZONE = 5,
	ITEM_ZONE_INVEN = 6,
	ITEM_INVEN_TO_COSP = 7,
	ITEM_COSP_TO_INVEN = 8,
	ITEM_INVEN_TO_MBAG = 9,
	ITEM_MBAG_TO_INVEN = 10,
	ITEM_MBAG_TO_MBAG = 11
};

enum ZoneChangeOpcode
{
	ZoneChangeLoading = 1,
	ZoneChangeLoaded = 2,
	ZoneChangeTeleport = 3,
	ZoneChangeMilitaryCamp = 4,
};

enum ZoneInfo
{
	ZONE_KARUS = 1,                         // Luferson Castle - Military Camp 1
	ZONE_ELMORAD = 2,                       // Elmorad Castle - Military Camp 1
	ZONE_KARUS2 = 5,                        // Luferson Castle - Military Camp 2
	ZONE_ELMORAD2 = 7,                      // Elmorad Castle - Military Camp 2

	ZONE_KARUS_ESLANT = 11,                 // Karus Eslant - Military Camp 1
	ZONE_ELMORAD_ESLANT = 12,               // Elmorad Eslant - Military Camp 1
	ZONE_KARUS_ESLANT2 = 13,                // Karus Eslant - Military Camp 2
	ZONE_ELMORAD_ESLANT2 = 15,              // Elmorad Eslant - Military Camp 2

	ZONE_MORADON = 21,						// Moradon - Military Camp 1
	ZONE_MORADON2 = 22,                     // Moradon - Military Camp 2
	ZONE_MORADON3 = 23,                     // Moradon - Military Camp 3
	ZONE_MORADON4 = 24,                     // Moradon - Military Camp 4
	ZONE_MORADON5 = 25,                     // Moradon - Military Camp 5

	ZONE_DELOS = 30,
	ZONE_BIFROST = 31,
	ZONE_DESPERATION_ABYSS = 32,
	ZONE_HELL_ABYSS = 33,
	ZONE_DRAGON_CAVE = 34,
	ZONE_ARENA = 48,
	ZONE_ORC_ARENA = 51,
	ZONE_BLOOD_DON_ARENA = 52,
	ZONE_GOBLIN_ARENA = 53,
	ZONE_CAITHAROS_ARENA = 54,
	ZONE_FORGOTTEN_TEMPLE = 55,
	ZONE_BATTLE_BASE = 60,

	ZONE_BATTLE = ZONE_BATTLE_BASE + 1,		// Napies Gorge
	ZONE_BATTLE2 = ZONE_BATTLE_BASE + 2,    // Alseids Prairie
	ZONE_BATTLE3 = ZONE_BATTLE_BASE + 3,    // Nieds Triangle
	ZONE_BATTLE4 = ZONE_BATTLE_BASE + 4,    // Nereid's Island
	ZONE_BATTLE5 = ZONE_BATTLE_BASE + 5,    // Zipang
	ZONE_BATTLE6 = ZONE_BATTLE_BASE + 6,    // Oreads

	ZONE_SNOW_BATTLE = 69,
	ZONE_RONARK_LAND = 71,
	ZONE_ARDREAM = 72,
	ZONE_RONARK_LAND_BASE = 73,
	ZONE_KROWAZ_DOMINION = 75,

	ZONE_MONSTER_STONE1 = 81,
	ZONE_MONSTER_STONE2 = 82,
	ZONE_MONSTER_STONE3 = 83,

	ZONE_BORDER_DEFENSE_WAR = 84,
	ZONE_CHAOS_DUNGEON = 85,
	ZONE_UNDER_CASTLE = 86,
	ZONE_JURAD_MOUNTAIN = 87,
	ZONE_PRISON = 92,
	ZONE_ISILOON_ARENA = 93,
	ZONE_FELANKOR_ARENA = 94,
	ZONE_DRAKI_TOWER = 95
};

enum AbnormalType
{
	ABNORMAL_INVISIBLE = 0,	// Hides character completely (for GM visibility only).
	ABNORMAL_NORMAL = 1,	// Shows character. This is the default for players.
	ABNORMAL_GIANT = 2,	// Enlarges character.
	ABNORMAL_DWARF = 3,	// Shrinks character.
	ABNORMAL_BLINKING = 4,	// Forces character to start blinking.
	ABNORMAL_GIANT_TARGET = 6,		// Enlarges character and shows "Hit!" effect.
	ABNORMAL_BLINKING_END = 7
};

enum SubPacketState 
{
	N3_SP_STATE_CHANGE_SITDOWN = 0x01,
	N3_SP_STATE_CHANGE_RECRUIT_PARTY = 0x02,
	N3_SP_STATE_CHANGE_SIZE = 0x03,
	N3_SP_STATE_CHANGE_ACTION = 0x04,			// 1 - 인사, 11 - 도발
	N3_SP_STATE_CHANGE_VISIBLE = 0x05
};		// 투명 0 ~ 255

enum ChatMode 
{
	GENERAL_CHAT = 1,
	PRIVATE_CHAT = 2,
	PARTY_CHAT = 3,
	FORCE_CHAT = 4,
	SHOUT_CHAT = 5,
	KNIGHTS_CHAT = 6,
	PUBLIC_CHAT = 7,
	WAR_SYSTEM_CHAT = 8,
	PERMANENT_CHAT = 9,
	END_PERMANENT_CHAT = 10,
	MONUMENT_NOTICE = 11,
	GM_CHAT = 12,
	COMMAND_CHAT = 13,
	MERCHANT_CHAT = 14,
	ALLIANCE_CHAT = 15,
	ANNOUNCEMENT_CHAT = 17,
	SEEKING_PARTY_CHAT = 19,
	GM_INFO_CHAT = 21,	// info window : "Level: 0, UserCount:16649" (NOTE: Think this is the missing overhead info (probably in form of a command (with args))
	COMMAND_PM_CHAT = 22,	// Commander Chat PM??
	CLAN_NOTICE = 24,
	KROWAZ_NOTICE = 25,
	DEATH_NOTICE = 26,
	CHAOS_STONE_ENEMY_NOTICE = 27,	// The enemy has destroyed the Chaos stone something (Red text, middle of screen)
	CHAOS_STONE_NOTICE = 28,
	ANNOUNCEMENT_WHITE_CHAT = 29	// what's it used for?
};

enum ItemAttribute 
{
	ITEM_ATTRIBUTE_GENERAL = 0,
	ITEM_ATTRIBUTE_MAGIC = 1,
	ITEM_ATTRIBUTE_LAIR = 2,
	ITEM_ATTRIBUTE_CRAFT = 3,
	ITEM_ATTRIBUTE_UNIQUE = 4,
	ITEM_ATTRIBUTE_UPGRADE = 5,
	ITEM_ATTRIBUTE_EVENT = 6,
	ITEM_ATTRIBUTE_PET = 7,
	ITEM_ATTRIBUTE_COSPRE = 8,
	ITEM_ATTRIBUTE_MINERVA = 9,
	ITEM_ATTRIBUTE_UPGRADE_REVERSE = 11,
	ITEM_ATTRIBUTE_UNIQUE_REVERSE = 12,
	ITEM_ATTRIBUTE_RED = 13,
	ITEM_ATTRIBUTE_UNKNOWN = 0xffffffff
};

enum ItemRace
{
	RACE_TRADEABLE_IN_72HR = 69,  // These items can't be traded until 72 hours from the time of creation
	RACE_NO_TRADE = 71,  // Cannot be traded
	RACE_NO_TRADE_SOLD = 20,  // Cannot be traded or sold.
	RACE_NO_TRADE_SOLD2 = 70,  // Cannot be traded or sold.
	RACE_NO_TRADE_SOLD_STORE = 73,  // Cannot be traded, sold or stored
};

enum InventoryItemFlag
{
	INVENTORY_ITEM_FLAG_NONE,
	INVENTORY_ITEM_FLAG_DELETE,
	INVENTORY_ITEM_FLAG_SELL,
	INVENTORY_ITEM_FLAG_INN,
	INVENTORY_ITEM_FLAG_VIP,
	INVENTORY_ITEM_FLAG_EQUIP,
};

enum ItemPosition 
{
	ITEM_POS_DUAL = 0, 
	ITEM_POS_RIGHTHAND, 
	ITEM_POS_LEFTHAND, 
	ITEM_POS_TWOHANDRIGHT, 
	ITEM_POS_TWOHANDLEFT,
	ITEM_POS_UPPER = 5, 
	ITEM_POS_LOWER, 
	ITEM_POS_HEAD, 
	ITEM_POS_GLOVES, 
	ITEM_POS_SHOES,
	ITEM_POS_EAR = 10, 
	ITEM_POS_NECK, 
	ITEM_POS_FINGER, 
	ITEM_POS_SHOULDER, 
	ITEM_POS_BELT,
	ITEM_POS_INVENTORY = 15, 
	ITEM_POS_GOLD = 16, 
	ITEM_POS_SONGPYUN = 17,
	ITEM_POS_UNKNOWN = 0xffffffff
};

enum RouteStepType
{
	STEP_NONE,
	STEP_MOVE,
	STEP_TOWN,
	STEP_SUNDRIES,
	STEP_INN,
	STEP_GENIE,
	STEP_POTION,
	STEP_BOT_START,
	STEP_OBJECT_EVENT,
	STEP_WARP,
	STEP_DC_SUNDRIES,
};