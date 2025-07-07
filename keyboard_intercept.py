import evdev
from evdev import InputDevice, categorize, ecodes

# Replace this path with your second keyboard's device
DEVICE_PATH = '/dev/input/by-id/usb-SEMICO_USB_Keyboard-event-kbd'

# Open the device
dev = InputDevice(DEVICE_PATH)

# Grab exclusive access to prevent events from reaching X11 or the kernel
dev.grab()

print(f"Listening to {dev.name} as macropad...")

try:
    for event in dev.read_loop():
        if event.type == ecodes.EV_KEY:
            key_event = categorize(event)
            if key_event.keystate == key_event.key_down:
                print(f"Pressed: {key_event.keycode}")
                # Example macro action
                if key_event.keycode == 'KEY_F1':
                    print("Running macro: Launching script")
                    # call your script here (os.system, subprocess, etc.)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    dev.ungrab()
