# Week 10  Захист серверної валідації

Захищена версія проєкту Team Deathmatch (база Week 6), UE 5.7.

> Захист застосовано **прямо в основному модулі** `Deathmatch` — увесь проєкт і є захищеною версією.
> Ця тека `Week10/` містить документи (`audit.md` + цей README). Щоб зберегти оригінал, зафіксуйте
> версію до Week 10 в окремій гілці/тезі перед мержем.

## Що змінено
Новий шар anti-cheat у `Source/Deathmatch/AntiCheat/`:
- **`AntiCheatLog.*`**  окрема категорія `LogAntiCheat` + `LogAntiCheatRejection()` (пише час,
  гравця, дію, причину).
- **`AntiCheatComponent.*`**  `UAntiCheatComponent`, ліміт частоти на **token bucket**, на гравця й
  на дію (на PlayerController).
- **`HardenedCharacterMovementComponent.*`**  перевизначає `ServerCheckClientError`: відхиляє
  неможливу швидкість через штатну корекцію рушія.

Геймплей:
- **`TDM/TDMPlayerController.*`**  `ServerFire` з лімітом частоти й перевірками (живий, патрони,
  точка й напрям прицілу) перед серверним трейсом; новий `ServerReload` з лімітом і перевіркою
  стану. Постріл  **ліва кнопка миші**, перезарядка — **R**.
- **`TDM/TDMCharacter.*`**  серверна реплікована модель патронів.

Повний опис (6 проблем) — у `audit.md`.





Відхилення руху (`action=Move`) видно лише з модифікованим/спідхак-клієнтом; сама перевірка описана в
`audit.md` §5..

## Формат логу
Див. `Docs/expected-log-format.txt` це формат, який видає `UE_LOG(LogAntiCheat, …)` у
`AntiCheatLog.cpp`.
