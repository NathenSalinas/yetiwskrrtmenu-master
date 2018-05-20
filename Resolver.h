#pragma once
#include <array>
#include <string>
#include <deque>
#include <algorithm>
#include "Entities.h"
#include "CommonIncludes.h"
#include "Entities.h"
#include "Vector.h"
#include <map>
#include "Interfaces.h"
#include "Hooks.h"

namespace Globals
{
	extern CUserCmd* UserCmd;
	extern IClientEntity* Target;
	extern int Shots;
	extern bool change;
	extern bool missedshots;
	extern int TargetID;
	extern bool didhitHS;
	extern int shots;
	extern bool OldLBY;
	extern bool SendPacket;
	//extern std::map<int, QAngle> storedshit;
}

class CResolverData
{
public:
	int index = 0;
	Vector realAngles, oldAngles;
public:
	CResolverData(int _index, Vector _realAngles, Vector _oldAngles)
	{
		this->index = _index;
		this->realAngles = _realAngles;
		this->oldAngles = _oldAngles;
	}
};

class CResolver
{
public:
	bool didhit;
	void Resolve(IClientEntity* pEntity, IClientEntity* pLocal, IClientEntity* resolver_data);
	void FSN(IClientEntity * pEntity, ClientFrameStage_t stage);
	void CM(IClientEntity* pEntity);
	void StoreThings(IClientEntity* pEntity);
	void anglestore(IClientEntity * pEntity);
	void StoreExtra(IClientEntity* pEntity);
	std::map<int, float>StoredAngles; //y and x lol (stored)
	std::map<int, float>NewANgles; //y and x lol (new)
	std::map<int, float>storedlby;
	std::map<int, float>panicangle;
	std::map<int, float>storedhp;
	std::map<int, float>badangle;
	std::map<int, float>storedpanic;
	std::map<int, float>newlby;
	std::map<int, float>storeddelta;
	std::map<int, float>newdelta;
	std::map<int, float>finaldelta;
	std::map<int, float>storedlbydelta;
	std::map<int, float>storedhealth;
	std::map<int, float>newlbydelta;
	std::map<int, float>finallbydelta;
	float newsimtime;
	float storedsimtime;
	bool lbyupdated;
	float storedlbyFGE;
	bool OldLBY;
	bool LBYBreakerTimer;
	bool bSwitch;
	bool LastLBYUpdateTime;
	float storedanglesFGE;
	float storedsimtimeFGE;
	bool didhitHS;
	void StoreFGE(IClientEntity* pEntity);
	void LowerBodyYawFix(IClientEntity* pEntity);
    void GetSimulationTime(IClientEntity* pEntity);
	void OldY(IClientEntity * pEntity);
	void yArraBoi(IClientEntity* pEntity);
	void FakeWalkFix(IClientEntity* pEntity, int Hitbox);
	static CResolver GetInst()
	{
		static CResolver instance;
		return instance;
	}
	std::vector<CResolverData> corrections;
	void draw_developer_data();
	//void PitchCorrection(CUserCmd * cmd, IClientEntity * pEntity, IClientEntity * pLocal);
	void PitchCorrection(IClientEntity * pEntity, IClientEntity * pLocal);
	void add_corrections();
	void apply_corrections(CUserCmd* m_pcmd);
};
extern CResolver resolver;

//void PitchCorrection(IClientEntity * pEntity);