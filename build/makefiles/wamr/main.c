#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xsAll.h"
#include "xs.h"
#include "mc.xs.h"
#include "mc.defines.h"

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
	txPreparation* preparation = xsPreparation();
	return fxPrepareMachine(NULL, preparation, (txString)fxApplicationName(), NULL, archive);
}

static void fxRunDefaultModule(txMachine* the)
{
	xsBeginHost(the);
	{
		xsVars(1);
		xsVar(0) = xsImportNow(((txPreparation *)xsPreparationAndCreation(NULL))->main);
		xsVar(0) = xsGet(xsVar(0), xsID_default);
		if (xsTest(xsVar(0)) && xsIsInstanceOf(xsVar(0), xsFunctionPrototype))
			xsCallFunction0(xsVar(0), xsGlobal);
	}
	xsEndHost(the);
}

static void fxDrainPromiseJobs(txMachine* the)
{
	if (!the)
		return;
	xsBeginHost(the);
	fxRunPromiseJobs(the);
	xsEndHost(the);
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
	if (gxMachine)
		return gxMachine;
	gxMachine = fxCreateHeadlessMachine(archive);
	if (!gxMachine) {
		fprintf(stderr, "[wamr] fxMainLaunch failed\n");
		return NULL;
	}
	fxRunDefaultModule(gxMachine);
	fxDrainPromiseJobs(gxMachine);
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
