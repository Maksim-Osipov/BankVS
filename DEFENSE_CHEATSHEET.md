# DEFENSE_CHEATSHEET

Короткая шпаргалка для защиты проекта BankVS.

## 1. 20 возможных вопросов преподавателя

| Вопрос | Короткий ответ | Где в коде |
|---|---|---|
| 1. Что делает программа? | Это банковская система: создает счета, пополняет, снимает, переводит деньги, начисляет проценты, сохраняет данные, показывает историю, выписки и статистику. | `main.cpp`, `MainWindow.cpp`, `Bank.cpp` |
| 2. Почему проект многофайловый? | Логика разделена по ответственности: счета, банк, файлы, главное окно и отдельные диалоги. Так код проще понимать и дорабатывать. | Все `.h`/`.cpp`, особенно `Bank.cpp`, `MainWindow.cpp`, dialog-файлы |
| 3. Зачем нужен класс `Account`? | Это общий базовый класс для всех счетов: ID, владелец, баланс, история и общий интерфейс операций. | `Account.h`, `Account.cpp` |
| 4. Почему `Account` абстрактный? | В нем есть чисто виртуальные методы: `withdraw`, `applyInterest`, `getType`, `serialize`, `canWithdraw`. Нельзя создать счет без конкретного типа. | `Account.h` |
| 5. Где используется наследование? | `SavingsAccount`, `CheckingAccount`, `CreditAccount` наследуются от `Account`. | `SavingsAccount.h`, `CheckingAccount.h`, `CreditAccount.h` |
| 6. Где используется полиморфизм? | `Bank` хранит `shared_ptr<Account>`, но вызывает методы конкретных наследников: `withdraw`, `applyInterest`, `serialize`. | `Bank.h`, `Bank.cpp` |
| 7. Зачем нужны виртуальные методы? | Чтобы одно обращение через `Account` выполняло разную логику для разных типов счетов. | `Account.h`, реализации наследников |
| 8. Чем отличается накопительный счет? | Не может уходить в минус, проценты начисляются на положительный баланс. | `SavingsAccount.cpp` |
| 9. Чем отличается расчетный счет? | Может уходить в минус в пределах `overdraftLimit`, проценты не начисляются. | `CheckingAccount.cpp` |
| 10. Чем отличается кредитный счет? | Может уходить в минус в пределах `creditLimit`, проценты начисляются на долг. | `CreditAccount.cpp` |
| 11. Что такое овердрафт? | Разрешенный отрицательный баланс расчетного счета. Проверка: `balance - amount >= -overdraftLimit`. | `CheckingAccount.cpp` |
| 12. Как работает перевод? | `TransferDialog` вызывает `Bank::transfer`, дальше `operator+` проверяет счета и лимиты, меняет балансы и добавляет `Transfer Out`/`Transfer In`. | `TransferDialog.cpp`, `Bank.cpp` |
| 13. Зачем перегружен `operator+`? | Для демонстрации перегрузки операторов: перевод оформлен как добавление `TransferRequest` к банку. | `Bank.h`, `Bank.cpp` |
| 14. Где используется шаблон? | `Transaction<T>` - шаблонный класс операции; в проекте используется `Transaction<double>`. | `Transaction.h`, `Account.h` |
| 15. Как хранится история операций? | В каждом счете есть `std::vector<Transaction<double>> history`. | `Account.h`, `Account.cpp` |
| 16. Почему используется `std::map`? | Счета хранятся по ID, поэтому удобно искать, удалять и обходить их по ключу. | `Bank.h` |
| 17. Зачем `std::shared_ptr`? | Чтобы хранить разные наследники через указатель на `Account` и автоматически управлять памятью. | `Bank.h`, `Bank.cpp` |
| 18. Как работает сохранение? | `FileManager::saveBank` открывает файл и вызывает `file << bank`; `Bank::operator<<` вызывает `serialize` у счетов. | `FileManager.cpp`, `Bank.cpp` |
| 19. Как работает загрузка? | `FileManager::loadBank` вызывает `file >> bank`; `Bank::operator>>` читает тип счета, создает объект и восстанавливает транзакции. | `FileManager.cpp`, `Bank.cpp` |
| 20. Где используется WinAPI и GDI? | WinAPI создает окна, кнопки, таблицу и обрабатывает события; GDI рисует график статистики. | `MainWindow.cpp`, dialog-файлы, `StatisticsDialog.cpp` |

## 2. 10 мест, которые нужно быстро находить

1. `main.cpp` - старт программы, создание `Bank` и `MainWindow`.
2. `MainWindow.cpp` - главное окно, кнопки, меню, таблица, обработка команд.
3. `Bank.cpp` - вся основная банковская логика.
4. `Account.h` - базовый абстрактный класс и виртуальные методы.
5. `SavingsAccount.cpp` - логика накопительного счета и проценты.
6. `CheckingAccount.cpp` - овердрафт расчетного счета.
7. `CreditAccount.cpp` - кредитный лимит и проценты по долгу.
8. `Transaction.h` - шаблонный класс операций и история.
9. `FileManager.cpp` - сохранение и загрузка `bank_data.txt`.
10. `StatisticsDialog.cpp` - окно статистики, `WM_PAINT`, GDI-график.

Дополнительно полезно помнить:

- `DialogUtils.cpp` - чтение ввода, ошибки, создание контролов.
- `CreateAccountDialog.cpp` - окно создания счета.
- `MoneyOperationDialog.cpp` - пополнение и снятие.
- `TransferDialog.cpp` - перевод.
- `StatementDialog.cpp` - выписки.

## 3. Быстро поменять типовые вещи

### Текст кнопки

- Файл: `MainWindow.cpp` или нужный `...Dialog.cpp`.
- Искать: `CreateButton(hwnd, L"...")`.
- Менять: строку `L"..."`.

Пример: кнопки главного окна находятся в `MainWindow::CreateControls`.

### Порядок кнопок

- Файл: чаще всего `MainWindow.cpp`.
- Искать: `CreateControls`, `addButton`.
- Менять: порядок вызовов `addButton(...)` или `CreateButton(...)`.

Если это кнопка внутри отдельного окна, искать `CreateControls` в нужном dialog-файле.

### Сообщение ошибки

- Файл: `DialogUtils.cpp` для общих GUI-ошибок.
- Искать: `BuildErrorMessage`, `ShowException`.
- Для конкретной банковской ошибки искать `throw InvalidAmountException`, `throw InvalidAccountException`, `throw InsufficientFundsException`.

Основные файлы с проверками: `Bank.cpp`, `Account.cpp`, `SavingsAccount.cpp`, `CheckingAccount.cpp`, `CreditAccount.cpp`, `DialogUtils.cpp`.

### Размер окна

- Главное окно: `MainWindow.cpp`, метод `MainWindow::Create`.
- Искать: `CreateWindowExW` и числа `1220, 760`.
- Диалоговые окна: нужный `...Dialog.cpp`, метод `Create`.
- Искать: `CreateWindowExW` и параметры ширины/высоты, например `510, 355`.

### Формат вывода баланса

- Для GUI-таблицы: `UiHelpers.cpp`, функция `FormatMoney`.
- Для списка счетов: `Bank.cpp`, `getAllAccountsText`.
- Для статистики: `StatisticsVisualizer.cpp`, `getAccountTable`, `getBalanceChart`.
- Для выписки: `Bank.cpp`, `generateStatement`.

Искать: `std::fixed`, `std::setprecision(2)`, `FormatMoney`.

## 4. Самая короткая схема проекта

`main.cpp` создает `Bank` и `MainWindow`.

`MainWindow.cpp` показывает интерфейс и открывает диалоги.

Диалоги читают ввод через `DialogUtils.cpp` и вызывают методы `Bank`.

`Bank.cpp` выполняет операции со счетами.

Конкретный тип счета сам решает, можно ли снять деньги и как начислять проценты.

Данные сохраняются через `FileManager.cpp`, а реальный формат сохранения находится в `Bank::operator<<`, `Bank::operator>>` и `serialize` у счетов.
