# home screen: Father (a priest) standing beside his cannon pointing straight up
import math, random
from PIL import Image, ImageDraw
N=32
random.seed(11)
STARS=[(random.randrange(0,128),random.randrange(0,40),random.randrange(0,N)) for _ in range(40)]
def mirror(left):
    rows=[r for r in left.strip('\n').split('\n')]
    return [list(r+r[::-1]) for r in rows]
PRIEST=mirror("""
.........XXXX
.......XXXXXX
......XXXXXXX
.....XXXXXXXX
.....XXoooooo
.....XoXXXXXX
....XXoXXXXXX
...XXoooooooo
...XXoXXXXXoX
...XXoXXoXXoX
...XXoXXXXXoX
...XXoooooooX
.....XXXXXXXX
.....XXXoXXXX
......XXXoooo
.......XXXXXX
........ooXXX
......XXXoXXX
....XXXooooXX
...XXooooooXo
..XXoooooooXX
..XooooooooXX
.XXoooooooXXX
.XoXooooooooX
.XoXooooooooX
.XoXooooooooo
XooXooooooooo
XooXooooooooo
XooXooooooooo
XXXXooooooooo
XXX.Xoooooooo
....Xoooooooo
....XoooooooX
....XXXXXXXXX
....Xoooooooo
...Xooooooooo
...Xooooooooo
...Xooooooooo
...Xooooooooo
..Xoooooooooo
..Xoooooooooo
..Xoooooooooo
..Xoooooooooo
.Xooooooooooo
.Xooooooooooo
.XXXXXXXXXXXX
...XXXXX.....
..XXXXXX.....""")
W=max(len(r) for r in PRIEST)
PRIEST=[r+['.']*(W-len(r)) for r in PRIEST]
# chest: plain black cassock, then a t-shaped cross lower down
for y in range(17,29):
    for x in range(8,W-8):
        if PRIEST[y][x]!='.': PRIEST[y][x]='o'
for y in range(20,28):
    for x in (12,13): PRIEST[y][x]='X'
for x in range(10,16): PRIEST[22][x]='X'
# cannon pointing straight up
PX,PY=110,46; L=30; ANG=90
A_=math.radians(ANG); C_,S_=math.cos(A_),math.sin(A_)
def barrel():
    I={};O={};SS=4
    for y in range(64):
        for x in range(128):
            w=o=0
            for sy in range(SS):
                for sx in range(SS):
                    X=x+(sx+.5)/SS-PX; Y=-(y+(sy+.5)/SS-PY)
                    t=X*C_+Y*S_; k=-X*S_+Y*C_
                    r=13-5*(t/L)
                    if t>L-5: r+=1.8
                    v=0;op=0
                    if -4<=t<=L and abs(k)<=r:
                        op=1
                        edge=abs(k)>r-1.3 or t>L-1.3
                        band=(t%10)<4 and 0<t<L-7
                        ring=t>L-6
                        hl=r-3.6<k<r-2.4 and not band and t<L-6
                        v=1 if (edge or band or ring or hl) else 0
                        if ring and L-4.6<t<L-3.4: v=0
                        if ring and t>L-1.5 and abs(k)<r-2.5: v=0
                    dd=math.hypot(k,t+4)
                    if dd<=12.5 and t<-3: op=1; v=1 if (dd>11.2 or dd<2.6) else 0
                    w+=v;o+=op
            if o*2>=SS*SS: O[(x,y)]=1; I[(x,y)]=1 if w*2>=SS*SS else 0
    return I,O
BI,BO=barrel()
# smaller title text, baked in (5x7 font drawn 1.5x)
import re
from font5x7 import FONT
def big_text(d,s,x0,y0):
    # each glyph pixel -> 1.5x using an 8x11 cell
    xm=[0,1,3,4,6]; xw=[1,2,1,2,1]
    ym=[0,1,3,4,6,7,9]; yh=[1,2,1,2,1,2,1]
    for ch in s:
        if ch=='+':      # a cross standing in for the T
            d.rectangle((x0+3,y0-3,x0+4,y0+10),fill=1)
            d.rectangle((x0,y0,x0+7,y0+1),fill=1)
            x0+=9; continue
        g=FONT[ord(ch)*5:ord(ch)*5+5]
        for cx in range(5):
            for cy in range(7):
                if g[cx]>>cy&1:
                    d.rectangle((x0+xm[cx],y0+ym[cy],x0+xm[cx]+xw[cx],y0+ym[cy]+yh[cy]-1),fill=1)
        x0+=9
def frame(f):
    im=Image.new('1',(128,64),0); d=ImageDraw.Draw(im)
    for (x,y,ph) in STARS:
        if x<62 and y<46: continue
        tw=(f+ph)%N
        if tw<2: d.line((x-1,y,x+1,y),1); d.line((x,y-1,x,y+1),1)
        elif tw!=5: d.point((x,y),1)
    big_text(d,'CANNON',2,3); big_text(d,'FA+HER',2,17)
    # soft smoke puffs drifting up from the muzzle
    for i in range(3):
        ph=((f+i*11)%N)/N
        if ph<0.06: continue
        cx=PX+ph*5+math.sin((f+i*9)/6)*1.5; cy=PY-L-5-ph*22
        R=1.6+ph*3.2
        blobs=[(cx,cy,R),(cx-R*0.9,cy+R*0.4,R*0.75),(cx+R*0.9,cy+R*0.3,R*0.7)]
        pts=set()
        for (bx,by,br) in blobs:
            for yy in range(int(by-br)-1,int(by+br)+2):
                for xx in range(int(bx-br)-1,int(bx+br)+2):
                    if (xx-bx)**2+(yy-by)**2<=br*br: pts.add((xx,yy))
        for (xx,yy) in pts:
            if 0<=xx<128 and 0<=yy<64:
                edge=any((xx+dx,yy+dy) not in pts for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)))
                im.putpixel((xx,yy),1 if edge else 0)
    # carriage with two wheels behind the barrel
    d.polygon([(PX-16,60),(PX+16,60),(PX+11,48),(PX-11,48)],fill=0,outline=1)
    for cx in (PX-12,PX+12):
        cy,r=PY+1,7
        d.ellipse((cx-r-1,cy-r-1,cx+r+1,cy+r+1),fill=0)
        d.ellipse((cx-r,cy-r,cx+r,cy+r),outline=1); d.ellipse((cx-r+1,cy-r+1,cx+r-1,cy+r-1),outline=1)
        for kk in range(6):
            an=kk*math.pi/3+0.3
            d.line((cx,cy,cx+math.cos(an)*(r-1),cy+math.sin(an)*(r-1)),1)
        d.ellipse((cx-2,cy-2,cx+2,cy+2),fill=1)
    for (x,y) in BO: im.putpixel((x,y),BI[(x,y)])
    d.line((0,61,127,61),1)
    for x in range(0,128,3): d.point((x+(x//3)%2,63),1)
    # Father
    g=[r[:] for r in PRIEST]
    MX,MY=68,61-len(g)+1
    waving = f<16
    if waving:   # his right hand (screen left) leaves the robe to wave
        for y in range(23,30):
            for x in range(0,3): g[y][x]='.'
            if g[y][3]=='.': g[y][3]='X'
    blink = f in (24,25)
    pts=[(x,y) for y,r in enumerate(g) for x,c in enumerate(r) if c!='.']
    for (x,y) in pts:
        for dx in (-1,0,1):
            for dy in (-1,0,1):
                X,Y=MX+x+dx,MY+y+dy
                if 0<=X<128 and 0<=Y<64: im.putpixel((X,Y),0)
    for (x,y) in pts:
        if 0<=MX+x<128 and 0<=MY+y<64: im.putpixel((MX+x,MY+y),1 if g[y][x]=='X' else 0)
    if blink:
        for ex in (MX+8,MX+16): d.line((ex,MY+9,ex+1,MY+9),1)
    if waving:
        wave=[0.5,0.15,-0.2,0.15][(f//4)%4]
        sx_,sy_=MX+3,MY+19
        ex,ey=sx_-6,sy_-2
        gx,gy=ex-math.sin(wave)*8,ey-math.cos(wave)*8
        for wdt,col in ((6,0),(4,1)):
            d.line((sx_,sy_,ex,ey),col,width=wdt); d.line((ex,ey,gx,gy),col,width=wdt)
        d.line((sx_,sy_,ex,ey),0); d.line((ex,ey,gx,gy),0)     # black sleeve inside
        d.ellipse((gx-3,gy-3,gx+3,gy+3),fill=0); d.ellipse((gx-2,gy-2,gx+2,gy+2),fill=1)
    return im
frames=[frame(f) for f in range(N)]
if __name__=='__main__':
    o=Image.new('L',(260,64)); o.paste(frames[0].convert('L'),(0,0)); o.paste(frames[24].convert('L'),(132,0))
    o.resize((1040,256),Image.NEAREST).save('/tmp/t4.png')
