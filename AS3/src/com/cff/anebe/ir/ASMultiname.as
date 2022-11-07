package com.cff.anebe.ir
{
    /**
     * An identifier for an AS3 class, method, function, variable, or possibly others. Used for name resolution in instructions.
     * @author Chris
     */
    public class ASMultiname
    {
        /** Type of the multiname. Types are listed as TYPE_* constants in this class */
        public var type:String;

        /** For types QName, Multiname, RTQName, and their A equivalents, the name associated with the multiname. Null for others. */
        public var name:String;

        /** For types QName and QNameA, the namespace associated with the multiname. Null for others. */
        public var ns:ASNamespace;

        /** For types Multiname, MultinameL, and their A equivalents, the set of namespaces associated with the multiname. Null for others. */
        public var nsSet:Vector.<ASNamespace>;

        /** For type TypeName, the name of the type being parameterized. Null for others. */
        public var typename:ASMultiname;

        /** For type TypeName, the parameters to the type being parameterized. Null for others. */
        public var params:Vector.<ASMultiname>;

        /** Type string for QNames, which have a namespace and a name. */
        public static const TYPE_QNAME:String = "QName";

        /** Type string for Multinames, which have a name and a set of namespaces. */
        public static const TYPE_MULTINAME:String = "Multiname";

        /** Type string for run-time QNames, which have a name but no namespace. */
        public static const TYPE_RTQNAME:String = "RTQName";

        /** Type string for run-time late QNames, which have neither name nor namespace. */
        public static const TYPE_RTQNAMEL:String = "RTQNameL";

        /** Type string for late Multinames, which have a set of namespaces but no name. */
        public static const TYPE_MULTINAMEL:String = "MultinameL";

        /** Type string for TypeNames, which have a type and parameters to that type. */
        public static const TYPE_TYPENAME:String = "TypeName";

        /** Type string for A-type QNames, which have a namespace and a name. Usage of this is unknown. */
        public static const TYPE_QNAMEA:String = "QNameA";

        /** Type string for A-type Multinames, which have a name and a set of namespaces. Usage of this is unknown. */
        public static const TYPE_MULTINAMEA:String = "MultinameA";

        /** Type string for A-type run-time QNames, which have a name but no namespace. Usage of this is unknown. */
        public static const TYPE_RTQNAMEA:String = "RTQNameA";

        /** Type string for A-type run-time late QNames, which have neither name nor namespace. Usage of this is unknown. */
        public static const TYPE_RTQNAMELA:String = "RTQNameLA";

        /** Type string for A-type late Multinames, which have a set of namespaces but no name. Usage of this is unknown. */
        public static const TYPE_MULTINAMELA:String = "MultinameLA";

        /**
         * Builds an ASMultiname from scratch. Should probably not be used; see instead helper functions in the package com.cff.anebe.ir.multinames
         * @param type Type of the ASMultiname
         * @param name Name associated with the multiname, if the type has one
         * @param ns Namespace associated with the multiname, if the type has one
         * @param nsSet Namespace set associated with the multiname, if the type has one
         * @param typename Type name being parameterized (for TypeNames)
         * @param params Type parameters (for TypeNames)
         */
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
