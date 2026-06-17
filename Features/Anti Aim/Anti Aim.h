#pragma once

#include "../../SDK/SDK.h"
#include "../Vars.h"

class antiaim
{
public:
	void run(CUserCmd* cmd, bool* BSendPacket);
};
namespace f { inline antiaim aa; }