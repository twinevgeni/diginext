# diginext

**Requirements**

* cmake 3.15+
* gcc 7+ / msvc 2018+ / apple clang 10+
* boost
* nlohman_json
* gtest

**Build**

1. install `vcpkg`

`git clone https://github.com/microsoft/vcpkg`

`cd ./vcpkg && ./vcpkg install`

2. install `boost, nlohman_json, gtest`

`./vcpkg install boost nlohmann-json gtest`

3. build 

`mkdir build && cd build`

`cmake -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release ..`

`cd .. && cmake --build build --target all`

4. run server

``