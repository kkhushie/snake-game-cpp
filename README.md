
# 🐍 Snake Quest — Console Snake Game (C++)

Snake Quest is a modern, emoji-styled Snake Game made using **Object-Oriented Programming in C++**.  
It runs in both **Windows** and **Linux/macOS terminals**, with smooth movement, levels, bombs, and a score system.

---

## ✨ Features

| Feature | Description |
|--------|-------------|
| 🎮 Player Name | User enters their name when game starts |
| 🧭 Smooth Direction Control | `W A S D` for up/left/down/right |
| 🧱 Bordered Arena | Snake dies if it touches walls |
| 🍎 Food System | Eating food increases score & tail |
| 💣 Bomb System | Hitting a bomb ends the game instantly |
| 🌟 High Score | Best score is tracked dynamically |
| ⚡ Difficulty Levels | Easy / Medium / Hard affect speed |
| ✅ Cross-Platform | Windows and Linux/Unix are supported |
| ⏱️ Non-blocking Input | Snake continues moving without waiting for key |

---

## 🛠️ Technologies Used

- Programming Language: **C++17**
- Concepts Included:
  - Object-Oriented Programming (Classes & Inheritance)
  - Encapsulation & Atomic variables
  - Cross-platform terminal handling
  - Game loops & Rendering logic
  - Real-time user input

---

## 🕹️ Controls

| Key | Action |
|-----|--------|
| `W` | Move Up |
| `S` | Move Down |
| `A` | Move Left |
| `D` | Move Right |
| `X` | Quit The Game |

---

## 🚀 How to Run

### ✅ Windows
1. Install a C++ compiler (ex: MinGW or Code::Blocks)
2. Open Command Prompt and compile:
   ```sh
   g++ snake.cpp -o snake.exe -std=c++17
   ```
3. Run:
   ```sh
   snake.exe
   ```

### 🐧 Linux & macOS
1. Install g++ if not available:
   ```sh
   sudo apt install g++  # Ubuntu/Debian
   ```
2. Compile & Run:
   ```sh
   g++ snake.cpp -o snake -std=c++17
   ./snake
   ```

---

## 📌 Game Structure (OOP Design)

| Class | Responsibility |
|-------|----------------|
| `Board` | Draw boundaries (💎 walls) |
| `Food` | Generates and draws food |
| `Bomb` | Inherits Food & draws bomb 💣 |
| `Snake` | Movement, tail logic, collisions |
| `Game` | Main gameplay logic, UI, difficulty |

---

## 🧠 Game Logic Summary

- Snake moves every loop based on direction
- Food increases:
  - ✅ Score by +10
  - ✅ Snake tail length
  - ✅ Respawns fruit & bomb
- Self-collision OR wall collision → Game Over
- Bomb collision → Explosive Game Over
- High score saved during runtime

---

## 🎨 Visual UI (Console Example)

 **Welcome Screen**
 
![WhatsApp Image 2025-11-06 at 12 43 23_22bd23dc](https://github.com/user-attachments/assets/9f93455d-ddd2-4db7-b9d3-2ce28188c4a5)

**Game Screen**

![WhatsApp Image 2025-11-06 at 12 43 20_ec1e629f](https://github.com/user-attachments/assets/a9126c14-4ee8-4ccd-be0b-f63b9a6f52f2)

**Ending Screen**

![WhatsApp Image 2025-11-06 at 12 43 21_b595ebbb](https://github.com/user-attachments/assets/434ccff9-1fe6-4fc5-9148-2a9d29ca6bdc)


---

## 🐍 Future Improvements (Optional)

✅ Walls for higher levels  
✅ Sound effects  
✅ Menu animations  
✅ Save high score permanently  
✅ Multiple fruit types  
✅ Pausing feature  

---

## 👤 Developer Info

**Project Name:** Snake Quest  
**Language:** C++  
**Year:** 2025  
**By:** VenomBytes

---


### 💎 THANKS FOR PLAYING SNAKE QUEST! 🎮

