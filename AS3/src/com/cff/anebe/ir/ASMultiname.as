package com.cff.anebe.ir
{
    /**
     * An identifier for an AS3 class, method, function, variable, or possibly others. Used for name resolution in instructions.
     * @author Chris
     */
    public class ASMultiname
    {
        public var type:String;
        // Used for all types except RTQNameL, RTQNameLA, and TypeName
        public var name:String;
        // Used for QName and QNameA
        public var ns:ASNamespace;
        // Used for Multiname and MultinameL
        public var nsSet:Vector.<ASNamespace>;
        // Used for typename
        public var typename:ASMultiname;
        // Also used for typename
        public var params:Vector.<ASMultiname>;

        public static const TYPE_QNAME:String = "QName";
        public static const TYPE_MULTINAME:String = "Multiname";
        public static const TYPE_RTQNAME:String = "RTQName";
        public static const TYPE_RTQNAMEL:String = "RTQNameL";
        public static const TYPE_MULTINAMEL:String = "MultinameL";
        public static const TYPE_TYPENAME:String = "TypeName";

        public static const TYPE_QNAMEA:String = "QNameA";
        public static const TYPE_MULTINAMEA:String = "MultinameA";
        public static const TYPE_RTQNAMEA:String = "RTQNameA";
        public static const TYPE_RTQNAMELA:String = "RTQNameLA";
        public static const TYPE_MULTINAMELA:String = "MultinameLA";

        public function ASMultiname(type:String = null, name:String = null, ns:ASNamespace = null, nsSet:Vector.<ASNamespace> = null, typename:ASMultiname = null, params:Vector.<ASMultiname> = null)
        {
            this.type = type;
            this.name = name;
            this.ns = ns;
            this.nsSet = nsSet;
            this.typename = typename;
            this.params = params;
        }
    }
}
