#include "aesop/aesop.h"
#include "engines/advancedDetector.h"

class AesopMetaEngine : public AdvancedMetaEngine {
public:
	const char* getName() const override {
		return "aesop";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error AesopMetaEngine::createInstance(OSystem* syst, Engine** engine, const ADGameDescription* desc) const {
	*engine = new Aesop::AesopEngine(syst);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(AESOP)
REGISTER_PLUGIN_DYNAMIC(AESOP, PLUGIN_TYPE_ENGINE, AesopMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AESOP, PLUGIN_TYPE_ENGINE, AesopMetaEngine);
#endif
