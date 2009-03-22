import Blender
from Blender import Scene, Object, NMesh
from Blender.BGL import *
from Blender.Draw import *
import os

# only write one vertex/vertex
def writevertices(f, mesh):
  global COORD_X, COORD_Y, COORD_Z
  written = {}
  # index of written vertices
  idx = 1
  for face in mesh.faces:
    for v in face.v:
      coords = v.co
      if v.index not in written:
        li = [coords.x, coords.y, coords.z]
        f.write("v " + str(li[COORD_X]) + " " + str(li[COORD_Y]) + " "
          + str(li[COORD_Z]) + "\n")
        written[v.index] = idx
        idx+=1
  return written

# UV coordinates shared if equal
def writeuv(f, mesh):
  written = {}
  idx = 1
  for face in mesh.faces:
    for uv in face.uv:
      if uv not in written:
        f.write("vt " + str(uv[0]) + " " + str(uv[1]) + "\n")
        written[uv] = idx
        idx+=1
  return written

# Normals also shared if equal
def writenorm(f, mesh):
  global COORD_X, COORD_Y, COORD_Z
  written = {}
  idx = 1
  for face in mesh.faces:
    for v in face.v:
      norm = v.no
      if v.index not in written:
        li = [norm.x, norm.y, norm.z]
        f.write("vn " + str(li[COORD_X]) + " " + str(li[COORD_Y]) + " "
          + str(li[COORD_Z]) + "\n")
        written[v.index] = idx
        idx+=1
  return written

# triangulates (possibly quadrilateral) faces too, cos it's easy to do here
def writeface(f, mesh, vtab, uvtab, ntab):
  cmat = ""
  for face in mesh.faces:
    mat = face.image.name
    if mat != cmat:
      f.write("usemtl "+mat+"\n")
      cmat = mat
    nof = len(face.v)
    if nof==3:
      f.write("f ")
      for i in range(3):
        v = face.v[i].index
        f.write(str(vtab[v]) + "/" + str(uvtab[face.uv[i]]) + "/"
          + str(ntab[v]) + " ")
      f.write("\n")
    else:
      f.write("f ")
      for i in [0,1,3]:
        v = face.v[i].index
        f.write(str(vtab[v]) + "/" + str(uvtab[face.uv[i]]) + "/"
          + str(ntab[v]) + " ")
      f.write("\nf ")
      for i in [3,1,2]:
        v = face.v[i].index
        f.write(str(vtab[v]) + "/" + str(uvtab[face.uv[i]]) + "/"
          + str(ntab[v]) + " ")
      f.write("\n")

def findmaterials(mesh):
  mats = {}
  for f in mesh.faces:
    print f.mode
    if (f.mode & 4) != 0: mats[f.image.name] = True
  return mats

# bind "materials" to textures
def writemats(basedir, levdir, mats, name):
  matlist = []
  for m in mats:
    libnamerel = os.path.join(levdir, "mats", name+m+".mtl")
    libname = os.path.join(basedir, libnamerel)
    print "writing material",libname
    f = open(libname, "w")
    f.write("newmtl "+m+"\n")
    f.write("map_Kd "+os.path.join("textures", m)+"\n")
    f.close()
    matlist.append(libnamerel)
  return matlist

def writeobj(basedir, levdir, mesh, name, mtllibs):
  objnamerel = os.path.join(levdir, "objs", name+".obj")
  objname = os.path.join(basedir, objnamerel)
  f = open(objname, "w")
  f.write("# Written by TTD Blender level export script\n")
  for mtl in mtllibs:
    f.write("mtllib " + mtl + "\n")
  vtab = writevertices(f, mesh)
  uvtab = writeuv(f, mesh)
  ntab = writenorm(f, mesh)
  writeface(f, mesh, vtab, uvtab, ntab)
  f.close()
  return objnamerel

def writeactor(file, actorname, o):
  global COORD_X, COORD_Y, COORD_Z
  file.write('actor "' + actorname + '" {\n')
  loc = o.getLocation()
  if actorname=="ship":
    file.write(
"""  model {
    "ship5.obj"
    modify {
      scale <0.15, 0.15, 0.15>
    }
  }
""")
  elif actorname=="thing":
    file.write(
"""  sphere {
    0.2
    transform {\n""")
    file.write("      translate <" + str(loc[COORD_X]) + ", "
                         + str(loc[COORD_Y]) + ", "
                         + str(loc[COORD_Z]) + ">\n")
    file.write("    }\n  }\n")
  
  file.write("}\n\n")

def writemodel(file, modelname, obj):
  global COORD_X, COORD_Y, COORD_Z
  print "model name:",modelname
  print "object matrix:"
  for x in obj.mat:
    print x
  print "scale:",obj.size
  sz = obj.size
  loc = obj.loc
  file.write("model {\n")
  file.write('  "'+modelname+'"\n')
  file.write('  modify {\n')
  file.write('    scale <' + str(sz[COORD_X]) + ", "
                           + str(sz[COORD_Y]) + ", "
                           + str(sz[COORD_Z]) + ">\n")
  file.write('    translate <' + str(loc[COORD_X]) + ", "
                               + str(loc[COORD_Y]) + ", "
                               + str(loc[COORD_Z]) + ">\n")
  file.write('  }\n')
  file.write("}\n\n")

def draw():
  global T_FlipYZ, T_GameDir, T_SkyBox, T_Level
  global EVENT_NOEVENT, EVENT_EXPORT, EVENT_EXIT
  
  glClear(GL_COLOR_BUFFER_BIT)
  glRasterPos2d(8, 135)
  Text("TTD Level Export Script")
  
  glRasterPos2d(8, 120)
  Text("Parameters:")
#  T_NumberOfSides = Number("No. of sides: ", EVENT_NOEVENT, 10, 55, 210, 18,
#              T_NumberOfSides.val, 3, 20, "Number of sides of out polygon");
  T_FlipYZ = Toggle("Exchange YZ", EVENT_FLIPYZ, 10, 95, 210, 18,
                    T_FlipYZ.val, "Output file has Y & Z axes inverted");
  T_GameDir = String("Game dir: ", EVENT_NOEVENT, 10, 75, 210, 18,
                    T_GameDir.val, 1000, "Game dir to export into");
  T_SkyBox = String("Sky box: ", EVENT_NOEVENT, 10, 55, 210, 18,
                    T_SkyBox.val, 1000, "Sky box to use for this level");
  T_Level = String("Level: ", EVENT_NOEVENT, 10, 35, 210, 18,
                   T_Level.val, 1000, "Level directory");
  
  Button("Export", EVENT_EXPORT, 10, 10, 80, 18)
  Button("Exit", EVENT_EXIT, 140, 10, 80, 18)

def event(evt, val):
  if (evt == QKEY and not val):
    Exit()

def bevent(evt):
  global T_FlipYZ, T_GameDir, T_SkyBox, T_Level
  global EVENT_NOEVENT, EVENT_EXPORT, EVENT_EXIT
  global COORD_X, COORD_Y, COORD_Z
  
  if (evt == EVENT_EXIT):
    Exit()
  elif (evt == EVENT_EXPORT):
    if T_FlipYZ.val==1:
      COORD_X=1
      COORD_Y=2
      COORD_Z=0
    else:
      COORD_X=0
      COORD_Y=1
      COORD_Z=2
    writescene(T_GameDir.val, T_Level.val, T_SkyBox.val)
    Blender.Redraw()

def writescene(gamedir, level, skybox):
  scene = Scene.GetCurrent()

  objects = scene.getChildren()

  levdir = os.path.join("levels", level)

  lf = open(os.path.join(gamedir, levdir, level+".lev"), "w")
  lf.write("# TTD level '"+level+"', written by Blender export script\n")

  lf.write("""
  skybox {
    "sky/sky1.png"
    "sky/sky2.png"
    "sky/sky3.png"
    "sky/sky4.png"
    "sky/sky5.png"
    "sky/sky6.png"
  }

  """)

  for o in objects:
    type = o.getType()
    if type=='Mesh':
      name = o.getName()
      print "Found mesh",name
      if name=="Player":
        writeactor(lf, 'ship', o)
      elif name=="Thing":
        writeactor(lf, 'thing', o)
      else:
        mesh = o.getData()
        mats = findmaterials(mesh)
        mtllibs = writemats(gamedir, levdir, mats, name)
        objname = writeobj(gamedir, levdir, mesh, name, mtllibs)
        writemodel(lf, objname, o)

  lf.close()

T_FlipYZ = Create(True)
T_GameDir = Create("/mnt/data/code/ttd")
T_SkyBox = Create("sky/sky?.png")
T_Level = Create("level1")

# default axis ordering
COORD_X = 0
COORD_Y = 1
COORD_Z = 2

EVENT_NOEVENT = 1
EVENT_EXPORT = 2
EVENT_EXIT = 3
EVENT_FLIPYZ = 4

Register(draw, event, bevent)
