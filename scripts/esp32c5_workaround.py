# scripts/esp32c5_workaround.py
from SCons.Script import Import
from os.path import join, abspath, basename
import os, shutil

Import("env")

print(">>> [esp32c5_workaround] enabling RISC-V for ESP32-C5 (spawn hook)")

# 0) assicura PATH con riscv32-esp-elf-*
def _ensure_riscv_in_path():
    # prova prima il pacchetto PIO (se esiste)
    pkg = env.PioPlatform().get_package_dir("toolchain-riscv32-esp")
    if pkg:
        bindir = join(pkg, "bin")
        env.PrependENVPath("PATH", bindir)
        print("RISC-V toolchain (PIO) ->", bindir)
    # se non c'è, prova a pescare da ESP-IDF tools (~/.espressif/tools/...)
    # oppure lascia che 'which' lo trovi già nel PATH dell'utente
    found = shutil.which("riscv32-esp-elf-gcc", path=env["ENV"].get("PATH", ""))
    if not found:
        home = os.path.expanduser("~")
        espressif_tools = os.path.join(home, ".espressif", "tools", "riscv32-esp-elf")
        if os.path.isdir(espressif_tools):
            # pick the latest subdir that contains bin
            candidates = []
            for root, dirs, files in os.walk(espressif_tools):
                if os.path.basename(root) == "bin" and "riscv32-esp-elf-gcc.exe" in files or "riscv32-esp-elf-gcc" in files:
                    candidates.append(root)
            if candidates:
                candidates.sort()
                bindir2 = candidates[-1]
                env.PrependENVPath("PATH", bindir2)
                print("RISC-V toolchain (IDF) ->", bindir2)

_ensure_riscv_in_path()

# passa il target a CMake/IDF
env.Append(ENV={"IDF_TARGET": "esp32c5"})

# mappa dei binari da “xtensa-esp32c5-elf-*” a “riscv32-esp-elf-*”
TOOLS_MAP = {
    "xtensa-esp32c5-elf-gcc":     "riscv32-esp-elf-gcc",
    "xtensa-esp32c5-elf-g++.exe": "riscv32-esp-elf-g++",   # tollerante a .exe
    "xtensa-esp32c5-elf-g++":     "riscv32-esp-elf-g++",
    "xtensa-esp32c5-elf-ar":      "riscv32-esp-elf-ar",
    "xtensa-esp32c5-elf-as":      "riscv32-esp-elf-as",
    "xtensa-esp32c5-elf-objcopy": "riscv32-esp-elf-objcopy",
    "xtensa-esp32c5-elf-objdump": "riscv32-esp-elf-objdump",
    "xtensa-esp32c5-elf-ranlib":  "riscv32-esp-elf-ranlib",
    "xtensa-esp32c5-elf-size":    "riscv32-esp-elf-size",
    "xtensa-esp32c5-elf-gdb":     "riscv32-esp-elf-gdb",
    "xtensa-esp32c5-elf-gcc.exe": "riscv32-esp-elf-gcc",
}

# wrapper sullo SPAWN: intercetta la exec e sostituisce il comando se serve
_orig_spawn = env["SPAWN"]

def _spawn_hook(sh, escape, cmd, args, envp):
    base = basename(cmd).lower()
    # normalizza anche args[0]
    if args:
        a0 = basename(str(args[0])).lower()
    else:
        a0 = ""

    replaced = None
    if base in TOOLS_MAP:
        new = TOOLS_MAP[base]
        cmd = new
        if args:
            args = list(args)
            args[0] = new
        replaced = (base, new)
    elif a0 in TOOLS_MAP:
        new = TOOLS_MAP[a0]
        if args:
            args = list(args)
            args[0] = new
            cmd = new
        replaced = (a0, new)

    if replaced:
        print(f">>> [esp32c5_workaround] swap '{replaced[0]}' -> '{replaced[1]}'")

    return _orig_spawn(sh, escape, cmd, args, envp)

env["SPAWN"] = _spawn_hook