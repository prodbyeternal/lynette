#include "BasePlayer.h"

#include "../../Features/Vars.h"

using namespace Hooks;

void __fastcall BasePlayer::CalcPlayerView::Detour(C_BasePlayer* pThis, void* edx, Vector& eyeOrigin, Vector& eyeAngles, float& fov)
{
	if (Vars::Removals::NoVisualRecoil && pThis && !pThis->deadflag()) //Thanks Spook for telling me to do it here.
	{
		const Vector vOldPunch = pThis->GetPunchAngle();

		pThis->m_vecPunchAngle().Init();
		Func.Original<FN>()(pThis, edx, eyeOrigin, eyeAngles, fov);
		pThis->m_vecPunchAngle() = vOldPunch;
	}
	else
	{
		Func.Original<FN>()(pThis, edx, eyeOrigin, eyeAngles, fov);
	}
}

void __fastcall BasePlayer::CalcViewModelView::Detour(void* ecx, void* edx, int a2, Vector& eyePosition, const Vector& eyeAngles) // thanks to motrinyt on github for this offset and code
{
	if (ecx)
	{
		if (!Vars::ESP::ViewModelChanger) // return if not enabled bla bla bla
			return 	Func.Original<FN>()(ecx, edx, a2, std::ref(eyePosition), std::ref(eyeAngles));

		Vector Forward = {}, Right = {}, Up = {};
		U::Math.angleVectors(eyeAngles, &Forward, &Right, &Up);

		Vector ViewModelPos = eyePosition + (
			(Forward * Vars::ESP::viewmodel_x/*x*/) +
			(Right * Vars::ESP::viewmodel_y/*y*/) +
			(Up * Vars::ESP::viewmodel_z/*z*/)
			);

		Vector ModifiedAngles = eyeAngles;
		ModifiedAngles.z += Vars::ESP::viewmodel_roll;

		Func.Original<FN>()(ecx, edx, a2, std::ref(ViewModelPos), std::ref(ModifiedAngles));
	}
	else
	{
		Func.Original<FN>()(ecx, edx, a2, std::ref(eyePosition), std::ref(eyeAngles));
	}
}

void BasePlayer::Init()
{
	//CalcPlayerView
	{
		using namespace CalcPlayerView;

		const FN pfCalcPlayerView = reinterpret_cast<FN>(U::Offsets.m_dwCalcPlayerView);
		XASSERT(pfCalcPlayerView == nullptr);

		if (pfCalcPlayerView)
			XASSERT(Func.Init(pfCalcPlayerView, &Detour) == false);
	}

	//CalcViewModelView
	{
		using namespace CalcViewModelView;

		const FN pfCalcViewModelView = reinterpret_cast<FN>(U::Offsets.m_dwCalcViewModelView);
		XASSERT(pfCalcViewModelView == nullptr);

		if (pfCalcViewModelView)
			XASSERT(Func.Init(pfCalcViewModelView, &Detour) == false);
	}
}