// Плагин импортирует функцию хоста host_log(ptr, len) -- зеркально
// противоположность export_name: import_module задаёт неймспейс
// ("env"), import_name -- имя, под которым хост должен зарегистрировать
// свою реализацию.
__attribute__((import_module("env"), import_name("host_log")))
extern void host_log(const char* ptr, int len);

__attribute__((export_name("run")))
void run(void) {
    const char* msg = "Hello from the plugin, logged via host!";
    int len = 0;
    while (msg[len]) len++;

    // Плагин просто передаёт указатель+длину НА СВОЮ ЖЕ память -- как
    // и раньше, это просто смещение внутри его linear memory.
    host_log(msg, len);
}
