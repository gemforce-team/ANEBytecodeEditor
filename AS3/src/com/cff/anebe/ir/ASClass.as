package com.cff.anebe.ir
{
    /**
     * A representation of an AS3 class, editable at a high level.
     * @author Chris
     */
    public class ASClass extends ASReadOnlyClass
    {
        /**
         * This function should not be called by any user of this library. It will be automatically called by GetClass or CreateAndInsertClass.
         */
        public function ASClass()
        {
            super();
        }

        /**
         * Sets the name of this class's superclass
         * @param superClass The new superclass name
         */
        public function setSuperClass(superClass:ASMultiname):void
        {
            var ret:Object = context.call("SetSuperclass", superClass);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets the instance name of this class
         * @param name The new instance name
         */
        public function setInstanceName(name:ASMultiname):void
        {
            var ret:Object = context.call("SetInstanceName", name);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets a trait as static into the class.
         * If a trait does not exist by the name of the new trait, the new trait will be added.
         * If a getter exists by the name of the new trait and the new trait is a setter, the new trait will be added.
         * If a setter exists by the name of the new trait and the new trait is a getter, the new trait will be added.
         * If both a getter and setter exist by the name of the new trait and the new trait is neither a getter nor setter, the new trait will replace both the getter and setter.
         * If a trait of the same kind already exists by the name of the new trait, the new trait will replace that trait.
         * If a trait exists by the name of the new trait and the kinds do not match any of the above rules, the new trait will replace the original.
         * @param value The trait to set or add to the class
         */
        public function setStaticTrait(value:ASTrait):void
        {
            if (value == null)
            {
                throw new ArgumentError("Cannot set a null trait");
            }
            if (value.name == null || value.name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("SetStaticTrait", value);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Deletes a static trait present in the class
         * @param name Name of the trait to retrieve
         * @param favorSetter If this name refers to both a getter and a setter, true will delete the getter, while false will delete the setter. If it is neither or only one exists, has no effect.
         */
        public function deleteStaticTrait(name:ASMultiname, favorSetter:Boolean = false):void
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("DeleteStaticTrait", name, favorSetter);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets an instance trait as into the class.
         * If a trait does not exist by the name of the new trait, the new trait will be added.
         * If a getter exists by the name of the new trait and the new trait is a setter, the new trait will be added.
         * If a setter exists by the name of the new trait and the new trait is a getter, the new trait will be added.
         * If both a getter and setter exist by the name of the new trait and the new trait is neither a getter nor setter, the new trait will replace both the getter and setter.
         * If a trait of the same kind already exists by the name of the new trait, the new trait will replace that trait.
         * If a trait exists by the name of the new trait and the kinds do not match any of the above rules, the new trait will replace the original.
         * @param value The trait to set or add to the class
         */
        public function setInstanceTrait(value:ASTrait):void
        {
            if (value == null)
            {
                throw new ArgumentError("Cannot set a null trait");
            }
            if (value.name == null || value.name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("SetInstanceTrait", value);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Deletes an instance trait present in the class.
         * @param name Name of the trait to retrieve
         * @param favorSetter If this name refers to both a getter and a setter, true will delete the getter, while false will delete the setter. If it is neither or only one exists, has no effect.
         */
        public function deleteInstanceTrait(name:ASMultiname, favorSetter:Boolean = false):void
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("DeleteInstanceTrait", name, favorSetter);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets the static constructor for a class.
         * Will be run "when the newclass instruction is executed on the class_info entry for the class".
         * @param v The ASMethod that represents the new static constructor
         */
        public function setStaticConstructor(v:ASMethod):void
        {
            if (v == null)
            {
                throw new ArgumentError("Static constructor cannot be set to null");
            }
            var ret:Object = context.call("SetStaticConstructor", v);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets the instance constructor for a class
         * @param v The ASMethod that represents the new static constructor
         */
        public function setConstructor(v:ASMethod):void
        {
            if (v == null)
            {
                throw new ArgumentError("Instance constructor cannot be set to null");
            }
            var ret:Object = context.call("SetConstructor", v);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets the list of interfaces this class implements
         * @param v New list of classes the this class implements
         */
        public function setInterfaces(v:Vector.<ASMultiname>):void
        {
            if (v == null)
            {
                v = new <ASMultiname>[];
            }
            var ret:Object = context.call("SetInterfaces", v);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets the list of flags this class has
         * @param v New list of flags
         */
        public function setFlags(v:Vector.<String>):void
        {
            if (v == null)
            {
                v = new <String>[];
            }
            var ret:Object = context.call("SetFlags", v);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /**
         * Sets the protected namespace associated with this class
         * @param v The new protected namespace
         */
        public function setProtectedNamespace(v:ASNamespace):void
        {
            if (v == null)
            {
                throw new Error("Cannot set protected namespace to null");
            }
            var ret:Object = context.call("SetProtectedNamespace", v);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }

        /** Used internally */
        public function setNativePointerForConversion():void
        {
            context.call("ConvertClassHelper");
        }
    }
}
