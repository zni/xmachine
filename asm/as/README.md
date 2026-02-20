# binutils pdp11-aout

## Building

- Grab a binutils release (I'm using GNU binutils 2.45 as of 2025-10-05).
- Compile and install it:
```
./configure --target=pdp11-aout --prefix=$HOME/opt/pdp11-cross
make
make install
```

Then:

```
$ pdp11-aout-as -aln -m11/40 <asm> -o <output file>
```
