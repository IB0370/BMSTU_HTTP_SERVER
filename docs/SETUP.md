# Настройка окружения под задание «Многопоточный HTTP-сервер на C++20»
## Инструкция для второго участника (Windows → WSL2 → Ubuntu 24.04)

Документ описывает настройку с нуля на Windows 10/11 x64. Всё, что ниже, проверено на
Windows 11 Home + Intel i7-10750H + 16 ГБ RAM. Время на настройку — 40–60 минут, из них
большая часть — скачивание пакетов.

Зачем это нужно: задание требует Linux (GCC ≥ 12), ThreadSanitizer и AddressSanitizer,
а также проверку сборки командой `cmake -B build -DCMAKE_BUILD_TYPE=Release`.
На Windows всё это либо недоступно, либо работает с ограничениями, поэтому проект живёт
внутри WSL2 — полноценного Linux-ядра, работающего рядом с Windows без перезагрузок.

---

## 0. Что должно получиться

```
Windows 11
├── Bruno 4.x                  — HTTP-клиент на хосте (проверка ручек API)
└── WSL2 / Ubuntu 24.04        — Linux внутри Windows
    ├── GCC 13, Clang 18, CMake, Ninja, GDB
    ├── ThreadSanitizer / AddressSanitizer / UBSan
    ├── GoogleTest, nlohmann/json
    ├── wrk, oha               — нагрузочные тесты
    ├── valgrind, ccache       — профилирование и быстрые пересборки
    └── ~/projects/server      — клон репозитория
```

Порты пробрасываются автоматически: сервер внутри Linux, запущенный на `:8080`,
доступен в Windows как `http://localhost:8080`. Docker не нужен — он не даёт для этой
задачи ничего, кроме лишних 10 ГБ на диске и 2 ГБ оперативной памяти.

### Требования к машине

| Параметр | Минимум | Комфортно |
|---|---|---|
| RAM | 8 ГБ | 16 ГБ |
| Свободно на C: | 20 ГБ | 40 ГБ |
| ОС | Windows 10 2004+ | Windows 11 |
| Виртуализация | включена в BIOS (VT-x / AMD-V) | — |
| Права | администратор (для шага 1–2) | — |

Проверить свободное место и виртуализацию (PowerShell):

```powershell
Get-Volume -DriveLetter C | Select-Object @{n='FreeGB';e={[math]::Round($_.SizeRemaining/1GB,1)}}
(Get-CimInstance Win32_ComputerSystem).HypervisorPresent
(Get-CimInstance Win32_Processor).VirtualizationFirmwareEnabled
```

`VirtualizationFirmwareEnabled` должен быть `True`. Если `False` — включите
Intel VT-x / SVM в BIOS/UEFI, иначе WSL2 не запустится (ошибка `0x80370102`).

> **Если WSL2 уже установлен и работает** — шаги 1–3 пропустите и переходите к шагу 4.

---

## 1. Включить гипервизор и компоненты WSL (нужен администратор)

Откройте PowerShell **от имени администратора** (Пуск → `PowerShell` → правый клик →
«Запуск от имени администратора») и выполните:

```powershell
# 1. Гипервизор (без него WSL2 не работает)
bcdedit /set hypervisorlaunchtype auto

# 2. Компоненты Windows
dism /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
```

Обе команды `dism` должны завершиться кодом **3010** — это «успешно, нужна перезагрузка»,
а не ошибка.

> ⚠️ **Если у вас установлен VirtualBox**, учтите: после включения гипервизора VirtualBox
> будет работать через WHPX и потеряет 30–50 % производительности в 3D. Это неизбежный
> компромисс: либо WSL2, либо «быстрый» VirtualBox. Для этой задачи нужен WSL2.

**Перезагрузите компьютер.** Без этого шага дальше ничего не заведётся.

После перезагрузки проверьте:

```powershell
(Get-CimInstance Win32_ComputerSystem).HypervisorPresent   # должно быть True
```

---

## 2. Установить современный WSL (версия 3.x)

Важно: в Windows бывает «коробочный» WSL (файл `C:\Windows\System32\wsl.exe`, версия 10.0.x),
который не умеет `--update` и не имеет дистрибутивов. Если попытаться сразу поставить
Ubuntu, получите сообщение «Для этого продукта Windows для Linux не установлены дистрибутивы».

В PowerShell **от имени администратора**:

```powershell
wsl --install --no-distribution --web-download
```

Скачается и установится MSI (~350 МБ). Ожидаемый результат:

```
Загрузка: Подсистема Windows для Linux 3.0.5
Восстановление состояния: Подсистема Windows для Linux 3.0.5
Установка: «Подсистема Windows для Linux 3.0.5» выполнена.
```

Проверка (уже можно без администратора):

```powershell
wsl --version
```

Должно вывести примерно:

```
Версия WSL: 3.0.5.0
Версия ядра: 6.18.58.1-1
Версия WSLg: 1.0.79
Версия по умолчанию: 2
```

`Версия по умолчанию: 2` — критично. Если стоит 1, выполните `wsl --set-default-version 2`.

---

## 3. Установить Ubuntu 24.04

```powershell
wsl --install -d Ubuntu-24.04 --no-launch
wsl -l -v
```

Ожидаемый вывод:

```
  NAME            STATE           VERSION
* Ubuntu-24.04    Stopped         2
```

Флаг `--no-launch` нужен, чтобы не проходить интерактивную первичную настройку
(имя пользователя/пароль) руками — мы сделаем это скриптом на следующем шаге.

---

## 4. Первичная настройка Ubuntu

Запустите подготовительный скрипт от root (он создаст пользователя, включит systemd,
задаст часовой пояс):

```powershell
wsl -d Ubuntu-24.04 -u root -- bash -c "
useradd -m -s /bin/bash -G sudo dev && echo 'dev:dev' | chpasswd;
echo 'dev ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/90-dev; chmod 0440 /etc/sudoers.d/90-dev;
printf '[user]\ndefault=dev\n[boot]\nsystemd=true\n[interop]\nenabled=true\nappendWindowsPath=true\n' > /etc/wsl.conf;
ln -snf /usr/share/zoneinfo/Europe/Moscow /etc/localtime; echo Europe/Moscow > /etc/timezone;
echo done"
```

Затем перезапустите WSL, чтобы применились `/etc/wsl.conf`:

```powershell
wsl --shutdown
wsl -d Ubuntu-24.04 -- bash -lc "whoami; systemctl is-system-running; nproc; free -m | sed -n 2p"
```

Ожидаемо:

```
dev
running
12
Mem:  ...
```

### Лимиты памяти

По умолчанию WSL2 забирает до половины RAM, чего на 16 ГБ мало для одновременной работы
Windows + IDE + сборок под санитайзерами. Создайте файл `C:\Users\<Вы>\.wslconfig`:

```powershell
@"
[wsl2]
memory=6GB
processors=6
swap=2GB
localhostForwarding=true
"@ | Set-Content -Path "$env:USERPROFILE\.wslconfig" -Encoding ASCII
```

Применится после `wsl --shutdown`. Подставьте свои значения: `memory` ≈ 40 % RAM,
`processors` ≈ половина ядер.

---

## 5. Инструменты C++ (один блок команд)

```powershell
wsl -d Ubuntu-24.04 -u root -- bash -c "
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
apt-get install -y --no-install-recommends \
  build-essential gcc-13 g++-13 clang clang-format clang-tidy \
  cmake ninja-build make gdb lldb git curl wget jq unzip ca-certificates \
  python3 python3-pip nlohmann-json3-dev libgtest-dev libgmock-dev libbenchmark-dev \
  ccache valgrind pkg-config htop netcat-openbsd lsof file
apt-get install -y wrk
"
```

Проверка версий:

```powershell
wsl -d Ubuntu-24.04 -- bash -lc "g++-13 --version | head -1; clang --version | head -1; cmake --version | head -1; gdb --version | head -1"
```

Ожидаемо: GCC **13.x** (требование задания — ≥ 12), Clang 18, CMake 3.28, GDB 15.

Установите `oha` (нагрузочный тест с перцентилями):

```powershell
wsl -d Ubuntu-24.04 -u root -- bash -c "curl -fsSL -o /usr/local/bin/oha https://github.com/hatoo/oha/releases/latest/download/oha-linux-amd64 && chmod +x /usr/local/bin/oha && oha --version"
```

---

## 6. Починить санитайзеры (обязательно!)

На новых ядрах TSan в WSL2 падает с `unexpected memory mapping` или
`Shadow memory range interleaves with an existing memory mapping` из-за высокой энтропии ASLR.
Лечится понижением `vm.mmap_rnd_bits`:

```powershell
wsl -d Ubuntu-24.04 -u root -- bash -c "
printf 'vm.mmap_rnd_bits = 28\n' > /etc/sysctl.d/99-sanitizers.conf
sysctl -w vm.mmap_rnd_bits=28
"
```

Проверка, что санитайзеры действительно работают (не пропускайте — иначе рискуете
получить «чистый» прогон, который ничего не проверяет). Выполнять **в терминале Ubuntu**:

```bash
cd ~
cat > race.cpp <<'EOF'
#include <thread>
int main() {
    int x = 0;
    std::thread a([&]{ x = 1; });
    std::thread b([&]{ x = 2; });
    a.join(); b.join();
}
EOF
g++-13 -std=c++20 -fsanitize=thread -g -O1 race.cpp -o race && ./race; echo "exit=$?"
```

Должен появиться отчёт `WARNING: ThreadSanitizer: data race` и код возврата **66**.
Именно 66 — значит, TSan работает и в CI упадёт на гонке. Если кода 66 нет, а есть
`FATAL: ThreadSanitizer: unexpected memory mapping` — вернитесь к `sysctl` выше.

---

## 7. VS Code и расширения

Установите VS Code, затем в Windows (PowerShell):

```powershell
code --install-extension ms-vscode-remote.remote-wsl
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
```

Открыть проект правильно:

```powershell
wsl -d Ubuntu-24.04 -- bash -lc "cd ~/projects/server && code ."
```

Признак корректного открытия — в левом нижнем углу окна VS Code зелёная плашка
`>< WSL: Ubuntu-24.04`. Если плашки нет, вы открыли папку на Windows, и компилятор
из Linux не будет виден.

> Файлы проекта живут в `/home/dev/projects/...`, а не на диске C. Из Windows они видны
> по UNC-пути `\\wsl.localhost\Ubuntu-24.04\home\dev\projects\server`, и через него
> **не надо** собирать проект: доступ к `/mnt/c/...` медленный, а `inotify` не работает.

---

## 8. Клонировать репозиторий и собрать

В терминале Ubuntu (или в терминале VS Code, `Ctrl + ~`):

```bash
mkdir -p ~/projects
cd ~/projects
git clone git@github.com:IB0370/BMSTU_HTTP_SERVER.git server
cd server
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/server
```

Ожидаемый результат: `Hello, server!` (на момент скелета), при готовом сервере —
он молча ждёт запросы на порту.

Сборки под санитайзеры (как требует п. 3 задания):

```bash
cmake -B build-tsan -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" && cmake --build build-tsan -j
cmake -B build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g" && cmake --build build-asan -j
```

Отладочная сборка (для точек останова; в Release они «уезжают»):

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O1 -g" && cmake --build build-debug -j
```

Ускорение пересборок (по желанию): добавьте в `~/.bashrc` строку
`export PATH=/usr/lib/ccache:$PATH` — тогда `g++`/`clang++` будут ходить через ccache.

---

## 9. Bruno (проверка API с хост-машины)

```powershell
winget install --id Bruno.Bruno -e
```

Дальше:

1. Запустить сервер в WSL: `./build/server --port 8080 --threads 8`
2. В Bruno создать коллекцию, окружение с `baseUrl = http://localhost:8080`
3. Проверить ручку: `GET {{baseUrl}}/health` → `200 {"status":"ok"}`

Проверка без Bruno (в отдельном терминале):

```bash
curl -i localhost:8080/health
curl -i -X POST localhost:8080/items -H 'Content-Type: application/json' -d '{"name":"a","value":1}'
```

Если `curl` из Linux работает, а из Windows `http://localhost:8080` — нет, выполните
`wsl --shutdown` и проверьте, что в `.wslconfig` стоит `localhostForwarding=true`.

---

## 10. Git: подпись, ключ, процесс

```bash
git config --global user.name "Фамилия Имя"
git config --global user.email "you@example.com"
git config --global init.defaultBranch main
git config --global core.autocrlf input   # чтобы CRLF из Windows не ломал сборку
```

SSH-ключ для пуша (без пароля — проще для учебного репозитория):

```bash
ssh-keygen -t ed25519 -C "you@example.com" -N "" -f ~/.ssh/id_ed25519
cat ~/.ssh/id_ed25519.pub
```

Публичную строку вставить в GitHub: **Settings → SSH and GPG keys → New SSH key**.
Проверка: `ssh -T git@github.com` → `Hi <логин>! You've successfully authenticated`.

Перевести существующий клон на SSH:

```bash
git remote set-url origin git@github.com:IB0370/BMSTU_HTTP_SERVER.git
```

Правила репозитория из задания (обязательны к соблюдению):

- `main` защищена, изменения — только через PR;
- каждый PR проходит ревью второго участника (осмысленный комментарий или approve);
- коммиты в стиле Conventional Commits: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `perf:`;
- вклад каждого — не меньше 30 % содержательных коммитов;
- теги на контрольных точках: `v0.1`, `v0.2`, `v0.3`, `v1.0`;
- в репозитории нет `build/`, бинарников и IDE-файлов.

---

## 11. Ежедневный цикл работы

```bash
cd ~/projects/server
git switch main && git pull            # получить свежий код
git switch -c feat/название-задачи     # новая ветка на задачу

# ... пишем код в VS Code ...
cmake --build build -j                 # собрать
./build/server --port 8080 --threads 8 # запустить (Ctrl+C — остановить)

git add -A
git commit -m "feat: ..."
git push -u origin feat/название-задачи
# → открыть PR на GitHub → получить ревью → merge
```

Отладка в VS Code: поставить точку останова щелчком слева от номера строки → `F5`.
`F10` — шаг через строку, `F11` — войти в функцию, панель Debug показывает переменные,
стек и список потоков (для многопоточной модели это основной инструмент).

Тесты:

```bash
cmake --build build -j && cd build && ctest --output-on-failure
```

---

## 12. Чек-лист приёмки окружения

- [ ] `wsl --version` → 3.x, `Версия по умолчанию: 2`
- [ ] `wsl -d Ubuntu-24.04 -- bash -lc "whoami"` → `dev`
- [ ] `systemctl is-system-running` → `running`
- [ ] `g++-13 --version` → GCC ≥ 13
- [ ] `cmake --version` → 3.28+
- [ ] TSan-пример даёт `data race` и код **66**
- [ ] `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j` → без ошибок
- [ ] `curl -s localhost:8080/health` из Windows → `200`
- [ ] Bruno видит `{{baseUrl}}/health`
- [ ] точка останова в VS Code останавливает поток
- [ ] `ssh -T git@github.com` → успешная аутентификация
- [ ] `cat ~/.wslconfig` → лимит памяти задан

---

## 13. Типичные проблемы

| Симптом | Причина | Решение |
|---|---|---|
| `wsl --status` → «не установлены дистрибутивы, выполните wsl.exe --install» | стоит «коробочный» WSL без пакета | шаг 2: `wsl --install --no-distribution --web-download` |
| `wsl --update` → та же ошибка | то же самое | не пользоваться `--update` до установки WSL 3 |
| Ошибка `0x80370102` при старте | виртуализация выключена в BIOS / нет `VirtualMachinePlatform` | включить VT-x, повторить шаг 1 |
| WSL не стартует после шага 1 | не было перезагрузки | перезагрузить |
| TSan: `unexpected memory mapping` | энтропия ASLR | `vm.mmap_rnd_bits=28` (шаг 6) |
| TSan молчит на явной гонке | сборка без `-fsanitize=thread` | пересобрать с флагом, отдельный `build-tsan` |
| Точка останова «серая», не срабатывает | Release-сборка / нет `-g` | `build-debug` с `-DCMAKE_BUILD_TYPE=Debug -O1 -g` |
| Файлы из `/tmp` исчезают после перезапуска | WSL очищает `/tmp` при старте | хранить данные в `~/projects`, не в `/tmp` |
| Сборка в `/mnt/c/...` очень медленная | 9p-монтирование Windows-диска | проект держать в `/home/dev/...` |
| В git попали CRLF, сборка ругается | Windows-редактор | `git config --global core.autocrlf input`, `core.eol lf` |
| `Permission denied (publickey)` при push | ключ не добавлен в GitHub | `cat ~/.ssh/id_ed25519.pub` → GitHub → SSH keys |
| Bruno не видит сервер, curl в Linux видит | проброс `localhost` | `wsl --shutdown`, проверить `localhostForwarding=true` |
| VirtualBox стал тормозить | гипервизор включён | ожидаемо; для этой задачи VirtualBox не нужен |
| `perf: command not found` / не работает | нет сборки под WSL-ядро | профилировать `valgrind --tool=callgrind` |
| `linux-perf: no installation candidate` | пакета нет для WSL | то же; не пытаться ставить |
| Мало места на C: | WSL-диск растёт | `wsl --manage Ubuntu-24.04 --move <другой диск>` |

---

## 14. Полезные мелочи

```bash
wsl --shutdown                     # полностью остановить WSL (освободить RAM)
wsl --manage Ubuntu-24.04 --move D:\WSL   # перенести дистрибутив на другой диск
df -h /                            # сколько занял Linux
du -sh ~/projects/server/build*    # сколько занимают сборки
```

Полный путь к диску Ubuntu: `C:\Users\<Вы>\AppData\Local\wsl\{GUID}\ext4.vhdx`.
Он растёт динамически и не сжимается сам; если место кончилось —
`wsl --shutdown`, затем `Optimize-VHD` или перенос через `--move`.

---

*Документ актуален для WSL 3.0.5 / Ubuntu 24.04.5 / GCC 13.3 / CMake 3.28.
Если какая-то команда выдаёт не то, что написано в «ожидаемом» — не идите дальше,
пришлите вывод команды напарнику: почти всегда причина в предыдущем шаге.*
