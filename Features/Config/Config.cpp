#include "Config.h"
#include "../Vars.h"

#include <Windows.h>
#include <fstream>
#include <sstream>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Serialization helpers
// ---------------------------------------------------------------------------
namespace {

	// Each entry maps a stable string key to a pointer to a Vars field, so the
	// same table drives both saving and loading and the two can never drift.
	struct ConfigField {
		const char* key;
		enum Type { BOOL, INT, FLOAT, COLOR } type;
		void* ptr;
	};

	std::vector<ConfigField> BuildFieldTable()
	{
		std::vector<ConfigField> f;

		auto addB = [&](const char* k, bool* p)  { f.push_back({ k, ConfigField::BOOL,  p }); };
		auto addI = [&](const char* k, int* p)   { f.push_back({ k, ConfigField::INT,   p }); };
		auto addF = [&](const char* k, float* p) { f.push_back({ k, ConfigField::FLOAT, p }); };
		auto addC = [&](const char* k, Color* p) { f.push_back({ k, ConfigField::COLOR, p }); };

		// Aimbot / Hitscan
		addB("hitscan.enable",      &Vars::Hitscan::bEnable);
		addB("hitscan.autoshoot",   &Vars::Hitscan::bAutoShoot);
		addB("hitscan.silent",      &Vars::Hitscan::bSilentAim);
		addI("hitscan.fov",         &Vars::Hitscan::iFov);
		addI("hitscan.hitbox",      &Vars::Hitscan::iHitbox);
		addI("hitscan.aimmode",     &Vars::Hitscan::AimMode);
		addI("hitscan.aimkey",      &Vars::Hitscan::AimKey.m_Var);
		addB("hitscan.fovcircle",   &Vars::Hitscan::AimFovCircle);

		// Target filters
		addB("ignore.witch",        &Vars::Ignore::IgnoreWitchUntilStartled);
		addB("ignore.common",       &Vars::Ignore::IgnoreCommonInfected);
		addB("ignore.friends",      &Vars::Ignore::IgnoreFriends);
		addB("ignore.teammates",    &Vars::Ignore::IgnoreTeammates);

		// Anti-Aim
		addB("aa.enabled",          &Vars::HvH::AntiAim);
		addB("aa.fakeangle",        &Vars::HvH::FakeAngle);
		addF("aa.fakeoffset",       &Vars::HvH::FakeOffset);
		addI("aa.realyaw",          &Vars::HvH::RealYaw);
		addI("aa.pitchmode",        &Vars::HvH::PitchMode);
		addF("aa.spinspeed",        &Vars::HvH::SpinSpeed);
		addF("aa.desyncoffset",     &Vars::HvH::DesyncOffset);

		// ESP
		addB("esp.enabled",         &Vars::ESP::Enabled);
		addB("esp.players",         &Vars::ESP::players);
		addB("esp.infected",        &Vars::ESP::infected);
		addB("esp.specialinfected", &Vars::ESP::specialinfected);
		addB("esp.box",             &Vars::ESP::Box);
		addB("esp.weapons",         &Vars::ESP::Weapons);
		addB("esp.minigun",         &Vars::ESP::Minigun);
		addB("esp.ammo",            &Vars::ESP::Ammo);
		addB("esp.healthbar",       &Vars::ESP::Healthbar);
		addB("esp.name",            &Vars::ESP::Name);
		addB("esp.localesp",        &Vars::ESP::LocalESP);
		addC("esp.playercolor",     &Vars::ESP::PlayerColor);
		addC("esp.sicolor",         &Vars::ESP::PlayerInfectedColor);
		addC("esp.cicolor",         &Vars::ESP::InfectedColor);
		addB("esp.velhud",          &Vars::ESP::VelocityHUD);
		addB("esp.velgraph",        &Vars::ESP::VelocityGraph);
		addF("esp.velx",            &Vars::ESP::VelocityX);
		addF("esp.vely",            &Vars::ESP::VelocityY);

		// Viewmodel / FOV / Aspect
		addB("esp.vmchanger",       &Vars::ESP::ViewModelChanger);
		addF("esp.vm_x",            &Vars::ESP::viewmodel_x);
		addF("esp.vm_y",            &Vars::ESP::viewmodel_y);
		addF("esp.vm_z",            &Vars::ESP::viewmodel_z);
		addF("esp.vm_roll",         &Vars::ESP::viewmodel_roll);
		addB("esp.fovchanger",      &Vars::ESP::m_bFovChanger);
		addF("esp.viewfov",         &Vars::ESP::m_fViewFov);
		addF("esp.gamefov",         &Vars::ESP::m_fFov);
		addB("esp.aspectchanger",   &Vars::ESP::m_bAspectRatioChanger);
		addF("esp.aspectratio",     &Vars::ESP::m_fAspectRatio);
		addB("esp.flashlightfovchg",&Vars::ESP::m_bFlashlightFovChanger);
		addF("esp.flashlightfov",   &Vars::ESP::m_fFlashlightFov);
		addB("esp.unhidecommands",  &Vars::ESP::m_bUnhideCommands);

		// Chams
		addB("chams.players",       &Vars::Chams::Players);
		addB("chams.si",            &Vars::Chams::PlayerInfected);
		addB("chams.ci",            &Vars::Chams::Infected);
		addB("chams.viewmodelgun",  &Vars::Chams::ViewmodelGun);
		addC("chams.guncolor",      &Vars::Chams::ViewmodelGunColor);
		addI("chams.material",      &Vars::Chams::ViewmodelMaterial);
		addB("chams.throughwalls",  &Vars::Chams::ThroughWalls);
		addI("chams.survivormat",   &Vars::Chams::SurvivorMaterial);
		addI("chams.simat",         &Vars::Chams::SIMaterial);
		addI("chams.cimat",         &Vars::Chams::CIMaterial);
		addI("chams.gunmat",        &Vars::Chams::GunMaterial);
		addB("chams.survivortw",    &Vars::Chams::SurvivorThroughWalls);
		addB("chams.sitw",          &Vars::Chams::SIThroughWalls);
		addB("chams.citw",          &Vars::Chams::CIThroughWalls);
		addC("chams.playercolor",   &Vars::Chams::PlayerColor);
		addC("chams.sicolor",       &Vars::Chams::PlayerInfectedColor);
		addC("chams.cicolor",       &Vars::Chams::InfectedColor);

		// Skybox
		addB("sky.enable",          &Vars::SkyBox::Enable);
		addB("sky.rainbow",         &Vars::SkyBox::RainbowSkyBox);
		addC("sky.color",           &Vars::SkyBox::color);

		// Fog
		addB("fog.enable",          &Vars::Fog::Enable);
		addB("fog.remove",          &Vars::Fog::RemoveFog);
		addC("fog.color",           &Vars::Fog::color);

		// Grenade
		addB("nade.trajectory",     &Vars::Grenade::TrajectoryPrediction);
		addC("nade.trajcolor",      &Vars::Grenade::TrajectoryColor);
		addB("nade.projcolor",      &Vars::Grenade::ProjectileColorChanger);
		addC("nade.bilecolor",      &Vars::Grenade::BileColor);
		addC("nade.molocolor",      &Vars::Grenade::MolotovColor);
		addB("nade.molorange",      &Vars::Grenade::MolotovRangeVisual);
		addF("nade.molorangerad",   &Vars::Grenade::MolotovRangeRadius);
		addC("nade.molorangecol",   &Vars::Grenade::MolotovRangeColor);
		addB("nade.bloodcolor",     &Vars::Grenade::BloodColorChanger);
		addC("nade.bloodcol",       &Vars::Grenade::BloodColor);

		// Removals
		addB("rem.nospread",        &Vars::Removals::NoSpread);
		addB("rem.norecoil",        &Vars::Removals::NoRecoil);
		addB("rem.novisualrecoil",  &Vars::Removals::NoVisualRecoil);
		addB("rem.boomervisual",    &Vars::Removals::BoomerVisual);

		// Movement
		addB("move.bhop",           &Vars::Movement::bBhop);
		addB("move.edgejump",       &Vars::Movement::bEdgeJump);
		addI("move.edgejumpkey",    &Vars::Movement::kEdgeJump);
		addB("move.longjump",       &Vars::Movement::bLongJump);
		addI("move.longjumpkey",    &Vars::Movement::kLongJump);
		addB("move.minijump",       &Vars::Movement::bMiniJump);
		addI("move.minijumpkey",    &Vars::Movement::kMiniJump);
		addB("move.edgebug",        &Vars::Movement::bEdgeBug);
		addI("move.edgebugkey",     &Vars::Movement::kEdgeBug);
		addI("move.edgebugticks",   &Vars::Movement::EdgeBugTicks);
		addB("move.extendedpaths",  &Vars::Movement::ExtendedEdgeBugPaths);
		addI("move.edgebugpaths",   &Vars::Movement::EdgeBugPaths);
		addF("move.edgebugangle",   &Vars::Movement::EdgeBugAngle);
		addB("move.lan",            &Vars::Movement::bLAN);
		addB("move.csmovement",     &Vars::Movement::bCSGameMovement);
		addB("move.edgebugnodmg",   &Vars::Movement::EdgeBugNoDamage);
		addB("move.edgebugdebug",   &Vars::Movement::EdgeBugDebugVisual);
		addB("misc.autostrafe",     &Vars::Misc::AutoStrafe);
		addB("misc.autoshove",      &Vars::Misc::AutoShove);

		// Exploits / Misc
		addB("exploit.speedhack",   &Vars::Exploits::Speedhack);
		addI("exploit.speedkey",    &Vars::Exploits::Key.m_Var);
		addI("exploit.speedvalue",  &Vars::Exploits::SpeedHackValue);
		addB("misc.seqfreeze",      &Vars::Misc::SequenceFreezing);
		addI("misc.seqfreezekey",   &Vars::Misc::Key.m_Var);
		addI("misc.seqfreezeval",   &Vars::Misc::SequenceFreezingValue);
		addB("misc.teleport",       &Vars::Misc::Teleport);
		addI("misc.teleportkey",    &Vars::Misc::TeleportKey.m_Var);
		addB("misc.thirdperson",    &Vars::Misc::ThirdPerson);
		addI("misc.thirdpersonkey", &Vars::Misc::ThirdPersonKey.m_Var);
		addI("misc.thirdpersondist",&Vars::Misc::ThirdPersonDistance);

		return f;
	}

	std::string SanitizeName(const std::string& name)
	{
		std::string out;
		for (char c : name)
		{
			if (isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == ' ')
				out += c;
		}
		if (out.empty())
			out = "default";
		return out;
	}
}

// ---------------------------------------------------------------------------
// CConfig
// ---------------------------------------------------------------------------
void CConfig::Init()
{
	if (!m_Directory.empty())
		return;

	char modulePath[MAX_PATH] = { 0 };
	// Path relative to the game executable directory.
	GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
	std::string exePath = modulePath;
	size_t slash = exePath.find_last_of("\\/");
	std::string baseDir = (slash != std::string::npos) ? exePath.substr(0, slash) : ".";

	m_Directory = baseDir + "\\lynette_configs";
	CreateDirectoryA(m_Directory.c_str(), nullptr);

	RefreshList();
}

void CConfig::RefreshList()
{
	m_Configs.clear();
	if (m_Directory.empty())
		return;

	std::string search = m_Directory + "\\*.cfg";
	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string fileName = fd.cFileName;
				size_t dot = fileName.find_last_of('.');
				if (dot != std::string::npos)
					fileName = fileName.substr(0, dot);
				m_Configs.push_back(fileName);
			}
		} while (FindNextFileA(hFind, &fd));
		FindClose(hFind);
	}
}

bool CConfig::Save(const std::string& name)
{
	Init();

	std::string path = m_Directory + "\\" + SanitizeName(name) + ".cfg";
	std::ofstream out(path, std::ios::trunc);
	if (!out.is_open())
		return false;

	out << "# lynette config: " << name << "\n";

	for (const auto& field : BuildFieldTable())
	{
		out << field.key << "=";
		switch (field.type)
		{
		case ConfigField::BOOL:  out << (*static_cast<bool*>(field.ptr) ? "1" : "0"); break;
		case ConfigField::INT:   out << *static_cast<int*>(field.ptr); break;
		case ConfigField::FLOAT: out << *static_cast<float*>(field.ptr); break;
		case ConfigField::COLOR: out << static_cast<Color*>(field.ptr)->GetRawColor(); break;
		}
		out << "\n";
	}

	out.close();
	RefreshList();
	return true;
}

bool CConfig::Load(const std::string& name)
{
	Init();

	std::string path = m_Directory + "\\" + SanitizeName(name) + ".cfg";
	std::ifstream in(path);
	if (!in.is_open())
		return false;

	// Read all key=value pairs into a map first.
	std::unordered_map<std::string, std::string> values;
	std::string line;
	while (std::getline(in, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		size_t eq = line.find('=');
		if (eq == std::string::npos)
			continue;
		std::string key = line.substr(0, eq);
		std::string val = line.substr(eq + 1);
		// Trim trailing whitespace/CR
		while (!val.empty() && (val.back() == '\r' || val.back() == '\n' || val.back() == ' '))
			val.pop_back();
		values[key] = val;
	}
	in.close();

	for (const auto& field : BuildFieldTable())
	{
		auto it = values.find(field.key);
		if (it == values.end())
			continue;

		const std::string& v = it->second;
		try {
			switch (field.type)
			{
			case ConfigField::BOOL:  *static_cast<bool*>(field.ptr) = (v == "1" || v == "true"); break;
			case ConfigField::INT:   *static_cast<int*>(field.ptr) = std::stoi(v); break;
			case ConfigField::FLOAT: *static_cast<float*>(field.ptr) = std::stof(v); break;
			case ConfigField::COLOR: static_cast<Color*>(field.ptr)->SetRawColor(std::stoi(v)); break;
			}
		}
		catch (...) {
			// Skip malformed values rather than aborting the whole load.
		}
	}

	return true;
}
