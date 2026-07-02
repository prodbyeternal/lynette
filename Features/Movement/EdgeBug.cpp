#include "Movement.h"
#include "../../SDK/L4D2/Interfaces/EngineTrace.h"

std::vector<Vector> ebpos;

static bool ValidateEdgeBugSurface(const Vector& origin) {
	Vector checkOrigin = origin;
	checkOrigin.z += 200.f;

	const float step = M_PI_F * 2.0f / 16.0f;
	for (float a = 0.f; a < M_PI_F * 2.0f; a += step) {
		Vector start(32.f * cosf(a) + checkOrigin.x, 32.f * sinf(a) + checkOrigin.y, checkOrigin.z);
		Vector end = start;
		end.z -= 300.f;

		Ray_t ray;
		ray.Init(start, end);
		CTraceFilterWorldAndPropsOnly filter;
		trace_t tr;
		I::EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &tr);

		if (tr.fraction != 1.f && tr.plane.normal.z < 0.6f)
			return false;
	}
	return true;
}

bool check_edge_bug(CUserCmd* cmd, bool& brk) {
	if (!l4d2::local)
		return false;

	Vector unpredicted_velocity = Prediction_backup::vec_velocity;
	Vector predicted_velocity = l4d2::local->m_vecVelocity();
	Vector unpredicted_origin = Prediction_backup::vec_origin;
	Vector predicted_origin = l4d2::local->m_vecOrigin();
	int predicted_flags = l4d2::local->m_fFlags();

	static auto Sv_gravity = I::Cvars->FindVar("sv_gravity");
	float sv_gravity = Sv_gravity->GetFloat();
	float fTickInterval = I::GlobalVars->interval_per_tick;
	float gravityvelo = (sv_gravity * 0.5f * fTickInterval) * -1.f;

	float z_vel_pred = round(predicted_velocity.z);
	float l2d_vel_pred = round(predicted_velocity.Lenght2D());

	if (z_vel_pred >= 0.f || (predicted_flags & FL_ONGROUND) || l2d_vel_pred == 0.f) {
		brk = true;
		return false;
	}

	if (unpredicted_velocity.z < gravityvelo && round(predicted_velocity.z) == round(gravityvelo) && l4d2::local->m_MoveType() != MOVETYPE_LADDER) {
		return true;
	}

	if (unpredicted_velocity.z < -6.0f && predicted_velocity.z > unpredicted_velocity.z && predicted_velocity.z < -6.0f) {
		if (unpredicted_origin.z < predicted_origin.z)
			return false;

		float velocty_before = predicted_velocity.z;

		Prediction::Begin(cmd);
		Prediction::Finish();

		float gravity_velocity_constant = roundf(-sv_gravity * fTickInterval + velocty_before);
		float post_pred_z = round(l4d2::local->m_vecVelocity().z);

		if (gravity_velocity_constant == post_pred_z) {
			if (!ValidateEdgeBugSurface(predicted_origin))
				return false;

			return true;
		}

		if (post_pred_z == 0.f && (l4d2::local->m_fFlags() & FL_ONGROUND))
			return true;

		brk = true;
		return false;
	}

	return false;
}

static void ApplyAutoStrafe(CUserCmd* predictcmd, bool duck) {
	if (duck)
		predictcmd->buttons |= IN_DUCK;
	else
		predictcmd->buttons &= ~IN_DUCK;

	Vector velocity = l4d2::local->m_vecVelocity();
	float speed = velocity.Lenght2D();
	if (speed < 1.f) return;

	static float side = 1.f;
	side = -side;

	float ideal_strafe = std::clamp(Math::Rad2Deg(atanf(15.f / speed)), 0.f, 90.f);

	predictcmd->forwardmove = 0.f;

	Vector vel_dir;
	Math::VectorAngles(velocity, vel_dir);
	float velocity_delta = std::remainderf(predictcmd->viewangles.y - vel_dir.y, 360.0f);
	float retrack = std::clamp(Math::Rad2Deg(atanf(30.f / speed)), 0.f, 90.f) * 2.f;

	if (velocity_delta <= retrack || speed <= 15.f) {
		if (-retrack <= velocity_delta || speed <= 15.0f) {
			predictcmd->sidemove = 450.f * side;
		} else {
			predictcmd->sidemove = 450.f;
		}
	} else {
		predictcmd->sidemove = -450.f;
	}
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
		float originalfmove = l4d2::cmd->forwardmove;
		float originalsmove = l4d2::cmd->sidemove;
		Vector originalangles = l4d2::cmd->viewangles;
		EdgeBug_data.StartingYaw = originalangles.y;

		int pred_rounds = 2;
		if (Vars::Movement::ExtendedEdgeBugPaths && yawdelta != 0.f)
			pred_rounds = 4;
		pred_rounds += 2; // autostrafe variants (rounds 4-5 or 2-3 depending on extended)

		int total_rounds = (Vars::Movement::ExtendedEdgeBugPaths && yawdelta != 0.f) ? 6 : 4;

		ebpos.clear();
		ebpos.push_back(l4d2::local->m_vecOrigin());

		auto prediction_custom = reinterpret_cast<CPrediction_Custom*>(I::Prediction);

		for (int round = 0; round < total_rounds; ++round) {
			if (EdgeBug_data.Ticks_Left)
				break;

			if (round >= 2 && round <= 3 && !(Vars::Movement::ExtendedEdgeBugPaths && yawdelta != 0.f)) {
				// Rounds 2-3 are autostrafe when extended paths are off
				CUserCmd predictcmd = *l4d2::cmd;
				Prediction::RestoreEntityToPredictedFrame(prediction_custom->m_Split->m_nCommandsPredicted - 1);

				for (int t = 1; t <= ticklimit; ++t) {
					ApplyAutoStrafe(&predictcmd, round == 3);

					Prediction::Begin(&predictcmd);
					Prediction::Finish();

					Vector vel = l4d2::local->m_vecVelocity();
					if (vel.z > 0.f || vel.Lenght2D() == 0.f || l4d2::local->m_MoveType() == MOVETYPE_LADDER)
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
						EdgeBug_data.Strafing = true;
						EdgeBug_data.Crouched = (round == 3);
						break;
					}
					if (br) break;
				}
			}
			else if (round >= 4) {
				// Rounds 4-5 are autostrafe when extended paths are on
				CUserCmd predictcmd = *l4d2::cmd;
				Prediction::RestoreEntityToPredictedFrame(prediction_custom->m_Split->m_nCommandsPredicted - 1);

				for (int t = 1; t <= ticklimit; ++t) {
					ApplyAutoStrafe(&predictcmd, round == 5);

					Prediction::Begin(&predictcmd);
					Prediction::Finish();

					Vector vel = l4d2::local->m_vecVelocity();
					if (vel.z > 0.f || vel.Lenght2D() == 0.f || l4d2::local->m_MoveType() == MOVETYPE_LADDER)
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
						EdgeBug_data.Strafing = true;
						EdgeBug_data.Crouched = (round == 5);
						break;
					}
					if (br) break;
				}
			}
			else if (round >= 2) {
				// Rounds 2-3 with extended paths (strafe + yaw delta)
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
						predictcmd.viewangles.y = Math::NormalizeYaw(originalangles.y + (yawdelta * t));

						if (abs(predictcmd.viewangles.y - EdgeBug_data.StartingYaw) > Vars::Movement::EdgeBugAngle)
							break;

						Prediction::Begin(&predictcmd);
						Prediction::Finish();

						Vector vel = l4d2::local->m_vecVelocity();
						if (vel.z > 0.f || vel.Lenght2D() == 0.f || l4d2::local->m_MoveType() == MOVETYPE_LADDER)
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
						if (br) break;
					}
				}
			}
			else {
				// Rounds 0-1: basic duck/no-duck, no strafe
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

					Vector vel = l4d2::local->m_vecVelocity();
					if (vel.z > 0.f || vel.Lenght2D() == 0.f || l4d2::local->m_MoveType() == MOVETYPE_LADDER)
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
					if (br) break;
				}
			}
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
