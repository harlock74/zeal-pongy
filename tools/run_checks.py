#!/usr/bin/env python3
from pathlib import Path
import subprocess,tempfile,sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from gen_maps import generate
from art import read

def run():
 subprocess.run([sys.executable,str(ROOT/'tests/authoring_checks.py')],check=True)
 with tempfile.TemporaryDirectory(prefix='pong-checks-') as tmp:
  path=Path(tmp);generate(ROOT/'assets',path/'maps.generated.h')
  cases={'logic_checks':['src/pong.c'],'native_checks':['src/pong.c'],'simulation_checks':['src/pong.c'],'input_checks':['src/input.c'],'render_checks':['src/pong.c','src/render.c'],'audio_checks':['src/audio.c'],'flow_checks':['src/pong.c','src/input.c']}
  for name,sources in cases.items():
   exe=path/name
   subprocess.run(['cc']+(['-DPONG_FLOW_TEST'] if name=='flow_checks' else [])+['-std=c99','-Wall','-Wextra','-Werror','-Wno-unused-function','-fsanitize=address,undefined','-g','-I'+str(ROOT/'tests/host/include'),'-I'+str(ROOT/'src'),'-I'+str(path),str(ROOT/'tests'/f'{name}.c')]+[str(ROOT/s) for s in sources]+['-o',str(exe)],check=True)
   args=[str(exe)]
   if name=='render_checks':
    capture=ROOT/'build/preview/splash-capture.bin';capture.parent.mkdir(parents=True,exist_ok=True);args+=[str(capture)]
   subprocess.run(args,check=True)
 # Preserve original artwork except new actors (12-14) and restyled digits (20-29).
 w,h,p,pal=read(ROOT/'assets/pong.aseprite');ow,oh,old,op=read(ROOT/'docs/before-audit/assets/pong.aseprite')
 assert w==ow==256 and h==64 and pal==op
 for n in list(range(12))+list(range(15,20))+list(range(30,32)):
  for y in range(16):
   start=(n//16*16+y)*256+n%16*16;assert p[start:start+16]==old[start:start+16]
 print('Non-digit original artwork and palette preserved; shared font in single-frame Aseprite: PASS')
if __name__=='__main__':run()
