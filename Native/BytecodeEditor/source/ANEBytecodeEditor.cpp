#include "ANEBytecodeEditor.hpp"
#include "ANEFunctions.hpp"
#include "BytecodeEditor.hpp"
#include "utils/ANEFunctionContext.hpp"
#include "utils/ANEUtils.hpp"

#include <array>
#include <string_view>
#include <vector>

using namespace std::literals::string_view_literals;

void ContextInitializer(void*, const uint8_t* ctxTypeRaw, FREContext ctx, uint32_t* numFunctions,
    const FRENamedFunction** functions)
{
    std::string_view ctxType = reinterpret_cast<const char*>(ctxTypeRaw);

    ANEFunctionContext* context = new ANEFunctionContext;

#define TZA TransparentZeroArg
#define BE BytecodeEditor
#define CAV CheckAssemblyValid

    if (ctxType == "BytecodeEditor"sv)
    {
        context->editor    = std::shared_ptr<BytecodeEditor>(new BytecodeEditor(ctx));
        context->functions = std::unique_ptr<FRENamedFunction[]>(new FRENamedFunction[]{
            {(const uint8_t*)"Disassemble",          context, &Disassemble<&BE::disassemble>      },
            {(const uint8_t*)"Assemble",             context, &Assemble<&BE::assemble>            },
            {(const uint8_t*)"PartialAssemble",      context, &Assemble<&BE::partialAssemble>     },
            {(const uint8_t*)"FinishAssemble",       context, &TZA<&BE::finishAssemble>           },
            {(const uint8_t*)"DisassembleAsync",     context, &Disassemble<&BE::disassembleAsync> },
            {(const uint8_t*)"AssembleAsync",        context, &Assemble<&BE::assembleAsync>       },
            {(const uint8_t*)"PartialAssembleAsync", context, &Assemble<&BE::partialAssembleAsync>},
            {(const uint8_t*)"FinishAssembleAsync",  context, &TZA<&BE::finishAssembleAsync>      },
            {(const uint8_t*)"AsyncTaskResult",      context, &TZA<&BE::taskResult>               },
            {(const uint8_t*)"InsertABCToSWF",       context, &InsertABCToSWF                     },
            {(const uint8_t*)"Cleanup",              context, &Cleanup                            },
            {(const uint8_t*)"GetClass",             context, &GetClass                           },
            {(const uint8_t*)"GetScript",            context, &GetScript                          },
            {(const uint8_t*)"CreateScript",         context, &CreateScript                       },
            {(const uint8_t*)"ListClasses",          context, &TZA<&BE::listClasses>              },
            {(const uint8_t*)"ListScripts",          context, &TZA<&BE::listScripts>              },
        });

        *functions    = context->functions.get();
        *numFunctions = 16;
    }
    else if (ctxType == "SWFIntrospector"sv)
    {
        context->editor    = std::shared_ptr<BytecodeEditor>(new BytecodeEditor(ctx));
        context->functions = std::unique_ptr<FRENamedFunction[]>(new FRENamedFunction[]{
            {(const uint8_t*)"BeginIntrospection", context, &Disassemble<&BE::beginIntrospection>},
            {(const uint8_t*)"GetClass",           context, &GetROClass                          },
            {(const uint8_t*)"GetScript",          context, &GetROScript                         },
            {(const uint8_t*)"ListClasses",        context, &TZA<&BE::listClasses>               },
            {(const uint8_t*)"ListScripts",        context, &TZA<&BE::listScripts>               },
        });
        *functions         = context->functions.get();
        *numFunctions      = 5;
    }
    else if (ctxType == "Class"sv)
    {
        context->editor     = nextObjectContext->editor;
        context->functions  = std::unique_ptr<FRENamedFunction[]>(new FRENamedFunction[]{
            {(const uint8_t*)"GetStaticTrait",        context, &CAV<&ASClass::GetStaticTrait>      },
            {(const uint8_t*)"ListStaticTraits",      context, &CAV<&ASClass::ListStaticTraits>    },
            {(const uint8_t*)"SetStaticTrait",        context, &CAV<&ASClass::SetStaticTrait>      },
            {(const uint8_t*)"DeleteStaticTrait",     context, &CAV<&ASClass::DeleteStaticTrait>   },
            {(const uint8_t*)"GetInstanceTrait",      context, &CAV<&ASClass::GetInstanceTrait>    },
            {(const uint8_t*)"ListInstanceTraits",    context, &CAV<&ASClass::ListInstanceTraits>  },
            {(const uint8_t*)"SetInstanceTrait",      context, &CAV<&ASClass::SetInstanceTrait>    },
            {(const uint8_t*)"DeleteInstanceTrait",   context, &CAV<&ASClass::DeleteInstanceTrait> },
            {(const uint8_t*)"GetStaticConstructor",  context, &CAV<&ASClass::GetStaticConstructor>},
            {(const uint8_t*)"SetStaticConstructor",  context, &CAV<&ASClass::SetStaticConstructor>},
            {(const uint8_t*)"GetConstructor",        context, &CAV<&ASClass::GetConstructor>      },
            {(const uint8_t*)"SetConstructor",        context, &CAV<&ASClass::SetConstructor>      },
            {(const uint8_t*)"GetInterfaces",         context, &CAV<&ASClass::GetInterfaces>       },
            {(const uint8_t*)"SetInterfaces",         context, &CAV<&ASClass::SetInterfaces>       },
            {(const uint8_t*)"GetSuperclass",         context, &CAV<&ASClass::GetSuperclass>       },
            {(const uint8_t*)"SetSuperclass",         context, &CAV<&ASClass::SetSuperclass>       },
            {(const uint8_t*)"GetInstanceName",       context, &CAV<&ASClass::GetInstanceName>     },
            {(const uint8_t*)"SetInstanceName",       context, &CAV<&ASClass::SetInstanceName>     },
            {(const uint8_t*)"GetFlags",              context, &CAV<&ASClass::GetFlags>            },
            {(const uint8_t*)"SetFlags",              context, &CAV<&ASClass::SetFlags>            },
            {(const uint8_t*)"GetProtectedNamespace", context,
             &CAV<&ASClass::GetProtectedNamespace>                                                 },
            {(const uint8_t*)"SetProtectedNamespace", context,
             &CAV<&ASClass::SetProtectedNamespace>                                                 },
            {(const uint8_t*)"ConvertClassHelper",    context, &CAV<&ASClass::ConvertClassHelper>  }
        });
        context->objectData = {
            nextObjectContext->objectData->object, context->editor->partialAssembly.get()};
        nextObjectContext = std::nullopt;
        *functions        = context->functions.get();
        *numFunctions     = 21;
        FRESetContextNativeData(ctx, context);
    }
    else if (ctxType == "Script"sv)
    {
        context->editor     = nextObjectContext->editor;
        context->functions  = std::unique_ptr<FRENamedFunction[]>(new FRENamedFunction[]{
            {(const uint8_t*)"GetTrait",       context, &CAV<&ASScript::GetTrait>      },
            {(const uint8_t*)"ListTraits",     context, &CAV<&ASScript::ListTraits>    },
            {(const uint8_t*)"SetTrait",       context, &CAV<&ASScript::SetTrait>      },
            {(const uint8_t*)"DeleteTrait",    context, &CAV<&ASScript::DeleteTrait>   },
            {(const uint8_t*)"GetInitializer", context, &CAV<&ASScript::GetInitializer>},
            {(const uint8_t*)"SetInitializer", context, &CAV<&ASScript::SetInitializer>},
            {(const uint8_t*)"CreateClass",    context, &CAV<&ASScript::CreateClass>   },
        });
        context->objectData = {
            nextObjectContext->objectData->object, context->editor->partialAssembly.get()};
        nextObjectContext = std::nullopt;
        *functions        = context->functions.get();
        *numFunctions     = 7;
        FRESetContextNativeData(ctx, context);
    }
    FRESetContextNativeData(ctx, context);
}

void ContextFinalizer(FREContext ctx)
{
    void* d;
    FREGetContextNativeData(ctx, &d);
    delete static_cast<ANEFunctionContext*>(d);
}

extern "C" EXPORT void ExtInitializerANEBytecodeEditor(void** extDataToSet,
    FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet)
{
    *extDataToSet        = nullptr;
    *ctxInitializerToSet = &ContextInitializer;
    *ctxFinalizerToSet   = &ContextFinalizer;
}

extern "C" EXPORT void ExtFinalizerANEBytecodeEditor(void*) {}
