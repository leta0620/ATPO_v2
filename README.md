# Anolog-transistor-placement-optimizer
A small C++ project organized with CMake so it can be built on Visual Studio (Windows) and g++/Clang (Linux/macOS).
使用 CMake 管理，支援 Visual Studio（Windows） 與 g++/Clang（Linux/macOS）編譯。

## Directory Layout | 專案結構
```
├─ CMakeLists.txt
├─ header/
│  ├─ DeviceUnit.h
│  ├─ Group.h
│  ├─ InitialPlacement.h
│  ├─ IntermidiatePlacement.h
│  ├─ NetListLookupTable.h
│  ├─ NetlistUnit.h
│  ├─ OuterInput.h
│  ├─ Output.h
│  ├─ SAManager.h
│  ├─ TableManager.h
|  └─ Test.h
└─ src/
   ├─ main.cpp
   ├─ DeviceUnit.cpp
   ├─ Group.cpp
   ├─ InitialPlacement.cpp
   ├─ IntermidiatePlacement.cpp
   ├─ NetListLookupTable.cpp
   ├─ NetlistUnit.cpp
   ├─ OuterInput.cpp
   ├─ Output.cpp
   ├─ SAManager.cpp
   ├─ TableManager.cpp
   └─ Test.cpp
```

## Prerequisites
```
1. 安裝Visual Studio 2022。
2. 安裝CMAKE(CMake ≥ 3.20)。
```

## Build & Run on Windows (Visual Studio)
先在資料夾中新增build資料夾，並用 CMake 產生 .sln，使用CMAKE，並輸入：
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

接著用 VS 打開：build/AnalogTransistorPlacementOptimizer.sln
在 Solution Explorer 對 ATPO_v2 右鍵 → 設為啟動專案 → ctrl+F5執行。

## Build & Run on Windows/linux (g++)
```
mkdir build
cd build

cmake ..
make
```
編譯的可執行檔會在bin資料夾中

## Testinng
在傳入的參數中，若僅傳入 "test"，會進入測試模式(test class)，請不要更動main funtion中的內容，將你的test function定義在test class中，並透過test class的建構式運行實作(test.cpp)。

## Coding Style
變數：首字母小寫，後續字母大寫開頭。
``` e.g. string circuitType;```

函式、類別：全字母開頭大寫。
```
e.g. InItialPlacenent initialPlacement();
e.g. class SAManager{};
```

若要新增variable/function，請新增在類別中的private or protected中，保持class的封裝，需新增public variable/function請先和其他人確認此部分是否會影響並討論其合理性。
*合併遠端(github)主幹請使用github網頁提交merge request，若發生衝突無法提交合併申請，請先在本地端將main分支pull下來到最新版，並且將目前main分支的內容合併到自己的分支中，解決完衝突後，重新將自己的分支推上去，再從網頁提交合併。

## command example
sa_mode: 0 = random mode / 1 = CC mode / 2 = Interleaving mode
```
<groupSize> <rowSize> <CDL_input_file_path> <output_file_path> <thread_num> <left_is_S_or_D> <mode> <sa_round_per_temp>

4 4 ./cdl_test.txt ./output/ 1 S 1 100
```

