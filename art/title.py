from PIL import Image, ImageDraw
import math, random
W,H=128,64
im=Image.new('1',(W,H),0); d=ImageDraw.Draw(im)
random.seed(5)
for i in range(30):
    x,y=random.randrange(0,128),random.randrange(0,44)
    if not (60<x<128 and y>8): d.point((x,y),1)
# big top tent silhouette, left-bottom (behind)
# ground
d.line((0,59,127,59),1)
for x in range(0,128,3): d.point((x,61+(x//3)%2),1)
px,py=80,49; a=math.radians(30); dx,dy=math.cos(a),-math.sin(a); nx,ny=-dy,dx
L=34
def P(t,s): return (round(px+dx*t+nx*s), round(py+dy*t+ny*s))
def quad(t0,t1,s0,s1,f): d.polygon([P(t0,-s0),P(t1,-s1),P(t1,s1),P(t0,s0)],fill=f)
quad(-6,L,9,7.5,1)          # outline body
quad(-5,L-1,8,6.5,0)        # hollow
for t in range(-3,L-3,6):   # alternating white bands
    quad(t,t+3,8,7,1)
quad(L-2,L+3,9,9,1)         # muzzle ring
quad(L-1,L+2,8,8,0); quad(L-1,L+2,7,7,1)
# back of cannon round cap
d.pieslice((px-8-7,py-1-7,px-8+7,py-1+7),0,360,fill=1)
d.ellipse((px-8-5,py-1-5,px-8+5,py-1+5),outline=0)
# wheel
cx,cy,r=78,53,8
d.ellipse((cx-r-1,cy-r-1,cx+r+1,cy+r+1),fill=0)
d.ellipse((cx-r,cy-r,cx+r,cy+r),outline=1)
d.ellipse((cx-r+1,cy-r+1,cx+r-1,cy+r-1),outline=1)
for k in range(8):
    an=k*math.pi/4+0.2; d.line((cx,cy,round(cx+math.cos(an)*(r-1)),round(cy+math.sin(an)*(r-1))),1)
d.ellipse((cx-2,cy-2,cx+2,cy+2),fill=1)
im.save('title_raw.png'); im.convert('L').resize((W*5,H*5),Image.NEAREST).save('/tmp/title.png')
# man popping out of muzzle
hx,hy=P(L+8,-1)
d.ellipse((hx-6,hy-6,hx+6,hy+6),fill=0)
d.ellipse((hx-5,hy-5,hx+5,hy+5),fill=1)
d.chord((hx-5,hy-5,hx+5,hy+5),200,340,fill=1)
d.line((hx-5,hy-1,hx+5,hy-1),fill=0)           # helmet band
d.point((hx,hy+2),0); d.point((hx+3,hy+2),0); d.line((hx+1,hy+4,hx+2,hy+4),0)   # eyes
d.line((hx,hy-5,hx,hy-8),fill=1); d.point((hx+1,hy-8),1)
# waving arm
ax,ay=P(L+5,-6)
d.line((ax,ay,ax-3,ay-7),fill=1,width=2); d.ellipse((ax-6,ay-11,ax-2,ay-7),fill=1)
im.save('title_raw.png'); im.convert('L').resize((W*5,H*5),Image.NEAREST).save('/tmp/title.png')
