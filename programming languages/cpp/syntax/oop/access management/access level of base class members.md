---
tags:
  - programming-language
  - cpp
  - syntax
  - access
  - protected
---
[[__cpp syntax oop access management__|<==]]

Спецификаторы доступа `public`, `private`, `protected` играют большую роль в том, к каким именно переменным и функциям базового класса могут обращаться производные классы. Однако на доступ также влияет спецификатор доступа базового класса, применяемый при установке наследования

Эти спецификаторы накладываются друг на друга и образуют 9 возможных комбинаций.


| Base class mod | Field in base class mod | Field in derived class mod  |
| -------------- | ----------------------- | --------------------------- |
| public         | public                  | public                      |
| public         | protected               | protected                   |
| public         | private                 | наследуется, но нет доступа |
| protected      | public                  | protected                   |
| protected      | protected               | protected                   |
| protected      | private                 | наследуется, но нет доступа |
| private        | public                  | private                     |
| private        | protected               | private                     |
| private        | private                 | наследуется, но нет доступа |

---
[Управление доступом в базовых и производных классах](https://metanit.com/cpp/tutorial/5.22.php)