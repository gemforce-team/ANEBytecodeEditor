package com.cff.anebe.ir
{
    /**
     * A trait on an AS3 class. Traits contain methods and functions (see ASMethod), variables, or classes.
     * @author Chris
     */
    public class ASTrait
    {
        public var kind:String;
        public var attributes:Vector.<String>;
        public var slotId:uint;
        public var metadata:Vector.<ASMetadata>;
        public var name:ASMultiname;

        // If this is a slot or a const, this will be the type of the stored variable. Otherwise null.
        public var typename:ASMultiname;

        /** If this trait's kind is slot or const, the value of the stored variable, or null if there is none. Otherwise null. */
        public var value:ASValue;

        /** If this trait's kind is getter, setter, method, or function, the associated function object. Otherwise null. */
        public var funcOrMethod:ASMethod;

        /** If this trait's kind is class, the associated class object. Otherwise null. */
        public var clazz:ASClass;

        public static const KIND_SLOT:String = "slot";
        public static const KIND_METHOD:String = "method";
        public static const KIND_GETTER:String = "getter";
        public static const KIND_SETTER:String = "setter";
        public static const KIND_CLASS:String = "class";
        public static const KIND_FUNCTION:String = "function";
        public static const KIND_CONST:String = "const";

        public static const ATTR_FINAL:String = "FINAL";
        public static const ATTR_OVERRIDE:String = "OVERRIDE";
        public static const ATTR_METADATA:String = "METADATA";

        public function ASTrait(kind:String = null, attributes:Vector.<String> = null, slotId:uint = 0, name:ASMultiname = null, typename:ASMultiname = null, metadata:Vector.<ASMetadata> = null, value:ASValue = null, funcOrMethod:ASMethod = null, clazz:ASClass = null)
        {
            this.kind = kind;
            if (attributes == null)
            {
                this.attributes = new <String>[];
            }
            else
            {
                this.attributes = attributes;
            }
            this.slotId = slotId;
            this.name = name;
            this.typename = typename;
            if (metadata == null)
            {
                this.metadata = new <ASMetadata>[];
            }
            else
            {
                this.metadata = metadata;
            }
            this.value = value;
            this.funcOrMethod = funcOrMethod;
            this.clazz = clazz;
        }
    }
}
