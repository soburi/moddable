#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xsAll.h"
#include "xs.h"
#include "mc.xs.h"
#include "mc.defines.h"

static void fxLogStage(const char* stage)
{
        if (stage)
                fprintf(stderr, "[wamr] %s\n", stage);
}

static txMachine* gxMachine = NULL;

static const char* fxApplicationName(void)
{
#ifdef PIU_DOT_SIGNATURE
	const char* signature = PIU_DOT_SIGNATURE;
	const char* dot = strrchr(signature, '.');
	if (dot && dot[1])
		return dot + 1;
	return signature;
#else
	return "wamr";
#endif
}

static txMachine* fxCreateHeadlessMachine(void* archive)
{
        fxLogStage("fxCreateHeadlessMachine begin");
        txPreparation* preparation = xsPreparation();
        txMachine* machine = fxPrepareMachine(NULL, preparation, (txString)fxApplicationName(), NULL, archive);
        if (machine)
                fxLogStage("fxCreateHeadlessMachine success");
        else
                fxLogStage("fxCreateHeadlessMachine failed");
        return machine;
}

static void fxRunDefaultModule(txMachine* the)
{
        fxLogStage("fxRunDefaultModule begin");
        xsBeginHost(the);
        {
                xsVars(1);
                xsVar(0) = xsImportNow(((txPreparation *)xsPreparationAndCreation(NULL))->main);
                xsVar(0) = xsGet(xsVar(0), xsID_default);
                if (xsTest(xsVar(0)) && xsIsInstanceOf(xsVar(0), xsFunctionPrototype)) {
                        fxLogStage("fxRunDefaultModule calling default export");
                        xsCallFunction0(xsVar(0), xsGlobal);
                        fxLogStage("fxRunDefaultModule default export returned");
                }
                else {
                        fxLogStage("fxRunDefaultModule no callable default export");
                }
        }
        xsEndHost(the);
        fxLogStage("fxRunDefaultModule end");
}

static void fxDrainPromiseJobs(txMachine* the)
{
        if (!the) {
                fxLogStage("fxDrainPromiseJobs skipped (no machine)");
                return;
        }
        fxLogStage("fxDrainPromiseJobs begin");
        xsBeginHost(the);
        fxRunPromiseJobs(the);
        xsEndHost(the);
        fxLogStage("fxDrainPromiseJobs end");
}

static void fxDisposeMachine(void)
{
	if (!gxMachine)
		return;
	fxDrainPromiseJobs(gxMachine);
	fxDeleteMachine(gxMachine);
	gxMachine = NULL;
}

void fxAbort(xsMachine* the, int status)
{
	fprintf(stderr, "[wamr] fxAbort status=%d message=%s\n", status, fxAbortString(status));
	fxExitToHost(the);
}

int fxMainIdle(void)
{
	fxDrainPromiseJobs(gxMachine);
	return 0;
}

void* fxMainLaunch(int width, int height, void* archive)
{
        (void)width;
        (void)height;
        fxLogStage("fxMainLaunch begin");
        if (gxMachine)
                return gxMachine;
        gxMachine = fxCreateHeadlessMachine(archive);
        if (!gxMachine) {
                fprintf(stderr, "[wamr] fxMainLaunch failed\n");
                return NULL;
        }
        fxRunDefaultModule(gxMachine);
        fxDrainPromiseJobs(gxMachine);
        fxLogStage("fxMainLaunch end");
        return gxMachine;
}

int fxMainTouch(int kind, int index, int x, int y, double when)
{
	(void)kind;
	(void)index;
	(void)x;
	(void)y;
	(void)when;
	return 0;
}

int fxMainQuit(void)
{
        fxDisposeMachine();
        return 0;
}

int main(int argc, char* argv[])
{
        (void)argc;
        (void)argv;
        printf("[wamr] main start\n");
        if (!fxMainLaunch(0, 0, NULL)) {
                fprintf(stderr, "[wamr] application launch failed\n");
                return 1;
        }
        fxMainIdle();
        fxMainQuit();
        printf("[wamr] main exit\n");
        return 0;
}
