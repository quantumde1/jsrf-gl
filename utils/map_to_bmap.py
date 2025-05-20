import struct
import sys

MAGIC = b'BMAP'
VERSION = 1

def write_string(writer, s):
    encoded = s.encode('utf-8')
    writer.write(struct.pack('<I', len(encoded)))
    writer.write(encoded)

def write_vec3(writer, values):
    writer.write(struct.pack('<fff', *values))

def main(input_path, output_path):
    map_data = {
        'models': [],
        'placements': []
    }

    with open(input_path, 'r') as f:
        lines = [line.strip() for line in f if line.strip() and not line.startswith('#')]

        section = None
        for line in lines:
            if line == '[Models]':
                section = 'models'
                continue
            elif line == '[Placement]':
                section = 'placements'
                continue

            if section == 'models':
                parts = line.split()
                idx = int(parts[0])
                fname = parts[1]
                map_data['models'].append({'index': idx, 'filename': fname})

            elif section == 'placements':
                parts = list(map(float, line.split()))
                mi = int(parts[0])
                pos = parts[1:4]
                rot = parts[4:7]
                scale = parts[7:10]
                map_data['placements'].append({
                    'model_index': mi,
                    'position': pos,
                    'rotation': rot,
                    'scale': scale
                })

    # Записываем в бинарный формат
    with open(output_path, 'wb') as f:
        # Заголовок
        f.write(MAGIC)
        f.write(struct.pack('<III', VERSION, len(map_data['models']), len(map_data['placements'])))
        
        # Пишем модели
        for model in map_data['models']:
            f.write(struct.pack('<I', model['index']))
            write_string(f, model['filename'])

        # Пишем размещения
        for placement in map_data['placements']:
            f.write(struct.pack('<I', placement['model_index']))
            write_vec3(f, placement['position'])
            write_vec3(f, placement['rotation'])
            write_vec3(f, placement['scale'])

    print(f"Wrote {len(map_data['models'])} models and {len(map_data['placements'])} placements to {output_path}")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python map_to_bmap.py input.map output.bmap")
    else:
        main(sys.argv[1], sys.argv[2])