#pragma once
#include "BourrinData.h"
#include "BourrinRuntimeInfosData.h"

struct FAttackTargetData
{
	FString AttackerName;
	FBourrinData Attacker;
	FBourrinRuntimeInfosData AttackerRuntimeInfosData;
	FString TargetName;
	FBourrinData Target;
	FBourrinRuntimeInfosData TargetRuntimeInfosData;
};
