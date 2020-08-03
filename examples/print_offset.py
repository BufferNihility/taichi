import taichi as ti

ti.init(arch=ti.cpu, print_ir=True)

n = 4
m = 8

# a = ti.field(dtype=ti.u8, shape=N)
a = ti.field(dtype=ti.i32)
# ti.root.dense(ti.ij, (4, 8)).place(a)
# ti.root.dense(ti.ij, (2, 4)).dense(ti.ij, 2).place(a)
# ti.root.dense(ti.ij, (1, 2)).dense(ti.ij, 2).dense(ti.ij, 2).place(a)
# ti.root.dense(ti.i, 4).dense(ti.j, 8).place(a)
ti.root.dense(ti.i, 1).dense(ti.j, 8).dense(ti.i, 4).place(a)

@ti.kernel
def fill():
    for i, j in a:
        base = ti.get_addr(a.snode(), [0, 0])
        a[i, j] = int(ti.get_addr(a.snode(), [i, j]) - base) // 4

fill()
print(a.to_numpy())

gui = ti.GUI('layout', res=(256, 512), background_color=0xFFFFFF)

while True:
    for i in range(1, m):
        gui.line(begin=(0, i / m), end=(1, i / m), radius=2, color=0x000000)
    for i in range(1, n):
        gui.line(begin=(i / n, 0), end=(i / n, 1), radius=2, color=0x000000)
    for i in range(n):
        for j in range(m):
            gui.text(f'{a[i, j]}', (i / n, j / m), color=0x0)
    gui.show()
