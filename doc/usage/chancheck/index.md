# Channel Check

Channel Check mode allows you to set a single address to a defined level, moving through the universe sequentially.

```{image} screenshots/chancheck_light.png
:alt: Screenshot of Channel Check page
:class: only-light screenshot
:align: center
:scale: 50%
```

```{image} screenshots/chancheck_dark.png
:alt: Screenshot of Channel Check page
:class: only-dark screenshot
:align: center
:scale: 50%
```

## Usage

1. Set the level you wish to set the checked address to using the Level fader.

```{image} screenshots/level_light.png
:alt: Screenshot of level fader
:class: only-light screenshot
:align: center
:scale: 50%
```

```{image} screenshots/level_dark.png
:alt: Screenshot of level fader
:class: only-dark screenshot
:align: center
:scale: 50%
```

2. Use [<i class="bi bi-caret-left-fill"></i>]{.btn .btn-primary} and
   [<i class="bi bi-caret-right-fill"></i>]{.btn .btn-primary} buttons to set the first address to check.

3. Press [Start]{.btn .btn-success} to begin transmitting.

4. The current address will be displayed in green while the program is transmitting:

```{figure} screenshots/addr_notransmit_light.png
:alt: Screenshot of address field
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Not transmitting
```

```{figure} screenshots/addr_notransmit_dark.png
:alt: Screenshot of address field
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Not transmitting
```

```{figure} screenshots/addr_transmit_light.png
:alt: Screenshot of address field
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Transmitting
```

```{figure} screenshots/addr_transmit_dark.png
:alt: Screenshot of address field
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Transmitting
```

5. Press [Stop]{.btn .btn-danger} to stop transmitting.

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

(chancheck-config-universe)=
Universe
: sACN universe to broadcast on.

(chancheck-config-priority)=
Priority
: sACN priority. Levels with higher priority will take precedence over levels with lower priority. If two sources are
broadcasting with the same priority, the highest level will take precedence. The default priority on most sACN sources,
including this program, is `100`.

(chancheck-config-pap)=
Use Per-Address-Priority
: When enabled, the priority setting will be applied to only the address currently being checked. This mode is generally
only compatible with sACN receivers designed by ETC (including dimmers and gateways), as it is a proprietary extension
to the sACN standard. Receivers that do not understand per-address-priority will behave as if the priority setting is
applied to the entire universe.

(chancheck-effect-blink)=
Blink
: When checked, the selected address will flash between the chosen level and 0% every second.

## Tips

### Channel checking a system in use

If you must check a system that is in use by others, and you don't want to disturb other levels, use the
per-address-priority option and Blink effect:

1. Set the priority to `200`.
2. Enable [per-address-priority](#chancheck-config-pap).
3. Enable the [Blink](#chancheck-effect-blink) effect.
4. Press [Start]{.btn .btn-success}.

The address currently being checked will blink, while all others will remain at their previous level.
