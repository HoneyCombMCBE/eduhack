import re

path = 'C:/Users/Edward/AppData/Local/Temp/levilamina/src/mc/world/level/ILevel.h'
with open(path, 'r') as f:
    content = f.read()

lines = content.split('\n')

in_virtuals = False
index = 0
for line in lines:
    stripped = line.strip()
    if '// virtual functions' in stripped:
        in_virtuals = True
        continue
    if in_virtuals and '// NOLINTEND' in stripped:
        break
    if in_virtuals and stripped.startswith('virtual '):
        if 'getRuntimeActorList' in stripped:
            print(f'getRuntimeActorList at index {index} (0x{index*8:X})')
        index += 1

print(f'Total ILevel virtuals: {index}')
