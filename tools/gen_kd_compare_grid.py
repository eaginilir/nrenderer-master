# python d:\study\computer_graph\nrenderer-master\tools\gen_kd_compare_grid.py
import os

OUT = r"d:\study\computer_graph\nrenderer-master\resource\kd_compare_grid.scn"

GRID_N = 120     # 网格细分（N*N格子 -> 2*N*N三角形），对比建议 >= 100
SIZE_X = 400.0   # 网格尺寸X
SIZE_Z = 400.0   # 网格尺寸Z
Y0 = -150.0      # 网格所在Y平面
TX, TY, TZ = 0.0, -178.0, 900.0  # 网格Model的Translation，放入盒子内部

def write(s):
    f.write(s + "\n")

with open(OUT, "w", encoding="utf-8") as f:
    write("Begin Material")
    write("")
    write("Material White")
    write("Prop diffuseColor RGB 0.725 0.71 0.68")
    write("")
    write("Material Red")
    write("Prop diffuseColor RGB 0.63 0.065 0.05")
    write("")
    write("Material Green")
    write("Prop diffuseColor RGB 0.14 0.45 0.091")
    write("")
    write("End")
    write("")

    # Cornell Box 墙体
    write("Begin Model")
    write("Model Wall")
    write("Translation 0.0 0.0 1028.0")
    write("Plane LeftWall Red")
    write("N -1.0 0.0 0.0")
    write("P 278.0 278.0 278.0")
    write("U 0 -556.0 0")
    write("V 0 0 -556.0")
    write("")
    write("Plane RightWall Green")
    write("N 1.0 0.0 0.0")
    write("P -278.0 278.0 278")
    write("U 0 -556 0")
    write("V 0 0 -556.0")
    write("")
    write("Plane TopWall White")
    write("N 0.0 -1.0 0.0")
    write("P 278.0 278.0 278")
    write("U -556 0 0")
    write("V 0 0 -556")
    write("")
    write("Plane BottomWall White")
    write("N 0.0 1.0 0.0")
    write("P 278.0 -278.0 278")
    write("U -556 0 0")
    write("V 0 0 -556")
    write("")
    write("Plane BackWall White")
    write("N 0.0 0.0 -1.0")
    write("P 278.0 278.0 278")
    write("U -556 0 0")
    write("V 0 -556 0")
    write("End")
    write("")

    # 高细分网格（大量三角形）
    write("Begin Model")
    write("Model BigGrid")
    write(f"Translation {TX} {TY} {TZ}")
    dx = SIZE_X / GRID_N
    dz = SIZE_Z / GRID_N
    x0 = -SIZE_X / 2.0
    z0 = -SIZE_Z / 2.0
    tri_id = 0
    for i in range(GRID_N):
        for j in range(GRID_N):
            x00 = x0 + i*dx;      z00 = z0 + j*dz
            x10 = x0 + (i+1)*dx;  z10 = z0 + j*dz
            x01 = x0 + i*dx;      z01 = z0 + (j+1)*dz
            x11 = x0 + (i+1)*dx;  z11 = z0 + (j+1)*dz
            tri_id += 1
            write(f"Triangle T{tri_id} White")
            write("N 0 1 0")
            write(f"V1 {x00} {Y0} {z00}")
            write(f"V2 {x10} {Y0} {z10}")
            write(f"V3 {x11} {Y0} {z11}")
            tri_id += 1
            write(f"Triangle T{tri_id} White")
            write("N 0 1 0")
            write(f"V1 {x00} {Y0} {z00}")
            write(f"V2 {x11} {Y0} {z11}")
            write(f"V3 {x01} {Y0} {z01}")
    write("End")
    write("")

    # 面光源
    write("Begin Light")
    write("Area TopLight")
    write("IRV 47.8384 38.5664 31.0808")
    write("P 60 275 1088")
    write("U -120 0 0")
    write("V 0 0 -120")
    write("End")

print(f"Generated {OUT}")