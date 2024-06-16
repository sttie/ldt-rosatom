# ldt-rosatom

## Инициализация

```bash
git submodule init --recursive
git submodule update
```

## Сборка

### Linux/MacOS

```bash
mkdir build
cd build
cmake -DOPENXLSX_BUILD_TESTS=OFF ..
make
./scheduler.exe
```

### Windows

Для сборки необходимо иметь утилиту ```msbuild``` (обычно поставляется вместе с Visual Studio). Ее можно запустить через стандартную командную строку ```Developer Command Prompt```.

Также, необходимо установить ```CMake For Windows```.

Более простой альтернативой может быть сборка с использованием CLion. В него изначально встроена поддержка CMake.

Если, все-таки, хочется собрать через ```Developer Command Prompt```:

```bash
mkdir build
cd build
cmake -DOPENXLSX_BUILD_TESTS=OFF ..
msbuild .\scheduler.vcxproj

start Debug\scheduler.exe
# ИЛИ (зависит от настроек системы)
start Release\scheduler.exe
```
