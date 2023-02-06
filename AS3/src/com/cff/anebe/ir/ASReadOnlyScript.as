package com.cff.anebe.ir
{
    import flash.external.ExtensionContext;

    /**
     * A representation of an AS3 script, editable at a high level.
     * @author Chris
     */
    public class ASReadOnlyScript
    {
        protected var context:ExtensionContext;

        /**
         * This function should not be called by any user of this library. It will be automatically called by GetScript.
         */
        public function ASReadOnlyScript()
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
    }
}
