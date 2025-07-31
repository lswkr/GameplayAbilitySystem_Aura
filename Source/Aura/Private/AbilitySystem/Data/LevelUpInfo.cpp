// Copyright LeeSeungwon


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		//LevelUpInformation[1] = Level 1 information -> 1-indexed로 갈 것임
		
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if (XP >= LevelUpInformation[Level].LevelUpRequirement) //XP가 더 높으면 레벨이 요구하는 XP보다 높으면
		{
			Level++; //레벨 업
		}
		else //아니면
		{
			bSearching = false;//중단
		}
	}
	return Level;
}
