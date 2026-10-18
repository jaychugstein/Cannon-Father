# controls screen: pixel Arduboy with the buttons labelled (2 frames: outer button blinks)
from PIL import Image, ImageDraw
import re
from font5x7 import FONT
def text(im,s,x,y):
    for ch in s:
        g=FONT[ord(ch)*5:ord(ch)*5+5]
        for cx in range(5):
            for cy in range(7):
                if g[cx]>>cy&1: im.putpixel((x+cx,y+cy),1)
        x+=6
def frame(blink):
    im=Image.new('1',(128,64),0); d=ImageDraw.Draw(im)
    # the Arduboy, landscape credit-card shape
    X0,Y0,X1,Y1=36,6,92,58
    d.rounded_rectangle((X0,Y0,X1,Y1),radius=5,outline=1,width=1)
    d.rounded_rectangle((X0+2,Y0+2,X1-2,Y1-2),radius=4,outline=1,width=1)
    # screen with a tiny cannon on it
    sx0,sy0,sx1,sy1=X0+8,Y0+5,X1-8,Y0+27
    d.rectangle((sx0,sy0,sx1,sy1),outline=1,fill=0)
    d.line((sx0+2,sy1-3,sx1-2,sy1-3),1)
    d.ellipse((sx0+7,sy1-10,sx0+13,sy1-4),outline=1)
    d.polygon([(sx0+9,sy1-9),(sx0+12,sy1-12),(sx0+20,sy1-19),(sx0+23,sy1-16),(sx0+15,sy1-8)],fill=1)
    for (px,py) in [(sx0+28,sy1-15),(sx0+32,sy1-18),(sx0+34,sy1-12)]: d.point((px,py),1)
    # d-pad
    cx,cy=X0+14,Y0+40
    d.rectangle((cx-2,cy-7,cx+2,cy+7),fill=1); d.rectangle((cx-7,cy-2,cx+7,cy+2),fill=1)
    d.point((cx,cy),0)
    for (ax,ay,dx,dy) in [(cx,cy-5,0,-1),(cx,cy+5,0,1)]:
        d.point((ax,ay),0)
    # the two buttons: inner (lower left) and outer (upper right)
    ix,iy=X1-20,Y0+44; ox,oy=X1-11,Y0+38
    d.ellipse((ix-4,iy-4,ix+4,iy+4),outline=1,fill=0)
    d.ellipse((ox-4,oy-4,ox+4,oy+4),outline=1,fill=1 if blink else 0)
    if not blink: d.ellipse((ox-2,oy-2,ox+2,oy+2),outline=1)
    # labels + leader lines
    text(im,'AIM',2,34); d.line((20,37,cx-8,37),1)       # to the d-pad (up/down)
    d.polygon([(10,44),(13,48),(7,48)],fill=1); d.polygon([(10,55),(13,51),(7,51)],fill=1)
    text(im,'FIRE',100,24); d.line((112,32,112,oy),1); d.line((112,oy,ox+5,oy),1)
    text(im,'GLIDE',96,50); d.line((101,49,101,iy),1); d.line((101,iy,ix+5,iy),1)

    # little arrow into the GO text pointing at FIRE meaning
    return im
frames=[frame(0),frame(1)]
if __name__=='__main__':
    o=Image.new('L',(260,64)); o.paste(frames[0].convert('L'),(0,0)); o.paste(frames[1].convert('L'),(132,0))
    o.resize((1040,256),Image.NEAREST).save('/tmp/ctrl.png')
