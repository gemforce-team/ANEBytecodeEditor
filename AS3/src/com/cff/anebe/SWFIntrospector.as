package com.cff.anebe
{
    import flash.external.ExtensionContext;
    import flash.utils.ByteArray;
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASReadOnlyClass;
    import com.cff.anebe.ir.ASReadOnlyScript;

    /**
     * Partially disassembles an SWF so that high-level information can be pulled out of it. Does not allow rebuilding and editing.
     * @param swf SWF data to disassemble
     * @return Introspector for this SWF
     */
    public class SWFIntrospector
    {
        private var extContext:ExtensionContext;

        private function setSWF(replaceSWF:ByteArray):void
        {
            var ret:Object = extContext.call("SetCurrentSWF", replaceSWF);

            if (ret is String)
            {
                throw new Error(ret);
            }
            else if (ret == null || !(ret is Boolean))
            {
                throw new Error("Unknown error occurred while setting SWF");
            }
            else if (!(ret as Boolean))
            {
                throw new Error("SetCurrentSWF returned false somehow");
            }
        }

        private function beginIntrospection():void
        {
            var ret:Object = extContext.call("BeginIntrospection");

            if (ret is String)
            {
                throw new Error(ret);
            }
            else if (ret == null || !(ret is Boolean))
            {
                throw new Error("Unknown error occurred while setting SWF");
            }
            else if (!(ret as Boolean))
            {
                throw new Error("BeginIntrospection returned false somehow");
            }
        }

        public function SWFIntrospector(swf:ByteArray)
        {
            extContext = ExtensionContext.createExtensionContext("com.cff.anebe.ANEBytecodeEditor", "SWFIntrospector");

            setSWF(Utils.decompressSWF(swf));

            beginIntrospection();
        }

        /**
         * Gets a class after partial assembly.
         * This is essentially a shortcut for GetScript(name).GetTrait(name).clazz, but is more efficient and should likely be used instead.
         * @param className Name of class to be patched. Must be a QName.
         * @return The class requested.
         */
        public function GetClass(name:ASMultiname):ASReadOnlyClass
        {
            if (name == null || name.type != ASMultiname.TYPE_QNAME)
            {
                throw new ArgumentError("Class name must be a QName");
            }

            var ret:Object = extContext.call("GetClass", name);

            if (ret is String)
            {
                throw new Error(ret);
            }
            else if (ret == null || !(ret is ASReadOnlyClass))
            {
                throw new Error("Unknown error occurred");
            }
            else
            {
                return ret as ASReadOnlyClass;
            }
        }

        /**
         * Gets a script after partial assembly.
         * Scripts are the top-level building block of AS3 programs, but are never referenced in-engine.
         * They can contain functions, classes, and variables, and have a single initializer method that sets them up.
         * Note that direct editing of script is discouraged; using GetClass is generally a better idea.
         * This is mostly provided for the case of top-level-scope functions and variables, which can only be edited as script traits.
         * @param identifier Name of an entity in the script. Must be a QName.
         * @return The script requested.
         */
        public function GetScript(identifier:ASMultiname):ASReadOnlyScript
        {
            if (identifier == null || !(identifier is ASMultiname))
            {
                throw new Error("script identifier must be a QName");
            }

            var ret:Object = extContext.call("GetScript", identifier);

            if (ret is String)
            {
                throw new Error(ret);
            }
            else if (ret == null || !(ret is ASReadOnlyScript))
            {
                throw new Error("Unknown error occurred");
            }
            else
            {
                return ret as ASReadOnlyScript;
            }
        }
    }
}
