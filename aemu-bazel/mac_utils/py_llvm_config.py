#!/usr/bin/env python3
import os
import sys
import subprocess

from pathlib import Path

def main():
    # Set paths relative to this script
    script_dir = Path(__file__).resolve().parent
    # Workspace root is 4 levels up
    workspace_root = script_dir.parents[3]

    if sys.argv[1:] == ["--version"]:
        print("21.1.8")
        sys.exit(0)

    candidates = [
        workspace_root / "prebuilts/clang/host/darwin-x86/clang-r614150/bin/llvm-config",
        workspace_root / "prebuilts/clang/host/darwin-x86/clang-r584948b/bin/llvm-config",
    ]
    tool_path = None
    for cand in candidates:
        if cand.exists():
            tool_path = str(cand.resolve())
            break

    if not tool_path:
        print(f"Error: Cannot find llvm-config in candidates: {candidates}", file=sys.stderr)
        sys.exit(1)

    try:
        result = subprocess.run([tool_path] + sys.argv[1:], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        print(result.stdout, end='')
    except subprocess.CalledProcessError as e:
        print(e.stderr, file=sys.stderr, end='')
        sys.exit(e.returncode)

if __name__ == "__main__":
    main()
