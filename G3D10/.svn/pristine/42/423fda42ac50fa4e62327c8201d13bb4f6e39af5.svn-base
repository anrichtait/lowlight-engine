import os, sys

from pyg3d import *

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file = "tools/pyG3D.dll/files/boxModel.obj"
    pyg3d.init()
    pyg3d.loadScene("tools/viewer/data-files/ground.Scene.Any")
    s = g3d.ArticulatedModelSpecification()
    s.filename = g3d.String(file)
    model = g3d.ArticulatedModel.create(s)
    pyg3d.insertEntity(model)
    
    pyg3d.render()
    pyg3d.screenshot(os.path.join(script_dir, "files/model.png"))
    pyg3d.cleanup()

if __name__ == "__main__":
    main()
