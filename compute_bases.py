import re

def compute_size(path, start_marker=None):
    with open(path, 'r') as f:
        lines = f.readlines()

    # Find member variables section - look for TypedStorage lines
    # Skip inner types by finding the main member block
    start_idx = 0
    if start_marker:
        for i, line in enumerate(lines):
            if start_marker in line:
                start_idx = i
                break

    offset = 0
    found_any = False
    for i in range(start_idx, len(lines)):
        line = lines[i]
        m = re.search(r'::ll::TypedStorage<(\d+),\s*(\d+)', line)
        if not m:
            continue
        found_any = True
        align = int(m.group(1))
        size = int(m.group(2))
        if offset % align != 0:
            offset += align - (offset % align)
        offset += size

    return offset

base = 'C:/Users/Edward/AppData/Local/Temp/levilamina/src/mc/world/actor'

# Actor - find first TypedStorage that's a direct member
actor_size = compute_size(f'{base}/Actor.h')
print(f'Actor own fields size: 0x{actor_size:X}')

mob_size = compute_size(f'{base}/Mob.h')
print(f'Mob own fields size: 0x{mob_size:X}')

player_size = compute_size(f'{base}/player/Player.h')
print(f'Player own fields size: 0x{player_size:X}')

# Actor has EnableNonOwnerReferences base (vtable 8 + shared_ptr 16 = 24 bytes = 0x18)
# Plus its own vtable pointer
actor_base = 0x8 + 0x10  # vtable + EnableNonOwnerReferences control block
print(f'\nActor base overhead (vtable + EnableNonOwnerReferences): 0x{actor_base:X}')
print(f'Actor total: 0x{actor_base + actor_size:X}')
print(f'Actor+Mob total: 0x{actor_base + actor_size + mob_size:X}')

total = actor_base + actor_size + mob_size + 0x608
print(f'\nmGameMode absolute offset (base + Player 0x608): 0x{total:X}')
