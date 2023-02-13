package com.cff.anebe
{
	import com.cff.anebe.ir.ASClass;
	import com.cff.anebe.ir.ASMultiname;

	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.StatusEvent;
	import flash.external.ExtensionContext;
	import flash.utils.ByteArray;
	import com.cff.anebe.ir.ASScript;
	import com.cff.anebe.ir.ASMethod;

	/**
	 * The main interface of this library. Allows editing bytecode of passed in SWF files.
	 * @author Chris
	 */
	public class BytecodeEditor extends EventDispatcher
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

		public function BytecodeEditor()
		{
			extContext = ExtensionContext.createExtensionContext("com.cff.anebe.ANEBytecodeEditor", "BytecodeEditor");
			extContext.addEventListener(StatusEvent.STATUS, this.onStatusEvent);
		}

		/**
		 * Disassembles SWF into a map of file name to file contents.
		 * @param swf The SWF to disassemble
		 * @return The new map of file name to file contents.
		 */
		public function Disassemble(swf:ByteArray):Object
		{
			setSWF(Utils.decompressSWF(swf));

			var ret:Object = extContext.call("Disassemble");

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null)
			{
				throw new Error("Unknown error occurred");
			}
			else
			{
				return ret;
			}
		}

		/**
		 * Assembles an SWF from a map of file name to file contents.
		 * @param strings File name and contents map
		 * @param includeDebugInstructions Whether to include debug instructions in the built SWF
		 * @param replaceSWF SWF data to replace the DoABC2 tag within, or null to use last disassembled.
		 * @return The new SWF data.
		 */
		public function Assemble(strings:Object, includeDebugInstructions:Boolean, replaceSWF:ByteArray = null):ByteArray
		{
			if (strings == null)
			{
				throw new Error("Cannot assemble null strings");
			}

			if (replaceSWF != null)
			{
				setSWF(Utils.decompressSWF(replaceSWF));
			}

			var vec:Vector.<String> = new <String>[];
			for (var str:String in strings)
			{
				vec[vec.length] = str;
			}

			var ret:Object = extContext.call("Assemble", strings, vec, includeDebugInstructions);

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is ByteArray))
			{
				throw new Error("Unknown error occurred");
			}
			else
			{
				return ret as ByteArray;
			}
		}

		/**
		 * Disassembles SWF into a map of file name to file contents, asynchronously. To retrieve the data, subscribe to the DISASSEMBLY_DONE event.
		 * @param swf The SWF to disassemble
		 */
		public function DisassembleAsync(swf:ByteArray):void
		{
			setSWF(Utils.decompressSWF(swf));

			var ret:Object = extContext.call("DisassembleAsync");

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is Boolean))
			{
				throw new Error("Unknown error occurred during DisassembleAsync");
			}
			else if (!(ret as Boolean))
			{
				throw new Error("DisassembleAsync returned false somehow");
			}

		}

		/**
		 * Assembles an SWF from a map of file name to file contents, asynchronously. To retrieve the data, subscribe to the ASSEMBLY_DONE event.
		 * @param strings File name and contents map
		 * @param includeDebugInstructions Whether to include debug instructions in the built SWF
		 * @param replaceSWF SWF data to replace the DoABC2 tag within, or null to use last disassembled.
		 */
		public function AssembleAsync(strings:Object, includeDebugInstructions:Boolean, replaceSWF:ByteArray = null):void
		{
			if (strings == null)
			{
				throw new Error("Cannot assemble null strings");
			}
			if (replaceSWF != null)
			{
				setSWF(Utils.decompressSWF(replaceSWF));
			}

			var vec:Vector.<String> = new <String>[];
			for (var str:String in strings)
			{
				vec[vec.length] = str;
			}

			var ret:Object = extContext.call("AssembleAsync", strings, vec, includeDebugInstructions);

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is Boolean))
			{
				throw new Error("Unknown error occurred");
			}
			else if (!(ret as Boolean))
			{
				throw new Error("AssembleAsync returned false somehow");
			}
		}

		/**
		 * Cleans up any temporary data that was necessary during assembly or disassembly.
		 */
		public function Cleanup():void
		{
			extContext.call("Cleanup");
		}

		/**
		 * Partially assembles an SWF from a map of file name to file contents.
		 * This is meant to be used to allow using the GetClass function, which can provide a higher level interface to bytecode edits than text edits.
		 * @param strings File name and contents map
		 * @param includeDebugInstructions Whether to include debug instructions in the built SWF
		 * @param replaceSWF SWF data to replace the DoABC2 tag within, or null to use last disassembled.
		 */
		public function PartialAssemble(strings:Object, includeDebugInstructions:Boolean, replaceSWF:ByteArray = null):void
		{
			if (strings == null)
			{
				throw new Error("Cannot assemble null strings");
			}
			if (replaceSWF != null)
			{
				setSWF(Utils.decompressSWF(replaceSWF));
			}

			var vec:Vector.<String> = new <String>[];
			for (var str:String in strings)
			{
				vec[vec.length] = str;
			}

			var ret:Object = extContext.call("PartialAssemble", strings, vec, includeDebugInstructions);

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is Boolean))
			{
				throw new Error("Unknown error occurred");
			}
			else if (!(ret as Boolean))
			{
				throw new Error("PartialAssemble returned false somehow");
			}
		}

		/**
		 * Partially assembles an SWF from a map of file name to file contents, asynchronously. To be notified when it's done, subscribe to the PARTIAL_ASSEMBLY_DONE event.
		 * This is meant to be used to allow using the GetClass function, which can provide a higher level interface to bytecode edits than text edits.
		 * @param strings File name and contents map
		 * @param includeDebugInstructions Whether to include debug instructions in the built SWF
		 * @param replaceSWF SWF data to replace the DoABC2 tag within, or null to use last disassembled.
		 */
		public function PartialAssembleAsync(strings:Object, includeDebugInstructions:Boolean, replaceSWF:ByteArray = null):void
		{
			if (strings == null)
			{
				throw new Error("Cannot assemble null strings");
			}
			if (replaceSWF != null)
			{
				setSWF(Utils.decompressSWF(replaceSWF));
			}

			var vec:Vector.<String> = new <String>[];
			for (var str:String in strings)
			{
				vec[vec.length] = str;
			}

			var ret:Object = extContext.call("PartialAssembleAsync", strings, vec, includeDebugInstructions);

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is Boolean))
			{
				throw new Error("Unknown error occurred");
			}
			else if (!(ret as Boolean))
			{
				throw new Error("PartialAssembleAsync returned false somehow");
			}
		}

		/**
		 * Finishes assembly started by PartialAssemble or PartialAssembleAsync.
		 * @return The modified SWF data.
		 */
		public function FinishAssemble():ByteArray
		{
			var ret:Object = extContext.call("FinishAssemble");

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is ByteArray))
			{
				throw new Error("Unknown error occurred");
			}
			else
			{
				return ret as ByteArray;
			}
		}

		/**
		 * Finishes assembly started by PartialAssemble or PartialAssembleAsync. To retrieve the data, subscribe to the ASSEMBLY_DONE event.
		 */
		public function FinishAssembleAsync():void
		{
			var ret:Object = extContext.call("FinishAssembleAsync");

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is Boolean))
			{
				throw new Error("Unknown error occurred");
			}
			else if (!(ret as Boolean))
			{
				throw new Error("AssembleAsync returned false somehow");
			}
		}

		/**
		 * Gets a class after partial assembly.
		 * This is essentially a shortcut for GetScript(name).GetTrait(name).clazz, but is more efficient and should likely be used instead.
		 * @param className Name of class to be patched. Must be a QName.
		 * @return The class requested.
		 */
		public function GetClass(name:ASMultiname):ASClass
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
			else if (ret == null || !(ret is ASClass))
			{
				throw new Error("Unknown error occurred");
			}
			else
			{
				return ret as ASClass;
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
		public function GetScript(identifier:ASMultiname):ASScript
		{
			if (identifier == null || !(identifier is ASMultiname))
			{
				throw new Error("Script identifier must be a QName");
			}

			var ret:Object = extContext.call("GetScript", identifier);

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is ASScript))
			{
				throw new Error("Unknown error occurred");
			}
			else
			{
				return ret as ASScript;
			}
		}

		/**
		 * Creates a script and places it into the current partial assembly.
		 * @param sinit Script initializer method
		 * @return The script created
		 */
		public function CreateAndInsertScript(sinit:ASMethod):ASScript
		{
			if (sinit == null)
			{
				throw new Error("Scripts must have a script initializer");
			}
			var ret:Object = extContext.call("CreateScript", sinit);

			if (ret is String)
			{
				throw new Error(ret);
			}
			else if (ret == null || !(ret is ASScript))
			{
				throw new Error("Unknown error occurred");
			}
			else
			{
				return ret as ASScript;
			}
		}

		private function onStatusEvent(e:StatusEvent):void
		{
			if (e.level == "ERROR")
			{
				var error:Object = extContext.call("AsyncTaskResult");
				if (error is String)
				{
					throw new Error(error);
				}
				else
				{
					throw new Error("Unknown error occurred while performing an ANEByteCodeEditor async task");
				}
			}
			else if (e.level == "SUCCESS")
			{
				var data:Object = extContext.call("AsyncTaskResult");

				if (data is String)
				{
					throw new Error(data);
				}
				else if (data == null)
				{
					throw new Error("Unknown error occurred during AsyncTaskResult");
				}

				if (data is ByteArray)
				{
					this.dispatchEvent(new AssemblyDoneEvent(data as ByteArray));
				}
				else
				{
					this.dispatchEvent(new DisassemblyDoneEvent(data));
				}
			}
			else if (e.level == "PartialSuccess")
			{
				extContext.call("AsyncTaskResult");
				this.dispatchEvent(new Event(Events.PARTIAL_ASSEMBLY_DONE));
			}
		}
	}
}
