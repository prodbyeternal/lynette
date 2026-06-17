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

	static IMaterial* material = I::MaterialSystem->FindMaterial("debug/debugambientcube", "Model textures");
	static IMaterial* wireframe = I::MaterialSystem->FindMaterial("___wireframe_dx9_5", "Model textures");
	static IMaterial* MetalMat = I::MaterialSystem->FindMaterial("//platform/materials/debug/env_cubemap_model", "Model textures");
	static IMaterial* vomitboomer = I::MaterialSystem->FindMaterial(("particle/screenspaceboomervomit"), "Particle textures");

	if (pInfo.pModel && pInfo.entity_index && material)
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
				if (Vars::Chams::Players)
				{
					if (pEntity->GetClientClass()->m_ClassID == SurvivorBot
						|| pEntity->GetClientClass()->m_ClassID == CTerrorPlayer)
					{
						const bool bIsSurvivor = (pEntity->As<C_TerrorPlayer*>()->GetTeamNumber() == TEAM_SURVIVOR);
						// if it is survivor       if not 
						const Color clrTeam = bIsSurvivor ? Vars::Chams::PlayerColor : Vars::Chams::PlayerInfectedColor;

						if (bIsSurvivor)
						{
							if (pEntity->As<C_TerrorPlayer*>()->IsAlive())
							{
								OverridematerialXQZ(material, clrTeam.r(), clrTeam.g(), clrTeam.b(), clrTeam.a());
								Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
							}
							Overridematerial(material, clrTeam.r(), clrTeam.g(), clrTeam.b(), clrTeam.a());
						}
						else
						{
							if (Vars::Chams::PlayerInfected)
							{
								if (pEntity->As<C_TerrorPlayer*>()->IsAlive())
								{
									OverridematerialXQZ(material, clrTeam.r(), clrTeam.g(), clrTeam.b(), clrTeam.a());
									Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
								}
								Overridematerial(material, clrTeam.r(), clrTeam.g(), clrTeam.b(), clrTeam.a());
							}
						}
					}
				}
				if (Vars::Chams::PlayerInfected)
				{
					if (pEntity->IsZombie() || pEntity->m_usSolidFlags() == 4 || pEntity->GetClientClass()->m_ClassID == Witch)
					{
						if (pEntity->ValidEntity(pEntity->As<C_Infected*>()->m_nSequence(), pEntity->As<C_Infected*>()->m_usSolidFlags()))
						{
							OverridematerialXQZ(material, Vars::Chams::PlayerInfectedColor.r(), Vars::Chams::PlayerInfectedColor.g(), Vars::Chams::PlayerInfectedColor.b(), Vars::Chams::PlayerInfectedColor.a());
							Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
						}
						Overridematerial(material, Vars::Chams::PlayerInfectedColor.r(), Vars::Chams::PlayerInfectedColor.g(), Vars::Chams::PlayerInfectedColor.b(), Vars::Chams::PlayerInfectedColor.a());
					}
				}
				if (Vars::Chams::ViewmodelGun)
				{
					if (pEntity->GetClientClass()->m_ClassID == CTerrorViewModel)
					{
						const Color clrTeam = Vars::Chams::ViewmodelGunColor;
						Overridematerial(material, clrTeam.r(), clrTeam.g(), clrTeam.b(), clrTeam.a());
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