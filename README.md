<!-- header:start -->
<!-- header:end -->

# TotalControl

A keyboard-driven command launcher. Press a shortcut and a small window pops up. Type a short
code like `ww` or `res` and it opens the site, folder, file or program behind it, or runs a shell
command. It started as a Python hackathon project in 2023, and in 2025 I began rewriting it in C++ for Linux.

## C++ version (2025, Linux)

There are two programs, and they talk over a TCP socket on port 3142.

- **`command_server.cpp`**: a background server that handles each client connection on its own thread.
  Each message it receives is one command:
  - a code from its `COMMANDS` table, which opens a folder or URL with `xdg-open` or launches a program in `gnome-terminal`
  - `.<command>`, which runs a shell command
  - `-<command>`, which runs a command in a new `gnome-terminal` window
- **`main.cpp`**: the popup, a borderless 400x600 SDL2 window. Type a code (letters, `.` and `-`) and
  press Enter to send it to the server and close the window. Esc closes it without sending. Number keys
  are meant to send entries from a short quick list. The window says "Autocomplete is not ready yet".
- **`keyboard_intercept.py`** (July 2025): grabs a second USB keyboard through `evdev` so it can act as
  a macropad. So far it only prints key presses, with F1 as a placeholder macro.
- **`Daemon/`, `Launcher/`, `Runner/`**: empty files for a planned split into separate programs,
  including a trie for autocomplete. Nothing is written there yet.

### Build and run

```sh
sudo apt install g++ libsdl2-dev libsdl2-ttf-dev
g++ -o command_server command_server.cpp -pthread
make build                 # builds the popup as ./sdl_app
./command_server &         # leave it running
./sdl_app                  # bind this to a global keyboard shortcut in your desktop settings
```

The popup expects Fira Mono at `/usr/share/fonts/opentype/fira/FiraMono-Medium.otf`. Change `FONT_PATH`
in `main.cpp` if your system has it somewhere else. The command table points at paths on my
machine, so edit `COMMANDS` before using it.

> **Security note:** the server listens on every network interface (`INADDR_ANY`) and runs whatever it
> receives. A message starting with `.` goes straight to `system()`, so anyone who can reach port 3142
> can run commands as you. Before running it, bind the server to `127.0.0.1`
> (`serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);`) or block the port with a firewall.

Known rough edges, since this is still work in progress:

- Number key `1` picks the second quick-list entry, and `5` reads past the end of the list.
- The popup's quick list (`se`, `sem`, `ssh`, ...) is left over from the Python version and doesn't
  match the server's table.

## Python version (January 2023, ConUHacks VII)

[`python-v1/totalcontrol.py`](python-v1/totalcontrol.py) is the original, built at ConUHacks VII in
Montreal. It was written for Windows:

- `Ctrl+Space` (a global hotkey from the `keyboard` library) opens a borderless 360x500 pygame window.
- As you type, it lists up to five commands whose code starts with what you've typed. Press a number to
  pick one, or Enter to pick the first.
- `-<command>` runs in a new `cmd` window, and `.<command>` runs directly.
- Commands are read from a `config.json` next to the script. Each one has a `type`:
  - `text` types out a string for you, such as your email address
  - `browser` opens a URL
  - `terminal` runs a command in a `cmd` window
  - `os` runs a program

```json
{"commands": [
    ["pe",    {"type": "text",     "arg": "you@example.com",             "description": "Types out personal email"}],
    ["learn", {"type": "browser",  "arg": "https://learn.uwaterloo.ca/", "description": "Opens UW Learn"}],
    ["ssh",   {"type": "terminal", "arg": "ssh myserver",                "description": "Opens SSH into personal server"}],
    ["np",    {"type": "os",       "arg": "notepad",                     "description": "Open Notepad"}]
]}
```

```sh
pip install keyboard pygame
python python-v1/totalcontrol.py   # run from the folder with config.json in it
```

The original `config.json` isn't included because it held my contact details. The example above
shows its format.

Two years later, on 2025-01-22, I started the C++ rewrite.

## History

- The 2025 C++ commits are the original ones.
- `python-v1/totalcontrol.py` was added in 2026 from my ConUHacks VII folder. Its commit carries
  the file's last-modified time, 2023-01-22, as the author date. That time was shifted 4 hours later,
  because the backup it came from has timestamps running 4 hours behind the real times.
- This README was written in 2026.
