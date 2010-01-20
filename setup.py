from distutils.core import setup, Extension
setup(name="quadtree", version="0.1",
    ext_modules=[Extension("quadtree", ["src/quadtree.cpp","src/pyquadtree.cpp"])])
