import os, sys

from pyg3d import *

def main():
    pyg3d.init()
    pyg3d.loadScene("tools/viewer/data-files/ground.Scene.Any")
    pyg3d.render()
    pyg3d.screenshot(os.path.join(script_dir, "files/scene.png"))
    pyg3d.cleanup()

if __name__ == "__main__":
    main()
