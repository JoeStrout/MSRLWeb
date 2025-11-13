#!/usr/bin/env python3
"""
API Coverage Checker for MSRLWeb
Scans raylib.h to find all APIs by module using module comment headers,
then checks which ones are wrapped in our code.
"""

import re
import sys
from pathlib import Path
from collections import defaultdict

# Map raylib module names to our source files
MODULE_FILES = {
    'core': 'src/RCore.cpp',
    'shapes': 'src/RShapes.cpp',
    'textures': 'src/RTextures.cpp',
    'text': 'src/RText.cpp',
    'audio': 'src/RAudio.cpp',
}

def extract_raylib_apis(raylib_h_path):
    """
    Extract all RLAPI function declarations from raylib.h, organized by module.
    Uses module comment headers to determine which module each function belongs to.
    """
    apis = defaultdict(set)

    try:
        with open(raylib_h_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        current_module = None
        module_pattern = re.compile(r'//.*\(Module:\s*(\w+)\)')
        rlapi_pattern = re.compile(r'RLAPI\s+\w+[\s\*]+(\w+)\s*\([^)]*\)\s*;')

        for line in lines:
            # Check if this line declares a new module
            module_match = module_pattern.search(line)
            if module_match:
                current_module = module_match.group(1)
                continue

            # If we're in a module we care about, look for RLAPI declarations
            if current_module and current_module in MODULE_FILES:
                rlapi_match = rlapi_pattern.search(line)
                if rlapi_match:
                    func_name = rlapi_match.group(1)
                    apis[current_module].add(func_name)

        return apis

    except FileNotFoundError:
        print(f"Error: Could not find raylib.h at {raylib_h_path}")
        sys.exit(1)

def extract_wrapped_apis(module_file_path):
    """Extract all wrapped APIs from a module .cpp file"""
    wrapped = set()

    try:
        with open(module_file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Find all raylibModule.SetValue("FunctionName", ...) calls
        pattern = r'raylibModule\.SetValue\("(\w+)",\s*i->GetFunc\(\)\)'
        matches = re.finditer(pattern, content)

        for match in matches:
            wrapped.add(match.group(1))

        return wrapped
    except FileNotFoundError:
        print(f"Warning: Could not find {module_file_path}")
        return set()

def main():
    # Find raylib.h
    raylib_h = Path("raylib/src/raylib.h")
    if not raylib_h.exists():
        print(f"Error: raylib.h not found at {raylib_h}")
        sys.exit(1)

    print("=" * 80)
    print("MSRLWeb API Coverage Report")
    print("=" * 80)
    print()

    # Extract all Raylib APIs by module
    raylib_apis = extract_raylib_apis(raylib_h)

    total_raylib = 0
    total_wrapped = 0
    total_missing = 0

    # Check each module
    for module, module_file in sorted(MODULE_FILES.items()):
        module_file_path = Path(module_file)

        print(f"Module: {module.upper()}")
        print("-" * 80)

        raylib_funcs = raylib_apis.get(module, set())
        wrapped_funcs = extract_wrapped_apis(module_file_path) if module_file_path.exists() else set()
        missing_funcs = raylib_funcs - wrapped_funcs

        total_raylib += len(raylib_funcs)
        total_wrapped += len(wrapped_funcs)
        total_missing += len(missing_funcs)

        print(f"Raylib APIs found: {len(raylib_funcs)}")
        print(f"APIs wrapped: {len(wrapped_funcs)}")
        print(f"APIs missing: {len(missing_funcs)}")

        if missing_funcs:
            print(f"\nMissing APIs in {module}:")
            for func in sorted(missing_funcs):
                print(f"  - {func}")
        else:
            print(f"\n✓ All {module} APIs are wrapped!")

        # Also report wrapped APIs not found in raylib.h (possibly deprecated or custom)
        extra_funcs = wrapped_funcs - raylib_funcs
        if extra_funcs:
            print(f"\nWrapped but not found in raylib.h (may be custom/deprecated):")
            for func in sorted(extra_funcs):
                print(f"  - {func}")

        print()

    # Summary
    print("=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print(f"Total Raylib APIs identified: {total_raylib}")
    print(f"Total APIs wrapped: {total_wrapped}")
    print(f"Total APIs missing: {total_missing}")

    if total_missing == 0:
        print("\n✓ All identified APIs are wrapped!")
    else:
        coverage = (total_wrapped / total_raylib * 100) if total_raylib > 0 else 0
        print(f"\nCoverage: {coverage:.1f}%")

if __name__ == '__main__':
    main()
