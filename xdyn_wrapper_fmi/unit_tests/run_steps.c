/* Loads xdyn_fmu from an unzipped FMU, runs 20 steps of 0.5 s of propelled_ship.yml at
   100 rpm and prints the final state: the same program for Linux and for Windows (run under
   wine), so that the two libraries can be compared without a Python on the Windows side.

       run_steps <unzipped fmu directory> <library>
*/
#include "fmi2Functions.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define LOAD(path) ((void*)LoadLibraryA(path))
#define SYMBOL(lib, name) ((void*)GetProcAddress((HMODULE)(lib), name))
#else
#include <dlfcn.h>
#define LOAD(path) dlopen(path, RTLD_NOW | RTLD_LOCAL)
#define SYMBOL(lib, name) dlsym(lib, name)
#endif

static void logger(fmi2ComponentEnvironment env, fmi2String instance, fmi2Status status,
                   fmi2String category, fmi2String message, ...)
{
    (void)env;
    (void)category;
    va_list args;
    va_start(args, message);
    fprintf(stderr, "[%s %d] ", instance, (int)status);
    vfprintf(stderr, message, args);
    fputc('\n', stderr);
    va_end(args);
}

#define GET(type, name)                                                                       \
    type name = (type)SYMBOL(lib, #name);                                                      \
    if (!name)                                                                                 \
    {                                                                                          \
        fprintf(stderr, "no %s in %s\n", #name, argv[2]);                                      \
        return 1;                                                                              \
    }

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: run_steps <unzipped fmu> <library>\n");
        return 2;
    }
    void* lib = LOAD(argv[2]);
    if (!lib)
    {
        fprintf(stderr, "cannot load %s\n", argv[2]);
        return 1;
    }
    GET(fmi2InstantiateTYPE*, fmi2Instantiate)
    GET(fmi2SetupExperimentTYPE*, fmi2SetupExperiment)
    GET(fmi2EnterInitializationModeTYPE*, fmi2EnterInitializationMode)
    GET(fmi2ExitInitializationModeTYPE*, fmi2ExitInitializationMode)
    GET(fmi2SetRealTYPE*, fmi2SetReal)
    GET(fmi2GetRealTYPE*, fmi2GetReal)
    GET(fmi2DoStepTYPE*, fmi2DoStep)
    GET(fmi2FreeInstanceTYPE*, fmi2FreeInstance)

    char resources[4096];
    snprintf(resources, sizeof resources, "file:///%s/resources", argv[1]);
    const fmi2CallbackFunctions callbacks = {logger, NULL, NULL, NULL, NULL};
    fmi2Component c = fmi2Instantiate("ship", fmi2CoSimulation, "", resources, &callbacks,
                                      fmi2False, fmi2False);
    if (!c) return 1;
    /* propeller(rpm), propeller(P/D): the model's two commands, value references 1000 and 1001 */
    const fmi2ValueReference commands[2] = {1000, 1001};
    const fmi2Real values[2] = {100 * 2 * 3.14159265358979323846 / 60, 1.064935};
    if (fmi2SetupExperiment(c, fmi2False, 0, 0, fmi2False, 0) != fmi2OK
        || fmi2EnterInitializationMode(c) != fmi2OK || fmi2ExitInitializationMode(c) != fmi2OK
        || fmi2SetReal(c, commands, 2, values) != fmi2OK)
        return 1;
    for (int k = 0; k < 20; ++k)
        if (fmi2DoStep(c, k * 0.5, 0.5, fmi2True) != fmi2OK) return 1;
    fmi2ValueReference states[13];
    fmi2Real state[13];
    for (unsigned i = 0; i < 13; ++i) states[i] = 1 + i;
    if (fmi2GetReal(c, states, 13, state) != fmi2OK) return 1;
    for (unsigned i = 0; i < 13; ++i) printf("%.17g\n", state[i]);
    fmi2FreeInstance(c);
    return 0;
}
