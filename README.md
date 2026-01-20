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

```bash
git clone https://github.com/MaximDevoir/covlcov.git --recurse-submodules
```

If you have already cloned the repo without `--recurse-submodules`, you can initialize the submodules with:

```bash
git submodule update --init --recursive
```

## Licensing

This project follows the parent GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
