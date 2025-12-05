print('x y z w u')
for x in 0, 1:
    for y in 0, 1:
        for z in 0, 1:
            for w in 0, 1:
                for u in 0, 1:
                    f = ((x<=y)and(z==(not w))) <= (u == (x or z))
                    if f == 0:
                        print(x, y, z, w, u)
