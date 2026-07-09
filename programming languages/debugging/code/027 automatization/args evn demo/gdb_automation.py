"""
GDB smart breakpoints и автоматизация.
Загрузка: (gdb) source gdb_automation.py
"""
import gdb

class EveryNthBreakpoint(gdb.Breakpoint):
    """
    Останавливается только на КАЖДОМ N-ом попадании, остальные - тихо
    пропускает и продолжает сам. Логика с состоянием, которую нельзя
    выразить обычным expression-условием (нужен персистентный счётчик).
    """

    def __init__(self, location, every_n):
        super().__init__(location)
        self.every_n = every_n
        self.hit_counter = 0

    def stop(self):
        self.hit_counter += 1
        if self.hit_counter % self.every_n == 0:
            gdb.write(f"[EveryNthBreakpoint] Остановка на попадании #{self.hit_counter}\n")
            return True  # реально остановить выполнение
        return False  # тихо продолжить, не останавливаясь


class ThreadStateBreakpoint(gdb.Breakpoint):
    """
    Останавливается, только если СРЕДИ ВСЕХ потоков есть хотя бы один
    с конкретным значением локальной переменной. Такую кросс-тредовую
    проверку невозможно выразить в обычном breakpoint condition,
    который видит только состояние текущего потока.
    """

    def __init__(self, location, var_name, target_value):
        super().__init__(location)
        self.var_name = var_name
        self.target_value = target_value

    def stop(self):
        inferior = gdb.selected_inferior()
        for thread in inferior.threads():
            thread.switch()
            try:
                frame = gdb.selected_frame()
                val = frame.read_var(self.var_name)
                if int(val) == self.target_value:
                    gdb.write(f"[ThreadStateBreakpoint] Найдено {self.var_name}="
                              f"{self.target_value} в потоке {thread.num}\n")
                    return True
            except (gdb.error, ValueError):
                continue
        return False


class DumpAllThreads(gdb.Command):
    """
    Кастомная команда: (gdb) dump-threads
    Показывает компактную сводку по всем потокам: номер, функция,
    строка, и (если есть в скоупе) значение указанной переменной.
    """

    def __init__(self):
        super().__init__("dump-threads", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        var_name = arg.strip() if arg else None
        inferior = gdb.selected_inferior()
        current = gdb.selected_thread()

        gdb.write(f"{'ID':<4}{'LWP':<10}{'Function':<30}{'Extra':<20}\n")
        gdb.write("-" * 64 + "\n")

        for thread in inferior.threads():
            thread.switch()
            frame = gdb.selected_frame()
            func_name = frame.name() or "??"
            extra = ""
            if var_name:
                try:
                    val = frame.read_var(var_name)
                    extra = f"{var_name}={val}"
                except (gdb.error, ValueError):
                    extra = f"{var_name}=<нет в скоупе>"
            gdb.write(f"{thread.num:<4}{thread.ptid[1]:<10}{func_name:<30}{extra:<20}\n")

        current.switch()  # вернуться на исходный поток


class AutoLogAndContinue(gdb.Command):
    """
    (gdb) auto-log-loop <file:line> <count>
    Автоматически ставит breakpoint, логирует указанную переменную
    и продолжает N раз подряд - замена ручного commands/continue.
    """

    def __init__(self):
        super().__init__("auto-log-loop", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        parts = arg.split()
        if len(parts) < 3:
            gdb.write("Использование: auto-log-loop <file:line> <var> <count>\n")
            return

        location, var_name, count = parts[0], parts[1], int(parts[2])
        bp = gdb.Breakpoint(location)

        if gdb.selected_inferior().pid == 0:
            gdb.execute("run", to_string=True)
        else:
            gdb.execute("continue", to_string=True)

        for i in range(count):
            if not gdb.selected_thread() or not gdb.selected_thread().is_valid():
                gdb.write("Процесс завершился раньше времени\n")
                break
            try:
                val = gdb.selected_frame().read_var(var_name)
                gdb.write(f"[{i+1}/{count}] {var_name} = {val}\n")
            except gdb.error as e:
                gdb.write(f"[{i+1}/{count}] ошибка чтения {var_name}: {e}\n")

            if i < count - 1:
                gdb.execute("continue", to_string=True)

        bp.delete()


DumpAllThreads()
AutoLogAndContinue()