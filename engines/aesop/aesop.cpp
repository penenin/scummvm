#include "common/debug.h"
#include "common/error.h"

#include "aesop/aesop.h"

namespace Aesop {

AesopEngine::AesopEngine(OSystem* syst)
	: Engine(syst) {

	_rnd = new Common::RandomSource("aesop");

	debug("AesopEngine::AesopEngine");
}

AesopEngine::~AesopEngine() {
	debug("AesopEngine::~AesopEngine");

	delete _rnd;
}

Common::Error AesopEngine::run() {
	return Common::kNoError;
}

}
