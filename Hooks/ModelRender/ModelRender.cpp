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

	static IMaterial* materialAmbient = I::MaterialSystem->FindMaterial("debug/debugambientcube", "Model textures");
	static IMaterial* materialFlat = I::MaterialSystem->FindMaterial("debug/debugdrawflat", "Model textures");
	static IMaterial* materialWire = I::MaterialSystem->FindMaterial("___wireframe_dx9_5", "Model textures");
	static IMaterial* vomitboomer = I::MaterialSystem->FindMaterial("particle/screenspaceboomervomit", "Particle textures");

	auto applyChams = [&](IMaterial* mat, const Color& col, bool ignoreZ) {
		mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
		mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
		mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, (Vars::Chams::ViewmodelMaterial == 2));
		mat->ColorModulate(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
		mat->AlphaModulate(col.a() / 255.f);
		I::ModelRender->ForcedMaterialOverride(mat);
	};

	auto getActiveMat = []() -> IMaterial* {
		if (Vars::Chams::ViewmodelMaterial == 1) return materialFlat;
		if (Vars::Chams::ViewmodelMaterial == 2) return materialWire;
		return materialAmbient;
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
				IMaterial* chamMat = getActiveMat();
				if (chamMat)
				{
					if (Vars::Chams::Players && (pEntity->GetClientClass()->m_ClassID == SurvivorBot || pEntity->GetClientClass()->m_ClassID == CTerrorPlayer))
					{
						const bool bIsSurvivor = (pEntity->As<C_TerrorPlayer*>()->GetTeamNumber() == TEAM_SURVIVOR);
						const Color clrTeam = bIsSurvivor ? Vars::Chams::PlayerColor : Vars::Chams::PlayerInfectedColor;

						if (bIsSurvivor)
						{
							if (pEntity->As<C_TerrorPlayer*>()->IsAlive())
							{
								if (Vars::Chams::ThroughWalls)
								{
									applyChams(chamMat, clrTeam, true);
									Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
								}
								applyChams(chamMat, clrTeam, false);
							}
						}
						else if (Vars::Chams::PlayerInfected)
						{
							if (pEntity->As<C_TerrorPlayer*>()->IsAlive())
							{
								if (Vars::Chams::ThroughWalls)
								{
									applyChams(chamMat, clrTeam, true);
									Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
								}
								applyChams(chamMat, clrTeam, false);
							}
						}
					}
					else if (Vars::Chams::PlayerInfected && (pEntity->IsZombie() || pEntity->m_usSolidFlags() == 4 || pEntity->GetClientClass()->m_ClassID == Witch))
					{
						if (pEntity->ValidEntity(pEntity->As<C_Infected*>()->m_nSequence(), pEntity->As<C_Infected*>()->m_usSolidFlags()))
						{
							if (Vars::Chams::ThroughWalls)
							{
								applyChams(chamMat, Vars::Chams::PlayerInfectedColor, true);
								Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
							}
							applyChams(chamMat, Vars::Chams::PlayerInfectedColor, false);
						}
					}
					else if (Vars::Chams::ViewmodelGun && pEntity->GetClientClass()->m_ClassID == CTerrorViewModel)
					{
						applyChams(chamMat, Vars::Chams::ViewmodelGunColor, false);
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