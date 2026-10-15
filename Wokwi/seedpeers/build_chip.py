import os
import subprocess
import shutil
from pathlib import Path

Import("env")

DIST_DIR = Path("dist")
DIST_DIR.mkdir(parents=True, exist_ok=True)

CHIP_SRC = "src/D4184-mosfet.chip.c"
WASM_OUT = "dist/D4184-mosfet.chip.wasm"
JSON_SRC = "D4184-mosfet.chip.json"
JSON_OUT = "dist/D4184-mosfet.chip.json"

def compile_chip():
    if os.path.exists(JSON_SRC):
        shutil.copy(JSON_SRC, JSON_OUT)

    cmd = [
        "clang",
        "--target=wasm32-unknown-wasi",
        "-nostartfiles",
        "-nodefaultlibs",
        "-lc",
        "-Wl,--import-memory",
        "-Wl,--export-table",
        "-Wl,--no-entry",
        "-O3",
        "-Isrc",
        "-o", WASM_OUT,
        CHIP_SRC
    ]
    
    print(f"\n[WOKWI CHIP BUILD] Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"[WOKWI CHIP BUILD ERROR]:\n{result.stderr}")
        env.Exit(1)
    else:
        print("[WOKWI CHIP BUILD] Successfully compiled to dist/!\n")

compile_chip()