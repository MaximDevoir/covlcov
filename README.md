# covlcov

> An lcov exporter for OpenCppCoverage

## Usage

To generate lcov reports, use the `lcov` export type in your command:

```bash
--export_type=lcov
--export_type=lcov:reports/coverage.info
```

## Development

Clone the repo and initialize the submodules:

**Note**: The submodule source is set to a private repository. Do not ``--recurse-submodules`` if you do not have access.
*Change the submodule url to the public repository before initializing.

```bash
git clone https://github.com/MaximDevoir/covlcov.git --recurse-submodules
```


If you have already cloned the repo without `--recurse-submodules`, you can initialize the submodules with:

```bash
git submodule update --init --recursive
```

### Testing

After building the solution, run the tests within lcovTest or use the following command in the build directory:

## Licensing

This project follows the parent GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
