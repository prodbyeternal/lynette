#include "Movement.h"

void Movement::Bhop()
{
	if (!Vars::Movement::bBhop)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	// In L4D2 SDK, get move type from netvar or cast
	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	if (!(l4d2::local->m_fFlags() & FL_ONGROUND))
		l4d2::cmd->buttons &= ~IN_JUMP;
}

void Movement::EdgeJump()
{
	if (!Vars::Movement::bEdgeJump)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	if (CheckKey(Vars::Movement::kEdgeJump))
	{
		if (Prediction_backup::flags & FL_ONGROUND && !(l4d2::local->m_fFlags() & FL_ONGROUND))
			l4d2::cmd->buttons |= IN_JUMP;
	}
}

static int saved_tick_count = 0;

void Movement::LongJump()
{
	if (!Vars::Movement::bLongJump)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	if (l4d2::local->m_fFlags() & FL_ONGROUND)
		saved_tick_count = I::GlobalVars->tickcount;

	if (I::GlobalVars->tickcount - saved_tick_count > 2)
		Should_LJ = false;
	else
		Should_LJ = true;

	if (CheckKey(Vars::Movement::kLongJump))
	{
		if (Should_LJ && !(l4d2::local->m_fFlags() & FL_ONGROUND))
		{
			l4d2::cmd->buttons |= IN_DUCK;
		}
	}
}

void Movement::MiniJump()
{
	if (!Vars::Movement::bMiniJump)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	if (CheckKey(Vars::Movement::kMiniJump))
	{
		if (Prediction_backup::flags & FL_ONGROUND && !(l4d2::local->m_fFlags() & FL_ONGROUND))
		{
			l4d2::cmd->buttons |= IN_JUMP;
			l4d2::cmd->buttons |= IN_DUCK;
		}
	}
}
