#include "Movement.h"

void Movement::Bhop()
{
	if (!Vars::Movement::bBhop)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	const bool holdingJump = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

	if (!holdingJump)
		return;

	if (l4d2::local->m_fFlags() & FL_ONGROUND)
	{
		l4d2::cmd->buttons |= IN_JUMP;
	}
	else
	{
		l4d2::cmd->buttons &= ~IN_JUMP;
	}
}

void Movement::EdgeJump()
{
	if (!Vars::Movement::bEdgeJump)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_NOCLIP)
		return;

	static int ej_tick = 0;

	if (CheckKey(Vars::Movement::kEdgeJump))
	{
		bool wasOnGround = Prediction_backup::flags & FL_ONGROUND;
		bool isOnGround = l4d2::local->m_fFlags() & FL_ONGROUND;

		if (wasOnGround && !isOnGround && moveType != MOVETYPE_LADDER)
		{
			l4d2::cmd->buttons |= IN_JUMP;
			ej_tick = I::GlobalVars->tickcount;
			Detect_EJ = true;
		}
		else
		{
			Detect_EJ = false;
		}

		// Post-jump: duck and zero movement for 14 ticks after edge jump
		if (ej_tick && I::GlobalVars->tickcount - ej_tick <= 14 && I::GlobalVars->tickcount - ej_tick > 0)
		{
			l4d2::cmd->buttons |= IN_DUCK;
			l4d2::cmd->forwardmove = 0.f;
			l4d2::cmd->sidemove = 0.f;
		}

		// Ladder exit edge jump
		if (moveType == MOVETYPE_LADDER)
		{
			bool backupOnLadder = (Prediction_backup::flags & FL_ONGROUND) == 0;
			if (backupOnLadder && isOnGround)
			{
				l4d2::cmd->buttons |= IN_JUMP;
				l4d2::cmd->buttons |= IN_DUCK;
				l4d2::cmd->forwardmove = 0.f;
				l4d2::cmd->sidemove = 0.f;
				ej_tick = I::GlobalVars->tickcount;
				Detect_EJ = true;
			}
		}
	}
	else
	{
		ej_tick = 0;
		Detect_EJ = false;
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
			Detect_LJ = true;
		}
		else
		{
			Detect_LJ = false;
		}
	}
	else
	{
		Detect_LJ = false;
	}
}

static bool mj_triggered = false;

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
			mj_triggered = true;
			Detect_MJ = true;
		}
		else if (mj_triggered && !(l4d2::local->m_fFlags() & FL_ONGROUND) && Vars::Movement::bMiniJumpHoldDuck)
		{
			l4d2::cmd->buttons |= IN_DUCK;
			Detect_MJ = false;
		}
		else
		{
			Detect_MJ = false;
		}

		if (l4d2::local->m_fFlags() & FL_ONGROUND)
			mj_triggered = false;
	}
	else
	{
		mj_triggered = false;
		Detect_MJ = false;
	}
}
