# Week 06: Team Deathmatch Framework



---

Клієнтам не можна довіряти керування власним рахунком. Якби логіка вбивств оброблялася на стороні клієнта, зловмисник міг би модифікувати свою гру та надсилати підроблені запити для накрутки вбивств. Завдяки розміщенню цієї логіки в GameMode, Сервер виступає єдиним авторитетом . Він безпечно перевіряє факт вбивства, оновлює дані в TDMGameState та TDMPlayerState, а рушій Unreal Engine автоматично розсилає ці зміни всім клієнтам. Клієнти просто чекають на ці оновлені дані за допомогою RepNotify і пасивно оновлюють свій інтерфейс .

### Architecture Diagram

```mermaid
graph TD
    subgraph Server [Server Only]
        GM[TDMGameMode<br>- Manages match flow rules<br>- Handles ScoreKill logic<br>- Controls match timer]
    end

    subgraph Replicated [Replicated State]
        GS[TDMGameState<br>- TeamScores array<br>- RemainingMatchTime]
        PS[TDMPlayerState<br>- TeamId<br>- Kills<br>- Deaths]
    end

    subgraph Client [Client]
        PC[TDMGamePlayerController<br>- Local player control<br>- Spawns UI widgets]
        UI[WBP_Scoreboard / WBP_PlayerRow<br>- Displays score table<br>- Updates passively via RepNotify]
    end

    GM -->|Authoritative Updates| GS
    GM -->|Authoritative Updates| PS
    GS -.->|Network Replication| UI
    PS -.->|Network Replication| UI
    PC -->|Locally Spawns| UI

