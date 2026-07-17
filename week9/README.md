# Week 09 Сервер і запуск 



Обрано **Path B - Listen Server**.

**Причина:** мій движок  це **бінарна  збірка з Epic Games Launcher**, а не source-build.
Для такої збірки **Path A (dedicated server) не збирається**  Visual Studio / UBT видає
«Server targets are not currently supported from this engine distribution». Тому dedicated-сервер
неможливий без source-build движка, і я йду Path B, який працює на бінарному UE.

Listen-сервер це один запущений клієнт, який **одночасно грає і хостить** гру для інших.
Перемикач `?listen`— це runtime-параметр URL, працює з будь-якою спакованою UE-грою (не треба
окремого build-таргета).

## Як запустити (команди)

Скрипти лежать поруч і мають бути **скопійовані в теку зі спакованим `Multi.exe`** (`Packaged\Windows\`).

### Хост (listen server)
```
Multi.exe /Game/ThirdPerson/Lvl_ThirdPerson?listen?port=7777 -log -nosteam
```
або просто запустити `host_listen_server.bat`.

Розшифровка аргументів:
| Аргумент | Що робить |
|---|---|
| `/Game/ThirdPerson/Lvl_ThirdPerson` | яку карту відкрити |
| `?listen` | зробити цей екземпляр listen-сервером (хостить і грає) |
| `?port=7777` | UDP-порт, який слухає сервер |
| `-log` | відкрити вікно логу |

> Примітка: до dedicated-сервера `?listen` **не** додають  це саме listen-server параметр.

### Клієнт (приєднання по прямому IP)
```
Multi.exe 192.168.x.x:7777 -log -nosteam
```
або `join_client.bat 192.168.x.x:7777`.

(`-nosteam`  Path B використовує пряме IP-з’єднання, тому Steam тут не потрібен.)

Ще варіант: запустити `Multi.exe` звичайно, натиснути `~` (консоль) і ввести:
```
open 192.168.x.x:7777
```

## Перевірено (лог)

Локальний тест (хост + клієнт через `127.0.0.1:7777`) підтвердив, що все працює:

**Хост:**
```
LogNet: Created socket for bind address: 0.0.0.0:7777
LogNet: IpNetDriver listening on port 7777
LogNet: NotifyAcceptedConnection: ... RemoteAddr: 127.0.0.1:60911
LogNet: AddClientConnection: Added client connection ...
LogNet: Join request: /Game/ThirdPerson/Lvl_ThirdPerson?...
LogNet: Join succeeded: DESKTOP-...
```
**Клієнт:**
```
LogNet: Welcomed by server (Level: /Game/ThirdPerson/Lvl_ThirdPerson, Game: BP_ThirdPersonGameMode_C)
LogGlobalStatus: UPendingNetGame::TravelCompleted
```


## Обмеження listen-сервера (спостереження)

- Хост **одночасно рендерить, симулює і роздає мережевий трафік** на одній машині  на слабкому
  залізі хост може підгальмовувати інших гравців. Для навчання й малого ко-опу ок, для реальної
  гри  ні.
- Якщо **хост вийшов  сесія закривається** (немає host migration).
- Це не масштабоване рішення; dedicated-сервер (Path A) був би кращим, але потребує source-build.


