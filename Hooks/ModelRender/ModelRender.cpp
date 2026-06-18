#include "ModelRender.h"
#include "../../Features/Vars.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
using namespace Hooks;


void __fastcall ModelRender::ForcedMaterialOverride::Detour(void* ecx, void* edx, IMaterial* newMaterial, OverrideType_t nOverrideType)
{
	Table.Original<FN>(Index)(ecx, edx, newMaterial, nOverrideType);
}

void OverridematerialXQZ(IMaterial* mat, float r, float g, float b, float a)
{
	mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
	mat->ColorModulate(r / 255, g / 255, b / 255);
	mat->AlphaModulate(a / 255);
	I::ModelRender->ForcedMaterialOverride(mat);
}

void Overridematerial(IMaterial* mat, float r, float g, float b, float a)
{
	mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
	mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
	mat->ColorModulate(r / 255, g / 255, b / 255);
	mat->AlphaModulate(a / 255);
	I::ModelRender->ForcedMaterialOverride(mat);
}

void OverrideColor(IMaterial* mat, float r, float g, float b, float a)
{
	mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
	mat->ColorModulate(r / 255, g / 255, b / 255);
	mat->AlphaModulate(a / 255);
	I::ModelRender->ForcedMaterialOverride(mat);
}

void OverrideWireFrame(IMaterial* mat, float r, float g, float b, float a)
{
	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
	mat->ColorModulate(r / 255, g / 255, b / 255);
	mat->AlphaModulate(a / 255);
	I::ModelRender->ForcedMaterialOverride(mat);
}

void OverrideFlat(IMaterial* mat, float r, float g, float b, float a)
{
	mat->SetMaterialVarFlag(MATERIAL_VAR_FLAT, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
	mat->ColorModulate(r / 255, g / 255, b / 255);
	mat->AlphaModulate(a / 255);
	I::ModelRender->ForcedMaterialOverride(mat);
}

void __fastcall ModelRender::DrawModelExecute::Detour(void* ecx, void* edx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
	if (!I::EngineClient->IsInGame())
		Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);

	// Known-good base materials available in this build. We toggle material-var
	// flags on these to achieve the different chams styles instead of relying on
	// CreateMaterial() (the KeyValues patterns are stale in this build).
	static IMaterial* materialAmbient = I::MaterialSystem->FindMaterial("debug/debugambientcube", "Model textures");
	static IMaterial* materialGlow    = I::MaterialSystem->FindMaterial("dev/glow_color", "Model textures");
	static IMaterial* materialShaded  = I::MaterialSystem->FindMaterial("debug/debugdrawflat", "Model textures");
	static IMaterial* vomitboomer = I::MaterialSystem->FindMaterial("particle/screenspaceboomervomit", "Particle textures");

	// Material modes: 0 - Solid, 1 - Flat, 2 - Wireframe, 3 - Glow/Additive, 4 - Shaded
	auto pickMat = [&](int mode) -> IMaterial* {
		switch (mode)
		{
		case 3: return (materialGlow && !materialGlow->IsErrorMaterial()) ? materialGlow : materialAmbient;
		case 4: return (materialShaded && !materialShaded->IsErrorMaterial()) ? materialShaded : materialAmbient;
		default: return materialAmbient; // solid/flat/wireframe share the ambient base
		}
	};

	auto applyChams = [&](int mode, const Color& col, bool ignoreZ) {
		IMaterial* mat = pickMat(mode);
		if (!mat || mat->IsErrorMaterial())
			return;
		mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
		mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, (mode == 0 || mode == 4)); // lit for solid/shaded
		mat->SetMaterialVarFlag(MATERIAL_VAR_FLAT, mode == 1);                        // flat unlit
		mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, mode == 2);                   // wireframe
		mat->ColorModulate(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
		mat->AlphaModulate(col.a() / 255.f);
		I::ModelRender->ForcedMaterialOverride(mat);
	};

	if (pInfo.pModel && pInfo.entity_index)
	{
		if (Vars::SkyBox::Enable)
		{
			static ConVar* skyName = I::Cvars->FindVar("sv_skyname");
			if (skyName)
			{
				std::string skyBase = skyName->GetString();
				const char* suffixes[7] = { "bk", "dn", "ft", "lf", "rt", "cf", "up" };
				static IMaterial* skyMats[7] = { nullptr };
				if (!skyMats[0])
				{
					for (int i = 0; i < 7; i++)
					{
						std::string skyPath = "skybox/" + skyBase + suffixes[i];
						IMaterial* mat = I::MaterialSystem->FindMaterial(skyPath.c_str(), TEXTURE_GROUP_SKYBOX);
						if (mat && !mat->IsErrorMaterial())
							skyMats[i] = mat;
					}
				}
				for (int i = 0; i < 7; i++)
				{
					if (skyMats[i])
					{
						if (Vars::SkyBox::RainbowSkyBox)
						{
							float time = I::GlobalVars->curtime;
							float r = (sin(time * 2.0f) * 0.5f + 0.5f);
							float g = (sin(time * 2.0f + 2.0f) * 0.5f + 0.5f);
							float b = (sin(time * 2.0f + 4.0f) * 0.5f + 0.5f);
							skyMats[i]->ColorModulate(r, g, b);
						}
						else
						{
							float r = Vars::SkyBox::color.r() / 255.0f;
							float g = Vars::SkyBox::color.g() / 255.0f;
							float b = Vars::SkyBox::color.b() / 255.0f;
							skyMats[i]->ColorModulate(r, g, b);
						}
					}
				}
			}
		}

		if (vomitboomer)
		{
			vomitboomer->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, Vars::Removals::BoomerVisual);
		}

		C_BaseEntity* pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<C_BaseEntity*>();
		if (pEntity && !pEntity->IsDormant())
		{
			if (pEntity->GetClientClass())
			{
				if (Vars::Chams::Players && (pEntity->GetClientClass()->m_ClassID == SurvivorBot || pEntity->GetClientClass()->m_ClassID == CTerrorPlayer))
				{
					const bool bIsSurvivor = (pEntity->As<C_TerrorPlayer*>()->GetTeamNumber() == TEAM_SURVIVOR);

					if (bIsSurvivor)
					{
						if (pEntity->As<C_TerrorPlayer*>()->IsAlive())
						{
							if (Vars::Chams::SurvivorThroughWalls)
							{
								applyChams(Vars::Chams::SurvivorMaterial, Vars::Chams::PlayerColor, true);
								Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
							}
							applyChams(Vars::Chams::SurvivorMaterial, Vars::Chams::PlayerColor, false);
						}
					}
					else if (Vars::Chams::PlayerInfected)
					{
						// A player on the infected team (versus mode).
						if (pEntity->As<C_TerrorPlayer*>()->IsAlive())
						{
							if (Vars::Chams::SIThroughWalls)
							{
								applyChams(Vars::Chams::SIMaterial, Vars::Chams::PlayerInfectedColor, true);
								Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
							}
							applyChams(Vars::Chams::SIMaterial, Vars::Chams::PlayerInfectedColor, false);
						}
					}
				}
				else if (Vars::Chams::PlayerInfected && (pEntity->IsZombie() || pEntity->GetClientClass()->m_ClassID == Witch))
				{
					// Special Infected (Hunter, Smoker, Boomer, etc.) and the Witch.
					if (Vars::Chams::SIThroughWalls)
					{
						applyChams(Vars::Chams::SIMaterial, Vars::Chams::PlayerInfectedColor, true);
						Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
					}
					applyChams(Vars::Chams::SIMaterial, Vars::Chams::PlayerInfectedColor, false);
				}
				else if (Vars::Chams::Infected && pEntity->GetClientClass()->m_ClassID == Infected)
				{
					// Common Infected (the horde). Validate sequence/solid flags so we
					// don't paint ragdolls/dying bodies.
					C_Infected* pInfected = pEntity->As<C_Infected*>();
					if (pInfected && pInfected->ValidEntity(pInfected->m_nSequence(), pInfected->m_usSolidFlags()))
					{
						if (Vars::Chams::CIThroughWalls)
						{
							applyChams(Vars::Chams::CIMaterial, Vars::Chams::InfectedColor, true);
							Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
						}
						applyChams(Vars::Chams::CIMaterial, Vars::Chams::InfectedColor, false);
					}
				}
				else if (Vars::Chams::ViewmodelGun && pEntity->GetClientClass()->m_ClassID == CTerrorViewModel)
				{
					applyChams(Vars::Chams::GunMaterial, Vars::Chams::ViewmodelGunColor, false);
				}
				else if (Vars::Grenade::ProjectileColorChanger)
				{
					// Recolor the in-flight throwable MODELS (the bottle / jar mesh).
					// NOTE: the fire and bile SPLASH are particle systems, not models,
					// so they cannot be recolored from this model hook — only the
					// thrown object model is affected here.
					const int cid = pEntity->GetClientClass()->m_ClassID;
					if (cid == CVomitJarProjectile)
					{
						applyChams(0, Vars::Grenade::BileColor, false);
					}
					else if (cid == CMolotovProjectile)
					{
						applyChams(0, Vars::Grenade::MolotovColor, false);
					}
				}
			}
		}
	}

	Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
	I::ModelRender->ForcedMaterialOverride(nullptr);
}

void ModelRender::Init()
{
	XASSERT(Table.Init(I::ModelRender) == false);
	XASSERT(Table.Hook(&ForcedMaterialOverride::Detour, ForcedMaterialOverride::Index) == false);
	XASSERT(Table.Hook(&DrawModelExecute::Detour, DrawModelExecute::Index) == false);
}