#include "MegaJam.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogJam);

class FMegaJamModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		// The packaged smoke test looks for this line.
		UE_LOG(LogJam, Log, TEXT("MegaJam module started"));
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FMegaJamModule, MegaJam, "MegaJam");
