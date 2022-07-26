#pragma once
#include <iostream>
#include "PH.hpp"

#ifdef _DEBUG
#define dbg_print(s, ...) printf(s, __VA_ARGS__)
#else
#define dbg_print(s, ...) // empty
#endif

class c_globals {
public:
	bool runningInjection = false;
	bool runningLogin = false;
	bool loginSuccess = false;
	bool injectSuccess = false;
	//bool runningInjectionBackup = false;
	bool active = true;
	int menuNumber = NULL;
	int timoutTime = 90;
};

inline c_globals globals;