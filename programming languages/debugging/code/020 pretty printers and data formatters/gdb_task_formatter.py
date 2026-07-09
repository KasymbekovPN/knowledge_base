"""
GDB pretty-printer для struct Task { std::string name; int priority; }

Загрузка:
  (gdb) source gdb_printers.py
или в .gdbinit:
  source gdb_printers.py
"""
import gdb

class TaskPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        name = str(self.val['name'])
        priority = int(self.val['priority'])
        priority_label = {1: 'HIGH', 2: 'MEDIUM', 3: 'LOW'}.get(priority, 'UNKNOWN')
        return f'Task({name}, priority={priority_label})'

def task_lookup(val):
    if str(val.type.strip_typedefs()) == 'Task':
        return TaskPrinter(val)
    return None

gdb.pretty_printers.append(task_lookup)