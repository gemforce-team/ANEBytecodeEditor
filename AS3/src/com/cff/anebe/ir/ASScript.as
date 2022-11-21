package com.cff.anebe.ir
{
    import flash.external.ExtensionContext;

    /**
     * A representation of an AS3 script, editable at a high level.
     * @author Chris
     */
    public class ASScript
    {
        private var context:ExtensionContext;

        /**
         * This function should not be called by any user of this library. It will be automatically called by BytecodeEditor.GetScript.
         */
        public function ASScript()
        {
            this.context = ExtensionContext.createExtensionContext("com.cff.anebe.ANEBytecodeEditor", "Script");
        }

        /**
         * Gets an instance trait present in the script
         * @param name Name of the trait to retrieve
         * @param favorSetter If this name refers to both a getter and a setter, true will retrieve the getter, while false will retrieve the setter. If it is neither or only one exists, has no effect.
         * @return The trait retrieved, or null if none were found
         */
        public function getTrait(name:ASMultiname, favorSetter:Boolean = false):ASTrait
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("GetTrait", name, favorSetter);

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
         * Sets a trait into the script.
         * If a trait does not exist by the name of the new trait, the new trait will be added.
         * If a getter exists by the name of the new trait and the new trait is a setter, the new trait will be added.
         * If a setter exists by the name of the new trait and the new trait is a getter, the new trait will be added.
         * If both a getter and setter exist by the name of the new trait and the new trait is neither a getter nor setter, the new trait will replace both the getter and setter.
         * If a trait of the same kind already exists by the name of the new trait, the new trait will replace that trait.
         * If a trait exists by the name of the new trait and the kinds do not match any of the above rules, the new trait will replace the original.
         * @param value The trait to set or add to the script
         */
        public function setTrait(value:ASTrait):void
        {
            if (value == null)
            {
                throw new ArgumentError("Cannot set a null trait");
            }
            if (value.name == null || value.name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("SetTrait", value);

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
         * Deletes a trait present in the script.
         * @param name Name of the trait to retrieve
         * @param favorSetter If this name refers to both a getter and a setter, true will delete the getter, while false will delete the setter. If it is neither or only one exists, has no effect.
         */
        public function deleteTrait(name:ASMultiname, favorSetter:Boolean = false):void
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Trait name must be a QName");
            }

            var ret:Object = context.call("DeleteTrait", name, favorSetter);

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
         * Gets the initializer for a script.
         * Will be run either on program start (for the main script) or when "an entity exported from that script is first referenced by the program during property lookup".
         * @return The ASMethod that represents the script initializer
         */
        public function getScriptInitializer():ASMethod
        {
            var ret:Object = context.call("GetInitializer");

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
         * Sets the initializer for a script.
         * Will be run either on program start (for the main script) or when "an entity exported from that script is first referenced by the program during property lookup".
         * @param v The ASMethod that represents the new script initializer
         */
        public function setScriptInitializer(v:ASMethod):void
        {
            if (v == null)
            {
                throw new ArgumentError("Static constructor cannot be set to null");
            }
            var ret:Object = context.call("SetInitializer", v);

            if (ret is String)
            {
                throw new Error(ret);
            }
            if (!(ret is Boolean) || !ret)
            {
                throw new Error("An unspecified error occurred");
            }
        }
    }
}
