"""
LLDB scripting: кастомные команды и breakpoint callback'и.

Загрузка:
  (lldb) command script import lldb_automation.py
"""
import lldb


def dump_threads(debugger, command, result, internal_dict):
    """
    (lldb) dump-threads [var_name]
    Аналог GDB-команды dump-threads: компактная сводка по всем потокам.
    """
    var_name = command.strip() if command.strip() else None
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()

    result.AppendMessage(f"{'ID':<4}{'TID':<10}{'Function':<30}{'Extra':<20}")
    result.AppendMessage("-" * 64)

    for thread in process:
        frame = thread.GetFrameAtIndex(0)
        func_name = frame.GetFunctionName() or "??"
        extra = ""
        if var_name:
            val = frame.FindVariable(var_name)
            if val.IsValid():
                extra = f"{var_name}={val.GetValue()}"
            else:
                extra = f"{var_name}=<нет в скоупе>"
        result.AppendMessage(f"{thread.GetIndexID():<4}{thread.GetThreadID():<10}"
                             f"{func_name:<30}{extra:<20}")


_every_nth_state = {"counter": 0}


def every_nth_callback(frame, bp_loc, internal_dict):
    """
    Breakpoint callback - LLDB-аналог gdb.Breakpoint.stop().
    Состояние храним в module-level словаре _every_nth_state, а не в
    internal_dict аргументе - на практике internal_dict оказался
    ненадёжным для персистентности между вызовами.

    Подключение:
      (lldb) breakpoint set --file main.cpp --line 26
      (lldb) breakpoint command add 1 -F lldb_automation.every_nth_callback
    Вернуть True - остановиться, False - тихо продолжить (аналог stop()).
    """
    _every_nth_state["counter"] += 1
    counter = _every_nth_state["counter"]

    every_n = 5
    if counter % every_n == 0:
        print(f"[every_nth_callback] Остановка на попадании #{counter}")
        return True
    return False


def __lldb_init_module(debugger, internal_dict):
    """
    Вызывается автоматически при 'command script import' - здесь
    регистрируем кастомные команды, аналог автозагрузки в GDB.
    """
    debugger.HandleCommand(
        'command script add -f lldb_automation.dump_threads dump-threads'
    )
    print("lldb_automation.py loaded: dump-threads command available")