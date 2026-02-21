# GNU Assembler

In here are the macro11 examples translated to GNU Assembler, targeting pdp11-aout.

To assemble these files you'll need to build binutils for the pdp11-aout target.

I'm using GNU binutils 2.45 as of 2025-10-05.

## Building binutils
```
$ ./configure --target=pdp11-aout --prefix=$HOME/opt/pdp11-cross
$ make
$ make install
```

## Assembling the source
```
$ pdp11-aout-as -aln -m11/40 <asm> -o <output file>
```
