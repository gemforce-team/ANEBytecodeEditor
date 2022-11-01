#include "ANEBytecodeEditor.hpp"
#include "ANEFunctions.hpp"
#include "BytecodeEditor.hpp"
#include "utils/ANEUtils.hpp"

#include <array>
#include <string_view>
#include <vector>

using namespace std::literals::string_view_literals;

void ContextInitializer(void*, const uint8_t* ctxTypeRaw, FREContext ctx,
    uint32_t* numFunctions, const FRENamedFunction** functions)
{
    std::string_view ctxType = reinterpret_cast<const char*>(ctxTypeRaw);

#define TZA TransparentZeroArg
#define BE BytecodeEditor

    if (ctxType == "BytecodeEditor"sv)
    {
        BytecodeEditor* editorForContext  = new BytecodeEditor(ctx);
        FRENamedFunction* ANEBE_functions = new FRENamedFunction[]{
            {(const uint8_t*)"Disassemble",          editorForContext, &TZA<&BE::disassemble>         },
            {(const uint8_t*)"Assemble",             editorForContext, &Assemble<&BE::assemble>       },
            {(const uint8_t*)"PartialAssemble",      editorForContext, &Assemble<&BE::partialAssemble>},
            {(const uint8_t*)"FinishAssemble",       editorForContext, &TZA<&BE::finishAssemble>      },
            {(const uint8_t*)"DisassembleAsync",     editorForContext, &TZA<&BE::disassembleAsync>    },
            {(const uint8_t*)"AssembleAsync",        editorForContext, &Assemble<&BE::assembleAsync>  },
            {(const uint8_t*)"PartialAssembleAsync", editorForContext,
             &Assemble<&BE::partialAssembleAsync>                                                     },
            {(const uint8_t*)"FinishAssembleAsync",  editorForContext,
             &TZA<&BE::finishAssembleAsync>                                                           },
            {(const uint8_t*)"AsyncTaskResult",      editorForContext, &TZA<&BE::taskResult>          },
            {(const uint8_t*)"SetCurrentSWF",        editorForContext, &SetCurrentSWF                 },
            {(const uint8_t*)"Cleanup",              editorForContext, &Cleanup                       },
            {(const uint8_t*)"GetClass",             editorForContext, &GetClass                      },
        };
        *functions    = ANEBE_functions;
        *numFunctions = 12;
        FRESetContextNativeData(ctx, (void*)ANEBE_functions);
    }
    else if (ctxType == "Class"sv)
    {
        FRENamedFunction* CLASS_functions = new FRENamedFunction[]{
            {(const uint8_t*)"GetStaticTrait",        classPointerHelper, &ASClass::GetStaticTrait   },
            {(const uint8_t*)"SetStaticTrait",        classPointerHelper, &ASClass::SetStaticTrait   },
            {(const uint8_t*)"DeleteStaticTrait",     classPointerHelper, &ASClass::DeleteStaticTrait},
            {(const uint8_t*)"GetInstanceTrait",      classPointerHelper, &ASClass::GetInstanceTrait },
            {(const uint8_t*)"SetInstanceTrait",      classPointerHelper, &ASClass::SetInstanceTrait },
            {(const uint8_t*)"DeleteInstanceTrait",   classPointerHelper,
             &ASClass::DeleteInstanceTrait                                                           },
            {(const uint8_t*)"GetStaticConstructor",  classPointerHelper,
             &ASClass::GetStaticConstructor                                                          },
            {(const uint8_t*)"SetStaticConstructor",  classPointerHelper,
             &ASClass::SetStaticConstructor                                                          },
            {(const uint8_t*)"GetConstructor",        classPointerHelper, &ASClass::GetConstructor   },
            {(const uint8_t*)"SetConstructor",        classPointerHelper, &ASClass::SetConstructor   },
            {(const uint8_t*)"GetInterfaces",         classPointerHelper, &ASClass::GetInterfaces    },
            {(const uint8_t*)"SetInterfaces",         classPointerHelper, &ASClass::SetInterfaces    },
            {(const uint8_t*)"GetSuperclass",         classPointerHelper, &ASClass::GetSuperclass    },
            {(const uint8_t*)"SetSuperclass",         classPointerHelper, &ASClass::SetSuperclass    },
            {(const uint8_t*)"GetFlags",              classPointerHelper, &ASClass::GetFlags         },
            {(const uint8_t*)"SetFlags",              classPointerHelper, &ASClass::SetFlags         },
            {(const uint8_t*)"GetProtectedNamespace", classPointerHelper,
             &ASClass::GetProtectedNamespace                                                         },
            {(const uint8_t*)"SetProtectedNamespace", classPointerHelper,
             &ASClass::SetProtectedNamespace                                                         },
            {(const uint8_t*)"ConvertClassHelper",    classPointerHelper,
             &ASClass::ConvertClassHelper                                                            }
        };
        classPointerHelper = nullptr;
        *functions         = CLASS_functions;
        *numFunctions      = 19;
        FRESetContextNativeData(ctx, (void*)CLASS_functions);
    }
}

void ContextFinalizer(FREContext ctx)
{
    void* d;
    FREGetContextNativeData(ctx, &d);
    delete[] static_cast<FRENamedFunction*>(d);
}

extern "C" EXPORT void ExtInitializerANEBytecodeEditor(void** extDataToSet,
    FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet)
{
    *extDataToSet        = nullptr;
    *ctxInitializerToSet = &ContextInitializer;
    *ctxFinalizerToSet   = &ContextFinalizer;
}

extern "C" EXPORT void ExtFinalizerANEBytecodeEditor(void*) {}
