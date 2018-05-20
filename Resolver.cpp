#include "Resolver.h"
#include "Hooks.h"

CResolver resolver;

float OldLowerBodyYaw[64];
float YawDelta[64];
float reset[64];
float Delta[64];
float GetSimulationTime[64];
float Resolved_angles[64];
int iSmart;
static int jitter = -1;
float LatestLowerBodyYaw[64];
bool LbyUpdated[64];
bool Globals::missedshots;
float YawDifference[64];
float OldYawDifference[64]; 
float LatestLowerBodyYawUpdateTime[64];
float OldDeltaY;
float tolerance = 20.f;

Vector vecZero = Vector(0.0f, 0.0f, 0.0f);
QAngle angZero = QAngle(0.0f, 0.0f, 0.0f);

#define M_RADPI     57.295779513082f

QAngle CalcAngle(Vector src, Vector dst)
{
	Vector delta = src - dst;
	if (delta == vecZero)
	{
		return angZero;
	}

	float len = delta.Length();

	if (delta.z == 0.0f && len == 0.0f)
		return angZero;

	if (delta.y == 0.0f && delta.x == 0.0f)
		return angZero;

	QAngle angles;
	angles.x = (asinf(delta.z / delta.Length()) * M_RADPI);
	angles.y = (atanf(delta.y / delta.x) * M_RADPI);
	angles.z = 0.0f;
	if (delta.x >= 0.0f) { angles.y += 180.0f; }



	return angles;
}


///YEY REsolver
//--------------------------------------------// MrZep

float Vec2Ang(Vector Velocity)
{
	if (Velocity.x == 0 || Velocity.y == 0)
		return 0;
	float rise = Velocity.x;
	float run = Velocity.y;
	float value = rise / run;
	float theta = atan(value);
	theta = RAD2DEG(theta) + 90;
	if (Velocity.y < 0 && Velocity.x > 0 || Velocity.y < 0 && Velocity.x < 0)
		theta *= -1;
	else
		theta = 180 - theta;
	return theta;
}
void clamp(float &value)
{
	while (value > 180)
		value -= 360;
	while (value < -180)
		value += 360;
}
float clamp2(float value)
{
	while (value > 180)
		value -= 360;
	while (value < -180)
		value += 360;
	return value;
}

float difference(float first, float second)
{
	clamp(first);
	clamp(second);
	float returnval = first - second;
	if (first < -91 && second> 91 || first > 91 && second < -91)
	{
		double negativedifY = 180 - abs(first);
		double posdiffenceY = 180 - abs(second);
		returnval = negativedifY + posdiffenceY;
	}
	return returnval;
}
struct BruteforceInfo
{
	enum BruteforceStep : unsigned int {
		BF_STEP_YAW_STANDING,
		BF_STEP_YAW_FAKEWALK,
		BF_STEP_YAW_AIR,
		BF_STEP_YAW_DUCKED,
		BF_STEP_YAW_PITCH,
		BF_STEP_COUNT,
	};

	unsigned char step[BF_STEP_COUNT];
	bool changeStep[BF_STEP_COUNT];
	bool missedBySpread;
	int missedCount;
	int spentBullets;
};

int GetEstimatedServerTickCount(float latency)
{
	return (int)floorf((float)((float)(latency) / (float)((uintptr_t)&m_pGlobals->interval_per_tick)) + 0.5) + 1 + (int)((uintptr_t)&m_pGlobals->tickcount);
}
inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}

bool HasFakeHead(IClientEntity* pEntity)
{
	//lby should update if distance from lby to eye angles exceeds 35 degrees
	return abs(pEntity->GetEyeAngles()->y - pEntity->GetLowerBodyYaw()) > 35;
}
bool Lbywithin35(IClientEntity* pEntity) {
	//lby should update if distance from lby to eye angles less than 35 degrees
	return abs(pEntity->GetEyeAngles()->y - pEntity->GetLowerBodyYaw()) < 35;
}
bool IsMovingOnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetVelocity().Length2D() > 45.f && pEntity->GetFlags() & FL_ONGROUND;
}
bool IsMovingOnInAir(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return !(pEntity->GetFlags() & FL_ONGROUND);
}
bool OnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetFlags() & FL_ONGROUND;
}
bool IsFakeWalking(IClientEntity* pEntity) {
	//Check if a player is moving, but at below a velocity of 36
	return IsMovingOnGround(pEntity) && pEntity->GetVelocity().Length2D() < 36.0f;
}

void CResolver::Resolve(IClientEntity* pEntity, IClientEntity* pLocal, IClientEntity* resolver_data)
{
	bool isAlive;

	float angletolerance;
	angletolerance = pEntity->GetEyeAngles()->y + 5;
	angletolerance = pEntity->GetEyeAngles()->y - 5;
	angletolerance = pEntity->GetEyeAngles()->y + 15;
	angletolerance = pEntity->GetEyeAngles()->y - 15;
	angletolerance = pEntity->GetEyeAngles()->y - 0;
	angletolerance = pEntity->GetEyeAngles()->y + 0;
	angletolerance = pEntity->GetEyeAngles()->y + 10;
	angletolerance = pEntity->GetEyeAngles()->y - 10;
	angletolerance = pEntity->GetEyeAngles()->y + 20;
	angletolerance = pEntity->GetEyeAngles()->y - 20;
	angletolerance = pEntity->GetEyeAngles()->y + 30;
	angletolerance = pEntity->GetEyeAngles()->y - 30;
	angletolerance = pEntity->GetEyeAngles()->y + 40;
	angletolerance = pEntity->GetEyeAngles()->y - 40;
	angletolerance = pEntity->GetEyeAngles()->y + 50;
	angletolerance = pEntity->GetEyeAngles()->y - 50;
	angletolerance = pEntity->GetEyeAngles()->y + 60;
	angletolerance = pEntity->GetEyeAngles()->y - 60;
	angletolerance = pEntity->GetEyeAngles()->y + 70;
	angletolerance = pEntity->GetEyeAngles()->y - 70;
	angletolerance = pEntity->GetEyeAngles()->y + 80;
	angletolerance = pEntity->GetEyeAngles()->y - 80;
	angletolerance = pEntity->GetEyeAngles()->y + 90;
	angletolerance = pEntity->GetEyeAngles()->y - 90;
	angletolerance = pEntity->GetEyeAngles()->y + 100;
	angletolerance = pEntity->GetEyeAngles()->y - 100;
	angletolerance = pEntity->GetEyeAngles()->y + 110;
	angletolerance = pEntity->GetEyeAngles()->y - 110;
	angletolerance = pEntity->GetEyeAngles()->y + 120;
	angletolerance = pEntity->GetEyeAngles()->y - 120;
	angletolerance = pEntity->GetEyeAngles()->y + 130;
	angletolerance = pEntity->GetEyeAngles()->y - 130;
	angletolerance = pEntity->GetEyeAngles()->y + 140;
	angletolerance = pEntity->GetEyeAngles()->y - 140;
	angletolerance = pEntity->GetEyeAngles()->y + 150;
	angletolerance = pEntity->GetEyeAngles()->y - 150;
	angletolerance = pEntity->GetEyeAngles()->y + 160;
	angletolerance = pEntity->GetEyeAngles()->y - 160;
	angletolerance = pEntity->GetEyeAngles()->y + 170;
	angletolerance = pEntity->GetEyeAngles()->y - 170;
	angletolerance = pEntity->GetEyeAngles()->y + 180;
	angletolerance = pEntity->GetEyeAngles()->y - 180;
	angletolerance = pEntity->GetEyeAngles()->y + 190;
	angletolerance = pEntity->GetEyeAngles()->y - 190;
	angletolerance = pEntity->GetEyeAngles()->y + 200;
	angletolerance = pEntity->GetEyeAngles()->y - 200;
	angletolerance = pEntity->GetEyeAngles()->y + 210;
	angletolerance = pEntity->GetEyeAngles()->y - 210;
	angletolerance = pEntity->GetEyeAngles()->y + 220;
	angletolerance = pEntity->GetEyeAngles()->y - 220;
	angletolerance = pEntity->GetEyeAngles()->y + 230;
	angletolerance = pEntity->GetEyeAngles()->y - 230;
	angletolerance = pEntity->GetEyeAngles()->y + 240;
	angletolerance = pEntity->GetEyeAngles()->y - 240;
	angletolerance = pEntity->GetEyeAngles()->y + 250;
	angletolerance = pEntity->GetEyeAngles()->y - 250;
	angletolerance = pEntity->GetEyeAngles()->y + 260;
	angletolerance = pEntity->GetEyeAngles()->y - 260;
	angletolerance = pEntity->GetEyeAngles()->y + 270;
	angletolerance = pEntity->GetEyeAngles()->y - 270;
	angletolerance = pEntity->GetEyeAngles()->y + 280;
	angletolerance = pEntity->GetEyeAngles()->y - 280;
	angletolerance = pEntity->GetEyeAngles()->y + 290;
	angletolerance = pEntity->GetEyeAngles()->y - 290;
	angletolerance = pEntity->GetEyeAngles()->y + 300;
	angletolerance = pEntity->GetEyeAngles()->y - 300;
	angletolerance = pEntity->GetEyeAngles()->y + 310;
	angletolerance = pEntity->GetEyeAngles()->y - 310;
	angletolerance = pEntity->GetEyeAngles()->y + 320;
	angletolerance = pEntity->GetEyeAngles()->y - 320;
	angletolerance = pEntity->GetEyeAngles()->y + 330;
	angletolerance = pEntity->GetEyeAngles()->y - 330;
	angletolerance = pEntity->GetEyeAngles()->y + 340;
	angletolerance = pEntity->GetEyeAngles()->y - 340;
	angletolerance = pEntity->GetEyeAngles()->y + 350;
	angletolerance = pEntity->GetEyeAngles()->y - 350;
	angletolerance = pEntity->GetEyeAngles()->y + 360;
	angletolerance = pEntity->GetEyeAngles()->y - 360;
	float v23; float v24;
	double v20;

	int rnd = rand() % 360;
	if (rnd < 30)
		rnd = 30;

	if (m_pEngine->IsConnected() & pLocal->IsPlayer())
	{
		if (pLocal->IsAlive())
		{
#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
			std::string aa_info[64];
			//------bool------//

			bool Prediction; //Func: Prediction
			bool maybefakehead = 0;
			bool shouldpredict;
			//------bool------//

			//------float------//
			float org_yaw;
			float server_time = pLocal->GetTickBase() * m_pGlobals->interval_per_tick;

			//------float------//


			//------Statics------//
			static Vector Skip[65];
			static float StaticHeadAngle = 0;

			static bool GetEyeAngles[65]; //Resolve: Frame EyeAngle
			static bool GetLowerBodyYawTarget[65]; //Resolve: LowerBody
			static bool isLBYPredictited[65];
			static bool switch2;

			static float OldLowerBodyYaws[65];
			static float OldYawDeltas[65];
			static float oldTimer[65];


			auto new_yaw = org_yaw;
			bool MeetsLBYReq;
			if (pEntity->GetFlags() & FL_ONGROUND)
				MeetsLBYReq = true;
			else
				MeetsLBYReq = false;

			std::vector<int> HitBoxesToScan;

			CResolver::NewANgles[pEntity->GetIndex()];
			CResolver::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
			CResolver::newsimtime = pEntity->m_flSimulationTime();
			CResolver::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAngles()->y;
			CResolver::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
			CResolver::finaldelta[pEntity->GetIndex()] = CResolver::newdelta[pEntity->GetIndex()] - CResolver::storeddelta[pEntity->GetIndex()];
			CResolver::finallbydelta[pEntity->GetIndex()] = CResolver::newlbydelta[pEntity->GetIndex()] - CResolver::storedlbydelta[pEntity->GetIndex()];
			if (newlby == storedlby);
			CResolver::lbyupdated = false;
			if (CResolver::lbyupdated = true);
			StoreThings(pEntity);

			IClientEntity* player = (IClientEntity*)pLocal;
			IClientEntity* pLocal = m_pEntityList->GetClientEntity(m_pEngine->GetLocalPlayer());
			INetChannelInfo *nci = m_pEngine->GetNetChannelInfo();
			Vector* eyeAngles = player->GetEyeAngles();

			if (pEntity->GetFlags() & FL_ONGROUND)
				MeetsLBYReq = true;
			else
				MeetsLBYReq = false;

			StoreFGE(pEntity);

			float Delta[64];
			float OldLowerBodyYaw;
			float Resolved_angles[64];
			static Vector vLast[65];
			static bool bShotLastTime[65];
			static bool bJitterFix[65];

			float prevlby = 0.f;
			int avg = 1;
			int count = 1;
			static float LatestLowerBodyYawUpdateTime[55];
			static float LatestLowerBodyYawUpdateTime1[55];

			static float time_at_update[65];
			float kevin[64];
			static bool bLowerBodyIsUpdated = false;

			if (pEntity->GetLowerBodyYaw() != kevin[pEntity->GetIndex()])
				bLowerBodyIsUpdated = true;
			else
				bLowerBodyIsUpdated = false;

			if (pEntity->GetVelocity().Length2D() > 0.1)
			{
				kevin[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();//storing their moving lby for later
				LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->m_flSimulationTime() + 0.22;
			}
			bool bHasAA;
			bool bSpinbot;


			bool IsMoving;

			if (pEntity->GetVelocity().Length2D() >= 0.4)
				IsMoving = true;
			else
				IsMoving = false;

			bool tolerantLBY;
			if (!MeetsLBYReq)
				if (pEntity->GetLowerBodyYaw() + 35)
					tolerantLBY = true;
				else
					tolerantLBY = false;

			bool IsBreathing;
			{
				bool idk = 0;
				{
					if (pEntity->GetEyePosition().y)
						idk = true;
					else
						idk = false;
				}

				server_time >= idk;
				if (server_time >= idk + 0.705f)
				{
					IsBreathing = true;
				}
				else
					IsBreathing = false;

			}

			float healthbase = pEntity->GetHealth();
			float rnde = rand() % 160;
			if (rnde < 30)
				rnde = 30;


			bool IsStatic;
			{
				bool breathing = false;
				if (IsBreathing)
				{
					pEntity->GetEyePosition();
					IsStatic = true;
				}
				else
					IsStatic = false;
			}

			bool wrongsideleft;

			bool wrongsideright;

			float panicflip = CResolver::panicangle[pEntity->GetIndex()] + 30;
			bool shouldpanic;

			pEntity->GetChokedPackets();


			float yaw = pEntity->GetLowerBodyYaw();





			bool IsFast;

			if (pEntity->GetFlags() & FL_ONGROUND & pEntity->GetVelocity().Length2D() >= 170.5)
				IsFast = true;
			else
				IsFast = false;


			int flip = (int)floorf(m_pGlobals->curtime / 0.34) % 3;
			static bool bFlipYaw;
			float flInterval = m_pGlobals->interval_per_tick;

			if (CResolver::storedlby[pEntity->GetIndex()] > yaw + 150)
				lbyupdated = true;
			{
				if (Menu::Window.RageBotTab.AntiAimCorrection.GetState())
				{
					std::vector<int> HitBoxesToScan;
					if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] < pEntity->m_flSimulationTime() || bLowerBodyIsUpdated)
					{
						pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw();
						LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->m_flSimulationTime() + 1.1;
					}
					else {

						if (IsMovingOnGround(pEntity))
						{
							if (IsFakeWalking(pEntity))
							{
								HitBoxesToScan.clear();
								HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
								HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
								HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
								HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
								HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
							}
							pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw();
						}
						else if (IsMovingOnInAir(pEntity))
						{
							switch (game::globals.Shots % 4)//logging hits for everyhitgroup//not anymore
							{
							case 0:
								pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 45;
								break;
							case 1:
								pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 45;
								break;
							case 2:
								pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 90;
								break;
							case 3:
								pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 100;
								break;

							}
						}
						else
						{
							if (HasFakeHead(pEntity))
							{
								pEntity->GetEyeAngles()->y = pEntity->GetEyeAngles()->y - pEntity->GetLowerBodyYaw();
							}

							if (IsMovingOnGround(pEntity))
								pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw();
							else if (MeetsLBYReq && !IsMoving && pEntity->GetHealth() > CResolver::storedhealth[pEntity->GetIndex()])
							{
								if ((pEntity->GetEyeAngles()->y + 180.0) <= 180.0)
								{
									if (angletolerance < -180.0)
										angletolerance = angletolerance + 360.0;
								}
								else
								{
									angletolerance = angletolerance - 360.0;
								}
								v23 = angletolerance - pEntity->GetLowerBodyYaw();
								if (v23 <= 180.0)
								{
									if (v23 < -180.0)
										v23 = v23 + 360.0;
								}
								else
								{
									v23 = v23 - 360.0;
								}
								if (v23 >= 0.0)
									v24 = RandomFloat(0.0, v23 / 2);
								else
									v24 = RandomFloat(v23 / 2, 0.0);
								v20 = v24 + pEntity->GetEyeAngles()->y;
								pEntity->GetEyeAngles()->y = v20;
								{
									if (Lbywithin35(pEntity))
									{
										switch (Globals::missedshots % 5)

										{
										case 1:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 1;
											break;
										case 2:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 1;
											break;
										case 3:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 2;
											break;
										case 4:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 2;
											break;
										case 5:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 3;
											break;
										case 6:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 3;
											break;
										case 7:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 4;
											break;
										case 8:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 4;
											break;
										case 9:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 5;
											break;
										case 10:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 5;
											break;
										case 11:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 6;
											break;
										case 12:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 6;
											break;
										case 13:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 7;
											break;
										case 14:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 7;
											break;
										case 15:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 8;
											break;
										case 16:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 8;
											break;
										case 17:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 9;
											break;
										case 18:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 9;
											break;
										case 19:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 10;
											break;
										case 20:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 10;
											break;
										case 21:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 11;
											break;
										case 22:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 11;
											break;
										case 23:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 12;
											break;
										case 24:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 12;
											break;
										case 25:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 13;
											break;
										case 26:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 13;
											break;
										case 27:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 14;
											break;
										case 28:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 14;
											break;
										case 29:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 15;
											break;
										case 30:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 15;
											break;
										case 31:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 16;
											break;
										case 32:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 16;
											break;
										case 33:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 17;
											break;
										case 34:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 17;
											break;
										case 35:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 18;
											break;
										case 36:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 18;
											break;
										case 37:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 19;
											break;
										case 38:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 19;
											break;
										case 39:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 20;
											break;
										case 40:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 20;
											break;
										case 41:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 21;
											break;
										case 42:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 21;
											break;
										case 43:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 22;
											break;
										case 44:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 22;
											break;
										case 45:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 23;
											break;
										case 46:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 23;
											break;
										case 47:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 24;
											break;
										case 48:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 24;
											break;
										case 49:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 25;
											break;
										case 50:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 25;
											break;
										case 51:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 26;
											break;
										case 52:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 26;
											break;
										case 53:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 27;
											break;
										case 54:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 27;
											break;
										case 55:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 28;
											break;
										case 56:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 28;
											break;
										case 57:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 29;
											break;
										case 58:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 29;
											break;
										case 59:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 30;
											break;
										case 60:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 30;
											break;
										case 61:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 31;
											break;
										case 62:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 31;
											break;
										case 63:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 32;
											break;
										case 64:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 32;
											break;
										case 65:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 33;
											break;
										case 66:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 33;
											break;
										case 67:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 34;
											break;
										case 68:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 34;
											break;
										case 69:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 35;
											break;
										case 70:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 35;
											break;
										case 71:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 36;
											break;
										case 72:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 36;
											break;
										case 73:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 37;
											break;
										case 74:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 37;
											break;
										case 75:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 38;
											break;
										case 76:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 38;
											break;
										case 77:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 39;
											break;
										case 78:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 39;
											break;
										case 79:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 40;
											break;
										case 80:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 40;
											break;
										case 81:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 41;
											break;
										case 82:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 41;
											break;
										case 83:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 42;
											break;
										case 84:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 42;
											break;
										case 85:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 43;
											break;
										case 86:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 43;
											break;
										case 87:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 44;
											break;
										case 88:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 44;
											break;
										case 89:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 45;
											break;
										case 90:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 45;
											break;
										case 91:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 46;
											break;
										case 92:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 46;
											break;
										case 93:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 47;
											break;
										case 94:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 47;
											break;
										case 95:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 48;
											break;
										case 96:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 48;
											break;
										case 97:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 49;
											break;
										case 98:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 49;
											break;
										case 99:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 50;
											break;
										case 100:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 50;
											break;
										case 101:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 51;
											break;
										case 102:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 51;
											break;
										case 103:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 52;
											break;
										case 104:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 52;
											break;
										case 105:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 53;
											break;
										case 106:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 53;
											break;
										case 107:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 54;
											break;
										case 108:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 54;
											break;
										case 109:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 55;
											break;
										case 110:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 55;
											break;
										case 111:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 56;
											break;
										case 112:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 56;
											break;
										case 113:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 57;
											break;
										case 114:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 57;
											break;
										case 115:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 58;
											break;
										case 116:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 58;
											break;
										case 117:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 59;
											break;
										case 118:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 59;
											break;
										case 119:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 60;
											break;
										case 120:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 60;
											break;
										case 121:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 61;
											break;
										case 122:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 61;
											break;
										case 123:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 62;
											break;
										case 124:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 62;
											break;
										case 125:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 63;
											break;
										case 126:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 63;
											break;
										case 127:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 64;
											break;
										case 128:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 64;
											break;
										case 129:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 65;
											break;
										case 130:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 65;
											break;
										case 131:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 66;
											break;
										case 132:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 66;
											break;
										case 133:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 67;
											break;
										case 134:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 67;
											break;
										case 135:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 68;
											break;
										case 136:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 68;
											break;
										case 137:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 69;
											break;
										case 138:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 69;
											break;
										case 139:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 70;
											break;
										case 140:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 70;
											break;
										case 141:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 71;
											break;
										case 142:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 71;
											break;
										case 143:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 72;
											break;
										case 144:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 72;
											break;
										case 145:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 73;
											break;
										case 146:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 73;
											break;
										case 147:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 74;
											break;
										case 148:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 74;
											break;
										case 149:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 75;
											break;
										case 150:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 75;
											break;
										case 151:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 76;
											break;
										case 152:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 76;
											break;
										case 153:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 77;
											break;
										case 154:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 77;
											break;
										case 155:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 78;
											break;
										case 156:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 78;
											break;
										case 157:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 79;
											break;
										case 158:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 79;
											break;
										case 159:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 80;
											break;
										case 160:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 80;
											break;
										case 161:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 81;
											break;
										case 162:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 81;
											break;
										case 163:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 82;
											break;
										case 164:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 82;
											break;
										case 165:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 83;
											break;
										case 166:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 83;
											break;
										case 167:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 84;
											break;
										case 168:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 84;
											break;
										case 169:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 85;
											break;
										case 170:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 85;
											break;
										case 171:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 86;
											break;
										case 172:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 86;
											break;
										case 173:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 87;
											break;
										case 174:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 87;
											break;
										case 175:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 88;
											break;
										case 176:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 88;
											break;
										case 177:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 89;
											break;
										case 178:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 89;
											break;
										case 179:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 90;
											break;
										case 180:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 90;
											break;
										case 181:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 91;
											break;
										case 182:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 91;
											break;
										case 183:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 92;
											break;
										case 184:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 92;
											break;
										case 185:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 93;
											break;
										case 186:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 93;
											break;
										case 187:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 94;
											break;
										case 188:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 94;
											break;
										case 189:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 95;
											break;
										case 190:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 95;
											break;
										case 191:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 96;
											break;
										case 192:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 96;
											break;
										case 193:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 97;
											break;
										case 194:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 97;
											break;
										case 195:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 98;
											break;
										case 196:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 98;
											break;
										case 197:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 99;
											break;
										case 198:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 99;
											break;
										case 199:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 100;
											break;
										case 200:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 100;
											break;
										case 201:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 101;
											break;
										case 202:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 101;
											break;
										case 203:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 102;
											break;
										case 204:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 102;
											break;
										case 205:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 103;
											break;
										case 206:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 103;
											break;
										case 207:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 104;
											break;
										case 208:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 104;
											break;
										case 209:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 105;
											break;
										case 210:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 105;
											break;
										case 211:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 106;
											break;
										case 212:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 106;
											break;
										case 213:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 107;
											break;
										case 214:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 107;
											break;
										case 215:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 108;
											break;
										case 216:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 108;
											break;
										case 217:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 109;
											break;
										case 218:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 109;
											break;
										case 219:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 110;
											break;
										case 220:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 110;
											break;
										case 221:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 111;
											break;
										case 222:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 111;
											break;
										case 223:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 112;
											break;
										case 224:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 112;
											break;
										case 225:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 113;
											break;
										case 226:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 113;
											break;
										case 227:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 114;
											break;
										case 228:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 114;
											break;
										case 229:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 115;
											break;
										case 230:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 115;
											break;
										case 231:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 116;
											break;
										case 232:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 116;
											break;
										case 233:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 117;
											break;
										case 234:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 117;
											break;
										case 235:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 118;
											break;
										case 236:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 118;
											break;
										case 237:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 119;
											break;
										case 238:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 119;
											break;
										case 239:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 120;
											break;
										case 240:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 120;
											break;
										case 241:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 121;
											break;
										case 242:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 121;
											break;
										case 243:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 122;
											break;
										case 244:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 122;
											break;
										case 245:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 123;
											break;
										case 246:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 123;
											break;
										case 247:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 124;
											break;
										case 248:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 124;
											break;
										case 249:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 125;
											break;
										case 250:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 125;
											break;
										case 251:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 126;
											break;
										case 252:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 126;
											break;
										case 253:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 127;
											break;
										case 254:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 127;
											break;
										case 255:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 128;
											break;
										case 256:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 128;
											break;
										case 257:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 129;
											break;
										case 258:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 129;
											break;
										case 259:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 130;
											break;
										case 260:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 130;
											break;
										case 261:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 131;
											break;
										case 262:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 131;
											break;
										case 263:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 132;
											break;
										case 264:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 132;
											break;
										case 265:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 133;
											break;
										case 266:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 133;
											break;
										case 267:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 134;
											break;
										case 268:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 134;
											break;
										case 269:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 135;
											break;
										case 270:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 135;
											break;
										case 271:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 136;
											break;
										case 272:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 136;
											break;
										case 273:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 137;
											break;
										case 274:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 137;
											break;
										case 275:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 138;
											break;
										case 276:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 138;
											break;
										case 277:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 139;
											break;
										case 278:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 139;
											break;
										case 279:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 140;
											break;
										case 280:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 140;
											break;
										case 281:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 141;
											break;
										case 282:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 141;
											break;
										case 283:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 142;
											break;
										case 284:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 142;
											break;
										case 285:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 143;
											break;
										case 286:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 143;
											break;
										case 287:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 144;
											break;
										case 288:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 144;
											break;
										case 289:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 145;
											break;
										case 290:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 145;
											break;
										case 291:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 146;
											break;
										case 292:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 146;
											break;
										case 293:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 147;
											break;
										case 294:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 147;
											break;
										case 295:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 148;
											break;
										case 296:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 148;
											break;
										case 297:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 149;
											break;
										case 298:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 149;
											break;
										case 299:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 150;
											break;
										case 300:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 150;
											break;
										case 301:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 151;
											break;
										case 302:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 151;
											break;
										case 303:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 152;
											break;
										case 304:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 152;
											break;
										case 305:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 153;
											break;
										case 306:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 153;
											break;
										case 307:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 154;
											break;
										case 308:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 154;
											break;
										case 309:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 155;
											break;
										case 310:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 155;
											break;
										case 311:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 156;
											break;
										case 312:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 156;
											break;
										case 313:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 157;
											break;
										case 314:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 157;
											break;
										case 315:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 158;
											break;
										case 316:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 158;
											break;
										case 317:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 159;
											break;
										case 318:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 159;
											break;
										case 319:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 160;
											break;
										case 320:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 160;
											break;
										case 321:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 161;
											break;
										case 322:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 161;
											break;
										case 323:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 162;
											break;
										case 324:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 162;
											break;
										case 325:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 163;
											break;
										case 326:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 163;
											break;
										case 327:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 164;
											break;
										case 328:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 164;
											break;
										case 329:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 165;
											break;
										case 330:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 165;
											break;
										case 331:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 166;
											break;
										case 332:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 166;
											break;
										case 333:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 167;
											break;
										case 334:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 167;
											break;
										case 335:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 168;
											break;
										case 336:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 168;
											break;
										case 337:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 169;
											break;
										case 338:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 169;
											break;
										case 339:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 170;
											break;
										case 340:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 171;
											break;
										case 341:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 171;
											break;
										case 342:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 172;
											break;
										case 343:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 172;
											break;
										case 344:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 173;
											break;
										case 345:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 173;
											break;
										case 346:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 174;
											break;
										case 347:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 174;
											break;
										case 348:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 175;
											break;
										case 349:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 175;
											break;
										case 350:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 176;
											break;
										case 351:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 176;
											break;
										case 352:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 177;
											break;
										case 353:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 178;
											break;
										case 354:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 178;
											break;
										case 355:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 179;
											break;
										case 356:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 179;
											break;
										case 357:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 180;
											break;
										case 358:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 180;
											break;
										case 359:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 181;
											break;
										case 360:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 181;
											break;
										case 361:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 182;
											break;
										case 362:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 182;
											break;
										case 363:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 182;
											break;
										case 364:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 183;
											break;
										case 365:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 183;
											break;
										case 366:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 184;
											break;
										case 367:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 184;
											break;
										case 368:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 185;
											break;
										case 369:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 185;
											break;
										case 370:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 186;
											break;
										case 371:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 186;
											break;
										case 372:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 187;
											break;
										case 373:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 187;
											break;
										case 374:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 188;
											break;
										case 375:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 188;
											break;
										case 376:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 189;
											break;
										case 377:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 189;
											break;
										case 379:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 190;
											break;
										case 380:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 190;
											break;
										case 381:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 191;
											break;
										case 382:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 191;
											break;
										case 383:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 192;
											break;
										case 384:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 193;
											break;
										case 385:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 194;
											break;
										case 386:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 194;
											break;
										case 387:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 195;
											break;
										case 388:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 195;
											break;
										case 389:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 196;
											break;
										case 390:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 196;
											break;
										case 391:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 197;
											break;
										case 392:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 197;
											break;
										case 393:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 198;
											break;
										case 394:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 198;
											break;
										case 395:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 199;
											break;
										case 396:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 199;
											break;
										case 397:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 200;
											break;
										case 398:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 200;
											break;
										case 399:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 201;
											break;
										case 400:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 201;
											break;
										case 401:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 202;
											break;
										case 402:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 202;
											break;
										case 403:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 203;
											break;
										case 404:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 203;
											break;
										case 405:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 204;
											break;
										case 406:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 204;
											break;
										case 407:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 205;
											break;
										case 408:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 205;
											break;
										case 409:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 206;
											break;
										case 410:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 206;
											break;
										case 411:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 207;
											break;
										case 412:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 207;
											break;
										case 413:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 208;
											break;
										case 414:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 208;
											break;
										case 415:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 209;
											break;
										case 416:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 209;
											break;
										case 417:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 210;
											break;
										case 418:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 210;
											break;
										case 419:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 211;
											break;
										case 420:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 211;
											break;
										case 421:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 212;
											break;
										case 422:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 212;
											break;
										case 423:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 213;
											break;
										case 424:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 213;
											break;
										case 425:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 214;
											break;
										case 426:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 214;
											break;
										case 427:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 215;
											break;
										case 428:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 215;
											break;
										case 429:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 216;
											break;
										case 430:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 216;
											break;
										case 431:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 217;
											break;
										case 432:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 217;
											break;
										case 433:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 218;
											break;
										case 434:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 218;
											break;
										case 435:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 219;
											break;
										case 436:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 219;
											break;
										case 437:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 220;
											break;
										case 438:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 220;
											break;
										case 439:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 221;
											break;
										case 440:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 221;
											break;
										case 441:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 222;
											break;
										case 442:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 222;
											break;
										case 443:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 223;
											break;
										case 444:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 223;
											break;
										case 445:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 224;
											break;
										case 446:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 224;
											break;
										case 447:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 225;
											break;
										case 448:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 225;
											break;
										case 449:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 226;
											break;
										case 450:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 226;
											break;
										case 451:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 227;
											break;
										case 452:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 227;
											break;
										case 453:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 228;
											break;
										case 454:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 228;
											break;
										case 455:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 229;
											break;
										case 456:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 229;
											break;
										case 457:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 230;
											break;
										case 458:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 230;
											break;
										case 459:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 231;
											break;
										case 460:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 231;
											break;
										case 461:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 232;
											break;
										case 462:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 232;
											break;
										case 463:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 233;
											break;
										case 464:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 233;
											break;
										case 465:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 234;
											break;
										case 466:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 234;
											break;
										case 467:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 235;
											break;
										case 468:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 235;
											break;
										case 469:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 236;
											break;
										case 470:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 236;
											break;
										case 471:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 237;
											break;
										case 472:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 237;
											break;
										case 473:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 238;
											break;
										case 474:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 238;
											break;
										case 475:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 239;
											break;
										case 476:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 239;
											break;
										case 477:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 240;
											break;
										case 478:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 240;
											break;
										case 479:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 241;
											break;
										case 480:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 241;
											break;
										case 481:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 242;
											break;
										case 482:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 242;
											break;
										case 483:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 243;
											break;
										case 484:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 243;
											break;
										case 485:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 244;
											break;
										case 486:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 244;
											break;
										case 487:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 245;
											break;
										case 488:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 246;
											break;
										case 489:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 246;
											break;
										case 490:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 247;
											break;
										case 491:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 247;
											break;
										case 492:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 248;
											break;
										case 493:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 248;
											break;
										case 494:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 249;
											break;
										case 495:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 249;
											break;
										case 496:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 250;
											break;
										case 497:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 250;
											break;
										case 498:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 251;
											break;
										case 499:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 251;
											break;
										case 500:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 252;
											break;
										case 501:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 252;
											break;
										case 502:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 253;
											break;
										case 503:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 253;
											break;
										case 504:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 254;
											break;
										case 505:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 254;
											break;
										case 506:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 255;
											break;
										case 507:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 255;
											break;
										case 508:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 256;
											break;
										case 509:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 256;
											break;
										case 510:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 257;
											break;
										case 511:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 257;
											break;
										case 512:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 258;
											break;
										case 513:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 258;
											break;
										case 514:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 259;
											break;
										case 515:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 259;
											break;
										case 516:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 260;
											break;
										case 517:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 260;
											break;
										case 518:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 261;
											break;
										case 519:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 261;
											break;
										case 520:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 262;
											break;
										case 521:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 262;
											break;
										case 522:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 263;
											break;
										case 523:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 263;
											break;
										case 524:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 264;
											break;
										case 525:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 264;
											break;
										case 526:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 265;
											break;
										case 527:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 265;
											break;
										case 528:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 266;
											break;
										case 529:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 266;
											break;
										case 530:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 267;
											break;
										case 531:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 267;
											break;
										case 532:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 268;
											break;
										case 533:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 268;
											break;
										case 534:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 269;
											break;
										case 535:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 269;
											break;
										case 536:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 270;
											break;
										case 537:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 270;
											break;
										case 538:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 271;
											break;
										case 539:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 271;
											break;
										case 540:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 272;
											break;
										case 541:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 272;
											break;
										case 542:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 273;
											break;
										case 543:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 273;
											break;
										case 544:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 274;
											break;
										case 545:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 274;
											break;
										case 546:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 275;
											break;
										case 547:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 275;
											break;
										case 548:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 276;
											break;
										case 549:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 277;
											break;
										case 550:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 277;
											break;
										case 551:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 278;
											break;
										case 552:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 279;
											break;
										case 553:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 279;
											break;
										case 554:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 280;
											break;
										case 555:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 280;
											break;
										case 556:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 281;
											break;
										case 557:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 281;
											break;
										case 558:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 282;
											break;
										case 559:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 282;
											break;
										case 560:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 283;
											break;
										case 561:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 283;
											break;
										case 562:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 284;
											break;
										case 563:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 284;
											break;
										case 564:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 285;
											break;
										case 565:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 285;
											break;
										case 566:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 286;
											break;
										case 567:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 286;
											break;
										case 568:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 287;
											break;
										case 569:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 287;
											break;
										case 570:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 288;
											break;
										case 571:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 288;
											break;
										case 572:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 289;
											break;
										case 573:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 289;
											break;
										case 574:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 290;
											break;
										case 575:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 290;
											break;
										case 576:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 291;
											break;
										case 577:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 291;
											break;
										case 578:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 292;
											break;
										case 579:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 292;
											break;
										case 580:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 293;
											break;
										case 581:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 293;
											break;
										case 582:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 294;
											break;
										case 583:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 294;
											break;
										case 584:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 295;
											break;
										case 585:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 295;
											break;
										case 586:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 296;
											break;
										case 587:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 296;
											break;
										case 588:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 297;
											break;
										case 589:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 297;
											break;
										case 590:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 298;
											break;
										case 591:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 298;
											break;
										case 592:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 299;
											break;
										case 593:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 299;
											break;
										case 594:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 300;
											break;
										case 595:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 300;
											break;
										case 596:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 301;
											break;
										case 597:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 301;
											break;
										case 598:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 302;
											break;
										case 599:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 302;
											break;
										case 600:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 303;
											break;
										case 601:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 303;
											break;
										case 602:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 304;
											break;
										case 603:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 304;
											break;
										case 604:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 305;
											break;
										case 605:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 305;
											break;
										case 606:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 306;
											break;
										case 607:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 306;
											break;
										case 608:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 307;
											break;
										case 609:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 307;
											break;
										case 610:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 308;
											break;
										case 611:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 308;
											break;
										case 612:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 309;
											break;
										case 613:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 309;
											break;
										case 614:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 310;
											break;
										case 615:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 310;
											break;
										case 616:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 311;
											break;
										case 617:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 311;
											break;
										case 618:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 312;
											break;
										case 619:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 312;
											break;
										case 620:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 313;
											break;
										case 621:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 313;
											break;
										case 622:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 314;
											break;
										case 623:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 314;
											break;
										case 624:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 315;
											break;
										case 625:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 315;
											break;
										case 626:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 316;
											break;
										case 627:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 316;
											break;
										case 628:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 317;
											break;
										case 629:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 317;
											break;
										case 630:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 318;
											break;
										case 631:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 318;
											break;
										case 632:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 319;
											break;
										case 633:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 319;
											break;
										case 634:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 320;
											break;
										case 635:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 320;
											break;
										case 636:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 321;
											break;
										case 637:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 321;
											break;
										case 638:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 322;
											break;
										case 639:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 322;
											break;
										case 640:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 323;
											break;
										case 641:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 323;
											break;
										case 642:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 324;
											break;
										case 643:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 324;
											break;
										case 644:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 325;
											break;
										case 645:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 325;
											break;
										case 646:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 326;
											break;
										case 647:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 326;
											break;
										case 648:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 327;
											break;
										case 649:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 327;
											break;
										case 650:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 328;
											break;
										case 651:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 328;
											break;
										case 652:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 329;
											break;
										case 653:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 329;
											break;
										case 654:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 330;
											break;
										case 655:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 330;
											break;
										case 656:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 331;
											break;
										case 657:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 331;
											break;
										case 658:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 332;
											break;
										case 659:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 332;
											break;
										case 660:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 333;
											break;
										case 661:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 333;
											break;
										case 662:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 334;
											break;
										case 663:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 334;
											break;
										case 664:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 335;
											break;
										case 665:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 335;
											break;
										case 666:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 336;
											break;
										case 667:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 336;
											break;
										case 668:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 337;
											break;
										case 669:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 337;
											break;
										case 670:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 338;
											break;
										case 671:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 338;
											break;
										case 672:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 339;
											break;
										case 673:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 339;
											break;
										case 674:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 340;
											break;
										case 675:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 340;
											break;
										case 676:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 341;
											break;
										case 677:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 341;
											break;
										case 678:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 342;
											break;
										case 679:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 342;
											break;
										case 680:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 343;
											break;
										case 681:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 343;
											break;
										case 682:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 344;
											break;
										case 683:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 344;
											break;
										case 684:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 345;
											break;
										case 685:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 345;
											break;
										case 686:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 346;
											break;
										case 687:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 346;
											break;
										case 688:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 347;
											break;
										case 689:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 347;
											break;
										case 690:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 348;
											break;
										case 691:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 348;
											break;
										case 692:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 349;
											break;
										case 693:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 350;
											break;
										case 694:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 351;
											break;
										case 695:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 351;
											break;
										case 696:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 352;
											break;
										case 697:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 352;
											break;
										case 698:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 353;
											break;
										case 699:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 353;
											break;
										case 700:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 354;
											break;
										case 701:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 354;
											break;
										case 702:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 355;
											break;
										case 703:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 355;
											break;
										case 704:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 356;
											break;
										case 705:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 356;
											break;
										case 706:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 357;
											break;
										case 707:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 357;
											break;
										case 708:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 358;
											break;
										case 709:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 358;
											break;
										case 710:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 359;
											break;
										case 711:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 359;
											break;
										case 712:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 360;
											break;
										case 713:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 360;
											break;
										case 714:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 1080;
											break;
										case 715:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() - 1080;
											break;
										case 716:
											pEntity->GetEyeAngles()->y = pEntity->GetLowerBodyYaw() + 245;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
void CResolver::add_corrections() {
	if (m_pEngine->IsInGame() && m_pEngine->IsConnected()) {

		IClientEntity* m_local = game::localdata.localplayer();

		for (int i = 1; i < m_pGlobals->maxClients; i++) {

			auto m_entity = m_pEntityList->GetClientEntity(i);
			if (!m_entity || m_entity == m_local || m_entity->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer || !m_entity->IsAlive()) continue;

			CPlayer* m_player = plist.get_player(i);
			m_player->entity = m_entity;

			bool enable_resolver_y = resolverconfig.bAntiAimCorrection;

			int resolvermode_y = resolverconfig.bAntiAimCorrection ? 1 : 0;
			if (m_player->ForceYaw && m_player->ForceYaw_Yaw) {
				resolvermode_y = m_player->ForceYaw_Yaw;
				enable_resolver_y = true;
			}

			Vector* m_angles = m_entity->GetEyeAngles();
			Vector at_target_angle;

			game::math.calculate_angle(m_entity->GetOrigin(), m_local->GetOrigin(), at_target_angle);
			game::math.normalize_vector(at_target_angle);
			if (enable_resolver_y) {
				if (resolvermode_y == 1) {
					if (m_entity->GetVelocity().Length2D() > .1) {
						m_player->resolver_data.newer_stored_lby = m_entity->GetLowerBodyYaw();
					}

					float simtime_delta = m_entity->m_flSimulationTime() - m_player->resolver_data.time_at_update;

					if (m_entity->GetVelocity().Length2D() > 36) {
						m_player->resolver_data.stored_lby = m_entity->GetLowerBodyYaw();
						m_player->resolver_data.stored_lby_two = m_entity->GetLowerBodyYaw();
					}

					if (m_player->resolver_data.old_lowerbody_yaws != m_entity->GetLowerBodyYaw()) {
						m_player->resolver_data.old_yaw_deltas = m_entity->GetLowerBodyYaw() - m_player->resolver_data.old_lowerbody_yaws;
						m_player->resolver_data.old_lowerbody_yaws = m_entity->GetLowerBodyYaw();
						m_player->resolver_data.time_at_update = m_entity->m_flSimulationTime();

						if (m_entity->GetVelocity().Length2D() > 0.1f && (m_entity->GetFlags() & FL_ONGROUND)) {
							m_player->resolver_data.temp = m_entity->GetLowerBodyYaw();
							m_player->resolver_data.old_lowerbody_yaws = m_entity->GetLowerBodyYaw();
						}
						else {
							m_player->resolver_data.temp = m_player->resolver_data.old_lowerbody_yaws;
						}
					}
					else {
						m_player->resolver_data.temp = m_entity->GetLowerBodyYaw() - m_player->resolver_data.old_yaw_deltas;
					}

					float fixed_resolve = m_player->resolver_data.temp;

					if (m_entity->GetVelocity().Length2D() > 36) {
						m_angles->y = m_entity->GetLowerBodyYaw();
					}
					else if (simtime_delta > 1.525f) {
						if (simtime_delta > 1.525f && simtime_delta < 2.25f) {
							switch (m_player->resolver_data.shots % 2) {
							case 0:m_angles->y = m_player->resolver_data.stored_lby; break;
							case 1:m_angles->y = m_player->resolver_data.newer_stored_lby; break;
							}
						}
						else if (simtime_delta > 2.25f && simtime_delta < 2.95f) {
							m_angles->y = m_entity->GetLowerBodyYaw();
						}
						else if (simtime_delta > 2.95f) {
							switch (m_player->resolver_data.shots % 3) {
							case 0:m_angles->y = m_player->resolver_data.stored_lby; break;
							case 1:m_angles->y = m_entity->GetLowerBodyYaw(); break;
							case 2:m_angles->y = m_player->resolver_data.newer_stored_lby; break;
							}
						}
					}
					else if (simtime_delta <= 1.525f && simtime_delta > 1.125f) {
						m_angles->y = m_entity->GetLowerBodyYaw();
					}
					else {
						if (simtime_delta <= .57f) {
							m_player->resolver_data.resolved_yaw = m_entity->GetLowerBodyYaw();
						}
						else {
							if (simtime_delta <= .1) {
								m_angles->y = m_entity->GetLowerBodyYaw();
							}
							else if ((fabs(m_entity->GetLowerBodyYaw() - m_player->resolver_data.stored_lby)) >= 65) {
								if ((fabs(m_player->resolver_data.newer_stored_lby - m_player->resolver_data.stored_lby)) >= 55) {
									m_angles->y = m_player->resolver_data.stored_lby;
								}
								else {
									switch (m_player->resolver_data.shots % 2) {
									case 0: m_angles->y = m_player->resolver_data.newer_stored_lby; break;
									case 1: m_angles->y = m_entity->GetLowerBodyYaw() + 0.5; break;
									case 2: m_angles->y = m_entity->GetLowerBodyYaw() - 0.5; break;
									case 3: m_angles->y = m_entity->GetLowerBodyYaw() + 1; break;
									case 4: m_angles->y = m_entity->GetLowerBodyYaw() - 1; break;
									case 5: m_angles->y = m_entity->GetLowerBodyYaw() + 1.5; break;
									case 6: m_angles->y = m_entity->GetLowerBodyYaw() - 1.5; break;
									case 7: m_angles->y = m_entity->GetLowerBodyYaw() + 2; break;
									case 8: m_angles->y = m_entity->GetLowerBodyYaw() - 2; break;
									case 9: m_angles->y = m_entity->GetLowerBodyYaw() + 2.5; break;
									case 10: m_angles->y = m_entity->GetLowerBodyYaw() - 2.5; break;
									case 11: m_angles->y = m_entity->GetLowerBodyYaw() + 3; break;
									case 12: m_angles->y = m_entity->GetLowerBodyYaw() - 3; break;
									case 13: m_angles->y = m_entity->GetLowerBodyYaw() + 3.5; break;
									case 14: m_angles->y = m_entity->GetLowerBodyYaw() - 3.5; break;
									case 15: m_angles->y = m_entity->GetLowerBodyYaw() + 4; break;
									case 16: m_angles->y = m_entity->GetLowerBodyYaw() - 4; break;
									case 17: m_angles->y = m_entity->GetLowerBodyYaw() + 4.5; break;
									case 18: m_angles->y = m_entity->GetLowerBodyYaw() - 4.5; break;
									case 19: m_angles->y = m_entity->GetLowerBodyYaw() + 5; break;
									case 20: m_angles->y = m_entity->GetLowerBodyYaw() - 5; break;
									case 21: m_angles->y = m_entity->GetLowerBodyYaw() + 5.5; break;
									case 22: m_angles->y = m_entity->GetLowerBodyYaw() - 5.5; break;
									case 23: m_angles->y = m_entity->GetLowerBodyYaw() + 6; break;
									case 24: m_angles->y = m_entity->GetLowerBodyYaw() - 6; break;
									case 25: m_angles->y = m_entity->GetLowerBodyYaw() + 6.5; break;
									case 26: m_angles->y = m_entity->GetLowerBodyYaw() - 6.5; break;
									case 27: m_angles->y = m_entity->GetLowerBodyYaw() + 7; break;
									case 28: m_angles->y = m_entity->GetLowerBodyYaw() - 7; break;
									case 29: m_angles->y = m_entity->GetLowerBodyYaw() + 7.5; break;
									case 30: m_angles->y = m_entity->GetLowerBodyYaw() - 7.5; break;
									case 31: m_angles->y = m_entity->GetLowerBodyYaw() + 8; break;
									case 32: m_angles->y = m_entity->GetLowerBodyYaw() - 8; break;
									case 33: m_angles->y = m_entity->GetLowerBodyYaw() + 9.5; break;
									case 34: m_angles->y = m_entity->GetLowerBodyYaw() - 9.5; break;
									case 35: m_angles->y = m_entity->GetLowerBodyYaw() + 10; break;
									case 36: m_angles->y = m_entity->GetLowerBodyYaw() - 10; break;
									case 37: m_angles->y = m_entity->GetLowerBodyYaw() + 10.5; break;
									case 38: m_angles->y = m_entity->GetLowerBodyYaw() - 10.5; break;
									case 39: m_angles->y = m_entity->GetLowerBodyYaw() + 11; break;
									case 40: m_angles->y = m_entity->GetLowerBodyYaw() - 11; break;
									case 41: m_angles->y = m_entity->GetLowerBodyYaw() + 11.5; break;
									case 42: m_angles->y = m_entity->GetLowerBodyYaw() - 11.5; break;
									case 43: m_angles->y = m_entity->GetLowerBodyYaw() + 12; break;
									case 44: m_angles->y = m_entity->GetLowerBodyYaw() - 12; break;
									case 45: m_angles->y = m_entity->GetLowerBodyYaw() + 12.5; break;
									case 46: m_angles->y = m_entity->GetLowerBodyYaw() - 12.5; break;
									case 47: m_angles->y = m_entity->GetLowerBodyYaw() + 13; break;
									case 48: m_angles->y = m_entity->GetLowerBodyYaw() - 13; break;
									case 49: m_angles->y = m_entity->GetLowerBodyYaw() + 13.5; break;
									case 50: m_angles->y = m_entity->GetLowerBodyYaw() - 13.5; break;
									case 51: m_angles->y = m_entity->GetLowerBodyYaw() + 14; break;
									case 52: m_angles->y = m_entity->GetLowerBodyYaw() - 14; break;
									case 53: m_angles->y = m_entity->GetLowerBodyYaw() + 14.5; break;
									case 54: m_angles->y = m_entity->GetLowerBodyYaw() - 14.5; break;
									case 55: m_angles->y = m_entity->GetLowerBodyYaw() + 15; break;
									case 56: m_angles->y = m_entity->GetLowerBodyYaw() - 15; break;
									case 57: m_angles->y = m_entity->GetLowerBodyYaw() + 15.5; break;
									case 58: m_angles->y = m_entity->GetLowerBodyYaw() - 15.5; break;
									case 59: m_angles->y = m_entity->GetLowerBodyYaw() + 16; break;
									case 60: m_angles->y = m_entity->GetLowerBodyYaw() - 16; break;
									case 61: m_angles->y = m_entity->GetLowerBodyYaw() + 16.5; break;
									case 62: m_angles->y = m_entity->GetLowerBodyYaw() - 16.5; break;
									case 63: m_angles->y = m_entity->GetLowerBodyYaw() + 17; break;
									case 64: m_angles->y = m_entity->GetLowerBodyYaw() - 17; break;
									case 65: m_angles->y = m_entity->GetLowerBodyYaw() + 17.5; break;
									case 66: m_angles->y = m_entity->GetLowerBodyYaw() - 17.5; break;
									case 67: m_angles->y = m_entity->GetLowerBodyYaw() + 18; break;
									case 68: m_angles->y = m_entity->GetLowerBodyYaw() - 18; break;
									case 69: m_angles->y = m_entity->GetLowerBodyYaw() + 19.5; break;
									case 70: m_angles->y = m_entity->GetLowerBodyYaw() - 19.5; break;
									case 71: m_angles->y = m_entity->GetLowerBodyYaw() + 20; break;
									case 72: m_angles->y = m_entity->GetLowerBodyYaw() - 20; break;
									case 73: m_angles->y = m_entity->GetLowerBodyYaw() + 20.5; break;
									case 74: m_angles->y = m_entity->GetLowerBodyYaw() - 20.5; break;
									case 75: m_angles->y = m_entity->GetLowerBodyYaw() + 21; break;
									case 76: m_angles->y = m_entity->GetLowerBodyYaw() - 21; break;
									case 77: m_angles->y = m_entity->GetLowerBodyYaw() + 22.5; break;
									case 78: m_angles->y = m_entity->GetLowerBodyYaw() - 22.5; break;
									case 79: m_angles->y = m_entity->GetLowerBodyYaw() + 23; break;
									case 80: m_angles->y = m_entity->GetLowerBodyYaw() - 23; break;
									case 81: m_angles->y = m_entity->GetLowerBodyYaw() + 23.5; break;
									case 82: m_angles->y = m_entity->GetLowerBodyYaw() - 23.5; break;
									case 83: m_angles->y = m_entity->GetLowerBodyYaw() + 24; break;
									case 84: m_angles->y = m_entity->GetLowerBodyYaw() - 24; break;
									case 85: m_angles->y = m_entity->GetLowerBodyYaw() + 24.5; break;
									case 86: m_angles->y = m_entity->GetLowerBodyYaw() - 24.5; break;
									case 87: m_angles->y = m_entity->GetLowerBodyYaw() + 25; break;
									case 88: m_angles->y = m_entity->GetLowerBodyYaw() - 25; break;
									case 89: m_angles->y = m_entity->GetLowerBodyYaw() + 25.5; break;
									case 90: m_angles->y = m_entity->GetLowerBodyYaw() - 25.5; break;
									case 91: m_angles->y = m_entity->GetLowerBodyYaw() + 26; break;
									case 92: m_angles->y = m_entity->GetLowerBodyYaw() - 26; break;
									case 93: m_angles->y = m_entity->GetLowerBodyYaw() + 26.5; break;
									case 94: m_angles->y = m_entity->GetLowerBodyYaw() - 26.5; break;
									case 95: m_angles->y = m_entity->GetLowerBodyYaw() + 27; break;
									case 96: m_angles->y = m_entity->GetLowerBodyYaw() - 27; break;
									case 97: m_angles->y = m_entity->GetLowerBodyYaw() + 27.5; break;
									case 98: m_angles->y = m_entity->GetLowerBodyYaw() - 27.5; break;
									case 99: m_angles->y = m_entity->GetLowerBodyYaw() + 28; break;
									case 100: m_angles->y = m_entity->GetLowerBodyYaw() - 28; break;
									case 101: m_angles->y = m_entity->GetLowerBodyYaw() + 28.5; break;
									case 102: m_angles->y = m_entity->GetLowerBodyYaw() - 28.5; break;
									case 103: m_angles->y = m_entity->GetLowerBodyYaw() + 29; break;
									case 104: m_angles->y = m_entity->GetLowerBodyYaw() - 29; break;
									case 105: m_angles->y = m_entity->GetLowerBodyYaw() + 29.5; break;
									case 106: m_angles->y = m_entity->GetLowerBodyYaw() - 29.5; break;
									case 107: m_angles->y = m_entity->GetLowerBodyYaw() + 30; break;
									case 108: m_angles->y = m_entity->GetLowerBodyYaw() - 30; break;
									case 109: m_angles->y = m_entity->GetLowerBodyYaw() + 30.5; break;
									case 110: m_angles->y = m_entity->GetLowerBodyYaw() - 30.5; break;
									case 111: m_angles->y = m_entity->GetLowerBodyYaw() + 31; break;
									case 112: m_angles->y = m_entity->GetLowerBodyYaw() - 31; break;
									case 113: m_angles->y = m_entity->GetLowerBodyYaw() + 31.5; break;
									case 114: m_angles->y = m_entity->GetLowerBodyYaw() - 31.5; break;
									case 115: m_angles->y = m_entity->GetLowerBodyYaw() + 32; break;
									case 116: m_angles->y = m_entity->GetLowerBodyYaw() - 32; break;
									case 117: m_angles->y = m_entity->GetLowerBodyYaw() + 32.5; break;
									case 118: m_angles->y = m_entity->GetLowerBodyYaw() - 32.5; break;
									case 119: m_angles->y = m_entity->GetLowerBodyYaw() + 33; break;
									case 120: m_angles->y = m_entity->GetLowerBodyYaw() - 33; break;
									case 121: m_angles->y = m_entity->GetLowerBodyYaw() + 33.5; break;
									case 122: m_angles->y = m_entity->GetLowerBodyYaw() - 33.5; break;
									case 123: m_angles->y = m_entity->GetLowerBodyYaw() + 34; break;
									case 124: m_angles->y = m_entity->GetLowerBodyYaw() - 34; break;
									case 125: m_angles->y = m_entity->GetLowerBodyYaw() + 34.5; break;
									case 126: m_angles->y = m_entity->GetLowerBodyYaw() - 34.5; break;
									case 127: m_angles->y = m_entity->GetLowerBodyYaw() + 35; break;
									case 128: m_angles->y = m_entity->GetLowerBodyYaw() - 35; break;
									case 129: m_angles->y = m_entity->GetLowerBodyYaw() + 35.5; break;
									case 130: m_angles->y = m_entity->GetLowerBodyYaw() - 35.5; break;
									case 131: m_angles->y = m_entity->GetLowerBodyYaw() + 36; break;
									case 132: m_angles->y = m_entity->GetLowerBodyYaw() - 36; break;
									case 133: m_angles->y = m_entity->GetLowerBodyYaw() + 36.5; break;
									case 134: m_angles->y = m_entity->GetLowerBodyYaw() - 36.5; break;
									case 135: m_angles->y = m_entity->GetLowerBodyYaw() + 37; break;
									case 136: m_angles->y = m_entity->GetLowerBodyYaw() - 37; break;
									case 137: m_angles->y = m_entity->GetLowerBodyYaw() + 37.5; break;
									case 138: m_angles->y = m_entity->GetLowerBodyYaw() - 37.5; break;
									case 139: m_angles->y = m_entity->GetLowerBodyYaw() + 38; break;
									case 140: m_angles->y = m_entity->GetLowerBodyYaw() - 38; break;
									case 141: m_angles->y = m_entity->GetLowerBodyYaw() + 39.5; break;
									case 142: m_angles->y = m_entity->GetLowerBodyYaw() - 39.5; break;
									case 143: m_angles->y = m_entity->GetLowerBodyYaw() + 40; break;
									case 144: m_angles->y = m_entity->GetLowerBodyYaw() - 40; break;
									case 145: m_angles->y = m_entity->GetLowerBodyYaw() + 40.5; break;
									case 146: m_angles->y = m_entity->GetLowerBodyYaw() - 40.5; break;
									case 147: m_angles->y = m_entity->GetLowerBodyYaw() + 41; break;
									case 148: m_angles->y = m_entity->GetLowerBodyYaw() - 41; break;
									case 149: m_angles->y = m_entity->GetLowerBodyYaw() + 41.5; break;
									case 150: m_angles->y = m_entity->GetLowerBodyYaw() - 41.5; break;
									case 151: m_angles->y = m_entity->GetLowerBodyYaw() + 42; break;
									case 152: m_angles->y = m_entity->GetLowerBodyYaw() - 42; break;
									case 153: m_angles->y = m_entity->GetLowerBodyYaw() + 42.5; break;
									case 154: m_angles->y = m_entity->GetLowerBodyYaw() - 42.5; break;
									case 155: m_angles->y = m_entity->GetLowerBodyYaw() + 43; break;
									case 156: m_angles->y = m_entity->GetLowerBodyYaw() - 43; break;
									case 157: m_angles->y = m_entity->GetLowerBodyYaw() + 43.5; break;
									case 158: m_angles->y = m_entity->GetLowerBodyYaw() - 43.5; break;
									case 159: m_angles->y = m_entity->GetLowerBodyYaw() + 44; break;
									case 160: m_angles->y = m_entity->GetLowerBodyYaw() - 44; break;
									case 161: m_angles->y = m_entity->GetLowerBodyYaw() + 44.5; break;
									case 162: m_angles->y = m_entity->GetLowerBodyYaw() - 44.5; break;
									case 163: m_angles->y = m_entity->GetLowerBodyYaw() + 45; break;
									case 164: m_angles->y = m_entity->GetLowerBodyYaw() + 45.5; break;
									case 165: m_angles->y = m_entity->GetLowerBodyYaw() - 45.5; break;
									case 166: m_angles->y = m_entity->GetLowerBodyYaw() + 46; break;
									case 167: m_angles->y = m_entity->GetLowerBodyYaw() - 46; break;
									case 168: m_angles->y = m_entity->GetLowerBodyYaw() + 46.5; break;
									case 169: m_angles->y = m_entity->GetLowerBodyYaw() - 46.5; break;
									case 170: m_angles->y = m_entity->GetLowerBodyYaw() + 47; break;
									case 171: m_angles->y = m_entity->GetLowerBodyYaw() - 47; break;
									case 172: m_angles->y = m_entity->GetLowerBodyYaw() + 47.5; break;
									case 173: m_angles->y = m_entity->GetLowerBodyYaw() - 47.5; break;
									case 174: m_angles->y = m_entity->GetLowerBodyYaw() + 48; break;
									case 175: m_angles->y = m_entity->GetLowerBodyYaw() - 48; break;
									case 176: m_angles->y = m_entity->GetLowerBodyYaw() + 48.5; break;
									case 177: m_angles->y = m_entity->GetLowerBodyYaw() - 48.5; break;
									case 178: m_angles->y = m_entity->GetLowerBodyYaw() + 49; break;
									case 179: m_angles->y = m_entity->GetLowerBodyYaw() - 49; break;
									case 180: m_angles->y = m_entity->GetLowerBodyYaw() + 49.5; break;
									case 181: m_angles->y = m_entity->GetLowerBodyYaw() - 49.5; break;
									case 182: m_angles->y = m_entity->GetLowerBodyYaw() + 50; break;
									case 183: m_angles->y = m_entity->GetLowerBodyYaw() - 50; break;
									case 184: m_angles->y = m_entity->GetLowerBodyYaw() + 50.5; break;
									case 185: m_angles->y = m_entity->GetLowerBodyYaw() - 50.5; break;
									case 186: m_angles->y = m_entity->GetLowerBodyYaw() + 51; break;
									case 187: m_angles->y = m_entity->GetLowerBodyYaw() - 51; break;
									case 188: m_angles->y = m_entity->GetLowerBodyYaw() + 51.5; break;
									case 189: m_angles->y = m_entity->GetLowerBodyYaw() - 51.5; break;
									case 190: m_angles->y = m_entity->GetLowerBodyYaw() + 52; break;
									case 191: m_angles->y = m_entity->GetLowerBodyYaw() - 52; break;
									case 192: m_angles->y = m_entity->GetLowerBodyYaw() + 52.5; break;
									case 193: m_angles->y = m_entity->GetLowerBodyYaw() - 52.5; break;
									case 194: m_angles->y = m_entity->GetLowerBodyYaw() + 53; break;
									case 195: m_angles->y = m_entity->GetLowerBodyYaw() - 53; break;
									case 196: m_angles->y = m_entity->GetLowerBodyYaw() + 53.5; break;
									case 197: m_angles->y = m_entity->GetLowerBodyYaw() - 53.5; break;
									case 198: m_angles->y = m_entity->GetLowerBodyYaw() + 54; break;
									case 199: m_angles->y = m_entity->GetLowerBodyYaw() - 54; break;
									case 200: m_angles->y = m_entity->GetLowerBodyYaw() + 54.5; break;
									case 201: m_angles->y = m_entity->GetLowerBodyYaw() - 54.5; break;
									case 202: m_angles->y = m_entity->GetLowerBodyYaw() + 55; break;
									case 203: m_angles->y = m_entity->GetLowerBodyYaw() - 55; break;
									case 204: m_angles->y = m_entity->GetLowerBodyYaw() + 55.5; break;
									case 205: m_angles->y = m_entity->GetLowerBodyYaw() - 55.5; break;
									case 206: m_angles->y = m_entity->GetLowerBodyYaw() + 56; break;
									case 207: m_angles->y = m_entity->GetLowerBodyYaw() - 56; break;
									case 208: m_angles->y = m_entity->GetLowerBodyYaw() + 56.5; break;
									case 209: m_angles->y = m_entity->GetLowerBodyYaw() - 56.5; break;
									case 210: m_angles->y = m_entity->GetLowerBodyYaw() + 57; break;
									case 211: m_angles->y = m_entity->GetLowerBodyYaw() - 57; break;
									case 212: m_angles->y = m_entity->GetLowerBodyYaw() + 57.5; break;
									case 213: m_angles->y = m_entity->GetLowerBodyYaw() - 57.5; break;
									case 214: m_angles->y = m_entity->GetLowerBodyYaw() + 58; break;
									case 215: m_angles->y = m_entity->GetLowerBodyYaw() - 58; break;
									case 216: m_angles->y = m_entity->GetLowerBodyYaw() + 58.5; break;
									case 217: m_angles->y = m_entity->GetLowerBodyYaw() - 58.5; break;
									case 218: m_angles->y = m_entity->GetLowerBodyYaw() + 58; break;
									case 219: m_angles->y = m_entity->GetLowerBodyYaw() - 58; break;
									case 220: m_angles->y = m_entity->GetLowerBodyYaw() + 58.5; break;
									case 221: m_angles->y = m_entity->GetLowerBodyYaw() - 58.5; break;
									case 222: m_angles->y = m_entity->GetLowerBodyYaw() + 59; break;
									case 223: m_angles->y = m_entity->GetLowerBodyYaw() - 59; break;
									case 224: m_angles->y = m_entity->GetLowerBodyYaw() + 59.5; break;
									case 225: m_angles->y = m_entity->GetLowerBodyYaw() - 59.5; break;
									case 226: m_angles->y = m_entity->GetLowerBodyYaw() + 60; break;
									case 227: m_angles->y = m_entity->GetLowerBodyYaw() - 60; break;
									case 228: m_angles->y = m_entity->GetLowerBodyYaw() + 60.5; break;
									case 229: m_angles->y = m_entity->GetLowerBodyYaw() - 60.5; break;
									case 230: m_angles->y = m_entity->GetLowerBodyYaw() + 61; break;
									case 231: m_angles->y = m_entity->GetLowerBodyYaw() - 61; break;
									case 232: m_angles->y = m_entity->GetLowerBodyYaw() + 62.5; break;
									case 233: m_angles->y = m_entity->GetLowerBodyYaw() - 62.5; break;
									case 234: m_angles->y = m_entity->GetLowerBodyYaw() + 63; break;
									case 235: m_angles->y = m_entity->GetLowerBodyYaw() - 63; break;
									case 236: m_angles->y = m_entity->GetLowerBodyYaw() + 63.5; break;
									case 237: m_angles->y = m_entity->GetLowerBodyYaw() - 63.5; break;
									case 238: m_angles->y = m_entity->GetLowerBodyYaw() + 64; break;
									case 239: m_angles->y = m_entity->GetLowerBodyYaw() - 64; break;
									case 240: m_angles->y = m_entity->GetLowerBodyYaw() + 64.5; break;
									case 241: m_angles->y = m_entity->GetLowerBodyYaw() - 64.5; break;
									case 242: m_angles->y = m_entity->GetLowerBodyYaw() + 65; break;
									case 243: m_angles->y = m_entity->GetLowerBodyYaw() - 65; break;
									case 244: m_angles->y = m_entity->GetLowerBodyYaw() + 65.5; break;
									case 245: m_angles->y = m_entity->GetLowerBodyYaw() - 65.5; break;
									case 246: m_angles->y = m_entity->GetLowerBodyYaw() + 66; break;
									case 247: m_angles->y = m_entity->GetLowerBodyYaw() - 66; break;
									case 248: m_angles->y = m_entity->GetLowerBodyYaw() + 66.5; break;
									case 249: m_angles->y = m_entity->GetLowerBodyYaw() - 66.5; break;
									case 250: m_angles->y = m_entity->GetLowerBodyYaw() + 67; break;
									case 251: m_angles->y = m_entity->GetLowerBodyYaw() - 67; break;
									case 252: m_angles->y = m_entity->GetLowerBodyYaw() + 67.5; break;
									case 253: m_angles->y = m_entity->GetLowerBodyYaw() - 67.5; break;
									case 254: m_angles->y = m_entity->GetLowerBodyYaw() + 68; break;
									case 255: m_angles->y = m_entity->GetLowerBodyYaw() - 68; break;
									case 256: m_angles->y = m_entity->GetLowerBodyYaw() + 68.5; break;
									case 257: m_angles->y = m_entity->GetLowerBodyYaw() - 68.5; break;
									case 258: m_angles->y = m_entity->GetLowerBodyYaw() + 69; break;
									case 259: m_angles->y = m_entity->GetLowerBodyYaw() - 69; break;
									case 260: m_angles->y = m_entity->GetLowerBodyYaw() + 69.5; break;
									case 261: m_angles->y = m_entity->GetLowerBodyYaw() - 69.5; break;
									case 262: m_angles->y = m_entity->GetLowerBodyYaw() + 70; break;
									case 263: m_angles->y = m_entity->GetLowerBodyYaw() - 70; break;
									case 264: m_angles->y = m_entity->GetLowerBodyYaw() + 70.5; break;
									case 265: m_angles->y = m_entity->GetLowerBodyYaw() - 70.5; break;
									case 266: m_angles->y = m_entity->GetLowerBodyYaw() + 71; break;
									case 267: m_angles->y = m_entity->GetLowerBodyYaw() - 71; break;
									case 268: m_angles->y = m_entity->GetLowerBodyYaw() + 71.5; break;
									case 269: m_angles->y = m_entity->GetLowerBodyYaw() - 71.5; break;
									case 270: m_angles->y = m_entity->GetLowerBodyYaw() + 72; break;
									case 271: m_angles->y = m_entity->GetLowerBodyYaw() - 72; break;
									case 272: m_angles->y = m_entity->GetLowerBodyYaw() + 72.5; break;
									case 273: m_angles->y = m_entity->GetLowerBodyYaw() - 72.5; break;
									case 274: m_angles->y = m_entity->GetLowerBodyYaw() + 73; break;
									case 275: m_angles->y = m_entity->GetLowerBodyYaw() - 73; break;
									case 276: m_angles->y = m_entity->GetLowerBodyYaw() + 73.5; break;
									case 277: m_angles->y = m_entity->GetLowerBodyYaw() - 73.5; break;
									case 278: m_angles->y = m_entity->GetLowerBodyYaw() + 74; break;
									case 279: m_angles->y = m_entity->GetLowerBodyYaw() - 74; break;
									case 280: m_angles->y = m_entity->GetLowerBodyYaw() + 74.5; break;
									case 281: m_angles->y = m_entity->GetLowerBodyYaw() - 74.5; break;
									case 282: m_angles->y = m_entity->GetLowerBodyYaw() + 75; break;
									case 283: m_angles->y = m_entity->GetLowerBodyYaw() - 75; break;
									case 284: m_angles->y = m_entity->GetLowerBodyYaw() + 75.5; break;
									case 285: m_angles->y = m_entity->GetLowerBodyYaw() - 75.5; break;
									case 286: m_angles->y = m_entity->GetLowerBodyYaw() + 76; break;
									case 287: m_angles->y = m_entity->GetLowerBodyYaw() - 76; break;
									case 288: m_angles->y = m_entity->GetLowerBodyYaw() + 76.5; break;
									case 289: m_angles->y = m_entity->GetLowerBodyYaw() - 76.5; break;
									case 290: m_angles->y = m_entity->GetLowerBodyYaw() + 77; break;
									case 291: m_angles->y = m_entity->GetLowerBodyYaw() - 77; break;
									case 292: m_angles->y = m_entity->GetLowerBodyYaw() + 77.5; break;
									case 293: m_angles->y = m_entity->GetLowerBodyYaw() - 77.5; break;
									case 294: m_angles->y = m_entity->GetLowerBodyYaw() + 78; break;
									case 295: m_angles->y = m_entity->GetLowerBodyYaw() - 78; break;
									case 296: m_angles->y = m_entity->GetLowerBodyYaw() + 78.5; break;
									case 297: m_angles->y = m_entity->GetLowerBodyYaw() - 78.5; break;
									case 298: m_angles->y = m_entity->GetLowerBodyYaw() + 79; break;
									case 299: m_angles->y = m_entity->GetLowerBodyYaw() - 79; break;
									case 300: m_angles->y = m_entity->GetLowerBodyYaw() + 79.5; break;
									case 301: m_angles->y = m_entity->GetLowerBodyYaw() - 79.5; break;
									case 302: m_angles->y = m_entity->GetLowerBodyYaw() + 80; break;
									case 303: m_angles->y = m_entity->GetLowerBodyYaw() - 80; break;
									case 304: m_angles->y = m_entity->GetLowerBodyYaw() + 80.5; break;
									case 305: m_angles->y = m_entity->GetLowerBodyYaw() - 80.5; break;
									case 306: m_angles->y = m_entity->GetLowerBodyYaw() + 81; break;
									case 307: m_angles->y = m_entity->GetLowerBodyYaw() - 81; break;
									case 308: m_angles->y = m_entity->GetLowerBodyYaw() + 81.5; break;
									case 309: m_angles->y = m_entity->GetLowerBodyYaw() - 81.5; break;
									case 310: m_angles->y = m_entity->GetLowerBodyYaw() + 82; break;
									case 311: m_angles->y = m_entity->GetLowerBodyYaw() - 82; break;
									case 312: m_angles->y = m_entity->GetLowerBodyYaw() + 82.5; break;
									case 313: m_angles->y = m_entity->GetLowerBodyYaw() - 82.5; break;
									case 314: m_angles->y = m_entity->GetLowerBodyYaw() + 83; break;
									case 315: m_angles->y = m_entity->GetLowerBodyYaw() - 83; break;
									case 316: m_angles->y = m_entity->GetLowerBodyYaw() + 83.5; break;
									case 317: m_angles->y = m_entity->GetLowerBodyYaw() - 83.5; break;
									case 318: m_angles->y = m_entity->GetLowerBodyYaw() + 84; break;
									case 319: m_angles->y = m_entity->GetLowerBodyYaw() - 84; break;
									case 320: m_angles->y = m_entity->GetLowerBodyYaw() + 84.5; break;
									case 321: m_angles->y = m_entity->GetLowerBodyYaw() - 84.5; break;
									case 322: m_angles->y = m_entity->GetLowerBodyYaw() + 85; break;
									case 323: m_angles->y = m_entity->GetLowerBodyYaw() - 85; break;
									case 324: m_angles->y = m_entity->GetLowerBodyYaw() + 85.5; break;
									case 325: m_angles->y = m_entity->GetLowerBodyYaw() - 85.5; break;
									case 326: m_angles->y = m_entity->GetLowerBodyYaw() + 86; break;
									case 327: m_angles->y = m_entity->GetLowerBodyYaw() - 86; break;
									case 328: m_angles->y = m_entity->GetLowerBodyYaw() + 86.5; break;
									case 329: m_angles->y = m_entity->GetLowerBodyYaw() - 86.5; break;
									case 330: m_angles->y = m_entity->GetLowerBodyYaw() + 87; break;
									case 331: m_angles->y = m_entity->GetLowerBodyYaw() - 87; break;
									case 332: m_angles->y = m_entity->GetLowerBodyYaw() + 87.5; break;
									case 333: m_angles->y = m_entity->GetLowerBodyYaw() - 87.5; break;
									case 334: m_angles->y = m_entity->GetLowerBodyYaw() + 88; break;
									case 335: m_angles->y = m_entity->GetLowerBodyYaw() - 88; break;
									case 336: m_angles->y = m_entity->GetLowerBodyYaw() + 88.5; break;
									case 337: m_angles->y = m_entity->GetLowerBodyYaw() - 88.5; break;
									case 338: m_angles->y = m_entity->GetLowerBodyYaw() + 89; break;
									case 339: m_angles->y = m_entity->GetLowerBodyYaw() - 89; break;
									case 340: m_angles->y = m_entity->GetLowerBodyYaw() + 89.5; break;
									case 341: m_angles->y = m_entity->GetLowerBodyYaw() - 89.5; break;
									case 342: m_angles->y = m_entity->GetLowerBodyYaw() + 90; break;
									case 343: m_angles->y = m_entity->GetLowerBodyYaw() - 90; break;
									case 344: m_angles->y = m_entity->GetLowerBodyYaw() + 90.5; break;
									case 345: m_angles->y = m_entity->GetLowerBodyYaw() - 90.5; break;
									case 346: m_angles->y = m_entity->GetLowerBodyYaw() + 91; break;
									case 347: m_angles->y = m_entity->GetLowerBodyYaw() - 91; break;
									case 348: m_angles->y = m_entity->GetLowerBodyYaw() + 91.5; break;
									case 349: m_angles->y = m_entity->GetLowerBodyYaw() - 91.5; break;
									case 350: m_angles->y = m_entity->GetLowerBodyYaw() + 92; break;
									case 351: m_angles->y = m_entity->GetLowerBodyYaw() - 92; break;
									case 352: m_angles->y = m_entity->GetLowerBodyYaw() + 92.5; break;
									case 353: m_angles->y = m_entity->GetLowerBodyYaw() - 92.5; break;
									case 354: m_angles->y = m_entity->GetLowerBodyYaw() + 93; break;
									case 355: m_angles->y = m_entity->GetLowerBodyYaw() - 93; break;
									case 356: m_angles->y = m_entity->GetLowerBodyYaw() + 93.5; break;
									case 357: m_angles->y = m_entity->GetLowerBodyYaw() - 93.5; break;
									case 358: m_angles->y = m_entity->GetLowerBodyYaw() + 94; break;
									case 359: m_angles->y = m_entity->GetLowerBodyYaw() - 94; break;
									case 360: m_angles->y = m_entity->GetLowerBodyYaw() + 94.5; break;
									case 361: m_angles->y = m_entity->GetLowerBodyYaw() - 94.5; break;
									case 362: m_angles->y = m_entity->GetLowerBodyYaw() + 95; break;
									case 363: m_angles->y = m_entity->GetLowerBodyYaw() - 95; break;
									case 364: m_angles->y = m_entity->GetLowerBodyYaw() + 95.5; break;
									case 365: m_angles->y = m_entity->GetLowerBodyYaw() - 95.5; break;
									case 366: m_angles->y = m_entity->GetLowerBodyYaw() + 96; break;
									case 368: m_angles->y = m_entity->GetLowerBodyYaw() - 96; break;
									case 369: m_angles->y = m_entity->GetLowerBodyYaw() + 96.5; break;
									case 370: m_angles->y = m_entity->GetLowerBodyYaw() - 96.5; break;
									case 371: m_angles->y = m_entity->GetLowerBodyYaw() + 97; break;
									case 372: m_angles->y = m_entity->GetLowerBodyYaw() - 97; break;
									case 373: m_angles->y = m_entity->GetLowerBodyYaw() + 97.5; break;
									case 374: m_angles->y = m_entity->GetLowerBodyYaw() - 97.5; break;
									case 375: m_angles->y = m_entity->GetLowerBodyYaw() + 98; break;
									case 376: m_angles->y = m_entity->GetLowerBodyYaw() - 98; break;
									case 377: m_angles->y = m_entity->GetLowerBodyYaw() + 98.5; break;
									case 378: m_angles->y = m_entity->GetLowerBodyYaw() - 98.5; break;
									case 379: m_angles->y = m_entity->GetLowerBodyYaw() + 99; break;
									case 380: m_angles->y = m_entity->GetLowerBodyYaw() - 99; break;
									case 381: m_angles->y = m_entity->GetLowerBodyYaw() + 99.5; break;
									case 382: m_angles->y = m_entity->GetLowerBodyYaw() - 99.5; break;
									case 383: m_angles->y = m_entity->GetLowerBodyYaw() + 100; break;
									case 384: m_angles->y = m_entity->GetLowerBodyYaw() - 100; break;
									case 385: m_angles->y = m_entity->GetLowerBodyYaw() + 100.5; break;
									case 386: m_angles->y = m_entity->GetLowerBodyYaw() - 100.5; break;
									case 387: m_angles->y = m_entity->GetLowerBodyYaw() + 101; break;
									case 388: m_angles->y = m_entity->GetLowerBodyYaw() - 101; break;
									case 389: m_angles->y = m_entity->GetLowerBodyYaw() + 101.5; break;
									case 390: m_angles->y = m_entity->GetLowerBodyYaw() - 101.5; break;
									case 391: m_angles->y = m_entity->GetLowerBodyYaw() + 102; break;
									case 392: m_angles->y = m_entity->GetLowerBodyYaw() - 102; break;
									case 393: m_angles->y = m_entity->GetLowerBodyYaw() + 102.5; break;
									case 394: m_angles->y = m_entity->GetLowerBodyYaw() - 102.5; break;
									case 395: m_angles->y = m_entity->GetLowerBodyYaw() + 103; break;
									case 396: m_angles->y = m_entity->GetLowerBodyYaw() - 103; break;
									case 397: m_angles->y = m_entity->GetLowerBodyYaw() + 103.5; break;
									case 398: m_angles->y = m_entity->GetLowerBodyYaw() - 103.5; break;
									case 399: m_angles->y = m_entity->GetLowerBodyYaw() + 104; break;
									case 400: m_angles->y = m_entity->GetLowerBodyYaw() - 104; break;
									case 401: m_angles->y = m_entity->GetLowerBodyYaw() + 104.5; break;
									case 402: m_angles->y = m_entity->GetLowerBodyYaw() - 104.5; break;
									case 403: m_angles->y = m_entity->GetLowerBodyYaw() + 104; break;
									case 404: m_angles->y = m_entity->GetLowerBodyYaw() - 104; break;
									case 405: m_angles->y = m_entity->GetLowerBodyYaw() + 104.5; break;
									case 406: m_angles->y = m_entity->GetLowerBodyYaw() - 104.5; break;
									case 407: m_angles->y = m_entity->GetLowerBodyYaw() + 105; break;
									case 408: m_angles->y = m_entity->GetLowerBodyYaw() - 105; break;
									case 409: m_angles->y = m_entity->GetLowerBodyYaw() + 105.5; break;
									case 410: m_angles->y = m_entity->GetLowerBodyYaw() - 105.5; break;
									case 411: m_angles->y = m_entity->GetLowerBodyYaw() + 106; break;
									case 412: m_angles->y = m_entity->GetLowerBodyYaw() - 106; break;
									case 413: m_angles->y = m_entity->GetLowerBodyYaw() + 106.5; break;
									case 414: m_angles->y = m_entity->GetLowerBodyYaw() - 106.5; break;
									case 415: m_angles->y = m_entity->GetLowerBodyYaw() + 107; break;
									case 416: m_angles->y = m_entity->GetLowerBodyYaw() - 107; break;
									case 417: m_angles->y = m_entity->GetLowerBodyYaw() + 107.5; break;
									case 418: m_angles->y = m_entity->GetLowerBodyYaw() - 107.5; break;
									case 419: m_angles->y = m_entity->GetLowerBodyYaw() + 108; break;
									case 420: m_angles->y = m_entity->GetLowerBodyYaw() - 108; break;
									case 421: m_angles->y = m_entity->GetLowerBodyYaw() + 108.5; break;
									case 422: m_angles->y = m_entity->GetLowerBodyYaw() - 108.5; break;
									case 423: m_angles->y = m_entity->GetLowerBodyYaw() + 109; break;
									case 424: m_angles->y = m_entity->GetLowerBodyYaw() - 109; break;
									case 425: m_angles->y = m_entity->GetLowerBodyYaw() + 109.5; break;
									case 426: m_angles->y = m_entity->GetLowerBodyYaw() - 109.5; break;
									case 427: m_angles->y = m_entity->GetLowerBodyYaw() + 110; break;
									case 428: m_angles->y = m_entity->GetLowerBodyYaw() - 110; break;
									case 429: m_angles->y = m_entity->GetLowerBodyYaw() + 110.5; break;
									case 430: m_angles->y = m_entity->GetLowerBodyYaw() - 110.5; break;
									case 431: m_angles->y = m_entity->GetLowerBodyYaw() + 111; break;
									case 432: m_angles->y = m_entity->GetLowerBodyYaw() - 111; break;
									case 433: m_angles->y = m_entity->GetLowerBodyYaw() + 111.5; break;
									case 434: m_angles->y = m_entity->GetLowerBodyYaw() - 111.5; break;
									case 435: m_angles->y = m_entity->GetLowerBodyYaw() + 112; break;
									case 436: m_angles->y = m_entity->GetLowerBodyYaw() - 112; break;
									case 437: m_angles->y = m_entity->GetLowerBodyYaw() + 112.5; break;
									case 438: m_angles->y = m_entity->GetLowerBodyYaw() - 112.5; break;
									case 439: m_angles->y = m_entity->GetLowerBodyYaw() + 113; break;
									case 440: m_angles->y = m_entity->GetLowerBodyYaw() - 113; break;
									case 441: m_angles->y = m_entity->GetLowerBodyYaw() + 113.5; break;
									case 442: m_angles->y = m_entity->GetLowerBodyYaw() - 113.5; break;
									case 443: m_angles->y = m_entity->GetLowerBodyYaw() + 114; break;
									case 444: m_angles->y = m_entity->GetLowerBodyYaw() - 114; break;
									case 445: m_angles->y = m_entity->GetLowerBodyYaw() + 114.5; break;
									case 446: m_angles->y = m_entity->GetLowerBodyYaw() - 114.5; break;
									case 447: m_angles->y = m_entity->GetLowerBodyYaw() + 115; break;
									case 448: m_angles->y = m_entity->GetLowerBodyYaw() - 115; break;
									case 449: m_angles->y = m_entity->GetLowerBodyYaw() + 115.5; break;
									case 450: m_angles->y = m_entity->GetLowerBodyYaw() - 115.5; break;
									case 451: m_angles->y = m_entity->GetLowerBodyYaw() + 116; break;
									case 452: m_angles->y = m_entity->GetLowerBodyYaw() - 116; break;
									case 453: m_angles->y = m_entity->GetLowerBodyYaw() + 116.5; break;
									case 454: m_angles->y = m_entity->GetLowerBodyYaw() - 116.5; break;
									case 455: m_angles->y = m_entity->GetLowerBodyYaw() + 117; break;
									case 456: m_angles->y = m_entity->GetLowerBodyYaw() - 117; break;
									case 457: m_angles->y = m_entity->GetLowerBodyYaw() + 117.5; break;
									case 458: m_angles->y = m_entity->GetLowerBodyYaw() - 117.5; break;
									case 459: m_angles->y = m_entity->GetLowerBodyYaw() + 118; break;
									case 460: m_angles->y = m_entity->GetLowerBodyYaw() - 118; break;
									case 461: m_angles->y = m_entity->GetLowerBodyYaw() + 118.5; break;
									case 462: m_angles->y = m_entity->GetLowerBodyYaw() - 118.5; break;
									case 463: m_angles->y = m_entity->GetLowerBodyYaw() + 119; break;
									case 464: m_angles->y = m_entity->GetLowerBodyYaw() - 119; break;
									case 465: m_angles->y = m_entity->GetLowerBodyYaw() + 119.5; break;
									case 466: m_angles->y = m_entity->GetLowerBodyYaw() - 119.5; break;
									case 467: m_angles->y = m_entity->GetLowerBodyYaw() + 120; break;
									case 468: m_angles->y = m_entity->GetLowerBodyYaw() - 120; break;
									case 469: m_angles->y = m_entity->GetLowerBodyYaw() + 120.5; break;
									case 470: m_angles->y = m_entity->GetLowerBodyYaw() - 120.5; break;
									case 471: m_angles->y = m_entity->GetLowerBodyYaw() + 121; break;
									case 472: m_angles->y = m_entity->GetLowerBodyYaw() - 121; break;
									case 473: m_angles->y = m_entity->GetLowerBodyYaw() + 121.5; break;
									case 474: m_angles->y = m_entity->GetLowerBodyYaw() - 121.5; break;
									case 475: m_angles->y = m_entity->GetLowerBodyYaw() + 122; break;
									case 476: m_angles->y = m_entity->GetLowerBodyYaw() - 122; break;
									case 477: m_angles->y = m_entity->GetLowerBodyYaw() + 122.5; break;
									case 478: m_angles->y = m_entity->GetLowerBodyYaw() - 122.5; break;
									case 479: m_angles->y = m_entity->GetLowerBodyYaw() + 123; break;
									case 480: m_angles->y = m_entity->GetLowerBodyYaw() - 123; break;
									case 481: m_angles->y = m_entity->GetLowerBodyYaw() + 123.5; break;
									case 482: m_angles->y = m_entity->GetLowerBodyYaw() - 123.5; break;
									case 483: m_angles->y = m_entity->GetLowerBodyYaw() + 124; break;
									case 484: m_angles->y = m_entity->GetLowerBodyYaw() - 124; break;
									case 485: m_angles->y = m_entity->GetLowerBodyYaw() + 124.5; break;
									case 486: m_angles->y = m_entity->GetLowerBodyYaw() - 124.5; break;
									case 487: m_angles->y = m_entity->GetLowerBodyYaw() + 125; break;
									case 488: m_angles->y = m_entity->GetLowerBodyYaw() - 125; break;
									case 489: m_angles->y = m_entity->GetLowerBodyYaw() + 125.5; break;
									case 490: m_angles->y = m_entity->GetLowerBodyYaw() - 125.5; break;
									case 491: m_angles->y = m_entity->GetLowerBodyYaw() + 126; break;
									case 492: m_angles->y = m_entity->GetLowerBodyYaw() - 126; break;
									case 493: m_angles->y = m_entity->GetLowerBodyYaw() + 126.5; break;
									case 494: m_angles->y = m_entity->GetLowerBodyYaw() - 126.5; break;
									case 495: m_angles->y = m_entity->GetLowerBodyYaw() + 127; break;
									case 496: m_angles->y = m_entity->GetLowerBodyYaw() - 127; break;
									case 497: m_angles->y = m_entity->GetLowerBodyYaw() + 127.5; break;
									case 498: m_angles->y = m_entity->GetLowerBodyYaw() - 127.5; break;
									case 499: m_angles->y = m_entity->GetLowerBodyYaw() + 128; break;
									case 500: m_angles->y = m_entity->GetLowerBodyYaw() - 128; break;
									case 501: m_angles->y = m_entity->GetLowerBodyYaw() + 128.5; break;
									case 502: m_angles->y = m_entity->GetLowerBodyYaw() - 128.5; break;
									case 503: m_angles->y = m_entity->GetLowerBodyYaw() + 129; break;
									case 504: m_angles->y = m_entity->GetLowerBodyYaw() - 129; break;
									case 505: m_angles->y = m_entity->GetLowerBodyYaw() + 129.5; break;
									case 506: m_angles->y = m_entity->GetLowerBodyYaw() - 129.5; break;
									case 507: m_angles->y = m_entity->GetLowerBodyYaw() + 130; break;
									case 508: m_angles->y = m_entity->GetLowerBodyYaw() - 130; break;
									case 509: m_angles->y = m_entity->GetLowerBodyYaw() + 130.5; break;
									case 510: m_angles->y = m_entity->GetLowerBodyYaw() - 130.5; break;
									case 511: m_angles->y = m_entity->GetLowerBodyYaw() + 131; break;
									case 512: m_angles->y = m_entity->GetLowerBodyYaw() - 131; break;
									case 513: m_angles->y = m_entity->GetLowerBodyYaw() + 131.5; break;
									case 514: m_angles->y = m_entity->GetLowerBodyYaw() - 131.5; break;
									case 515: m_angles->y = m_entity->GetLowerBodyYaw() + 132; break;
									case 516: m_angles->y = m_entity->GetLowerBodyYaw() - 132; break;
									case 517: m_angles->y = m_entity->GetLowerBodyYaw() + 132.5; break;
									case 518: m_angles->y = m_entity->GetLowerBodyYaw() - 132.5; break;
									case 519: m_angles->y = m_entity->GetLowerBodyYaw() + 133; break;
									case 520: m_angles->y = m_entity->GetLowerBodyYaw() - 133; break;
									case 521: m_angles->y = m_entity->GetLowerBodyYaw() + 134.5; break;
									case 522: m_angles->y = m_entity->GetLowerBodyYaw() - 134.5; break;
									case 523: m_angles->y = m_entity->GetLowerBodyYaw() + 135; break;
									case 524: m_angles->y = m_entity->GetLowerBodyYaw() - 135; break;
									case 525: m_angles->y = m_entity->GetLowerBodyYaw() + 135.5; break;
									case 526: m_angles->y = m_entity->GetLowerBodyYaw() - 135.5; break;
									case 527: m_angles->y = m_entity->GetLowerBodyYaw() + 136; break;
									case 528: m_angles->y = m_entity->GetLowerBodyYaw() - 136; break;
									case 529: m_angles->y = m_entity->GetLowerBodyYaw() + 136.5; break;
									case 530: m_angles->y = m_entity->GetLowerBodyYaw() - 136.5; break;
									case 531: m_angles->y = m_entity->GetLowerBodyYaw() + 137; break;
									case 532: m_angles->y = m_entity->GetLowerBodyYaw() - 137; break;
									case 533: m_angles->y = m_entity->GetLowerBodyYaw() + 137.5; break;
									case 534: m_angles->y = m_entity->GetLowerBodyYaw() - 137.5; break;
									case 535: m_angles->y = m_entity->GetLowerBodyYaw() + 138; break;
									case 536: m_angles->y = m_entity->GetLowerBodyYaw() - 138; break;
									case 537: m_angles->y = m_entity->GetLowerBodyYaw() + 138.5; break;
									case 538: m_angles->y = m_entity->GetLowerBodyYaw() - 138.5; break;
									case 539: m_angles->y = m_entity->GetLowerBodyYaw() + 140; break;
									case 540: m_angles->y = m_entity->GetLowerBodyYaw() - 140; break;
									case 541: m_angles->y = m_entity->GetLowerBodyYaw() + 140.5; break;
									case 542: m_angles->y = m_entity->GetLowerBodyYaw() - 140.5; break;
									case 543: m_angles->y = m_entity->GetLowerBodyYaw() + 141; break;
									case 544: m_angles->y = m_entity->GetLowerBodyYaw() - 141; break;
									case 545: m_angles->y = m_entity->GetLowerBodyYaw() + 141.5; break;
									case 546: m_angles->y = m_entity->GetLowerBodyYaw() - 141.5; break;
									case 547: m_angles->y = m_entity->GetLowerBodyYaw() + 142; break;
									case 548: m_angles->y = m_entity->GetLowerBodyYaw() - 142; break;
									case 549: m_angles->y = m_entity->GetLowerBodyYaw() + 142.5; break;
									case 550: m_angles->y = m_entity->GetLowerBodyYaw() - 142.5; break;
									case 551: m_angles->y = m_entity->GetLowerBodyYaw() + 143; break;
									case 552: m_angles->y = m_entity->GetLowerBodyYaw() - 143; break;
									case 553: m_angles->y = m_entity->GetLowerBodyYaw() + 143.5; break;
									case 554: m_angles->y = m_entity->GetLowerBodyYaw() - 143.5; break;
									case 555: m_angles->y = m_entity->GetLowerBodyYaw() + 144; break;
									case 556: m_angles->y = m_entity->GetLowerBodyYaw() - 144; break;
									case 557: m_angles->y = m_entity->GetLowerBodyYaw() + 144.5; break;
									case 558: m_angles->y = m_entity->GetLowerBodyYaw() - 144.5; break;
									case 559: m_angles->y = m_entity->GetLowerBodyYaw() + 145; break;
									case 560: m_angles->y = m_entity->GetLowerBodyYaw() - 145; break;
									case 561: m_angles->y = m_entity->GetLowerBodyYaw() + 145.5; break;
									case 562: m_angles->y = m_entity->GetLowerBodyYaw() - 145.5; break;
									case 563: m_angles->y = m_entity->GetLowerBodyYaw() + 146; break;
									case 564: m_angles->y = m_entity->GetLowerBodyYaw() - 146; break;
									case 565: m_angles->y = m_entity->GetLowerBodyYaw() + 146.5; break;
									case 566: m_angles->y = m_entity->GetLowerBodyYaw() - 146.5; break;
									case 567: m_angles->y = m_entity->GetLowerBodyYaw() + 147; break;
									case 568: m_angles->y = m_entity->GetLowerBodyYaw() - 147; break;
									case 569: m_angles->y = m_entity->GetLowerBodyYaw() + 147.5; break;
									case 570: m_angles->y = m_entity->GetLowerBodyYaw() - 147.5; break;
									case 571: m_angles->y = m_entity->GetLowerBodyYaw() + 148; break;
									case 572: m_angles->y = m_entity->GetLowerBodyYaw() - 148; break;
									case 573: m_angles->y = m_entity->GetLowerBodyYaw() + 148.5; break;
									case 574: m_angles->y = m_entity->GetLowerBodyYaw() - 148.5; break;
									case 575: m_angles->y = m_entity->GetLowerBodyYaw() + 149; break;
									case 576: m_angles->y = m_entity->GetLowerBodyYaw() - 149; break;
									case 577: m_angles->y = m_entity->GetLowerBodyYaw() + 149.5; break;
									case 578: m_angles->y = m_entity->GetLowerBodyYaw() - 149.5; break;
									case 579: m_angles->y = m_entity->GetLowerBodyYaw() + 150; break;
									case 580: m_angles->y = m_entity->GetLowerBodyYaw() - 150; break;
									case 581: m_angles->y = m_entity->GetLowerBodyYaw() + 150.5; break;
									case 582: m_angles->y = m_entity->GetLowerBodyYaw() - 150.5; break;
									case 583: m_angles->y = m_entity->GetLowerBodyYaw() + 151; break;
									case 584: m_angles->y = m_entity->GetLowerBodyYaw() - 151; break;
									case 585: m_angles->y = m_entity->GetLowerBodyYaw() + 151.5; break;
									case 586: m_angles->y = m_entity->GetLowerBodyYaw() - 151.5; break;
									case 587: m_angles->y = m_entity->GetLowerBodyYaw() + 152; break;
									case 588: m_angles->y = m_entity->GetLowerBodyYaw() - 152; break;
									case 589: m_angles->y = m_entity->GetLowerBodyYaw() + 152.5; break;
									case 590: m_angles->y = m_entity->GetLowerBodyYaw() - 152.5; break;
									case 591: m_angles->y = m_entity->GetLowerBodyYaw() + 153; break;
									case 592: m_angles->y = m_entity->GetLowerBodyYaw() - 153; break;
									case 593: m_angles->y = m_entity->GetLowerBodyYaw() + 153.5; break;
									case 594: m_angles->y = m_entity->GetLowerBodyYaw() - 153.5; break;
									case 595: m_angles->y = m_entity->GetLowerBodyYaw() + 154; break;
									case 596: m_angles->y = m_entity->GetLowerBodyYaw() - 154; break;
									case 597: m_angles->y = m_entity->GetLowerBodyYaw() + 154.5; break;
									case 598: m_angles->y = m_entity->GetLowerBodyYaw() - 154.5; break;
									case 599: m_angles->y = m_entity->GetLowerBodyYaw() + 155; break;
									case 600: m_angles->y = m_entity->GetLowerBodyYaw() - 155; break;
									case 601: m_angles->y = m_entity->GetLowerBodyYaw() + 155.5; break;
									case 602: m_angles->y = m_entity->GetLowerBodyYaw() - 155.5; break;
									case 603: m_angles->y = m_entity->GetLowerBodyYaw() + 156; break;
									case 604: m_angles->y = m_entity->GetLowerBodyYaw() - 156; break;
									case 605: m_angles->y = m_entity->GetLowerBodyYaw() + 156.5; break;
									case 606: m_angles->y = m_entity->GetLowerBodyYaw() - 156.5; break;
									case 607: m_angles->y = m_entity->GetLowerBodyYaw() + 157; break;
									case 608: m_angles->y = m_entity->GetLowerBodyYaw() - 157; break;
									case 609: m_angles->y = m_entity->GetLowerBodyYaw() + 157.5; break;
									case 610: m_angles->y = m_entity->GetLowerBodyYaw() - 157.5; break;
									case 611: m_angles->y = m_entity->GetLowerBodyYaw() + 158; break;
									case 612: m_angles->y = m_entity->GetLowerBodyYaw() - 158; break;
									case 613: m_angles->y = m_entity->GetLowerBodyYaw() + 158.5; break;
									case 614: m_angles->y = m_entity->GetLowerBodyYaw() - 158.5; break;
									case 615: m_angles->y = m_entity->GetLowerBodyYaw() + 159; break;
									case 616: m_angles->y = m_entity->GetLowerBodyYaw() - 159; break;
									case 617: m_angles->y = m_entity->GetLowerBodyYaw() + 159.5; break;
									case 618: m_angles->y = m_entity->GetLowerBodyYaw() - 159.5; break;
									case 619: m_angles->y = m_entity->GetLowerBodyYaw() + 160; break;
									case 620: m_angles->y = m_entity->GetLowerBodyYaw() - 160; break;
									case 621: m_angles->y = m_entity->GetLowerBodyYaw() + 160.5; break;
									case 622: m_angles->y = m_entity->GetLowerBodyYaw() - 160.5; break;
									case 623: m_angles->y = m_entity->GetLowerBodyYaw() + 161; break;
									case 624: m_angles->y = m_entity->GetLowerBodyYaw() - 161; break;
									case 625: m_angles->y = m_entity->GetLowerBodyYaw() + 161.5; break;
									case 626: m_angles->y = m_entity->GetLowerBodyYaw() - 161.5; break;
									case 627: m_angles->y = m_entity->GetLowerBodyYaw() + 162; break;
									case 628: m_angles->y = m_entity->GetLowerBodyYaw() - 162; break;
									case 629: m_angles->y = m_entity->GetLowerBodyYaw() + 162.5; break;
									case 630: m_angles->y = m_entity->GetLowerBodyYaw() - 162.5; break;
									case 631: m_angles->y = m_entity->GetLowerBodyYaw() + 163; break;
									case 632: m_angles->y = m_entity->GetLowerBodyYaw() - 163; break;
									case 633: m_angles->y = m_entity->GetLowerBodyYaw() + 163.5; break;
									case 634: m_angles->y = m_entity->GetLowerBodyYaw() - 163.5; break;
									case 635: m_angles->y = m_entity->GetLowerBodyYaw() + 164; break;
									case 636: m_angles->y = m_entity->GetLowerBodyYaw() - 164; break;
									case 637: m_angles->y = m_entity->GetLowerBodyYaw() + 164.5; break;
									case 638: m_angles->y = m_entity->GetLowerBodyYaw() - 164.5; break;
									case 639: m_angles->y = m_entity->GetLowerBodyYaw() + 165; break;
									case 640: m_angles->y = m_entity->GetLowerBodyYaw() - 165; break;
									case 641: m_angles->y = m_entity->GetLowerBodyYaw() + 165.5; break;
									case 642: m_angles->y = m_entity->GetLowerBodyYaw() - 165.5; break;
									case 643: m_angles->y = m_entity->GetLowerBodyYaw() + 166; break;
									case 644: m_angles->y = m_entity->GetLowerBodyYaw() - 166; break;
									case 645: m_angles->y = m_entity->GetLowerBodyYaw() + 166.5; break;
									case 646: m_angles->y = m_entity->GetLowerBodyYaw() - 166.5; break;
									case 647: m_angles->y = m_entity->GetLowerBodyYaw() + 167; break;
									case 648: m_angles->y = m_entity->GetLowerBodyYaw() - 167; break;
									case 649: m_angles->y = m_entity->GetLowerBodyYaw() + 167.5; break;
									case 650: m_angles->y = m_entity->GetLowerBodyYaw() - 167.5; break;
									case 651: m_angles->y = m_entity->GetLowerBodyYaw() + 168; break;
									case 652: m_angles->y = m_entity->GetLowerBodyYaw() - 168; break;
									case 653: m_angles->y = m_entity->GetLowerBodyYaw() + 168.5; break;
									case 654: m_angles->y = m_entity->GetLowerBodyYaw() - 168.5; break;
									case 655: m_angles->y = m_entity->GetLowerBodyYaw() + 169; break;
									case 656: m_angles->y = m_entity->GetLowerBodyYaw() - 169; break;
									case 657: m_angles->y = m_entity->GetLowerBodyYaw() + 169.5; break;
									case 658: m_angles->y = m_entity->GetLowerBodyYaw() - 169.5; break;
									case 659: m_angles->y = m_entity->GetLowerBodyYaw() + 170; break;
									case 660: m_angles->y = m_entity->GetLowerBodyYaw() - 170; break;
									case 661: m_angles->y = m_entity->GetLowerBodyYaw() + 170.5; break;
									case 662: m_angles->y = m_entity->GetLowerBodyYaw() - 170.5; break;
									case 663: m_angles->y = m_entity->GetLowerBodyYaw() + 171; break;
									case 664: m_angles->y = m_entity->GetLowerBodyYaw() - 171; break;
									case 665: m_angles->y = m_entity->GetLowerBodyYaw() + 171.5; break;
									case 666: m_angles->y = m_entity->GetLowerBodyYaw() - 171.5; break;
									case 667: m_angles->y = m_entity->GetLowerBodyYaw() + 172; break;
									case 668: m_angles->y = m_entity->GetLowerBodyYaw() - 172; break;
									case 669: m_angles->y = m_entity->GetLowerBodyYaw() + 172.5; break;
									case 670: m_angles->y = m_entity->GetLowerBodyYaw() - 172.5; break;
									case 671: m_angles->y = m_entity->GetLowerBodyYaw() + 173; break;
									case 672: m_angles->y = m_entity->GetLowerBodyYaw() - 173; break;
									case 673: m_angles->y = m_entity->GetLowerBodyYaw() + 173.5; break;
									case 674: m_angles->y = m_entity->GetLowerBodyYaw() - 173.5; break;
									case 675: m_angles->y = m_entity->GetLowerBodyYaw() + 174; break;
									case 676: m_angles->y = m_entity->GetLowerBodyYaw() - 174; break;
									case 677: m_angles->y = m_entity->GetLowerBodyYaw() + 174.5; break;
									case 678: m_angles->y = m_entity->GetLowerBodyYaw() - 174.5; break;
									case 679: m_angles->y = m_entity->GetLowerBodyYaw() + 175; break;
									case 680: m_angles->y = m_entity->GetLowerBodyYaw() - 175; break;
									case 681: m_angles->y = m_entity->GetLowerBodyYaw() + 175.5; break;
									case 682: m_angles->y = m_entity->GetLowerBodyYaw() - 175.5; break;
									case 683: m_angles->y = m_entity->GetLowerBodyYaw() + 176; break;
									case 684: m_angles->y = m_entity->GetLowerBodyYaw() - 176; break;
									case 685: m_angles->y = m_entity->GetLowerBodyYaw() + 176.5; break;
									case 686: m_angles->y = m_entity->GetLowerBodyYaw() - 176.5; break;
									case 687: m_angles->y = m_entity->GetLowerBodyYaw() + 177; break;
									case 688: m_angles->y = m_entity->GetLowerBodyYaw() - 177; break;
									case 689: m_angles->y = m_entity->GetLowerBodyYaw() + 177.5; break;
									case 690: m_angles->y = m_entity->GetLowerBodyYaw() - 177.5; break;
									case 691: m_angles->y = m_entity->GetLowerBodyYaw() + 178; break;
									case 692: m_angles->y = m_entity->GetLowerBodyYaw() - 178; break;
									case 693: m_angles->y = m_entity->GetLowerBodyYaw() + 178.5; break;
									case 694: m_angles->y = m_entity->GetLowerBodyYaw() - 178.5; break;
									case 695: m_angles->y = m_entity->GetLowerBodyYaw() + 179; break;
									case 696: m_angles->y = m_entity->GetLowerBodyYaw() - 179; break;
									case 697: m_angles->y = m_entity->GetLowerBodyYaw() + 179.5; break;
									case 698: m_angles->y = m_entity->GetLowerBodyYaw() - 179.5; break;
									case 699: m_angles->y = m_entity->GetLowerBodyYaw() + 180; break;
									case 700: m_angles->y = m_entity->GetLowerBodyYaw() - 180; break;
									case 701: m_angles->y = m_entity->GetLowerBodyYaw() + 180.5; break;
									case 702: m_angles->y = m_entity->GetLowerBodyYaw() - 180.5; break;
									case 703: m_angles->y = m_entity->GetLowerBodyYaw() + 181; break;
									case 704: m_angles->y = m_entity->GetLowerBodyYaw() - 181; break;
									case 705: m_angles->y = m_entity->GetLowerBodyYaw() + 181.5; break;
									case 706: m_angles->y = m_entity->GetLowerBodyYaw() - 181.5; break;
									case 707: m_angles->y = m_entity->GetLowerBodyYaw() + 182; break;
									case 708: m_angles->y = m_entity->GetLowerBodyYaw() - 182; break;
									case 709: m_angles->y = m_entity->GetLowerBodyYaw() + 182.5; break;
									case 710: m_angles->y = m_entity->GetLowerBodyYaw() - 182.5; break;
									case 711: m_angles->y = m_entity->GetLowerBodyYaw() + 183; break;
									case 712: m_angles->y = m_entity->GetLowerBodyYaw() - 183; break;
									case 713: m_angles->y = m_entity->GetLowerBodyYaw() + 183.5; break;
									case 714: m_angles->y = m_entity->GetLowerBodyYaw() - 183.5; break;
									case 715: m_angles->y = m_entity->GetLowerBodyYaw() + 184; break;
									case 716: m_angles->y = m_entity->GetLowerBodyYaw() - 184; break;
									case 717: m_angles->y = m_entity->GetLowerBodyYaw() + 184.5; break;
									case 718: m_angles->y = m_entity->GetLowerBodyYaw() - 184.5; break;
									case 719: m_angles->y = m_entity->GetLowerBodyYaw() + 185; break;
									case 720: m_angles->y = m_entity->GetLowerBodyYaw() - 185; break;
									case 721: m_angles->y = m_entity->GetLowerBodyYaw() + 185.5; break;
									case 722: m_angles->y = m_entity->GetLowerBodyYaw() - 185.5; break;
									case 723: m_angles->y = m_entity->GetLowerBodyYaw() + 186; break;
									case 724: m_angles->y = m_entity->GetLowerBodyYaw() - 186; break;
									case 725: m_angles->y = m_entity->GetLowerBodyYaw() + 186.5; break;
									case 726: m_angles->y = m_entity->GetLowerBodyYaw() - 186.5; break;
									case 727: m_angles->y = m_entity->GetLowerBodyYaw() + 187; break;
									case 728: m_angles->y = m_entity->GetLowerBodyYaw() - 187; break;
									case 729: m_angles->y = m_entity->GetLowerBodyYaw() + 187.5; break;
									case 730: m_angles->y = m_entity->GetLowerBodyYaw() - 187.5; break;
									case 731: m_angles->y = m_entity->GetLowerBodyYaw() + 188; break;
									case 732: m_angles->y = m_entity->GetLowerBodyYaw() - 188; break;
									case 733: m_angles->y = m_entity->GetLowerBodyYaw() + 188.5; break;
									case 734: m_angles->y = m_entity->GetLowerBodyYaw() - 188.5; break;
									case 735: m_angles->y = m_entity->GetLowerBodyYaw() + 189; break;
									case 736: m_angles->y = m_entity->GetLowerBodyYaw() - 189; break;
									case 737: m_angles->y = m_entity->GetLowerBodyYaw() + 189.5; break;
									case 738: m_angles->y = m_entity->GetLowerBodyYaw() - 189.5; break;
									case 739: m_angles->y = m_entity->GetLowerBodyYaw() + 190; break;
									case 740: m_angles->y = m_entity->GetLowerBodyYaw() - 190; break;
									case 741: m_angles->y = m_entity->GetLowerBodyYaw() + 190.5; break;
									case 742: m_angles->y = m_entity->GetLowerBodyYaw() - 190.5; break;
									case 743: m_angles->y = m_entity->GetLowerBodyYaw() + 191; break;
									case 744: m_angles->y = m_entity->GetLowerBodyYaw() - 191; break;
									case 745: m_angles->y = m_entity->GetLowerBodyYaw() + 191.5; break;
									case 746: m_angles->y = m_entity->GetLowerBodyYaw() - 191.5; break;
									case 747: m_angles->y = m_entity->GetLowerBodyYaw() + 192; break;
									case 748: m_angles->y = m_entity->GetLowerBodyYaw() - 192; break;
									case 749: m_angles->y = m_entity->GetLowerBodyYaw() + 192.5; break;
									case 750: m_angles->y = m_entity->GetLowerBodyYaw() - 192.5; break;
									case 751: m_angles->y = m_entity->GetLowerBodyYaw() + 193; break;
									case 752: m_angles->y = m_entity->GetLowerBodyYaw() - 193; break;
									case 753: m_angles->y = m_entity->GetLowerBodyYaw() + 193.5; break;
									case 754: m_angles->y = m_entity->GetLowerBodyYaw() - 193.5; break;
									case 755: m_angles->y = m_entity->GetLowerBodyYaw() + 194; break;
									case 756: m_angles->y = m_entity->GetLowerBodyYaw() - 194; break;
									case 757: m_angles->y = m_entity->GetLowerBodyYaw() + 194.5; break;
									case 758: m_angles->y = m_entity->GetLowerBodyYaw() - 194.5; break;
									case 759: m_angles->y = m_entity->GetLowerBodyYaw() + 195; break;
									case 760: m_angles->y = m_entity->GetLowerBodyYaw() - 195; break;
									case 761: m_angles->y = m_entity->GetLowerBodyYaw() + 195.5; break;
									case 762: m_angles->y = m_entity->GetLowerBodyYaw() - 195.5; break;
									case 763: m_angles->y = m_entity->GetLowerBodyYaw() + 196; break;
									case 764: m_angles->y = m_entity->GetLowerBodyYaw() - 196; break;
									case 765: m_angles->y = m_entity->GetLowerBodyYaw() + 196.5; break;
									case 766: m_angles->y = m_entity->GetLowerBodyYaw() - 196.5; break;
									case 767: m_angles->y = m_entity->GetLowerBodyYaw() + 197; break;
									case 768: m_angles->y = m_entity->GetLowerBodyYaw() - 197; break;
									case 769: m_angles->y = m_entity->GetLowerBodyYaw() + 197.5; break;
									case 770: m_angles->y = m_entity->GetLowerBodyYaw() - 197.5; break;
									case 771: m_angles->y = m_entity->GetLowerBodyYaw() + 198; break;
									case 772: m_angles->y = m_entity->GetLowerBodyYaw() - 198; break;
									case 773: m_angles->y = m_entity->GetLowerBodyYaw() + 198.5; break;
									case 774: m_angles->y = m_entity->GetLowerBodyYaw() - 198.5; break;
									case 775: m_angles->y = m_entity->GetLowerBodyYaw() + 199; break;
									case 776: m_angles->y = m_entity->GetLowerBodyYaw() - 199; break;
									case 777: m_angles->y = m_entity->GetLowerBodyYaw() + 199.5; break;
									case 778: m_angles->y = m_entity->GetLowerBodyYaw() - 199.5; break;
									case 779: m_angles->y = m_entity->GetLowerBodyYaw() + 200; break;
									case 780: m_angles->y = m_entity->GetLowerBodyYaw() - 200; break;
									case 781: m_angles->y = m_entity->GetLowerBodyYaw() + 200.5; break;
									case 782: m_angles->y = m_entity->GetLowerBodyYaw() - 200.5; break;
									case 783: m_angles->y = m_entity->GetLowerBodyYaw() + 201; break;
									case 784: m_angles->y = m_entity->GetLowerBodyYaw() - 201; break;
									case 785: m_angles->y = m_entity->GetLowerBodyYaw() + 201.5; break;
									case 786: m_angles->y = m_entity->GetLowerBodyYaw() - 201.5; break;
									case 787: m_angles->y = m_entity->GetLowerBodyYaw() + 202; break;
									case 788: m_angles->y = m_entity->GetLowerBodyYaw() - 202; break;
									case 789: m_angles->y = m_entity->GetLowerBodyYaw() + 202.5; break;
									case 790: m_angles->y = m_entity->GetLowerBodyYaw() - 202.5; break;
									case 791: m_angles->y = m_entity->GetLowerBodyYaw() + 203; break;
									case 792: m_angles->y = m_entity->GetLowerBodyYaw() - 203; break;
									case 793: m_angles->y = m_entity->GetLowerBodyYaw() + 203.5; break;
									case 794: m_angles->y = m_entity->GetLowerBodyYaw() - 203.5; break;
									case 795: m_angles->y = m_entity->GetLowerBodyYaw() + 204; break;
									case 796: m_angles->y = m_entity->GetLowerBodyYaw() - 204; break;
									case 797: m_angles->y = m_entity->GetLowerBodyYaw() + 204.5; break;
									case 798: m_angles->y = m_entity->GetLowerBodyYaw() - 204.5; break;
									case 799: m_angles->y = m_entity->GetLowerBodyYaw() + 205; break;
									case 800: m_angles->y = m_entity->GetLowerBodyYaw() - 205; break;
									case 801: m_angles->y = m_entity->GetLowerBodyYaw() + 205.5; break;
									case 802: m_angles->y = m_entity->GetLowerBodyYaw() - 205.5; break;
									case 803: m_angles->y = m_entity->GetLowerBodyYaw() + 206; break;
									case 804: m_angles->y = m_entity->GetLowerBodyYaw() - 206; break;
									case 805: m_angles->y = m_entity->GetLowerBodyYaw() + 206.5; break;
									case 806: m_angles->y = m_entity->GetLowerBodyYaw() - 206.5; break;
									case 807: m_angles->y = m_entity->GetLowerBodyYaw() + 207; break;
									case 808: m_angles->y = m_entity->GetLowerBodyYaw() - 207; break;
									case 809: m_angles->y = m_entity->GetLowerBodyYaw() + 207.5; break;
									case 810: m_angles->y = m_entity->GetLowerBodyYaw() - 207.5; break;
									case 811: m_angles->y = m_entity->GetLowerBodyYaw() + 208; break;
									case 812: m_angles->y = m_entity->GetLowerBodyYaw() - 208; break;
									case 813: m_angles->y = m_entity->GetLowerBodyYaw() + 208.5; break;
									case 814: m_angles->y = m_entity->GetLowerBodyYaw() - 208.5; break;
									case 815: m_angles->y = m_entity->GetLowerBodyYaw() + 209; break;
									case 816: m_angles->y = m_entity->GetLowerBodyYaw() - 209; break;
									case 817: m_angles->y = m_entity->GetLowerBodyYaw() + 209.5; break;
									case 818: m_angles->y = m_entity->GetLowerBodyYaw() - 209.5; break;
									case 819: m_angles->y = m_entity->GetLowerBodyYaw() + 210; break;
									case 820: m_angles->y = m_entity->GetLowerBodyYaw() - 210; break;
									case 821: m_angles->y = m_entity->GetLowerBodyYaw() + 210.5; break;
									case 822: m_angles->y = m_entity->GetLowerBodyYaw() - 210.5; break;
									case 823: m_angles->y = m_entity->GetLowerBodyYaw() + 211; break;
									case 824: m_angles->y = m_entity->GetLowerBodyYaw() - 211; break;
									case 825: m_angles->y = m_entity->GetLowerBodyYaw() + 211.5; break;
									case 826: m_angles->y = m_entity->GetLowerBodyYaw() - 211.5; break;
									case 827: m_angles->y = m_entity->GetLowerBodyYaw() + 212; break;
									case 828: m_angles->y = m_entity->GetLowerBodyYaw() - 212; break;
									case 829: m_angles->y = m_entity->GetLowerBodyYaw() - 212.5; break;
									case 830: m_angles->y = m_entity->GetLowerBodyYaw() + 213; break;
									case 831: m_angles->y = m_entity->GetLowerBodyYaw() - 213; break;
									case 832: m_angles->y = m_entity->GetLowerBodyYaw() + 213.5; break;
									case 833: m_angles->y = m_entity->GetLowerBodyYaw() - 213.5; break;
									case 834: m_angles->y = m_entity->GetLowerBodyYaw() + 214; break;
									case 835: m_angles->y = m_entity->GetLowerBodyYaw() - 214; break;
									case 836: m_angles->y = m_entity->GetLowerBodyYaw() + 214.5; break;
									case 837: m_angles->y = m_entity->GetLowerBodyYaw() - 214.5; break;
									case 838: m_angles->y = m_entity->GetLowerBodyYaw() + 215; break;
									case 839: m_angles->y = m_entity->GetLowerBodyYaw() - 215; break;
									case 840: m_angles->y = m_entity->GetLowerBodyYaw() + 215.5; break;
									case 841: m_angles->y = m_entity->GetLowerBodyYaw() - 215.5; break;
									case 842: m_angles->y = m_entity->GetLowerBodyYaw() + 216; break;
									case 843: m_angles->y = m_entity->GetLowerBodyYaw() - 216; break;
									case 844: m_angles->y = m_entity->GetLowerBodyYaw() + 216.5; break;
									case 845: m_angles->y = m_entity->GetLowerBodyYaw() - 216.5; break;
									case 846: m_angles->y = m_entity->GetLowerBodyYaw() + 217; break;
									case 847: m_angles->y = m_entity->GetLowerBodyYaw() - 217; break;
									case 848: m_angles->y = m_entity->GetLowerBodyYaw() + 217.5; break;
									case 849: m_angles->y = m_entity->GetLowerBodyYaw() - 217.5; break;
									case 850: m_angles->y = m_entity->GetLowerBodyYaw() + 218; break;
									case 851: m_angles->y = m_entity->GetLowerBodyYaw() - 218; break;
									case 852: m_angles->y = m_entity->GetLowerBodyYaw() + 218.5; break;
									case 853: m_angles->y = m_entity->GetLowerBodyYaw() - 218.5; break;
									case 854: m_angles->y = m_entity->GetLowerBodyYaw() + 219; break;
									case 855: m_angles->y = m_entity->GetLowerBodyYaw() - 219; break;
									case 856: m_angles->y = m_entity->GetLowerBodyYaw() + 219.5; break;
									case 857: m_angles->y = m_entity->GetLowerBodyYaw() - 219.5; break;
									case 858: m_angles->y = m_entity->GetLowerBodyYaw() + 220; break;
									case 859: m_angles->y = m_entity->GetLowerBodyYaw() - 220; break;
									case 860: m_angles->y = m_entity->GetLowerBodyYaw() + 220.5; break;
									case 861: m_angles->y = m_entity->GetLowerBodyYaw() - 220.5; break;
									case 862: m_angles->y = m_entity->GetLowerBodyYaw() + 221; break;
									case 863: m_angles->y = m_entity->GetLowerBodyYaw() - 221; break;
									case 864: m_angles->y = m_entity->GetLowerBodyYaw() + 221.5; break;
									case 865: m_angles->y = m_entity->GetLowerBodyYaw() - 221.5; break;
									case 866: m_angles->y = m_entity->GetLowerBodyYaw() + 222; break;
									case 867: m_angles->y = m_entity->GetLowerBodyYaw() - 222; break;
									case 868: m_angles->y = m_entity->GetLowerBodyYaw() + 222.5; break;
									case 869: m_angles->y = m_entity->GetLowerBodyYaw() - 222.5; break;
									case 870: m_angles->y = m_entity->GetLowerBodyYaw() + 223; break;
									case 871: m_angles->y = m_entity->GetLowerBodyYaw() - 223; break;
									case 872: m_angles->y = m_entity->GetLowerBodyYaw() + 223.5; break;
									case 873: m_angles->y = m_entity->GetLowerBodyYaw() - 223.5; break;
									case 874: m_angles->y = m_entity->GetLowerBodyYaw() + 224; break;
									case 875: m_angles->y = m_entity->GetLowerBodyYaw() - 224; break;
									case 876: m_angles->y = m_entity->GetLowerBodyYaw() + 224.5; break;
									case 877: m_angles->y = m_entity->GetLowerBodyYaw() - 224.5; break;
									case 878: m_angles->y = m_entity->GetLowerBodyYaw() + 225; break;
									case 879: m_angles->y = m_entity->GetLowerBodyYaw() - 225; break;
									case 880: m_angles->y = m_entity->GetLowerBodyYaw() + 225.5; break;
									case 881: m_angles->y = m_entity->GetLowerBodyYaw() - 225.5; break;
									case 882: m_angles->y = m_entity->GetLowerBodyYaw() + 226; break;
									case 883: m_angles->y = m_entity->GetLowerBodyYaw() - 226; break;
									case 884: m_angles->y = m_entity->GetLowerBodyYaw() + 226.5; break;
									case 885: m_angles->y = m_entity->GetLowerBodyYaw() - 226.5; break;
									case 886: m_angles->y = m_entity->GetLowerBodyYaw() + 227; break;
									case 887: m_angles->y = m_entity->GetLowerBodyYaw() - 227; break;
									case 888: m_angles->y = m_entity->GetLowerBodyYaw() + 227.5; break;
									case 889: m_angles->y = m_entity->GetLowerBodyYaw() - 227.5; break;
									case 900: m_angles->y = m_entity->GetLowerBodyYaw() + 228; break;
									case 901: m_angles->y = m_entity->GetLowerBodyYaw() - 228; break;
									case 902: m_angles->y = m_entity->GetLowerBodyYaw() + 228.5; break;
									case 903: m_angles->y = m_entity->GetLowerBodyYaw() - 228.5; break;
									case 904: m_angles->y = m_entity->GetLowerBodyYaw() + 229; break;
									case 905: m_angles->y = m_entity->GetLowerBodyYaw() - 229; break;
									case 906: m_angles->y = m_entity->GetLowerBodyYaw() + 229.5; break;
									case 907: m_angles->y = m_entity->GetLowerBodyYaw() - 229.5; break;
									case 908: m_angles->y = m_entity->GetLowerBodyYaw() + 230; break;
									case 909: m_angles->y = m_entity->GetLowerBodyYaw() - 230; break;
									case 910: m_angles->y = m_entity->GetLowerBodyYaw() + 230.5; break;
									case 911: m_angles->y = m_entity->GetLowerBodyYaw() - 230.5; break;
									case 912: m_angles->y = m_entity->GetLowerBodyYaw() + 231; break;
									case 913: m_angles->y = m_entity->GetLowerBodyYaw() - 231; break;
									case 914: m_angles->y = m_entity->GetLowerBodyYaw() + 231.5; break;
									case 915: m_angles->y = m_entity->GetLowerBodyYaw() - 231.5; break;
									case 916: m_angles->y = m_entity->GetLowerBodyYaw() + 232; break;
									case 917: m_angles->y = m_entity->GetLowerBodyYaw() - 232; break;
									case 918: m_angles->y = m_entity->GetLowerBodyYaw() + 232.5; break;
									case 919: m_angles->y = m_entity->GetLowerBodyYaw() - 232.5; break;
									case 920: m_angles->y = m_entity->GetLowerBodyYaw() + 233; break;
									case 921: m_angles->y = m_entity->GetLowerBodyYaw() - 233; break;
									case 922: m_angles->y = m_entity->GetLowerBodyYaw() + 233.5; break;
									case 923: m_angles->y = m_entity->GetLowerBodyYaw() - 233.5; break;
									case 924: m_angles->y = m_entity->GetLowerBodyYaw() + 234; break;
									case 925: m_angles->y = m_entity->GetLowerBodyYaw() - 234; break;
									case 926: m_angles->y = m_entity->GetLowerBodyYaw() + 234.5; break;
									case 927: m_angles->y = m_entity->GetLowerBodyYaw() - 234.5; break;
									case 928: m_angles->y = m_entity->GetLowerBodyYaw() + 235; break;
									case 929: m_angles->y = m_entity->GetLowerBodyYaw() - 235; break;
									case 930: m_angles->y = m_entity->GetLowerBodyYaw() + 235.5; break;
									case 931: m_angles->y = m_entity->GetLowerBodyYaw() - 235.5; break;
									case 932: m_angles->y = m_entity->GetLowerBodyYaw() + 236; break;
									case 933: m_angles->y = m_entity->GetLowerBodyYaw() - 236; break;
									case 934: m_angles->y = m_entity->GetLowerBodyYaw() + 236.5; break;
									case 935: m_angles->y = m_entity->GetLowerBodyYaw() - 236.5; break;
									case 936: m_angles->y = m_entity->GetLowerBodyYaw() + 237; break;
									case 937: m_angles->y = m_entity->GetLowerBodyYaw() - 237; break;
									case 938: m_angles->y = m_entity->GetLowerBodyYaw() + 237.5; break;
									case 939: m_angles->y = m_entity->GetLowerBodyYaw() - 237.5; break;
									case 940: m_angles->y = m_entity->GetLowerBodyYaw() + 238; break;
									case 941: m_angles->y = m_entity->GetLowerBodyYaw() - 238; break;
									case 942: m_angles->y = m_entity->GetLowerBodyYaw() + 238.5; break;
									case 943: m_angles->y = m_entity->GetLowerBodyYaw() - 238.5; break;
									case 944: m_angles->y = m_entity->GetLowerBodyYaw() + 239; break;
									case 945: m_angles->y = m_entity->GetLowerBodyYaw() - 239; break;
									case 946: m_angles->y = m_entity->GetLowerBodyYaw() + 239.5; break;
									case 947: m_angles->y = m_entity->GetLowerBodyYaw() - 239.5; break;
									case 948: m_angles->y = m_entity->GetLowerBodyYaw() + 240; break;
									case 949: m_angles->y = m_entity->GetLowerBodyYaw() - 240; break;
									case 950: m_angles->y = m_entity->GetLowerBodyYaw() + 240.5; break;
									case 951: m_angles->y = m_entity->GetLowerBodyYaw() - 240.5; break;
									case 952: m_angles->y = m_entity->GetLowerBodyYaw() + 241; break;
									case 953: m_angles->y = m_entity->GetLowerBodyYaw() - 241; break;
									case 954: m_angles->y = m_entity->GetLowerBodyYaw() + 241.5; break;
									case 955: m_angles->y = m_entity->GetLowerBodyYaw() - 241.5; break;
									case 956: m_angles->y = m_entity->GetLowerBodyYaw() + 242; break;
									case 957: m_angles->y = m_entity->GetLowerBodyYaw() - 242; break;
									case 958: m_angles->y = m_entity->GetLowerBodyYaw() + 242.5; break;
									case 959: m_angles->y = m_entity->GetLowerBodyYaw() - 242.5; break;
									case 960: m_angles->y = m_entity->GetLowerBodyYaw() + 243; break;
									case 961: m_angles->y = m_entity->GetLowerBodyYaw() - 243; break;
									case 962: m_angles->y = m_entity->GetLowerBodyYaw() + 243.5; break;
									case 963: m_angles->y = m_entity->GetLowerBodyYaw() - 243.5; break;
									case 964: m_angles->y = m_entity->GetLowerBodyYaw() + 244; break;
									case 965: m_angles->y = m_entity->GetLowerBodyYaw() - 244; break;
									case 966: m_angles->y = m_entity->GetLowerBodyYaw() + 244.5; break;
									case 967: m_angles->y = m_entity->GetLowerBodyYaw() - 244.5; break;
									case 968: m_angles->y = m_entity->GetLowerBodyYaw() + 245; break;
									case 969: m_angles->y = m_entity->GetLowerBodyYaw() - 245; break;
									case 970: m_angles->y = m_entity->GetLowerBodyYaw() + 245.5; break;
									case 971: m_angles->y = m_entity->GetLowerBodyYaw() - 245.5; break;
									case 972: m_angles->y = m_entity->GetLowerBodyYaw() + 246; break;
									case 973: m_angles->y = m_entity->GetLowerBodyYaw() - 246; break;
									case 974: m_angles->y = m_entity->GetLowerBodyYaw() + 246.5; break;
									case 975: m_angles->y = m_entity->GetLowerBodyYaw() - 246.5; break;
									case 976: m_angles->y = m_entity->GetLowerBodyYaw() + 247; break;
									case 977: m_angles->y = m_entity->GetLowerBodyYaw() - 247; break;
									case 978: m_angles->y = m_entity->GetLowerBodyYaw() + 247.5; break;
									case 979: m_angles->y = m_entity->GetLowerBodyYaw() - 247.5; break;
									case 980: m_angles->y = m_entity->GetLowerBodyYaw() + 248; break;
									case 981: m_angles->y = m_entity->GetLowerBodyYaw() - 248; break;
									case 982: m_angles->y = m_entity->GetLowerBodyYaw() + 248.5; break;
									case 983: m_angles->y = m_entity->GetLowerBodyYaw() - 248.5; break;
									case 984: m_angles->y = m_entity->GetLowerBodyYaw() + 249; break;
									case 985: m_angles->y = m_entity->GetLowerBodyYaw() - 249; break;
									case 986: m_angles->y = m_entity->GetLowerBodyYaw() + 249.5; break;
									case 987: m_angles->y = m_entity->GetLowerBodyYaw() - 249.5; break;
									case 988: m_angles->y = m_entity->GetLowerBodyYaw() + 250; break;
									case 989: m_angles->y = m_entity->GetLowerBodyYaw() - 250; break;
									case 990: m_angles->y = m_entity->GetLowerBodyYaw() + 250.5; break;
									case 991: m_angles->y = m_entity->GetLowerBodyYaw() - 250.5; break;
									case 992: m_angles->y = m_entity->GetLowerBodyYaw() + 251; break;
									case 993: m_angles->y = m_entity->GetLowerBodyYaw() - 251; break;
									case 994: m_angles->y = m_entity->GetLowerBodyYaw() + 251.5; break;
									case 995: m_angles->y = m_entity->GetLowerBodyYaw() - 251.5; break;
									case 996: m_angles->y = m_entity->GetLowerBodyYaw() + 252; break;
									case 997: m_angles->y = m_entity->GetLowerBodyYaw() - 252; break;
									case 998: m_angles->y = m_entity->GetLowerBodyYaw() + 252.5; break;
									case 999: m_angles->y = m_entity->GetLowerBodyYaw() - 252.5; break;
									case 1000: m_angles->y = m_entity->GetLowerBodyYaw() + 253; break;
									case 1001: m_angles->y = m_entity->GetLowerBodyYaw() - 253; break;
									case 1002: m_angles->y = m_entity->GetLowerBodyYaw() + 253.5; break;
									case 1003: m_angles->y = m_entity->GetLowerBodyYaw() - 253.5; break;
									case 1004: m_angles->y = m_entity->GetLowerBodyYaw() + 254; break;
									case 1005: m_angles->y = m_entity->GetLowerBodyYaw() - 254; break;
									case 1006: m_angles->y = m_entity->GetLowerBodyYaw() + 254.5; break;
									case 1007: m_angles->y = m_entity->GetLowerBodyYaw() - 254.5; break;
									case 1008: m_angles->y = m_entity->GetLowerBodyYaw() + 255; break;
									case 1009: m_angles->y = m_entity->GetLowerBodyYaw() - 255; break;
									case 1010: m_angles->y = m_entity->GetLowerBodyYaw() + 255.5; break;
									case 1011: m_angles->y = m_entity->GetLowerBodyYaw() - 255.5; break;
									case 1012: m_angles->y = m_entity->GetLowerBodyYaw() + 256; break;
									case 1013: m_angles->y = m_entity->GetLowerBodyYaw() - 256; break;
									case 1014: m_angles->y = m_entity->GetLowerBodyYaw() + 256.5; break;
									case 1015: m_angles->y = m_entity->GetLowerBodyYaw() - 256.5; break;
									case 1016: m_angles->y = m_entity->GetLowerBodyYaw() + 257; break;
									case 1017: m_angles->y = m_entity->GetLowerBodyYaw() - 257; break;
									case 1018: m_angles->y = m_entity->GetLowerBodyYaw() + 257.5; break;
									case 1019: m_angles->y = m_entity->GetLowerBodyYaw() - 257.5; break;
									case 1020: m_angles->y = m_entity->GetLowerBodyYaw() + 258; break;
									case 1021: m_angles->y = m_entity->GetLowerBodyYaw() - 258; break;
									case 1022: m_angles->y = m_entity->GetLowerBodyYaw() + 258.5; break;
									case 1023: m_angles->y = m_entity->GetLowerBodyYaw() - 258.5; break;
									case 1024: m_angles->y = m_entity->GetLowerBodyYaw() + 259; break;
									case 1025: m_angles->y = m_entity->GetLowerBodyYaw() - 259; break;
									case 1026: m_angles->y = m_entity->GetLowerBodyYaw() + 259.5; break;
									case 1027: m_angles->y = m_entity->GetLowerBodyYaw() - 259.5; break;
									case 1028: m_angles->y = m_entity->GetLowerBodyYaw() + 260; break;
									case 1029: m_angles->y = m_entity->GetLowerBodyYaw() - 260; break;
									case 1030: m_angles->y = m_entity->GetLowerBodyYaw() + 260.5; break;
									case 1031: m_angles->y = m_entity->GetLowerBodyYaw() - 260.5; break;
									case 1032: m_angles->y = m_entity->GetLowerBodyYaw() + 261; break;
									case 1033: m_angles->y = m_entity->GetLowerBodyYaw() - 261; break;
									case 1034: m_angles->y = m_entity->GetLowerBodyYaw() + 261.5; break;
									case 1035: m_angles->y = m_entity->GetLowerBodyYaw() - 261.5; break;
									case 1036: m_angles->y = m_entity->GetLowerBodyYaw() + 262; break;
									case 1037: m_angles->y = m_entity->GetLowerBodyYaw() - 262; break;
									case 1038: m_angles->y = m_entity->GetLowerBodyYaw() + 262.5; break;
									case 1039: m_angles->y = m_entity->GetLowerBodyYaw() - 262.5; break;
									case 1040: m_angles->y = m_entity->GetLowerBodyYaw() + 263; break;
									case 1041: m_angles->y = m_entity->GetLowerBodyYaw() - 263; break;
									case 1042: m_angles->y = m_entity->GetLowerBodyYaw() + 263.5; break;
									case 1043: m_angles->y = m_entity->GetLowerBodyYaw() - 263.5; break;
									case 1044: m_angles->y = m_entity->GetLowerBodyYaw() + 264; break;
									case 1045: m_angles->y = m_entity->GetLowerBodyYaw() - 264; break;
									case 1046: m_angles->y = m_entity->GetLowerBodyYaw() + 264.5; break;
									case 1047: m_angles->y = m_entity->GetLowerBodyYaw() - 264.5; break;
									case 1048: m_angles->y = m_entity->GetLowerBodyYaw() + 265; break;
									case 1049: m_angles->y = m_entity->GetLowerBodyYaw() - 265; break;
									case 1050: m_angles->y = m_entity->GetLowerBodyYaw() + 265.5; break;
									case 1051: m_angles->y = m_entity->GetLowerBodyYaw() - 265.5; break;
									case 1052: m_angles->y = m_entity->GetLowerBodyYaw() + 266; break;
									case 1053: m_angles->y = m_entity->GetLowerBodyYaw() - 266; break;
									case 1054: m_angles->y = m_entity->GetLowerBodyYaw() + 266.5; break;
									case 1055: m_angles->y = m_entity->GetLowerBodyYaw() - 266.5; break;
									case 1056: m_angles->y = m_entity->GetLowerBodyYaw() + 267; break;
									case 1057: m_angles->y = m_entity->GetLowerBodyYaw() - 267; break;
									case 1058: m_angles->y = m_entity->GetLowerBodyYaw() + 267.5; break;
									case 1059: m_angles->y = m_entity->GetLowerBodyYaw() - 267.5; break;
									case 1060: m_angles->y = m_entity->GetLowerBodyYaw() + 268; break;
									case 1061: m_angles->y = m_entity->GetLowerBodyYaw() + 268.5; break;
									case 1062: m_angles->y = m_entity->GetLowerBodyYaw() - 268.5; break;
									case 1063: m_angles->y = m_entity->GetLowerBodyYaw() + 269; break;
									case 1064: m_angles->y = m_entity->GetLowerBodyYaw() - 269; break;
									case 1065: m_angles->y = m_entity->GetLowerBodyYaw() + 269.5; break;
									case 1066: m_angles->y = m_entity->GetLowerBodyYaw() - 269.5; break;
									case 1067: m_angles->y = m_entity->GetLowerBodyYaw() + 270; break;
									case 1068: m_angles->y = m_entity->GetLowerBodyYaw() - 270; break;
									case 1069: m_angles->y = m_entity->GetLowerBodyYaw() + 270.5; break;
									case 1070: m_angles->y = m_entity->GetLowerBodyYaw() - 270.5; break;
									case 1071: m_angles->y = m_entity->GetLowerBodyYaw() + 271; break;
									case 1072: m_angles->y = m_entity->GetLowerBodyYaw() - 271; break;
									case 1073: m_angles->y = m_entity->GetLowerBodyYaw() + 271.5; break;
									case 1074: m_angles->y = m_entity->GetLowerBodyYaw() - 271.5; break;
									case 1075: m_angles->y = m_entity->GetLowerBodyYaw() + 272; break;
									case 1076: m_angles->y = m_entity->GetLowerBodyYaw() - 272; break;
									case 1077: m_angles->y = m_entity->GetLowerBodyYaw() + 272.5; break;
									case 1078: m_angles->y = m_entity->GetLowerBodyYaw() - 272.5; break;
									case 1079: m_angles->y = m_entity->GetLowerBodyYaw() + 273; break;
									case 1080: m_angles->y = m_entity->GetLowerBodyYaw() - 273; break;
									case 1081: m_angles->y = m_entity->GetLowerBodyYaw() + 273.5; break;
									case 1082: m_angles->y = m_entity->GetLowerBodyYaw() - 273.5; break;
									case 1083: m_angles->y = m_entity->GetLowerBodyYaw() + 274; break;
									case 1084: m_angles->y = m_entity->GetLowerBodyYaw() - 274; break;
									case 1085: m_angles->y = m_entity->GetLowerBodyYaw() + 274.5; break;
									case 1086: m_angles->y = m_entity->GetLowerBodyYaw() - 274.5; break;
									case 1087: m_angles->y = m_entity->GetLowerBodyYaw() + 275; break;
									case 1088: m_angles->y = m_entity->GetLowerBodyYaw() - 275; break;
									case 1089: m_angles->y = m_entity->GetLowerBodyYaw() + 275.5; break;
									case 1090: m_angles->y = m_entity->GetLowerBodyYaw() - 275.5; break;
									case 1091: m_angles->y = m_entity->GetLowerBodyYaw() + 276; break;
									case 1092: m_angles->y = m_entity->GetLowerBodyYaw() - 276; break;
									case 1093: m_angles->y = m_entity->GetLowerBodyYaw() + 276.5; break;
									case 1094: m_angles->y = m_entity->GetLowerBodyYaw() - 276.5; break;
									case 1095: m_angles->y = m_entity->GetLowerBodyYaw() + 277; break;
									case 1096: m_angles->y = m_entity->GetLowerBodyYaw() - 277; break;
									case 1097: m_angles->y = m_entity->GetLowerBodyYaw() + 277.5; break;
									case 1098: m_angles->y = m_entity->GetLowerBodyYaw() - 277.5; break;
									case 1099: m_angles->y = m_entity->GetLowerBodyYaw() + 278; break;
									case 1100: m_angles->y = m_entity->GetLowerBodyYaw() - 278; break;
									case 1101: m_angles->y = m_entity->GetLowerBodyYaw() + 278.5; break;
									case 1102: m_angles->y = m_entity->GetLowerBodyYaw() - 278.5; break;
									case 1103: m_angles->y = m_entity->GetLowerBodyYaw() + 279; break;
									case 1104: m_angles->y = m_entity->GetLowerBodyYaw() - 279; break;
									case 1105: m_angles->y = m_entity->GetLowerBodyYaw() + 279.5; break;
									case 1106: m_angles->y = m_entity->GetLowerBodyYaw() - 279.5; break;
									case 1107: m_angles->y = m_entity->GetLowerBodyYaw() + 280; break;
									case 1108: m_angles->y = m_entity->GetLowerBodyYaw() - 280; break;
									case 1109: m_angles->y = m_entity->GetLowerBodyYaw() + 280.5; break;
									case 1110: m_angles->y = m_entity->GetLowerBodyYaw() - 280.5; break;
									case 1111: m_angles->y = m_entity->GetLowerBodyYaw() + 281; break;
									case 1112: m_angles->y = m_entity->GetLowerBodyYaw() - 281; break;
									case 1113: m_angles->y = m_entity->GetLowerBodyYaw() + 281.5; break;
									case 1114: m_angles->y = m_entity->GetLowerBodyYaw() - 281.5; break;
									case 1115: m_angles->y = m_entity->GetLowerBodyYaw() + 282; break;
									case 1116: m_angles->y = m_entity->GetLowerBodyYaw() - 282; break;
									case 1117: m_angles->y = m_entity->GetLowerBodyYaw() + 282.5; break;
									case 1118: m_angles->y = m_entity->GetLowerBodyYaw() - 283.5; break;
									case 1119: m_angles->y = m_entity->GetLowerBodyYaw() + 283; break;
									case 1120: m_angles->y = m_entity->GetLowerBodyYaw() - 283; break;
									case 1121: m_angles->y = m_entity->GetLowerBodyYaw() + 283.5; break;
									case 1122: m_angles->y = m_entity->GetLowerBodyYaw() - 283.5; break;
									case 1123: m_angles->y = m_entity->GetLowerBodyYaw() + 284; break;
									case 1124: m_angles->y = m_entity->GetLowerBodyYaw() - 284; break;
									case 1125: m_angles->y = m_entity->GetLowerBodyYaw() + 284.5; break;
									case 1126: m_angles->y = m_entity->GetLowerBodyYaw() - 284.5; break;
									case 1127: m_angles->y = m_entity->GetLowerBodyYaw() + 285; break;
									case 1128: m_angles->y = m_entity->GetLowerBodyYaw() - 285; break;
									case 1129: m_angles->y = m_entity->GetLowerBodyYaw() + 285.5; break;
									case 1130: m_angles->y = m_entity->GetLowerBodyYaw() - 285.5; break;
									case 1131: m_angles->y = m_entity->GetLowerBodyYaw() + 286; break;
									case 1132: m_angles->y = m_entity->GetLowerBodyYaw() - 286; break;
									case 1133: m_angles->y = m_entity->GetLowerBodyYaw() + 286.5; break;
									case 1134: m_angles->y = m_entity->GetLowerBodyYaw() - 286.5; break;
									case 1135: m_angles->y = m_entity->GetLowerBodyYaw() + 287; break;
									case 1136: m_angles->y = m_entity->GetLowerBodyYaw() - 287; break;
									case 1137: m_angles->y = m_entity->GetLowerBodyYaw() + 287.5; break;
									case 1138: m_angles->y = m_entity->GetLowerBodyYaw() - 287.5; break;
									case 1139: m_angles->y = m_entity->GetLowerBodyYaw() + 288; break;
									case 1140: m_angles->y = m_entity->GetLowerBodyYaw() - 288; break;
									case 1141: m_angles->y = m_entity->GetLowerBodyYaw() + 288.5; break;
									case 1142: m_angles->y = m_entity->GetLowerBodyYaw() - 288.5; break;
									case 1143: m_angles->y = m_entity->GetLowerBodyYaw() + 289; break;
									case 1144: m_angles->y = m_entity->GetLowerBodyYaw() - 289; break;
									case 1145: m_angles->y = m_entity->GetLowerBodyYaw() + 289.5; break;
									case 1146: m_angles->y = m_entity->GetLowerBodyYaw() - 289.5; break;
									case 1147: m_angles->y = m_entity->GetLowerBodyYaw() + 290; break;
									case 1148: m_angles->y = m_entity->GetLowerBodyYaw() - 290; break;
									case 1149: m_angles->y = m_entity->GetLowerBodyYaw() + 290.5; break;
									case 1150: m_angles->y = m_entity->GetLowerBodyYaw() - 290.5; break;
									case 1151: m_angles->y = m_entity->GetLowerBodyYaw() + 291; break;
									case 1152: m_angles->y = m_entity->GetLowerBodyYaw() - 291; break;
									case 1153: m_angles->y = m_entity->GetLowerBodyYaw() + 291.5; break;
									case 1154: m_angles->y = m_entity->GetLowerBodyYaw() - 291.5; break;
									case 1155: m_angles->y = m_entity->GetLowerBodyYaw() + 292; break;
									case 1156: m_angles->y = m_entity->GetLowerBodyYaw() - 292; break;
									case 1157: m_angles->y = m_entity->GetLowerBodyYaw() + 292.5; break;
									case 1158: m_angles->y = m_entity->GetLowerBodyYaw() - 292.5; break;
									case 1159: m_angles->y = m_entity->GetLowerBodyYaw() + 293; break;
									case 1160: m_angles->y = m_entity->GetLowerBodyYaw() - 293; break;
									case 1161: m_angles->y = m_entity->GetLowerBodyYaw() + 293.5; break;
									case 1162: m_angles->y = m_entity->GetLowerBodyYaw() - 293.5; break;
									case 1163: m_angles->y = m_entity->GetLowerBodyYaw() + 294; break;
									case 1164: m_angles->y = m_entity->GetLowerBodyYaw() - 294; break;
									case 1165: m_angles->y = m_entity->GetLowerBodyYaw() + 294.5; break;
									case 1166: m_angles->y = m_entity->GetLowerBodyYaw() - 294.5; break;
									case 1167: m_angles->y = m_entity->GetLowerBodyYaw() + 295; break;
									case 1168: m_angles->y = m_entity->GetLowerBodyYaw() - 295; break;
									case 1169: m_angles->y = m_entity->GetLowerBodyYaw() + 295.5; break;
									case 1170: m_angles->y = m_entity->GetLowerBodyYaw() - 295.5; break;
									case 1171: m_angles->y = m_entity->GetLowerBodyYaw() + 296; break;
									case 1172: m_angles->y = m_entity->GetLowerBodyYaw() - 296; break;
									case 1173: m_angles->y = m_entity->GetLowerBodyYaw() + 296.5; break;
									case 1174: m_angles->y = m_entity->GetLowerBodyYaw() - 296.5; break;
									case 1175: m_angles->y = m_entity->GetLowerBodyYaw() + 297; break;
									case 1176: m_angles->y = m_entity->GetLowerBodyYaw() - 297; break;
									case 1177: m_angles->y = m_entity->GetLowerBodyYaw() + 297.5; break;
									case 1178: m_angles->y = m_entity->GetLowerBodyYaw() - 297.5; break;
									case 1179: m_angles->y = m_entity->GetLowerBodyYaw() + 298; break;
									case 1180: m_angles->y = m_entity->GetLowerBodyYaw() - 298; break;
									case 1181: m_angles->y = m_entity->GetLowerBodyYaw() + 298.5; break;
									case 1182: m_angles->y = m_entity->GetLowerBodyYaw() - 298.5; break;
									case 1183: m_angles->y = m_entity->GetLowerBodyYaw() + 299; break;
									case 1184: m_angles->y = m_entity->GetLowerBodyYaw() - 299; break;
									case 1185: m_angles->y = m_entity->GetLowerBodyYaw() + 299.5; break;
									case 1186: m_angles->y = m_entity->GetLowerBodyYaw() - 299.5; break;
									case 1187: m_angles->y = m_entity->GetLowerBodyYaw() + 300; break;
									case 1188: m_angles->y = m_entity->GetLowerBodyYaw() - 300; break;
									case 1189: m_angles->y = m_entity->GetLowerBodyYaw() + 300.5; break;
									case 1190: m_angles->y = m_entity->GetLowerBodyYaw() - 300.5; break;
									case 1191: m_angles->y = m_entity->GetLowerBodyYaw() + 301; break;
									case 1192: m_angles->y = m_entity->GetLowerBodyYaw() - 301; break;
									case 1193: m_angles->y = m_entity->GetLowerBodyYaw() + 301.5; break;
									case 1194: m_angles->y = m_entity->GetLowerBodyYaw() - 301.5; break;
									case 1195: m_angles->y = m_entity->GetLowerBodyYaw() + 302; break;
									case 1196: m_angles->y = m_entity->GetLowerBodyYaw() - 302; break;
									case 1197: m_angles->y = m_entity->GetLowerBodyYaw() + 302.5; break;
									case 1198: m_angles->y = m_entity->GetLowerBodyYaw() - 302.5; break;
									case 1199: m_angles->y = m_entity->GetLowerBodyYaw() + 303; break;
									case 1200: m_angles->y = m_entity->GetLowerBodyYaw() - 303; break;
									case 1201: m_angles->y = m_entity->GetLowerBodyYaw() + 303.5; break;
									case 1202: m_angles->y = m_entity->GetLowerBodyYaw() - 303.5; break;
									case 1203: m_angles->y = m_entity->GetLowerBodyYaw() + 304; break;
									case 1204: m_angles->y = m_entity->GetLowerBodyYaw() - 304; break;
									case 1205: m_angles->y = m_entity->GetLowerBodyYaw() + 304.5; break;
									case 1206: m_angles->y = m_entity->GetLowerBodyYaw() - 304.5; break;
									case 1207: m_angles->y = m_entity->GetLowerBodyYaw() + 305; break;
									case 1208: m_angles->y = m_entity->GetLowerBodyYaw() - 305; break;
									case 1209: m_angles->y = m_entity->GetLowerBodyYaw() + 305.5; break;
									case 1210: m_angles->y = m_entity->GetLowerBodyYaw() - 305.5; break;
									case 1211: m_angles->y = m_entity->GetLowerBodyYaw() + 306; break;
									case 1212: m_angles->y = m_entity->GetLowerBodyYaw() - 306; break;
									case 1213: m_angles->y = m_entity->GetLowerBodyYaw() + 306.5; break;
									case 1214: m_angles->y = m_entity->GetLowerBodyYaw() - 306.5; break;
									case 1215: m_angles->y = m_entity->GetLowerBodyYaw() + 307; break;
									case 1216: m_angles->y = m_entity->GetLowerBodyYaw() - 307; break;
									case 1217: m_angles->y = m_entity->GetLowerBodyYaw() + 307.5; break;
									case 1218: m_angles->y = m_entity->GetLowerBodyYaw() - 307.5; break;
									case 1219: m_angles->y = m_entity->GetLowerBodyYaw() + 308; break;
									case 1220: m_angles->y = m_entity->GetLowerBodyYaw() - 308; break;
									case 1221: m_angles->y = m_entity->GetLowerBodyYaw() + 308.5; break;
									case 1222: m_angles->y = m_entity->GetLowerBodyYaw() - 308.5; break;
									case 1223: m_angles->y = m_entity->GetLowerBodyYaw() + 309; break;
									case 1224: m_angles->y = m_entity->GetLowerBodyYaw() - 309; break;
									case 1225: m_angles->y = m_entity->GetLowerBodyYaw() + 309.5; break;
									case 1226: m_angles->y = m_entity->GetLowerBodyYaw() - 309.5; break;
									case 1227: m_angles->y = m_entity->GetLowerBodyYaw() + 310; break;
									case 1228: m_angles->y = m_entity->GetLowerBodyYaw() - 310; break;
									case 1229: m_angles->y = m_entity->GetLowerBodyYaw() + 310.5; break;
									case 1230: m_angles->y = m_entity->GetLowerBodyYaw() - 310.5; break;
									case 1231: m_angles->y = m_entity->GetLowerBodyYaw() + 311; break;
									case 1232: m_angles->y = m_entity->GetLowerBodyYaw() - 311; break;
									case 1233: m_angles->y = m_entity->GetLowerBodyYaw() + 311.5; break;
									case 1234: m_angles->y = m_entity->GetLowerBodyYaw() - 311.5; break;
									case 1235: m_angles->y = m_entity->GetLowerBodyYaw() + 312; break;
									case 1236: m_angles->y = m_entity->GetLowerBodyYaw() - 312; break;
									case 1237: m_angles->y = m_entity->GetLowerBodyYaw() + 312.5; break;
									case 1238: m_angles->y = m_entity->GetLowerBodyYaw() - 312.5; break;
									case 1239: m_angles->y = m_entity->GetLowerBodyYaw() + 313; break;
									case 1240: m_angles->y = m_entity->GetLowerBodyYaw() - 313; break;
									case 1241: m_angles->y = m_entity->GetLowerBodyYaw() + 313.5; break;
									case 1242: m_angles->y = m_entity->GetLowerBodyYaw() - 313.5; break;
									case 1243: m_angles->y = m_entity->GetLowerBodyYaw() + 314; break;
									case 1244: m_angles->y = m_entity->GetLowerBodyYaw() - 314; break;
									case 1245: m_angles->y = m_entity->GetLowerBodyYaw() + 314.5; break;
									case 1246: m_angles->y = m_entity->GetLowerBodyYaw() - 314.5; break;
									case 1247: m_angles->y = m_entity->GetLowerBodyYaw() + 315; break;
									case 1248: m_angles->y = m_entity->GetLowerBodyYaw() - 315; break;
									case 1249: m_angles->y = m_entity->GetLowerBodyYaw() + 315.5; break;
									case 1250: m_angles->y = m_entity->GetLowerBodyYaw() - 315.5; break;
									case 1251: m_angles->y = m_entity->GetLowerBodyYaw() + 316; break;
									case 1252: m_angles->y = m_entity->GetLowerBodyYaw() - 316; break;
									case 1253: m_angles->y = m_entity->GetLowerBodyYaw() + 316.5; break;
									case 1254: m_angles->y = m_entity->GetLowerBodyYaw() - 316.5; break;
									case 1255: m_angles->y = m_entity->GetLowerBodyYaw() + 317; break;
									case 1256: m_angles->y = m_entity->GetLowerBodyYaw() - 317; break;
									case 1257: m_angles->y = m_entity->GetLowerBodyYaw() + 317.5; break;
									case 1258: m_angles->y = m_entity->GetLowerBodyYaw() - 317.5; break;
									case 1259: m_angles->y = m_entity->GetLowerBodyYaw() + 318; break;
									case 1260: m_angles->y = m_entity->GetLowerBodyYaw() - 318; break;
									case 1261: m_angles->y = m_entity->GetLowerBodyYaw() + 318.5; break;
									case 1262: m_angles->y = m_entity->GetLowerBodyYaw() - 318.5; break;
									case 1263: m_angles->y = m_entity->GetLowerBodyYaw() + 319; break;
									case 1264: m_angles->y = m_entity->GetLowerBodyYaw() - 319; break;
									case 1265: m_angles->y = m_entity->GetLowerBodyYaw() + 319.5; break;
									case 1266: m_angles->y = m_entity->GetLowerBodyYaw() - 319.5; break;
									case 1267: m_angles->y = m_entity->GetLowerBodyYaw() + 320; break;
									case 1268: m_angles->y = m_entity->GetLowerBodyYaw() - 320; break;
									case 1269: m_angles->y = m_entity->GetLowerBodyYaw() + 320.5; break;
									case 1270: m_angles->y = m_entity->GetLowerBodyYaw() - 320.5; break;
									case 1271: m_angles->y = m_entity->GetLowerBodyYaw() + 321; break;
									case 1272: m_angles->y = m_entity->GetLowerBodyYaw() - 321; break;
									case 1273: m_angles->y = m_entity->GetLowerBodyYaw() + 321.5; break;
									case 1274: m_angles->y = m_entity->GetLowerBodyYaw() - 321.5; break;
									case 1275: m_angles->y = m_entity->GetLowerBodyYaw() + 322; break;
									case 1276: m_angles->y = m_entity->GetLowerBodyYaw() - 322; break;
									case 1277: m_angles->y = m_entity->GetLowerBodyYaw() + 322.5; break;
									case 1278: m_angles->y = m_entity->GetLowerBodyYaw() - 322.5; break;
									case 1279: m_angles->y = m_entity->GetLowerBodyYaw() + 323; break;
									case 1280: m_angles->y = m_entity->GetLowerBodyYaw() - 323; break;
									case 1281: m_angles->y = m_entity->GetLowerBodyYaw() + 323.5; break;
									case 1282: m_angles->y = m_entity->GetLowerBodyYaw() - 323.5; break;
									case 1283: m_angles->y = m_entity->GetLowerBodyYaw() + 324; break;
									case 1284: m_angles->y = m_entity->GetLowerBodyYaw() - 324; break;
									case 1285: m_angles->y = m_entity->GetLowerBodyYaw() + 324.5; break;
									case 1286: m_angles->y = m_entity->GetLowerBodyYaw() - 324.5; break;
									case 1287: m_angles->y = m_entity->GetLowerBodyYaw() + 325; break;
									case 1288: m_angles->y = m_entity->GetLowerBodyYaw() - 325; break;
									case 1289: m_angles->y = m_entity->GetLowerBodyYaw() + 325.5; break;
									case 1290: m_angles->y = m_entity->GetLowerBodyYaw() - 325.5; break;
									case 1291: m_angles->y = m_entity->GetLowerBodyYaw() + 326; break;
									case 1292: m_angles->y = m_entity->GetLowerBodyYaw() - 326; break;
									case 1293: m_angles->y = m_entity->GetLowerBodyYaw() + 326.5; break;
									case 1294: m_angles->y = m_entity->GetLowerBodyYaw() - 326.5; break;
									case 1295: m_angles->y = m_entity->GetLowerBodyYaw() + 327; break;
									case 1296: m_angles->y = m_entity->GetLowerBodyYaw() - 327; break;
									case 1297: m_angles->y = m_entity->GetLowerBodyYaw() + 327.5; break;
									case 1298: m_angles->y = m_entity->GetLowerBodyYaw() - 327.5; break;
									case 1299: m_angles->y = m_entity->GetLowerBodyYaw() + 328; break;
									case 1300: m_angles->y = m_entity->GetLowerBodyYaw() - 328; break;
									case 1301: m_angles->y = m_entity->GetLowerBodyYaw() + 328.5; break;
									case 1302: m_angles->y = m_entity->GetLowerBodyYaw() - 328.5; break;
									case 1303: m_angles->y = m_entity->GetLowerBodyYaw() + 329; break;
									case 1304: m_angles->y = m_entity->GetLowerBodyYaw() - 329; break;
									case 1305: m_angles->y = m_entity->GetLowerBodyYaw() + 329.5; break;
									case 1306: m_angles->y = m_entity->GetLowerBodyYaw() - 329.5; break;
									case 1307: m_angles->y = m_entity->GetLowerBodyYaw() + 330; break;
									case 1308: m_angles->y = m_entity->GetLowerBodyYaw() - 330; break;
									case 1309: m_angles->y = m_entity->GetLowerBodyYaw() + 330.5; break;
									case 1310: m_angles->y = m_entity->GetLowerBodyYaw() - 330.5; break;
									case 1311: m_angles->y = m_entity->GetLowerBodyYaw() + 331; break;
									case 1312: m_angles->y = m_entity->GetLowerBodyYaw() - 331; break;
									case 1313: m_angles->y = m_entity->GetLowerBodyYaw() + 331.5; break;
									case 1314: m_angles->y = m_entity->GetLowerBodyYaw() - 331.5; break;
									case 1315: m_angles->y = m_entity->GetLowerBodyYaw() + 332; break;
									case 1316: m_angles->y = m_entity->GetLowerBodyYaw() - 332; break;
									case 1317: m_angles->y = m_entity->GetLowerBodyYaw() + 332.5; break;
									case 1318: m_angles->y = m_entity->GetLowerBodyYaw() - 332.5; break;
									case 1319: m_angles->y = m_entity->GetLowerBodyYaw() + 333; break;
									case 1320: m_angles->y = m_entity->GetLowerBodyYaw() - 333; break;
									case 1321: m_angles->y = m_entity->GetLowerBodyYaw() + 333.5; break;
									case 1322: m_angles->y = m_entity->GetLowerBodyYaw() - 333.5; break;
									case 1323: m_angles->y = m_entity->GetLowerBodyYaw() + 334; break;
									case 1324: m_angles->y = m_entity->GetLowerBodyYaw() - 334; break;
									case 1325: m_angles->y = m_entity->GetLowerBodyYaw() + 334.5; break;
									case 1326: m_angles->y = m_entity->GetLowerBodyYaw() - 334.5; break;
									case 1327: m_angles->y = m_entity->GetLowerBodyYaw() + 335; break;
									case 1328: m_angles->y = m_entity->GetLowerBodyYaw() - 335; break;
									case 1329: m_angles->y = m_entity->GetLowerBodyYaw() + 335.5; break;
									case 1330: m_angles->y = m_entity->GetLowerBodyYaw() - 335.5; break;
									case 1331: m_angles->y = m_entity->GetLowerBodyYaw() + 336; break;
									case 1332: m_angles->y = m_entity->GetLowerBodyYaw() - 336; break;
									case 1333: m_angles->y = m_entity->GetLowerBodyYaw() + 336.5; break;
									case 1334: m_angles->y = m_entity->GetLowerBodyYaw() - 336.5; break;
									case 1335: m_angles->y = m_entity->GetLowerBodyYaw() + 337; break;
									case 1336: m_angles->y = m_entity->GetLowerBodyYaw() - 337; break;
									case 1337: m_angles->y = m_entity->GetLowerBodyYaw() + 337.5; break;
									case 1338: m_angles->y = m_entity->GetLowerBodyYaw() - 337.5; break;
									case 1339: m_angles->y = m_entity->GetLowerBodyYaw() + 338; break;
									case 1340: m_angles->y = m_entity->GetLowerBodyYaw() - 338; break;
									case 1341: m_angles->y = m_entity->GetLowerBodyYaw() + 338.5; break;
									case 1342: m_angles->y = m_entity->GetLowerBodyYaw() - 338.5; break;
									case 1343: m_angles->y = m_entity->GetLowerBodyYaw() + 339; break;
									case 1344: m_angles->y = m_entity->GetLowerBodyYaw() - 339; break;
									case 1345: m_angles->y = m_entity->GetLowerBodyYaw() + 339.5; break;
									case 1346: m_angles->y = m_entity->GetLowerBodyYaw() - 339.5; break;
									case 1347: m_angles->y = m_entity->GetLowerBodyYaw() + 340; break;
									case 1348: m_angles->y = m_entity->GetLowerBodyYaw() - 340; break;
									case 1349: m_angles->y = m_entity->GetLowerBodyYaw() + 340.5; break;
									case 1350: m_angles->y = m_entity->GetLowerBodyYaw() - 340.5; break;
									case 1351: m_angles->y = m_entity->GetLowerBodyYaw() + 341; break;
									case 1352: m_angles->y = m_entity->GetLowerBodyYaw() - 341; break;
									case 1353: m_angles->y = m_entity->GetLowerBodyYaw() + 341.5; break;
									case 1354: m_angles->y = m_entity->GetLowerBodyYaw() - 341.5; break;
									case 1355: m_angles->y = m_entity->GetLowerBodyYaw() + 342; break;
									case 1356: m_angles->y = m_entity->GetLowerBodyYaw() - 342; break;
									case 1357: m_angles->y = m_entity->GetLowerBodyYaw() + 342.5; break;
									case 1358: m_angles->y = m_entity->GetLowerBodyYaw() - 342.5; break;
									case 1359: m_angles->y = m_entity->GetLowerBodyYaw() + 343; break;
									case 1360: m_angles->y = m_entity->GetLowerBodyYaw() - 343; break;
									case 1361: m_angles->y = m_entity->GetLowerBodyYaw() + 343.5; break;
									case 1362: m_angles->y = m_entity->GetLowerBodyYaw() - 343.5; break;
									case 1363: m_angles->y = m_entity->GetLowerBodyYaw() + 344; break;
									case 1364: m_angles->y = m_entity->GetLowerBodyYaw() - 344; break;
									case 1365: m_angles->y = m_entity->GetLowerBodyYaw() + 344.5; break;
									case 1366: m_angles->y = m_entity->GetLowerBodyYaw() - 344.5; break;
									case 1367: m_angles->y = m_entity->GetLowerBodyYaw() + 345; break;
									case 1368: m_angles->y = m_entity->GetLowerBodyYaw() - 345; break;
									case 1369: m_angles->y = m_entity->GetLowerBodyYaw() + 345.5; break;
									case 1370: m_angles->y = m_entity->GetLowerBodyYaw() - 345.5; break;
									case 1371: m_angles->y = m_entity->GetLowerBodyYaw() + 346; break;
									case 1372: m_angles->y = m_entity->GetLowerBodyYaw() - 346; break;
									case 1373: m_angles->y = m_entity->GetLowerBodyYaw() + 346.5; break;
									case 1374: m_angles->y = m_entity->GetLowerBodyYaw() - 346.5; break;
									case 1375: m_angles->y = m_entity->GetLowerBodyYaw() + 347; break;
									case 1376: m_angles->y = m_entity->GetLowerBodyYaw() - 347; break;
									case 1377: m_angles->y = m_entity->GetLowerBodyYaw() + 347.5; break;
									case 1378: m_angles->y = m_entity->GetLowerBodyYaw() - 347.5; break;
									case 1379: m_angles->y = m_entity->GetLowerBodyYaw() + 348; break;
									case 1380: m_angles->y = m_entity->GetLowerBodyYaw() - 348; break;
									case 1381: m_angles->y = m_entity->GetLowerBodyYaw() + 348.5; break;
									case 1382: m_angles->y = m_entity->GetLowerBodyYaw() - 348.5; break;
									case 1383: m_angles->y = m_entity->GetLowerBodyYaw() + 349; break;
									case 1384: m_angles->y = m_entity->GetLowerBodyYaw() - 349; break;
									case 1385: m_angles->y = m_entity->GetLowerBodyYaw() + 349.5; break;
									case 1386: m_angles->y = m_entity->GetLowerBodyYaw() - 349.5; break;
									case 1387: m_angles->y = m_entity->GetLowerBodyYaw() + 350; break;
									case 1388: m_angles->y = m_entity->GetLowerBodyYaw() - 350; break;
									case 1389: m_angles->y = m_entity->GetLowerBodyYaw() + 350.5; break;
									case 1390: m_angles->y = m_entity->GetLowerBodyYaw() - 350.5; break;
									case 1391: m_angles->y = m_entity->GetLowerBodyYaw() + 351; break;
									case 1392: m_angles->y = m_entity->GetLowerBodyYaw() - 351; break;
									case 1393: m_angles->y = m_entity->GetLowerBodyYaw() + 351.5; break;
									case 1394: m_angles->y = m_entity->GetLowerBodyYaw() - 351.5; break;
									case 1395: m_angles->y = m_entity->GetLowerBodyYaw() + 352; break;
									case 1396: m_angles->y = m_entity->GetLowerBodyYaw() - 352; break;
									case 1397: m_angles->y = m_entity->GetLowerBodyYaw() + 352.5; break;
									case 1398: m_angles->y = m_entity->GetLowerBodyYaw() - 352.5; break;
									case 1399: m_angles->y = m_entity->GetLowerBodyYaw() + 353; break;
									case 1400: m_angles->y = m_entity->GetLowerBodyYaw() - 353; break;
									case 1401: m_angles->y = m_entity->GetLowerBodyYaw() + 353.5; break;
									case 1402: m_angles->y = m_entity->GetLowerBodyYaw() - 353.5; break;
									case 1403: m_angles->y = m_entity->GetLowerBodyYaw() + 354; break;
									case 1404: m_angles->y = m_entity->GetLowerBodyYaw() - 354; break;
									case 1405: m_angles->y = m_entity->GetLowerBodyYaw() + 354.5; break;
									case 1406: m_angles->y = m_entity->GetLowerBodyYaw() - 354.5; break;
									case 1407: m_angles->y = m_entity->GetLowerBodyYaw() + 355; break;
									case 1408: m_angles->y = m_entity->GetLowerBodyYaw() - 355; break;
									case 1409: m_angles->y = m_entity->GetLowerBodyYaw() + 355.5; break;
									case 1410: m_angles->y = m_entity->GetLowerBodyYaw() - 355.5; break;
									case 1411: m_angles->y = m_entity->GetLowerBodyYaw() + 356; break;
									case 1412: m_angles->y = m_entity->GetLowerBodyYaw() - 356; break;
									case 1413: m_angles->y = m_entity->GetLowerBodyYaw() + 356.5; break;
									case 1414: m_angles->y = m_entity->GetLowerBodyYaw() - 356.5; break;
									case 1415: m_angles->y = m_entity->GetLowerBodyYaw() + 357; break;
									case 1416: m_angles->y = m_entity->GetLowerBodyYaw() - 357; break;
									case 1417: m_angles->y = m_entity->GetLowerBodyYaw() + 357.5; break;
									case 1418: m_angles->y = m_entity->GetLowerBodyYaw() - 357.5; break;
									case 1419: m_angles->y = m_entity->GetLowerBodyYaw() + 358; break;
									case 1420: m_angles->y = m_entity->GetLowerBodyYaw() - 358; break;
									case 1421: m_angles->y = m_entity->GetLowerBodyYaw() + 358.5; break;
									case 1422: m_angles->y = m_entity->GetLowerBodyYaw() - 358.5; break;
									case 1423: m_angles->y = m_entity->GetLowerBodyYaw() + 359; break;
									case 1424: m_angles->y = m_entity->GetLowerBodyYaw() - 359; break;
									case 1425: m_angles->y = m_entity->GetLowerBodyYaw() + 359.5; break;
									case 1426: m_angles->y = m_entity->GetLowerBodyYaw() - 359.5; break;
									case 1427: m_angles->y = m_entity->GetLowerBodyYaw() + 360; break;
									case 1428: m_angles->y = m_entity->GetLowerBodyYaw() - 360; break;
									case 1429: m_angles->y = m_entity->GetLowerBodyYaw() - 268; break;
									case 1430: m_angles->y = m_entity->GetLowerBodyYaw() + 212.5; break;
									case 1431: m_angles->y = m_entity->GetLowerBodyYaw() - 45; break;
									}
								}
							}
							else {
								m_angles->y = m_entity->GetLowerBodyYaw();
							}
						}
						m_angles->y = m_player->resolver_data.resolved_yaw;
					}
				}
				else if (resolvermode_y == 0) m_angles->y = m_entity->GetEyeAngles()->y;
				else if (resolvermode_y == 1) m_angles->y = at_target_angle.y - 0;
				else if (resolvermode_y == 2) m_angles->y = at_target_angle.y + 0;
				else if (resolvermode_y == 3) m_angles->y = at_target_angle.y + 10;
				else if (resolvermode_y == 4) m_angles->y = at_target_angle.y - 10;
				else if (resolvermode_y == 5) m_angles->y = at_target_angle.y + 20;
				else if (resolvermode_y == 6) m_angles->y = at_target_angle.y - 20;
				else if (resolvermode_y == 7) m_angles->y = at_target_angle.y + 30;
				else if (resolvermode_y == 8) m_angles->y = at_target_angle.y - 30;
				else if (resolvermode_y == 9) m_angles->y = at_target_angle.y + 40;
				else if (resolvermode_y == 10) m_angles->y = at_target_angle.y - 40;
				else if (resolvermode_y == 11) m_angles->y = at_target_angle.y + 50;
				else if (resolvermode_y == 12) m_angles->y = at_target_angle.y - 50;
				else if (resolvermode_y == 13) m_angles->y = at_target_angle.y + 60;
				else if (resolvermode_y == 14) m_angles->y = at_target_angle.y - 60;
				else if (resolvermode_y == 15) m_angles->y = at_target_angle.y + 70;
				else if (resolvermode_y == 16) m_angles->y = at_target_angle.y - 70;
				else if (resolvermode_y == 17) m_angles->y = at_target_angle.y + 80;
				else if (resolvermode_y == 18) m_angles->y = at_target_angle.y - 80;
				else if (resolvermode_y == 19) m_angles->y = at_target_angle.y + 90;
				else if (resolvermode_y == 20) m_angles->y = at_target_angle.y - 90;
				else if (resolvermode_y == 21) m_angles->y = at_target_angle.y + 100;
				else if (resolvermode_y == 22) m_angles->y = at_target_angle.y - 100;
				else if (resolvermode_y == 23) m_angles->y = at_target_angle.y + 110;
				else if (resolvermode_y == 24) m_angles->y = at_target_angle.y - 110;
				else if (resolvermode_y == 25) m_angles->y = at_target_angle.y + 120;
				else if (resolvermode_y == 26) m_angles->y = at_target_angle.y - 120;
				else if (resolvermode_y == 27) m_angles->y = at_target_angle.y + 130;
				else if (resolvermode_y == 28) m_angles->y = at_target_angle.y - 130;
				else if (resolvermode_y == 29) m_angles->y = at_target_angle.y + 140;
				else if (resolvermode_y == 30) m_angles->y = at_target_angle.y - 140;
				else if (resolvermode_y == 31) m_angles->y = at_target_angle.y + 150;
				else if (resolvermode_y == 32) m_angles->y = at_target_angle.y - 150;
				else if (resolvermode_y == 33) m_angles->y = at_target_angle.y + 160;
				else if (resolvermode_y == 34) m_angles->y = at_target_angle.y - 160;
				else if (resolvermode_y == 35) m_angles->y = at_target_angle.y + 170;
				else if (resolvermode_y == 36) m_angles->y = at_target_angle.y - 170;
				else if (resolvermode_y == 37) m_angles->y = at_target_angle.y + 180;
				else if (resolvermode_y == 38) m_angles->y = at_target_angle.y - 180;
				else if (resolvermode_y == 39) m_angles->y = at_target_angle.y + 190;
				else if (resolvermode_y == 40) m_angles->y = at_target_angle.y - 190;
				else if (resolvermode_y == 41) m_angles->y = at_target_angle.y + 200;
				else if (resolvermode_y == 42) m_angles->y = at_target_angle.y - 200;
				else if (resolvermode_y == 43) m_angles->y = at_target_angle.y + 210;
				else if (resolvermode_y == 44) m_angles->y = at_target_angle.y - 210;
				else if (resolvermode_y == 45) m_angles->y = at_target_angle.y + 220;
				else if (resolvermode_y == 46) m_angles->y = at_target_angle.y - 220;
				else if (resolvermode_y == 47) m_angles->y = at_target_angle.y + 230;
				else if (resolvermode_y == 48) m_angles->y = at_target_angle.y - 230;
				else if (resolvermode_y == 49) m_angles->y = at_target_angle.y + 240;
				else if (resolvermode_y == 50) m_angles->y = at_target_angle.y - 240;
				else if (resolvermode_y == 51) m_angles->y = at_target_angle.y + 250;
				else if (resolvermode_y == 52) m_angles->y = at_target_angle.y - 250;
				else if (resolvermode_y == 53) m_angles->y = at_target_angle.y + 260;
				else if (resolvermode_y == 54) m_angles->y = at_target_angle.y - 260;
				else if (resolvermode_y == 55) m_angles->y = at_target_angle.y + 270;
				else if (resolvermode_y == 56) m_angles->y = at_target_angle.y - 270;
				else if (resolvermode_y == 57) m_angles->y = at_target_angle.y + 280;
				else if (resolvermode_y == 58) m_angles->y = at_target_angle.y - 280;
				else if (resolvermode_y == 59) m_angles->y = at_target_angle.y + 290;
				else if (resolvermode_y == 60) m_angles->y = at_target_angle.y - 290;
				else if (resolvermode_y == 61) m_angles->y = at_target_angle.y + 300;
				else if (resolvermode_y == 62) m_angles->y = at_target_angle.y - 300;
				else if (resolvermode_y == 63) m_angles->y = at_target_angle.y + 310;
				else if (resolvermode_y == 64) m_angles->y = at_target_angle.y - 310;
				else if (resolvermode_y == 65) m_angles->y = at_target_angle.y + 320;
				else if (resolvermode_y == 66) m_angles->y = at_target_angle.y - 320;
				else if (resolvermode_y == 67) m_angles->y = at_target_angle.y + 330;
				else if (resolvermode_y == 68) m_angles->y = at_target_angle.y - 330;
				else if (resolvermode_y == 69) m_angles->y = at_target_angle.y + 340;
				else if (resolvermode_y == 70) m_angles->y = at_target_angle.y - 340;
				else if (resolvermode_y == 71) m_angles->y = at_target_angle.y + 350;
				else if (resolvermode_y == 72) m_angles->y = at_target_angle.y - 350;
				else if (resolvermode_y == 73) m_angles->y = at_target_angle.y + 360;
				else if (resolvermode_y == 74) m_angles->y = at_target_angle.y - 360;

			}

			if (m_player->resolver_data.has_hit_angle) m_angles->y = m_player->resolver_data.last_hit_angle.y;

			if (resolverconfig.bResolverOverride && GetAsyncKeyState(resolverconfig.iResolverOverrideKey)) {
				Vector pos_enemy;
				Vector local_target_angle;
				if (game::functions.world_to_screen(m_entity->GetOrigin(), pos_enemy)) {
					game::math.calculate_angle(m_local->GetOrigin(), m_entity->GetOrigin(), local_target_angle);

					POINT mouse = GUI.GetMouse();
					float delta = mouse.x - pos_enemy.x;

					if (delta <  1) m_angles->y = local_target_angle.y + 0.5;
					else m_angles->y = local_target_angle.y - 0.5;
					if (delta <  3) m_angles->y = local_target_angle.y + 1;
					else m_angles->y = local_target_angle.y - 1;
					if (delta <  5) m_angles->y = local_target_angle.y + 1.5;
					else m_angles->y = local_target_angle.y - 1.5;
					if (delta <  7) m_angles->y = local_target_angle.y + 2;
					else m_angles->y = local_target_angle.y - 2;
					if (delta <  9) m_angles->y = local_target_angle.y + 2.5;
					else m_angles->y = local_target_angle.y - 2.5;
					if (delta <  11) m_angles->y = local_target_angle.y + 3;
					else m_angles->y = local_target_angle.y - 3;
					if (delta <  13) m_angles->y = local_target_angle.y + 3.5;
					else m_angles->y = local_target_angle.y - 3.5;
					if (delta <  15) m_angles->y = local_target_angle.y + 4;
					else m_angles->y = local_target_angle.y - 4;
					if (delta <  17) m_angles->y = local_target_angle.y + 4.5;
					else m_angles->y = local_target_angle.y - 4.5;
					if (delta <  19) m_angles->y = local_target_angle.y + 5;
					else m_angles->y = local_target_angle.y - 5;
					if (delta <  21) m_angles->y = local_target_angle.y + 5.5;
					else m_angles->y = local_target_angle.y - 5.5;
					if (delta <  23) m_angles->y = local_target_angle.y + 6;
					else m_angles->y = local_target_angle.y - 6;
					if (delta <  25) m_angles->y = local_target_angle.y + 6.5;
					else m_angles->y = local_target_angle.y - 6.5;
					if (delta <  27) m_angles->y = local_target_angle.y + 7;
					else m_angles->y = local_target_angle.y - 7;
					if (delta <  29) m_angles->y = local_target_angle.y + 7.5;
					else m_angles->y = local_target_angle.y - 7.5;
					if (delta <  31) m_angles->y = local_target_angle.y + 8;
					else m_angles->y = local_target_angle.y - 8;
					if (delta <  33) m_angles->y = local_target_angle.y + 9.5;
					else m_angles->y = local_target_angle.y - 9.5;
					if (delta <  35) m_angles->y = local_target_angle.y + 10;
					else m_angles->y = local_target_angle.y - 10;
					if (delta <  37) m_angles->y = local_target_angle.y + 10.5;
					else m_angles->y = local_target_angle.y - 10.5;
					if (delta <  39) m_angles->y = local_target_angle.y + 11;
					else m_angles->y = local_target_angle.y - 11;
					if (delta <  41) m_angles->y = local_target_angle.y + 11.5;
					else m_angles->y = local_target_angle.y - 11.5;
					if (delta <  43) m_angles->y = local_target_angle.y + 12;
					else m_angles->y = local_target_angle.y - 12;
					if (delta <  45) m_angles->y = local_target_angle.y + 12.5;
					else m_angles->y = local_target_angle.y - 12.5;
					if (delta <  47) m_angles->y = local_target_angle.y + 13;
					else m_angles->y = local_target_angle.y - 13;
					if (delta <  49) m_angles->y = local_target_angle.y + 13.5;
					else m_angles->y = local_target_angle.y - 13.5;
					if (delta <  51) m_angles->y = local_target_angle.y + 14;
					else m_angles->y = local_target_angle.y - 14;
					if (delta <  53) m_angles->y = local_target_angle.y + 14.5;
					else m_angles->y = local_target_angle.y - 14.5;
					if (delta <  55) m_angles->y = local_target_angle.y + 15;
					else m_angles->y = local_target_angle.y - 15;
					if (delta <  57) m_angles->y = local_target_angle.y + 15.5;
					else m_angles->y = local_target_angle.y - 15.5;
					if (delta <  59) m_angles->y = local_target_angle.y + 16;
					else m_angles->y = local_target_angle.y - 16;
					if (delta <  61) m_angles->y = local_target_angle.y + 16.5;
					else m_angles->y = local_target_angle.y - 16.5;
					if (delta <  63) m_angles->y = local_target_angle.y + 17;
					else m_angles->y = local_target_angle.y - 17;
					if (delta <  65) m_angles->y = local_target_angle.y + 17.5;
					else m_angles->y = local_target_angle.y - 17.5;
					if (delta <  67) m_angles->y = local_target_angle.y + 18;
					else m_angles->y = local_target_angle.y - 18;
					if (delta <  69) m_angles->y = local_target_angle.y + 19.5;
					else m_angles->y = local_target_angle.y - 19.5;
					if (delta <  71) m_angles->y = local_target_angle.y + 20;
					else m_angles->y = local_target_angle.y - 20;
					if (delta <  73) m_angles->y = local_target_angle.y + 20.5;
					else m_angles->y = local_target_angle.y - 20.5;
					if (delta <  75) m_angles->y = local_target_angle.y + 21;
					else m_angles->y = local_target_angle.y - 21;
					if (delta <  77) m_angles->y = local_target_angle.y + 22.5;
					else m_angles->y = local_target_angle.y - 22.5;
					if (delta <  79) m_angles->y = local_target_angle.y + 23;
					else m_angles->y = local_target_angle.y - 23;
					if (delta <  81) m_angles->y = local_target_angle.y + 23.5;
					else m_angles->y = local_target_angle.y - 23.5;
					if (delta <  83) m_angles->y = local_target_angle.y + 24;
					else m_angles->y = local_target_angle.y - 24;
					if (delta <  85) m_angles->y = local_target_angle.y + 24.5;
					else m_angles->y = local_target_angle.y - 24.5;
					if (delta <  87) m_angles->y = local_target_angle.y + 25;
					else m_angles->y = local_target_angle.y - 25;
					if (delta <  89) m_angles->y = local_target_angle.y + 25.5;
					else m_angles->y = local_target_angle.y - 25.5;
					if (delta <  91) m_angles->y = local_target_angle.y + 26;
					else m_angles->y = local_target_angle.y - 26;
					if (delta <  93) m_angles->y = local_target_angle.y + 26.5;
					else m_angles->y = local_target_angle.y - 26.5;
					if (delta <  95) m_angles->y = local_target_angle.y + 27;
					else m_angles->y = local_target_angle.y - 27;
					if (delta <  97) m_angles->y = local_target_angle.y + 27.5;
					else m_angles->y = local_target_angle.y - 27.5;
					if (delta <  99) m_angles->y = local_target_angle.y + 28;
					else m_angles->y = local_target_angle.y - 28;
					if (delta <  101) m_angles->y = local_target_angle.y + 28.5;
					else m_angles->y = local_target_angle.y - 28.5;
					if (delta <  103) m_angles->y = local_target_angle.y + 29;
					else m_angles->y = local_target_angle.y - 29;
					if (delta <  105) m_angles->y = local_target_angle.y + 29.5;
					else m_angles->y = local_target_angle.y - 29.5;
					if (delta <  107) m_angles->y = local_target_angle.y + 30;
					else m_angles->y = local_target_angle.y - 30;
					if (delta <  109) m_angles->y = local_target_angle.y + 30.5;
					else m_angles->y = local_target_angle.y - 30.5;
					if (delta <  111) m_angles->y = local_target_angle.y + 31;
					else m_angles->y = local_target_angle.y - 31;
					if (delta <  113) m_angles->y = local_target_angle.y + 31.5;
					else m_angles->y = local_target_angle.y - 31.5;
					if (delta <  115) m_angles->y = local_target_angle.y + 32;
					else m_angles->y = local_target_angle.y - 32;
					if (delta <  117) m_angles->y = local_target_angle.y + 32.5;
					else m_angles->y = local_target_angle.y - 32.5;
					if (delta <  119) m_angles->y = local_target_angle.y + 33;
					else m_angles->y = local_target_angle.y - 33;
					if (delta <  121) m_angles->y = local_target_angle.y + 33.5;
					else m_angles->y = local_target_angle.y - 33.5;
					if (delta <  123) m_angles->y = local_target_angle.y + 34;
					else m_angles->y = local_target_angle.y - 34;
					if (delta <  125) m_angles->y = local_target_angle.y + 34.5;
					else m_angles->y = local_target_angle.y - 34.5;
					if (delta <  127) m_angles->y = local_target_angle.y + 35;
					else m_angles->y = local_target_angle.y - 35;
					if (delta <  129) m_angles->y = local_target_angle.y + 35.5;
					else m_angles->y = local_target_angle.y - 35.5;
					if (delta <  131) m_angles->y = local_target_angle.y + 36;
					else m_angles->y = local_target_angle.y - 36;
					if (delta <  133) m_angles->y = local_target_angle.y + 36.5;
					else m_angles->y = local_target_angle.y - 36.5;
					if (delta <  135) m_angles->y = local_target_angle.y + 37;
					else m_angles->y = local_target_angle.y - 37;
					if (delta <  137) m_angles->y = local_target_angle.y + 37.5;
					else m_angles->y = local_target_angle.y - 37.5;
					if (delta <  139) m_angles->y = local_target_angle.y + 38;
					else m_angles->y = local_target_angle.y - 38;
					if (delta <  141) m_angles->y = local_target_angle.y + 39.5;
					else m_angles->y = local_target_angle.y - 39.5;
					if (delta <  143) m_angles->y = local_target_angle.y + 40;
					else m_angles->y = local_target_angle.y - 40;
					if (delta <  145) m_angles->y = local_target_angle.y + 40.5;
					else m_angles->y = local_target_angle.y - 40.5;
					if (delta <  147) m_angles->y = local_target_angle.y + 41;
					else m_angles->y = local_target_angle.y - 41;
					if (delta <  149) m_angles->y = local_target_angle.y + 41.5;
					else m_angles->y = local_target_angle.y - 41.5;
					if (delta <  151) m_angles->y = local_target_angle.y + 42;
					else m_angles->y = local_target_angle.y - 42;
					if (delta <  153) m_angles->y = local_target_angle.y + 42.5;
					else m_angles->y = local_target_angle.y - 42.5;
					if (delta <  155) m_angles->y = local_target_angle.y + 43;
					else m_angles->y = local_target_angle.y - 43;
					if (delta <  157) m_angles->y = local_target_angle.y + 43.5;
					else m_angles->y = local_target_angle.y - 43.5;
					if (delta <  159) m_angles->y = local_target_angle.y + 44;
					else m_angles->y = local_target_angle.y - 44;
					if (delta <  161) m_angles->y = local_target_angle.y + 44.5;
					else m_angles->y = local_target_angle.y - 44.5;
					//else m_angles->y = local_target_angle.y - 45;
					if (delta <  164) m_angles->y = local_target_angle.y + 45.5;
					else m_angles->y = local_target_angle.y - 45.5;
					if (delta <  166) m_angles->y = local_target_angle.y + 46;
					else m_angles->y = local_target_angle.y - 46;
					if (delta <  168) m_angles->y = local_target_angle.y + 46.5;
					else m_angles->y = local_target_angle.y - 46.5;
					if (delta <  170) m_angles->y = local_target_angle.y + 47;
					else m_angles->y = local_target_angle.y - 47;
					if (delta <  172) m_angles->y = local_target_angle.y + 47.5;
					else m_angles->y = local_target_angle.y - 47.5;
					if (delta <  174) m_angles->y = local_target_angle.y + 48;
					else m_angles->y = local_target_angle.y - 48;
					if (delta <  176) m_angles->y = local_target_angle.y + 48.5;
					else m_angles->y = local_target_angle.y - 48.5;
					if (delta <  178) m_angles->y = local_target_angle.y + 49;
					else m_angles->y = local_target_angle.y - 49;
					if (delta <  180) m_angles->y = local_target_angle.y + 49.5;
					else m_angles->y = local_target_angle.y - 49.5;
					if (delta <  182) m_angles->y = local_target_angle.y + 50;
					else m_angles->y = local_target_angle.y - 50;
					if (delta <  184) m_angles->y = local_target_angle.y + 50.5;
					else m_angles->y = local_target_angle.y - 50.5;
					if (delta <  186) m_angles->y = local_target_angle.y + 51;
					else m_angles->y = local_target_angle.y - 51;
					if (delta <  188) m_angles->y = local_target_angle.y + 51.5;
					else m_angles->y = local_target_angle.y - 51.5;
					if (delta <  190) m_angles->y = local_target_angle.y + 52;
					else m_angles->y = local_target_angle.y - 52;
					if (delta <  192) m_angles->y = local_target_angle.y + 52.5;
					else m_angles->y = local_target_angle.y - 52.5;
					if (delta <  194) m_angles->y = local_target_angle.y + 53;
					else m_angles->y = local_target_angle.y - 53;
					if (delta <  196) m_angles->y = local_target_angle.y + 53.5;
					else m_angles->y = local_target_angle.y - 53.5;
					if (delta <  198) m_angles->y = local_target_angle.y + 54;
					else m_angles->y = local_target_angle.y - 54;
					if (delta <  200) m_angles->y = local_target_angle.y + 54.5;
					else m_angles->y = local_target_angle.y - 54.5;
					if (delta <  202) m_angles->y = local_target_angle.y + 55;
					else m_angles->y = local_target_angle.y - 55;
					if (delta <  204) m_angles->y = local_target_angle.y + 55.5;
					else m_angles->y = local_target_angle.y - 55.5;
					if (delta <  206) m_angles->y = local_target_angle.y + 56;
					else m_angles->y = local_target_angle.y - 56;
					if (delta <  208) m_angles->y = local_target_angle.y + 56.5;
					else m_angles->y = local_target_angle.y - 56.5;
					if (delta <  210) m_angles->y = local_target_angle.y + 57;
					else m_angles->y = local_target_angle.y - 57;
					if (delta <  212) m_angles->y = local_target_angle.y + 57.5;
					else m_angles->y = local_target_angle.y - 57.5;
					if (delta <  214) m_angles->y = local_target_angle.y + 58;
					else m_angles->y = local_target_angle.y - 58;
					if (delta <  216) m_angles->y = local_target_angle.y + 58.5;
					else m_angles->y = local_target_angle.y - 58.5;
					if (delta <  218) m_angles->y = local_target_angle.y + 58;
					else m_angles->y = local_target_angle.y - 58;
					if (delta <  220) m_angles->y = local_target_angle.y + 58.5;
					else m_angles->y = local_target_angle.y - 58.5;
					if (delta <  222) m_angles->y = local_target_angle.y + 59;
					else m_angles->y = local_target_angle.y - 59;
					if (delta <  224) m_angles->y = local_target_angle.y + 59.5;
					else m_angles->y = local_target_angle.y - 59.5;
					if (delta <  226) m_angles->y = local_target_angle.y + 60;
					else m_angles->y = local_target_angle.y - 60;
					if (delta <  228) m_angles->y = local_target_angle.y + 60.5;
					else m_angles->y = local_target_angle.y - 60.5;
					if (delta <  230) m_angles->y = local_target_angle.y + 61;
					else m_angles->y = local_target_angle.y - 61;
					if (delta <  232) m_angles->y = local_target_angle.y + 62.5;
					else m_angles->y = local_target_angle.y - 62.5;
					if (delta <  234) m_angles->y = local_target_angle.y + 63;
					else m_angles->y = local_target_angle.y - 63;
					if (delta <  236) m_angles->y = local_target_angle.y + 63.5;
					else m_angles->y = local_target_angle.y - 63.5;
					if (delta <  238) m_angles->y = local_target_angle.y + 64;
					else m_angles->y = local_target_angle.y - 64;
					if (delta <  240) m_angles->y = local_target_angle.y + 64.5;
					else m_angles->y = local_target_angle.y - 64.5;
					if (delta <  242) m_angles->y = local_target_angle.y + 65;
					else m_angles->y = local_target_angle.y - 65;
					if (delta <  244) m_angles->y = local_target_angle.y + 65.5;
					else m_angles->y = local_target_angle.y - 65.5;
					if (delta <  246) m_angles->y = local_target_angle.y + 66;
					else m_angles->y = local_target_angle.y - 66;
					if (delta <  248) m_angles->y = local_target_angle.y + 66.5;
					else m_angles->y = local_target_angle.y - 66.5;
					if (delta <  250) m_angles->y = local_target_angle.y + 67;
					else m_angles->y = local_target_angle.y - 67;
					if (delta <  252) m_angles->y = local_target_angle.y + 67.5;
					else m_angles->y = local_target_angle.y - 67.5;
					if (delta <  254) m_angles->y = local_target_angle.y + 68;
					else m_angles->y = local_target_angle.y - 68;
					if (delta <  256) m_angles->y = local_target_angle.y + 68.5;
					else m_angles->y = local_target_angle.y - 68.5;
					if (delta <  258) m_angles->y = local_target_angle.y + 69;
					else m_angles->y = local_target_angle.y - 69;
					if (delta <  260) m_angles->y = local_target_angle.y + 69.5;
					else m_angles->y = local_target_angle.y - 69.5;
					if (delta <  262) m_angles->y = local_target_angle.y + 70;
					else m_angles->y = local_target_angle.y - 70;
					if (delta <  264) m_angles->y = local_target_angle.y + 70.5;
					else m_angles->y = local_target_angle.y - 70.5;
					if (delta <  266) m_angles->y = local_target_angle.y + 71;
					else m_angles->y = local_target_angle.y - 71;
					if (delta <  268) m_angles->y = local_target_angle.y + 71.5;
					else m_angles->y = local_target_angle.y - 71.5;
					if (delta <  270) m_angles->y = local_target_angle.y + 72;
					else m_angles->y = local_target_angle.y - 72;
					if (delta <  272) m_angles->y = local_target_angle.y + 72.5;
					else m_angles->y = local_target_angle.y - 72.5;
					if (delta <  274) m_angles->y = local_target_angle.y + 73;
					else m_angles->y = local_target_angle.y - 73;
					if (delta <  276) m_angles->y = local_target_angle.y + 73.5;
					else m_angles->y = local_target_angle.y - 73.5;
					if (delta <  278) m_angles->y = local_target_angle.y + 74;
					else m_angles->y = local_target_angle.y - 74;
					if (delta <  280) m_angles->y = local_target_angle.y + 74.5;
					else m_angles->y = local_target_angle.y - 74.5;
					if (delta <  282) m_angles->y = local_target_angle.y + 75;
					else m_angles->y = local_target_angle.y - 75;
					if (delta <  284) m_angles->y = local_target_angle.y + 75.5;
					else m_angles->y = local_target_angle.y - 75.5;
					if (delta <  286) m_angles->y = local_target_angle.y + 76;
					else m_angles->y = local_target_angle.y - 76;
					if (delta <  288) m_angles->y = local_target_angle.y + 76.5;
					else m_angles->y = local_target_angle.y - 76.5;
					if (delta <  290) m_angles->y = local_target_angle.y + 77;
					else m_angles->y = local_target_angle.y - 77;
					if (delta <  292) m_angles->y = local_target_angle.y + 77.5;
					else m_angles->y = local_target_angle.y - 77.5;
					if (delta <  294) m_angles->y = local_target_angle.y + 78;
					else m_angles->y = local_target_angle.y - 78;
					if (delta <  296) m_angles->y = local_target_angle.y + 78.5;
					else m_angles->y = local_target_angle.y - 78.5;
					if (delta <  298) m_angles->y = local_target_angle.y + 79;
					else m_angles->y = local_target_angle.y - 79;
					if (delta <  300) m_angles->y = local_target_angle.y + 79.5;
					else m_angles->y = local_target_angle.y - 79.5;
					if (delta <  302) m_angles->y = local_target_angle.y + 80;
					else m_angles->y = local_target_angle.y - 80;
					if (delta <  304) m_angles->y = local_target_angle.y + 80.5;
					else m_angles->y = local_target_angle.y - 80.5;
					if (delta <  306) m_angles->y = local_target_angle.y + 81;
					else m_angles->y = local_target_angle.y - 81;
					if (delta <  308) m_angles->y = local_target_angle.y + 81.5;
					else m_angles->y = local_target_angle.y - 81.5;
					if (delta <  310) m_angles->y = local_target_angle.y + 82;
					else m_angles->y = local_target_angle.y - 82;
					if (delta <  312) m_angles->y = local_target_angle.y + 82.5;
					else m_angles->y = local_target_angle.y - 82.5;
					if (delta <  314) m_angles->y = local_target_angle.y + 83;
					else m_angles->y = local_target_angle.y - 83;
					if (delta <  316) m_angles->y = local_target_angle.y + 83.5;
					else m_angles->y = local_target_angle.y - 83.5;
					if (delta <  318) m_angles->y = local_target_angle.y + 84;
					else m_angles->y = local_target_angle.y - 84;
					if (delta <  320) m_angles->y = local_target_angle.y + 84.5;
					else m_angles->y = local_target_angle.y - 84.5;
					if (delta <  322) m_angles->y = local_target_angle.y + 85;
					else m_angles->y = local_target_angle.y - 85;
					if (delta <  324) m_angles->y = local_target_angle.y + 85.5;
					else m_angles->y = local_target_angle.y - 85.5;
					if (delta <  326) m_angles->y = local_target_angle.y + 86;
					else m_angles->y = local_target_angle.y - 86;
					if (delta <  328) m_angles->y = local_target_angle.y + 86.5;
					else m_angles->y = local_target_angle.y - 86.5;
					if (delta <  330) m_angles->y = local_target_angle.y + 87;
					else m_angles->y = local_target_angle.y - 87;
					if (delta <  332) m_angles->y = local_target_angle.y + 87.5;
					else m_angles->y = local_target_angle.y - 87.5;
					if (delta <  334) m_angles->y = local_target_angle.y + 88;
					else m_angles->y = local_target_angle.y - 88;
					if (delta <  336) m_angles->y = local_target_angle.y + 88.5;
					else m_angles->y = local_target_angle.y - 88.5;
					if (delta <  338) m_angles->y = local_target_angle.y + 89;
					else m_angles->y = local_target_angle.y - 89;
					if (delta <  340) m_angles->y = local_target_angle.y + 89.5;
					else m_angles->y = local_target_angle.y - 89.5;
					if (delta <  342) m_angles->y = local_target_angle.y + 90;
					else m_angles->y = local_target_angle.y - 90;
					if (delta <  344) m_angles->y = local_target_angle.y + 90.5;
					else m_angles->y = local_target_angle.y - 90.5;
					if (delta <  346) m_angles->y = local_target_angle.y + 91;
					else m_angles->y = local_target_angle.y - 91;
					if (delta <  348) m_angles->y = local_target_angle.y + 91.5;
					else m_angles->y = local_target_angle.y - 91.5;
					if (delta <  350) m_angles->y = local_target_angle.y + 92;
					else m_angles->y = local_target_angle.y - 92;
					if (delta <  352) m_angles->y = local_target_angle.y + 92.5;
					else m_angles->y = local_target_angle.y - 92.5;
					if (delta <  354) m_angles->y = local_target_angle.y + 93;
					else m_angles->y = local_target_angle.y - 93;
					if (delta <  356) m_angles->y = local_target_angle.y + 93.5;
					else m_angles->y = local_target_angle.y - 93.5;
					if (delta <  358) m_angles->y = local_target_angle.y + 94;
					else m_angles->y = local_target_angle.y - 94;
					if (delta <  360) m_angles->y = local_target_angle.y + 94.5;
					else m_angles->y = local_target_angle.y - 94.5;
					if (delta <  362) m_angles->y = local_target_angle.y + 95;
					else m_angles->y = local_target_angle.y - 95;
					if (delta <  364) m_angles->y = local_target_angle.y + 95.5;
					else m_angles->y = local_target_angle.y - 95.5;
					if (delta <  366) m_angles->y = local_target_angle.y + 96;
					else m_angles->y = local_target_angle.y - 96;
					if (delta <  369) m_angles->y = local_target_angle.y + 96.5;
					else m_angles->y = local_target_angle.y - 96.5;
					if (delta <  371) m_angles->y = local_target_angle.y + 97;
					else m_angles->y = local_target_angle.y - 97;
					if (delta <  373) m_angles->y = local_target_angle.y + 97.5;
					else m_angles->y = local_target_angle.y - 97.5;
					if (delta <  375) m_angles->y = local_target_angle.y + 98;
					else m_angles->y = local_target_angle.y - 98;
					if (delta <  377) m_angles->y = local_target_angle.y + 98.5;
					else m_angles->y = local_target_angle.y - 98.5;
					if (delta <  379) m_angles->y = local_target_angle.y + 99;
					else m_angles->y = local_target_angle.y - 99;
					if (delta <  381) m_angles->y = local_target_angle.y + 99.5;
					else m_angles->y = local_target_angle.y - 99.5;
					if (delta <  383) m_angles->y = local_target_angle.y + 100;
					else m_angles->y = local_target_angle.y - 100;
					if (delta <  385) m_angles->y = local_target_angle.y + 100.5;
					else m_angles->y = local_target_angle.y - 100.5;
					if (delta <  387) m_angles->y = local_target_angle.y + 101;
					else m_angles->y = local_target_angle.y - 101;
					if (delta <  389) m_angles->y = local_target_angle.y + 101.5;
					else m_angles->y = local_target_angle.y - 101.5;
					if (delta <  391) m_angles->y = local_target_angle.y + 102;
					else m_angles->y = local_target_angle.y - 102;
					if (delta <  393) m_angles->y = local_target_angle.y + 102.5;
					else m_angles->y = local_target_angle.y - 102.5;
					if (delta <  395) m_angles->y = local_target_angle.y + 103;
					else m_angles->y = local_target_angle.y - 103;
					if (delta <  397) m_angles->y = local_target_angle.y + 103.5;
					else m_angles->y = local_target_angle.y - 103.5;
					if (delta <  399) m_angles->y = local_target_angle.y + 104;
					else m_angles->y = local_target_angle.y - 104;
					if (delta <  401) m_angles->y = local_target_angle.y + 104.5;
					else m_angles->y = local_target_angle.y - 104.5;
					if (delta <  403) m_angles->y = local_target_angle.y + 104;
					else m_angles->y = local_target_angle.y - 104;
					if (delta <  405) m_angles->y = local_target_angle.y + 104.5;
					else m_angles->y = local_target_angle.y - 104.5;
					if (delta <  407) m_angles->y = local_target_angle.y + 105;
					else m_angles->y = local_target_angle.y - 105;
					if (delta <  409) m_angles->y = local_target_angle.y + 105.5;
					else m_angles->y = local_target_angle.y - 105.5;
					if (delta <  411) m_angles->y = local_target_angle.y + 106;
					else m_angles->y = local_target_angle.y - 106;
					if (delta <  413) m_angles->y = local_target_angle.y + 106.5;
					else m_angles->y = local_target_angle.y - 106.5;
					if (delta <  415) m_angles->y = local_target_angle.y + 107;
					else m_angles->y = local_target_angle.y - 107;
					if (delta <  417) m_angles->y = local_target_angle.y + 107.5;
					else m_angles->y = local_target_angle.y - 107.5;
					if (delta <  419) m_angles->y = local_target_angle.y + 108;
					else m_angles->y = local_target_angle.y - 108;
					if (delta <  421) m_angles->y = local_target_angle.y + 108.5;
					else m_angles->y = local_target_angle.y - 108.5;
					if (delta <  423) m_angles->y = local_target_angle.y + 109;
					else m_angles->y = local_target_angle.y - 109;
					if (delta <  425) m_angles->y = local_target_angle.y + 109.5;
					else m_angles->y = local_target_angle.y - 109.5;
					if (delta <  427) m_angles->y = local_target_angle.y + 110;
					else m_angles->y = local_target_angle.y - 110;
					if (delta <  429) m_angles->y = local_target_angle.y + 110.5;
					else m_angles->y = local_target_angle.y - 110.5;
					if (delta <  431) m_angles->y = local_target_angle.y + 111;
					else m_angles->y = local_target_angle.y - 111;
					if (delta <  433) m_angles->y = local_target_angle.y + 111.5;
					else m_angles->y = local_target_angle.y - 111.5;
					if (delta <  435) m_angles->y = local_target_angle.y + 112;
					else m_angles->y = local_target_angle.y - 112;
					if (delta <  437) m_angles->y = local_target_angle.y + 112.5;
					else m_angles->y = local_target_angle.y - 112.5;
					if (delta <  439) m_angles->y = local_target_angle.y + 113;
					else m_angles->y = local_target_angle.y - 113;
					if (delta <  441) m_angles->y = local_target_angle.y + 113.5;
					else m_angles->y = local_target_angle.y - 113.5;
					if (delta <  443) m_angles->y = local_target_angle.y + 114;
					else m_angles->y = local_target_angle.y - 114;
					if (delta <  445) m_angles->y = local_target_angle.y + 114.5;
					else m_angles->y = local_target_angle.y - 114.5;
					if (delta <  447) m_angles->y = local_target_angle.y + 115;
					else m_angles->y = local_target_angle.y - -115;
					if (delta <  449) m_angles->y = local_target_angle.y + 115.5;
					else m_angles->y = local_target_angle.y - 115.5;
					if (delta <  451) m_angles->y = local_target_angle.y + 116;
					else m_angles->y = local_target_angle.y - 116;
					if (delta <  453) m_angles->y = local_target_angle.y + 116.5;
					else m_angles->y = local_target_angle.y - 116.5;
					if (delta <  455) m_angles->y = local_target_angle.y + 117;
					else m_angles->y = local_target_angle.y - 117;
					if (delta <  457) m_angles->y = local_target_angle.y + 117.5;
					else m_angles->y = local_target_angle.y - 117.5;
					if (delta <  459) m_angles->y = local_target_angle.y + 118;
					else m_angles->y = local_target_angle.y - 118;
					if (delta <  461) m_angles->y = local_target_angle.y + 118.5;
					else m_angles->y = local_target_angle.y - 118.5;
					if (delta <  463) m_angles->y = local_target_angle.y + 119;
					else m_angles->y = local_target_angle.y - 119;
					if (delta <  465) m_angles->y = local_target_angle.y + 119.5;
					else m_angles->y = local_target_angle.y - 119.5;
					if (delta <  467) m_angles->y = local_target_angle.y + 120;
					else m_angles->y = local_target_angle.y - 120;
					if (delta <  469) m_angles->y = local_target_angle.y + 120.5;
					else m_angles->y = local_target_angle.y - 120.5;
					if (delta <  471) m_angles->y = local_target_angle.y + 121;
					else m_angles->y = local_target_angle.y - 121;
					if (delta <  473) m_angles->y = local_target_angle.y + 121.5;
					else m_angles->y = local_target_angle.y - 121.5;
					if (delta <  475) m_angles->y = local_target_angle.y + 122;
					else m_angles->y = local_target_angle.y - 122;
					if (delta <  477) m_angles->y = local_target_angle.y + 122.5;
					else m_angles->y = local_target_angle.y - 122.5;
					if (delta <  479) m_angles->y = local_target_angle.y + 123;
					else m_angles->y = local_target_angle.y - 123;
					if (delta <  481) m_angles->y = local_target_angle.y + 123.5;
					else m_angles->y = local_target_angle.y - 123.5;
					if (delta <  483) m_angles->y = local_target_angle.y + 124;
					else m_angles->y = local_target_angle.y - 124;
					if (delta <  485) m_angles->y = local_target_angle.y + 124.5;
					else m_angles->y = local_target_angle.y - 124.5;
					if (delta <  487) m_angles->y = local_target_angle.y + 125;
					else m_angles->y = local_target_angle.y - 125;
					if (delta <  489) m_angles->y = local_target_angle.y + 125.5;
					else m_angles->y = local_target_angle.y - 125.5;
					if (delta <  491) m_angles->y = local_target_angle.y + 126;
					else m_angles->y = local_target_angle.y - 126;
					if (delta <  493) m_angles->y = local_target_angle.y + 126.5;
					else m_angles->y = local_target_angle.y - 126.5;
					if (delta <  495) m_angles->y = local_target_angle.y + 127;
					else m_angles->y = local_target_angle.y - 127;
					if (delta <  497) m_angles->y = local_target_angle.y + 127.5;
					else m_angles->y = local_target_angle.y - 127.5;
					if (delta <  499) m_angles->y = local_target_angle.y + 128;
					else m_angles->y = local_target_angle.y - 128;
					if (delta <  501) m_angles->y = local_target_angle.y + 128.5;
					else m_angles->y = local_target_angle.y - 128.5;
					if (delta <  503) m_angles->y = local_target_angle.y + 129;
					else m_angles->y = local_target_angle.y - 129;
					if (delta <  505) m_angles->y = local_target_angle.y + 129.5;
					else m_angles->y = local_target_angle.y - 129.5;
					if (delta <  507) m_angles->y = local_target_angle.y + 130;
					else m_angles->y = local_target_angle.y - 130;
					if (delta <  509) m_angles->y = local_target_angle.y + 130.5;
					else m_angles->y = local_target_angle.y - 130.5;
					if (delta <  511) m_angles->y = local_target_angle.y + 131;
					else m_angles->y = local_target_angle.y - 131;
					if (delta <  513) m_angles->y = local_target_angle.y + 131.5;
					else m_angles->y = local_target_angle.y - 131.5;
					if (delta <  515) m_angles->y = local_target_angle.y + 132;
					else m_angles->y = local_target_angle.y - 132;
					if (delta <  517) m_angles->y = local_target_angle.y + 132.5;
					else m_angles->y = local_target_angle.y - 132.5;
					if (delta <  519) m_angles->y = local_target_angle.y + 133;
					else m_angles->y = local_target_angle.y - 133;
					if (delta <  521) m_angles->y = local_target_angle.y + 134.5;
					else m_angles->y = local_target_angle.y - 134.5;
					if (delta <  523) m_angles->y = local_target_angle.y + 135;
					else m_angles->y = local_target_angle.y - 135;
					if (delta <  525) m_angles->y = local_target_angle.y + 135.5;
					else m_angles->y = local_target_angle.y - 135.5;
					if (delta <  527) m_angles->y = local_target_angle.y + 136;
					else m_angles->y = local_target_angle.y - 136;
					if (delta <  529) m_angles->y = local_target_angle.y + 136.5;
					else m_angles->y = local_target_angle.y - 136.5;
					if (delta <  531) m_angles->y = local_target_angle.y + 137;
					else m_angles->y = local_target_angle.y - 137;
					if (delta <  533) m_angles->y = local_target_angle.y + 137.5;
					else m_angles->y = local_target_angle.y - 137.5;
					if (delta <  535) m_angles->y = local_target_angle.y + 138;
					else m_angles->y = local_target_angle.y - 138;
					if (delta <  537) m_angles->y = local_target_angle.y + 138.5;
					else m_angles->y = local_target_angle.y - 138.5;
					if (delta <  539) m_angles->y = local_target_angle.y + 140;
					else m_angles->y = local_target_angle.y - 140;
					if (delta <  541) m_angles->y = local_target_angle.y + 140.5;
					else m_angles->y = local_target_angle.y - 140.5;
					if (delta <  543) m_angles->y = local_target_angle.y + 141;
					else m_angles->y = local_target_angle.y - 141;
					if (delta <  545) m_angles->y = local_target_angle.y + 141.5;
					else m_angles->y = local_target_angle.y - 141.5;
					if (delta <  547) m_angles->y = local_target_angle.y + 142;
					else m_angles->y = local_target_angle.y - 142;
					if (delta <  549) m_angles->y = local_target_angle.y + 142.5;
					else m_angles->y = local_target_angle.y - 142.5;
					if (delta <  551) m_angles->y = local_target_angle.y + 143;
					else m_angles->y = local_target_angle.y - 143;
					if (delta <  553) m_angles->y = local_target_angle.y + 143.5;
					else m_angles->y = local_target_angle.y - 143.5;
					if (delta <  555) m_angles->y = local_target_angle.y + 144;
					else m_angles->y = local_target_angle.y - 144;
					if (delta <  557) m_angles->y = local_target_angle.y + 144.5;
					else m_angles->y = local_target_angle.y - 144.5;
					if (delta <  559) m_angles->y = local_target_angle.y + 145;
					else m_angles->y = local_target_angle.y - 145;
					if (delta <  561) m_angles->y = local_target_angle.y + 145.5;
					else m_angles->y = local_target_angle.y - 145.5;
					if (delta <  563) m_angles->y = local_target_angle.y + 146;
					else m_angles->y = local_target_angle.y - 146;
					if (delta <  565) m_angles->y = local_target_angle.y + 146.5;
					else m_angles->y = local_target_angle.y - 146.5;
					if (delta <  567) m_angles->y = local_target_angle.y + 147;
					else m_angles->y = local_target_angle.y - 147;
					if (delta <  569) m_angles->y = local_target_angle.y + 147.5;
					else m_angles->y = local_target_angle.y - 147.5;
					if (delta <  571) m_angles->y = local_target_angle.y + 148;
					else m_angles->y = local_target_angle.y - 148;
					if (delta <  573) m_angles->y = local_target_angle.y + 148.5;
					else m_angles->y = local_target_angle.y - 148.5;
					if (delta <  575) m_angles->y = local_target_angle.y + 149;
					else m_angles->y = local_target_angle.y - 149;
					if (delta <  577) m_angles->y = local_target_angle.y + 149.5;
					else m_angles->y = local_target_angle.y - 149.5;
					if (delta <  579) m_angles->y = local_target_angle.y + 150;
					else m_angles->y = local_target_angle.y - 150;
					if (delta <  581) m_angles->y = local_target_angle.y + 150.5;
					else m_angles->y = local_target_angle.y - 150.5;
					if (delta <  583) m_angles->y = local_target_angle.y + 151;
					else m_angles->y = local_target_angle.y - 151;
					if (delta <  585) m_angles->y = local_target_angle.y + 151.5;
					else m_angles->y = local_target_angle.y - 151.5;
					if (delta <  587) m_angles->y = local_target_angle.y + 152;
					else m_angles->y = local_target_angle.y - 152;
					if (delta <  589) m_angles->y = local_target_angle.y + 152.5;
					else m_angles->y = local_target_angle.y - 152.5;
					if (delta <  591) m_angles->y = local_target_angle.y + 153;
					else m_angles->y = local_target_angle.y - 153;
					if (delta <  593) m_angles->y = local_target_angle.y + 153.5;
					else m_angles->y = local_target_angle.y - 153.5;
					if (delta <  595) m_angles->y = local_target_angle.y + 154;
					else m_angles->y = local_target_angle.y - 154;
					if (delta <  597) m_angles->y = local_target_angle.y + 154.5;
					else m_angles->y = local_target_angle.y - 154.5;
					if (delta <  599) m_angles->y = local_target_angle.y + 155;
					else m_angles->y = local_target_angle.y - 155;
					if (delta <  601) m_angles->y = local_target_angle.y + 155.5;
					else m_angles->y = local_target_angle.y - 155.5;
					if (delta <  603) m_angles->y = local_target_angle.y + 156;
					else m_angles->y = local_target_angle.y - 156;
					if (delta <  605) m_angles->y = local_target_angle.y + 156.5;
					else m_angles->y = local_target_angle.y - 156.5;
					if (delta <  607) m_angles->y = local_target_angle.y + 157;
					else m_angles->y = local_target_angle.y - 157;
					if (delta <  609) m_angles->y = local_target_angle.y + 157.5;
					else m_angles->y = local_target_angle.y - 157.5;
					if (delta <  611) m_angles->y = local_target_angle.y + 158;
					else m_angles->y = local_target_angle.y - 158;
					if (delta <  613) m_angles->y = local_target_angle.y + 158.5;
					else m_angles->y = local_target_angle.y - 158.5;
					if (delta <  615) m_angles->y = local_target_angle.y + 159;
					else m_angles->y = local_target_angle.y - 159;
					if (delta <  617) m_angles->y = local_target_angle.y + 159.5;
					else m_angles->y = local_target_angle.y - 159.5;
					if (delta <  619) m_angles->y = local_target_angle.y + 160;
					else m_angles->y = local_target_angle.y - 160;
					if (delta <  621) m_angles->y = local_target_angle.y + 160.5;
					else m_angles->y = local_target_angle.y - 160.5;
					if (delta <  623) m_angles->y = local_target_angle.y + 161;
					else m_angles->y = local_target_angle.y - 161;
					if (delta <  625) m_angles->y = local_target_angle.y + 161.5;
					else m_angles->y = local_target_angle.y - 161.5;
					if (delta <  627) m_angles->y = local_target_angle.y + 162;
					else m_angles->y = local_target_angle.y - 162;
					if (delta <  629) m_angles->y = local_target_angle.y + 162.5;
					else m_angles->y = local_target_angle.y - 162.5;
					if (delta <  631) m_angles->y = local_target_angle.y + 163;
					else m_angles->y = local_target_angle.y - 163;
					if (delta <  633) m_angles->y = local_target_angle.y + 163.5;
					else m_angles->y = local_target_angle.y - 163.5;
					if (delta <  635) m_angles->y = local_target_angle.y + 164;
					else m_angles->y = local_target_angle.y - 164;
					if (delta <  637) m_angles->y = local_target_angle.y + 164.5;
					else m_angles->y = local_target_angle.y - 164.5;
					if (delta <  639) m_angles->y = local_target_angle.y + 165;
					else m_angles->y = local_target_angle.y - 165;
					if (delta <  641) m_angles->y = local_target_angle.y + 165.5;
					else m_angles->y = local_target_angle.y - 165.5;
					if (delta <  643) m_angles->y = local_target_angle.y + 166;
					else m_angles->y = local_target_angle.y - 166;
					if (delta <  645) m_angles->y = local_target_angle.y + 166.5;
					else m_angles->y = local_target_angle.y - 166.5;
					if (delta <  647) m_angles->y = local_target_angle.y + 167;
					else m_angles->y = local_target_angle.y - 167;
					if (delta <  649) m_angles->y = local_target_angle.y + 167.5;
					else m_angles->y = local_target_angle.y - 167.5;
					if (delta <  651) m_angles->y = local_target_angle.y + 168;
					else m_angles->y = local_target_angle.y - 168;
					if (delta <  653) m_angles->y = local_target_angle.y + 168.5;
					else m_angles->y = local_target_angle.y - 168.5;
					if (delta <  655) m_angles->y = local_target_angle.y + 169;
					else m_angles->y = local_target_angle.y - 169;
					if (delta <  657) m_angles->y = local_target_angle.y + 169.5;
					else m_angles->y = local_target_angle.y - 169.5;
					if (delta <  659) m_angles->y = local_target_angle.y + 170;
					else m_angles->y = local_target_angle.y - 170;
					if (delta <  661) m_angles->y = local_target_angle.y + 170.5;
					else m_angles->y = local_target_angle.y - 170.5;
					if (delta <  663) m_angles->y = local_target_angle.y + 171;
					else m_angles->y = local_target_angle.y - 171;
					if (delta <  665) m_angles->y = local_target_angle.y + 171.5;
					else m_angles->y = local_target_angle.y - 171.5;
					if (delta <  667) m_angles->y = local_target_angle.y + 172;
					else m_angles->y = local_target_angle.y - 172;
					if (delta <  669) m_angles->y = local_target_angle.y + 172.5;
					else m_angles->y = local_target_angle.y - 172.5;
					if (delta <  671) m_angles->y = local_target_angle.y + 173;
					else m_angles->y = local_target_angle.y - 173;
					if (delta <  673) m_angles->y = local_target_angle.y + 173.5;
					else m_angles->y = local_target_angle.y - 173.5;
					if (delta <  675) m_angles->y = local_target_angle.y + 174;
					else m_angles->y = local_target_angle.y - 174;
					if (delta <  677) m_angles->y = local_target_angle.y + 174.5;
					else m_angles->y = local_target_angle.y - 174.5;
					if (delta <  679) m_angles->y = local_target_angle.y + 175;
					else m_angles->y = local_target_angle.y - 175;
					if (delta <  681) m_angles->y = local_target_angle.y + 175.5;
					else m_angles->y = local_target_angle.y - 175.5;
					if (delta <  683) m_angles->y = local_target_angle.y + 176;
					else m_angles->y = local_target_angle.y - 176;
					if (delta <  685) m_angles->y = local_target_angle.y + 176.5;
					else m_angles->y = local_target_angle.y - 176.5;
					if (delta <  687) m_angles->y = local_target_angle.y + 177;
					else m_angles->y = local_target_angle.y - 177;
					if (delta <  689) m_angles->y = local_target_angle.y + 177.5;
					else m_angles->y = local_target_angle.y - 177.5;
					if (delta <  691) m_angles->y = local_target_angle.y + 178;
					else m_angles->y = local_target_angle.y - 178;
					if (delta <  693) m_angles->y = local_target_angle.y + 178.5;
					else m_angles->y = local_target_angle.y - 178.5;
					if (delta <  695) m_angles->y = local_target_angle.y + 179;
					else m_angles->y = local_target_angle.y - 179;
					if (delta <  697) m_angles->y = local_target_angle.y + 179.5;
					else m_angles->y = local_target_angle.y - 179.5;
					if (delta <  699) m_angles->y = local_target_angle.y + 180;
					else m_angles->y = local_target_angle.y - 180;
					if (delta <  701) m_angles->y = local_target_angle.y + 180.5;
					else m_angles->y = local_target_angle.y - 180.5;
					if (delta <  703) m_angles->y = local_target_angle.y + 181;
					else m_angles->y = local_target_angle.y - 181;
					if (delta <  705) m_angles->y = local_target_angle.y + 181.5;
					else m_angles->y = local_target_angle.y - 181.5;
					if (delta <  707) m_angles->y = local_target_angle.y + 182;
					else m_angles->y = local_target_angle.y - 182;
					if (delta <  709) m_angles->y = local_target_angle.y + 182.5;
					else m_angles->y = local_target_angle.y - 182.5;
					if (delta <  711) m_angles->y = local_target_angle.y + 183;
					else m_angles->y = local_target_angle.y - 183;
					if (delta <  713) m_angles->y = local_target_angle.y + 183.5;
					else m_angles->y = local_target_angle.y - 183.5;
					if (delta <  715) m_angles->y = local_target_angle.y + 184;
					else m_angles->y = local_target_angle.y - 184;
					if (delta <  717) m_angles->y = local_target_angle.y + 184.5;
					else m_angles->y = local_target_angle.y - 184.5;
					if (delta <  719) m_angles->y = local_target_angle.y + 185;
					else m_angles->y = local_target_angle.y - 185;
					if (delta <  721) m_angles->y = local_target_angle.y + 185.5;
					else m_angles->y = local_target_angle.y - 185.5;
					if (delta <  723) m_angles->y = local_target_angle.y + 186;
					else m_angles->y = local_target_angle.y - 186;
					if (delta <  725) m_angles->y = local_target_angle.y + 186.5;
					else m_angles->y = local_target_angle.y - 186.5;
					if (delta <  727) m_angles->y = local_target_angle.y + 187;
					else m_angles->y = local_target_angle.y - 187;
					if (delta <  729) m_angles->y = local_target_angle.y + 187.5;
					else m_angles->y = local_target_angle.y - 187.5;
					if (delta <  731) m_angles->y = local_target_angle.y + 188;
					else m_angles->y = local_target_angle.y - 188;
					if (delta <  733) m_angles->y = local_target_angle.y + 188.5;
					else m_angles->y = local_target_angle.y - 188.5;
					if (delta <  735) m_angles->y = local_target_angle.y + 189;
					else m_angles->y = local_target_angle.y - 189;
					if (delta <  737) m_angles->y = local_target_angle.y + 189.5;
					else m_angles->y = local_target_angle.y - 189.5;
					if (delta <  739) m_angles->y = local_target_angle.y + 190;
					else m_angles->y = local_target_angle.y - 190;
					if (delta <  741) m_angles->y = local_target_angle.y + 190.5;
					else m_angles->y = local_target_angle.y - 190.5;
					if (delta <  743) m_angles->y = local_target_angle.y + 191;
					else m_angles->y = local_target_angle.y - 191;
					if (delta <  745) m_angles->y = local_target_angle.y + 191.5;
					else m_angles->y = local_target_angle.y - 191.5;
					if (delta <  747) m_angles->y = local_target_angle.y + 192;
					else m_angles->y = local_target_angle.y - 192;
					if (delta <  749) m_angles->y = local_target_angle.y + 192.5;
					else m_angles->y = local_target_angle.y - 192.5;
					if (delta <  751) m_angles->y = local_target_angle.y + 193;
					else m_angles->y = local_target_angle.y - 193;
					if (delta <  753) m_angles->y = local_target_angle.y + 193.5;
					else m_angles->y = local_target_angle.y - 193.5;
					if (delta <  755) m_angles->y = local_target_angle.y + 194;
					else m_angles->y = local_target_angle.y - 194;
					if (delta <  757) m_angles->y = local_target_angle.y + 194.5;
					else m_angles->y = local_target_angle.y - 194.5;
					if (delta <  759) m_angles->y = local_target_angle.y + 195;
					else m_angles->y = local_target_angle.y - 195;
					if (delta <  761) m_angles->y = local_target_angle.y + 195.5;
					else m_angles->y = local_target_angle.y - 195.5;
					if (delta <  763) m_angles->y = local_target_angle.y + 196;
					else m_angles->y = local_target_angle.y - 196;
					if (delta <  765) m_angles->y = local_target_angle.y + 196.5;
					else m_angles->y = local_target_angle.y - 196.5;
					if (delta <  767) m_angles->y = local_target_angle.y + 197;
					else m_angles->y = local_target_angle.y - 197;
					if (delta <  769) m_angles->y = local_target_angle.y + 197.5;
					else m_angles->y = local_target_angle.y - 197.5;
					if (delta <  771) m_angles->y = local_target_angle.y + 198;
					else m_angles->y = local_target_angle.y - 198;
					if (delta <  773) m_angles->y = local_target_angle.y + 198.5;
					else m_angles->y = local_target_angle.y - 198.5;
					if (delta <  775) m_angles->y = local_target_angle.y + 199;
					else m_angles->y = local_target_angle.y - 199;
					if (delta <  777) m_angles->y = local_target_angle.y + 199.5;
					else m_angles->y = local_target_angle.y - 199.5;
					if (delta <  779) m_angles->y = local_target_angle.y + 200;
					else m_angles->y = local_target_angle.y - 200;
					if (delta <  781) m_angles->y = local_target_angle.y + 200.5;
					else m_angles->y = local_target_angle.y - 200.5;
					if (delta <  783) m_angles->y = local_target_angle.y + 201;
					else m_angles->y = local_target_angle.y - 201;
					if (delta <  785) m_angles->y = local_target_angle.y + 201.5;
					else m_angles->y = local_target_angle.y - 201.5;
					if (delta <  787) m_angles->y = local_target_angle.y + 202;
					else m_angles->y = local_target_angle.y - 202;
					if (delta <  789) m_angles->y = local_target_angle.y + 202.5;
					else m_angles->y = local_target_angle.y - 202.5;
					if (delta <  791) m_angles->y = local_target_angle.y + 203;
					else m_angles->y = local_target_angle.y - 203;
					if (delta <  793) m_angles->y = local_target_angle.y + 203.5;
					else m_angles->y = local_target_angle.y - 203.5;
					if (delta <  795) m_angles->y = local_target_angle.y + 204;
					else m_angles->y = local_target_angle.y - 204;
					if (delta <  797) m_angles->y = local_target_angle.y + 204.5;
					else m_angles->y = local_target_angle.y - 204.5;
					if (delta <  799) m_angles->y = local_target_angle.y + 205;
					else m_angles->y = local_target_angle.y - 205;
					if (delta <  801) m_angles->y = local_target_angle.y + 205.5;
					else m_angles->y = local_target_angle.y - 205.5;
					if (delta <  803) m_angles->y = local_target_angle.y + 206;
					else m_angles->y = local_target_angle.y - 206;
					if (delta <  805) m_angles->y = local_target_angle.y + 206.5;
					else m_angles->y = local_target_angle.y - 206.5;
					if (delta <  807) m_angles->y = local_target_angle.y + 207;
					else m_angles->y = local_target_angle.y - 207;
					if (delta <  809) m_angles->y = local_target_angle.y + 207.5;
					else m_angles->y = local_target_angle.y - 207.5;
					if (delta <  811) m_angles->y = local_target_angle.y + 208;
					else m_angles->y = local_target_angle.y - 208;
					if (delta <  813) m_angles->y = local_target_angle.y + 208.5;
					else m_angles->y = local_target_angle.y - 208.5;
					if (delta <  815) m_angles->y = local_target_angle.y + 209;
					else m_angles->y = local_target_angle.y - 209;
					if (delta <  817) m_angles->y = local_target_angle.y + 209.5;
					else m_angles->y = local_target_angle.y - 209.5;
					if (delta <  819) m_angles->y = local_target_angle.y + 210;
					else m_angles->y = local_target_angle.y - 210;
					if (delta <  821) m_angles->y = local_target_angle.y + 210.5;
					else m_angles->y = local_target_angle.y - 210.5;
					if (delta <  823) m_angles->y = local_target_angle.y + 211;
					else m_angles->y = local_target_angle.y - 211;
					if (delta <  825) m_angles->y = local_target_angle.y + 211.5;
					else m_angles->y = local_target_angle.y - 211.5;
					if (delta <  827) m_angles->y = local_target_angle.y + 212;
					else m_angles->y = local_target_angle.y - 212;
					if (delta <  829) m_angles->y = local_target_angle.y - 212.5;
					else m_angles->y = local_target_angle.y + 213;
					if (delta <  831) m_angles->y = local_target_angle.y - 213;
					else m_angles->y = local_target_angle.y + 213.5;
					if (delta <  833) m_angles->y = local_target_angle.y - 213.5;
					else m_angles->y = local_target_angle.y + 214;
					if (delta <  835) m_angles->y = local_target_angle.y - 214;
					else m_angles->y = local_target_angle.y + 214.5;
					if (delta <  837) m_angles->y = local_target_angle.y - 214.5;
					else m_angles->y = local_target_angle.y + 215;
					if (delta <  839) m_angles->y = local_target_angle.y - 215;
					else m_angles->y = local_target_angle.y + 215.5;
					if (delta <  841) m_angles->y = local_target_angle.y - 215.5;
					else m_angles->y = local_target_angle.y + 216;
					if (delta <  843) m_angles->y = local_target_angle.y - 216;
					else m_angles->y = local_target_angle.y + 216.5;
					if (delta <  845) m_angles->y = local_target_angle.y - 216.5;
					else m_angles->y = local_target_angle.y + 217;
					if (delta <  847) m_angles->y = local_target_angle.y - 217;
					else m_angles->y = local_target_angle.y + 217.5;
					if (delta <  849) m_angles->y = local_target_angle.y - 217.5;
					else m_angles->y = local_target_angle.y + 218;
					if (delta <  851) m_angles->y = local_target_angle.y - 218;
					else m_angles->y = local_target_angle.y + 218.5;
					if (delta <  853) m_angles->y = local_target_angle.y - 218.5;
					else m_angles->y = local_target_angle.y + 219;
					if (delta <  855) m_angles->y = local_target_angle.y - 219;
					else m_angles->y = local_target_angle.y + 219.5;
					if (delta <  857) m_angles->y = local_target_angle.y - 219.5;
					else m_angles->y = local_target_angle.y + 220;
					if (delta <  859) m_angles->y = local_target_angle.y - 220;
					else m_angles->y = local_target_angle.y + 220.5;
					if (delta <  861) m_angles->y = local_target_angle.y - 220.5;
					else m_angles->y = local_target_angle.y + 221;
					if (delta <  863) m_angles->y = local_target_angle.y - 221;
					else m_angles->y = local_target_angle.y + 221.5;
					if (delta <  865) m_angles->y = local_target_angle.y - 221.5;
					else m_angles->y = local_target_angle.y + 222;
					if (delta <  867) m_angles->y = local_target_angle.y - 222;
					else m_angles->y = local_target_angle.y + 222.5;
					if (delta <  869) m_angles->y = local_target_angle.y - 222.5;
					else m_angles->y = local_target_angle.y + 223;
					if (delta <  871) m_angles->y = local_target_angle.y - 223;
					else m_angles->y = local_target_angle.y + 223.5;
					if (delta <  873) m_angles->y = local_target_angle.y - 223.5;
					else m_angles->y = local_target_angle.y + 224;
					if (delta <  875) m_angles->y = local_target_angle.y - 224;
					else m_angles->y = local_target_angle.y + 224.5;
					if (delta <  877) m_angles->y = local_target_angle.y - 224.5;
					else m_angles->y = local_target_angle.y + 225;
					if (delta <  879) m_angles->y = local_target_angle.y - 225;
					else m_angles->y = local_target_angle.y + 225.5;
					if (delta <  881) m_angles->y = local_target_angle.y - 225.5;
					else m_angles->y = local_target_angle.y + 226;
					if (delta <  883) m_angles->y = local_target_angle.y - 226;
					else m_angles->y = local_target_angle.y + 226.5;
					if (delta <  885) m_angles->y = local_target_angle.y - 226.5;
					else m_angles->y = local_target_angle.y + 227;
					if (delta <  887) m_angles->y = local_target_angle.y - 227;
					else m_angles->y = local_target_angle.y + 227.5;
					if (delta <  889) m_angles->y = local_target_angle.y - 227.5;
					else m_angles->y = local_target_angle.y + 228;
					if (delta <  901) m_angles->y = local_target_angle.y - 228;
					else m_angles->y = local_target_angle.y + 228.5;
					if (delta <  903) m_angles->y = local_target_angle.y - 228.5;
					else m_angles->y = local_target_angle.y + 229;
					if (delta <  905) m_angles->y = local_target_angle.y - 229;
					else m_angles->y = local_target_angle.y + 229.5;
					if (delta <  907) m_angles->y = local_target_angle.y - 229.5;
					else m_angles->y = local_target_angle.y + 230;
					if (delta <  909) m_angles->y = local_target_angle.y - 230;
					else m_angles->y = local_target_angle.y + 230.5;
					if (delta <  911) m_angles->y = local_target_angle.y - 230.5;
					else m_angles->y = local_target_angle.y + 231;
					if (delta <  913) m_angles->y = local_target_angle.y - 231;
					else m_angles->y = local_target_angle.y + 231.5;
					if (delta <  915) m_angles->y = local_target_angle.y - 231.5;
					else m_angles->y = local_target_angle.y + 232;
					if (delta <  917) m_angles->y = local_target_angle.y - 232;
					else m_angles->y = local_target_angle.y + 232.5;
					if (delta <  919) m_angles->y = local_target_angle.y - 232.5;
					else m_angles->y = local_target_angle.y + 233;
					if (delta <  921) m_angles->y = local_target_angle.y - 233;
					else m_angles->y = local_target_angle.y + 233.5;
					if (delta <  923) m_angles->y = local_target_angle.y - 233.5;
					else m_angles->y = local_target_angle.y + 234;
					if (delta <  925) m_angles->y = local_target_angle.y - 234;
					else m_angles->y = local_target_angle.y + 234.5;
					if (delta <  927) m_angles->y = local_target_angle.y - 234.5;
					else m_angles->y = local_target_angle.y + 235;
					if (delta <  929) m_angles->y = local_target_angle.y - 235;
					else m_angles->y = local_target_angle.y + 235.5;
					if (delta <  931) m_angles->y = local_target_angle.y - 235.5;
					else m_angles->y = local_target_angle.y + 236;
					if (delta <  933) m_angles->y = local_target_angle.y - 236;
					else m_angles->y = local_target_angle.y + 236.5;
					if (delta <  935) m_angles->y = local_target_angle.y - 236.5;
					else m_angles->y = local_target_angle.y + 237;
					if (delta <  937) m_angles->y = local_target_angle.y - 237;
					else m_angles->y = local_target_angle.y + 237.5;
					if (delta <  939) m_angles->y = local_target_angle.y - 237.5;
					else m_angles->y = local_target_angle.y + 238;
					if (delta <  941) m_angles->y = local_target_angle.y - 238;
					else m_angles->y = local_target_angle.y + 238.5;
					if (delta <  943) m_angles->y = local_target_angle.y - 238.5;
					else m_angles->y = local_target_angle.y + 239;
					if (delta <  945) m_angles->y = local_target_angle.y - 239;
					else m_angles->y = local_target_angle.y + 239.5;
					if (delta <  947) m_angles->y = local_target_angle.y - 239.5;
					else m_angles->y = local_target_angle.y + 240;
					if (delta <  949) m_angles->y = local_target_angle.y - 240;
					else m_angles->y = local_target_angle.y + 240.5;
					if (delta <  951) m_angles->y = local_target_angle.y - 240.5;
					else m_angles->y = local_target_angle.y + 241;
					if (delta <  953) m_angles->y = local_target_angle.y - 241;
					else m_angles->y = local_target_angle.y + 241.5;
					if (delta <  955) m_angles->y = local_target_angle.y - 241.5;
					else m_angles->y = local_target_angle.y + 242;
					if (delta <  957) m_angles->y = local_target_angle.y - 242;
					else m_angles->y = local_target_angle.y + 242.5;
					if (delta <  959) m_angles->y = local_target_angle.y - 242.5;
					else m_angles->y = local_target_angle.y + 243;
					if (delta <  961) m_angles->y = local_target_angle.y - 243;
					else m_angles->y = local_target_angle.y + 243.5;
					if (delta <  963) m_angles->y = local_target_angle.y - 243.5;
					else m_angles->y = local_target_angle.y + 244;
					if (delta <  965) m_angles->y = local_target_angle.y - 244;
					else m_angles->y = local_target_angle.y + 244.5;
					if (delta <  967) m_angles->y = local_target_angle.y - 244.5;
					else m_angles->y = local_target_angle.y + 245;
					if (delta <  969) m_angles->y = local_target_angle.y - 245;
					else m_angles->y = local_target_angle.y + 245.5;
					if (delta <  971) m_angles->y = local_target_angle.y - 245.5;
					else m_angles->y = local_target_angle.y + 246;
					if (delta <  973) m_angles->y = local_target_angle.y - 246;
					else m_angles->y = local_target_angle.y + 246.5;
					if (delta <  975) m_angles->y = local_target_angle.y - 246.5;
					else m_angles->y = local_target_angle.y + 247;
					if (delta <  977) m_angles->y = local_target_angle.y - 247;
					else m_angles->y = local_target_angle.y + 247.5;
					if (delta <  979) m_angles->y = local_target_angle.y - 247.5;
					else m_angles->y = local_target_angle.y + 248;
					if (delta <  981) m_angles->y = local_target_angle.y - 248;
					else m_angles->y = local_target_angle.y + 248.5;
					if (delta <  983) m_angles->y = local_target_angle.y - 248.5;
					else m_angles->y = local_target_angle.y + 249;
					if (delta <  985) m_angles->y = local_target_angle.y - 249;
					else m_angles->y = local_target_angle.y + 249.5;
					if (delta <  987) m_angles->y = local_target_angle.y - 249.5;
					else m_angles->y = local_target_angle.y + 250;
					if (delta <  989) m_angles->y = local_target_angle.y - 250;
					else m_angles->y = local_target_angle.y + 250.5;
					if (delta <  991) m_angles->y = local_target_angle.y - 250.5;
					else m_angles->y = local_target_angle.y + 251;
					if (delta <  993) m_angles->y = local_target_angle.y - 251;
					else m_angles->y = local_target_angle.y + 251.5;
					if (delta <  995) m_angles->y = local_target_angle.y - 251.5;
					else m_angles->y = local_target_angle.y + 252;
					if (delta <  997) m_angles->y = local_target_angle.y - 252;
					else m_angles->y = local_target_angle.y + 252.5;
					if (delta <  999) m_angles->y = local_target_angle.y - 252.5;
					else m_angles->y = local_target_angle.y + 253;
					if (delta <  1001) m_angles->y = local_target_angle.y - 253;
					else m_angles->y = local_target_angle.y + 253.5;
					if (delta <  1003) m_angles->y = local_target_angle.y - 253.5;
					else m_angles->y = local_target_angle.y + 254;
					if (delta <  1005) m_angles->y = local_target_angle.y - 254;
					else m_angles->y = local_target_angle.y + 254.5;
					if (delta <  1007) m_angles->y = local_target_angle.y - 254.5;
					else m_angles->y = local_target_angle.y + 255;
					if (delta <  1009) m_angles->y = local_target_angle.y - 255;
					else m_angles->y = local_target_angle.y + 255.5;
					if (delta <  1011) m_angles->y = local_target_angle.y - 255.5;
					else m_angles->y = local_target_angle.y + 256;
					if (delta <  1013) m_angles->y = local_target_angle.y - 256;
					else m_angles->y = local_target_angle.y + 256.5;
					if (delta <  1015) m_angles->y = local_target_angle.y - 256.5;
					else m_angles->y = local_target_angle.y + 257;
					if (delta <  1017) m_angles->y = local_target_angle.y - 257;
					else m_angles->y = local_target_angle.y + 257.5;
					if (delta <  1019) m_angles->y = local_target_angle.y - 257.5;
					else m_angles->y = local_target_angle.y + 258;
					if (delta <  1021) m_angles->y = local_target_angle.y - 258;
					else m_angles->y = local_target_angle.y + 258.5;
					if (delta <  1023) m_angles->y = local_target_angle.y - 258.5;
					else m_angles->y = local_target_angle.y + 259;
					if (delta <  1025) m_angles->y = local_target_angle.y - 259;
					else m_angles->y = local_target_angle.y + 259.5;
					if (delta <  1027) m_angles->y = local_target_angle.y - 259.5;
					else m_angles->y = local_target_angle.y + 260;
					if (delta <  1029) m_angles->y = local_target_angle.y - 260;
					else m_angles->y = local_target_angle.y + 260.5;
					if (delta <  1031) m_angles->y = local_target_angle.y - 260.5;
					else m_angles->y = local_target_angle.y + 261;
					if (delta <  1033) m_angles->y = local_target_angle.y - 261;
					else m_angles->y = local_target_angle.y + 261.5;
					if (delta <  1035) m_angles->y = local_target_angle.y - 261.5;
					else m_angles->y = local_target_angle.y + 262;
					if (delta <  1037) m_angles->y = local_target_angle.y - 262;
					else m_angles->y = local_target_angle.y + 262.5;
					if (delta <  1039) m_angles->y = local_target_angle.y - 262.5;
					else m_angles->y = local_target_angle.y + 263;
					if (delta <  1041) m_angles->y = local_target_angle.y - 263;
					else m_angles->y = local_target_angle.y + 263.5;
					if (delta <  1043) m_angles->y = local_target_angle.y - 263.5;
					else m_angles->y = local_target_angle.y + 264;
					if (delta <  1045) m_angles->y = local_target_angle.y - 264;
					else m_angles->y = local_target_angle.y + 264.5;
					if (delta <  1047) m_angles->y = local_target_angle.y - 264.5;
					else m_angles->y = local_target_angle.y + 265;
					if (delta <  1049) m_angles->y = local_target_angle.y - 265;
					else m_angles->y = local_target_angle.y + 265.5;
					if (delta <  1051) m_angles->y = local_target_angle.y - 265.5;
					else m_angles->y = local_target_angle.y + 266;
					if (delta <  1053) m_angles->y = local_target_angle.y - 266;
					else m_angles->y = local_target_angle.y + 266.5;
					if (delta <  1055) m_angles->y = local_target_angle.y - 266.5;
					else m_angles->y = local_target_angle.y + 267;
					if (delta <  1057) m_angles->y = local_target_angle.y - 267;
					else m_angles->y = local_target_angle.y + 267.5;
					if (delta <  1059) m_angles->y = local_target_angle.y - 267.5;
					else m_angles->y = local_target_angle.y + 268;
					if (delta <  1061) m_angles->y = local_target_angle.y + 268.5;
					else m_angles->y = local_target_angle.y + 268.5;
					if (delta <  1063) m_angles->y = local_target_angle.y + 269;
					else m_angles->y = local_target_angle.y - 269;
					if (delta <  1065) m_angles->y = local_target_angle.y + 269.5;
					else m_angles->y = local_target_angle.y - 269.5;
					if (delta <  1067) m_angles->y = local_target_angle.y + 270;
					else m_angles->y = local_target_angle.y - 270;
					if (delta <  1069) m_angles->y = local_target_angle.y + 270.5;
					else m_angles->y = local_target_angle.y - 270.5;
					if (delta <  1071) m_angles->y = local_target_angle.y + 271;
					else m_angles->y = local_target_angle.y - 271;
					if (delta <  1073) m_angles->y = local_target_angle.y + 271.5;
					else m_angles->y = local_target_angle.y - 271.5;
					if (delta <  1075) m_angles->y = local_target_angle.y + 272;
					else m_angles->y = local_target_angle.y - 272;
					if (delta <  1077) m_angles->y = local_target_angle.y + 272.5;
					else m_angles->y = local_target_angle.y - 272.5;
					if (delta <  1079) m_angles->y = local_target_angle.y + 273;
					else m_angles->y = local_target_angle.y - 273;
					if (delta <  1081) m_angles->y = local_target_angle.y + 273.5;
					else m_angles->y = local_target_angle.y - 273.5;
					if (delta <  1083) m_angles->y = local_target_angle.y + 274;
					else m_angles->y = local_target_angle.y - 274;
					if (delta <  1085) m_angles->y = local_target_angle.y + 274.5;
					else m_angles->y = local_target_angle.y - 274.5;
					if (delta <  1087) m_angles->y = local_target_angle.y + 275;
					else m_angles->y = local_target_angle.y - 275;
					if (delta <  1089) m_angles->y = local_target_angle.y + 275.5;
					else m_angles->y = local_target_angle.y - 275.5;
					if (delta <  1091) m_angles->y = local_target_angle.y + 276;
					else m_angles->y = local_target_angle.y - 276;
					if (delta <  1093) m_angles->y = local_target_angle.y + 276.5;
					else m_angles->y = local_target_angle.y - 276.5;
					if (delta <  1095) m_angles->y = local_target_angle.y + 277;
					else m_angles->y = local_target_angle.y - 277;
					if (delta <  1097) m_angles->y = local_target_angle.y + 277.5;
					else m_angles->y = local_target_angle.y - 277.5;
					if (delta <  1099) m_angles->y = local_target_angle.y + 278;
					else m_angles->y = local_target_angle.y - 278;
					if (delta <  1101) m_angles->y = local_target_angle.y + 278.5;
					else m_angles->y = local_target_angle.y - 278.5;
					if (delta <  1103) m_angles->y = local_target_angle.y + 279;
					else m_angles->y = local_target_angle.y - 279;
					if (delta <  1105) m_angles->y = local_target_angle.y + 279.5;
					else m_angles->y = local_target_angle.y - 279.5;
					if (delta <  1107) m_angles->y = local_target_angle.y + 280;
					else m_angles->y = local_target_angle.y - 280;
					if (delta <  1109) m_angles->y = local_target_angle.y + 280.5;
					else m_angles->y = local_target_angle.y - 280.5;
					if (delta <  1111) m_angles->y = local_target_angle.y + 281;
					else m_angles->y = local_target_angle.y - 281;
					if (delta <  1113) m_angles->y = local_target_angle.y + 281.5;
					else m_angles->y = local_target_angle.y - 281.5;
					if (delta <  1115) m_angles->y = local_target_angle.y + 282;
					else m_angles->y = local_target_angle.y - 282;
					if (delta <  1117) m_angles->y = local_target_angle.y + 282.5;
					else m_angles->y = local_target_angle.y - 283.5;
					if (delta <  1119) m_angles->y = local_target_angle.y + 283;
					else m_angles->y = local_target_angle.y - 283;
					if (delta <  1121) m_angles->y = local_target_angle.y + 283.5;
					else m_angles->y = local_target_angle.y - 283.5;
					if (delta <  1123) m_angles->y = local_target_angle.y + 284;
					else m_angles->y = local_target_angle.y - 284;
					if (delta <  1125) m_angles->y = local_target_angle.y + 284.5;
					else m_angles->y = local_target_angle.y - 284.5;
					if (delta <  1127) m_angles->y = local_target_angle.y + 285;
					else m_angles->y = local_target_angle.y - 285;
					if (delta <  1129) m_angles->y = local_target_angle.y + 285.5;
					else m_angles->y = local_target_angle.y - 285.5;
					if (delta <  1131) m_angles->y = local_target_angle.y + 286;
					else m_angles->y = local_target_angle.y - 286;
					if (delta <  1133) m_angles->y = local_target_angle.y + 286.5;
					else m_angles->y = local_target_angle.y - 286.5;
					if (delta <  1135) m_angles->y = local_target_angle.y + 287;
					else m_angles->y = local_target_angle.y - 287;
					if (delta <  1137) m_angles->y = local_target_angle.y + 287.5;
					else m_angles->y = local_target_angle.y - 287.5;
					if (delta <  1139) m_angles->y = local_target_angle.y + 288;
					else m_angles->y = local_target_angle.y - 288;
					if (delta <  1141) m_angles->y = local_target_angle.y + 288.5;
					else m_angles->y = local_target_angle.y - 288.5;
					if (delta <  1143) m_angles->y = local_target_angle.y + 289;
					else m_angles->y = local_target_angle.y - 289;
					if (delta <  1145) m_angles->y = local_target_angle.y + 289.5;
					else m_angles->y = local_target_angle.y - 289.5;
					if (delta <  1147) m_angles->y = local_target_angle.y + 290;
					else m_angles->y = local_target_angle.y - 290;
					if (delta <  1149) m_angles->y = local_target_angle.y + 290.5;
					else m_angles->y = local_target_angle.y - 290.5;
					if (delta <  1151) m_angles->y = local_target_angle.y + 291;
					else m_angles->y = local_target_angle.y - 291;
					if (delta <  1153) m_angles->y = local_target_angle.y + 291.5;
					else m_angles->y = local_target_angle.y - 291.5;
					if (delta <  1155) m_angles->y = local_target_angle.y + 292;
					else m_angles->y = local_target_angle.y - 292;
					if (delta <  1157) m_angles->y = local_target_angle.y + 292.5;
					else m_angles->y = local_target_angle.y - 292.5;
					if (delta <  1159) m_angles->y = local_target_angle.y + 293;
					else m_angles->y = local_target_angle.y - 293;
					if (delta <  1161) m_angles->y = local_target_angle.y + 293.5;
					else m_angles->y = local_target_angle.y - 293.5;
					if (delta <  1163) m_angles->y = local_target_angle.y + 294;
					else m_angles->y = local_target_angle.y - 294;
					if (delta <  1165) m_angles->y = local_target_angle.y + 294.5;
					else m_angles->y = local_target_angle.y - 294.5;
					if (delta <  1167) m_angles->y = local_target_angle.y + 295;
					else m_angles->y = local_target_angle.y - 295;
					if (delta <  1169) m_angles->y = local_target_angle.y + 295.5;
					else m_angles->y = local_target_angle.y - 295.5;
					if (delta <  1171) m_angles->y = local_target_angle.y + 296;
					else m_angles->y = local_target_angle.y - 296;
					if (delta <  1173) m_angles->y = local_target_angle.y + 296.5;
					else m_angles->y = local_target_angle.y - 296.5;
					if (delta <  1175) m_angles->y = local_target_angle.y + 297;
					else m_angles->y = local_target_angle.y - 297;
					if (delta <  1177) m_angles->y = local_target_angle.y + 297.5;
					else m_angles->y = local_target_angle.y - 297.5;
					if (delta <  1179) m_angles->y = local_target_angle.y + 298;
					else m_angles->y = local_target_angle.y - 298;
					if (delta <  1181) m_angles->y = local_target_angle.y + 298.5;
					else m_angles->y = local_target_angle.y - 298.5;
					if (delta <  1183) m_angles->y = local_target_angle.y + 299;
					else m_angles->y = local_target_angle.y - 299;
					if (delta <  1185) m_angles->y = local_target_angle.y + 299.5;
					else m_angles->y = local_target_angle.y - 299.5;
					if (delta <  1187) m_angles->y = local_target_angle.y + 300;
					else m_angles->y = local_target_angle.y - 300;
					if (delta <  1189) m_angles->y = local_target_angle.y + 300.5;
					else m_angles->y = local_target_angle.y - 300.5;
					if (delta <  1191) m_angles->y = local_target_angle.y + 301;
					else m_angles->y = local_target_angle.y - 301;
					if (delta <  1193) m_angles->y = local_target_angle.y + 301.5;
					else m_angles->y = local_target_angle.y - 301.5;
					if (delta <  1195) m_angles->y = local_target_angle.y + 302;
					else m_angles->y = local_target_angle.y - 302;
					if (delta <  1197) m_angles->y = local_target_angle.y + 302.5;
					else m_angles->y = local_target_angle.y - 302.5;
					if (delta <  1199) m_angles->y = local_target_angle.y + 303;
					else m_angles->y = local_target_angle.y - 303;
					if (delta <  1201) m_angles->y = local_target_angle.y + 303.5;
					else m_angles->y = local_target_angle.y - 303.5;
					if (delta <  1203) m_angles->y = local_target_angle.y + 304;
					else m_angles->y = local_target_angle.y - 304;
					if (delta <  1205) m_angles->y = local_target_angle.y + 304.5;
					else m_angles->y = local_target_angle.y - 304.5;
					if (delta <  1207) m_angles->y = local_target_angle.y + 305;
					else m_angles->y = local_target_angle.y - 305;
					if (delta <  1209) m_angles->y = local_target_angle.y + 305.5;
					else m_angles->y = local_target_angle.y - 305.5;
					if (delta <  1211) m_angles->y = local_target_angle.y + 306;
					else m_angles->y = local_target_angle.y - 306;
					if (delta <  1213) m_angles->y = local_target_angle.y + 306.5;
					else m_angles->y = local_target_angle.y - 306.5;
					if (delta <  1215) m_angles->y = local_target_angle.y + 307;
					else m_angles->y = local_target_angle.y - 307;
					if (delta <  1217) m_angles->y = local_target_angle.y + 307.5;
					else m_angles->y = local_target_angle.y - 307.5;
					if (delta <  1219) m_angles->y = local_target_angle.y + 308;
					else m_angles->y = local_target_angle.y - 308;
					if (delta <  1221) m_angles->y = local_target_angle.y + 308.5;
					else m_angles->y = local_target_angle.y - 308.5;
					if (delta <  1223) m_angles->y = local_target_angle.y + 309;
					else m_angles->y = local_target_angle.y - 309;
					if (delta <  1225) m_angles->y = local_target_angle.y + 309.5;
					else m_angles->y = local_target_angle.y - 309.5;
					if (delta <  1227) m_angles->y = local_target_angle.y + 310;
					else m_angles->y = local_target_angle.y - 310;
					if (delta <  1229) m_angles->y = local_target_angle.y + 310.5;
					else m_angles->y = local_target_angle.y - 310.5;
					if (delta <  1231) m_angles->y = local_target_angle.y + 311;
					else m_angles->y = local_target_angle.y - 311;
					if (delta <  1233) m_angles->y = local_target_angle.y + 311.5;
					else m_angles->y = local_target_angle.y - 311.5;
					if (delta <  1235) m_angles->y = local_target_angle.y + 312;
					else m_angles->y = local_target_angle.y - 312;
					if (delta <  1237) m_angles->y = local_target_angle.y + 312.5;
					else m_angles->y = local_target_angle.y - 312.5;
					if (delta <  1239) m_angles->y = local_target_angle.y + 313;
					else m_angles->y = local_target_angle.y - 313;
					if (delta <  1241) m_angles->y = local_target_angle.y + 313.5;
					else m_angles->y = local_target_angle.y - 313.5;
					if (delta <  1243) m_angles->y = local_target_angle.y + 314;
					else m_angles->y = local_target_angle.y - 314;
					if (delta <  1245) m_angles->y = local_target_angle.y + 314.5;
					else m_angles->y = local_target_angle.y - 314.5;
					if (delta <  1247) m_angles->y = local_target_angle.y + 315;
					else m_angles->y = local_target_angle.y - 315;
					if (delta <  1249) m_angles->y = local_target_angle.y + 315.5;
					else m_angles->y = local_target_angle.y - 315.5;
					if (delta <  1251) m_angles->y = local_target_angle.y + 316;
					else m_angles->y = local_target_angle.y - 316;
					if (delta <  1253) m_angles->y = local_target_angle.y + 316.5;
					else m_angles->y = local_target_angle.y - 316.5;
					if (delta <  1255) m_angles->y = local_target_angle.y + 317;
					else m_angles->y = local_target_angle.y - 317;
					if (delta <  1257) m_angles->y = local_target_angle.y + 317.5;
					else m_angles->y = local_target_angle.y - 317.5;
					if (delta <  1259) m_angles->y = local_target_angle.y + 318;
					else m_angles->y = local_target_angle.y - 318;
					if (delta <  1261) m_angles->y = local_target_angle.y + 318.5;
					else m_angles->y = local_target_angle.y - 318.5;
					if (delta <  1263) m_angles->y = local_target_angle.y + 319;
					else m_angles->y = local_target_angle.y - 319;
					if (delta <  1265) m_angles->y = local_target_angle.y + 319.5;
					else m_angles->y = local_target_angle.y - 319.5;
					if (delta <  1267) m_angles->y = local_target_angle.y + 320;
					else m_angles->y = local_target_angle.y - 320;
					if (delta <  1269) m_angles->y = local_target_angle.y + 320.5;
					else m_angles->y = local_target_angle.y - 320.5;
					if (delta <  1271) m_angles->y = local_target_angle.y + 321;
					else m_angles->y = local_target_angle.y - 321;
					if (delta <  1273) m_angles->y = local_target_angle.y + 321.5;
					else m_angles->y = local_target_angle.y - 321.5;
					if (delta <  1275) m_angles->y = local_target_angle.y + 322;
					else m_angles->y = local_target_angle.y - 322;
					if (delta <  1277) m_angles->y = local_target_angle.y + 322.5;
					else m_angles->y = local_target_angle.y - 322.5;
					if (delta <  1279) m_angles->y = local_target_angle.y + 323;
					else m_angles->y = local_target_angle.y - 323;
					if (delta <  1281) m_angles->y = local_target_angle.y + 323.5;
					else m_angles->y = local_target_angle.y - 323.5;
					if (delta <  1283) m_angles->y = local_target_angle.y + 324;
					else m_angles->y = local_target_angle.y - 324;
					if (delta <  1285) m_angles->y = local_target_angle.y + 324.5;
					else m_angles->y = local_target_angle.y - 324.5;
					if (delta <  1287) m_angles->y = local_target_angle.y + 325;
					else m_angles->y = local_target_angle.y - 325;
					if (delta <  1289) m_angles->y = local_target_angle.y + 325.5;
					else m_angles->y = local_target_angle.y - 325.5;
					if (delta <  1291) m_angles->y = local_target_angle.y + 326;
					else m_angles->y = local_target_angle.y - 326;
					if (delta <  1293) m_angles->y = local_target_angle.y + 326.5;
					else m_angles->y = local_target_angle.y - 326.5;
					if (delta <  1295) m_angles->y = local_target_angle.y + 327;
					else m_angles->y = local_target_angle.y - 327;
					if (delta <  1297) m_angles->y = local_target_angle.y + 327.5;
					else m_angles->y = local_target_angle.y - 327.5;
					if (delta <  1299) m_angles->y = local_target_angle.y + 328;
					else m_angles->y = local_target_angle.y - 328;
					if (delta <  1301) m_angles->y = local_target_angle.y + 328.5;
					else m_angles->y = local_target_angle.y - 328.5;
					if (delta <  1303) m_angles->y = local_target_angle.y + 329;
					else m_angles->y = local_target_angle.y - 329;
					if (delta <  1305) m_angles->y = local_target_angle.y + 329.5;
					else m_angles->y = local_target_angle.y - 329.5;
					if (delta <  1307) m_angles->y = local_target_angle.y + 330;
					else m_angles->y = local_target_angle.y - 330;
					if (delta <  1309) m_angles->y = local_target_angle.y + 330.5;
					else m_angles->y = local_target_angle.y - 330.5;
					if (delta <  1311) m_angles->y = local_target_angle.y + 331;
					else m_angles->y = local_target_angle.y - 331;
					if (delta <  1313) m_angles->y = local_target_angle.y + 331.5;
					else m_angles->y = local_target_angle.y - 331.5;
					if (delta <  1315) m_angles->y = local_target_angle.y + 332;
					else m_angles->y = local_target_angle.y - 332;
					if (delta <  1317) m_angles->y = local_target_angle.y + 332.5;
					else m_angles->y = local_target_angle.y - 332.5;
					if (delta <  1319) m_angles->y = local_target_angle.y + 333;
					else m_angles->y = local_target_angle.y - 333;
					if (delta <  1321) m_angles->y = local_target_angle.y + 333.5;
					else m_angles->y = local_target_angle.y - 333.5;
					if (delta <  1323) m_angles->y = local_target_angle.y + 334;
					else m_angles->y = local_target_angle.y - 334;
					if (delta <  1325) m_angles->y = local_target_angle.y + 334.5;
					else m_angles->y = local_target_angle.y - 334.5;
					if (delta <  1327) m_angles->y = local_target_angle.y + 335;
					else m_angles->y = local_target_angle.y - 335;
					if (delta <  1329) m_angles->y = local_target_angle.y + 335.5;
					else m_angles->y = local_target_angle.y - 335.5;
					if (delta <  1331) m_angles->y = local_target_angle.y + 336;
					else m_angles->y = local_target_angle.y - 336;
					if (delta <  1333) m_angles->y = local_target_angle.y + 336.5;
					else m_angles->y = local_target_angle.y - 336.5;
					if (delta <  1335) m_angles->y = local_target_angle.y + 337;
					else m_angles->y = local_target_angle.y - 337;
					if (delta <  1337) m_angles->y = local_target_angle.y + 337.5;
					else m_angles->y = local_target_angle.y - 337.5;
					if (delta <  1339) m_angles->y = local_target_angle.y + 338;
					else m_angles->y = local_target_angle.y - 338;
					if (delta <  1341) m_angles->y = local_target_angle.y + 338.5;
					else m_angles->y = local_target_angle.y - 338.5;
					if (delta <  1343) m_angles->y = local_target_angle.y + 339;
					else m_angles->y = local_target_angle.y - 339;
					if (delta <  1345) m_angles->y = local_target_angle.y + 339.5;
					else m_angles->y = local_target_angle.y - 339.5;
					if (delta <  1347) m_angles->y = local_target_angle.y + 340;
					else m_angles->y = local_target_angle.y - 340;
					if (delta <  1349) m_angles->y = local_target_angle.y + 340.5;
					else m_angles->y = local_target_angle.y - 340.5;
					if (delta <  1351) m_angles->y = local_target_angle.y + 341;
					else m_angles->y = local_target_angle.y - 341;
					if (delta <  1353) m_angles->y = local_target_angle.y + 341.5;
					else m_angles->y = local_target_angle.y - 341.5;
					if (delta <  1355) m_angles->y = local_target_angle.y + 342;
					else m_angles->y = local_target_angle.y - 342;
					if (delta <  1357) m_angles->y = local_target_angle.y + 342.5;
					else m_angles->y = local_target_angle.y - 342.5;
					if (delta <  1359) m_angles->y = local_target_angle.y + 343;
					else m_angles->y = local_target_angle.y - 343;
					if (delta <  1361) m_angles->y = local_target_angle.y + 343.5;
					else m_angles->y = local_target_angle.y - 343.5;
					if (delta <  1363) m_angles->y = local_target_angle.y + 344;
					else m_angles->y = local_target_angle.y - 344;
					if (delta <  1365) m_angles->y = local_target_angle.y + 344.5;
					else m_angles->y = local_target_angle.y - 344.5;
					if (delta <  1367) m_angles->y = local_target_angle.y + 345;
					else m_angles->y = local_target_angle.y - 345;
					if (delta <  1369) m_angles->y = local_target_angle.y + 345.5;
					else m_angles->y = local_target_angle.y - 345.5;
					if (delta <  1371) m_angles->y = local_target_angle.y + 346;
					else m_angles->y = local_target_angle.y - 346;
					if (delta <  1373) m_angles->y = local_target_angle.y + 346.5;
					else m_angles->y = local_target_angle.y - 346.5;
					if (delta <  1375) m_angles->y = local_target_angle.y + 347;
					else m_angles->y = local_target_angle.y - 347;
					if (delta <  1377) m_angles->y = local_target_angle.y + 347.5;
					else m_angles->y = local_target_angle.y - 347.5;
					if (delta <  1379) m_angles->y = local_target_angle.y + 348;
					else m_angles->y = local_target_angle.y - 348;
					if (delta <  1381) m_angles->y = local_target_angle.y + 348.5;
					else m_angles->y = local_target_angle.y - 348.5;
					if (delta <  1383) m_angles->y = local_target_angle.y + 349;
					else m_angles->y = local_target_angle.y - 349;
					if (delta <  1385) m_angles->y = local_target_angle.y + 349.5;
					else m_angles->y = local_target_angle.y - 349.5;
					if (delta <  1387) m_angles->y = local_target_angle.y + 350;
					else m_angles->y = local_target_angle.y - 350;
					if (delta <  1389) m_angles->y = local_target_angle.y + 350.5;
					else m_angles->y = local_target_angle.y - 350.5;
					if (delta <  1391) m_angles->y = local_target_angle.y + 351;
					else m_angles->y = local_target_angle.y - 351;
					if (delta <  1393) m_angles->y = local_target_angle.y + 351.5;
					else m_angles->y = local_target_angle.y - 351.5;
					if (delta <  1395) m_angles->y = local_target_angle.y + 352;
					else m_angles->y = local_target_angle.y - 352;
					if (delta <  1397) m_angles->y = local_target_angle.y + 352.5;
					else m_angles->y = local_target_angle.y - 352.5;
					if (delta <  1399) m_angles->y = local_target_angle.y + 353;
					else m_angles->y = local_target_angle.y - 353;
					if (delta <  1401) m_angles->y = local_target_angle.y + 353.5;
					else m_angles->y = local_target_angle.y - 353.5;
					if (delta <  1403) m_angles->y = local_target_angle.y + 354;
					else m_angles->y = local_target_angle.y - 354;
					if (delta <  1405) m_angles->y = local_target_angle.y + 354.5;
					else m_angles->y = local_target_angle.y - 354.5;
					if (delta <  1407) m_angles->y = local_target_angle.y + 355;
					else m_angles->y = local_target_angle.y - 355;
					if (delta <  1409) m_angles->y = local_target_angle.y + 355.5;
					else m_angles->y = local_target_angle.y - 355.5;
					if (delta <  1411) m_angles->y = local_target_angle.y + 356;
					else m_angles->y = local_target_angle.y - 356;
					if (delta <  1413) m_angles->y = local_target_angle.y + 356.5;
					else m_angles->y = local_target_angle.y - 356.5;
					if (delta <  1415) m_angles->y = local_target_angle.y + 357;
					else m_angles->y = local_target_angle.y - 357;
					if (delta <  1417) m_angles->y = local_target_angle.y + 357.5;
					else m_angles->y = local_target_angle.y - 357.5;
					if (delta <  1419) m_angles->y = local_target_angle.y + 358;
					else m_angles->y = local_target_angle.y - 358;
					if (delta <  1421) m_angles->y = local_target_angle.y + 358.5;
					else m_angles->y = local_target_angle.y - 358.5;
					if (delta <  1423) m_angles->y = local_target_angle.y + 359;
					else m_angles->y = local_target_angle.y - 359;
					if (delta <  1425) m_angles->y = local_target_angle.y + 359.5;
					else m_angles->y = local_target_angle.y - 359.5;
					if (delta <  1427) m_angles->y = local_target_angle.y + 360;
					else m_angles->y = local_target_angle.y - 360;
					//else m_angles->y = local_target_angle.y - 268;
					if (delta <  1430) m_angles->y = local_target_angle.y + 212.5;

				}

			}

			if (m_player->resolver_data.has_hit_angle && m_player->resolver_data.missed_shots >= 2) {
				m_player->resolver_data.has_hit_angle = false;
				m_player->resolver_data.last_hit_angle = Vector(0, 0, 0);
			}

			if (m_player->resolver_data.missed_shots >= 8) {
				m_player->resolver_data.has_hit_angle = false;
				m_player->resolver_data.last_hit_angle = Vector(0, 0, 0);
				m_player->resolver_data.shots = 0;
				m_player->resolver_data.missed_shots = 0;
			}

			if (!miscconfig.bAntiUntrusted && m_entity == game::globals.Target)
				m_angles->x = game::globals.Shots % 5 == 4 ? -90.f : 90.f;

			if (m_player->ForcePitch && m_player->ForcePitch_Pitch) {
				float angles[] = { -90.f, 90.f };
				m_angles->x = angles[m_player->ForcePitch_Pitch + 1];
			}

			corrections.push_back(CResolverData(i, *m_angles, *m_entity->GetEyeAngles()));
		}
	}
}

void CResolver::apply_corrections(CUserCmd* m_pcmd) {
	if (m_pEngine->IsInGame() && m_pEngine->IsConnected() && game::localdata.localplayer()) {
		for each (CResolverData current in resolver.corrections) {
			IClientEntity* ent = (IClientEntity*)m_pEntityList->GetClientEntity(current.index);
			if (!ent || ent == game::localdata.localplayer() || ent->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer || !ent->IsAlive()) continue;
			ent->GetEyeAngles()->y = current.realAngles.y;
			ent->GetEyeAngles()->x = current.realAngles.x;
		}
	}
}

void CResolver::StoreFGE(IClientEntity* pEntity)
{
	CResolver::storedanglesFGE = pEntity->GetEyeAngles()->y;
	CResolver::storedlbyFGE = pEntity->GetLowerBodyYaw();
	CResolver::storedsimtimeFGE = pEntity->m_flSimulationTime();
}

void CResolver::StoreThings(IClientEntity* pEntity)
{
	//CResolver::StoredAngles[pEntity->GetIndex()] = pEntity->GetEyeAngles();
	CResolver::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	CResolver::storedsimtime = pEntity->m_flSimulationTime();
	CResolver::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAngles()->y;
	CResolver::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
}

void CResolver::anglestore(IClientEntity * pEntity)
{
	CResolver::badangle[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();

}

void CResolver::StoreExtra(IClientEntity * pEntity)
{
	CResolver::storedpanic[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	CResolver::panicangle[pEntity->GetIndex()] = pEntity->GetEyeAngles()->y - 30;
	CResolver::storedhp[pEntity->GetIndex()] = pEntity->GetHealth();
}

void CResolver::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < m_pEngine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)m_pEntityList->GetClientEntity(x);

		if (!pEntity
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		CResolver::StoreThings(pEntity);
	}
}

void CResolver::LowerBodyYawFix(IClientEntity* pEntity)
{
	if (Menu::Window.RageBotTab.LBYfix.GetState());
	bool NextLBYUpdate();
	{
		static bool wilupdate;
		static float LastLBYUpdateTime = 0.0f;
		float flServerTime = (m_pGlobals->tickcount);
		if (flServerTime >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = flServerTime + 1.125f;
			wilupdate = true;

		}
		else
		{
			wilupdate = false;
		};

		IClientEntity* LocalPlayer = m_pEntityList->GetClientEntity(m_pEngine->GetLocalPlayer());
		int tick = *(int*)((DWORD)LocalPlayer);
		float servertime = (float)(tick * m_pGlobals->interval_per_tick);
		if (_CRTDBG_MODE_DEBUG)
		{
			printf("%f\n", LocalPlayer->GetLowerBodyYaw());
		}

		if (OldLBY != LocalPlayer->GetLowerBodyYaw())
		{
			LBYBreakerTimer++;
			OldLBY = LocalPlayer->GetLowerBodyYaw();
			bSwitch = !bSwitch;
			LastLBYUpdateTime = servertime;
		}

		if (LocalPlayer)0.1;
		{
			LastLBYUpdateTime = servertime;
			if (false);
		}

		if ((LastLBYUpdateTime + 1.125f - (LATENCY_TIME() * 2.0f) < servertime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
		{
			if (LastLBYUpdateTime + 1.125f - (LATENCY_TIME() * 2.0f) < servertime)
			{
				LastLBYUpdateTime += 1.125f;
			}
			if (true);
		}
		if (false);
	}
	float GetLatency();
	{
		INetChannelInfo *nci = m_pEngine->GetNetChannelInfo();
		if (nci)
		{
			float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		}
		else
		{
			if (true)
			{

			} 0.00f;
		}
	}
	float GetOutgoingLatency();
	{
		INetChannelInfo *nci = m_pEngine->GetNetChannelInfo();
		if (nci)
		{
			float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
			float(OutGoingLatency);
		}
		else
		{
			if (true)
			{

			} 0.00f;
		}
	}
	float GetIncomingLatency();
	{
		INetChannelInfo *nci = m_pEngine->GetNetChannelInfo();
		if (nci)
		{
			float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		}
		else
		{
			if (true)
			{

			} 0.00f;
		}
	}
}

void CResolver::draw_developer_data()
{
	if (resolverconfig.bResolverDebug) {
		int pos_y = 30;
		char buffer_shots[128];
		float shots = game::globals.Shots % 4;
		sprintf_s(buffer_shots, "Shots: %1.0f", shots);
		draw.text(4, pos_y, buffer_shots, draw.fonts.menu_bold, Color(255, 255, 255));
		pos_y += 10;


		char buffer_chokedticks[128];
		float choked_ticks = game::globals.choked_ticks;
		sprintf_s(buffer_chokedticks, "Choked ticks: %1.0f", choked_ticks);
		draw.text(4, pos_y, buffer_chokedticks, draw.fonts.menu_bold, Color(255, 255, 255));
		pos_y += 10;

		if (game::globals.UserCmd) {
			char buffer_realyaw[128];
			static float real_yaw = 0;
			if (!game::globals.SendPacket) real_yaw = game::globals.UserCmd->viewangles.y;
			sprintf_s(buffer_realyaw, "Real yaw: %1.0f", real_yaw);
			draw.text(4, pos_y, buffer_realyaw, draw.fonts.menu_bold, Color(255, 255, 255));
			pos_y += 10;

			char buffer_fakeyaw[128];
			static float fake_yaw = 0;
			if (game::globals.SendPacket) fake_yaw = game::globals.UserCmd->viewangles.y;
			sprintf_s(buffer_fakeyaw, "Fake yaw: %1.0f", fake_yaw);
			draw.text(4, pos_y, buffer_fakeyaw, draw.fonts.menu_bold, Color(255, 255, 255));
			pos_y += 20;
		}
	}
}

void CResolver::OldY(IClientEntity* pEntity)
{
	float flInterval = m_pGlobals->interval_per_tick;

	if (pEntity->GetLowerBodyYaw() > 130 && pEntity->GetLowerBodyYaw() < -130)
	{
		StoreThings(pEntity);
	}
	else
	{
		pEntity->GetLowerBodyYaw() - 100 - rand() % 30;

		if (CResolver::didhitHS && pEntity->GetHealth() > 60)
			pEntity->GetLowerBodyYaw() - 90;
	}


}
void CResolver::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < m_pEngine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)m_pEntityList->GetClientEntity(i);

			if (!pEntity
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;
		}
	}
};

float Normalize(float delta)
{
	float tolerance = 10.f;

	while (delta <= -180) delta += 360;
	while (delta > 180) delta -= 360;
	while (delta <= -90) delta += 270;
	while (delta > 90) delta -= 270;
	while (delta <= -360) delta += 180;
	while (delta > 360) delta -= 180;
	while (delta <= -270) delta += 90;
	while (delta > 270) delta -= 90;

	return delta;
}

const inline float GetDelta(float a, float b) {
	return abs(Normalize(a - b));


}
void CResolver::PitchCorrection(IClientEntity * pEntity, IClientEntity * pLocal)
{
	CUserCmd* pCmd;
	for (int i = 0; i < m_pEngine->GetMaxClients(); ++i)
	{
		IClientEntity* m_Local(); // might be here
		IClientEntity *player = (IClientEntity*)m_pEntityList->GetClientEntity(i);

		if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
			continue;

		if (!player)
			continue;

		if (pLocal)
			continue;
		{
			if (Menu::Window.RageBotTab.pitchcorrection.GetState())
			{
				Vector* eyeAngles = player->GetEyeAngles();
				if (eyeAngles->x < -179.f) eyeAngles->x += 360.f;
				else if (eyeAngles->x > 90.0 || eyeAngles->x < -90.0) eyeAngles->x = 89.f;
				else if (eyeAngles->x > 89.0 && eyeAngles->x < 91.0) eyeAngles->x -= 90.f;
				else if (eyeAngles->x > 179.0 && eyeAngles->x < 181.0) eyeAngles->x -= 180;
				else if (eyeAngles->x > -179.0 && eyeAngles->x < -181.0) eyeAngles->x += 180;
				else if (fabs(eyeAngles->x) == 0) eyeAngles->x = std::copysign(89.0f, eyeAngles->x);
			}
		}
	}
}