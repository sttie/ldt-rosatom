# ldt-rosatom

## Инициализация

```bash
git submodule update --init --recursive
```

## Сборка и запуск

### Linux/MacOS

```bash
cmake -DOPENXLSX_BUILD_TESTS=OFF .
make
pip install flask geopandas matplotlib pandas numpy shapely
bash run.sh
```

После запуска скрипта run.sh перейдите на локальную страничку с портом 8111 (В командной строке Flask покажет эту ссылку). Затем выберите все файлы для запуска программы (файл с ледовой проходимостью, графом и кораблями), а также размер каравана и нажмите кнопку "Начать". Файлы можно найти в папке dataset. Программа проработает несколько минут. Процесс можно наблюдать в консоли. По завершению работы программы на сайте отобразится расписание с диаграммой Ганта. Также результат будет находится в файле webservice/run/schedule.json. 


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
