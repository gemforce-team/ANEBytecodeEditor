package com.cff.anebe.ir
{
    /**
     * An AS3 namespace in which an ASMultiname might check
     * @author Chris
     */
    public class ASNamespace
    {
        /** Type of the namespace. Types are listed as TYPE_* in this class. */
        public var type:String;

        /** Namespace name. */
        public var name:String;

        /** If there are multiple namespaces with the same name, disambiguates them. This should only very rarely be non-null. */
        public var secondaryName:String;

        /** Type string for private namespaces */
        public static const TYPE_PRIVATE:String = "PrivateNamespace";

        /** Type string for "normal" namespaces */
        public static const TYPE_NORMAL:String = "Namespace";

        /** Type string for package namespaces */
        public static const TYPE_PACKAGE:String = "PackageNamespace";

        /** Type string for package internal namespaces (things defined as "internal" in AS3 code) */
        public static const TYPE_PACKAGEINTERNAL:String = "PackageInternalNs";

        /** Type string for protected namespaces */
        public static const TYPE_PROTECTED:String = "ProtectedNamespace";

        /** Type string for explicit namespaces */
        public static const TYPE_EXPLICIT:String = "ExplicitNamespace";

        /** Type string for static protected namespaces */
        public static const TYPE_STATICPROTECTED:String = "StaticProtectedNs";

        /**
         * Builds an ASNamespace from scratch. Should probably not be used; see instead helper functions in the package com.cff.anebe.ir.namespaces
         * @param type Namespace type
         * @param name Namespace name
         * @param secondaryName Namespace secondary name, if disambiguation is required
         */
        public function ASNamespace(type:String, name:String, secondaryName:String = null)
        {
            this.type = type;
            this.name = name;
            this.secondaryName = secondaryName;
        }
    }
}
