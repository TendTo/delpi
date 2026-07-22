"""Install script for setuptools."""

import glob
import os
import re
import shutil
import stat
import subprocess
import sysconfig

import setuptools
import setuptools.errors
from setuptools.command import build_ext

PERMISSIONS = (
    stat.S_IWUSR
    | stat.S_IRUSR
    | stat.S_IXUSR
    | stat.S_IWGRP
    | stat.S_IRGRP
    | stat.S_IXGRP
    | stat.S_IROTH
    | stat.S_IXOTH
)


def get_bazel_target_args(command):
    disk_cache = os.environ.get("DELPI_BAZEL_DISK_CACHE", "~/.cache/bazel-pydelpi-disk-cache")
    if command == "build":
        return [
            "bazel",
            "build",
            "--config=python",
            f"--python_version={sysconfig.get_python_version()}",
            f"--disk_cache={disk_cache}",
        ]
    if command == "cquery":
        return [
            "bazel",
            "cquery",
            "--output=files",
            "--config=python",
            f"--python_version={sysconfig.get_python_version()}",
            f"--disk_cache={disk_cache}",
        ]
    if command == "query":
        return [
            "bazel",
            "query",
            "--output=build",
            f"--disk_cache={disk_cache}",
        ]


class GlobalVariables:
    def __init__(self):
        self.DELPI_VERSION = self._get_value_from_query("delpi_version")
        self.DELPI_DESCRIPTION = self._get_value_from_query("delpi_description")

    def _get_value_from_query(self, target):
        bazel_argv = [*get_bazel_target_args("query"), f"//tools:{target}"]
        matches: "list[str]" = re.findall(r"value = +(.+),", subprocess.getoutput(" ".join(bazel_argv)))
        if len(matches) == 0:
            raise ValueError(f"Could not parse {target} from Bazel query output.")
        return matches[0].rstrip('"').lstrip('"')


class BazelExtension(setuptools.Extension):
    """A C/C++ extension that is defined as a Bazel BUILD target."""

    def __init__(self, ext_name, bazel_target):
        self.bazel_target = bazel_target
        folders = ("delpi", "pydelpi", "tools", "third_party", "frontend")
        files = []
        for folder in folders:
            files += [file for file in glob.glob(f"{folder}/**/*", recursive=True) if "node_modules" not in file]
        files += ["BUILD.bazel", "MODULE.bazel", ".bazelversion", ".bazelrc", ".bazelignore", "frontend/.npmrc"]
        setuptools.Extension.__init__(self, ext_name, sources=files)


class BuildBazelExtension(build_ext.build_ext):
    """A command that runs Bazel to build a C/C++ extension."""

    def run(self):
        for ext in self.extensions:
            self.bazel_build(ext)
        if os.environ.get("DELPI_BAZEL_SHUTDOWN", ""):
            # Run the Bazel shutdown command to clean up
            self.spawn(["bazel", "shutdown"])

    def bazel_build(self, ext: BazelExtension):
        if shutil.which("bazel") is None:
            raise setuptools.errors.CompileError(
                "Bazel not found (https://bazel.build/). It is required to install this package from source."
            )

        # Build all needed files
        bazel_argv = [*get_bazel_target_args("build"), "//pydelpi:pydelpi_files"]
        self.spawn(bazel_argv)
        paths = (
            subprocess.check_output([*get_bazel_target_args("cquery"), "//pydelpi:pydelpi_files"])
            .decode("utf-8")
            .strip()
            .split("\n")
        )
        # Copy the built files to the extension directory
        for path in paths:
            file = re.sub(r"^.*pydelpi/", "", path)
            ext_dest_dir = os.path.dirname(self.get_ext_fullpath(ext.name))
            os.makedirs(os.path.join(ext_dest_dir, os.path.dirname(file)), exist_ok=True)
            out_path = os.path.join(ext_dest_dir, file)
            if os.path.isdir(path):
                # If the path is a directory, copy it recursively
                shutil.copytree(path, out_path, dirs_exist_ok=True)
                # Set permissions for all files in the directory
                os.chmod(out_path, PERMISSIONS)
                for root, dirs, files in os.walk(out_path):
                    for name in files + dirs:
                        file_path = os.path.join(root, name)
                        os.chmod(file_path, PERMISSIONS)
            else:
                # If the path is a file, copy it directly
                shutil.copyfile(path, out_path)
                os.chmod(out_path, PERMISSIONS)


config_vars = GlobalVariables()
setuptools.setup(
    version=config_vars.DELPI_VERSION,
    description=config_vars.DELPI_DESCRIPTION,
    ext_modules=[BazelExtension("pydelpi._pydelpi", "//pydelpi:_pydelpi")],
    cmdclass={"build_ext": BuildBazelExtension},
)
