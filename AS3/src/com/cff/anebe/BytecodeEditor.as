package com.cff.anebe 
{
	import flash.events.EventDispatcher;
	import flash.events.StatusEvent;
	import flash.external.ExtensionContext;
	import flash.utils.ByteArray;
	import flash.utils.CompressionAlgorithm;
	/**
	 * ...
	 * @author Chris
	 */
	public class BytecodeEditor extends EventDispatcher
	{
		private var extContext:ExtensionContext;
		
		public function BytecodeEditor() 
		{
			extContext = ExtensionContext.createExtensionContext("com.cff.anebe.ANEBytecodeEditor", "");
			extContext.addEventListener(StatusEvent.STATUS, this.onStatusEvent);
		}
		
		// Disassembles SWF into a map of file name to file contents
		public function Disassemble(swf:ByteArray):Object
		{
			swf.position = 0;
			var decompressor:ByteArray = new ByteArray();
			switch (swf.readUTFBytes(1))
			{
				case "F":
					swf.position = 0;
					break;
				case "C":
					swf.position = 8;
					swf.readBytes(decompressor);
					swf.position = 8;
					decompressor.uncompress(CompressionAlgorithm.ZLIB);
					swf.writeBytes(decompressor);
					swf.position = 0;
					swf.writeUTFBytes("F");
					swf.position = 0;
					decompressor = null;
					break;
				case "Z":
					swf.position = 8;
					swf.readBytes(decompressor, 8);
					swf.position = 8;
					decompressor.uncompress(CompressionAlgorithm.LZMA);
					swf.writeBytes(decompressor);
					swf.position = 0;
					swf.writeUTFBytes("F");
					swf.position = 0;
					decompressor = null;
					break;
				default:
					throw new Error("Unrecognized compression scheme for SWF");
			}
			
			var ret:Object = extContext.call("SetCurrentSWF", swf);
			
			if (ret == null)
			{
				throw new Error("Unknown error occurred while setting SWF");
			}
			else if (ret is String)
			{
				throw new Error(ret);
			}
			else // returned Boolean
			{
				if (!(ret as Boolean))
				{
					throw new Error("SetCurrentSWF returned false somehow");
				}
				else
				{
					ret = extContext.call("Disassemble");
			
					if (ret == null)
					{
						throw new Error("Unknown error occurred");
					}
					else if (ret is String)
					{
						throw new Error(ret);
					}
					else
					{
						return ret;
					}
				}
			}
		}
		
		// Assembles an SWF from a map of file name to file contents. If replaceSWF is unspecified, and Disassemble was called, uses
		// the last SWF's data for the rest of the SWF.
		public function Assemble(strings:Object, replaceSWF:ByteArray = null):ByteArray
		{
			if (replaceSWF != null)
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
			
				if (ret == null)
				{
					throw new Error("Unknown error occurred while setting SWF");
				}
				else if (ret is String)
				{
					throw new Error(ret);
				}
				else // returned Boolean
				{
					if (!(ret as Boolean))
					{
						throw new Error("SetCurrentSWF returned false somehow");
					}
				}
			}
			
			var vec:Vector.<String> = new <String>[];
			for (var str:String in strings)
			{
				vec[vec.length] = str;
			}
			
			ret = extContext.call("Assemble", strings, vec);
			
			if (ret == null)
			{
				throw new Error("Unknown error occurred");
			}
			else if (ret is String)
			{
				throw new Error(ret);
			}
			else // byte array was returned
			{
				return ret as ByteArray;
			}
		}
		
		// Disassembles SWF into a map of file name to file contents, asynchronously
		public function DisassembleAsync(swf:ByteArray):void
		{
			swf.position = 0;
			var decompressor:ByteArray = new ByteArray();
			switch (swf.readUTFBytes(1))
			{
				case "F":
					swf.position = 0;
					break;
				case "C":
					swf.position = 8;
					swf.readBytes(decompressor);
					swf.position = 8;
					decompressor.uncompress(CompressionAlgorithm.ZLIB);
					swf.writeBytes(decompressor);
					swf.position = 0;
					swf.writeUTFBytes("F");
					swf.position = 0;
					decompressor = null;
					break;
				case "Z":
					swf.position = 8;
					swf.readBytes(decompressor, 8);
					swf.position = 8;
					decompressor.uncompress(CompressionAlgorithm.LZMA);
					swf.writeBytes(decompressor);
					swf.position = 0;
					swf.writeUTFBytes("F");
					swf.position = 0;
					decompressor = null;
					break;
				default:
					throw new Error("Unrecognized compression scheme for SWF");
			}
			
			var ret:Object = extContext.call("SetCurrentSWF", swf);
			
			if (ret == null)
			{
				throw new Error("Unknown error occurred while setting SWF");
			}
			else if (ret is String)
			{
				throw new Error(ret);
			}
			else // returned Boolean
			{
				if (!(ret as Boolean))
				{
					throw new Error("SetCurrentSWF returned false somehow");
				}
				else
				{
					ret = extContext.call("DisassembleAsync");
			
					if (ret == null)
					{
						throw new Error("Unknown error occurred during DisassembleAsync");
					}
					else if (ret is String)
					{
						throw new Error(ret);
					}
					else // Returned Boolean
					{
						if (!(ret as Boolean))
						{
							throw new Error("DisassembleAsync returned false somehow");
						}
					}
				}
			}
		}
		
		// Assembles an SWF from a map of file name to file contents, asynchronously. If replaceSWF is unspecified, and Disassemble was called, uses
		// the last SWF's data for the rest of the SWF.
		public function AssembleAsync(strings:Object, replaceSWF:ByteArray = null):void
		{
			if (replaceSWF != null)
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
			
				if (ret == null)
				{
					throw new Error("Unknown error occurred while setting SWF");
				}
				else if (ret is String)
				{
					throw new Error(ret);
				}
				else // returned Boolean
				{
					if (!(ret as Boolean))
					{
						throw new Error("SetCurrentSWF returned false somehow");
					}
				}
			}
			
			var vec:Vector.<String> = new <String>[];
			for (var str:String in strings)
			{
				vec[vec.length] = str;
			}
			
			ret = extContext.call("AssembleAsync", strings, vec);
			
			if (ret == null)
			{
				throw new Error("Unknown error occurred");
			}
			else if (ret is String)
			{
				throw new Error(ret);
			}
			else // returned Boolean
			{
				if (!(ret as Boolean))
				{
					throw new Error("AssembleAsync returned false somehow");
				}
			}
		}
		
		public function Cleanup():void
		{
			extContext.call("Cleanup");
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
			else
			{
				var data:Object = extContext.call("AsyncTaskResult");
				
				if (data is ByteArray)
				{
					this.dispatchEvent(new AssemblyDoneEvent(data as ByteArray));
				}
				else
				{
					this.dispatchEvent(new DisassemblyDoneEvent(data));
				}
			}
		}
	}

}