package com.cff.anebe.ir
{

    /**
     * A method or function callable via AS3 code. May be either a function written in AS3
     * (in which case an ASMethodBody will be present) or a native method (in which case one will not)
     * @author Chris
     */
    public class ASMethod
    {

        /** The parameter types of this method. A null multiname means the any type. */
        public var paramTypes:Vector.<ASMultiname>;

        /** The return type of this method. Null represents the any type. */
        public var returnType:ASMultiname;

        /** The name of this method, as encoded in the SWF */
        public var name:String;

        /** Flags on this method; valid flags are listed as FLAG_* */
        public var flags:Vector.<String>;

        /** Default values for arguments. First default is for parameter number paramTypes.length - options.length. */
        public var options:Vector.<ASValue>;

        /** Parameter names of arguments */
        public var paramNames:Vector.<String>;

        /** The method body of a non-native method */
        public var body:ASMethodBody;

        public static const FLAG_NEED_ARGUMENTS:String = "NEED_ARGUMENTS";
        public static const FLAG_NEED_ACTIVATION:String = "NEED_ACTIVATION";
        public static const FLAG_NEED_REST:String = "NEED_REST";
        public static const FLAG_HAS_OPTIONAL:String = "HAS_OPTIONAL";
        public static const FLAG_SET_DXNS:String = "SET_DXNS";
        public static const FLAG_HAS_PARAM_NAMES:String = "HAS_PARAM_NAMES";

        /**
         * Creates a method from scratch
         * @param paramTypes Parameter types for the new method
         * @param returnType Return type of the new method
         * @param name Name of the new method
         * @param flags New method's flags
         * @param options Default values
         * @param paramNames Parameter names
         * @param body Implementation, if it has one
         */
        public function ASMethod(paramTypes:Vector.<ASMultiname> = null, returnType:ASMultiname = null, name:String = "", flags:Vector.<String> = null, options:Vector.<ASValue> = null, paramNames:Vector.<String> = null, body:ASMethodBody = null)
        {
            if (paramTypes == null)
            {
                this.paramTypes = new <ASMultiname>[];
            }
            else
            {
                this.paramTypes = paramTypes;
            }
            this.returnType = returnType;
            this.name = name;
            if (flags == null)
            {
                this.flags = new <String>[];
            }
            else
            {
                this.flags = flags;
            }
            if (options == null)
            {
                this.options = new <ASValue>[];
            }
            else
            {
                this.options = options;
            }
            if (paramNames == null)
            {
                this.paramNames = new <String>[];
            }
            else
            {
                this.paramNames = paramNames;
            }
            this.body = body;
        }
    }
}
