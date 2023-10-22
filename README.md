# xmachine

Take a dash of PDP-11 instructions, while borrowing some of its registers, 
and add in a few other made up things. What do you get? This. This stuff.

## The goal

While away my waking hours keeping myself amused.

## What?

Oh right. Building this "thing". You'll need `clang` and `make`.
Armed with those tools, run the following:

```
make all
```

You'll end up with a newly created `bin/` directory containing this stuff.

## Alright... now what?

With `bin/machine` and `bin/asm` now you can do some stuff. Namely, build
some machine code programs and run them with `bin/machine`.

### asm

`asm` exists to parse text files of machine code and turn them into binaries.
I could have just parsed the text files and loaded them, but I wanted an extra step.
Anywho... to build `test/mov.mcode` for example:

```
bin/asm test/mov.mcode testbin/mov.bin
```

basically:

```
bin/asm <source> <output binary>
```

The format of the input `.mcode` files is as follows:

```
<comments starting with ; (no leading spaces)>
<machine code instructions in octal (C notation, leading 0 to indicate octal): 0xxxxxx>
.end
```

The `.end` as the last line with no newline following it is necessary, because I am a lazy 
man and it was the simplest way to tell the program to stop.

The format of the output file is as follows:

```
<16-bits number of program instructions> <16-bit instruction 01> ... <16-bit instruction n>
```

### machine

With `testbin/mov.bin` built, now it just needs to be run by `machine`:

```
bin/machine testbin/mov.bin
```

which will run and dump its state when it halts. Something like:

```
% bin/machine testbin/jmp.bin
PSW: 0o000000
IR : 0o000000
PC : 0o000007
MAR: 0o000000
MBR: 0o000007
ALU: 0o000000
SP : 0o000000
R0 : 0o000007
R1 : 0o000000
R2 : 0o000000
R3 : 0o000000
R4 : 0o000000
0o012700 0o000007 0o000100
```

except the output of `mov.bin`, the above is `jmp.bin`.

### Why?

Bit operations are fun, probably because I don't get to do much of that stuff in my 
working life. Something about working on simple virtual machines is fun. Probably because 
(in the simple cases at least) there's not much time investment until you have a program 
that does your limited bidding courtesy of a program written in its language. And finally,
getting used to octal is interesting. I know there's not much to get used to, but having 
lived my life never having to think about it outside of file permissions, it's a change
from hexadecimal.

This will probably never be finished, or useful. The more I end up working on these things,
I don't think that was the goal anyway. In any case, this might morph into more of a 
PDP-11 as time goes on. Right now I'm just borrowing its opcode format and some registers,
while having fun.

## References

- Michael Singer, PDP-11. Assembler Language Programming and Machine Organization, John Wiley & Sons, NY: 1980.
    - This helped me out with immediate addressing, as the Wikipedia article, while pretty 
    good, is a little unclear for someone who never programmed a PDP-11 in their lifetime.