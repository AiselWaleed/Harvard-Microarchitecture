# Harvard Microarchitecture Simulator

A C-based simulator for a Harvard-style, three-stage pipelined processor. The project parses a simple assembly-like language, stores the encoded instructions in instruction memory, and executes them through `FETCH`, `DECODE`, and `EXECUTE` stages.

The simulator prints detailed execution information for every cycle, including decoded operands, ALU results, register and data-memory changes, control-hazard flushes, condition flags, and the final processor state.

## Features

- Three-stage pipeline: `IF` (fetch), `ID` (decode), and `IE` (execute).
- Separate instruction and data memory arrays.
- 64 general-purpose registers, `R0` through `R63`.
- Eight-bit signed register and data values (`int8_t`).
- Sixteen-bit instruction words.
- Five status flags: carry (`C`), overflow (`V`), negative (`N`), sign (`S`), and zero (`Z`).
- Relative conditional branches and register-based unconditional branches.
- Control-hazard flushing when a branch is taken.
- Sample programs in the `programs/` directory.

## Repository Layout

```text
.
|-- include/
|   |-- alu.h          ALU and status-flag declarations
|   |-- memory.h       Register and memory declarations
|   |-- parser.h       Assembly parser declarations
|   `-- pipeline.h     Pipeline-stage declarations
|-- programs/          Assembly-like input programs
|-- src/
|   |-- alu.c          ALU operations and flag updates
|   |-- memory.c       Instruction memory, data memory, and registers
|   |-- parser.c       Source parsing and instruction encoding
|   `-- pipeline.c     Pipeline control and program entry point
|-- bin/               Compiled executable output
|-- Makefile           Reserved for build automation
`-- README.md          Project documentation
```

## Requirements

You need a C compiler with support for C99 or later. GCC or Clang is recommended. On Windows, the current implementation uses `Sleep` from the Windows API to pause for one second between cycles. On non-Windows systems, `SLEEP_SECOND` is not defined yet, so portability requires adding an equivalent sleep implementation in `include/pipeline.h`.

The ALU uses functions from the math library, so the program must be linked with `-lm` when using GCC or Clang.

## Build

The checked-in `Makefile` is currently empty. Build the simulator directly from the repository root:

```bash
gcc -std=c99 -Wall -Wextra -Iinclude src/*.c -lm -o bin/program.exe
```

On Windows with MinGW, run the same command in PowerShell, Git Bash, or a MinGW terminal. The executable is written to `bin/program.exe`.

## Run

Run the executable from the repository root:

```bash
./bin/program.exe
```

On PowerShell:

```powershell
.\bin\program.exe
```

The current entry point loads `program5.txt` unconditionally:

```c
loadProgram("program5.txt");
```

Because the parser resolves a bare filename relative to `programs/`, the simulator must be launched from the repository root. To run another sample, change the filename in `src/pipeline.c`, rebuild, and run again. A path containing `/` or `\\` can also be passed directly to `loadProgram` in source code.

## Instruction Set

Each instruction is encoded as a 16-bit word:

```text
15                12 11        6 5         0
+-------------------+-----------+-----------+
|      opcode       |    R1     | R2 / imm  |
+-------------------+-----------+-----------+
```

`R1` and register-form `R2` are six-bit register numbers. Immediate values occupy six bits and are sign-extended for instructions that use signed immediates.

| Mnemonic | Opcode | Format           | Operation                                                 |
| -------- | -----: | ---------------- | --------------------------------------------------------- |
| `ADD`    |      0 | `ADD R1 R2`      | `R1 = R1 + R2`                                            |
| `SUB`    |      1 | `SUB R1 R2`      | `R1 = R1 - R2`                                            |
| `MUL`    |      2 | `MUL R1 R2`      | `R1 = R1 * R2`                                            |
| `MOVI`   |      3 | `MOVI R1 imm`    | `R1 = imm`                                                |
| `BEQZ`   |      4 | `BEQZ R1 imm`    | If `R1 == 0`, branch relative to the current instruction  |
| `ANDI`   |      5 | `ANDI R1 imm`    | `R1 = R1 & imm`                                           |
| `EOR`    |      6 | `EOR R1 R2`      | `R1 = R1 XOR R2`                                          |
| `BR`     |      7 | `BR R1 R2`       | Branch to the address formed from the two register values |
| `SLC`    |      8 | `SLC R1 imm`     | Circular left shift of `R1`                               |
| `SRC`    |      9 | `SRC R1 imm`     | Circular right shift of `R1`                              |
| `LDR`    |     10 | `LDR R1 address` | `R1 = data_memory[address]`                               |
| `STR`    |     11 | `STR R1 address` | `data_memory[address] = R1`                               |

### Operand and value rules

- Registers are numbered from `R0` to `R63`.
- `R0` is treated as read-only by execution instructions. Writes to `R0` are rejected.
- Normal immediates are six-bit signed values from `-32` through `31`.
- Shift amounts are masked to three bits by the ALU, giving an effective range of `0` through `7`.
- Arithmetic results are stored as eight-bit values. Values outside the signed eight-bit range wrap according to C integer conversion behavior.
- `BEQZ` uses the ALU result `PC + 1 + immediate` when the register is zero.
- `BR` combines the two eight-bit register operands as `(R1 << 8) | R2` to form a branch address.

## Program File Format

Program files contain one instruction per line. Operands are separated by spaces, and register names use an uppercase `R` followed by a decimal number.

```text
# Comments must begin in the first column.
MOVI R1 10
MOVI R2 20
ADD R1 R2
STR R1 50
LDR R3 50
```

Blank lines and lines whose first character is `#` are ignored. Instruction mnemonics are currently matched as uppercase strings. Malformed operands can cause parser errors, so use the exact formats in the instruction table.

## Pipeline Behavior

The simulator advances instructions through three pipeline buffers:

1. `FETCH` reads the instruction at `PC` and increments `PC`.
2. `DECODE` extracts the opcode, registers, immediate, and register values.
3. `EXECUTE` invokes the ALU or performs a memory/branch operation.

When a branch is taken, the simulator invalidates the `IF` and `ID` buffers. This flushes instructions that were fetched along the wrong path before continuing at the new program counter. The console labels these events as control hazards.

Execution pauses for one second after each cycle on Windows, making the pipeline transitions easier to observe. The pause can be removed or changed in `include/pipeline.h`.

## Output

During execution, the program reports:

- The current cycle and pipeline stage.
- The fetched instruction and its binary representation.
- Decoded opcode, registers, values, and immediates.
- ALU results and branch decisions.
- Non-zero registers and data-memory locations after execution.
- Status flags in the format `C`, `V`, `N`, `S`, and `Z`.

After execution, it prints the complete register file, non-zero data-memory entries, and the encoded instruction memory.

## Sample Programs

The `programs/` directory contains `program1.txt` through `program11.txt`, plus `programx.txt`. `program1 explanation.txt` documents an example program instruction by instruction. Some samples intentionally exercise branches, memory operations, shifts, or pipeline flushing.

Inspect branch programs carefully: an unconditional `BR` can jump back to an earlier instruction and create an infinite loop.

## Current Limitations

- The input program is selected by editing and rebuilding `src/pipeline.c`; there is no command-line argument yet.
- The `Makefile` does not currently define build targets.
- Data-hazard detection is declared in `pipeline.h` but is not implemented or used by the current pipeline loop.
- Only control hazards caused by taken branches are flushed.
- Memory access bounds are not checked before indexing data memory.
- The non-Windows sleep path is not implemented.
- The simulator has no cycle limit, so a program with a backward unconditional branch may run indefinitely.
- Parser diagnostics print warnings for some out-of-range values, but do not prevent every invalid instruction from being encoded.

## Development Notes

The main extension points are:

- Add or change instruction semantics in `src/alu.c` and `src/pipeline.c`.
- Change instruction parsing and encoding in `src/parser.c`.
- Change register, PC, and memory behavior in `src/memory.c`.
- Add command-line program selection in `main` or in `run_program`.
- Add build targets to the root `Makefile` for repeatable compilation.

When adding an instruction, update the parser, decode logic, execute logic, ALU behavior if needed, and this instruction table together.
