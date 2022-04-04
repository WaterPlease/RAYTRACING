import numpy as np
from math import *
from scipy.spatial.transform import Rotation as R

class plane:
    def __init__(self,_points,_scale,_rot,_pos):
        self.points = _points
        self.rot = _rot
        self.scale = _scale
        self.pos = _pos

def headGen(N):
    return """CATMULL_ROM # Curve type (BSPLINE or CATMULL_ROM)\n"""+ \
    str(N)+""" # The number of cross sections.\n8 # The number of control points per cross section\n"""

points = """1.0 0
0.707106 0.707106
0 1.0
-0.707106 0.707106
-1.0 0
-0.707106 -0.707106
0 -1.0
0.707106 -0.707106"""

r = 50
def pos(t,a,b):
    global r
    theta = a*t
    psi = b*t
    s = sin(theta)
    sp = sin(psi)
    cp = cos(psi)
    c = cos(theta)
    #return r*np.array([s*cp,s*sp,c])
    return r*np.array([s*cp,c,s*sp])


def ori(t,a,b):
    theta = a*t
    psi = b*t
    s = sin(theta)
    sp = sin(psi)
    cp = cos(psi)
    c = cos(theta)
    #return np.array([a*c*cp-b*s*sp,a*c*sp+b*s*cp,-a*s])
    return np.array([-sp,cp,-s])

def oriTorot(t,L,N): # t: 0~L effT : t=0 => 0, t=L => N*2pi
    r = R.from_rotvec((t*N*2*np.pi/L)*np.array([0,1,0]))
    nrm = np.linalg.norm(r.as_rotvec())
    res = [nrm]
    if(nrm < 1e-6):
        res[0] = 0.0
        res.extend([0,0,0])
    else:
        res.extend(r.as_rotvec()/res[0])
    return np.array(res)

a = 1.0
b = 2.0*10



L = 50 # screw length
N = 7 # num of screw
M = N*5 # num of sample
dt = L/M
ts = [dt*i for i in range(M)]

hL = 10
offset = np.array([0,-(L+hL)/2,0])


planes = []
for t in (ts):
    scale = None
    if(t < 5):
        scale= 0.2*t*0.8
    else:
        scale = 0.8
    planes.append(
        plane(points1,scale,oriTorot(t,L,N),t*np.array([0,1,0])+offset)
    )
planes.append(plane(points,2,[0,0,1,0],L*np.array([0,1,0])+offset))
planes.append(plane(points,2,[0,0,1,0],(L+hL)*np.array([0,1,0])+offset))
planes.append(plane(points,0.0,[0,0,1,0],(L+hL)*np.array([0,1,0])+offset))
planes.append(plane(points,0.0,[0,0,1,0],(L+hL)*np.array([0,1,0])+offset))
cont = headGen(len(planes))
for pl in (planes):
    cont += pl.points+"\n"+"{:.6f}\n{:.6f} {:.6f} {:.6f} {:.6f}\n{:.6f} {:.6f} {:.6f}\n\n".format(pl.scale,
        pl.rot[0],pl.rot[1],pl.rot[2],pl.rot[3],
        pl.pos[0],pl.pos[1],pl.pos[2]
    )
print(cont)