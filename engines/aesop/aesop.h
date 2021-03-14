#ifndef AESOP_H
#define AESOP_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Aesop {

class Console;

class AesopEngine : public Engine {
private:
	Common::RandomSource *_rnd;
public:
	AesopEngine(OSystem *syst);
	~AesopEngine();

	Common::Error run() override;
};

class Console : public GUI::Debugger {
public:
	Console(AesopEngine* vm) {
	}
	virtual ~Console(void) {
	}
};

}

#endif
