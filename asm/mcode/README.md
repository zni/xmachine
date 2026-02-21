# PDP-11 Homebrew Machine Code Format

## Overview
When I was just starting (and before I found the MACRO-11 implementation on
github), I just wrote stuff in machine code and loaded it in.

Obviously, this was kind of a pain and not really optimal for anything beyond
simple examples.

## File Format
```
<machine code>
.end
```

Machine code lines are in octal.

