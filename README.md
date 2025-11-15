# xmachine

A PDP-11/40 emulator (eventually).

## The goal

To while away my waking hours.

## What?

Oh, right. Building this "thing". You'll need `gcc` and `make`.
Armed with those tools, run the following:

```
make all
```

You'll end up with a newly created `bin/` directory containing some stuff.

## Alright... now what?

With `bin/bus`, `bin/cpu`, and `bin/mem` built you can now do some stuff... eventually.

Not now though.

If you want you can watch the priority bus interactions between the bus arbitrator and the cpu.

```
$ bin/mem &
$ bin/bus &
$ bin/cpu &
```

Failure to start the devices in that order will probably break things right now.


## Why?

Bit operations are fun, probably because I don't get to do much of that stuff in my
working life. Something about working on simple emulators is fun. Probably because
(in the simple cases at least) there's not much time investment until you have a program
that does your limited bidding courtesy of a program written in its language. And finally,
getting used to octal is interesting. I know there's not much to get used to, but having
lived my life never having to think about it outside of file permissions, it's a change
from hexadecimal.

This will probably never be finished, or useful. The more I end up working on these things,
I don't think that was the goal anyway.

## References

- Michael Singer, PDP-11. Assembler Language Programming and Machine Organization, John Wiley & Sons, NY: 1980.
    - This helped me out with immediate addressing, as the Wikipedia article, while pretty
    good, is a little unclear for someone who never programmed a PDP-11 in their lifetime.

### DEC Manuals
You can find most of these on bit savers or bit saver mirrors.

- EK-RX01-0P-001 RX8/RX11 floppy disk system user's manual, Digital Equipment Corporation: 1976.
- PDP11/40 Processor Handbook, Digital Equipment Corporation: 1972.
- DEC-11-HIAA-D Unibus Interface Manual, Digital Equipment Corporation: 1970.
- PDP11 Peripherals Handbook, Digital Equipment Corporation: 1976.
- RT-11 Volume and File Formats Manual, Digital Equipment Corporation: 1991.

