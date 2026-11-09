# Лабораторная работа № 3 — Упаковка приложения в контейнер

Вариант **7**. Основание: «Занятие 5.pdf», §16, стр. 32–36. Номер работы в вашем курсе — 3, в методичке — 5. Здесь выполняются обязательные задания 1–9; П1–П3 приведены отдельно по желанию. **Этап сквозного проекта (§16.8) исключён по вашему указанию.**

Рядом находится шаблон `Облачные технологии. Лабораторная работа 3. Шаблон отчета.docx`. Номера скриншотов ниже совпадают с пометками в шаблоне. Можно вставлять текстовый вывод вместо снимков терминала. Реальные размеры, время, дайджесты и измерения нужно получить на своей машине. После заполнения сохраните отчёт в PDF — это требование §16.2.

## 0. Подготовка — один раз

Docker Desktop уже установлен: включите Linux containers и Settings → Resources → WSL Integration → Ubuntu. Не устанавливайте второй Docker Engine в Ubuntu поверх Docker Desktop. AWS, Moto и VirtualBox для этой работы не нужны. AMD Ryzen 5 5600 соответствует платформе `linux/amd64`.

В PowerShell проверьте WSL и откройте Ubuntu:

```powershell
wsl --list --verbose
wsl -d Ubuntu
```

Если дистрибутив называется иначе, подставьте его имя. **Все дальнейшие команды выполняются в Bash внутри Ubuntu WSL**, не в PowerShell. Интервал 500 мс — шаг временных меток генератора, программа не ожидает по 500 мс между котировками.

Работайте с копией опорного проекта в файловой системе WSL: там корректно работают Linux-права, а сборка быстрее, чем через `/mnt/d`. Исходные файлы в `lab3/files` останутся исходной опорой. Если папка назначения уже существует, выберите другое имя, чтобы не перезаписать свою работу.

```bash
mkdir -p ~/cloud-labs
cp -a /mnt/d/Projects/bsu/cloud-lab/lab3/files/quantflow ~/cloud-labs/lab3-var07
cd ~/cloud-labs/lab3-var07
mkdir -p ../lab3-var07-evidence
export EVIDENCE="$(cd ../lab3-var07-evidence && pwd)"
set -o pipefail

VAR_ARGS=(--symbol GOLF --seed 1259 --base 10250 --step 7 \
  --interval 500 --count 1000 --sma 12 --ema 16 --rsi 21)
```

`EVIDENCE` хранится **за пределами контекста сборки**: новые журналы не должны сбрасывать кэш `COPY . .`. В новом терминале снова выполните `cd`, `export EVIDENCE=...`, `set -o pipefail` и объявление `VAR_ARGS`. Для редактирования удобно открыть этот каталог через `code .` (VS Code с WSL).

Создайте `.dockerignore` до первой сборки; он будет общим для трёх Dockerfile:

```bash
cat > .dockerignore <<'EOF'
.git
.aws
.env
.env.*
build
build-*
out
__pycache__
*.pyc
*.log
*.tar
*.bak
EOF
```

Параметры из таблицы 16.1 и приложения Г к занятию 4:

| Параметр | Вариант 7 |
|---|---|
| Инструмент / seed | GOLF / 1259 |
| Начальная цена / шаг | 10250 / 7 копеек |
| Интервал / число котировок | 500 мс / 1000 |
| SMA / EMA / RSI | 12 / 16 / 21 |
| Пределы ресурсов | `--cpus 1 --memory 512m` |
| Свой индикатор | `mean(r) / s(r)`, выборочное СКО |
| Регион схемы из предыдущей работы | `sa-east-1`, Южная Америка, Сан-Паулу |

Команды `docker build` автоматически выполняют тесты в исходных Dockerfile. Если сборка завершилась ошибкой, сначала прочитайте журнал, не убирайте `ctest`.

## 1. Установка и проверка

```bash
docker version | tee "$EVIDENCE/01-version.txt"
docker info | tee "$EVIDENCE/01-info.txt"
docker info | grep -E 'Operating System|Architecture|Storage Driver|Cgroup Version'
docker run --rm hello-world | tee "$EVIDENCE/01-hello.txt"
```

Снимок №1: версии клиента и сервера, три требуемых поля `docker info`, приветствие hello-world. Укажите фактическую ОС из вывода. Она относится к Linux-среде демона Docker Desktop, поэтому отличается от Windows 11. Если отсутствует секция Server, запустите Docker Desktop и проверьте WSL Integration.

## 2. Три способа сборки

Из корня рабочей копии выполните:

```bash
/usr/bin/time -f 'wall_seconds=%e' -o "$EVIDENCE/02-single-time.txt" \
  docker build --progress=plain -t quant-cli:single -f docker/Dockerfile.single . \
  2>&1 | tee "$EVIDENCE/02-single-build.txt"
/usr/bin/time -f 'wall_seconds=%e' -o "$EVIDENCE/02-multi-time.txt" \
  docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/02-multi-build.txt"
/usr/bin/time -f 'wall_seconds=%e' -o "$EVIDENCE/02-distroless-time.txt" \
  docker build --progress=plain -t quant-cli:distroless -f docker/Dockerfile.distroless . \
  2>&1 | tee "$EVIDENCE/02-distroless-build.txt"
docker images quant-cli --format '{{.Tag}}\t{{.ID}}\t{{.Size}}'
for tag in single multi distroless; do
  docker image inspect "quant-cli:$tag" --format '{{.Size}}' \
    > "$EVIDENCE/02-$tag-size-bytes.txt"
  docker run --rm "quant-cli:$tag" "${VAR_ARGS[@]}" \
    | tee "$EVIDENCE/02-$tag-output.txt"
done
diff -u "$EVIDENCE/02-single-output.txt" "$EVIDENCE/02-multi-output.txt"
diff -u "$EVIDENCE/02-multi-output.txt" "$EVIDENCE/02-distroless-output.txt"
cat "$EVIDENCE"/02-*-time.txt
```

Если `/usr/bin/time` отсутствует: `sudo apt-get update && sudo apt-get install -y time`. Два `diff` не должны вывести различий. Снимок №2: размеры и результат расчёта; времена перенесите в таблицу отчёта. Первые сборки могут частично разделять кэш — укажите это. Числа 681/140/9 МБ из методички измерены на другой машине и не являются требуемым результатом.

Контроль для GOLF, 1000 котировок: `first=102.46`, `last=102.58`, `sma(12)=102.6083`, `ema(16)=102.5932`, `rsi(21)=48.5900`, `volatility=0.000424`, `max_dd=0.015490` (занятие 4, таблицы Г.3–Г.4). Начальная цена 102.50 руб. — состояние **до** первого шага генератора, поэтому первая котировка 102.46.

Ответ: single содержит компилятор, CMake, исходники и сборочные файлы; multi переносит в финальный Ubuntu только исполняемый файл; distroless использует статическую программу и минимальную среду без shell. Одинаковый результат расчёта не означает одинаковое содержимое файловой системы.

## 3. Слои образа

```bash
docker history --no-trunc quant-cli:single | tee "$EVIDENCE/03-single-history.txt"
docker history --no-trunc quant-cli:multi | tee "$EVIDENCE/03-multi-history.txt"
docker image inspect quant-cli:multi --format '{{json .RootFS.Layers}}'
```

Снимок №3: обе истории. В каждой найдите самый большой **файловый** слой по столбцу SIZE, запишите размер и CREATED BY; не подставляйте предполагаемый слой вместо измерения. `USER`, `ENTRYPOINT`, `WORKDIR` могут не добавлять файлового слоя или иметь нулевой размер. В multi не попадает слой установки компилятора из этапа build. Размер итогового образа включает базовую ОС и библиотеки, поэтому значительно больше бинарного файла. Общие слои нескольких образов физически могут храниться один раз.

## 4. Кэш сборки

Не меняйте `.dockerignore`, Dockerfile, журналы внутри контекста и другие файлы между измерениями, кроме явно указанного исходника. Результаты складывайте в `$EVIDENCE`.

### 4.1. Без изменений

```bash
/usr/bin/time -f 'wall_seconds=%e' -o "$EVIDENCE/04-cached-time.txt" \
  docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/04-cached-build.txt"
```

Шаги, которые можно кэшировать, должны иметь `CACHED`; загрузка метаданных и экспорт могут занимать время даже при полном кэше.

### 4.2. Изменение исходника при правильном порядке

В `apps/quant-cli/main.cpp` после строки `int main(int argc, char** argv) {` добавьте:

```cpp
  std::printf("variant     07\n");
```

```bash
/usr/bin/time -f 'wall_seconds=%e' -o "$EVIDENCE/04-source-time.txt" \
  docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/04-source-build.txt"
docker run --rm quant-cli:multi "${VAR_ARGS[@]}"
```

Слой установки пакетов должен переиспользоваться; `COPY . .`, CMake/компиляция/тесты и перенос изменённого бинарника выполняются заново.

### 4.3. Неудачный порядок и корректное сравнение

Сохраните исходный Dockerfile **снаружи** контекста:

```bash
cp docker/Dockerfile.multi "$EVIDENCE/Dockerfile.multi.good"
```

В **первом этапе build** перенесите `WORKDIR /src` и `COPY . .` перед `RUN apt-get ...`. Получится начало:

```dockerfile
FROM ubuntu:24.04 AS build
WORKDIR /src
COPY . .
RUN apt-get update && apt-get install -y --no-install-recommends build-essential cmake \
    && rm -rf /var/lib/apt/lists/*
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j \
    && ctest --test-dir build --output-on-failure
```

Этап runtime оставьте как в исходнике. Сначала создайте кэш для этого порядка:

```bash
docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/04-bad-warmup.txt"
```

Теперь измените **ту же строку** в main.cpp на `std::printf("variant     07 cache-check\n");` и измерьте повторную сборку:

```bash
/usr/bin/time -f 'wall_seconds=%e' -o "$EVIDENCE/04-bad-source-time.txt" \
  docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/04-bad-source-build.txt"
```

Теперь изменение исходника должно сбросить и слой установки пакетов. Отделяйте это измерение от первой сборки после перестановки: иначе смешаются две причины потери кэша. Восстановите порядок и простую строку `variant 07`:

```bash
cp "$EVIDENCE/Dockerfile.multi.good" docker/Dockerfile.multi
# В редакторе верните строку main.cpp к std::printf("variant     07\n");
docker build -t quant-cli:multi -f docker/Dockerfile.multi .
cat "$EVIDENCE"/04-*-time.txt
```

Снимок №4: CACHED/пересобранные шаги. Заполните таблицу трёх измерений. Главный вывод — зависимость инвалидации слоёв от порядка; абсолютное время также зависит от сети и нагрузки хоста.

## 5. Ресурсы, пользователь и сохранение данных

### 5.1. Пределы cgroup и пользователь

```bash
docker run --rm --cpus 1 --memory 512m --entrypoint sh quant-cli:multi \
  -c 'cat /sys/fs/cgroup/cpu.max /sys/fs/cgroup/memory.max; id' \
  | tee "$EVIDENCE/05-limits.txt"
```

Для cgroup v2 ожидается отношение квоты к периоду `1`, обычно `100000 100000`; память `536870912` байт = 512 МиБ. `id` должен показать UID 10001, не root. Если файлов нет, проверьте `docker info` → Cgroup Version. В cgroup v1 аналоги: `cpu.cfs_quota_us`, `cpu.cfs_period_us` в `/sys/fs/cgroup/cpu/` и `memory.limit_in_bytes` в `/sys/fs/cgroup/memory/`.

Снимок №5: значения и `id`. Ограничение CPU задаёт квоту времени, а не закрепление конкретного ядра. Превышение CPU-квоты замедляет расчёт; нехватка памяти может вызвать OOM. Код 137 означает SIGKILL, причину OOM подтверждают `OOMKilled` и события памяти.

### 5.2. Именованный том

```bash
docker volume create var_07
docker run --rm --cpus 1 --memory 512m -v var_07:/data \
  quant-cli:multi "${VAR_ARGS[@]}" --csv /data/ticks.csv
docker run --rm -v var_07:/data --entrypoint sh quant-cli:multi \
  -c 'head -n 6 /data/ticks.csv; wc -l /data/ticks.csv; id; ls -ld /data'
```

Снимок №6: CSV из **нового** контейнера, 1001 строка с заголовком. В этом опорном CLI столбец `price` хранит рубли: первые пять цен 102.46, 102.41, 102.38, 102.37, 102.34; объёмы 991, 370, 830, 592, 231. Это отличается от файлов `price_cents` в приложении к методичке.

В исходном Dockerfile `/data` принадлежит quant. Новый пустой том наследует права каталога. Если используете уже существовавший том с другими правами, проверьте владельца; не удаляйте том с нужными данными. Для пустого учебного тома можно однократно назначить владельца через вспомогательный контейнер с root, затем продолжить запуск приложения как quant.

### 5.3. Bind mount и воспроизводимая проверка прав

```bash
mkdir -p out
docker run --rm --cpus 1 --memory 512m -v "$PWD/out:/data" \
  quant-cli:multi "${VAR_ARGS[@]}" --csv /data/ticks.csv
```

В Linux-каталоге WSL, созданном вашим пользователем, UID 10001 обычно не имеет права записи. Сохраните ошибку и проверьте владельцев:

```bash
ls -ldn out
docker run --rm --entrypoint id quant-cli:multi
```

Запустите приложение с UID/GID владельца каталога:

```bash
docker run --rm --user "$(id -u):$(id -g)" --cpus 1 --memory 512m \
  -v "$PWD/out:/data" quant-cli:multi "${VAR_ARGS[@]}" --csv /data/ticks.csv
head -n 6 out/ticks.csv
wc -l out/ticks.csv
```

Снимок №7: ошибка, если возникла, выбранное исправление и файл. Если запись сразу удалась, так и укажите — не выдумывайте ошибку. `--user` сохраняет выполнение без root. На `/mnt/d` поведение прав отличается, поэтому выше рекомендована файловая система WSL.

### 5.4. Измерение потребления для задания 9

Лимит 512 МиБ не равен потреблению. Расчёт 1000 точек слишком короткий для надёжного `docker stats`; используйте `memory.peak` в cgroup v2. Обёртка остаётся активной после CLI и читает пик до удаления cgroup. `--memory-swap 512m` запрещает дополнительный swap для этого измерения.

```bash
for n in 1 2 3 4 5; do
  docker run --name "lab3-measure-$n" --cpus 1 --memory 512m --memory-swap 512m \
    --entrypoint sh quant-cli:multi -c '
      quant-cli "$@"
      rc=$?
      echo "cli_exit=$rc"
      echo memory_peak_bytes
      cat /sys/fs/cgroup/memory.peak
      echo memory_events
      cat /sys/fs/cgroup/memory.events
      exit "$rc"
    ' sh "${VAR_ARGS[@]}" | tee "$EVIDENCE/05-measure-$n.txt"
  docker inspect "lab3-measure-$n" \
    --format 'OOMKilled={{.State.OOMKilled}} ExitCode={{.State.ExitCode}}' \
    | tee -a "$EVIDENCE/05-measure-$n.txt"
  docker rm -v "lab3-measure-$n"
done
```

Пик включает shell и файловый кэш: это консервативная оценка памяти контейнера, а не только heap C++. Возьмите максимум пяти успешных прогонов, разделите на 1048576 для МиБ. Если `memory.peak` отсутствует, используйте `/usr/bin/time -v` внутри отдельного измерительного образа (установить пакет time в runtime), укажите, что Maximum resident set size — RSS процесса и не полный пик cgroup. Нельзя заменять отсутствующий замер числом из примера.

Снимок №8: пять пиков, коды завершения и OOMKilled. Проверка маленькой нагрузки не гарантирует достаточность памяти на произвольном размере данных.

## 6. Python-сервис и схема

```bash
docker build --progress=plain -t risk-model:1.0 services/risk-model \
  2>&1 | tee "$EVIDENCE/06-build.txt"
docker run -d --name risk -p 127.0.0.1:8000:8000 risk-model:1.0
curl --fail --show-error --retry 15 --retry-connrefused --retry-delay 1 \
  http://localhost:8000/health | tee "$EVIDENCE/06-health.json"
curl --fail --show-error http://localhost:8000/model-info \
  | tee "$EVIDENCE/06-model-info.json"
docker logs risk
docker inspect risk --format '{{json .State.Health}}'
```

Дождитесь первого успешного healthcheck (интервал 10 с) и повторите inspect. Затем:

```bash
docker stop risk
docker inspect risk --format '{{.State.Status}}'
docker rm risk
```

Снимок №9: ответы обоих HTTP-маршрутов, состояние проверки и остановка. Если 8000 занят, используйте `-p 127.0.0.1:8007:8000` и порт 8007 в curl, указав замену в отчёте. Образ Python не компилирует C++, а устанавливает Python-зависимости. Отдельный ранний `COPY requirements.txt .` позволяет сохранять кэш pip при правке app.py/model.py.

### Схема задания 6

Откройте https://app.diagrams.net/?libs=aws4, включите иконки AWS. Сохраните `lab3-task6-var07.drawio` и экспортируйте PNG. Это **самостоятельная схема задания 6**, без этапа сквозного проекта.

1. Вне облака нарисуйте «Рабочее место: Windows 11 → WSL2 → Docker Desktop».
2. Внутри границы AWS Region `sa-east-1` разместите Amazon ECR и CloudWatch Logs.
3. В этом регионе нарисуйте VPC, частную подсеть и задачу Fargate с контейнером; логическую границу ECS Cluster подпишите отдельно. ECR и CloudWatch не размещайте внутри подсети.
4. Стрелка рабочего места **к ECR** — `push image:1.0.0`; стрелка **от ECR к задаче** — `pull image@sha256:...`; стрелка от задачи к CloudWatch — `stdout/stderr → awslogs`.
5. Подпишите сетевой доступ из частной подсети: через NAT Gateway либо VPC endpoints ECR API + ECR DKR, S3 gateway endpoint для слоёв и endpoint CloudWatch Logs. Endpoint ECR сам по себе не заменяет доступ к слоям в S3.
6. Укажите execution role для скачивания образа/журналов и task role для AWS-запросов самого приложения.

Снимок №10 — PNG схемы; сам `.drawio` тоже приложите. Реальное развёртывание ECR/ECS/Fargate не требуется.

## 7. Собственный индикатор варианта 7

### 7.1. Формула и ручной пример

Для N цен нужно K=N−1 лог-доходностей `r[i]=ln(p[i]/p[i−1])`. Среднее `m=sum(r)/K`, **выборочное** СКО `s=sqrt(sum((r−m)^2)/(K−1))`. Ответ `m/s` по **всем** доходностям. Минимум — три цены. Никакой годовой нормализации, безрисковой ставки и процентов в формуле таблицы 16.2 нет.

Ручной тест: цены `{1, 2, 8}` → доходности `{ln 2, ln 4}={a,2a}`. Тогда `m=3a/2`, `s=a/sqrt(2)`, `m/s=3/sqrt(2)=2.121320343559642...`. Ожидаемое число получено аналитически, а не вызовом проверяемой функции.

### 7.2. Объявление и реализация

Перед закрывающей скобкой namespace quant в `core/include/quant/indicators.hpp` добавьте:

```cpp
// Отношение среднего лог-доходностей к их выборочному СКО.
double return_to_risk(const std::vector<double>& p);
```

В `core/src/indicators.cpp`, внутри namespace quant, добавьте:

```cpp
double return_to_risk(const std::vector<double>& p) {
  if (p.size() < 3)
    throw std::invalid_argument("return_to_risk: need at least 3 prices");
  for (double price : p) {
    if (!std::isfinite(price) || price <= 0.0)
      throw std::invalid_argument("return_to_risk: prices must be positive and finite");
  }
  const auto r = log_returns(p);
  const double m = mean(r);
  const double s = stddev(r);  // по умолчанию sample=true, делитель K-1
  if (!std::isfinite(s) || s <= 0.0)
    throw std::domain_error("return_to_risk: zero or invalid standard deviation");
  return m / s;
}
```

Здесь используются уже подключённые `<cmath>`, `<stdexcept>` и `quant/stats.hpp`. При постоянных доходностях знаменатель нулевой: выбран явный отказ через domain_error, а не произвольное число. Этот случай методичка отдельно не оговаривает, решение нужно объяснить.

### 7.3. Тесты

В `tests/test_core.cpp` перед `if (failures == 0)` вставьте блок. Проверка `isfinite` нужна дополнительно: существующий макрос CHECK_NEAR сам по себе может пропустить NaN.

```cpp
  const double ratio = quant::return_to_risk({1.0, 2.0, 8.0});
  CHECK(std::isfinite(ratio));
  CHECK_NEAR(ratio, 2.1213203435596424, 1e-12);

  bool short_thrown = false;
  try { quant::return_to_risk({1.0, 2.0}); }
  catch (const std::invalid_argument&) { short_thrown = true; }
  CHECK(short_thrown);

  bool zero_sd_thrown = false;
  try { quant::return_to_risk({1.0, 1.0, 1.0}); }
  catch (const std::domain_error&) { zero_sd_thrown = true; }
  CHECK(zero_sd_thrown);

  bool invalid_price_thrown = false;
  try { quant::return_to_risk({1.0, 0.0, 2.0}); }
  catch (const std::invalid_argument&) { invalid_price_thrown = true; }
  CHECK(invalid_price_thrown);
```

### 7.4. CLI и Dockerfile

В `apps/quant-cli/main.cpp` непосредственно перед `return 0;` добавьте:

```cpp
  std::printf("indicator %.6f\n", quant::return_to_risk(prices));
```

В **первом этапе** `docker/Dockerfile.multi` сделайте включение тестов явным; замените существующий RUN сборки на:

```dockerfile
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DQUANT_BUILD_TESTS=ON \
    && cmake --build build -j \
    && ctest --test-dir build --output-on-failure --no-tests=error
```

Остальной Dockerfile оставьте прежним. Это изменённый Dockerfile для приложения к отчёту. Тесты уже присутствовали в опорном файле; добавленные параметры явно требуют их создания и запрещают успех при пустом наборе тестов. **HEALTHCHECK для C++ CLI в заданиях 1–9 не требуется**; healthcheck Python-сервиса уже есть. Дополнительные требования §16.8 здесь не выполняются.

```bash
docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/07-good-build.txt"
docker run --rm --cpus 1 --memory 512m quant-cli:multi "${VAR_ARGS[@]}" \
  | tee "$EVIDENCE/07-indicator.txt"
```

Снимок №11: успешный ctest на этапе build и `indicator` с шестью знаками. Запишите фактическое значение, не подставляйте 2.121320: это значение **короткого теста**, не 1000 котировок GOLF.

### 7.5. Показать, что плохой тест останавливает сборку

```bash
cp tests/test_core.cpp "$EVIDENCE/test_core.good.cpp"
```

В одном новом CHECK_NEAR временно замените ожидаемое `2.1213203435596424` на `0.0`. Затем:

```bash
docker build --progress=plain -t quant-cli:must-fail -f docker/Dockerfile.multi . \
  > "$EVIDENCE/07-failed-build.txt" 2>&1
rc=$?
cat "$EVIDENCE/07-failed-build.txt"
echo "build_exit=$rc"
```

Ожидается ненулевой код и FAIL в CTest. Старые существующие образы Docker при неудачной сборке автоматически не удаляются. Снимок №12 — отказ сборки. Сразу восстановите тест и пересоберите:

```bash
cp "$EVIDENCE/test_core.good.cpp" tests/test_core.cpp
docker build --progress=plain -t quant-cli:multi -f docker/Dockerfile.multi .
docker run --rm quant-cli:multi "${VAR_ARGS[@]}"
```

В отчёт вставьте исходники объявления, функции, тестов, строки CLI и **полный итоговый Dockerfile.multi**. Можно вставлять текстом в приложение, сохраняя форматирование кода.

## 8. Теги, настоящий дайджест и экспорт

### 8.1. Два тега одного образа

```bash
docker tag quant-cli:multi quantflow/quant-cli:1.0.0
docker tag quant-cli:multi quantflow/quant-cli:latest
docker images quantflow/quant-cli --no-trunc
docker system df
docker image inspect quantflow/quant-cli:1.0.0 --format '{{.Id}}'
docker image inspect quantflow/quant-cli:latest --format '{{.Id}}'
```

Снимок №13: оба тега и одинаковый IMAGE ID. Теги — ссылки; они не копируют слои. `.Id` — хеш конфигурации образа, **не** дайджест реестрового манифеста. У только что собранного локального образа RepoDigests может быть пуст.

### 8.2. Локальный реестр без AWS-аккаунта

```bash
docker run -d --name lab3-registry -p 127.0.0.1:5000:5000 registry:2
docker tag quantflow/quant-cli:1.0.0 localhost:5000/quantflow/quant-cli:1.0.0
docker tag quantflow/quant-cli:latest localhost:5000/quantflow/quant-cli:latest
docker push localhost:5000/quantflow/quant-cli:1.0.0 \
  | tee "$EVIDENCE/08-push-version.txt"
docker push localhost:5000/quantflow/quant-cli:latest \
  | tee "$EVIDENCE/08-push-before.txt"
docker buildx imagetools inspect localhost:5000/quantflow/quant-cli:latest \
  | tee "$EVIDENCE/08-manifest-before.txt"
```

Запишите `digest: sha256:...` из успешного push. Если `imagetools` не поддерживает локальный HTTP-реестр в вашей конфигурации, для задания достаточно digest из push; дополнительно получите заголовок манифеста через curl:

```bash
curl --fail -sSI \
  -H 'Accept: application/vnd.oci.image.index.v1+json, application/vnd.docker.distribution.manifest.list.v2+json, application/vnd.oci.image.manifest.v1+json, application/vnd.docker.distribution.manifest.v2+json' \
  http://localhost:5000/v2/quantflow/quant-cli/manifests/latest \
  | tee "$EVIDENCE/08-headers-before.txt"
```

`Docker-Content-Digest` — дайджест объекта, на который указывает тег. При WSL localhost forwarding обычно доступен; если curl не соединяется, используйте HTTP-проверку из контейнера с сетью реестра или выполните curl.exe из Windows. Если порт 5000 занят, замените его в **обоих** местах привязки/адресов, например `127.0.0.1:5007:5000` и `localhost:5007/...`.

### 8.3. Изменение latest

В main.cpp измените строку печати варианта на `std::printf("variant     07 revision-2\n");`. Это реальное изменение бинарного файла, а не только комментария.

```bash
docker build --progress=plain -t quantflow/quant-cli:latest -f docker/Dockerfile.multi . \
  2>&1 | tee "$EVIDENCE/08-rebuild.txt"
docker tag quantflow/quant-cli:latest localhost:5000/quantflow/quant-cli:latest
docker push localhost:5000/quantflow/quant-cli:latest \
  | tee "$EVIDENCE/08-push-after.txt"
docker image inspect quantflow/quant-cli:1.0.0 --format '{{.Id}}'
docker image inspect quantflow/quant-cli:latest --format '{{.Id}}'
docker run --rm quantflow/quant-cli:latest "${VAR_ARGS[@]}"
curl --fail -sSI \
  -H 'Accept: application/vnd.oci.image.index.v1+json, application/vnd.docker.distribution.manifest.list.v2+json, application/vnd.oci.image.manifest.v1+json, application/vnd.docker.distribution.manifest.v2+json' \
  http://localhost:5000/v2/quantflow/quant-cli/manifests/latest \
  | tee "$EVIDENCE/08-headers-after.txt"
```

Снимок №14: дайджесты до/после и новый вывод CLI. Не перетегируйте `1.0.0`: он должен остаться на старой версии. Дайджест — SHA-256 байтов манифеста (либо индекса для нескольких манифестов); он связывает конфигурацию и слои через их собственные дайджесты. Это не хеш `docker save`-архива.

### 8.4. Экспорт и политика хранения

```bash
docker save -o "$EVIDENCE/quant-cli-latest.tar" quantflow/quant-cli:latest
stat -c '%n: %s bytes' "$EVIDENCE/quant-cli-latest.tar"
docker image inspect quantflow/quant-cli:latest --format 'image_size_bytes={{.Size}}'
docker images quantflow/quant-cli
docker stop lab3-registry
```

Снимок №15: точные размеры в байтах. TAR содержит слои, конфигурацию, манифесты и служебные заголовки. Он обычно хранит несжатые слои, а в реестре они могут быть сжаты. Поле SIZE зависит от представления Docker; в современных хранилищах может учитывать иные составляющие. Поэтому сравнивайте явно названные метрики и не обещайте, что TAR всегда меньше/больше.

Для развёртывания выбирают digest либо защищённый от перезаписи версионный тег. В локальном registry:2 версия `1.0.0` технически тоже перезаписываема; неизменяемость — настройка реестра/процесса выпуска. Политика: не удалять версии, используемые текущими развёртываниями и согласованными точками отката; удалять непривязанные к тегам образы старше 7 дней и старые неиспользуемые dev-версии после 30 дней с сохранением последних 10. Перед очисткой сверять используемые digest: простое «оставить последние 10» не защищает старую работающую версию.

Реестр остановлен, его контейнер сохранён; при необходимости `docker start lab3-registry`. Не выполняйте общий `docker system prune`: он может затронуть материалы других работ.

## 9. Размер задачи и способ запуска

### 9.1. Память

Из задания 5 возьмите `P=max(пять memory.peak)/1048576` МиБ. Требование с запасом: `M_req=1.30×P`, фактический запас `(M_task/P−1)×100%`. Не принимайте лимит 512 МиБ за измеренный пик.

### 9.2. Процессор и минимальная допустимая пара

В методичке есть неточная ссылка на §8: таблица допустимых пар находится также в §11.1, стр. 23. Для Linux:

| vCPU | Память |
|---|---|
| 0.25 | 512 МиБ, 1 ГиБ, 2 ГиБ |
| 0.5 | 1–4 ГиБ, шаг 1 ГиБ |
| 1 | 2–8 ГиБ, шаг 1 ГиБ |
| 2 | 4–16 ГиБ, шаг 1 ГиБ |
| 4 | 8–30 ГиБ, шаг 1 ГиБ |
| 8 | 16–60 ГиБ, шаг 4 ГиБ |
| 16 | 32–120 ГиБ, шаг 8 ГиБ |

Предел `--cpus 1` не доказывает потребность ровно в 1 vCPU. Для пакетного CLI без заданного срока завершения допустимо проверить минимальные 0.25 vCPU; для сохранения выделенного в задании 5 полного CPU нужна пара не меньше **1 vCPU / 2 ГиБ**, а не недопустимые 1 vCPU / 512 МиБ.

Проверьте минимальную пару, если `M_req≤512`:

```bash
for cpu in 1 0.25; do
  for n in 1 2 3 4 5; do
    /usr/bin/time -f "cpu=$cpu run=$n wall_seconds=%e" \
      -o "$EVIDENCE/09-cpu-$cpu-$n-time.txt" \
      docker run --name "lab3-sizing-$cpu-$n" --cpus "$cpu" --memory 512m \
        quant-cli:multi "${VAR_ARGS[@]}" \
      > "$EVIDENCE/09-cpu-$cpu-$n-output.txt"
    docker inspect "lab3-sizing-$cpu-$n" \
      --format 'OOMKilled={{.State.OOMKilled}} ExitCode={{.State.ExitCode}}'
    docker rm -v "lab3-sizing-$cpu-$n"
  done
done
cat "$EVIDENCE"/09-*-time.txt
```

Эти времена включают запуск Docker и не являются чистым временем CPU. Для 1000 точек накладные расходы старта могут доминировать. Если нет требования к задержке и расчёт устойчиво завершается, выбирайте минимальную пару по памяти; не делайте из отсутствующего SLA вывода о нужной производительности веб-сервиса. Если память превышает 512/1.30≈393.85 МиБ, округлите вверх по таблице. Для явно заданного срока нужно дополнительно проверить, что выбранная квота его обеспечивает. Снимок №16 — расчёт по фактическому P и результаты проверок.

### 9.3. Три реплики

Для выбранной пары `(C,M)` нужны **три задачи**, суммарно `3C vCPU` и `3M` памяти. При `(0.25,0.5 ГиБ)` это `0.75 vCPU / 1.5 ГиБ`; при `(1,2 ГиБ)` — `3 vCPU / 6 ГиБ`. Это условные примеры, выберите свой результат.

Один EC2 должен вместить сумму **зарезервированных** ресурсов задач плюс ОС/агент и эксплуатационный запас. Вариант прошлого занятия t3.micro (2 vCPU / 1 ГиБ) не вмещает даже 3×512 МиБ. Для первого примера по номинальной ёмкости достаточно узла 2 vCPU / 4 ГиБ; для второго — 4 vCPU / 8 ГиБ с проверкой фактически доступной ECS памяти. Конкретный тип не обязателен: сравнение требуется по ресурсам. Один узел — единая точка отказа; три задачи на нём не дают доступности трёх независимых узлов.

### 9.4. Неравенство и роли

Пусть `d` — доля времени работы задач за период T; `F` — суммарная ставка всех трёх задач за час активности, `E` — ставка постоянно включённого EC2, способного разместить их все. Упрощённая модель: `Cost_F=T×d×F`, `Cost_E=T×E`. Fargate выгоднее при `d<E/F`, EC2 — при `d>E/F`. При равенстве расходы равны. Если EC2 тоже выключать вместе с задачами, формула меняется; без ставок численный порог получить нельзя. Сеть, диски, журналы и труд администратора здесь не учтены.

Execution role: `ecr:GetAuthorizationToken`, `ecr:BatchCheckLayerAvailability`, `ecr:GetDownloadUrlForLayer`, `ecr:BatchGetImage`; для существующей группы CloudWatch Logs — `logs:CreateLogStream`, `logs:PutLogEvents`. Авторизация ECR требует Resource `*`, операции чтения образа ограничиваются нужным репозиторием. Группу журналов лучше создать заранее; иначе отдельно требуется `logs:CreateLogGroup`. Чтение Secrets Manager/SSM и KMS добавляется только при фактическом использовании секретов.

Task role: разрешения приложения на AWS API. Для текущего локального CLI с генератором и CSV AWS API не вызываются — дополнительные права не требуются. Если позже приложение само читает S3, ему отдельно понадобятся права на соответствующие объекты, а не права скачивания образа.

## П1–П3. Необязательно, по желанию

Это дополнительные баллы; для обязательных заданий 1–9 их выполнять не требуется. В шаблоне есть отдельное место для результатов, которое можно удалить, если их не выполняли.

- **П1, кэш:** в отдельной копии проекта зафиксируйте исходную сборку; по очереди меняйте исходник, файл зависимостей (для Python — requirements.txt), добавляйте неигнорируемый файл, используемый ARG, базовый образ. Перед каждым опытом возвращайте остальные изменения. Каждое действие повторите минимум три раза, отделяя прогрев от измерения. Сохраните время и CACHED-статусы. Оптимизируйте порядок COPY/RUN и повторите. Объясните, какой файл и какие инструкции реально зависят друг от друга; неиспользуемый ARG может вообще не влиять на кэш.
- **П2, память:** повторите расчёт своего варианта с 64, 96, 128, 256 МиБ (например, по циклу из 5.4 с заменой `--memory` и равным `--memory-swap`). Для каждого предела сделайте пять прогонов, сохраните ExitCode, OOMKilled, memory.peak и memory.events. При OOM обёртка может не успеть вывести пик — так и обозначьте; состояние контейнера inspect доступно до rm. Если все четыре предела успешны, минимальный **из проверенных** — 64 МиБ, а не доказанный глобальный минимум. Для промышленной настройки нужен запас и проверка максимальной рабочей нагрузки.
- **П3, уменьшение:** используйте отдельные Dockerfile для трёх способов и явно разделите факторы: multi на Ubuntu с динамической компоновкой, минимальная совместимая C++ runtime-база с динамической компоновкой, статический бинарник на distroless/static. Проверьте тесты и совпадение расчёта, измерьте размер и время (минимум три прогона, укажите режим кэша). Опишите наличие shell/диагностики, зависимости libc/libstdc++, архитектуру и обновления. Исходный Dockerfile.distroless объединяет минимальную базу и статическую компоновку, поэтому одно сравнение single/multi/distroless не отделяет их эффекты.

## Перед сдачей

1. Заполните все пометки в DOCX реальными результатами, удалите необязательные разделы, если не выполняли.
2. Проверьте, что показаны все 9 заданий, оба HTTP-маршрута, CSV после удаления контейнера, ошибка теста и два разных дайджеста latest.
3. Добавьте исходники задания 7 и полный Dockerfile.multi, таблицы размеров/времён/ресурсов, минимум три вывода.
4. Приложите `.drawio` и PNG задания 6. Сохраните заполненный отчёт в PDF.
5. Скопируйте материалы из `$EVIDENCE` в Windows при необходимости, например в отдельный каталог `lab3/results`. TAR может быть большим; приложите его только по требованию преподавателя. Для открытия папки WSL в Проводнике: `explorer.exe .`.

## Источники для пояснений

- Условия заданий: `lab3/Занятие 5.pdf`, §16.1–16.7; параметры и контрольные значения: `lab2/Занятие 4.pdf`, приложение Г.
- [Docker: ограничения ресурсов](https://docs.docker.com/engine/containers/resource_constraints/) и [метрики cgroup](https://docs.docker.com/engine/containers/runmetrics/).
- [Docker Registry: манифест, конфигурация и дайджесты слоёв](https://docs.docker.com/reference/api/registry/latest/operations/GetImageManifest/).
- [AWS: допустимые размеры задач Fargate](https://docs.aws.amazon.com/AmazonECS/latest/developerguide/fargate-tasks-services.html).
- [AWS: роли ECS](https://docs.aws.amazon.com/AmazonECS/latest/developerguide/ecs-iam-role-overview.html).
