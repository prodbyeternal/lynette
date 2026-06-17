#pragma once
#include "../../SDK/SDK.h"
#include "../../SDK/L4D2/Interfaces/Prediction.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"
#include "../../SDK/L4D2/Interfaces/ClientEntityList.h"
#include "../../SDK/L4D2/Interfaces/GameMovement.h"
#include "../../SDK/L4D2/Interfaces/MoveHelper.h"

// Define structures matching the movement project but mapped to target SDK types
namespace l4d2
{
	inline C_BasePlayer* local = nullptr;
	inline CUserCmd* cmd = nullptr;
	inline float half_gravity_per_tick = 0.f;
}

namespace Prediction_backup
{
	inline Vector vec_origin;
	inline Vector vec_velocity;
	inline int flags;
}

// Memory structure layout matching L4D2 CPrediction for accessing internal fields
class CPrediction_Custom
{
public:
	std::byte pad0[0x4];
	std::uintptr_t m_hLastGround;
	bool m_bInPrediction;
	bool m_bOldCLPredictValue;
	bool m_bEnginePaused;

	int m_nPreviousStartFrame;
	int m_nIncomingPacketNumber;

	float m_flLastServerWorldTimeStamp;

	struct Split_t
	{
		Split_t()
		{
			m_bFirstTimePredicted = false;
			m_nCommandsPredicted = 0;
			m_nServerCommandsAcknowledged = 0;
			m_bPreviousAckHadErrors = false;
			m_flIdealPitch = 0.0f;
		}

		bool m_bFirstTimePredicted;
		int m_nCommandsPredicted;
		int m_nServerCommandsAcknowledged;
		int m_bPreviousAckHadErrors;
		float m_flIdealPitch;
	};

	Split_t m_Split[1];
};

namespace Prediction
{
	inline void BackupPrediction()
	{
		if (l4d2::local)
		{
			Prediction_backup::vec_origin = l4d2::local->m_vecOrigin();
			Prediction_backup::vec_velocity = l4d2::local->m_vecVelocity();
			Prediction_backup::flags = l4d2::local->m_fFlags();
		}
	}

	void Update();
	void Begin(CUserCmd* cmd);
	void Finish();
	void RestoreEntityToPredictedFrame(int i);
	int GetTickBase(CUserCmd* cmd);

	inline CMoveData mv;
	inline unsigned int* Prediction_RandomSeed;
	inline bool InPrediction;

	inline float m_flOldCurTime;
	inline float m_flOldFrameTime;
	inline int m_nOldTickCount;
}
