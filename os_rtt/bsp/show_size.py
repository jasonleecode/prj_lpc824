#!/usr/bin/env python3
"""
Show Flash and RAM usage percentage after compilation
"""
import sys
import subprocess

if len(sys.argv) < 2:
    print("Usage: show_size.py <elf_file>")
    sys.exit(1)

elf_file = sys.argv[1]

# Get size information
try:
    result = subprocess.run(['arm-none-eabi-size', elf_file], 
                          capture_output=True, text=True)
    lines = result.stdout.strip().split('\n')
    if len(lines) >= 2:
        # Parse the last line (actual data)
        parts = lines[-1].split()
        text = int(parts[0])
        data = int(parts[1])
        bss = int(parts[2])
    else:
        print("Error: Could not parse size output")
        sys.exit(1)
except Exception as e:
    print(f"Error getting size: {e}")
    sys.exit(1)

# LPC824 memory configuration
FLASH_SIZE = 32 * 1024  # 32 KB
RAM_SIZE = 8 * 1024     # 8 KB

# Calculate usage
flash_used = text + data  # Code and initialized data go to Flash
ram_used = data + bss     # Initialized data and BSS go to RAM

# Calculate percentages
flash_percent = (flash_used * 100.0) / FLASH_SIZE
ram_percent = (ram_used * 100.0) / RAM_SIZE

# Display results
print()
print("╔════════════════════════════════════════╗")
print(f"║ Flash: {flash_used:6d}/{FLASH_SIZE:6d} bytes ({flash_percent:5.1f}%) ║")
print(f"║ RAM:   {ram_used:6d}/{RAM_SIZE:6d} bytes ({ram_percent:5.1f}%) ║")
print("╚════════════════════════════════════════╝")
print()
