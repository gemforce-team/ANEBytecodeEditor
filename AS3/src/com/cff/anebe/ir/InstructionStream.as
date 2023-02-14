package com.cff.anebe.ir
{
    /**
     * Represents the instructions of a method as a stream. Shouldn't really be constructed by users.
     */
    public class InstructionStream
    {
        private var instructions:Vector.<ASInstruction>;
        private var index:int;
        private var backwards:Boolean;

        /**
         * Constructs and initializes an InstructionStream, either forwards or backwards.
         * @param instructions Instructions to stream
         * @param backwards Whether it's backwards (true) or forwards (false)
         */
        public function InstructionStream(instructions:Vector.<ASInstruction>, backwards:Boolean)
        {
            this.instructions = instructions;
            this.backwards = backwards;
            this.index = !backwards ? 0 : instructions.length;
        }

        public function get currentIndex():int
        {
            if (backwards)
            {
                return index == 0 ? 0 : index - 1;
            }
            else
            {
                return index;
            }
        }

        /**
         * Reverses the stream, so that advance goes in the opposite direction from what it originally was.
         * Current instruction stays the current instruction.
         * @return this (for chaining)
         */
        public function reverse():InstructionStream
        {
            // Change direction
            backwards = !backwards;

            // Correct index
            if (backwards)
            {
                index++;
            }
            else
            {
                index--;
            }

            // Return this for chaining
            return this;
        }

        /**
         * Traverses the stream, starting with the current instruction, until it finds an instruction that matches the filter.
         * That instruction is then the current instruction.
         * @param filter Function to apply to the instructions. Should take an ASInstruction and return a Boolean
         * @return this (for chaining)
         * @throws Error Instruction not found
         */
        public function findNext(filter:Function):InstructionStream
        {
            var instr:ASInstruction;
            if (backwards)
            {
                while (index > 0)
                {
                    // Grab current instruction
                    instr = instructions[index - 1];
                    // Apply filter function
                    if (filter(instr))
                    {
                        // Break if it matches
                        break;
                    }
                    // Advance if it doesn't
                    index--;
                    // And if we went too far throw an error.
                    if (index == 0)
                    {
                        throw new Error("Instruction not found");
                    }
                }
            }
            else
            {
                var len:uint = instructions.length;
                while (index < len)
                {
                    // Grab current instruction
                    instr = instructions[index];
                    // Apply filter function
                    if (filter(instr))
                    {
                        // Break if it matches
                        break;
                    }
                    // Advance if it doesn't
                    index++;
                    // And if we went too far throw an error.
                    if (index == len)
                    {
                        throw new Error("Instruction not found");
                    }
                }
            }

            // Return this for chaining
            return this;
        }

        /**
         * Traverses the stream until it finds a window of instructions that match the passed in filters, in order.
         * Passing a single filter is treated exactly like findNext(that filter).
         * @param ignoreDebug Whether to pass debug instructions to the filters or ignore them.
         * @param ...filters Array of functions to apply to the instructions. Each should take an ASInstruction and return a Boolean.
         * @return this (for chaining)
         * @throws Error Instruction not found
         */
        public function findNextWindow(ignoreDebug:Boolean, ...filters:Array):InstructionStream
        {
            // Couple sanity checks
            if (filters.length == 0)
            {
                throw new Error("Window size was 0");
            }
            else if (filters.length == 1)
            {
                return findNext(filters[0]);
            }

            var instr:ASInstruction;
            var limit:uint;
            var i:uint = 0;
            var indexMod:uint;
            var shouldBreak:Boolean;
            var func:Function;
            if (backwards)
            {
                // Our limit is the amount of instructions we're looking for
                limit = filters.length;
                while (index > limit)
                {
                    // If any of the filters don't pass, the entire thing doesn't so we don't break
                    shouldBreak = true;
                    for (i = 0; i < filters.length; i++)
                    {
                        // Current filter
                        func = filters[i];

                        // Calculate the new index
                        if (ignoreDebug)
                        {
                            indexMod = countNextIgnoreDebug(i + 1);
                        }
                        else
                        {
                            indexMod = i;
                        }

                        // If we went past the end because of debug instructions, throw an error
                        if (indexMod >= index)
                        {
                            throw new Error("Instruction window not found");
                        }

                        instr = instructions[index - 1 - indexMod];
                        // Check the current filter
                        if (!func(instr))
                        {
                            // And if it doesn't pass make sure that we know that
                            shouldBreak = false;
                            break;
                        }
                    }
                    // If all filters passed then we can return
                    if (shouldBreak)
                    {
                        break;
                    }
                    // Otherwise we advance the index
                    index--;
                    // And check that we haven't reached the limit yet
                    if (index == limit)
                    {
                        throw new Error("Instruction window not found");
                    }
                }
            }
            else
            {
                // Our limit is the amount of instructions we're looking for
                limit = instructions.length - filters.length + 1;
                while (index < limit)
                {
                    // If any of the filters don't pass, the entire thing doesn't so we don't break
                    shouldBreak = true;
                    for (i = 0; i < filters.length; i++)
                    {
                        // Current filter
                        func = filters[i];

                        // Calculate the new index
                        if (ignoreDebug)
                        {
                            indexMod = countNextIgnoreDebug(i + 1);
                        }
                        else
                        {
                            indexMod = i;
                        }

                        // If we went past the end because of debug instructions, throw an error
                        if (index + indexMod >= instructions.length)
                        {
                            throw new Error("Instruction window not found");
                        }

                        instr = instructions[index + indexMod];
                        // Check the current filter
                        if (!func(instr))
                        {
                            // And if it doesn't pass make sure that we know that
                            shouldBreak = false;
                            break;
                        }
                    }
                    // If all filters passed then we can return
                    if (shouldBreak)
                    {
                        break;
                    }
                    // Otherwise we advance the index
                    index++;
                    // And check that we haven't reached the limit yet
                    if (index == limit)
                    {
                        throw new Error("Instruction window not found");
                    }
                }
            }

            // Return this for chaining
            return this;
        }

        // Fixes the passed in number for debug ignores
        private function countNextIgnoreDebug(number:uint):uint
        {
            var i:uint = 0;
            var boundary:uint;
            var newIndex:uint = index;
            if (backwards)
            {
                boundary = 0;
                while (i < number && newIndex != boundary)
                {
                    if (!instructions[newIndex - 1].isDebug())
                    {
                        i++;
                    }
                    newIndex--;
                }
            }
            else
            {
                boundary = instructions.length;
                while (i < number && newIndex != boundary)
                {
                    if (!instructions[newIndex].isDebug())
                    {
                        i++;
                    }
                    newIndex++;
                }
            }

            return newIndex < index ? index - newIndex : newIndex - index;
        }

        // Fixes the passed in number for debug ignores
        private function countPrevIgnoreDebug(number:uint):uint
        {
            var i:uint = 0;
            var boundary:uint;
            var newIndex:uint = index;
            if (backwards)
            {
                if (newIndex == 0)
                {
                    newIndex++;
                }
                boundary = instructions.length;
                while (i < number && newIndex != boundary)
                {
                    if (!instructions[newIndex - 1].isDebug())
                    {
                        i++;
                    }
                    newIndex++;
                }
            }
            else
            {
                if (newIndex == instructions.length)
                {
                    newIndex--;
                }
                boundary = 0;
                while (i < number && newIndex != boundary)
                {
                    if (!instructions[newIndex].isDebug())
                    {
                        i++;
                    }
                    newIndex--;
                }
            }

            return newIndex < index ? index - newIndex : newIndex - index;
        }

        /**
         * Seeks along the stream by a certain number of instructions.
         * Will not error, but instead stop when it reaches the end and tries to go past.
         * If amount is negative, behaves exactly as backtrack(-amount, ignoreDebug). Otherwise, behaves exactly as advance(amount, ignoreDebug).
         * @param amount Amount to seek.
         * @param ignoreDebug Whether or not to ignore debug instructions while advance
         * @return this (for chaining)
         */
        public function seek(amount:int, ignoreDebug:Boolean = true):InstructionStream
        {
            if (amount < 0)
            {
                backtrack(-amount, ignoreDebug);
            }
            else
            {
                advance(amount, ignoreDebug);
            }
            return this;
        }

        /**
         * Advances stream by a certain number of instructions.
         * Will not error, but instead stop when it reaches the end and tries to go past.
         * @param amount Number of instructions to advance
         * @param ignoreDebug Whether or not to ignore debug instructions while advance
         * @return this (for chaining)
         */
        public function advance(amount:uint = 1, ignoreDebug:Boolean = true):InstructionStream
        {
            // Fix count if ignoring debug instructions
            if (ignoreDebug)
            {
                amount = countNextIgnoreDebug(amount);
            }

            // Actually edit index
            if (backwards)
            {
                index -= amount;
            }
            else
            {
                index += amount;
            }

            // Correct index for past the end
            if (index > instructions.length)
            {
                index = instructions.length;
            }
            else if (index < 0)
            {
                index = 0;
            }

            // Return this for chaining
            return this;
        }

        /**
         * Advances stream by a certain number of instructions.
         * Will not error, but instead stop when it reaches the end and tries to go past.
         * @param amount Number of instructions to advance
         * @param ignoreDebug Whether or not to ignore debug instructions while advance
         * @return this (for chaining)
         */
        public function backtrack(amount:uint = 1, ignoreDebug:Boolean = true):InstructionStream
        {
            // Fix count if ignoring debug instructions
            if (ignoreDebug)
            {
                amount = countPrevIgnoreDebug(amount);
            }

            // Actually edit index
            if (backwards)
            {
                index += amount;
            }
            else
            {
                index -= amount;
            }

            // Correct index for past the end
            if (index > instructions.length)
            {
                index = instructions.length;
            }
            else if (index < 0)
            {
                index = 0;
            }

            // Return this for chaining
            return this;
        }

        /**
         * Calls a function on the current instruction in the stream.
         * @param func Function to be called. Should take an ASInstruction.
         * @return this (for chaining)
         */
        public function then(func:Function):InstructionStream
        {
            // Check range
            if ((!backwards && index >= instructions.length) ||
                (backwards && index <= 0))
            {
                throw new Error("Past the end of the stream");
            }

            // Actually apply function
            if (backwards)
            {
                func(instructions[index - 1]);
            }
            else
            {
                func(instructions[index]);
            }

            // Return this for chaining
            return this;
        }

        /**
         * Deletes a number of instructions from the stream, including the current one.
         * Throws an error if there aren't enough instructions to delete.
         * @param number Number of instructions to delete. Use -1 for "all remaining instructions".
         * @param ignoreDebug Whether deleteCount includes debug instructions. Ignored if -1 is specified there.
         * @return this (for chaining)
         */
        public function deleteNext(number:int, ignoreDebug:Boolean = true):InstructionStream
        {
            // Get number of instructions to delete
            if (number == -1)
            {
                if (backwards)
                {
                    number = index - 1;
                }
                else
                {
                    number = instructions.length - index;
                }
            }

            // Check range
            if (backwards)
            {
                if (index - 1 - number < 0)
                {
                    throw new Error("Could not delete " + number + " instructions: only " + index + " remain");
                }
            }
            else
            {
                if (index + number > instructions.length)
                {
                    throw new Error("Could not delete " + number + " instructions: only " + (instructions.length - index - 1) + " remain");
                }
            }

            // Fix number in ignoreDebug case
            if (ignoreDebug)
            {
                number = countNextIgnoreDebug(number);
            }

            // Actually splice out instructions
            if (backwards)
            {
                instructions.splice(index - 1 - number, number);
                // Correct index
                index -= number;
            }
            else
            {
                instructions.splice(index, number);
            }

            // Return this for chaining
            return this;
        }

        /**
         * Deletes instructions until a filter is true.
         * @param filter Function to apply to the instructions. Should take an ASInstruction and return a Boolean
         * @return this (for chaining)
         */
        public function deleteUntil(filter:Function):InstructionStream
        {
            var prevIndex:int = index;
            findNext(filter);
            var newIndex:int = index;

            var deleteMe:int = newIndex - prevIndex;
            if (deleteMe < 0)
            {
                deleteMe = -deleteMe;
            }

            backtrack(deleteMe);
            deleteNext(deleteMe, false);

            return this;
        }

        /**
         * Deletes instructions until a filter window is true.
         * @param ignoreDebug Whether to pass debug instructions to the filters or ignore them.
         * @param ...filters Array of functions to apply to the instructions. Each should take an ASInstruction and return a Boolean.
         * @return this (for chaining)
         */
        public function deleteUntilWindow(ignoreDebug:Boolean, ...filters:Array):InstructionStream
        {
            var prevIndex:int = index;
            filters.splice(0, 0, ignoreDebug);
            this.findNextWindow.apply(this, filters);
            var newIndex:int = index;

            var deleteMe:int = newIndex - prevIndex;
            if (deleteMe < 0)
            {
                deleteMe = -deleteMe;
            }

            backtrack(deleteMe);
            deleteNext(deleteMe, false);

            return this;
        }

        /**
         * Inserts instructions built by the passed-in function.
         * Inserts before (in forwards order) the current instruction, or, if past the end, at the end.
         * The current instruction will always be the same before and after an insert.
         * Note: if this is a backwards stream, it inserts the arguments *in the order passed*.
         * This means the final array will have them in the order you give, rather than backwards.
         * @param args Instructions to insert
         * @return this (for chaining)
         */
        public function insert(...args:Array):InstructionStream
        {
            // First argument, then delete count to splice
            if (backwards)
            {
                args.splice(0, 0, index == 0 ? 0 : index - 1);
            }
            else
            {
                args.splice(0, 0, index);
            }
            args.splice(1, 0, 0);

            instructions.splice.apply(instructions, args);

            // Advance index to proper index
            if (!backwards)
            {
                index += args.length - 2;
            }

            // Return this for chaining
            return this;
        }

        /**
         * Inserts and deletes instructions from the stream in a single step.
         * Follows the semantics of a deleteNext followed by an insert.
         * @param deleteCount Number to delete. Use -1 for "all remaining instructions".
         * @param ignoreDebug Whether deleteCount includes debug instructions. Ignored if -1 is specified there.
         * @param ...args Instructions to insert
         * @return this (for chaining)
         */
        public function splice(deleteCount:int, ignoreDebug:Boolean = true, ...args:Array):InstructionStream
        {
            // Get number of instructions to delete in the "rest of instructions" case
            if (deleteCount == -1)
            {
                if (backwards)
                {
                    deleteCount = index - 1;
                }
                else
                {
                    deleteCount = instructions.length - index;
                }
            }

            // Check range
            if (backwards)
            {
                if (index - 1 - deleteCount < 0)
                {
                    throw new Error("Could not delete " + deleteCount + " instructions: only " + index + " remain");
                }
            }
            else
            {
                if (index + deleteCount > instructions.length)
                {
                    throw new Error("Could not delete " + deleteCount + " instructions: only " + (instructions.length - index - 1) + " remain");
                }
            }

            // Fix number in ignoreDebug case
            if (ignoreDebug)
            {
                deleteCount = countNextIgnoreDebug(deleteCount);
            }

            // First argument to splice
            if (backwards)
            {
                args.splice(0, 0, index - 1 - deleteCount);
            }
            else
            {
                args.splice(0, 0, index);
            }

            // Delete count to splice
            args.splice(1, 0, deleteCount);

            // Actually apply splice
            instructions.splice.apply(instructions, args);

            // Advance index to proper index
            if (backwards)
            {
                index -= deleteCount;
            }
            else
            {
                index += args.length - 2 - deleteCount;
            }

            // Return this for chaining
            return this;
        }
    }
}
