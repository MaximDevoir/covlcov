# covlcov

[![.github/workflows/push.yml](https://github.com/MaximDevoir/covlcov/actions/workflows/push.yml/badge.svg)](https://github.com/MaximDevoir/covlcov/actions/workflows/push.yml)
[![codecov](https://codecov.io/gh/MaximDevoir/covlcov/graph/badge.svg?token=6LDRVMWCQU)](https://codecov.io/gh/MaximDevoir/covlcov)

> An lcov exporter for OpenCppCoverage.

## Usage

To generate lcov reports, use the `lcov` export type in your command:

```bash
--export_type=lcov
--export_type=lcov:reports/coverage.info
```

## `.covlcov` Format

The plugin will walk up the directory from the current script execution and look for a `.covlcov` file to read the configuration from.

The LCOV exporter can optionally restrict which files appear in the report and how their paths are written.
This behavior is controlled by two options in a .covlcov configuration file:

`includeByBaseDir`: `true` or `false` (optional, default `false`): Controls if the file should be included in the report. When it is enabled, only
files whose real path lies inside the resolved `baseDir` are included in the LCOV report, and their `SF:` entries are made relative to that base
directory whenever possible. Files outside `baseDir` are skipped from the report, and paths for included files are shortened to be relative to the
base. When `includeByBaseDir` is disabled, all files are included and their paths are left as‑is, regardless of `baseDir`.

`baseDir`: `<path>` (optional): Defines the logical “root” directory for your coverage report. It is interpreted relative to where the `.covlcov` file
lives (unless you use an absolute path), and then resolved to a full path. When active, this base directory is used as the reference point for source
file paths in the LCOV output: any file under `baseDir` can be written with a path that is relative to it (for example, `src/main.cpp` instead of
`C:/project/src/main.cpp`). In short, `baseDir` tells the exporter, “this is where my project starts; treat everything under here as part of the
report’s root".

Typically, if `.covlcov` is in the root of your project, you can set `includeByBaseDir` to `true` and leave `baseDir` unset. This will only include
files under the root directory in the report.

## Development

Clone the repo and initialize the submodules:

```bash
git clone https://github.com/MaximDevoir/covlcov.git --recurse-submodules
```

If you have already cloned the repo without `--recurse-submodules`, you can initialize the submodules with `git submodule update --init --recursive`.

The `bootstrap.bat` script will initialize the submodules for you.

### Building

We will build using the `Developer Command Prompt for VS` (VS 18) command prompt.

A bootstrap script is provided to build the project. This will build the project in both Debug and Release configurations.

```bash
.\bootstrap.bat
```

### Testing

> Testing will require building in both Debug and Release configurations for covlcov and OpenCppCoverage.

If your IDE supports running tests, great. Otherwise, you can run `lcovTest.exe` after compiling:

Note: For E2E test to succeed, you must run within the `lcovTest` directory.

```pwsh
cd lcovTest
.\..\x64\Debug\lcovTest.exe
or
.\..\x64\Release\lcovTest.exe
```

## Licensing

This project follows the parent GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
