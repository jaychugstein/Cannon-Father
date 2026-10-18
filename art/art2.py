# Bigger art for the FX version.  X = white, o = black (opaque), . = transparent
def mirror(left):
    """build a symmetric sprite from its left half"""
    rows = [r for r in left.strip('\n').split('\n')]
    return '\n'.join(r + r[::-1] for r in rows)

# ---- the man, 24x24, two flail poses (drawn as left halves, mirrored) ----
MAN_A = mirror("""
.........XXX
........XXXX
.......XXXXX
.......XXooo
.......XoXXX
.......XoooX
.......XoXoo
........XXXX
.........XXo
..........XX
XX........oX
XXX......XXo
.XXXXXXXXooo
...XooooXooo
.....Xoooooo
.....Xoooooo
.....Xoooooo
....Xooooooo
....Xooooooo
...Xoooooooo
...Xoooooooo
...XXXXXXXXX
.....XX.....
....XXX.....
""")
MAN_B = mirror("""
XX.......XXX
XXX.....XXXX
.XX....XXXXX
.XX....XXooo
..XX...XoXXX
..XX...XoooX
...XX..XoXoo
...XX...XXXX
....XX...XXo
.....XX...XX
......XX..oX
.......XXXXo
.......Xoooo
.......Xoooo
......Xooooo
......Xooooo
......Xooooo
.....Xoooooo
.....Xoooooo
....Xooooooo
....Xooooooo
....XXXXXXXX
......XX....
.....XXX....
""")
# legs sticking out of a hay bale (replaces the bale when he lands in it), 26x24
HAYLEGS = """
.........XX...XX..........
.........XX...XX..........
.........XX...XX..........
........XXX...XXX.........
........XX.....XX.........
........XX.....XX.........
.......XXX.....XXX........
.......XX.......XX........
....XXXXXXXXXXXXXXXXXX....
..XXX.X..X.XX.X..X.XXXX...
.XX.X..X..X..X.X..X..XXX..
XX..X.X..X.X..X..X.X..XX..
XXXXXXXXXXXXXXXXXXXXXXXXXX
X.X..X.X.XX.X..X.X.XX.X..X
XX.X..X.X..X.X..X.X..X.X.X
X..X.X..X.X..X.X..X.X..X.X
XXXXXXXXXXXXXXXXXXXXXXXXXX
X.X.X..X.X.XX.X.X..X.X.X.X
XX..X.X..X..X..X.X..X..X.X
XXXXXXXXXXXXXXXXXXXXXXXXXX
"""
# head + waving arm peeking out of the muzzle (16x16)
PEEK = mirror("""
....XXXX
..XXXXXX
.XXXXXXX
.XXooooo
XXoXXXXX
Xooooooo
XoXXXXoX
XoXoXXoX
XooooooX
.XXXXXXX
.XXXXooo
..XXXXXX
...XoooX
""")

# ---- ground things ----
TRAMP = """
.XXXXXXXXXXXXXXXXXXXXXXXXXXXX.
XXooooooooooooooooooooooooooXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
.XoX.X.X.X.X.X.X.X.X.X.X.XoX..
.XoXX.X.X.X.X.X.X.X.X.X.XXoX..
.XXX......................XXX.
.XX........................XX.
XXX........................XXX
"""
SPRING = """
XXXXXXXXXXXXXX
XooooooooooooX
XXXXXXXXXXXXXX
...XXXXXXXX...
..XX......XX..
...XXXXXXXX...
..XX......XX..
...XXXXXXXX...
..XX......XX..
...XXXXXXXX...
..XX......XX..
...XXXXXXXX...
XXXXXXXXXXXXXX
XXoXoXoXoXoXXX
XXXXXXXXXXXXXX
"""
BOMB_0 = """
............X.X...
.............X....
............X.X...
...........X......
..........X.......
.......XXXXX......
.......XoooX......
.....XXXXXXXXX....
...XXXXXXXXXXXXX..
..XXXooXXXXXXXXXX.
..XXoXXXXXXXXXXXX.
.XXXoXXXXXXXXXXXXX
.XXXXXXXXXXXXXXXXX
.XXXXXXXXXXXXXXXXX
.XXXXXXXXXXXXXXXXX
..XXXXXXXXXXXXXXX.
..XXXXXXXXXXXXXXX.
...XXXXXXXXXXXXX..
.....XXXXXXXXX....
"""
BOMB_1 = """
...........X...X..
.............X....
...........X.XX...
...........X......
..........X.......
.......XXXXX......
.......XoooX......
.....XXXXXXXXX....
...XXXXXXXXXXXXX..
..XXXooXXXXXXXXXX.
..XXoXXXXXXXXXXXX.
.XXXoXXXXXXXXXXXXX
.XXXXXXXXXXXXXXXXX
.XXXXXXXXXXXXXXXXX
.XXXXXXXXXXXXXXXXX
..XXXXXXXXXXXXXXX.
..XXXXXXXXXXXXXXX.
...XXXXXXXXXXXXX..
.....XXXXXXXXX....
"""
NET = """
XX..........................XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX.X..X..X..X..X..X..X..X..XXX
XX..X..X..X..X..X..X..X..X.XXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX.X..X..X..X..X..X..X..X..XXX
XX..X..X..X..X..X..X..X..X.XXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX.X..X..X..X..X..X..X..X..XXX
XX..X..X..X..X..X..X..X..X.XXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX..........................XX
XX..........................XX
XX..........................XX
XX..........................XX
XXX........................XXX
"""
HAY = """
....XXXXXXXXXXXXXXXXXX....
..XXX.X..X.XX.X..X.XXXX...
.XX.X..X..X..X.X..X..XXX..
XX..X.X..X.X..X..X.X..XX..
XXXXXXXXXXXXXXXXXXXXXXXXXX
X.X..X.X.XX.X..X.X.XX.X..X
XX.X..X.X..X.X..X.X..X.X.X
X..X.X..X.X..X.X..X.X..X.X
XXXXXXXXXXXXXXXXXXXXXXXXXX
X.X.X..X.X.XX.X.X..X.X.X.X
XX..X.X..X..X..X.X..X..X.X
XXXXXXXXXXXXXXXXXXXXXXXXXX
"""
CACTUS = """
.......XX.......
......XXXX......
......XoXX......
......XXoX......
.X....XoXX......
XXX...XXoX......
XoX...XoXX...X..
XXX...XXoX..XXX.
XoX...XoXX..XoX.
XXXX..XXoX..XXX.
.XoXXXXoXX..XoX.
.XXXXXXXoX.XXXX.
...XXXXoXXXXoX..
......XXoXXXXX..
......XoXXXX....
......XXoX......
......XoXX......
......XXoX......
......XoXX......
.....XXXXXX.....
"""
# ---- air things ----
BALLOON = """
...XXXXX...
..XXXXXXX..
.XXoXXXXXX.
.XoXXXXXXX.
XXoXXXXXXXX
XXXXXXXXXXX
XXXXXXXXXXX
.XXXXXXXXX.
.XXXXXXXXX.
..XXXXXXX..
...XXXXX...
....XXX....
.....X.....
.....X.....
....X......
....X......
.....X.....
......X....
"""
BIRD_0 = """
X...............X
XX.............XX
.XX...........XX.
..XXX.......XXX..
...XXXX.X.XXXX...
.....XXXXXXX.....
......XXXXX......
........X........
"""
BIRD_1 = """
.................
.................
.......X.X.......
......XXXXX......
..XXXXXXXXXXXXX..
XXX...XXXXX...XXX
.......XXX.......
........X........
"""
BIRD_2 = """
.................
.................
.......X.X.......
......XXXXX......
.....XXXXXXX.....
....XXX.X.XXX....
...XX.......XX...
..X...........X..
"""
STAR_0 = """
......X......
......X......
.....XXX.....
.....XXX.....
XXXXXXXXXXXXX
.XXXXXoXXXXX.
...XXXXXXX...
....XXXXX....
...XXXXXXX...
..XXX...XXX..
..XX.....XX..
.X.........X.
"""
STAR_1 = """
.............
......X......
......X......
.....XXX.....
..XXXXXXXXX..
...XXXoXXX...
....XXXXX....
....XXXXX....
...XXX.XXX...
...XX...XX...
..X.......X..
.............
"""
# ---- scenery ----
CLOUD = """
..........XXXXX.................
........XXoooooXX.....XXXX......
.......XooooooooXX..XXooooXX....
....XXXXoooooooooXXXooooooooX...
...XooooooooooooooooooooooooXX..
..XoooooooooooooooooooooooooooX.
.XooooooooooooooooooooooooooooX.
XoooooooooooooooooooooooooooooX.
XoooooooooooooooooooooooooooooX.
.XXXXXXXXXXXXXXXXXXXXXXXXXXXXX..
"""
SIGN = """
XXXXXXXXXXXXXXXXXX
XooooooooooooooooX
XooooooooooooooooX
XooooooooooooooooX
XooooooooooooooooX
XooooooooooooooooX
XXXXXXXXXXXXXXXXXX
.......XXXX.......
.......XooX.......
.......XooX.......
.......XooX.......
.......XooX.......
.......XooX.......
"""
TENT = mirror("""
.......................X
.......................X
.......................X
.......................X
.....................XXX
...................XXooX
.................XXoooXX
...............XXXooooXX
.............XXoXooooXXX
...........XXooXooooXXXX
.........XXoooXooooXXXXX
.......XXooooXooooXXXXXX
.....XXoooooXooooXXXXXXX
...XXooooooXooooXXXXXXXX
.XXXXXXXXXXXXXXXXXXXXXXX
XXoXXoXXoXXoXXoXXoXXoXXo
XooXooXooXooXooXooXooXoo
.XoooooooXXXXXXXXooooooo
.XoooooooXooooooXooooooo
.XoooooooXooooooXXXXXXXo
.XoooooooXooooooXoooooXo
.XoooooooXooooooXoooooXo
.XoooooooXooooooXoooooXo
.XoooooooXooooooXoooooXo
.XoooooooXooooooXoooooXo
.XoooooooXooooooXoooooXo
.XXXXXXXXXXXXXXXXXXXXXXX
""")

# ---- new things ----
DOVE_0 = """
..........XXX.........
.........XXXX.........
........XXXXX.........
.......XXXXXX.........
..XX..XXXXXX..........
.XXXXXXooooX..........
XXoXXXXXXXXXXXXX......
.XXXXXXXXXXXXXXXXXX...
...XXXXXXXXXXXXXXXXXX.
.....XXXXXXXXXX..XXXXX
.......XXXXXX......XXX
"""
DOVE_1 = """
......................
......................
......................
.......XXXXXXXXX......
..XX....XXXXXXXXXX....
.XXXXXXooooooooooXXX..
XXoXXXXXXXXXXXXXX.....
.XXXXXXXXXXXXXXXXXX...
...XXXXXXXXXXXXXXXXXX.
.....XXXXXXXXXX..XXXXX
.......XXXXXX......XXX
"""
DOVE_2 = """
......................
......................
......................
......................
..XX..................
.XXXXXXXX.............
XXoXXXXXXXXXXXXX......
.XXXXXXXXXXXXXXXXXX...
...XXXooooooXXXXXXXXX.
.....XXXXXXXXXXXXXXXXX
.......XXXXXXX.....XXX
........XXXXXX........
.........XXXX.........
..........XX..........
"""
MUD = """
......XXXX..........XXX.....
..XXXX....XXXXXXXXXX...XXX..
.XoXooXoooooXoooXoooXooooXX.
XooooXoooXooooooooXoooXooooX
XXXXXXXXXXXXXXXXXXXXXXXXXXXX
"""
FAN_0 = """
XXXXXXXXXXXXXXXXXXXX
XoXooXooXooXooXooXoX
XoXooXooXooXooXooXoX
XXXXXXXXXXXXXXXXXXXX
XooooooooooooooooooX
XoooXXXXooXXXXoooooX
XooooooooooooooooooX
XXXXXXXXXXXXXXXXXXXX
.XX..............XX.
"""
FAN_1 = """
XXXXXXXXXXXXXXXXXXXX
XoXooXooXooXooXooXoX
XoXooXooXooXooXooXoX
XXXXXXXXXXXXXXXXXXXX
XooooooooooooooooooX
XooooooXXXXooXXXXooX
XooooooooooooooooooX
XXXXXXXXXXXXXXXXXXXX
.XX..............XX.
"""
BELL_0 = """
......XX......
.....XXXX.....
....XXXXXX....
...XXooXXXX...
...XXoXXXXX...
...XXoXXXXX...
..XXXXXXXXXX..
..XXXXXXXXXX..
.XXXXXXXXXXXX.
XXXXXXXXXXXXXX
XooooooooooooX
.XXXXXXXXXXXX.
......XX......
......XX......
"""
BELL_1 = """
......XX......
.....XXXX.....
....XXXXXX....
...XXooXXXX...
...XXoXXXXX...
...XXoXXXXX...
..XXXXXXXXXX..
..XXXXXXXXXX..
.XXXXXXXXXXXX.
XXXXXXXXXXXXXX
XooooooooooooX
.XXXXXXXXXXXX.
...XX.........
...XX.........
"""
WINGS = """
.....XXXXXXXX.....
....X........X....
.....XXXXXXXX.....
XX..............XX
XXXX..........XXXX
.XXXXX......XXXXX.
.XXXXXX....XXXXXX.
..XXXXX....XXXXX..
...XXXX....XXXX...
....XXX....XXX....
"""

# ---- extra poses for the flying priest ----
MAN_C = """
XX.......XXXXXX.........
XXX.....XXXXXXXX........
.XX....XXXXXXXXXX.......
.XX....XXooooooXX.......
..XX...XoXXXXXXoX.......
..XX...XoooXXoooX.......
...XX..XoXooooXoX.......
...XX...XXXXXXXX........
....XX...XXooXX.........
.....XX...XXXX..........
......XX..oXXo..........
.......XXXXooXXXX.......
.......XooooooooXXX.....
.......XoooooooooXXX....
......XooooooooooX.XXX..
......XooooooooooX..XXX.
......XooooooooooX...XXX
.....XooooooooooooX...XX
.....XooooooooooooX.....
....XooooooooooooooX....
....XooooooooooooooX....
....XXXXXXXXXXXXXXXXXX..
......XX..........XXXX..
.....XXX............XX..
"""
MAN_D = '\n'.join(r[::-1] for r in MAN_C.strip('\n').split('\n'))
MAN_SUPER = mirror("""
........XXXX
.......XX..X
......XX....
......X..XXX
......X.XXXX
......X.XXoo
......X.XoXX
......X.Xooo
......X..XXX
......XX..Xo
.......XXXXX
........Xooo
........Xooo
........Xooo
........Xooo
.......Xoooo
.......Xoooo
.......Xoooo
......Xooooo
......Xooooo
......XXXXXX
........XX..
........XX..
.......XXX..
""")
MAN_PRAY = mirror("""
.........XXX
........XXXX
.......XXXXX
.......XXooo
.......XoXXX
.......XoooX
.......XoXoo
........XXXX
.........XXo
.......XXXXX
......XoooXX
......XooXXX
......XoooXX
......Xooooo
.....Xoooooo
.....Xoooooo
.....Xoooooo
....Xooooooo
....Xooooooo
....XXXXXXXX
......XX....
.....XXX....
""")

# ---- the good word ----
BIBLE_0 = """
XXXXXXX
XoooooX
XooXooX
XoXXXoX
XooXooX
XooXooX
XoooooX
XXXXXXX
"""
_b=[r for r in BIBLE_0.strip('\n').split('\n')]
BIBLE_1 = '\n'.join(''.join(_b[len(_b)-1-y][x] for y in range(len(_b))) for x in range(len(_b[0])))
def _crowd(cheer):
    P_IDLE = [".XXX.", ".XXX.", "..X..", "XXXXX", "X.X.X", "..X..", ".X.X.", ".X.X."]
    P_KID  = [".XX.", ".XX.", "XXXX", ".XX.", ".XX.", "X..X"]
    P_UP   = ["X.XXX.X", "X.XXX.X", ".X.X.X.", "..XXX..", "...X...", "...X...", "..X.X..", "..X.X.."]
    P_KUP  = ["X.XX.X", ".XXXX.", "..XX..", "..XX..", "..XX..", ".X..X."]
    people = [(0,'A'),(7,'K'),(13,'A'),(20,'A')] if True else []
    W,H=28,8; g=[['.']*W for _ in range(H)]
    for (ox,kind) in people:
        if kind=='A': spr = P_UP if cheer else P_IDLE; off=1 if cheer else 0
        else: spr = P_KUP if cheer else P_KID; off=0
        oy=H-len(spr)
        for y,row in enumerate(spr):
            for x,c in enumerate(row):
                X=ox+x-off
                if c=='X' and 0<=X<W: g[oy+y][X]='X'
    return '\n'.join(''.join(r) for r in g)
CROWD_0 = _crowd(False)
CROWD_1 = _crowd(True)
