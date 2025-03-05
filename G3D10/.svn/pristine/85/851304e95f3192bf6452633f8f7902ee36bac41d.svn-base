# This script will replace the materials on groups within an OBJ file
# that was exported by 3DS Max. It may work on other OBJ files, but is
# hardcoded for the formatting that 3DS Max produces.
#
# The group targets are of the form "groupname/materialname", which the G3D
# viewer will output to the command line and Visual Studio output pane
# when you click on a mesh.

import re

def readFile(filename):
  f = open(filename, 'r')
  s = f.read()
  f.close()
  return s
  
def writeFile(filename, contents):
  f = open(filename, 'w+')
  f.write(contents)
  f.close()

def process(filename, meshArray, newMaterialName):
  print('Reading ' + filename)
  contents = readFile(filename)

  for mesh in meshArray:
     temp = mesh.split('/')
     meshName = temp[0]
     print('Replacing material on ' + meshName)
     oldMaterialName = temp[1]
     pattern = r"g[\t ]+" + meshName + r"\s+" + r"usemtl[\t ]+" + oldMaterialName + r"\s+"
     contents = re.sub(pattern, f'g {meshName}\nusemtl {newMaterialName}\n', contents)

  print('Saving')
  writeFile(filename, contents)

  print('Done')


#############################################################

filename = 'incoming-model.obj'

# The material that you want to assign
newMaterialName = 'wire_141007058'

# G3D meshes, obtained by clicking on these meshes in the viewer and
# then copying them to this file.
meshArray = ["Line1982/wire_135059008",
"Line1973/wire_006135058",
"Line1980/wire_006135058"
# etc...
]

process(filename, meshArray, newMaterialName)
