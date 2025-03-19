#include "CGameMode.h"

ACGameMode::ACGameMode()
{
	ConstructorHelpers::FClassFinder<APawn> pawn(L"/Script/Engine.Blueprint'/Game/Blueprints/BP_CVRCharacter.BP_CVRCharacter_C'");
	if (pawn.Succeeded())
		DefaultPawnClass = pawn.Class;

}
