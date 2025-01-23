import subprocess
import argparse
import sys
import glob
import time
import shutil

arg_parser = argparse.ArgumentParser("setup")
arg_parser.add_argument("-c", "--clean", help="clean all build files", action="store_true")
arg_parser.add_argument("-l", "--list-deps", help="list all current dependencies", action="store_true")
arg_parser.add_argument("--copy-libs", help="copy shared libraries to desired destination")
arg_parser.add_argument("--build-dep", help="build only a specific dependency", action="append")
arg_parser.add_argument("-b", "--build-all", help="build and install all dependencies", action="store_true")
arg_parser.add_argument("-v", "--verbose", help="print debug information", action="store_true")
program_args = arg_parser.parse_args()

def log(message, delay = 2, verbose = False):
    if verbose and not program_args.verbose:
        return
    
    print(f"(setup.py) ------- {message}")
    time.sleep(delay)

def append_args(args, other = None):
    if type(other) == str:
        return args + [ other ]
    elif type(other) == list:
        return args + other

def cmake_command(args):
    command = [ "cmake" ]
    command = append_args(command, args)
    
    if sys.platform.startswith('win'):
        shell = True
    else:
        shell = False
    
    log(command, verbose=True)

    code = subprocess.check_call(command, stderr=subprocess.STDOUT, shell=shell)
    if code != 0:
        raise RuntimeError("Command execution failed.")
    return code

def dep_generate_project(name, gen_args = None):
    log(f"Generating project for dependency '{name}'.")
    args = [ "-S", f"deps/{name}", "-B", f"deps/{name}/build" ]
    args.append(f"-DCMAKE_INSTALL_PREFIX=deps/{name}/install")
    args = append_args(args, gen_args)
    return cmake_command(args)

def dep_install_project(name, build_args = None):
    log(f"Installing dependency '{name}'.")
    args = [ "--build", f"deps/{name}/build", "--target", "install" ]
    args = append_args(args, build_args)
    return cmake_command(args)

class Dependency:
    def __init__(self, name):
        self.name = name
        self.g_args = []
        self.b_args = []
    def gen_args(self, args):
        self.g_args = append_args(self.g_args, args)
        return self
    def gen_flag(self, flag):
        self.g_args = append_args(self.g_args, f"-D{flag}")
        return self
    def build_args(self, args):
        self.b_args = append_args(self.b_args, args)
        return self
    def shared_library(self):
        self.gen_args("-DBUILD_SHARED_LIBS=ON")
        self.is_shared = True
        return self
    def static_library(self):
        self.gen_args("-DBUILD_SHARED_LIBS=OFF")
        self.is_shared = False
        return self
    def setup(self):
        dep_generate_project(self.name, self.g_args)
        dep_install_project(self.name, self.b_args)

dependencies = [
    Dependency('glfw')
        .gen_flag('GLFW_BUILD_EXAMPLES=OFF')
        .shared_library(),
    Dependency('fastgltf')
        .shared_library(),
    Dependency('nlohmann-json')
        .shared_library(),
    Dependency('glm')
        .gen_flag('GLM_BUILD_TESTS=OFF')
        .static_library(),
    Dependency('reactphysics3d')
        .static_library(),
]

def dep_get(name):
    for dep in dependencies:
        if name == dep.name:
            return dep

if program_args.clean:
    log("Cleaning all build directories")
    shutil.rmtree("build")
    for dep in dependencies:
        shutil.rmtree(f"deps/{dep.name}/build")
        shutil.rmtree(f"deps/{dep.name}/install")
    log("Build directories cleaned")

if program_args.list_deps:
    log("Current dependencies:", delay = 0)
    for dep in dependencies:
        print(f"(dependency) {dep.name}")
        print(f"- CMake args: {dep.g_args}")
        print(f"- Build args: {dep.b_args}")

if program_args.build_all:
    log("Building all dependencies.")
    for dep in dependencies:
        dep.setup()
elif program_args.build_dep:
    for name in program_args.build_dep:
        dep = dep_get(name)
        dep.setup()

def create_prefix_path():
    res = "-DCMAKE_PREFIX_PATH="
    for dep in dependencies:
        res += f"deps/{dep.name}/install;"
    return res

log("Generating project files")
gen_args = [ "-S", ".", "-B", "build", create_prefix_path() ]
cmake_command(gen_args)

if program_args.copy_libs:
    log(f"Copying shared libraries to '{program_args.copy_libs}'")
    for dep in dependencies:
        if dep.is_shared:
            dll_files = glob.glob(f"deps/{dep.name}/install/bin/*.dll")
            for dll_file in dll_files:
                log(f"Copying '{dll_file}'", delay=0, verbose=True)
                shutil.copy(dll_file, program_args.copy_libs)
        