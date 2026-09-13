
## Суть задачи

Есть множество потоков двух типов: часть вызывает `hydrogen()`, часть — `oxygen()`. Нужно собрать их в молекулы воды: на каждую молекулу нужно ровно **2 атома H и 1 атом O**, при этом атомы должны выводиться группами по три, соответствующими одной молекуле (то есть нельзя, чтобы третий H начал печататься, пока не завершилась текущая тройка — иначе теоретически можно было бы "смешать" атомы из разных молекул в непредсказуемом порядке, хотя формально любой порядок H/O внутри правильной тройки допустим).

Это качественно другая задача по сравнению с LC1114/1115: там был **фиксированный, известный порядок** (first→second→third, foo→bar). Здесь порядка внутри тройки нет (H и O могут чередоваться как угодно — `HOH`, `HHO`, `OHH` — все валидны), но есть **количественное ограничение**: ровно 2 H и 1 O должны "собраться" вместе, прежде чем начнётся следующая тройка.

## Идея

Нужны два инструмента одновременно:

1. **Ограничение количества** — H-потоки не должны обгонять O-потоки больше чем на 2 "в моменте" (иначе три H успеют напечататься раньше, чем подоспеет O). Для этого — **счётный семафор** (counting semaphore), а не бинарный: разрешаем максимум 2 одновременных "прошедших" H на 1 O.
2. **Барьер** — способ гарантировать, что все трое участников текущей тройки завершили печать, прежде чем следующая тройка начнёт формироваться (иначе H из следующей молекулы может проскочить раньше O из текущей — тоже нарушение "группировки по тройкам").

**Почему `hydrogenSem` считает до 2, а не просто mutex:** ровно два H-потока должны иметь возможность **одновременно** пройти дальше (напечатать) в рамках одной молекулы — если бы это был обычный mutex (эксклюзивный доступ, 1 владелец), второй H-поток заблокировался бы, ожидая, пока первый полностью не завершит **весь** цикл (включая `sync_point.arrive_and_wait()`), а это создало бы дедлок: первый H ждёт барьер (нужны все трое), но второй H не может даже дойти до барьера, потому что заблокирован на mutex первого H.

**Почему нужен именно `barrier`, а не просто продолжить дальше после семафоров:** без барьера ничего не мешает H-потоку из **следующей** молекулы захватить освободившееся место в `hydrogenSem` (после `release()`) и начать печатать раньше, чем O из **текущей** молекулы завершил работу — тройки перемешаются. Барьер гарантирует: "никто не идёт дальше (не освобождает семафор), пока все трое участников этой конкретной тройки не допечатали".

Если интервьюер просит C++17-совместимое решение (`std::barrier`/`std::semaphore` появились только в C++20) — нужна замена на `condition_variable` + счётчики вручную:

Здесь есть тонкий баг-ловушка, которую стоит явно проговорить на собеседовании: **все ожидающие на втором `cv.wait` могут одновременно увидеть `hCount==2 && oCount==1` и одновременно попытаться сбросить счётчики** — это не страшно (сброс идемпотентен, они все установят одни и те же нули), но важно понимать, что здесь нет "единственного лидера", который гарантированно сбрасывает состояние, — это осознанное упрощение, а не ошибка, но проговорить это стоит, чтобы показать, что вы видите потенциальную гонку и почему в данном случае она безвредна.

## Связь с вакансией

Это прямая модель **join/barrier-синхронизации в конвейере обработки данных** — если рантайм устроен как граф стадий (что explicit в вашей вакансии), очень вероятен паттерн "стадия N+1 не может начать работу с батчем, пока **все** параллельные подзадачи стадии N не завершились" — это ровно barrier-семантика. Также H2O — хороший пример **rate limiting через counting semaphore** (не более K одновременных операций одного типа) — паттерн, применимый к ограничению конкурентных запросов к шарду/бэкенду.

```cpp
#include <iostream>
#include <thread>
#include <semaphore>
#include <barrier>
#include <functional>
#include <mutex>

namespace {
    class H20CV {
        static constexpr int O_QUANTITY {1};
        static constexpr int H_QUANTITY {2};

        std::mutex mtx;
        std::condition_variable cv;
        int h_count{0};
        int o_count{0};
        int departed_count{0};

    public:
        void hydrogen(const std::function<void()>& release_hydrogen) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&] { return h_count < H_QUANTITY; });
            h_count++;
            release_hydrogen();
            cv.notify_all();

            wait_for_triple_and_reset(lk);
        }

        void oxygen(const std::function<void()>& release_oxygen) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&] { return o_count < O_QUANTITY; });
            o_count++;
            release_oxygen();
            cv.notify_all();

            wait_for_triple_and_reset(lk);
        }

    private:
        [[nodiscard]] bool is_triple_completed() const {
            return o_count == O_QUANTITY && h_count == H_QUANTITY;
        }

        void wait_for_triple_and_reset(std::unique_lock<std::mutex>& lk) {
            cv.wait(lk, [&] { return is_triple_completed(); });

            departed_count++;
            if (departed_count == H_QUANTITY + O_QUANTITY) {
                h_count = o_count = 0;
                departed_count = 0;
            }
            cv.notify_all();
        }
    };

    class H20SEM {
        std::counting_semaphore<2> hydrogen_sem{2};
        std::binary_semaphore oxygen_sem{1};
        std::barrier<> sync_point{3};

    public:
        void hydrogen(const std::function<void()>& release_hydrogen) {
            hydrogen_sem.acquire();
            release_hydrogen();
            sync_point.arrive_and_wait();
            hydrogen_sem.release();
        }

        void oxygen(const std::function<void()>& release_oxygen) {
            oxygen_sem.acquire();
            release_oxygen();
            sync_point.arrive_and_wait();
            oxygen_sem.release();
        }
    };

    void start_test0() {
        const auto release_hydrogen = [] { std::cout << "H"; };
        const auto release_oxygen = [] { std::cout << "O"; };

        std::vector<std::thread> threads;

        H20CV h2o;
        for (int i{}; i < 3; i++) {
            threads.emplace_back(&H20CV::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20CV::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20CV::oxygen, &h2o, release_oxygen);
        }

        for (auto& t: threads) {
            t.join();
        }

        std::cout << std::endl;
    }

    void start_test1() {
        const auto release_hydrogen = [] { std::cout << "H"; };
        const auto release_oxygen = [] { std::cout << "O"; };

        std::vector<std::thread> threads;

        H20SEM h2o;
        for (int i{}; i < 3; i++) {
            threads.emplace_back(&H20SEM::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20SEM::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20SEM::oxygen, &h2o, release_oxygen);
        }

        for (auto& t: threads) {
            t.join();
        }

        std::cout << std::endl;
    }

}

int main() {
    start_test0();
    start_test1();

    return 0;
}

```