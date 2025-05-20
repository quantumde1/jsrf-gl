import struct
import sys
import os

MAGIC = 0x4C444D56  # 'VMDL'

def write_string(f, s, max_len):
    """Записывает строку с нулём в конце"""
    b = s.encode('utf-8')[:max_len]
    f.write(b)
    f.write(b'\x00' * (max_len - len(b)))

def load_mtl(mtl_path):
    materials = []
    if not os.path.exists(mtl_path):
        print(f"MTL файл не найден: {mtl_path}")
        return materials

    current_mat = None
    with open(mtl_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            if line.startswith('newmtl '):
                if current_mat is not None:
                    materials.append(current_mat)
                mat_name = line.split(' ', 1)[1]
                current_mat = {
                    'name': mat_name,
                    'map_kd': '',
                    'Ka': [0.0, 0.0, 0.0],
                    'Kd': [0.0, 0.0, 0.0],
                    'Ks': [0.0, 0.0, 0.0],
                    'Ns': 0.0
                }

            elif line.startswith('Ka ') and current_mat:
                parts = line.split()[1:]
                if len(parts) >= 3:
                    try:
                        current_mat['Ka'] = list(map(float, parts[:3]))
                    except:
                        pass

            elif line.startswith('Kd ') and current_mat:
                parts = line.split()[1:]
                if len(parts) >= 3:
                    try:
                        current_mat['Kd'] = list(map(float, parts[:3]))
                    except:
                        pass

            elif line.startswith('Ks ') and current_mat:
                parts = line.split()[1:]
                if len(parts) >= 3:
                    try:
                        current_mat['Ks'] = list(map(float, parts[:3]))
                    except:
                        pass

            elif line.startswith('Ns ') and current_mat:
                parts = line.split()[1:]
                if len(parts) >= 1:
                    try:
                        current_mat['Ns'] = float(parts[0])
                    except:
                        pass

            elif line.startswith('map_Kd ') and current_mat:
                parts = line.split(' ', 1)
                if len(parts) >= 2:
                    current_mat['map_kd'] = parts[1].strip()

        if current_mat:
            materials.append(current_mat)

    return materials

def convert_obj_to_mdl(obj_path, mtl_path, output_path):
    if not os.path.exists(obj_path):
        print(f"OBJ файл не найден: {obj_path}")
        return

    print(f"Конвертирую: {obj_path} -> {output_path}")

    vertices = []
    tex_coords = []
    faces = []
    face_materials = []

    # Загрузка материалов
    mtl_data = load_mtl(mtl_path)
    current_material = 0  # Если нет usemtl — используется первый материал

    # Парсинг OBJ
    with open(obj_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            if line.startswith('v '):
                parts = line.split()
                if len(parts) >= 4:
                    try:
                        x, y, z = map(float, parts[1:4])
                        vertices.append((x, y, z))
                    except:
                        pass

            elif line.startswith('vt '):
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        s, t = map(float, parts[1:3])
                        tex_coords.append((s, 1.0 - t))  # Flip Y
                    except:
                        pass

            elif line.startswith('usemtl '):
                mat_name = line.split(' ', 1)[1]
                for i, mat in enumerate(mtl_data):
                    if mat['name'] == mat_name:
                        current_material = i
                        break

            elif line.startswith('f '):
                parts = line.split()[1:]
                if len(parts) < 3:
                    continue

                indices = []
                for p in parts:
                    v_vt = p.split('/')
                    if len(v_vt) >= 2:
                        try:
                            v_idx = int(v_vt[0]) - 1
                            vt_idx = int(v_vt[1]) - 1
                            indices.append(v_idx)
                        except:
                            pass
                if len(indices) >= 3:
                    faces.extend(indices)
                    face_materials.append(current_material)

    print("Сохраняю в бинарный файл...")

    with open(output_path, 'wb') as f:
        # Заголовок
        header = (
            MAGIC,
            len(vertices),
            len(tex_coords),
            len(faces),
            len(mtl_data)
        )
        f.write(struct.pack('<IIIII', *header))

        # Вершины
        for v in vertices:
            f.write(struct.pack('<fff', *v))

        # Текстурные координаты
        for t in tex_coords:
            f.write(struct.pack('<ff', *t))

        # Индексы
        for idx in faces:
            f.write(struct.pack('<I', idx))

        # Материалы
        for mat in mtl_data:
            write_string(f, mat['name'], 64)
            write_string(f, mat.get('map_kd', ''), 128)
            f.write(struct.pack('<fff', *mat.get('Ka', [0.0, 0.0, 0.0])))
            f.write(struct.pack('<fff', *mat.get('Kd', [0.0, 0.0, 0.0])))
            f.write(struct.pack('<fff', *mat.get('Ks', [0.0, 0.0, 0.0])))
            f.write(struct.pack('<f', mat.get('Ns', 0.0)))

        # Лицевые материалы
        for fm in face_materials:
            f.write(struct.pack('<I', fm))

    print(f"Готово! Файл сохранён как: {output_path}")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Использование: python convert_obj_to_mdl.py model.obj model.mtl output.mdl")
    else:
        obj_file = sys.argv[1]
        mtl_file = sys.argv[2]
        mdl_file = sys.argv[3]
        convert_obj_to_mdl(obj_file, mtl_file, mdl_file)