import re
path = 'C:/Users/Edward/AppData/Local/Temp/levilamina/src/mc/world/actor/player/Player.h'
with open(path, 'r') as f:
    lines = f.readlines()
start_idx = None
for i, line in enumerate(lines):
    if 'mFroglights' in line and 'TypedStorage' in line:
        start_idx = i
        break
if start_idx is None:
    print('no mFroglights')
    exit(1)
offset = 0
for i in range(start_idx, len(lines)):
    line = lines[i]
    m = re.search(r'::ll::TypedStorage<(\d+),\s*(\d+)', line)
    if not m:
        continue
    align = int(m.group(1))
    size = int(m.group(2))
    if offset % align != 0:
        offset += align - (offset % align)
    if 'mGameMode' in line:
        print(f'mGameMode at Player-own offset: 0x{offset:X}  size={size}')
        break
    offset += size
else:
    print(f'mGameMode not found, last offset: 0x{offset:X}')
