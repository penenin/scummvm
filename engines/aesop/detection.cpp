#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Aesop {
static const PlainGameDescriptor aesopGames[] = {
	{ "eob3", "Eye of the Beholder III" },
	{ "hack", "Dungeon Hack" },
	{ 0, 0 }
};

static const ADGameDescription gameDescriptions[] = {
	{
		"eob3",
		0,
		AD_ENTRY1("eye.res", "a4ad50b2dfd38e67e2c7c671d4d15624"),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO0(),
	},
	{
		"hack",
		0,
		AD_ENTRY1("hack.res", "67345ba1870656dd54d8c8544954d834"),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO0()
	}
};
}

class AesopMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AesopMetaEngineDetection() : AdvancedMetaEngineDetection(Aesop::gameDescriptions, sizeof(ADGameDescription), Aesop::aesopGames) {
	}

	const char* getEngineId() const override {
		return "aesop";
	}

	const char* getName() const override {
		return "Aesop";
	}

	const char *getOriginalCopyright() const override {
		return "";
	}
};

REGISTER_PLUGIN_STATIC(AESOP_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AesopMetaEngineDetection);
