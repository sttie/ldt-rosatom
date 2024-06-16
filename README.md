# ldt-rosatom

## Инициализация

```bash
git submodule init --recursive
git submodule update
```

## Сборка и запуск

### Linux/MacOS

```bash
mkdir build
cd build
cmake -DOPENXLSX_BUILD_TESTS=OFF ..
make
cp scheduler ../
bash run.sh
```

После выполнения скрипта run.sh появится json файл с результатом. Нужно зайти на сайт website/index.html и открыть этот json файл на сайте. 

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

Для корректного запуска scheduler.exe требуется повторить действия скрипта run.sh
