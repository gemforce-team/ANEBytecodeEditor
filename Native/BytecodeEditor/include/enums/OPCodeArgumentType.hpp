#pragma once

enum class OPCodeArgumentType
{
    Unknown,

    ByteLiteral,
    UByteLiteral,
    IntLiteral,
    UIntLiteral,

    Int,
    UInt,
    Double,
    String,
    Namespace,
    Multiname,
    Class,
    Method,

    JumpTarget,
    SwitchDefaultTarget,
    SwitchTargets
};
