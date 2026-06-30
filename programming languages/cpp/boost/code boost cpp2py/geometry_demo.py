# & "C:\projects\vcpkg_installed\kb_cpp2py\x64-windows\tools\python3\python.exe" "C:\projects\knowledge_base\programming languages\cpp\boost\code boost cpp2py\geometry_demo.py"

import os, sys
build_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".build")
if os.name == "nt":
    os.add_dll_directory(build_dir)
if build_dir not in sys.path:
    sys.path.insert(0, build_dir)

import geometry

p1 = geometry.Point(0, 0)
p2 = geometry.Point(3, 4)
print(p1.distance_to(p2))
print(p1)

p1.x = 10
print(p1.x)