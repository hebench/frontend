# Contributing

All HEBench repos welcome pull requests from external contributors. In general, HEBench repositories require 3 steps for proper contribution:

1. [Code Formatting](#formatting)
2. [Coding Style Guidelines](#codingstyle)
3. [Building & Running](#building-and-running)
4. [Creating a Pull Request](#pull-request-template)

See the [Installing Resources](#resources) section for details on installing/setting up all of the requirements below.

In general, when wanting to contribute code, users should expect to take the following steps:
1. Fork from the repository that is being contributed to. If the user is part of the [HEBench](https://github.com/hebench) GitHub organization, they will be able create branches and push directly to the HEBench repositories without the need to Fork. Contact support@hebench.org for more information.
2.  Create a branch prefixed with the contributor's GitHub username, and a title describing the content of the branch (e.g. username/titled_describing_branch).
3.  Confirm that all of the requirements have been satisfied for contributing (see below and the [HEBench PR Template](https://github.com/hebench/frontend/blob/main/.github/PULL_REQUEST_TEMPLATE.md)).
4.  Create a Pull Request (either from the forked repository or the created branch) into the HEBench *development* branch of the repository being merged into. Note that PR's into branches other than *development* will be closed.
5.  Pending CI passing and CODEOWNER approval, the PR will be merged into the *development* branch. With a consensus from HEBench organization members, *development* will be merged into the *main* branch periodically.

## Code Formatting <a name="formatting"></a>

### Requirements
- clang-format (tested with clang-format-9)
- pre-commit (tested with version 1.15.2)
- doxywizard (optional)

### Steps to Follow
To contribute, please, install **clang-format** on your dev system and **pre-commit** tool and hooks on your local repo clone. This will ensure adherence to code formatting and enforce license text on all C++ source files. For visual doxygen documentation file editing, **doxywizard** is recommended, but not necessary.

Use the following commands at the root of the project to check formatting prior to committing (assuming all requirements are installed):
```bash
pre-commit install
pre-commit run --all-files
git status # Check if any files were changed by pre-commit
git add .  # assuming files were changed
git commit # assuming files were changed
```

## Coding Style Guidelines <a name="codingstyle"></a>
Code should match the following guidelines in order to maintain a consistent style across the project.

### Test class template example

example.h with coding style applied
```cpp
#ifndef __TESTCASE_H_
#define __TESTCASE_H_

namespace hebench {

class TestCase
{
public:
    static const int DefaultValue = 1;
    TestCase();
    /**
     * @brief Sets a new size for input geometry.
     * @param input_size[in] New input geometry size.
     * @details If the \p input_size value is not positive, the input size
     * will be reset to DefaultValue.
     */
    void setInputGeometry(int input_size);
private:
    int m_input_size;
};
}

#endif
```
Test class example .cpp with coding style applied
```cpp
#include "example.h"

namespace hebench {

TestCase::TestCase() :
    m_input_size(DefaultValue)
{ }

void TestCase::setInputGeometry(int input_size)
{
    // Set a new size for the input geometry.
    // Must be greater than zero.
    m_input_size = (input_size > 0 ? input_size : TestCase::DefaultValue);
}

}
```
### Indentation
 - Spaces preferred for indentation.
 - Indents must be 4 spaces wide if using spaces instead of tabs.

### Naming Conventions
 - Class, Structs, Enums, and other custom type names: CamelCase starting with UpperCase first letter
 - Constants: Follow same rules as classes
 - Defines: uppercase underscore style
 - Function and method names: CamelCase starting with lowerCase first letter
 - Member names: lowercase underscore style starting with m_ prefix
 - Parameters and local variables: lowercase underscore style

### Documentation Style
Documenting classes, methods, members, etc., should follow Doxygen-style documentation.

## Building & Running <a name="building-and-running"></a>

### Requirements
- CMake 3.13+
- C++17 capable compiler (tested with GCC version 9.3)
- GLIBC (tested with ldd version 2.31)

### Steps to Follow
Each repository in HEBench has build and run steps specific to the particular component. Please refer to [Building](https://github.com/hebench/frontend#building) and [Running the Benchmark](https://github.com/hebench/frontend#running-the-benchmark) for the steps specific to the HEBench Frontend.

## Creating a Pull Request <a name="pull-request-template"></a>

### Requirements
- Git 2.25+

### Steps to Follow
All Pull Requests must follow the the provided [HEBench PR Template](https://github.com/hebench/frontend/blob/main/.github/PULL_REQUEST_TEMPLATE.md). When you go to create a [New Pull Request](https://github.com/hebench/frontend/compare), the template will be populated in the PR message. Please fill in the empty fields as required.

## Installing Resources <a name="resources"></a>

If wanting a quick script to install the default dependencies, feel free to utilize/run the [Install Requirements](.github/workflows/install-requirements.sh) to save some time. Note that this script primarily installs `clang-format-9`, `pre-commit`, `CMake`, and `gcc`. If there is a previous installation, this script may uninstall them as part of this process. To avoid this happening (or to simply install the requirement manually), please feel free to continue below.

### clang-format-9
Ubuntu 16.04:
```bash
sudo bash
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9 main"
apt-get update
apt-get install clang-format-9
exit
```

Ubuntu 18.04/20.04:
```bash
sudo apt-get install clang-format-9
```

### pre-commit: [https://pre-commit.com](https://pre-commit.com/)
*Note that we use a python virtual environment to avoid potentially touching system python components*

Ubuntu 16.04/18.04/minimal 20.04:
```bash
sudo apt-get install python3-pip
python3 -m venv VIRTUAL_ENV_DIR
source VIRTUAL_ENV_DIR/bin/activate # This command must be run whenever using pre-commit
echo "appdirs==1.4.4
cfgv==3.2.0
distlib==0.3.1
filelock==3.0.12
identify==2.2.4
nodeenv==1.6.0
pre-commit==2.12.1
PyYAML==5.4.1
six==1.16.0
toml==0.10.2
virtualenv==20.4.6" > requirements.txt
python3 -m pip install -r requirements.txt
deactivate
```

### doxywizard (optional):
```bash
sudo apt-get install doxygen-gui
```

### CMake
```bash
VERSION=3.16.3 # Use any version 3.13+
wget https://github.com/Kitware/CMake/releases/download/v$VERSION/cmake-$VERSION-linux-x86_64.sh
chmod +x cmake-$VERSION-linux-x86_64.sh
sudo ./cmake-$VERSION-linux-x86_64.sh --prefix=/usr/local # or /usr if not using local hierarchy
```

### GCC
```bash
sudo apt install build-essential
sudo apt -y install gcc-9 g++-9
```
Ubuntu 16.04/18.04:
```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 900 --slave /usr/bin/g++ g++ /usr/bin/g++-9
```
Ubuntu 20.04:
```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 9
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 9
```

### Git
```bash
sudo apt-get install git
```
