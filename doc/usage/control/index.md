# Level Control

Control mode allows you to set levels for individual addresses. Only one universe may be controlled at a time.

```{image} screenshots/control_faders_light.png
:alt: Screenshot of Send Levels page
:class: only-light screenshot
:align: center
:scale: 50%
```

```{image} screenshots/control_faders_dark.png
:alt: Screenshot of Send Levels page
:class: only-dark screenshot
:align: center
:scale: 50%
```

## Usage

1. Select the universe in the [Config](#config) menu.

2. Press [Start]{.btn .btn-success}.

3. Adjust levels.

4. Press [Stop]{.btn .btn-danger} to stop transmitting.

```{note}
If this program was the only device transmitting sACN on the network, receivers will follow their programmed data 
loss behavior. This means that lights may not turn off immediately.
```

## Config

```{figure} screenshots/config_light.png
:alt: Screenshot of configuration menu
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Configuration menu
```

```{figure} screenshots/config_dark.png
:alt: Screenshot of configuration menu
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Configuration menu
```

(control-config-universe)=
Universe
: sACN universe to broadcast on.

(control-config-priority)=
Priority
: sACN priority. Levels with higher priority will take precedence over levels with lower priority. If two sources are
broadcasting with the same priority, the highest level will take precedence. The default priority on most sACN sources,
including this program, is `100`.

(control-config-pap)=
Use Per-Address-Priority
: When enabled, the priority setting will be applied to only address with levels above 0%. This mode is generally only
compatible with sACN receivers designed by ETC (including dimmers and gateways), as it is a proprietary extension to the
sACN standard. Receivers that do not understand per-address-priority will behave as if the priority setting is applied
to the entire universe.

## Modes

Multiple control modes are available. Set levels are retained when switching control modes.

### Faders

Press [<i class="bi bi-sliders"></i> Faders]{.btn .btn-tab} to show faders mode.

```{figure} screenshots/control_faders_light.png
:alt: Screenshot of faders mode
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Faders mode
```

```{figure} screenshots/control_faders_dark.png
:alt: Screenshot of faders mode
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Faders mode
```

The faders mode presents each address as a fader. Slide the fader from left (zero) to right (full) to adjust the level.

### Keypad

Press [<i class="bi bi-keyboard"></i> Keypad]{.btn .btn-tab} to show keypad mode.

```{figure} screenshots/control_keypad_light.png
:alt: Screenshot of keypad mode
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Keypad mode
```

```{figure} screenshots/control_keypad_dark.png
:alt: Screenshot of keypad mode
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Keypad mode
```

The keypad mode allows more fine-grained control of levels. Set levels using the keypad as you would on a command-line
lighting console, like the ETC Eos family.

To assist entering correct commands, buttons that are not allowed for the current command line are shaded a darker color
and cannot be pressed.

All commands must be committed by pressing [Enter]{.btn .btn-sm .btn-light}. Press [Clear]{.btn .btn-sm .btn-light} to
backspace. If the current command line has been committed (i.e. the last button pressed was [Enter]{.btn .btn-sm
.btn-light}), pressing any button will begin a new command line.

```{note}
Levels are always entered in the display format selected in the settings. This means if you have chosen to display
levels as hex, you must enter levels as hex (e.g. full is [F]{.btn .btn-sm .btn-light} [F]{.btn .btn-sm .btn-light}) or
if you have chosen to display levels as decimal (full is [2]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light}
[5]{.btn .btn-sm .btn-light}).
```

Some example command lines (assuming the level display mode is set to Percent):

- [1]{.btn .btn-sm .btn-light} [At]{.btn .btn-sm .btn-light} [1]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light}
  [0]{.btn .btn-sm .btn-light} [Enter]{.btn .btn-sm .btn-light} sets address 1 to full.
- [1]{.btn .btn-sm .btn-light} [Thru]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light}
  [At]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light}
  [Enter]{.btn .btn-sm .btn-light} sets addresses 1, 2, 3, 4, and 5 to 50.
- [1]{.btn .btn-sm .btn-light} [+]{.btn .btn-sm .btn-light} [3]{.btn .btn-sm .btn-light} [At]{.btn .btn-sm .btn-light}
  [5]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light} [Enter]{.btn .btn-sm .btn-light} sets addresses 1 and 3 to
  50.
- [1]{.btn .btn-sm .btn-light} [Thru]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light} [-]{.btn .btn-sm .btn-light}
  [3]{.btn .btn-sm .btn-light} [At]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light}
  [Enter]{.btn .btn-sm .btn-light} sets addresses 1, 2, 4, and 5 to 50.
- [1]{.btn .btn-sm .btn-light} [Thru]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light} [+]{.btn .btn-sm .btn-light}
  [1]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light} [At]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light}
- [0]{.btn .btn-sm .btn-light} [Enter]{.btn .btn-sm .btn-light} sets addresses 1, 2, 3, 4, 5, and 10 to 50.
- [1]{.btn .btn-sm .btn-light} [At]{.btn .btn-sm .btn-light} [+]{.btn .btn-sm .btn-light} [1]{.btn .btn-sm .btn-light}
  [0]{.btn .btn-sm .btn-light} [Enter]{.btn .btn-sm .btn-light} sets address 1 to 10 points above its current level.
- [1]{.btn .btn-sm .btn-light} [At]{.btn .btn-sm .btn-light} [-]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light}
  [Enter]{.btn .btn-sm .btn-light} sets address 1 to 5 points below its current level.
- [1]{.btn .btn-sm .btn-light} [Thru]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light}
  [At]{.btn .btn-sm .btn-light} [1]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light}
  [Thru]{.btn .btn-sm .btn-light} [5]{.btn .btn-sm .btn-light} [0]{.btn .btn-sm .btn-light}
  [Enter]{.btn .btn-sm .btn-light} sets address 1 to 10, 2 to 20, 3 to 30, 4 to 40, and 5 to 50.
