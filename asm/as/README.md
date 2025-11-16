# What?

Alright, so this is PDP-11 in GNU assembler syntax.

To assemble this stuff, just:

- Grab a binutils release (I'm using GNU binutils 2.45 as of 2025-10-05).
- Build it like this:

```
./configure --target=pdp11-aout --prefix=$HOME/opt/pdp11-cross
make
make install
```

Then:

```
$ pdp11-aout-as <asm> -o <output file>
```
