import re,sys
from PIL import Image
hdr=open('../CannonFather/fxdata.h').read(); data=open('../CannonFather/fxdata/fxdata.bin','rb').read()
items=re.findall(r'FX_(\w+) = 0x([0-9A-F]+); constexpr uint8_t \w+_W = (\d+), \w+_H = (\d+), \w+_N = (\d+)',hdr)
want=sys.argv[1].split(',') if len(sys.argv)>1 else None
tiles=[]
for name,off,W,H,N in items:
    if want and name not in want: continue
    off=int(off,16);W=int(W);H=int(H);N=int(N); masked=not (name.startswith('TITLE') or name=='CONTROLS'); P=(H+7)//8
    for f in range(N):
        im=Image.new('L',(W,H))
        base=off+4+f*W*P*(2 if masked else 1)
        for p in range(P):
            for x in range(W):
                i=base+(p*W+x)*(2 if masked else 1)
                v=data[i]; m=data[i+1] if masked else 255
                for k in range(8):
                    y=p*8+k
                    if y<H: im.putpixel((x,y),255 if v>>k&1 else (0 if m>>k&1 else 90))
        tiles.append(im)
cols=int(sys.argv[2]) if len(sys.argv)>2 else 12
cw=max(t.width for t in tiles)+2; ch=max(t.height for t in tiles)+2
rows=(len(tiles)+cols-1)//cols
o=Image.new('L',(cw*cols,ch*rows),60)
for i,t in enumerate(tiles): o.paste(t,((i%cols)*cw+1,(i//cols)*ch+1))
s=int(sys.argv[3]) if len(sys.argv)>3 else 3
o.resize((o.width*s,o.height*s),Image.NEAREST).save('/tmp/fx.png')
