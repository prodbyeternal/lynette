#include "Anti Aim.h"

inline void FixMovement(CUserCmd* cmd, const Vector& oldAngles, float oldForward, float oldSidemove)
{
    float deltaYaw = cmd->viewangles.y - oldAngles.y;
    float f1, f2;

    if (oldAngles.y < 0.f)
        f1 = 360.0f + oldAngles.y;
    else
        f1 = oldAngles.y;

    if (cmd->viewangles.y < 0.0f)
        f2 = 360.0f + cmd->viewangles.y;
    else
        f2 = cmd->viewangles.y;

    if (f2 < f1)
        deltaYaw = fabs(f2 - f1);
    else
        deltaYaw = 360.0f - fabs(f1 - f2);

    deltaYaw = 360.0f - deltaYaw;

    float rad = DEG2RAD(deltaYaw);

    cmd->forwardmove = cos(rad) * oldForward + cos(rad + DEG2RAD(90.f)) * oldSidemove;
    cmd->sidemove = sin(rad) * oldForward + sin(rad + DEG2RAD(90.f)) * oldSidemove;
}

void antiaim::run(CUserCmd* cmd, bool* BSendPacket)
{
    if (Vars::HvH::Enabled && cmd)
    {
        auto* ent = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
        if (!ent) return;
        C_TerrorPlayer* pLocal = ent->As<C_TerrorPlayer*>();

        Vector oldView = cmd->viewangles;
        float oldF = cmd->forwardmove;
        float oldS = cmd->sidemove;

        static bool flip = false;
        flip = !flip;

        if ((cmd->buttons & IN_USE) || (cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2))
            return;

        if (pLocal == nullptr ||
            !pLocal->IsAlive() ||
            pLocal->m_MoveType() == MOVETYPE_LADDER)
            return;

        if (Vars::HvH::AntiAim)
        {
            // ---- YAW ----
            switch (Vars::HvH::RealYaw)
            {
            case 0:
                break;

            case 1:
                cmd->viewangles.y += 180.f;
                break;

            case 2:
                cmd->viewangles.y += 90.f;
                break;

            case 3:
                cmd->viewangles.y -= 90.f;
                break;

            case 4:
                cmd->viewangles.y += flip ? 90.f : -90.f;
                break;

            case 5: // spin
            {
                static float spinAngle = 0.f;
                spinAngle = fmodf(spinAngle + Vars::HvH::SpinSpeed, 360.f);
                cmd->viewangles.y = spinAngle;
                break;
            }

            case 6: // random
                cmd->viewangles.y = U::Math.RandomFloat(-180.f, 180.f);
                break;

            case 7: // fake jitter
                if (BSendPacket)
                    cmd->viewangles.y += 180.f;
                else
                    cmd->viewangles.y += flip ? 90.f : -90.f;
                break;

            default:
                break;
            }

            // ---- PITCH ----
            switch (Vars::HvH::PitchMode)
            {
            case 1: //  down
                cmd->viewangles.x = 89.f;
                break;

            case 2: //  up
                cmd->viewangles.x = -89.f;
                break;

            case 3: // zero
                cmd->viewangles.x = 0.f;
                break;

            case 4: // jitter pitch
                cmd->viewangles.x = flip ? 89.f : -89.f;
                break;

            case 5: // random pitch
                cmd->viewangles.x = U::Math.RandomFloat(-89.f, 89.f);
                break;

            default:
                break;
            }

            if (Vars::HvH::FakeAngle)
            {
                if (BSendPacket)
                    cmd->viewangles.y += Vars::HvH::FakeOffset;

                *BSendPacket = false;
            }
            else
            {
                *BSendPacket = true;
            }
        }

        // movement fix
        FixMovement(cmd, oldView, oldF, oldS);

        // viewangles clamping
        cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.f, 89.f);
        cmd->viewangles.y = std::remainder(cmd->viewangles.y, 360.f);
        cmd->viewangles.z;
    }
}