import keyboard
import pygame
import threading
import time
import json
import os

import webbrowser

WIDTH = 360
HEIGHT = 500

COMMANDS = json.load(open('config.json'))['commands']

window_shown = False

def open_browser(url):
    webbrowser.open(url)

def type_text(text):
    time.sleep(0.5)
    keyboard.write(text, delay=0.05)

def run_term_command(cmd):
    os.system(f"start cmd /k {cmd}")

def run_os_command(cmd):
    os.system(cmd)


def show_window():
    global window_shown

    if window_shown: pass
    else:
        t = threading.Thread(target=display_window)
        window_shown = True
        t.run()

def display_window():
    global window_shown

    pygame.init()

    search_prompt_font = pygame.font.SysFont('calibri', 32)
    search = ""

    info_font = pygame.font.SysFont('calibri', 16)
    suggestions_font = pygame.font.SysFont('calibri', 20)
    suggestions_desc_font = pygame.font.SysFont('calibri', 10)
    suggestions_num_font = pygame.font.SysFont('calibri', 30)

    screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.NOFRAME)
    screen.fill((240, 240, 240))
    pygame.display.flip()

    running = True
    suggestions = []
    first_run = True
    while running:
        toupdate = False
        runcommand = False
        runopt = False
        opt = -1
        events = pygame.event.get()
        for event in events:
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_SPACE:
                    pass
                elif event.key == pygame.K_BACKSPACE:
                    if len(search) > 0:
                        search = search[:-1]
                        toupdate = True
                elif event.key == pygame.K_RETURN:
                    runopt = True
                    opt = 1
                elif event.unicode in ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']:
                    runopt = True
                    opt = int(event.unicode)
                else:
                    c = event.unicode
                    search += c
                    toupdate = True

        if toupdate or first_run:
            suggestions = []
            screen.fill((240, 240, 240))
            text_surf = search_prompt_font.render(search, True, (10, 10, 10))
            screen.blit(text_surf, text_surf.get_rect(center=(WIDTH // 2, 75)))

            text_surf = info_font.render("Type out a command and press ENTER to run.", True, (60, 60, 60))
            screen.blit(text_surf, text_surf.get_rect(center=(WIDTH // 2, 30)))

            pygame.draw.line(screen, (90, 90, 90), (0, 120), (WIDTH, 120), 1)

            if len(search) > 0:
                if search[0] == '-':
                    text_surf = info_font.render("This command will execute in terminal", True, (60, 60, 60))
                    screen.blit(text_surf, text_surf.get_rect(center=(WIDTH // 2, 200)))
                elif search[0] == '.':
                    text_surf = info_font.render("This command will be forwarded to the OS", True, (60, 60, 60))
                    screen.blit(text_surf, text_surf.get_rect(center=(WIDTH // 2, 200)))
                else:
                    for c in COMMANDS:
                        if c[0].startswith(search):
                            suggestions.append(c)

                    for s in range(min(len(suggestions), 5)):
                        text_surf = suggestions_font.render(suggestions[s][0], True, (60, 60, 60))
                        screen.blit(text_surf, (30, 150 + (50 * s)))

                        text_surf = suggestions_desc_font.render(suggestions[s][1]["description"], True, (60, 60, 60))
                        screen.blit(text_surf, (30, 150 + (50 * s) + 20))

                        text_surf = suggestions_num_font.render(str(s + 1), True, (60, 60, 60))
                        screen.blit(text_surf, (320, 150 + (50 * s)))

            else:
                text_surf = info_font.render("Command suggestions will show up here.", True, (60, 60, 60))
                screen.blit(text_surf, text_surf.get_rect(center=(WIDTH // 2, 200)))

            pygame.display.flip()

        if first_run: first_run = False

        if runopt:
            if search[0] in ['.', '-']:
                runcommand = True
                running = False
                break
            elif opt <= len(suggestions):
                search = suggestions[opt - 1][0]
                runcommand = True
                running = False
                break

    window_shown = False
    pygame.display.quit()
    pygame.quit()


    if runcommand:
        if search[0] == '-':
            if len(search) >= 1: run_term_command(search[1:])
        elif search[0] == '.':
            if len(search) >= 1: run_os_command(search[1:])
        else:
            for c in COMMANDS:
                if search == c[0]:
                    cmd = c[1]
                    if cmd["type"] == "text":
                        type_text(cmd["arg"])
                    elif cmd["type"] == "browser":
                        open_browser(cmd["arg"])
                    elif cmd["type"] == "terminal":
                        run_term_command(cmd["arg"])
                    elif cmd["type"] == "os":
                        run_os_command(cmd["arg"])

if __name__ == '__main__':
    keyboard.add_hotkey('ctrl + space', lambda: show_window())
    keyboard.wait()
