#include "Movement.h"

std::vector<Vector> ebpos;

bool check_edge_bug(CUserCmd* cmd, bool& brk) {
	if (!l4d2::local)
		return false;

	Vector unpredicted_velocity = Prediction_backup::vec_velocity;
	Vector predicted_velocity = l4d2::local->m_vecVelocity();
	int predicted_flags = l4d2::local->m_fFlags();

	static auto Sv_gravity = I::Cvars->FindVar("sv_gravity");
	auto sv_gravity = Sv_gravity->GetFloat();

	float z_vel_pred = round(predicted_velocity.z);
	if (z_vel_pred >= 0.f || (predicted_flags & FL_ONGROUND)) {
		brk = true;
		return false;
	}
	else if (unpredicted_velocity.z < 0.f && predicted_velocity.z > unpredicted_velocity.z && predicted_velocity.z == l4d2::half_gravity_per_tick && !(l4d2::local->m_fFlags() & FL_ONGROUND))
	{
		return true;
	}
	else if (unpredicted_velocity.z < 0.f && predicted_velocity.z > unpredicted_velocity.z && predicted_velocity.z < 0.f) {
		float z_vel = predicted_velocity.z;

		Prediction::Begin(cmd);
		Prediction::Finish();

		float rounded_vel = round(-sv_gravity * I::GlobalVars->interval_per_tick) + z_vel;
		float unpredicted_vel = round(l4d2::local->m_vecVelocity().z);

		if (rounded_vel == unpredicted_vel || (unpredicted_vel == 0.f && (l4d2::local->m_fFlags() & FL_ONGROUND))) {
			return true;
		}
		else {
			brk = true;
			return false;
		}
	}
	return false;
}

void Movement::EdgeBug()
{
	if (!Vars::Movement::bEdgeBug)
		return;

	if (!l4d2::local || !l4d2::cmd)
		return;

	int moveType = l4d2::local->m_MoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	if (CheckKey(Vars::Movement::kEdgeBug))
	{
		static auto get_yaw = I::Cvars->FindVar("m_yaw");
		static auto get_sens = I::Cvars->FindVar("sensitivity");

		const float m_yaw = get_yaw->GetFloat();
		const float sensitivity = get_sens->GetFloat();
		int ticklimit = Vars::Movement::EdgeBugTicks;
		float yawdelta = std::clamp(l4d2::cmd->mousedx * m_yaw * sensitivity, -30.f, 30.f);
		int pred_rounds = (Vars::Movement::ExtendedEdgeBugPaths && yawdelta != 0.f) ? 4 : 2;
		float originalfmove = l4d2::cmd->forwardmove;
		float originalsmove = l4d2::cmd->sidemove;
		Vector originalangles = l4d2::cmd->viewangles;
		EdgeBug_data.StartingYaw = originalangles.y;

		ebpos.clear();
		ebpos.push_back(l4d2::local->m_vecOrigin());

		// Cast prediction instance to customized mapping structure
		auto prediction_custom = reinterpret_cast<CPrediction_Custom*>(I::Prediction);

		for (int round = 0; round < pred_rounds; ++round) {
			if (round > 1) {
				float max_delta = yawdelta;
				float step = max_delta / Vars::Movement::EdgeBugPaths;
				if (step == 0.f) step = 1.f;
				for (yawdelta = step; abs(yawdelta) <= abs(max_delta); yawdelta += step) {
					CUserCmd predictcmd = *l4d2::cmd;
					if (round == 2) {
						EdgeBug_data.Crouched = true;
						predictcmd.buttons |= IN_DUCK;
						EdgeBug_data.Strafing = true;
						predictcmd.viewangles.y = originalangles.y;
						predictcmd.forwardmove = originalfmove;
						predictcmd.sidemove = originalsmove;
					}
					else if (round == 3) {
						EdgeBug_data.Crouched = false;
						predictcmd.buttons &= ~IN_DUCK;
						EdgeBug_data.Strafing = true;
						predictcmd.viewangles.y = originalangles.y;
						predictcmd.forwardmove = originalfmove;
						predictcmd.sidemove = originalsmove;
					}

					Prediction::RestoreEntityToPredictedFrame(prediction_custom->m_Split->m_nCommandsPredicted - 1);

					for (int t = 1; t <= ticklimit; ++t) {
						if (round > 1)
							predictcmd.viewangles.y = Math::NormalizeYaw(originalangles.y + (yawdelta * t));

						if (abs(predictcmd.viewangles.y - EdgeBug_data.StartingYaw) > Vars::Movement::EdgeBugAngle)
							break;

						Prediction::Begin(&predictcmd);
						Prediction::Finish();

						if (l4d2::local->m_vecVelocity().z > 0.f || l4d2::local->m_vecVelocity().Lenght2D() == 0.f || l4d2::local->m_MoveType() == MOVETYPE_LADDER)
							break;

						ebpos.push_back(l4d2::local->m_vecOrigin());

						bool br = false;

						if (check_edge_bug(&predictcmd, br)) {
							EdgeBug_data.Ticks_Left = t;
							EdgeBug_data.EbLength = t;
							EdgeBug_data.EdgebugTick = I::GlobalVars->tickcount + t;
							EdgeBug_data.DetectTick = I::GlobalVars->tickcount;
							EdgeBug_data.Forwardmove = predictcmd.forwardmove;
							EdgeBug_data.Sidemove = predictcmd.sidemove;
							EdgeBug_data.YawDelta = yawdelta;
							break;
						}

						if (br)
							break;
					}
				}
				if (EdgeBug_data.Ticks_Left)
					break;
			}
			else {
				CUserCmd predictcmd = *l4d2::cmd;
				if (round == 0) {
					EdgeBug_data.Crouched = true;
					predictcmd.buttons |= IN_DUCK;
					EdgeBug_data.Strafing = false;
					predictcmd.forwardmove = 0.f;
					predictcmd.sidemove = 0.f;
				}
				else if (round == 1) {
					EdgeBug_data.Crouched = false;
					predictcmd.buttons &= ~IN_DUCK;
					EdgeBug_data.Strafing = false;
					predictcmd.forwardmove = 0.f;
					predictcmd.sidemove = 0.f;
				}

				Prediction::RestoreEntityToPredictedFrame(prediction_custom->m_Split->m_nCommandsPredicted - 1);

				for (int t = 1; t <= ticklimit; ++t) {
		
					Prediction::Begin(&predictcmd);
					Prediction::Finish();

					if (l4d2::local->m_vecVelocity().z > 0.f || l4d2::local->m_vecVelocity().Lenght2D() == 0.f || l4d2::local->m_MoveType() == MOVETYPE_LADDER)
						break;

					ebpos.push_back(l4d2::local->m_vecOrigin());

					bool br = false;

					if (check_edge_bug(&predictcmd, br)) {
						EdgeBug_data.Ticks_Left = t;
						EdgeBug_data.EbLength = t;
						EdgeBug_data.EdgebugTick = I::GlobalVars->tickcount + t;
						EdgeBug_data.DetectTick = I::GlobalVars->tickcount;
						EdgeBug_data.Forwardmove = predictcmd.forwardmove;
						EdgeBug_data.Sidemove = predictcmd.sidemove;
						EdgeBug_data.YawDelta = 0.f;
						break;
					}

					if (br)
						break;
				}
			}

			if (EdgeBug_data.Ticks_Left)
				break;
		}
	}

	if (EdgeBug_data.Ticks_Left) {
		if (EdgeBug_data.Ticks_Left == 1) {
			Detect_EB = true;
		}

		Should_EB = true;

		if (EdgeBug_data.Crouched)
			l4d2::cmd->buttons |= IN_DUCK;
		else
			l4d2::cmd->buttons &= ~IN_DUCK;

		if (EdgeBug_data.Strafing) {
			l4d2::cmd->forwardmove = EdgeBug_data.Forwardmove;
			l4d2::cmd->sidemove = EdgeBug_data.Sidemove;
			l4d2::cmd->viewangles.y = Math::NormalizeYaw(EdgeBug_data.StartingYaw + (EdgeBug_data.YawDelta * (EdgeBug_data.EbLength - (EdgeBug_data.Ticks_Left - 1))));
		}
		else {
			l4d2::cmd->forwardmove = 0.f;
			l4d2::cmd->sidemove = 0.f;
		}
		EdgeBug_data.Ticks_Left--;
	}
	else {
		Should_EB = false;
		Detect_EB = false;
	}
}

void Movement::FixMovement(Vector& angle)
{
	if (!l4d2::local || !l4d2::cmd)
		return;

	Vector move, dir;
	Vector move_angle;
	float delta, len;

	if (!(l4d2::local->m_fFlags() & FL_ONGROUND) && l4d2::cmd->viewangles.z != 0 && l4d2::cmd->buttons & IN_ATTACK)
		l4d2::cmd->sidemove = 0;

	move = { l4d2::cmd->forwardmove, l4d2::cmd->sidemove, 0 };

	// Normalize movement
	// Let's implement inline movement normalization since Vector might not have NormalizeMovement in this SDK
	float length = move.Lenght();
	if (length > 0.0f)
	{
		move.x /= length;
		move.y /= length;
		move.z /= length;
	}
	len = length;

	if (!len)
		return;

	Math::VectorAngles(move, move_angle);

	delta = (l4d2::cmd->viewangles.y - angle.y);

	move_angle.y += delta;

	Math::AngleVectors(move_angle, &dir);

	dir *= len;

	if (l4d2::local->m_MoveType() == MOVETYPE_LADDER) {
		if (l4d2::cmd->viewangles.x >= 45 && angle.x < 45 && std::abs(delta) <= 65)
			dir.x = -dir.x;

		l4d2::cmd->forwardmove = dir.x;
		l4d2::cmd->sidemove = dir.y;

		if (l4d2::cmd->forwardmove > 200)
			l4d2::cmd->buttons |= IN_FORWARD;

		else if (l4d2::cmd->forwardmove < -200)
			l4d2::cmd->buttons |= IN_BACK;

		if (l4d2::cmd->sidemove > 200)
			l4d2::cmd->buttons |= IN_MOVERIGHT;

		else if (l4d2::cmd->sidemove < -200)
			l4d2::cmd->buttons |= IN_MOVELEFT;
	}
	else {
		if (l4d2::cmd->viewangles.x < -90 || l4d2::cmd->viewangles.x > 90)
			dir.x = -dir.x;

		l4d2::cmd->forwardmove = dir.x;
		l4d2::cmd->sidemove = dir.y;
	}

	l4d2::cmd->forwardmove = std::clamp(l4d2::cmd->forwardmove, -450.f, 450.f);
	l4d2::cmd->sidemove = std::clamp(l4d2::cmd->sidemove, -450.f, 450.f);
	l4d2::cmd->upmove = std::clamp(l4d2::cmd->upmove, -320.f, 320.f);

	if (l4d2::local->m_MoveType() != MOVETYPE_LADDER)
		l4d2::cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
}
