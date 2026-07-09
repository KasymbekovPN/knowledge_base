"""
LLDB summary provider для struct Task { std::string name; int priority; }

Загрузка:
  (lldb) command script import lldb_formatters.py
  (lldb) type summary add Task -F lldb_formatters.task_summary

Можно также автозагрузить через ~/.lldbinit:
  command script import /path/to/lldb_formatters.py
  type summary add Task -F lldb_formatters.task_summary
"""

def task_summary(valobj, internal_dict):
    name = valobj.GetChildMemberWithName('name').GetSummary()
    priority = valobj.GetChildMemberWithName('priority').GetValueAsSigned()
    priority_label = {1: 'HIGH', 2: 'MEDIUM', 3: 'LOW'}.get(priority, 'UNKNOWN')
    return f'Task({name}, priority={priority_label})'