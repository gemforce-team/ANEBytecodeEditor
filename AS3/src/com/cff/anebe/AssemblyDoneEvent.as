package com.cff.anebe
{
	import flash.events.Event;
	import flash.utils.ByteArray;

	public class AssemblyDoneEvent extends Event
	{
		/** The edited SWF contents */
		public var assembled:ByteArray;

		public function AssemblyDoneEvent(assembled:ByteArray, bubbles:Boolean = false, cancelable:Boolean = false)
		{
			super(Events.ASSEMBLY_DONE, bubbles, cancelable);
			this.assembled = assembled;
		}

		public override function clone():Event
		{
			return new AssemblyDoneEvent(assembled, bubbles, cancelable);
		}

		public override function toString():String
		{
			return formatToString("AssemblyDoneEvent", "type", "bubbles", "cancelable", "eventPhase");
		}

	}

}
