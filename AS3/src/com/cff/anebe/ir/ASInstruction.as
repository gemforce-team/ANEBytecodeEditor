package com.cff.anebe.ir
{
    /**
     * An AS3 AVM instruction
     * @author Chris
     */
    public class ASInstruction
    {
        /** The OPCode for this instruction. A list of recognized ones is listed in this class as OP_* */
        public var opcode:String;

        /**
         * A list of the arguments this instruction takes.
         * The exact format of each OPCode is not currently listed here, but the valid types inside this array are int, uint,
         * Number, String, ASNamespace, ASMultiname, ASClass, ASMethod, ASInstruction, or Vector.&lt;ASInstruction&gt;.
         * Any ASInstruction contained in this array must be a reference to an instruction in the body of the method in which this instruction appears.
         */
        public var args:Array;

        public static const OP_raw:String = "db";
        public static const OP_bkpt:String = "bkpt";
        public static const OP_nop:String = "nop";
        public static const OP_throw:String = "throw";
        public static const OP_getsuper:String = "getsuper";
        public static const OP_setsuper:String = "setsuper";
        public static const OP_dxns:String = "dxns";
        public static const OP_dxnslate:String = "dxnslate";
        public static const OP_kill:String = "kill";
        public static const OP_label:String = "label";
        public static const OP_ifnlt:String = "ifnlt";
        public static const OP_ifnle:String = "ifnle";
        public static const OP_ifngt:String = "ifngt";
        public static const OP_ifnge:String = "ifnge";
        public static const OP_jump:String = "jump";
        public static const OP_iftrue:String = "iftrue";
        public static const OP_iffalse:String = "iffalse";
        public static const OP_ifeq:String = "ifeq";
        public static const OP_ifne:String = "ifne";
        public static const OP_iflt:String = "iflt";
        public static const OP_ifle:String = "ifle";
        public static const OP_ifgt:String = "ifgt";
        public static const OP_ifge:String = "ifge";
        public static const OP_ifstricteq:String = "ifstricteq";
        public static const OP_ifstrictne:String = "ifstrictne";
        public static const OP_lookupswitch:String = "lookupswitch";
        public static const OP_pushwith:String = "pushwith";
        public static const OP_popscope:String = "popscope";
        public static const OP_nextname:String = "nextname";
        public static const OP_hasnext:String = "hasnext";
        public static const OP_pushnull:String = "pushnull";
        public static const OP_pushundefined:String = "pushundefined";
        public static const OP_pushuninitialized:String = "pushuninitialized";
        public static const OP_nextvalue:String = "nextvalue";
        public static const OP_pushbyte:String = "pushbyte";
        public static const OP_pushshort:String = "pushshort";
        public static const OP_pushtrue:String = "pushtrue";
        public static const OP_pushfalse:String = "pushfalse";
        public static const OP_pushnan:String = "pushnan";
        public static const OP_pop:String = "pop";
        public static const OP_dup:String = "dup";
        public static const OP_swap:String = "swap";
        public static const OP_pushstring:String = "pushstring";
        public static const OP_pushint:String = "pushint";
        public static const OP_pushuint:String = "pushuint";
        public static const OP_pushdouble:String = "pushdouble";
        public static const OP_pushscope:String = "pushscope";
        public static const OP_pushnamespace:String = "pushnamespace";
        public static const OP_hasnext2:String = "hasnext2";
        public static const OP_pushdecimal:String = "pushdecimal";
        public static const OP_pushdnan:String = "pushdnan";
        public static const OP_li8:String = "li8";
        public static const OP_li16:String = "li16";
        public static const OP_li32:String = "li32";
        public static const OP_lf32:String = "lf32";
        public static const OP_lf64:String = "lf64";
        public static const OP_si8:String = "si8";
        public static const OP_si16:String = "si16";
        public static const OP_si32:String = "si32";
        public static const OP_sf32:String = "sf32";
        public static const OP_sf64:String = "sf64";
        public static const OP_newfunction:String = "newfunction";
        public static const OP_call:String = "call";
        public static const OP_construct:String = "construct";
        public static const OP_callmethod:String = "callmethod";
        public static const OP_callstatic:String = "callstatic";
        public static const OP_callsuper:String = "callsuper";
        public static const OP_callproperty:String = "callproperty";
        public static const OP_returnvoid:String = "returnvoid";
        public static const OP_returnvalue:String = "returnvalue";
        public static const OP_constructsuper:String = "constructsuper";
        public static const OP_constructprop:String = "constructprop";
        public static const OP_callsuperid:String = "callsuperid";
        public static const OP_callproplex:String = "callproplex";
        public static const OP_callinterface:String = "callinterface";
        public static const OP_callsupervoid:String = "callsupervoid";
        public static const OP_callpropvoid:String = "callpropvoid";
        public static const OP_sxi1:String = "sxi1";
        public static const OP_sxi8:String = "sxi8";
        public static const OP_sxi16:String = "sxi16";
        public static const OP_applytype:String = "applytype";
        public static const OP_newobject:String = "newobject";
        public static const OP_newarray:String = "newarray";
        public static const OP_newactivation:String = "newactivation";
        public static const OP_newclass:String = "newclass";
        public static const OP_getdescendants:String = "getdescendants";
        public static const OP_newcatch:String = "newcatch";
        public static const OP_deldescendants:String = "deldescendants";
        public static const OP_findpropstrict:String = "findpropstrict";
        public static const OP_findproperty:String = "findproperty";
        public static const OP_finddef:String = "finddef";
        public static const OP_getlex:String = "getlex";
        public static const OP_setproperty:String = "setproperty";
        public static const OP_getlocal:String = "getlocal";
        public static const OP_setlocal:String = "setlocal";
        public static const OP_getglobalscope:String = "getglobalscope";
        public static const OP_getscopeobject:String = "getscopeobject";
        public static const OP_getproperty:String = "getproperty";
        public static const OP_getpropertylate:String = "getpropertylate";
        public static const OP_initproperty:String = "initproperty";
        public static const OP_setpropertylate:String = "setpropertylate";
        public static const OP_deleteproperty:String = "deleteproperty";
        public static const OP_deletepropertylate:String = "deletepropertylate";
        public static const OP_getslot:String = "getslot";
        public static const OP_setslot:String = "setslot";
        public static const OP_getglobalslot:String = "getglobalslot";
        public static const OP_setglobalslot:String = "setglobalslot";
        public static const OP_convert_s:String = "convert_s";
        public static const OP_esc_xelem:String = "esc_xelem";
        public static const OP_esc_xattr:String = "esc_xattr";
        public static const OP_convert_i:String = "convert_i";
        public static const OP_convert_u:String = "convert_u";
        public static const OP_convert_d:String = "convert_d";
        public static const OP_convert_b:String = "convert_b";
        public static const OP_convert_o:String = "convert_o";
        public static const OP_checkfilter:String = "checkfilter";
        public static const OP_convert_m:String = "convert_m";
        public static const OP_convert_m_p:String = "convert_m_p";
        public static const OP_coerce:String = "coerce";
        public static const OP_coerce_b:String = "coerce_b";
        public static const OP_coerce_a:String = "coerce_a";
        public static const OP_coerce_i:String = "coerce_i";
        public static const OP_coerce_d:String = "coerce_d";
        public static const OP_coerce_s:String = "coerce_s";
        public static const OP_astype:String = "astype";
        public static const OP_astypelate:String = "astypelate";
        public static const OP_coerce_u:String = "coerce_u";
        public static const OP_coerce_o:String = "coerce_o";
        public static const OP_negate_p:String = "negate_p";
        public static const OP_negate:String = "negate";
        public static const OP_increment:String = "increment";
        public static const OP_inclocal:String = "inclocal";
        public static const OP_decrement:String = "decrement";
        public static const OP_declocal:String = "declocal";
        public static const OP_typeof:String = "typeof";
        public static const OP_not:String = "not";
        public static const OP_bitnot:String = "bitnot";
        public static const OP_concat:String = "concat";
        public static const OP_add_d:String = "add_d";
        public static const OP_increment_p:String = "increment_p";
        public static const OP_inclocal_p:String = "inclocal_p";
        public static const OP_decrement_p:String = "decrement_p";
        public static const OP_declocal_p:String = "declocal_p";
        public static const OP_add:String = "add";
        public static const OP_subtract:String = "subtract";
        public static const OP_multiply:String = "multiply";
        public static const OP_divide:String = "divide";
        public static const OP_modulo:String = "modulo";
        public static const OP_lshift:String = "lshift";
        public static const OP_rshift:String = "rshift";
        public static const OP_urshift:String = "urshift";
        public static const OP_bitand:String = "bitand";
        public static const OP_bitor:String = "bitor";
        public static const OP_bitxor:String = "bitxor";
        public static const OP_equals:String = "equals";
        public static const OP_strictequals:String = "strictequals";
        public static const OP_lessthan:String = "lessthan";
        public static const OP_lessequals:String = "lessequals";
        public static const OP_greaterthan:String = "greaterthan";
        public static const OP_greaterequals:String = "greaterequals";
        public static const OP_instanceof:String = "instanceof";
        public static const OP_istype:String = "istype";
        public static const OP_istypelate:String = "istypelate";
        public static const OP_in:String = "in";
        public static const OP_add_p:String = "add_p";
        public static const OP_subtract_p:String = "subtract_p";
        public static const OP_multiply_p:String = "multiply_p";
        public static const OP_divide_p:String = "divide_p";
        public static const OP_modulo_p:String = "modulo_p";
        public static const OP_increment_i:String = "increment_i";
        public static const OP_decrement_i:String = "decrement_i";
        public static const OP_inclocal_i:String = "inclocal_i";
        public static const OP_declocal_i:String = "declocal_i";
        public static const OP_negate_i:String = "negate_i";
        public static const OP_add_i:String = "add_i";
        public static const OP_subtract_i:String = "subtract_i";
        public static const OP_multiply_i:String = "multiply_i";
        public static const OP_getlocal0:String = "getlocal0";
        public static const OP_getlocal1:String = "getlocal1";
        public static const OP_getlocal2:String = "getlocal2";
        public static const OP_getlocal3:String = "getlocal3";
        public static const OP_setlocal0:String = "setlocal0";
        public static const OP_setlocal1:String = "setlocal1";
        public static const OP_setlocal2:String = "setlocal2";
        public static const OP_setlocal3:String = "setlocal3";
        public static const OP_debug:String = "debug";
        public static const OP_debugline:String = "debugline";
        public static const OP_debugfile:String = "debugfile";
        public static const OP_bkptline:String = "bkptline";
        public static const OP_timestamp:String = "timestamp";

        /**
         * Builds an instruction from scratch. null OPCode is interpreted as OP_nop
         * @param opcode OPCode of the new instruction
         * @param args Arguments to this instruction
         */
        public function ASInstruction(opcode:String = null, args:Array = null)
        {
            if (opcode != null)
            {
                this.opcode = opcode;
            }
            else
            {
                this.opcode = OP_nop;
            }
            this.args = args;
        }

        /**
         * If this is a local get or set instruction, returns the index of the local. Otherwise throws an error.
         * @return Index of the local this instruction manipulates.
         */
        public function localIndex():uint
        {
            switch (opcode)
            {
                case OP_getlocal:
                case OP_setlocal:
                    return args[0] as uint;
                case OP_getlocal0:
                case OP_setlocal0:
                    return 0;
                case OP_getlocal1:
                case OP_setlocal1:
                    return 1;
                case OP_getlocal2:
                case OP_setlocal2:
                    return 2;
                case OP_getlocal3:
                case OP_setlocal3:
                    return 3;

                default:
                    throw new Error("Cannot get the local index of an instruction that does not get or set a local");
            }
        }

        /**
         * If v is 0-3, the corresponding result of GetLocal0-3. Otherwise the result of GetLocal(v).
         */
        public static function EfficientGetLocal(v:uint):ASInstruction
        {
            switch (v)
            {
                case 0:
                    return GetLocal0();
                case 1:
                    return GetLocal1();
                case 2:
                    return GetLocal2();
                case 3:
                    return GetLocal3();
                default:
                    return GetLocal(v);
            }
        }

        /**
         * If v is 0-3, the corresponding result of SetLocal0-3. Otherwise the result of SetLocal(v).
         */
        public static function EfficientSetLocal(v:uint):ASInstruction
        {
            switch (v)
            {
                case 0:
                    return SetLocal0();
                case 1:
                    return SetLocal1();
                case 2:
                    return SetLocal2();
                case 3:
                    return SetLocal3();
                default:
                    return SetLocal(v);
            }
        }

        /**
         * If this instruction is a debug instruction, returns true. Otherwise false.
         */
        public function isDebug():Boolean
        {
            return opcode == ASInstruction.OP_debug || opcode == ASInstruction.OP_debugfile || opcode == ASInstruction.OP_debugline;
        }

        /**
         * If this instruction is a getlocal instruction, returns true. Otherwise false.
         */
        public function isGetLocal():Boolean
        {
            return opcode == OP_getlocal || opcode == OP_getlocal0 || opcode == OP_getlocal1 || opcode == OP_getlocal2 || opcode == OP_getlocal3;
        }

        /**
         * If this instruction is a setlocal instruction, returns true. Otherwise false.
         */
        public function isSetLocal():Boolean
        {
            return opcode == OP_setlocal || opcode == OP_setlocal0 || opcode == OP_setlocal1 || opcode == OP_setlocal2 || opcode == OP_setlocal3;
        }

        public static function Raw(v:uint):ASInstruction
        {
            return new ASInstruction(OP_raw, [v]);
        }
        public static function Bkpt():ASInstruction
        {
            return new ASInstruction(OP_bkpt);
        }
        public static function Nop():ASInstruction
        {
            return new ASInstruction(OP_nop);
        }
        public static function Throw():ASInstruction
        {
            return new ASInstruction(OP_throw);
        }
        public static function GetSuper(propName:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_getsuper, [propName]);
        }
        public static function SetSuper(propName:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_setsuper, [propName]);
        }
        public static function Dxns(newXMLNamespace:String):ASInstruction
        {
            return new ASInstruction(OP_dxns, [newXMLNamespace]);
        }
        public static function DxnsLate():ASInstruction
        {
            return new ASInstruction(OP_dxnslate);
        }
        public static function Kill(slot:uint):ASInstruction
        {
            return new ASInstruction(OP_kill, [slot]);
        }
        public static function Label():ASInstruction
        {
            return new ASInstruction(OP_label);
        }
        public static function Ifnlt(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifnlt, [target]);
        }
        public static function Ifnle(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifnle, [target]);
        }
        public static function Ifngt(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifngt, [target]);
        }
        public static function Ifnge(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifnge, [target]);
        }
        public static function Jump(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_jump, [target]);
        }
        public static function IfTrue(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_iftrue, [target]);
        }
        public static function IfFalse(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_iffalse, [target]);
        }
        public static function Ifeq(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifeq, [target]);
        }
        public static function Ifne(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifne, [target]);
        }
        public static function Iflt(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_iflt, [target]);
        }
        public static function Ifle(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifle, [target]);
        }
        public static function Ifgt(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifgt, [target]);
        }
        public static function Ifge(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifge, [target]);
        }
        public static function IfStricteq(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifstricteq, [target]);
        }
        public static function IfStrictne(target:ASInstruction):ASInstruction
        {
            return new ASInstruction(OP_ifstrictne, [target]);
        }
        public static function LookupSwitch(Default:ASInstruction, switchTargets:Vector.<ASInstruction>):ASInstruction
        {
            return new ASInstruction(OP_lookupswitch, [Default, switchTargets]);
        }
        public static function PushWith():ASInstruction
        {
            return new ASInstruction(OP_pushwith);
        }
        public static function PopScope():ASInstruction
        {
            return new ASInstruction(OP_popscope);
        }
        public static function NextName():ASInstruction
        {
            return new ASInstruction(OP_nextname);
        }
        public static function HasNext():ASInstruction
        {
            return new ASInstruction(OP_hasnext);
        }
        public static function PushNull():ASInstruction
        {
            return new ASInstruction(OP_pushnull);
        }
        public static function PushUndefined():ASInstruction
        {
            return new ASInstruction(OP_pushundefined);
        }
        public static function NextValue():ASInstruction
        {
            return new ASInstruction(OP_nextvalue);
        }
        public static function PushByte(val:uint):ASInstruction
        {
            return new ASInstruction(OP_pushbyte, [val]);
        }
        public static function PushShort(val:int):ASInstruction
        {
            return new ASInstruction(OP_pushshort, [val]);
        }
        public static function PushTrue():ASInstruction
        {
            return new ASInstruction(OP_pushtrue);
        }
        public static function PushFalse():ASInstruction
        {
            return new ASInstruction(OP_pushfalse);
        }
        public static function PushNaN():ASInstruction
        {
            return new ASInstruction(OP_pushnan);
        }
        public static function Pop():ASInstruction
        {
            return new ASInstruction(OP_pop);
        }
        public static function Dup():ASInstruction
        {
            return new ASInstruction(OP_dup);
        }
        public static function Swap():ASInstruction
        {
            return new ASInstruction(OP_swap);
        }
        public static function PushString(val:String):ASInstruction
        {
            return new ASInstruction(OP_pushstring, [val]);
        }
        public static function PushInt(val:int):ASInstruction
        {
            return new ASInstruction(OP_pushint, [val]);
        }
        public static function PushUInt(val:uint):ASInstruction
        {
            return new ASInstruction(OP_pushuint, [val]);
        }
        public static function PushDouble(val:Number):ASInstruction
        {
            return new ASInstruction(OP_pushdouble, [val]);
        }
        public static function PushScope():ASInstruction
        {
            return new ASInstruction(OP_pushscope);
        }
        public static function PushNamespace(ns:ASNamespace):ASInstruction
        {
            return new ASInstruction(OP_pushnamespace, [ns]);
        }
        public static function HasNext2(objReg:uint, indexReg:uint):ASInstruction
        {
            return new ASInstruction(OP_hasnext2, [objReg, indexReg]);
        }
        public static function Li8():ASInstruction
        {
            return new ASInstruction(OP_li8);
        }
        public static function Li16():ASInstruction
        {
            return new ASInstruction(OP_li16);
        }
        public static function Li32():ASInstruction
        {
            return new ASInstruction(OP_li32);
        }
        public static function Lf32():ASInstruction
        {
            return new ASInstruction(OP_lf32);
        }
        public static function Lf64():ASInstruction
        {
            return new ASInstruction(OP_lf64);
        }
        public static function Si8():ASInstruction
        {
            return new ASInstruction(OP_si8);
        }
        public static function Si16():ASInstruction
        {
            return new ASInstruction(OP_si16);
        }
        public static function Si32():ASInstruction
        {
            return new ASInstruction(OP_si32);
        }
        public static function Sf32():ASInstruction
        {
            return new ASInstruction(OP_sf32);
        }
        public static function Sf64():ASInstruction
        {
            return new ASInstruction(OP_sf64);
        }
        public static function NewFunction(method:ASMethod):ASInstruction
        {
            return new ASInstruction(OP_newfunction, [method]);
        }
        public static function Call(args:uint):ASInstruction
        {
            return new ASInstruction(OP_call, [args]);
        }
        public static function Construct(args:uint):ASInstruction
        {
            return new ASInstruction(OP_construct, [args]);
        }
        public static function CallMethod(index:uint, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callmethod, [index, args]);
        }
        public static function CallStatic(method:ASMethod, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callstatic, [method, args]);
        }
        public static function CallSuper(methodName:ASMultiname, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callsuper, [methodName, args]);
        }
        public static function CallProperty(methodName:ASMultiname, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callproperty, [methodName, args]);
        }
        public static function ReturnVoid():ASInstruction
        {
            return new ASInstruction(OP_returnvoid);
        }
        public static function ReturnValue():ASInstruction
        {
            return new ASInstruction(OP_returnvalue);
        }
        public static function ConstructSuper(args:uint):ASInstruction
        {
            return new ASInstruction(OP_constructsuper, [args]);
        }
        public static function ConstructProp(propName:ASMultiname, args:uint):ASInstruction
        {
            return new ASInstruction(OP_constructprop, [propName, args]);
        }
        public static function CallPropLex(propName:ASMultiname, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callproplex, [propName, args]);
        }
        public static function CallSuperVoid(propName:ASMultiname, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callsupervoid, [propName, args]);
        }
        public static function CallPropVoid(propName:ASMultiname, args:uint):ASInstruction
        {
            return new ASInstruction(OP_callpropvoid, [propName, args]);
        }
        public static function Sxi1():ASInstruction
        {
            return new ASInstruction(OP_sxi1);
        }
        public static function Sxi8():ASInstruction
        {
            return new ASInstruction(OP_sxi8);
        }
        public static function Sxi16():ASInstruction
        {
            return new ASInstruction(OP_sxi16);
        }
        public static function ApplyType(args:uint):ASInstruction
        {
            return new ASInstruction(OP_applytype, [args]);
        }
        public static function NewObject(numProperties:uint):ASInstruction
        {
            return new ASInstruction(OP_newobject, [numProperties]);
        }
        public static function NewArray(numValues:uint):ASInstruction
        {
            return new ASInstruction(OP_newarray, [numValues]);
        }
        public static function NewActivation():ASInstruction
        {
            return new ASInstruction(OP_newactivation);
        }
        public static function NewClass(clazz:ASClass):ASInstruction
        {
            return new ASInstruction(OP_newclass, [clazz]);
        }
        public static function GetDescendants(descendantName:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_getdescendants, [descendantName]);
        }
        public static function NewCatch(exceptionIndex:uint):ASInstruction
        {
            return new ASInstruction(OP_newcatch, [exceptionIndex]);
        }
        public static function FindPropStrict(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_findpropstrict, [name]);
        }
        public static function FindProperty(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_findproperty, [name]);
        }
        public static function FindDef(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_finddef, [name]);
        }
        public static function GetLex(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_getlex, [name]);
        }
        public static function SetProperty(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_setproperty, [name]);
        }
        public static function GetLocal(idx:uint):ASInstruction
        {
            return new ASInstruction(OP_getlocal, [idx]);
        }
        public static function SetLocal(idx:uint):ASInstruction
        {
            return new ASInstruction(OP_setlocal, [idx]);
        }
        public static function GetGlobalScope():ASInstruction
        {
            return new ASInstruction(OP_getglobalscope);
        }
        public static function GetScopeObject(idx:uint):ASInstruction
        {
            return new ASInstruction(OP_getscopeobject, [idx]);
        }
        public static function GetProperty(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_getproperty, [name]);
        }
        public static function GetPropertyLate():ASInstruction
        {
            return new ASInstruction(OP_getpropertylate);
        }
        public static function InitProperty(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_initproperty, [name]);
        }
        public static function SetPropertyLate():ASInstruction
        {
            return new ASInstruction(OP_setpropertylate);
        }
        public static function DeleteProperty(name:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_deleteproperty, [name]);
        }
        public static function DeletePropertyLate():ASInstruction
        {
            return new ASInstruction(OP_deletepropertylate);
        }
        public static function GetSlot(slotId:uint):ASInstruction
        {
            return new ASInstruction(OP_getslot, [slotId]);
        }
        public static function SetSlot(slotId:uint):ASInstruction
        {
            return new ASInstruction(OP_setslot, [slotId]);
        }
        public static function GetGlobalSlot(slotId:uint):ASInstruction
        {
            return new ASInstruction(OP_getglobalslot, [slotId]);
        }
        public static function SetGlobalSlot(slotId:uint):ASInstruction
        {
            return new ASInstruction(OP_setglobalslot, [slotId]);
        }
        public static function Convert_s():ASInstruction
        {
            return new ASInstruction(OP_convert_s);
        }
        public static function Esc_xelem():ASInstruction
        {
            return new ASInstruction(OP_esc_xelem);
        }
        public static function Esc_xattr():ASInstruction
        {
            return new ASInstruction(OP_esc_xattr);
        }
        public static function Convert_i():ASInstruction
        {
            return new ASInstruction(OP_convert_i);
        }
        public static function Convert_u():ASInstruction
        {
            return new ASInstruction(OP_convert_u);
        }
        public static function Convert_d():ASInstruction
        {
            return new ASInstruction(OP_convert_d);
        }
        public static function Convert_b():ASInstruction
        {
            return new ASInstruction(OP_convert_b);
        }
        public static function Convert_o():ASInstruction
        {
            return new ASInstruction(OP_convert_o);
        }
        public static function CheckFilter():ASInstruction
        {
            return new ASInstruction(OP_checkfilter);
        }
        public static function Coerce(type:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_coerce, [type]);
        }
        public static function Coerce_b():ASInstruction
        {
            return new ASInstruction(OP_coerce_b);
        }
        public static function Coerce_a():ASInstruction
        {
            return new ASInstruction(OP_coerce_a);
        }
        public static function Coerce_i():ASInstruction
        {
            return new ASInstruction(OP_coerce_i);
        }
        public static function Coerce_d():ASInstruction
        {
            return new ASInstruction(OP_coerce_d);
        }
        public static function Coerce_s():ASInstruction
        {
            return new ASInstruction(OP_coerce_s);
        }
        public static function AsType(type:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_astype, [type]);
        }
        public static function AsTypeLate():ASInstruction
        {
            return new ASInstruction(OP_astypelate);
        }
        public static function Coerce_u():ASInstruction
        {
            return new ASInstruction(OP_coerce_u);
        }
        public static function Coerce_o():ASInstruction
        {
            return new ASInstruction(OP_coerce_o);
        }
        public static function Negate():ASInstruction
        {
            return new ASInstruction(OP_negate);
        }
        public static function Increment():ASInstruction
        {
            return new ASInstruction(OP_increment);
        }
        public static function IncLocal(localIdx:uint):ASInstruction
        {
            return new ASInstruction(OP_inclocal, [localIdx]);
        }
        public static function Decrement():ASInstruction
        {
            return new ASInstruction(OP_decrement);
        }
        public static function DecLocal(localIdx:uint):ASInstruction
        {
            return new ASInstruction(OP_declocal, [localIdx]);
        }
        public static function TypeOf():ASInstruction
        {
            return new ASInstruction(OP_typeof);
        }
        public static function Not():ASInstruction
        {
            return new ASInstruction(OP_not);
        }
        public static function BitNot():ASInstruction
        {
            return new ASInstruction(OP_bitnot);
        }
        public static function Add():ASInstruction
        {
            return new ASInstruction(OP_add);
        }
        public static function Subtract():ASInstruction
        {
            return new ASInstruction(OP_subtract);
        }
        public static function Multiply():ASInstruction
        {
            return new ASInstruction(OP_multiply);
        }
        public static function Divide():ASInstruction
        {
            return new ASInstruction(OP_divide);
        }
        public static function Modulo():ASInstruction
        {
            return new ASInstruction(OP_modulo);
        }
        public static function Lshift():ASInstruction
        {
            return new ASInstruction(OP_lshift);
        }
        public static function Rshift():ASInstruction
        {
            return new ASInstruction(OP_rshift);
        }
        public static function URshift():ASInstruction
        {
            return new ASInstruction(OP_urshift);
        }
        public static function BitAnd():ASInstruction
        {
            return new ASInstruction(OP_bitand);
        }
        public static function BitOr():ASInstruction
        {
            return new ASInstruction(OP_bitor);
        }
        public static function BitXor():ASInstruction
        {
            return new ASInstruction(OP_bitxor);
        }
        public static function Equals():ASInstruction
        {
            return new ASInstruction(OP_equals);
        }
        public static function StrictEquals():ASInstruction
        {
            return new ASInstruction(OP_strictequals);
        }
        public static function LessThan():ASInstruction
        {
            return new ASInstruction(OP_lessthan);
        }
        public static function LessEquals():ASInstruction
        {
            return new ASInstruction(OP_lessequals);
        }
        public static function GreaterThan():ASInstruction
        {
            return new ASInstruction(OP_greaterthan);
        }
        public static function GreaterEquals():ASInstruction
        {
            return new ASInstruction(OP_greaterequals);
        }
        public static function InstanceOf():ASInstruction
        {
            return new ASInstruction(OP_instanceof);
        }
        public static function IsType(type:ASMultiname):ASInstruction
        {
            return new ASInstruction(OP_istype, [type]);
        }
        public static function IsTypeLate():ASInstruction
        {
            return new ASInstruction(OP_istypelate);
        }
        public static function In():ASInstruction
        {
            return new ASInstruction(OP_in);
        }
        public static function Increment_i():ASInstruction
        {
            return new ASInstruction(OP_increment_i);
        }
        public static function Decrement_i():ASInstruction
        {
            return new ASInstruction(OP_decrement_i);
        }
        public static function IncLocal_i(localIdx:uint):ASInstruction
        {
            return new ASInstruction(OP_inclocal_i);
        }
        public static function DecLocal_i(localIdx:uint):ASInstruction
        {
            return new ASInstruction(OP_declocal_i);
        }
        public static function Negate_i():ASInstruction
        {
            return new ASInstruction(OP_negate_i);
        }
        public static function Add_i():ASInstruction
        {
            return new ASInstruction(OP_add_i);
        }
        public static function Subtract_i():ASInstruction
        {
            return new ASInstruction(OP_subtract_i);
        }
        public static function Multiply_i():ASInstruction
        {
            return new ASInstruction(OP_multiply_i);
        }
        public static function GetLocal0():ASInstruction
        {
            return new ASInstruction(OP_getlocal0);
        }
        public static function GetLocal1():ASInstruction
        {
            return new ASInstruction(OP_getlocal1);
        }
        public static function GetLocal2():ASInstruction
        {
            return new ASInstruction(OP_getlocal2);
        }
        public static function GetLocal3():ASInstruction
        {
            return new ASInstruction(OP_getlocal3);
        }
        public static function SetLocal0():ASInstruction
        {
            return new ASInstruction(OP_setlocal0);
        }
        public static function SetLocal1():ASInstruction
        {
            return new ASInstruction(OP_setlocal1);
        }
        public static function SetLocal2():ASInstruction
        {
            return new ASInstruction(OP_setlocal2);
        }
        public static function SetLocal3():ASInstruction
        {
            return new ASInstruction(OP_setlocal3);
        }
        public static function Debug(infoType:uint, name:String, register:uint, unused:uint = 0):ASInstruction
        {
            return new ASInstruction(OP_debug, [infoType, name, register, unused]);
        }
        public static function DebugLine(lineNum:uint):ASInstruction
        {
            return new ASInstruction(OP_debugline, [lineNum]);
        }
        public static function DebugFile(filename:String):ASInstruction
        {
            return new ASInstruction(OP_debugfile, [filename]);
        }
        public static function BkptLine(lineNum:uint):ASInstruction
        {
            return new ASInstruction(OP_bkptline, [lineNum]);
        }
        public static function Timestamp():ASInstruction
        {
            return new ASInstruction(OP_timestamp);
        }
    }
}
