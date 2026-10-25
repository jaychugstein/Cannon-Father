# builds game/fxdata/fxdata.bin + game/fxdata.h (Arduboy FX data)
import math, os
from PIL import Image, ImageDraw
import art2
OUT='../CannonFather'; os.makedirs(OUT+'/fxdata',exist_ok=True)

def grid(s):
    g=[r for r in s.strip('\n').split('\n')]; w=max(len(r) for r in g); return [r.ljust(w,'.') for r in g]
def from_grid(g):   # -> (img, opaque) 2d lists
    return ([[1 if c=='X' else 0 for c in r] for r in g],[[1 if c in 'Xo' else 0 for c in r] for r in g])
def pad(img,op,W,H):  # center into WxH
    h=len(img); w=len(img[0]); ox=(W-w)//2; oy=(H-h)//2
    I=[[0]*W for _ in range(H)]; O=[[0]*W for _ in range(H)]
    for y in range(h):
        for x in range(w): I[y+oy][x+ox]=img[y][x]; O[y+oy][x+ox]=op[y][x]
    return I,O
def padb(img,op,W,H):  # bottom aligned, centered horizontally
    h=len(img); w=len(img[0]); ox=(W-w)//2; oy=H-h
    I=[[0]*W for _ in range(H)]; O=[[0]*W for _ in range(H)]
    for y in range(h):
        for x in range(w): I[y+oy][x+ox]=img[y][x]; O[y+oy][x+ox]=op[y][x]
    return I,O
def dil(O):
    H=len(O); W=len(O[0]); M=[[0]*W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            if O[y][x]:
                for dy in (-1,0,1):
                    for dx in (-1,0,1):
                        if 0<=y+dy<H and 0<=x+dx<W: M[y+dy][x+dx]=1
    return M
def encode(frames,masked=True):
    H=len(frames[0][0]); W=len(frames[0][0][0])
    b=bytearray([W>>8,W&255,H>>8,H&255])
    for I,M in frames:
        for p in range((H+7)//8):
            for x in range(W):
                v=m=0
                for k in range(8):
                    y=p*8+k
                    if y<H:
                        v|=I[y][x]<<k
                        if masked: m|=M[y][x]<<k
                b.append(v&m if masked else v)
                if masked: b.append(m)
    return b,W,H

items=[]   # (name, bytes, W, H, frames)
def add(name,frames,masked=True):
    b,W,H=encode(frames,masked); items.append((name,b,W,H,len(frames)))

def simple(name,*srcs,border=1):
    gs=[from_grid(grid(s)) for s in srcs]
    W=max(len(g[0][0]) for g in gs)+2*border; H=max(len(g[0]) for g in gs)+2*border
    fr=[]
    for I,O in gs:
        I,O=padb(I,O,W,H-border) if False else pad(I,O,W,H)
        fr.append((I,dil(O)))
    add(name,fr)

# --- man: 2 poses x 16 rotations, 32x32 ---
def rot(g,deg,S=32):
    I,O=from_grid(grid(g))
    h=len(I); w=len(I[0])
    big=Image.new('L',(S*4,S*4),0); bo=Image.new('L',(S*4,S*4),0)
    ox=(S-w)//2; oy=(S-h)//2
    for y in range(h):
        for x in range(w):
            if O[y][x]:
                ImageDraw.Draw(bo).rectangle(((x+ox)*4,(y+oy)*4,(x+ox)*4+3,(y+oy)*4+3),fill=255)
            if I[y][x]:
                ImageDraw.Draw(big).rectangle(((x+ox)*4,(y+oy)*4,(x+ox)*4+3,(y+oy)*4+3),fill=255)
    if deg%90==0:
        k=(deg//90)%4
        ri=big.rotate(-deg); ro=bo.rotate(-deg)
    else:
        ri=big.rotate(-deg,Image.BILINEAR); ro=bo.rotate(-deg,Image.BILINEAR)
    ri=ri.resize((S,S),Image.BOX); ro=ro.resize((S,S),Image.BOX)
    Iw=[[1 if ri.getpixel((x,y))>110 else 0 for x in range(S)] for y in range(S)]
    Ow=[[1 if (ro.getpixel((x,y))>110 or Iw[y][x]) else 0 for x in range(S)] for y in range(S)]
    return Iw,dil(Ow)
def ball():
    S=20; rows=[]
    for y in range(S):
        r=''
        for x in range(S):
            dx=x-9.5; dy=y-9.5; d=math.hypot(dx,dy)
            hx=dx+3.5; hy=dy+3.5; hd=math.hypot(hx,hy)
            c='.'
            if d<=9.3: c='X' if d>8.2 else 'o'
            if hd<=4.6: c='X'
            if abs(hy-0.5)<0.6 and abs(hx)<3.6: c='o'          # glasses band
            if (abs(hx+1.6)<0.6 or abs(hx-1.6)<0.6) and abs(hy-0.5)<0.6: c='X'
            if math.hypot(dx-6.5,dy-1)<=2.1: c='X'               # hands
            if math.hypot(dx-2,dy-7)<=2.1: c='X'                 # feet
            r+=c
        rows.append(r)
    return '\n'.join(rows)
POSES=[art2.MAN_A,art2.MAN_B,art2.MAN_C,art2.MAN_D,art2.MAN_SUPER,ball(),art2.MAN_PRAY]
add('MAN',[rot(p,k*22.5) for p in POSES for k in range(16)])

simple('PEEK',art2.PEEK)
simple('TRAMP',art2.TRAMP); simple('SPRING',art2.SPRING); simple('BOMB',art2.BOMB_0,art2.BOMB_1)
simple('HAY',art2.HAY); simple('HAYLEGS',art2.HAYLEGS); simple('CACTUS',art2.CACTUS)
simple('BALLOON',art2.BALLOON); simple('DOVE',art2.DOVE_0,art2.DOVE_1,art2.DOVE_2,art2.DOVE_1)
simple('BIBLE',art2.BIBLE_0,art2.BIBLE_1); simple('CROWD',art2.CROWD_0,art2.CROWD_1)
simple('MUD',art2.MUD); simple('FAN',art2.FAN_0,art2.FAN_1); simple('BELL',art2.BELL_0,art2.BELL_1); simple('WINGS',art2.WINGS)
# 30 degree ramp: 30 wide, slope rises to the right
def ramp():
    W,H=30,18; I=[[0]*W for _ in range(H)]; O=[[0]*W for _ in range(H)]
    for x in range(W):
        top=H-1-int(round(x*math.tan(math.radians(30))))
        for y in range(max(top,0),H):
            O[y][x]=1
            edge = y<=top+1 or y==H-1 or x>=W-2
            strut = (x+ (H-1-y)) % 7 == 0 and y>top+2
            I[y][x]=1 if (edge or strut) else 0
    I2,O2=pad(I,O,W+2,H+2); return [(I2,dil(O2))]
add('RAMP',ramp())
# little ground cannon pointing up-right (relaunch)
def mini():
    S=24; I=[[0]*S for _ in range(S)]; O=[[0]*S for _ in range(S)]
    PX_,PY_=8,17; a=math.radians(45); c,s=math.cos(a),math.sin(a); L_=15
    for y in range(S):
        for x in range(S):
            X=x+.5-PX_; Y=-(y+.5-PY_); t=X*c+Y*s; k=-X*s+Y*c
            r=4.6+(1.2 if t>L_-3 else 0)
            if -4<=t<=L_ and abs(k)<=r:
                O[y][x]=1; I[y][x]=1 if (abs(k)>r-1.1 or t>L_-3 or (t%5)<2 and t>-2) else 0
                if t>L_-1.2 and abs(k)<r-1.6: I[y][x]=0
            d=math.hypot(x+.5-PX_,y+.5-PY_)
            if d<=5.5:
                O[y][x]=1; I[y][x]=1 if (d>4.4 or d<1.6 or abs((x+.5-PX_)-(y+.5-PY_))<0.8 or abs((x+.5-PX_)+(y+.5-PY_))<0.8) else 0
    I2,O2=pad(I,O,S+2,S+2); return [(I2,dil(O2))]
add('MINI',mini())
simple('STAR',art2.STAR_0,art2.STAR_1); simple('CLOUD',art2.CLOUD); simple('SIGN',art2.SIGN)

# --- big top circus tent for the start area, 66x46 ---
def bigtent():
    W,H=66,46; S=4
    im=Image.new('L',(W*S,H*S),0); op=Image.new('L',(W*S,H*S),0)
    di=ImageDraw.Draw(im); do=ImageDraw.Draw(op)
    pk=(33*S,6*S); eaveY=24*S; L_=(2*S,eaveY); R_=(64*S,eaveY)
    # roof (curved sides) as polygon
    roof=[pk]
    for i in range(1,11):
        t=i/10; x=pk[0]+(L_[0]-pk[0])*t; y=pk[1]+(eaveY-pk[1])*(t**0.8); roof.append((x,y))
    roofR=[(2*pk[0]-x,y) for (x,y) in reversed(roof[1:])]
    poly=roof+roofR
    do.polygon(poly,fill=255)
    di.polygon(poly,fill=255)
    # black wedges on the roof
    for k in range(-3,4):
        if k%2==0: continue
        a=(pk[0]+k*9*S,eaveY); b=(pk[0]+(k+1)*9*S,eaveY)
        di.polygon([pk,a,b],fill=0)
    di.line(poly+[poly[0]],fill=255,width=S)
    # scalloped valance
    for i in range(8):
        cx=(4+i*8)*S
        do.pieslice((cx-4*S,eaveY-3*S,cx+4*S,eaveY+5*S),0,180,fill=255)
        di.pieslice((cx-4*S,eaveY-3*S,cx+4*S,eaveY+5*S),0,180,fill=255 if i%2==0 else 0,outline=255,width=S)
    # walls with stripes
    do.rectangle((4*S,eaveY,62*S,(H-1)*S),fill=255)
    for i in range(4,62,4):
        di.rectangle((i*S,eaveY+4*S,(i+2)*S-1,(H-1)*S),fill=255 if (i//4)%2==0 else 0)
    di.rectangle((4*S,eaveY+4*S,62*S,(H-1)*S),outline=255,width=S)
    # entrance with curtains pulled back
    ex0,ex1=24*S,42*S
    di.rectangle((ex0,30*S,ex1,(H-1)*S),fill=0)
    di.pieslice((ex0,26*S,ex1,36*S),180,360,fill=0)
    di.polygon([(ex0,30*S),(ex0+6*S,30*S),(ex0,(H-1)*S)],fill=255)
    di.polygon([(ex1,30*S),(ex1-6*S,30*S),(ex1,(H-1)*S)],fill=255)
    di.arc((ex0,26*S,ex1,36*S),180,360,fill=255,width=S)
    # flag pole + pennant
    di.line((pk[0],pk[1],pk[0],0),fill=255,width=S); do.line((pk[0],pk[1],pk[0],0),fill=255,width=S*2)
    di.polygon([(pk[0]+S,0),(pk[0]+9*S,2*S),(pk[0]+S,4*S)],fill=255); do.polygon([(pk[0]+S,0),(pk[0]+9*S,2*S),(pk[0]+S,4*S)],fill=255)
    im=im.resize((W,H),Image.BOX); op=op.resize((W,H),Image.BOX)
    I=[[1 if im.getpixel((x,y))>110 else 0 for x in range(W)] for y in range(H)]
    O=[[1 if op.getpixel((x,y))>60 or I[y][x] else 0 for x in range(W)] for y in range(H)]
    I2,O2=pad(I,O,W+2,H+2); return [(I2,dil(O2))]
add('BIGTENT',bigtent())

# --- cannon barrel, angles 10..80 step 2, 48x48, pivot at (12,36) ---
CS=48; PX,PY=12,36; L=30; BACK=9
def barrel(angle):
    a=math.radians(angle); c,s=math.cos(a),math.sin(a)
    I=[[0]*CS for _ in range(CS)]; O=[[0]*CS for _ in range(CS)]
    SS=4
    for y in range(CS):
        for x in range(CS):
            white=opq=0
            for sy in range(SS):
                for sx in range(SS):
                    X=x+(sx+.5)/SS-PX; Y=-(y+(sy+.5)/SS-PY)
                    t=X*c+Y*s; k=-X*s+Y*c
                    r=6.5-0.6*(t/L)            # slight taper
                    if t>L-4: r+=1.6            # muzzle ring
                    v=0; o=0
                    if -BACK<=t<=L and abs(k)<=r:
                        o=1
                        edge = abs(k)>r-1.1 or t>L-1.1
                        band = (t%7)<3 and t<L-4 and t>-BACK+3
                        ring = t>L-4
                        v = 1 if (edge or band or ring) else 0
                        if ring and t>L-1.2 and abs(k)<r-2.2: v=0   # dark mouth
                    # round back cap
                    dx=t+BACK; 
                    if dx*dx+k*k<=6.8**2 and t<-BACK+1:
                        o=1; v = 1 if (dx*dx+k*k>5.7**2 or dx*dx+k*k<1.6**2) else 0
                    white+=v; opq+=o
            O[y][x]=1 if opq*2>=SS*SS else 0
            I[y][x]=1 if (white*2>=SS*SS and O[y][x]) else 0
    return I,dil(O)
add('CANNON',[barrel(a) for a in range(10,81)])
# wheel 22x22, 4 spoke positions
def wheel(ph):
    S=22; c=10.5; I=[[0]*S for _ in range(S)]; O=[[0]*S for _ in range(S)]
    for y in range(S):
        for x in range(S):
            dx=x-c; dy=y-c; d=math.hypot(dx,dy)
            if d<=10.4:
                O[y][x]=1
                ang=(math.degrees(math.atan2(dy,dx))+ph*11.25)%45
                if d>8.2 or d<2.6 or (d<8.4 and (ang<6 or ang>39)): I[y][x]=1
                if 1.2<d<2.0: I[y][x]=0
    return I,dil(O)
add('WHEEL',[wheel(p) for p in range(4)])
# carriage base 34x7
BASE="""
.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..
XXoXXXoXXXoXXXoXXXoXXXoXXXoXXXoXX.
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
.XX..........................XX...
.XX..........................XX...
"""
simple('BASE',BASE)
# animated title (32 full-screen frames, no mask)
import title4, ctrl
add('CONTROLS',[([[1 if fr.getpixel((x,y)) else 0 for x in range(128)] for y in range(64)],None) for fr in ctrl.frames],masked=False)
add('TITLEANIM',[([[1 if fr.getpixel((x,y)) else 0 for x in range(128)] for y in range(64)],None) for fr in title4.frames],masked=False)

data=bytearray(); hdr=['#pragma once','#include <ArduboyFX.h>','// generated by art/gen2.py']
for name,b,W,H,n in items:
    hdr.append('constexpr uint24_t FX_%s = 0x%06X; constexpr uint8_t %s_W = %d, %s_H = %d, %s_N = %d;'%(name,len(data),name,W,name,H,name,n))
    data+=b
pages=(len(data)+255)//256; data+=b'\0'*(pages*256-len(data))
hdr.insert(2,'constexpr uint16_t FX_DATA_PAGE = 0x%04X;'%(65536-pages))
open(OUT+'/fxdata/fxdata.bin','wb').write(data); open(OUT+'/fxdata.h','w').write('\n'.join(hdr)+'\n')
print('fx bytes',len(data))
# preview sheet of everything
prev=[]
for name,b,W,H,n in items:
    pass
# small lookup tables for the code
hill=[int(6+math.sin(i*2*math.pi/256*3)*5+math.sin(i*2*math.pi/256)*7) for i in range(256)]
sin90=[round(math.sin(math.radians(d))*256) for d in range(91)]
open(OUT+'/tables.h','w').write('#pragma once\n// generated by art/gen2.py\nconst uint8_t HILL[] PROGMEM = {%s};\nconst uint16_t SIN90[] PROGMEM = {%s};\n'%(','.join(str(v&255) for v in hill),','.join(map(str,sin90))))
