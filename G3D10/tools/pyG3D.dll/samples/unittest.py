import os, sys
from PIL import Image

from pyg3d import *

def gen_lines(file):
    with open(file, "r") as in_file:
        for line in in_file:
            yield line 

def test_vertex_count(file, expected_vertex_count):
    pyg3d.init()

    # ArticulatedModel::loadRBX test
    s = g3d.ArticulatedModelSpecification()
    s.filename = g3d.String(file)
    model = g3d.ArticulatedModel.create(s)
    triangle_count = 0
    vertex_count = 0
    model.countTrianglesAndVertices(triangle_count, vertex_count)
    assert(vertex_count == expected_vertex_count)

    # saveOBJ test
    outDir = "tools/pyG3D.dll/files/"
    outPath = os.path.join(outDir, model.name() + ".obj")
    model.saveOBJ(outPath)
    vertex_count = sum([line.count("v ") for line in gen_lines(outPath)])
    assert(vertex_count == expected_vertex_count)
    os.remove(outPath) # deletes the file after (comment out if want to keep file)

    pyg3d.cleanup()

def test_vertex_count_scene(file, expected_model_count = 0, expected_vertex_count = 0):
    pyg3d.init()

    # Scene::loadRBXL test
    pyg3d.loadScene(file)
    model_table = pyg3d.m_scene.modelTable()
    print("models:", model_table.size())
    # assert(model_table.size() == expected_model_count)
    total_vertex_count = 0
    for entry in model_table:
        vertex_count = entry.value.countVertices()
        print(str(entry.key) + ":", vertex_count)
        total_vertex_count += vertex_count
    # assert(total_vertex_count == expected_vertex_count)
    print("total:", total_vertex_count)

    # saveOBJ test
    outDir = "tools/pyG3D.dll/files/"
    total_vertex_count = 0
    for entry in model_table:
        outPath = os.path.join(outDir, str(entry.key) + ".obj")
        entry.value.saveOBJ(outPath)
        vertex_count = sum([line.count("v ") for line in gen_lines(outPath)])
        print(str(entry.key) + ":", vertex_count)
        total_vertex_count += vertex_count
        # assert(vertex_count == expected_vertex_count)
        os.remove(outPath) # deletes the file after (comment out if want to keep file)
    print("total:", total_vertex_count)

    pyg3d.cleanup()

def subtract(a, b):
    return (a[0]-b[0], a[1]-b[1], a[2]-b[2])

def test_screenshot(load_path, expected_path, name="out.png"):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    pyg3d.init()
    pyg3d.loadScene(load_path)
    pyg3d.render()
    img_path = os.path.join(script_dir, name)
    pyg3d.screenshot(img_path)
    assert(os.path.isfile(img_path))
    with Image.open(img_path) as img:
        with Image.open(expected_path) as expected:
            img = img.convert("RGB")
            expected = expected.convert("RGB")
            width, height = img.size
            img_pixels = img.load()
            expected_pixels = expected.load()
            for y in range(height):
                for x in range(width):
                    diff = subtract(img_pixels[x,y], expected_pixels[x,y])
                    diff_squared_sum = diff[0]**2 + diff[1]**2 + diff[2]**2
                    if diff_squared_sum > 30:
                        print(diff_squared_sum)
                    assert(diff_squared_sum < 30) # an approximate difference threshold for valid renderings based on a couple test scenes
    os.remove(img_path)
    pyg3d.cleanup()

def main():
    # Counts vertices in simple_cornell_box
    test_vertex_count_scene("../data10/common/scene/G3D_Simple_Cornell_Box.Scene.Any")
    
    # Tests that loadScene works with scene files
    test_screenshot("tools/viewer/data-files/ground.Scene.Any", "tools/pyG3D.dll/samples/unittest_assets/ground.Scene.Any.png")
    
    # Tests that loadScene works with scene files with objects
    test_screenshot("../data10/common/scene/G3D_Simple_Cornell_Box.Scene.Any", "tools/pyG3D.dll/samples/unittest_assets/G3D_Simple_Cornell_Box.Scene.Any.png")
   
    # Doesn't work yet because of bugs from merging beta into main
    # test_vertex_count("G3D-rbx.lib/assets/chairs_and_tables.rbxlx", 1488)

    print("TESTS COMPLETE")


if __name__ == "__main__":
    main()
