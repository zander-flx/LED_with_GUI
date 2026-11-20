import queue
import threading
import tkinter as tk
from tkinter import ttk, messagebox

import serial

PORT = "COM3"          # fixed serial port
BAUD = 115200
MODE_NAMES = {0: "OFF", 1: "BLINK", 2: "ALTERNATE", 3: "RUN"}


class ESP32GUI:
    def __init__(self, root):
        self.root = root
        root.title("ESP32 LED Mode Controller")
        root.resizable(False, False)

        self.ser = None
        self.rx = queue.Queue()

        # ---------- Status ----------
        stat = ttk.LabelFrame(root, text="Status")
        stat.grid(row=0, column=0, padx=10, pady=8, sticky="ew")

        self.cur_var  = tk.StringVar(value="Current Mode: --")
        self.pend_var = tk.StringVar(value="Next Mode: none")

        ttk.Label(stat, textvariable=self.cur_var,
                  font=("Arial", 11, "bold")
                  ).grid(row=0, sticky="w", padx=8, pady=2)
        ttk.Label(stat, textvariable=self.pend_var,
                  foreground="#0a60c0"
                  ).grid(row=1, sticky="w", padx=8, pady=2)

        # ---------- Mode selection ----------
        sel = ttk.LabelFrame(root, text="Select Next Mode")
        sel.grid(row=1, column=0, padx=10, pady=8, sticky="ew")

        self.buttons = []
        for m, name in MODE_NAMES.items():
            b = ttk.Button(sel, text=name, width=12, state="disabled",
                           command=lambda m=m: self.send_mode(m))
            b.grid(row=0, column=m, padx=5, pady=5)
            self.buttons.append(b)

        # Auto-connect to COM3
        self.connect()

        root.protocol("WM_DELETE_WINDOW", self.on_close)
        root.after(100, self.poll)

    # ---------- connection ----------
    def connect(self):
        try:
            self.ser = serial.Serial(PORT, BAUD, timeout=1)
        except Exception as e:
            messagebox.showerror("Serial Error",
                                 f"Could not open {PORT}:\n{e}")
            self.cur_var.set("Current Mode: -- (not connected)")
            return

        for b in self.buttons:
            b.config(state="normal")
        threading.Thread(target=self.reader, daemon=True).start()

    # ---------- serial I/O ----------
    def reader(self):
        while self.ser and self.ser.is_open:
            try:
                line = self.ser.readline().decode(errors="ignore").strip()
                if line:
                    self.rx.put(line)
            except Exception:
                break

    def poll(self):
        while True:
            try:
                line = self.rx.get_nowait()
            except queue.Empty:
                break
            self.handle_line(line)
        self.root.after(100, self.poll)

    def handle_line(self, line):
        if line.startswith("MODE:"):
            # Active mode changed -> clear the pending request
            m = int(line.split(":")[1])
            self.cur_var.set(f"Current Mode: {MODE_NAMES.get(m, '?')}")
            self.pend_var.set("Next Mode: none")

        elif line.startswith("PENDING:"):
            # A next mode was requested (switches or GUI) but the
            # current mode must finish its 3 cycles first
            parts = line.split(":")
            m = int(parts[1])
            src = parts[2] if len(parts) > 2 else ""
            where = "switches" if src == "SW" else "GUI"
            self.pend_var.set(
                f"Next Mode: {MODE_NAMES.get(m, '?')} ({where}) "
                f"- applies after 3 cycles")

    def send_mode(self, m):
        if not (self.ser and self.ser.is_open):
            return
        self.ser.write(f"{m}\n".encode())
        self.pend_var.set(
            f"Next Mode: {MODE_NAMES[m]} (GUI) - applies after 3 cycles")

    # ---------- cleanup ----------
    def on_close(self):
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = ESP32GUI(root)
    root.mainloop()