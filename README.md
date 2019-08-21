# TaskPool

When you want to reuse threads (reassign tasks), what is your approach?

Here is an experimental TaskPool

- TaskPool will create a thread per task
- The task can be reassigned or stay empty
- Currently the size is fixed at init time (vector stability, mv and copy ctor)
- RAII (it will dispose itself correctly)
- It's possible to add any type of function / task (when you implement a new fn signature add method)
- A repetition count can be set for a task
- Force stop of the tasks is possible
