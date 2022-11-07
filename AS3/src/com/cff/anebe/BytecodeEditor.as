package com.cff.anebe
{
	import com.cff.anebe.ir.ASClass;
	import com.cff.anebe.ir.ASMultiname;

	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.StatusEvent;
	import flash.external.ExtensionContext;
	import flash.utils.ByteArray;
	import flash.utils.CompressionAlgorithm;

	/**
	 * The main interface of this library. Allows editing bytecode of passed in SWF files.
	 * @author Chris
	 */
	public class BytecodeEditor extends EventDispatcher
	{
		private var extContext:ExtensionContext;

		private function decompressAndSetSWF(replaceSWF:ByteArray):void
		{
			replaceSWF.position = 0;
			var decompressor:ByteArray = new ByteArray();
			switch (replaceSWF.readUTFBytes(1))
			{
				case "F":
					replaceSWF.position = 0;
					break;
				case "C":
					replaceSWF.position = 8;
					replaceSWF.readBytes(decompressor);
					replaceSWF.position = 8;
					decompressor.uncompress(CompressionAlgorithm.ZLIB);
					replaceSWF.writeBytes(decompressor);
					replaceSWF.position = 0;
					replaceSWF.writeUTFBytes("F");
					replaceSWF.position = 0;
					decompressor = null;
					break;
				case "Z":
					replaceSWF.position = 8;
					replaceSWF.readBytes(decompressor, 8);
					replaceSWF.position = 8;
					decompressor.uncompress(CompressionAlgorithm.LZMA);
					replaceSWF.writeBytes(decompressor);
					replaceSWF.position = 0;
					replaceSWF.writeUTFBytes("F");
					replaceSWF.position = 0;
					decompressor = null;
					break;
				default:
					throw new Error("Unrecognized compression scheme for SWF");
			}

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
			decompressAndSetSWF(swf);

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
			if (replaceSWF != null)
			{
				decompressAndSetSWF(replaceSWF);
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
			decompressAndSetSWF(swf);

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
			if (replaceSWF != null)
			{
				decompressAndSetSWF(replaceSWF);
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
			if (replaceSWF != null)
			{
				decompressAndSetSWF(replaceSWF);
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
			if (replaceSWF != null)
			{
				decompressAndSetSWF(replaceSWF);
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
		 * @param className Name of class to be patched. Must be a QName.
		 * @param idx If there are multiple classes that match the name, disambiguates between them.
		 * @return The class requested.
		 */
		public function GetClass(name:ASMultiname, idx:uint):ASClass
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
