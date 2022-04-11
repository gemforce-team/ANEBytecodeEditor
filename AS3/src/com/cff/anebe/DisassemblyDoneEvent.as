package com.cff.anebe 
{
	import flash.events.Event;
	
	/**
	 * ...
	 * @author Chris
	 */
	public class DisassemblyDoneEvent extends Event 
	{
		public var strings:Object;
		public function DisassemblyDoneEvent(strings:Object, bubbles:Boolean=false, cancelable:Boolean=false) 
		{ 
			super(Events.DISASSEMBLY_DONE, bubbles, cancelable);
			
			this.strings = strings;
		} 
		
		public override function clone():Event 
		{ 
			return new DisassemblyDoneEvent(strings, bubbles, cancelable);
		} 
		
		public override function toString():String 
		{ 
			return formatToString("DisassemblyDoneEvent", "type", "bubbles", "cancelable", "eventPhase"); 
		}
		
	}
	
}