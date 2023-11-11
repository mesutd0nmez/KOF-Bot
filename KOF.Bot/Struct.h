#pragma once
#include <corecrt_math_defines.h>

static const int COSP_MAX = 9;              // 9 cospre slots
static const int SLOT_MAX = 14;             // 14 equipped item slots
static const int HAVE_MAX = 28;             // 28 inventory slots
static const int MBAG_COUNT = 2;            // 2 magic bag slots
static const int MBAG_MAX = 12;             // 12 slots per magic bag

static const int VIP_HAVE_MAX = 48;         // 48 Vip inventory slots
static const int WAREHOUSE_MAX = 192;       // warehouse slots
static const int MAX_MERCH_ITEMS = 12;      // merchant slots

static const int INVENTORY_MBAG = SLOT_MAX + HAVE_MAX + COSP_MAX;   // 14 + 28 + 9 = 51
static const int INVENTORY_MBAG1 = INVENTORY_MBAG;
static const int INVENTORY_MBAG2 = INVENTORY_MBAG + MBAG_MAX;       // 51 + 12 = 63
static const int INVENTORY_TOTAL = INVENTORY_MBAG2 + MBAG_MAX;      // 63 + 12 = 75

struct Vector3
{
	Vector3(float fX, float fZ, float fY) : m_fX(fX), m_fZ(fZ), m_fY(fY) {};

	float m_fX;
	float m_fZ;
	float m_fY;

	bool operator == (const Vector3& vec)
	{
		if (m_fX == vec.m_fX && m_fZ == vec.m_fZ && m_fY == vec.m_fY)
			return true;

		return false;
	}

	bool operator != (const Vector3& vec)
	{
		if (m_fX != vec.m_fX || m_fZ != vec.m_fZ || m_fY != vec.m_fY)
			return true;

		return false;
	}

	Vector3 operator+(const Vector3& other) const 
	{
		return Vector3(m_fX + other.m_fX, m_fZ + other.m_fZ, m_fY + other.m_fY);
	}

	Vector3 operator-(const Vector3& other) const 
	{
		return Vector3(m_fX - other.m_fX, m_fZ - other.m_fZ, m_fY - other.m_fY);
	}

	Vector3 operator*(float scalar) const 
	{
		return Vector3(m_fX * scalar, m_fZ * scalar, m_fY * scalar);
	}

	float Magnitude() const 
	{
		return std::sqrt(m_fX * m_fX + m_fZ * m_fZ + m_fY * m_fY);
	}

	Vector3 Normalize() const 
	{
		float mag = Magnitude();
		return Vector3(m_fX / mag, m_fZ / mag, m_fY / mag);
	}
};

struct SShopItem
{
	SShopItem(uint8_t iPage, uint8_t iPos, int32_t iItemId) : m_iPage(iPage), m_iPos(iPos), m_iItemId(iItemId) {};

	uint8_t m_iPage;
	uint8_t m_iPos;
	int32_t m_iItemId;
};

struct SSItemBuy
{
	SSItemBuy(int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount, uint8_t iShopPage, uint8_t iShopPosition) : 
		m_iItemId(iItemId), m_iInventoryPosition(iInventoryPosition), m_iCount(iCount), m_iShopPage(iShopPage), m_iShopPosition(iShopPosition) {};

	int32_t m_iItemId;
	uint8_t m_iInventoryPosition;
	int16_t m_iCount;
	uint8_t m_iShopPage;
	uint8_t m_iShopPosition;
};

struct SSItemSell
{
	SSItemSell(int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount) :
		m_iItemId(iItemId), m_iInventoryPosition(iInventoryPosition), m_iCount(iCount) {};

	int32_t m_iItemId;
	uint8_t m_iInventoryPosition;
	int16_t m_iCount;
};

struct SSupplyBuyList
{
	SSupplyBuyList(int32_t iNpcId, Vector3 vec3NpcPosition, std::vector<SSItemBuy> vecItemBuy) : m_iNpcId(iNpcId), m_vec3NpcPosition(vec3NpcPosition), m_vecItemBuy(vecItemBuy) {};

	int32_t m_iNpcId;
	Vector3 m_vec3NpcPosition;
	std::vector<SSItemBuy> m_vecItemBuy;
};

struct SNpcData
{
	uint32_t iProtoID;
};

struct EntityInfo
{
	EntityInfo(int32_t iBase, int32_t iId, int32_t iProtoId, int32_t iMaxHP, int32_t iHP, int32_t iState, int32_t iNation, Vector3 v3Position, float fDistance, bool bEnemy, float fRadius) :
		m_iBase(iBase), m_iId(iId), m_iProtoId(iProtoId), m_iMaxHP(iMaxHP), 
		m_iHP(iHP), m_iState(iState), m_iNation(iNation), m_v3Position(v3Position), 
		m_fDistance(fDistance), m_bEnemy(bEnemy), m_fRadius(fRadius) {};

	int32_t m_iBase;
	int32_t m_iId;
	int32_t m_iProtoId;
	int32_t m_iMaxHP;
	int32_t m_iHP;
	int32_t m_iState;
	int32_t m_iNation;
	Vector3 m_v3Position;
	float m_fDistance;
	bool m_bEnemy;
	float m_fRadius;
};

typedef struct SItemData
{
	int32_t				iPos;
	uint32_t			iBase;
	uint32_t			iItemID;
	uint16_t			iDurability;
	uint16_t			iCount;
	uint8_t				iFlag;
	int16_t				iRentalTime;
	uint32_t			iSerial;
	uint32_t			iExpirationTime;
} TItemData;

typedef struct SPlayer
{
	int32_t			iID;
	std::string		szName;
	float			fX;
	float			fY;
	float			fZ;
	Nation			eNation;
	Race			eRace;
	Class			eClass;
	uint8_t			iLevel;
	int32_t			iHPMax;
	int32_t			iHP;
	int32_t			iMSPMax;
	int32_t			iMSP;
	int32_t			iAuthority;
	uint16_t		iBonusPointRemain;

	uint8_t			iFace;
	int32_t			iHair;

	uint8_t			iRank;
	uint8_t			iTitle;

	uint8_t			iUnknown1;
	uint8_t			iUnknown2;

	uint8_t			iCity;
	int16_t			iKnightsID;
	std::string		szKnights;
	uint8_t			iKnightsGrade;
	uint8_t			iKnightsRank;

	uint32_t		iGold;
	uint64_t		iExpNext;
	uint64_t		iExp;
	uint32_t		iRealmPoint;
	uint32_t		iRealmPointMonthly;
	KnightsDuty		eKnightsDuty;
	uint32_t		iWeightMax;
	uint32_t		iWeight;

	uint8_t			iStrength;
	uint8_t			iStrength_Delta;
	uint8_t			iStamina;
	uint8_t			iStamina_Delta;
	uint8_t			iDexterity;
	uint8_t			iDexterity_Delta;
	uint8_t			iIntelligence;
	uint8_t			iIntelligence_Delta;
	uint8_t 		iMagicAttak;
	uint8_t 		iMagicAttak_Delta;

	int16_t 		iAttack;
	int16_t 		iGuard;

	uint8_t 		iRegistFire;
	uint8_t 		iRegistCold;
	uint8_t 		iRegistLight;
	uint8_t 		iRegistMagic;
	uint8_t 		iRegistCurse;
	uint8_t 		iRegistPoison;

	uint8_t			iSkillInfo[9];

	StateAction		eState;

	int16_t			iMoveSpeed;
	uint8_t			iMoveType;

	TItemData		tInventory[INVENTORY_TOTAL];
	TItemData		tWarehouse[WAREHOUSE_MAX];
	TItemData		tVipWarehouse[VIP_HAVE_MAX];

	bool			bBlinking;
	float			fRotation;
} TPlayer;

typedef struct SNpc
{
	int32_t			iID;
	uint16_t		iProtoID;
	uint8_t			iMonsterOrNpc;
	uint16_t		iPictureId;
	uint32_t		iUnknown1;
	uint8_t			iFamilyType;
	uint32_t		iSellingGroup;
	uint16_t		iModelsize;
	uint32_t		iWeapon1;
	uint32_t		iWeapon2;

	std::string		szPetOwnerName;
	std::string		szPetName;

	uint8_t			iModelGroup;
	uint8_t			iLevel;

	float			fX;
	float			fY;
	float			fZ;

	uint32_t		iStatus;

	uint8_t			iUnknown2;
	uint32_t		iUnknown3;
	float			fRotation;

	StateAction		eState;

	int16_t			iMoveSpeed;
	uint8_t			iMoveType;

	int32_t			iHPMax;
	int32_t			iHP;
} TNpc;

typedef struct SLoot
{
	int32_t			iNpcID;
	float			fNpcX;
	float			fNpcZ;
	float			fNpcY;
	uint32_t		iBundleID;
	uint8_t			iItemCount;
	float			fDropTime;
} TLoot;

typedef struct  __TABLE_TEXTS
{
	uint32_t			dwID;
	std::string			szText;
} TABLE_TEXTS;

typedef struct __TABLE_UPC_SKILL
{
	uint32_t			iID;
	std::string			szEngName;
	std::string			szName;
	std::string			szDesc;
	int32_t				iSelfAnimID1;
	int32_t				iSelfAnimID2;

	int32_t				idwTargetAnimID;
	int32_t				iSelfFX1;
	int32_t				iSelfPart1;
	int32_t				iSelfFX2;
	int32_t				iSelfPart2;
	int32_t				iFlyingFX;
	int32_t				iTargetFX;

	int32_t				iTargetPart;
	int32_t				iTarget;
	int32_t				iNeedLevel;
	int32_t				iNeedSkill;

	int32_t				iExhaustMSP;
	int32_t				iExhaustHP;
	int16_t				iExhaustStamina;	//19

	uint32_t			iCastTime;			//20

	uint32_t			dwNeedItem;			//21
	int32_t				iReCastTime;		//22
	int32_t				iCooldown;			//23
	float				fUnknown1;			//24
	uint8_t				iUnknown2;			//25
	float				fUnknown3;			//26
	int32_t				iPercentSuccess;	//27
	uint32_t			dw1stTableType;		//28
	uint32_t			dw2ndTableType;		//29
	int32_t				iValidDist;			//30

	int32_t				iUnknown4;			//31
	uint32_t			dwUnknown5;			//32
	int32_t				iUnknown6;			//33
	uint32_t			iBaseId;			//34
	uint8_t				byUnknown8;			//35
	uint8_t				byUnknown9;			//36
	int32_t				iUnknown10;			//37


	bool IsDeBuff()
	{
		if (iTarget == TARGET_ENEMY_ONLY && dw1stTableType == 4)
			return true;

		return false;
	}

} TABLE_UPC_SKILL;

typedef struct __TABLE_UPC_SKILL_EXTENSION1
{
	uint32_t			iID;
	int32_t				iUnknown1;
	int32_t				iUnknown2;
	int32_t				iUnknown3;
	int32_t				iUnknown4;
	int32_t				iUnknown5;
	int32_t				iUnknown6;
	int32_t				iUnknown7;
	int32_t				iUnknown8;
	int32_t				iUnknown9;
	int32_t				iUnknown10;
	int32_t				iUnknown11;
	int32_t				iUnknown12;
	int32_t				iUnknown13;
} TABLE_UPC_SKILL_EXTENSION1;

typedef struct __TABLE_UPC_SKILL_EXTENSION2
{
	uint32_t			iID;
	int32_t				iUnknown1;
	int32_t				iUnknown2;
	int32_t				iUnknown3;
	int32_t				iUnknown4;
	int32_t				iArrowCount;
} TABLE_UPC_SKILL_EXTENSION2;

typedef struct __TABLE_UPC_SKILL_EXTENSION3
{
	uint32_t			iID;
	int32_t				iUnknown1;
	int32_t				iHealType;
	int32_t				iHealValue;
	int32_t				iUnknown4;
	int32_t				iUnknown5;
	int32_t				iUnknown6;
} TABLE_UPC_SKILL_EXTENSION3;

typedef struct __TABLE_UPC_SKILL_EXTENSION4
{
	uint32_t			iID;
	int32_t				iBuffType;
	int32_t				iAreaRadius;
	int32_t				iBuffDuration;
	int32_t				iUnknown4;
	int32_t				iUnknown5;
	int32_t				iUnknown6;
	int32_t				iUnknown7;
	int32_t				iUnknown8;
	int32_t				iUnknown9;
	int32_t				iUnknown10;
	int32_t				iUnknown11;
	int32_t				iUnknown12;
	int32_t				iUnknown13;
	int32_t				iUnknown14;
	int32_t				iUnknown15;
	int32_t				iUnknown16;
	int32_t				iUnknown17;
	int32_t				iUnknown18;
	int32_t				iUnknown19;
	int32_t				iUnknown20;
	int32_t				iUnknown21;
	int32_t				iUnknown22;
	int32_t				iUnknown23;
	int32_t				iUnknown24;
	int32_t				iUnknown25;
	int32_t				iUnknown26;
} TABLE_UPC_SKILL_EXTENSION4;

typedef struct __TABLE_UPC_SKILL_EXTENSION5
{
	uint32_t			iID;
	int32_t				iUnknown1;
	int32_t				iUnknown2;
	int32_t				iUnknown3;
} TABLE_UPC_SKILL_EXTENSION5;

typedef struct __TABLE_UPC_SKILL_EXTENSION6
{
	uint32_t			iID;
	std::string			szUnknown1;
	std::string			szUnknown2;
	int32_t				iUnknown3;
	int32_t				iUnknown4;
	int32_t				iUnknown5;
	int32_t				iUnknown6;
	int32_t				iUnknown7;
	int32_t				iUnknown8;
	int32_t				iUnknown9;
	int32_t				iUnknown10;
	int32_t				iUnknown11;
	int32_t				iUnknown12;
	int32_t				iUnknown13;
	int32_t				iUnknown14;
	int32_t				iUnknown15;
	int32_t				iUnknown16;
	int32_t				iUnknown17;
	int32_t				iUnknown18;
	int32_t				iUnknown19;
	uint8_t				iUnknown20;
	uint32_t			iUnknown21;
	uint32_t			iUnknown22;
	uint32_t			iUnknown23;
	uint32_t			iUnknown24;
	uint8_t				iUnknown25;
	uint32_t			iUnknown26;
	uint32_t			iUnknown27;
	int32_t				iUnknown28;
} TABLE_UPC_SKILL_EXTENSION6;

typedef struct __TABLE_UPC_SKILL_EXTENSION7
{
	uint32_t			iID;
	int32_t				iUnknown1;
} TABLE_UPC_SKILL_EXTENSION7;

typedef struct __TABLE_UPC_SKILL_EXTENSION8
{
	uint32_t			iID;
	uint8_t				iUnknown1;
	int32_t				iUnknown2;
} TABLE_UPC_SKILL_EXTENSION8;

typedef struct __TABLE_UPC_SKILL_EXTENSION9
{
	uint32_t			iID;
	std::string			szUnknown1;
	std::string			szUnknown2;
	uint8_t				iUnknown3;
	uint8_t				iUnknown4;
	int32_t				iUnknown5;
	uint8_t				iUnknown6;
	uint8_t				iUnknown7;
	uint8_t				iUnknown8;
	uint8_t				iUnknown9;
	int32_t				iUnknown10;
	uint8_t				iUnknown11;
	uint8_t				iUnknown12;
	uint8_t				iUnknown13;
} TABLE_UPC_SKILL_EXTENSION9;

typedef struct __TABLE_ITEM
{
	uint32_t			iID;
	uint8_t 			byExtIndex;
	std::string			szName;
	std::string			szDesc;

	uint32_t			dwIDK0;
	uint8_t				byIDK1;

	uint32_t			dwIDResrc;
	uint32_t			dwIDIcon;
	uint32_t			dwSoundID0;
	uint32_t			dwSoundID1;

	uint8_t				byKind;
	uint8_t				byIsRobeType;
	uint8_t				byAttachPoint;
	uint8_t				byNeedRace;
	uint8_t				byNeedClass;

	int16_t				siDamage;
	int16_t				siAttackInterval;
	int16_t				siAttackRange;
	int16_t				siWeight;
	int16_t				siMaxDurability;
	int					iPriceRepair;
	int					iPriceSale;
	int16_t				siDefense;
	uint8_t				byContable;

	uint32_t			dwEffectID1;
	uint32_t			dwEffectID2;

	int8_t				byReqLevelMin;
	int8_t				byReqLevelMax;

	uint8_t				byNeedRank;
	uint8_t				byNeedTitle;
	uint8_t				byNeedStrength;
	uint8_t				byNeedStamina;
	uint8_t				byNeedDexterity;
	uint8_t				byNeedInteli;
	uint8_t				byNeedMagicAttack;

	uint8_t				bySellGroup;

	uint8_t				byItemGrade;

	int32_t				iUnknown1;
	int16_t				siBound;

	uint32_t			dwUnknown2;
	uint8_t				byUnknown3;
} TABLE_ITEM;

typedef struct __TABLE_ITEM_EXTENSION
{
	uint32_t			iExtensionID;
	std::string			szHeader;
	uint32_t			iBaseID;
	std::string			szDescription;
	uint32_t			iEffectID;
	uint32_t			iDxtID;
	uint32_t			iIconID;
	uint8_t				iItemType;
	int16_t				iDamage;
	int16_t				iAttackIntervalPercentage;
	int16_t				iHitRate;
	int16_t				iEvasionRate;
	int16_t				iMaxDurability;
	int16_t				iPriceMultiply;
	int16_t				iDefense;
	int16_t				iDaggerDefense;
	int16_t				iSwordDefense;
	int16_t				iClubDefense;
	int16_t				iAxeDefense;
	int16_t				iSpearDefense;
	int16_t				iArrowDefense;
	int16_t				iJamadarDefense;
	uint8_t				iFireDamage;
	uint8_t				iGlacierDamage;
	uint8_t				iLightningDamage;
	uint8_t				iPoisonDamage;
	uint8_t				iHPRecovery;
	uint8_t				iMPDamage;
	uint8_t				iMPRecovery;
	uint8_t				iRepelPhysDamage;
	uint8_t				iSoulBind;
	int16_t				iStrB;
	int16_t				iHpB;
	int16_t				iDexB;
	int16_t				iIntB;
	int16_t				iMpB;
	int16_t				iBonusHealth;
	int16_t				iBonusMP;
	int16_t				iFireResist;
	int16_t				iIceResist;
	int16_t				iLightningResist;
	int16_t				iMagicResist;
	int16_t				iPoisonResist;
	int16_t				iCurseResist;
	uint32_t			iEffectID1;
	uint32_t			iEffectID2;
	int16_t				iReqLevel;
	int16_t				iReqRank;
	int16_t				iReqTitle;
	int16_t				iReqStr;
	int16_t				iReqHp;
	int16_t				iReqDex;
	int16_t				iReqInt;
	int16_t				iReqMP;
	uint8_t				iUnknown2;
	int16_t				iUnknown3;
} TABLE_ITEM_EXTENSION;

typedef struct  __TABLE_NPC
{
	uint32_t			iID;
	std::string			szText;
	uint32_t			iProtoID;
	uint32_t			iBoss;
	uint32_t			iUnknown1;
	uint32_t			iSize;
	uint32_t			iUnknown2;
} TABLE_NPCS;

typedef struct  __TABLE_MOB
{
	uint32_t			iID;
	std::string			szText;
	uint32_t			iProtoID;
	uint8_t				iUnknown1;
	uint8_t				iUnknown2;
	int32_t				iUnknown3;
} TABLE_MOBS;

typedef struct  __TABLE_ITEM_SELL
{
	uint32_t			iID;
	uint32_t			iSellingGroup;
	uint32_t			iItem0;
	uint32_t			iItem1;
	uint32_t			iItem2;
	uint32_t			iItem3;
	uint32_t			iItem4;
	uint32_t			iItem5;
	uint32_t			iItem6;
	uint32_t			iItem7;
	uint32_t			iItem8;
	uint32_t			iItem9;
	uint32_t			iItem10;
	uint32_t			iItem11;
	uint32_t			iItem12;
	uint32_t			iItem13;
	uint32_t			iItem14;
	uint32_t			iItem15;
	uint32_t			iItem16;
	uint32_t			iItem17;
	uint32_t			iItem18;
	uint32_t			iItem19;
	uint32_t			iItem20;
	uint32_t			iItem21;
	uint32_t			iItem22;
	uint32_t			iItem23;
} TABLE_ITEM_SELLS;

typedef struct  __TABLE_DISGUISE_RING
{
	uint32_t			iID;
	std::string			szName;
	int32_t				iRequiredLevel;
	uint32_t			iSkillID;
	uint32_t			iItemID;
	uint8_t				iType;
	std::string			szDescription;
} TABLE_DISGUISE_RING;

typedef struct __Party
{
	int32_t				iID;
	std::string			szName;
	int16_t				iHP;
	int16_t				iMaxHP;
	int16_t				iMP;
	int16_t				iMaxMP;
	int32_t				iCure;
} Party;

typedef struct __PartyMemberBuffInfo
{
	int32_t				iMemberID;
	std::chrono::milliseconds iBuffTime;
} PartyMemberBuffInfo;

typedef struct __PartyMember
{
	uint8_t iIndex;
	int32_t iMemberID;
	std::string szName;
	uint16_t iHP;
	uint16_t iMaxHP;
	uint16_t iMP;
	uint16_t iMaxMP;
	uint8_t iLevel;
	uint16_t iClass;
	uint16_t iNation;

	float fHpBuffTime;
	uint8_t iHpBuffAttemptCount;

	float fACBuffTime;
	uint8_t iACBuffAttemptCount;

	float fMindBuffTime;
	uint8_t iMindBuffAttemptCount;
} PartyMember;