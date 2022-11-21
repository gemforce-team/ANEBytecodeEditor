package com.cff.anebe.ir
{
    /**
     * A trait on an AS3 class. Traits contain methods and functions (see ASMethod), variables, or classes.
     * @author Chris
     */
    public class ASTrait
    {
        /** Kind of the trait. Trait kinds are listed under KIND_* in this class. */
        public var kind:String;

        /** Attributes this trait has. Trait attributes are listed under ATTR_* in this class */
        public var attributes:Vector.<String>;

        /** Slot/dispId of this trait. 0 lets the runtime assign a slot. */
        public var slotId:uint;

        /** Metadata attached to this trait. */
        public var metadata:Vector.<ASMetadata>;

        /** Name of this trait. Must always be a QName. */
        public var name:ASMultiname;

        /** For slot or const kinds, type of the stored variable. Null for others. */
        public var typename:ASMultiname;

        /** For slot or const kinds, the value the trait gets initialized to. May be null if there is no value. Null for other kinds. */
        public var value:ASValue;

        /** For getter, setter, method, or function kinds, the ASMethod that represents the function. Null for others. */
        public var funcOrMethod:ASMethod;

        /**
         * For class kind, the ASClass that represents the stored class. Null for others.
         * NOTE: currently, editing an ASClass through its methods changes the C++-side data for it *without* using a wrapper setTrait on this object.
         * This is done for efficiency reasons. It is still recommended to perform a setTrait with this object in case this changes in the future.
         */
        public var clazz:ASClass;

        /** Kind string for slots */
        public static const KIND_SLOT:String = "slot";

        /** Kind string for methods */
        public static const KIND_METHOD:String = "method";

        /** Kind string for getters */
        public static const KIND_GETTER:String = "getter";

        /** Kind string for setters */
        public static const KIND_SETTER:String = "setter";

        /** Kind string for classes */
        public static const KIND_CLASS:String = "class";

        /** Kind string for functions */
        public static const KIND_FUNCTION:String = "function";

        /** Kind string for consts */
        public static const KIND_CONST:String = "const";

        /** Final attribute: indicates this trait cannot be overridden. */
        public static const ATTR_FINAL:String = "FINAL";

        /** Override attribute: indicates this trait overrides a base class's. */
        public static const ATTR_OVERRIDE:String = "OVERRIDE";

        /** Metadata attribute: indicates that this trait has metadata. */
        public static const ATTR_METADATA:String = "METADATA";

        /**
         * Builds an ASTrait from scratch. Probably shouldn't be used; see instead helper functions in the package com.cff.anebe.ir.traits
         * @param kind Trait kind
         * @param attributes Trait attributes
         * @param slotId Trait slot ID or disp ID
         * @param name Trait name
         * @param typename Trait type name, if present
         * @param metadata Trait metadata, if present
         * @param value Trait initial value, if present
         * @param funcOrMethod Function stored in trait, if present
         * @param clazz Class stored in trait, if present
         */
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
