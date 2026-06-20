# cs-seminar: Modern ROP Attacks

Demo code for a university seminar presentation on modern ROP (Return-Oriented
Programming) attacks. The repo is a collection of small, self-contained
vulnerable programs used live during the talk to show, progressively:

1. Binary representation / stack layout basics
2. Classic stack buffer overflow (no protections)
3. Stack canaries (and how they're defeated/bypassed)
4. NX / DEP (why shellcode injection stops working)
5. ASLR (why naive ROP breaks, and partial-overwrite / leak tricks)
6. Building a ROP chain (`ret2libc`, gadget chaining)
7. The same attack class on ARM vs x86_64 (calling convention / gadget
   differences)

This is offensive-security teaching material for an authorized academic
seminar. Keep everything self-contained, runnable only against the bundled
toy binaries, and clearly labeled as educational.

## Layout

One directory per concept, numbered to match the presentation narrative:

```
01-stack-layout/
02-buffer-overflow/
03-stack-canary/
04-nx-dep/
05-aslr/
06-ret2libc/
07-rop-chain/
08-arm-vs-x86/
...
```

Each example directory is independent and contains:

- `victim.c` — the vulnerable program (small, heavily commented for the
  *vulnerability*, not for C basics).
- `exploit.py` — the exploit, built with `pwntools`.
- `Dockerfile` — the reproducible build/run environment for that example
  (see below). If multiple examples share the exact same environment, they
  may share a Dockerfile one level up instead of duplicating it.
- `README.md` — slide-sized explanation: what protection is on/off, the
  exact compiler flags that matter, and the one-command way to reproduce
  the exploit.
- `Makefile` (inside the container) — builds `victim` with the flags called
  out in the README. Keep flags explicit and visible, never hidden in a
  configure script, since the flags themselves are the teaching point.

## Environment: Docker-based, per example

Reproducibility during a live talk matters more than elegance: the
presenter's host distro (modern Ubuntu/Fedora) enables hardening
(PIE, full RELRO, ASLR, stack canaries, sometimes CET) that fights every
demo. Don't fight the host — containerize.

- Each example's `Dockerfile` starts from a pinned base image (e.g.
  `ubuntu:22.04`), installs only what that example needs
  (`gcc`/`gdb`/`python3`/`pwntools`, plus `gcc-arm-linux-gnueabihf` and
  `qemu-user` for ARM examples), and disables/enables mitigations
  explicitly via compiler flags — not by patching the OS.
- ASLR for a *running* container is controlled at `docker run` time with
  `--security-opt seccomp=unconfined` is NOT how you disable ASLR — use
  `setarch -R` inside the container or `echo 0 > /proc/sys/kernel/randomize_va_space`
  if the container has the privilege (`--privileged` or
  `--cap-add=SYS_ADMIN`), and call this out explicitly in the example's
  README so it's an obvious teaching point, not hidden magic.
- ARM examples: cross-compile with `arm-linux-gnueabihf-gcc` (32-bit ARM)
  inside an x86_64 container, then run the resulting binary under
  `qemu-arm -L /usr/arm-linux-gnueabihf` for live gadget/exploit
  demonstration without needing real ARM hardware.
- Prefer one shared base Dockerfile (`docker/base.Dockerfile`) with the
  common toolchain, and thin per-example Dockerfiles that `FROM` it and add
  only what's different (mitigation flags, extra packages) — avoids
  reinstalling the whole toolchain per example while keeping each example's
  delta visible.
- Every example must be runnable with a single documented command, e.g.:
  ```
  docker build -t rop-02 02-buffer-overflow/
  docker run --rm -it --cap-add=SYS_ADMIN rop-02
  ```

## Compiler flags reference (keep visible in each README, don't bury)

| Protection | Disable flag | Enable flag |
|---|---|---|
| Stack canary | `-fno-stack-protector` | `-fstack-protector-all` |
| NX/DEP | `-z execstack` | `-z noexecstack` (default) |
| PIE/ASLR-relevant | `-no-pie -fno-pie` | `-pie -fpie` (default on modern gcc) |
| RELRO | `-z norelro` | `-z relro -z now` (full) |

Use `checksec` (binary or `pwntools`' `checksec`) in each README to show the
resulting protections on the built binary — this is the most slide-friendly
way to prove what's on/off.

## Conventions

- C examples: C11, compiled with `gcc`, no warnings suppressed except the
  ones strictly needed to keep the vulnerability intact (e.g.
  `-Wno-stack-protector` is irrelevant if canaries are off; don't suppress
  `-Wformat` etc. unless the example is specifically a format-string bug).
- Exploits: Python 3 + `pwntools`. Keep exploit scripts short and narratable
  — they'll be read aloud/line-by-line during the talk. Prefer explicit
  offsets computed via comments (`# saved RIP is at offset 0x48, found via
  cyclic pattern`) over magic numbers with no derivation shown.
- Every vulnerable binary takes input the same way (stdin or argv — pick one
  per example and say which in the README) so the audience isn't tracking
  incidental differences between examples.
- No network-facing services. Local-process exploitation only.
- Don't add real-world CVEs or production binaries; these are minimal
  programs built to demonstrate one mechanism each.

## Tooling expected to be present in containers

`gdb` (+ optionally `pwndbg` or `gef` for nicer live demo output), `objdump`,
`readelf`, `checksec`, `python3-pwntools`, `qemu-user` (ARM examples),
`gcc-arm-linux-gnueabihf` (ARM examples).

## What NOT to do

- Don't disable the container's isolation beyond what's needed to flip
  ASLR/mitigation flags for the demo.
- Don't write anything that scans, attacks, or targets systems outside the
  container — this is local, self-contained exploitation of toy binaries
  only.
- Don't add a generic "exploit framework" abstraction shared across
  examples — each example should be readable top-to-bottom on its own,
  since that's how it'll be presented.
