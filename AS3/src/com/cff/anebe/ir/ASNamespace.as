package com.cff.anebe.ir
{
    /**
     * An AS3 namespace in which an ASMultiname might check
     * @author Chris
     */
    public class ASNamespace
    {
        public var type:String;
        public var name:String;

        public var id:int = 0;

        public static const TYPE_PRIVATE:String = "PrivateNamespace";
        public static const TYPE_NORMAL:String = "Namespace";
        public static const TYPE_PACKAGE:String = "PackageNamespace";
        public static const TYPE_PACKAGEINTERNAL:String = "PackageInternalNs";
        public static const TYPE_PROTECTED:String = "ProtectedNamespace";
        public static const TYPE_EXPLICIT:String = "ExplicitNamespace";
        public static const TYPE_STATICPROTECTED:String = "StaticProtectedNs";

        public function ASNamespace(type:String = TYPE_NORMAL, name:String = "", id:int = 0)
        {
            this.type = type;
            this.name = name;
            this.id = id;
        }
    }
}
