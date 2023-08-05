#pragma once

#define LS_VERSION_REQ			0x01
#define LS_DOWNLOADINFO_REQ		0x02
#define LS_CRYPTION				0xF2
#define LS_LOGIN_REQ			0xF3
#define LS_MGAME_LOGIN			0xF4
#define LS_SERVERLIST			0xF5
#define LS_NEWS					0xF6
#define LS_UNKF7				0xF7
#define LS_OTP					0xFA
#define LS_OTP_UNKNOWN			0xFB

#define WIZ_LOGIN				0x01	// Account Login
#define WIZ_NEW_CHAR			0x02	// Create Character DB
#define WIZ_DEL_CHAR			0x03	// Delete Character DB
#define WIZ_SEL_CHAR			0x04	// Select Character
#define WIZ_SEL_NATION			0x05	// Select Nation
#define WIZ_MOVE				0x06	// Move ( 1 Second )
#define WIZ_USER_INOUT			0x07	// User Info Insert, delete
#define WIZ_ATTACK				0x08	// General Attack 
#define WIZ_ROTATE				0x09	// Rotate
#define WIZ_NPC_INOUT			0x0A	// Npc Info Insert, delete
#define WIZ_NPC_MOVE			0x0B	// Npc Move ( 1 Second )
#define WIZ_ALLCHAR_INFO_REQ	0x0C	// Account All Character Info Request
#define WIZ_GAMESTART			0x0D	// Request Other User, Npc Info
#define WIZ_MYINFO				0x0E	// User Detail Data Download
#define WIZ_LOGOUT				0x0F	// Request Logout
#define WIZ_CHAT				0x10	// User Chatting..
#define WIZ_DEAD				0x11	// User Dead
#define WIZ_REGENE				0x12	// User	Regeneration
#define WIZ_TIME				0x13	// Game Timer
#define WIZ_WEATHER				0x14	// Game Weather
#define WIZ_REGIONCHANGE		0x15	// Region UserInfo Receive
#define WIZ_REQ_USERIN			0x16	// Client Request UnRegistered User List
#define WIZ_HP_CHANGE			0x17	// Current HP Download
#define WIZ_MSP_CHANGE			0x18	// Current MP Download
#define WIZ_ITEM_LOG			0x19	// Send To Agent for Writing Log
#define WIZ_EXP_CHANGE			0x1A	// Current EXP Download
#define WIZ_LEVEL_CHANGE		0x1B	// Max HP, MP, SP, Weight, Exp Download
#define WIZ_NPC_REGION			0x1C	// Npc Region Change Receive
#define WIZ_REQ_NPCIN			0x1D	// Client Request UnRegistered NPC List
#define WIZ_WARP				0x1E	// User Remote Warp
#define WIZ_ITEM_MOVE			0x1F	// User Item Move
#define WIZ_NPC_EVENT			0x20	// User Click Npc Event
#define WIZ_ITEM_TRADE			0x21	// Item Trade 
#define WIZ_TARGET_HP			0x22	// Attack Result Target HP 
#define WIZ_ITEM_DROP			0x23	// Zone Item Insert
#define WIZ_BUNDLE_OPEN_REQ		0x24	// Zone Item list Request
#define WIZ_TRADE_NPC			0x25	// ITEM Trade start
#define WIZ_ITEM_GET			0x26	// Zone Item Get
#define WIZ_ZONE_CHANGE			0x27	// Zone Change
#define WIZ_POINT_CHANGE		0x28	// Str, Sta, dex, intel, cha, point up down
#define WIZ_STATE_CHANGE		0x29	// User Sitdown or Stand
#define WIZ_LOYALTY_CHANGE		0x2A	// Nation Contribution
#define WIZ_VERSION_CHECK		0x2B	// Client version check 
#define WIZ_CRYPTION			0x2C	// Cryption
#define WIZ_USERLOOK_CHANGE		0x2D	// User Slot Item Resource Change
#define WIZ_NOTICE				0x2E	// Update Notice Alarm 
#define WIZ_PARTY				0x2F	// Party Related Packet
#define WIZ_EXCHANGE			0x30	// Exchange Related Packet
#define WIZ_MAGIC_PROCESS		0x31	// Magic Related Packet
#define WIZ_SKILLPT_CHANGE		0x32	// User changed particular skill point
#define WIZ_OBJECT_EVENT		0x33	// Map Object Event Occur ( ex : Bind Point Setting )
#define WIZ_CLASS_CHANGE		0x34	// 10 level over can change class 
#define WIZ_CHAT_TARGET			0x35	// Select Private Chanting User
#define WIZ_CONCURRENTUSER		0x36	// Current Game User Count
#define WIZ_DATASAVE			0x37	// User GameData DB Save Request
#define WIZ_DURATION			0x38	// Item Durability Change
#define WIZ_TIMENOTIFY			0x39	// Time Adaption Magic Time Notify Packet ( 2 Seconds )
#define WIZ_REPAIR_NPC			0x3A	// Item Trade, Upgrade and Repair
#define WIZ_ITEM_REPAIR			0x3B	// Item Repair Processing
#define WIZ_KNIGHTS_PROCESS		0x3C	// Knights Related Packet..
#define WIZ_ITEM_COUNT_CHANGE   0x3D    // Item cout change.  
#define WIZ_KNIGHTS_LIST		0x3E	// All Knights List Info download
#define WIZ_ITEM_REMOVE			0x3F	// Item Remove from inventory
#define WIZ_OPERATOR			0x40	// Operator Authority Packet
#define WIZ_SPEEDHACK_CHECK		0x41	// Speed Hack Using Check
#define WIZ_COMPRESS_PACKET		0x42	// Data Compressing Packet
#define WIZ_SERVER_CHECK		0x43	// Server Status Check Packet
#define WIZ_CONTINOUS_PACKET	0x44	// Region Data Packet 
#define WIZ_WAREHOUSE			0x45	// Warehouse Open, In, Out
#define WIZ_SERVER_CHANGE		0x46	// When you change the server
#define WIZ_REPORT_BUG			0x47	// Report Bug to the manager
#define WIZ_HOME				0x48    // 'Come back home' by Seo Taeji & Boys
#define WIZ_FRIEND_PROCESS		0x49	// Get the status of your friend
#define WIZ_GOLD_CHANGE			0x4A	// When you get the gold of your enemy.
#define WIZ_WARP_LIST			0x4B	// Warp List by NPC or Object
#define WIZ_VIRTUAL_SERVER		0x4C	// Battle zone Server Info packet	(IP, Port)
#define WIZ_ZONE_CONCURRENT		0x4D	// Battle zone concurrent users request packet
#define WIZ_CORPSE				0x4e	// To have your corpse have an ID on top of it.
#define WIZ_PARTY_BBS			0x4f	// For the party wanted bulletin board service..
#define WIZ_MARKET_BBS			0x50	// For the market bulletin board service...
#define WIZ_KICKOUT				0x51	// Account ID forbid duplicate connection
#define WIZ_CLIENT_EVENT		0x52	// Client Event (for quest)
#define WIZ_MAP_EVENT			0x53	// Å¬¶óÀÌ¾ðÆ®¿¡¼­ ¹«½¼ ¿¡ÄÚ·Î ¾²°í ÀÖµ¥¿ä.
#define WIZ_WEIGHT_CHANGE		0x54	// Notify change of weight
#define WIZ_SELECT_MSG			0x55	// Select Event Message...
#define WIZ_NPC_SAY				0x56	// Select Event Message...
#define WIZ_BATTLE_EVENT		0x57	// Battle Event Result
#define WIZ_AUTHORITY_CHANGE	0x58	// Authority change 
#define WIZ_EDIT_BOX			0x59	// Activate/Receive info from Input_Box.
#define WIZ_SANTA				0x5A	// Activate motherfucking Santa Claus!!! :(
#define WIZ_ITEM_UPGRADE		0x5B
#define WIZ_PACKET1				0x5C
#define WIZ_PACKET2				0x5D
#define WIZ_ZONEABILITY			0x5E	
#define WIZ_EVENT				0x5F
#define WIZ_STEALTH				0x60 // stealth related.
#define WIZ_ROOM_PACKETPROCESS	0x61 // room system
#define WIZ_ROOM				0x62
#define WIZ_PACKET3				0x63 // new clan
#define WIZ_QUEST				0x64
#define WIZ_PACKET4				0x65 // login
#define WIZ_KISS				0x66
#define WIZ_RECOMMEND_USER		0x67
#define WIZ_MERCHANT			0x68
#define WIZ_MERCHANT_INOUT		0x69
#define WIZ_SHOPPING_MALL		0x6A
#define WIZ_SERVER_INDEX		0x6B
#define WIZ_EFFECT				0x6C
#define WIZ_SIEGE				0x6D
#define WIZ_NAME_CHANGE			0x6E
#define WIZ_WEBPAGE				0x6F
#define WIZ_CAPE				0x70
#define WIZ_PREMIUM				0x71
#define WIZ_HACKTOOL			0x72
#define WIZ_RENTAL				0x73
#define WIZ_PACKET5				0x74 //süreli item
#define WIZ_CHALLENGE			0x75
#define WIZ_PET					0x76
#define WIZ_CHINA				0x77 // we shouldn't need to worry about this
#define WIZ_KING				0x78
#define WIZ_SKILLDATA			0x79
#define WIZ_PROGRAMCHECK		0x7A
#define WIZ_BIFROST				0x7B
#define WIZ_REPORT				0x7C
#define WIZ_LOGOSSHOUT			0x7D
#define WIZ_PACKET6				0x7E
#define WIZ_PACKET7				0x7F
#define WIZ_RANK				0x80
#define WIZ_STORY				0x81
#define WIZ_PACKET8				0x82
#define WIZ_PACKET9				0x83
#define WIZ_PACKET10			0x84
#define WIZ_PACKET11			0x85
#define WIZ_MINING				0x86
#define WIZ_HELMET				0x87
#define WIZ_PVP					0x88
#define WIZ_CHANGE_HAIR			0x89 // Changes hair colour/facial features at character selection
#define WIZ_PACKET12			0x8A
#define WIZ_VIPWAREHOUSE		0x8B
#define WIZ_PACKET14			0x8C
#define WIZ_PACKET15			0x8D
#define WIZ_PACKET16			0x8E
#define WIZ_PACKET17			0x8F
#define WIZ_LOADING_LOGIN		0x9F
#define WIZ_DEATH_LIST			0x90
#define WIZ_CLANPOINTS_BATTLE	0x91 // not sure
#define WIZ_GENIE				0x97
#define WIZ_SURROUNDING  		0x98
#define WIZ_USER_ACHIEVE		0x99
#define WIZ_DEBUG_STRING_PACKET 0xFE
#define WIZ_TEST_PACKET			0xFF	// Test packet

#define WIZ_XIGNCODE			0xA0
#define WIZ_VOICECHAT			0xB5  //Voice Chat
#define WIZ_CAPTCHA				0xC0