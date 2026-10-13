import json
import sys

sys.stdout.reconfigure(encoding='utf-8')

with open(r'c:\Users\Asus\Downloads\SCH_New-Project_2026-10-13.json', 'r', encoding='utf-8') as f:
    data = json.load(f)

for sch in data.get('schematics', []):
    dataStr = sch.get('dataStr', {})
    shapes = dataStr.get('shape', [])
    for idx, s in enumerate(shapes):
        if s.startswith('W~'):
            pts = s.split('~')[1].split(' ')
            coords = [(float(pts[i]), float(pts[i+1])) for i in range(0, len(pts), 2)]
            for i in range(len(coords)-1):
                p1, p2 = coords[i], coords[i+1]
                x1, y1 = p1
                x2, y2 = p2
                if min(x1, x2) <= 300 <= max(x1, x2) and min(y1, y2) <= -370 <= max(y1, y2):
                    print(f"Wire {idx} passes through (300, -370):", s)
        elif s.startswith('F~'):
            if '300' in s or '-370' in s:
                print(f"Label {idx}:", s)
