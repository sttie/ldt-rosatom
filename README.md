# ldt-rosatom

## Инициализация

```bash
git submodule init --recursive
git submodule update
```

## Сборка

```bash
cmake -DOPENXLSX_BUILD_TESTS=OFF ..
make
./scheduler.exe
```
