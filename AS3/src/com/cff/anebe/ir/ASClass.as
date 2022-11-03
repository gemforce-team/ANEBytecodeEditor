package com.cff.anebe.ir
{
    import flash.external.ExtensionContext;

    /**
     * A representation of an AS3 class, editable at a high level.
     * @author Chris
     */
    public class ASClass
    {
        private var context:ExtensionContext;

        public static const FLAG_SEALED:String = "SEALED";
        public static const FLAG_FINAL:String = "FINAL";
        public static const FLAG_INTERFACE:String = "INTERFACE";
        public static const FLAG_PROTECTEDNS:String = "PROTECTEDNS";

        /**
         * This function should not be called by any user of this library. It will be automatically called by BytecodeEditor.GetClass.
         * This library does not support creating new classes in the SWF.
         */
        public function ASClass()
        {
            this.context = ExtensionContext.createExtensionContext("com.cff.anebe.ANEBytecodeEditor", "Class");
        }

        /**
         * Gets a static trait present in the class
         * @param name Name of the trait to retrieve
         * @param favorSetter If this name refers to both a getter and a setter, true will retrieve the getter, while false will retrieve the setter. If it is neither or only one exists, has no effect.
         * @return The trait retrieved, or null if none were found
         */
        public function getStaticTrait(name:ASMultiname, favorSetter:Boolean = false):ASTrait
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("GetStaticTrait", name, favorSetter);

            if (ret == null)
            {
                return null;
            }

            if (ret is String)
            {
                throw new Error(ret);
            }

            return ret as ASTrait;
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
         * Gets an instance trait present in the class
         * @param name Name of the trait to retrieve
         * @param favorSetter If this name refers to both a getter and a setter, true will retrieve the getter, while false will retrieve the setter. If it is neither or only one exists, has no effect.
         * @return The trait retrieved, or null if none were found
         */
        public function getInstanceTrait(name:ASMultiname, favorSetter:Boolean = false):ASTrait
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("GetInstanceTrait", name, favorSetter);

            if (ret == null)
            {
                return null;
            }

            if (ret is String)
            {
                throw new Error(ret);
            }

            return ret as ASTrait;
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
         * Gets the static constructor for a class
         * @return The ASMethod that represents the static constructor
         */
        public function getStaticConstructor():ASMethod
        {
            var ret:Object = context.call("GetStaticConstructor");

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is ASMethod))
            {
                throw new Error("An unspecified error occurred");
            }

            return ret as ASMethod;
        }

        /**
         * Sets the static constructor for a class
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
         * Gets the instance constructor for a class
         * @return The ASMethod that represents the instance constructor
         */
        public function getConstructor():ASMethod
        {
            var ret:Object = context.call("GetConstructor");

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is ASMethod))
            {
                throw new Error("An unspecified error occurred");
            }

            return ret as ASMethod;
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
         * Gets a list of the interfaces this class implements
         * @return List of interfaces this class implements
         */
        public function getInterfaces():Vector.<ASMultiname>
        {
            var ret:Object = context.call("GetInterfaces");

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Vector.<ASMultiname>))
            {
                throw new Error("An unspecified error occurred");
            }

            return ret as Vector.<ASMultiname>;
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
         * Gets the list of flags this class has
         * @return The list of flags
         */
        public function getFlags():Vector.<String>
        {
            var ret:Object = context.call("GetFlags");
            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Vector.<String>))
            {
                throw new Error("An unspecified error occurred");
            }
            return ret as Vector.<String>;
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
         * Gets the protected namespace associated with this class
         * @return The protected namespace
         */
        public function getProtectedNamespace():ASNamespace
        {
            var ret:Object = context.call("GetProtectedNamespace");
            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is ASNamespace))
            {
                throw new Error("An unspecified error occurred");
            }
            return ret as ASNamespace;
        }

        /**
         * Sets the protected namespace associated with this class
         * @param v The new protected namespace
         */
        public function setProtectedNamespace(v:ASNamespace):void
        {
            if (v == null)
            {
                v = new ASNamespace();
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
