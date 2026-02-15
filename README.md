# xmachine

A PDP-11/40-ish emulator (eventually).

## Build

Get GNU Assembler setup to assemble PDP-11 assembly to aout.
Grab `binutils` and do:

```
$ mkdir -p ~/opt/pdp11-cross
$ ./configure --target=pdp11-aout --prefix=$HOME/opt/pdp11-cross
$ make
$ make install
$ cd <repo root>
$ mkdir local
$ ~/opt/pdp11-cross/bin/pdp11-aout-as asm/as/<file>.s -o local/<file>.aout
```

Then run the following:

```
$ cd xmachine
$ make all
```

## Running

With `src/cpu/cpu` and `src/mem/mem` built you can run a few of the examples in
`asm/as`:

```
$ bin/mem -f local/<file>.aout &
$ bin/cpu &
```

