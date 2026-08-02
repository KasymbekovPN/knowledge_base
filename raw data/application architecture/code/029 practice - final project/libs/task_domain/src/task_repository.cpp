// #include "task_domain/task_repository.hpp"
// #include <algorithm>
// #include <stdexcept>
//
// namespace domain {
//
//     int TaskRepository::addTask(const std::string& title, int priority) {
//         int id = nextId_++;
//         tasks_.push_back({id, title, priority, false});
//         bus_.publish(TaskAdded{id, title, priority});
//         return id;
//     }
//
//     void TaskRepository::removeTask(int id) {
//         auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t) { return t.id == id; });
//         if (it == tasks_.end()) throw std::runtime_error("task not found: " + std::to_string(id));
//         Task removed = *it;
//         tasks_.erase(it);
//         bus_.publish(TaskRemoved{id, removed});
//     }
//
//     void TaskRepository::completeTask(int id) {
//         setDone(id, true);
//         bus_.publish(TaskCompleted{id});
//     }
//
//     void TaskRepository::reopenTask(int id) {
//         setDone(id, false);
//         bus_.publish(TaskReopened{id});
//     }
//
//     void TaskRepository::restoreTask(const Task& task) {
//         tasks_.push_back(task);
//         // nextId_ не трогаем - специально восстанавливаем СТАРЫЙ id, не выдаём новый
//         bus_.publish(TaskRestored{task.id});
//     }
//
//     std::optional<Task> TaskRepository::find(int id) const {
//         auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t) { return t.id == id; });
//         if (it == tasks_.end()) return std::nullopt;
//         return *it;
//     }
//
//     void TaskRepository::setDone(int id, bool done) {
//         auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t) { return t.id == id; });
//         if (it != tasks_.end()) it->done = done;
//     }
//
// }  // namespace domain
