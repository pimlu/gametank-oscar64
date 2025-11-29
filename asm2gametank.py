#!/usr/bin/env python3
"""
Convert oscar64 .asm output to a GameTank ROM file.

GameTank ROM format:
- 128 banks × 16KB = 2MB total
- Bank 127 (oscar64 bank 63) is fixed at $C000-$FFFF
- Banks 0-126 (oscar64 banks 62 down) appear at $8000-$BFFF

Usage: python3 asm2gametank.py input.asm output.bin
"""

import sys
import re

def parse_asm_file(filename):
    """Parse oscar64 .asm file and extract bank data."""
    banks = {}  # bank_num -> {address: [bytes]}
    
    with open(filename, 'r') as f:
        for line in f:
            # Match lines like: 3f:c000 : a9 ef __ LDA #$ef
            # Format: BB:AAAA : XX XX XX ...
            match = re.match(r'^([0-9a-f]{2}):([0-9a-f]{4}) : ((?:[0-9a-f_]{2} )*)', line, re.IGNORECASE)
            if match:
                bank = int(match.group(1), 16)
                addr = int(match.group(2), 16)
                byte_str = match.group(3).strip()
                
                # Parse bytes (skip '__' placeholders)
                bytes_data = []
                for b in byte_str.split():
                    if b != '__':
                        bytes_data.append(int(b, 16))
                
                if bytes_data:
                    if bank not in banks:
                        banks[bank] = {}
                    banks[bank][addr] = bytes_data
            
            # Also match BYT data lines: 3f:fffa : __ __ __ BYT 0c c1 00 c0 17 c1
            match = re.match(r'^([0-9a-f]{2}):([0-9a-f]{4}) : [_0-9a-f ]+BYT (.+?)(?:\s+:.*)?$', line, re.IGNORECASE)
            if match:
                bank = int(match.group(1), 16)
                addr = int(match.group(2), 16)
                byte_str = match.group(3).strip()
                
                bytes_data = []
                for b in byte_str.split():
                    if b != '__':
                        bytes_data.append(int(b, 16))
                
                if bytes_data:
                    if bank not in banks:
                        banks[bank] = {}
                    banks[bank][addr] = bytes_data
    
    return banks

def create_gametank_rom(banks, output_filename):
    """Create a GameTank ROM file from parsed bank data."""
    
    # GameTank has 128 banks, but we only use oscar64 banks 62 and 63
    # mapped to GameTank banks 126 and 127
    
    BANK_SIZE = 0x4000  # 16KB per bank
    NUM_BANKS = 128
    
    # Initialize ROM with 0xFF (unprogrammed flash)
    rom = bytearray([0xFF] * (NUM_BANKS * BANK_SIZE))
    
    # Map oscar64 banks to GameTank banks
    # oscar64 bank 62 -> GameTank bank 126 (bankable region, $8000-$BFFF)
    # oscar64 bank 63 -> GameTank bank 127 (fixed region, $C000-$FFFF)
    oscar_to_gametank = {
        62: 126,
        63: 127,
    }
    
    for oscar_bank, data in banks.items():
        if oscar_bank not in oscar_to_gametank:
            print(f"Warning: oscar64 bank {oscar_bank} not mapped, skipping")
            continue
            
        gt_bank = oscar_to_gametank[oscar_bank]
        
        for addr, bytes_data in data.items():
            # Calculate offset within the bank
            # Bankable banks (0-126): code at $8000-$BFFF -> offset 0-0x3FFF
            # Fixed bank (127): code at $C000-$FFFF -> offset 0-0x3FFF
            
            if gt_bank == 127:
                # Fixed bank: $C000-$FFFF
                if addr < 0xC000 or addr >= 0x10000:
                    print(f"Warning: Address {addr:04X} out of range for fixed bank")
                    continue
                offset_in_bank = addr - 0xC000
            else:
                # Bankable banks: $8000-$BFFF
                if addr < 0x8000 or addr >= 0xC000:
                    print(f"Warning: Address {addr:04X} out of range for bankable bank")
                    continue
                offset_in_bank = addr - 0x8000
            
            # Calculate ROM offset
            rom_offset = gt_bank * BANK_SIZE + offset_in_bank
            
            # Write bytes
            for i, b in enumerate(bytes_data):
                if rom_offset + i < len(rom):
                    rom[rom_offset + i] = b
    
    # Write ROM file
    with open(output_filename, 'wb') as f:
        f.write(rom)
    
    print(f"Created {output_filename} ({len(rom)} bytes)")
    
    # Print summary
    print("\nBank usage:")
    for oscar_bank in sorted(banks.keys()):
        gt_bank = oscar_to_gametank.get(oscar_bank, "?")
        addrs = sorted(banks[oscar_bank].keys())
        if addrs:
            print(f"  Oscar64 bank {oscar_bank} -> GameTank bank {gt_bank}: "
                  f"${addrs[0]:04X}-${addrs[-1]:04X}")

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.asm output.bin")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"Parsing {input_file}...")
    banks = parse_asm_file(input_file)
    
    print(f"Found banks: {sorted(banks.keys())}")
    
    create_gametank_rom(banks, output_file)

if __name__ == '__main__':
    main()

