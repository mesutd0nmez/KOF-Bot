#pragma once

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

