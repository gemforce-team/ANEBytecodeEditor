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
         * A list of the arguments this instruction takes. The exact format of each OPCode is not currently listed here,
         * but the valid types inside this array are int, uint, Number, String, ASNamespace, ASMultiname, ASClass, ASMethod, ASLabel, or Vector.&lt;ASLabel&gt;.
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
    }
}
