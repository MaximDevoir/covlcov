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

.\OpenCppCoverage\InstallThirdPartyLibraries.ps1

# If your Windows execution policy is restricted from running scripts try running a one-time bypass:

powershell -ExecutionPolicy Bypass -File .\OpenCppCoverage\InstallThirdPartyLibraries.ps1
```

### Testing

The easiest way to test is to use your IDE to run the tests within `lcovTest`.

You can also use [act](https://github.com/nektos/act) to run the tests locally:

```bash
act -P windows-latest=-self-hosted
```

## Licensing

This project follows the parent GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
