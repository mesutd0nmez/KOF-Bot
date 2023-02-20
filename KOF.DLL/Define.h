#pragma once

#define APP_TITLE			"Discord"
#define ADDRESS_INI_FILE	".\\Pointers.ini"

#define MAX_ATTACK_RANGE		50.0f
#define MAX_VIEW_RANGE		100.0f

enum State
{
	LOST = 0,
	BOOTSTRAP = 1,
	LOGIN = 2,
	SERVER_SELECT = 3,
	CHARACTER_SELECT = 4,
	GAME = 5
};