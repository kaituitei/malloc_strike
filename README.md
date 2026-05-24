# Orbital Strike Malloc ☄️

This program's purpose is to check for any improper malloc handling by replacing the real malloc with a custom malloc. After that, you can set the behavior of malloc as you wish.

### Behavior Setting Options

In `malloc_strike.c`, you can change these settings to configure malloc behavior:

```c
#define MAX_MALLOC_CALLS    10000       // maximum number of malloc calls
#define MAX_MALLOC_BYTES    1000000     // maximum number of bytes allocated (1MB default)
#define MALLOC_ID_FAIL      -1UL        // forces the malloc of that ID to fail (-1UL to unset)
#define PRINT_WARNINGS      true        // enable warnings (not necessarily a fail)
#define PRINT_CALLS         true        // print all malloc calls
```

## Quick Start

### Option 1: Create Your Own malloc_strike File

You can create your own `malloc_strike.c` file, put the source code into it, and compile it with your target file.

### Option 2: Git Clone Option

Run this command in your root repository to clone the malloc_strike file into your root directory:

```bash
git clone git@github.com:kaituitei/malloc_strike.git && cd malloc_strike
mv malloc_strike.c ..
cd ..
rm -rf malloc_strike
```

### Repository Structure

```
.
├── malloc_strike.c
└── test.c
```

### Usage

Compile `malloc_strike.c` with your target program:

```bash
cc -Wall -Werror -Wextra your_program.c malloc_strike.c
```

*Replace `your_program.c` with your target file name.*

### Examples

**Normal Situation** - Without malloc size limits:

![Normal situation](https://github.com/kaituitei/malloc_strike/blob/main/img/Screenshot%202569-05-24%20at%2016.54.39.png)

**Malloc Strike** - With malloc size limits applied:

![Malloc strike](https://github.com/kaituitei/malloc_strike/blob/main/img/Screenshot%202569-05-24%20at%2016.54.56.png)
