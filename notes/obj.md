# OBJ Files

The manual that contains the format of RT-11 OBJ files is:

    RT–11 Volume and File Formats Manual

The basic gist is that an OBJ file is made up of data blocks, like:

    01 00       Data Block Indicator
    0000        Length
    00          Block Type
    ...         Data
    00          Checksum

Where all data blocks begin with the bytes 01 00. An object file is made up of multiple of these.
Next, a word (2 bytes) of the length of the block. This length is the number of bytes in the block.
The indicator, length, block type, and checksum are all a part of the length along with the data.

GSD blocks have a block type of 01. TEXT blocks have a block type of 03.

For my purposes right now, I'm mostly interested in TEXT blocks, as they are the machine code listing.

Each block has its own format after the block type. For TEXT blocks, the format is roughly:

    03          Block Type
    0000        Load Offset
    00          Pad?
    ...         Instruction words