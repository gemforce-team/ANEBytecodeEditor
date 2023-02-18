package com.cff.anebe.ir
{
    import flash.external.ExtensionContext;

    /**
     * A representation of an AS3 class, readable at a high level.
     * @author Chris
     */
    public class ASReadOnlyClass
    {
        protected var context:ExtensionContext;

        /** Sealed flag: indicates that the class cannot have properties added to it dynamically */
        public static const FLAG_SEALED:String = "SEALED";

        /** Final flag: indicates that the class cannot be inherited from */
        public static const FLAG_FINAL:String = "FINAL";

        /** Interface flag: indicates that the class is an interface */
        public static const FLAG_INTERFACE:String = "INTERFACE";

        /** Protected namespace flag: indicates that the class uses and has a protected namespace. */
        public static const FLAG_PROTECTEDNS:String = "PROTECTEDNS";

        /**
         * This function should not be called by any user of this library. It will be automatically called by GetClass.
         * I would make it private if I could.
         */
        public function ASReadOnlyClass()
        {
            this.context = ExtensionContext.createExtensionContext("com.cff.anebe.ANEBytecodeEditor", "Class");
        }

        /**
         * Gets the name of this class's superclass
         * @return The superclass name
         */
        public function getSuperClass():ASMultiname
        {
            var ret:Object = context.call("GetSuperclass");

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is ASMultiname))
            {
                throw new Error("An unspecified error occurred");
            }
            return ret as ASMultiname;
        }

        /**
         * Gets the instance name of this class
         * @return The instance name
         */
        public function getInstanceName():ASMultiname
        {
            var ret:Object = context.call("GetInstanceName");

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is ASMultiname))
            {
                throw new Error("An unspecified error occurred");
            }
            return ret as ASMultiname;
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
         * Gets a list of all static traits' names
         * @return The list of trait names
         */
        public function listStaticTraits():Vector.<ASMultiname>
        {
            var ret:Object = context.call("ListStaticTraits");

            if (ret is String)
            {
                throw new Error(ret);
            }
            else if (!(ret is Vector.<ASMultiname>))
            {
                throw new Error("An unspecified error occurred");
            }

            return ret as Vector.<ASMultiname>;
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
         * Gets a list of all instance traits' names
         * @return The list of trait names
         */
        public function listInstanceTraits():Vector.<ASMultiname>
        {
            var ret:Object = context.call("ListInstanceTraits");

            if (ret is String)
            {
                throw new Error(ret);
            }
            else if (!(ret is Vector.<ASMultiname>))
            {
                throw new Error("An unspecified error occurred");
            }

            return ret as Vector.<ASMultiname>;
        }

        /**
         * Gets the static constructor for a class.
         * Will be run when the "when the newclass instruction is executed on the class_info entry for the class".
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

        /** Used internally */
        public function setNativePointerForConversion():void
        {
            context.call("ConvertClassHelper");
        }
    }
}
