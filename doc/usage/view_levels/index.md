<style>
/* Raise */
html[data-theme=light] table.table-flickerfinder tbody tr:nth-child(2) th {
    background-color: var(--bs-cyan);
}
html[data-theme=dark] table.table-flickerfinder tbody tr:nth-child(2) th {
    background-color: var(--bs-blue);
}

/* Lower */
html[data-theme=light] table.table-flickerfinder tbody tr:nth-child(3) th {
    background-color: var(--bs-teal);
}
html[data-theme=dark] table.table-flickerfinder tbody tr:nth-child(3) th {
    background-color: var(--bs-green);
}

/* Same */
html[data-theme=light] table.table-flickerfinder tbody tr:nth-child(4) th {
    background-color: var(--bs-gray-500);
}
html[data-theme=dark] table.table-flickerfinder tbody tr:nth-child(4) th {
    background-color: var(--bs-gray-700);
}
</style>

# View Levels

View Levels mode shows all sACN levels on the network and their merge result.

```{image} screenshots/viewlevels_list_light.png
:alt: Screenshot of View Levels page
:class: only-light screenshot
:align: center
:scale: 50%
```

```{image} screenshots/viewlevels_list_dark.png
:alt: Screenshot of View Levels page
:class: only-dark screenshot
:align: center
:scale: 50%
```

## Usage

If any sACN sources are active on the network, their levels are shown for the given universe. If more than one sACN
source is active, the levels from all sources is merged following the standard sACN merge rules. Each source is assigned
a different color; the color legend is shown by opening the "Sources" dropdown.

### Universe Discovery

Discovered sACN universes are shown as buttons. There is a small delay (usually around 10 seconds, but depends on the
transmitters) from entering View Levels mode and universe discovery. Additionally, not all sACN transmitters make their
transmitted universes discoverable. To select a universe that has not been discovered, press
[Choose Universe...]{.btn .btn-outline-secondary}.

## Modes

Multiple display modes are available:

### Bars

Press [<i class="bi bi-list"></i> Bars]{.btn .btn-tab} to show bars mode.

```{figure} screenshots/viewlevels_list_light.png
:alt: Screenshot of bars mode
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Bars mode
```

```{figure} screenshots/viewlevels_list_dark.png
:alt: Screenshot of bars mode
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Bars mode
```

The given universe is shown as a list of bars, where each address has a bar showing its current level. The color of the
bar corresponds to the color of the winning source. The address is shown on the left of the bar. The level and priority
(if "Show Priorities" is checked) are shown on the right.

### Grid

Press [<i class="bi bi-table"></i> Grid]{.btn .btn-tab} to show grid mode.

```{figure} screenshots/viewlevels_grid_light.png
:alt: Screenshot of grid mode
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

Grid mode
```

```{figure} screenshots/viewlevels_grid_dark.png
:alt: Screenshot of grid mode
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

Grid mode
```

The given universe is shown as a grid with the first address in each row shown on the left. Much like
[Bars Mode](#bars), the color of the box corresponds to the color of the winning source. In each box, the level is shown
above the priority.

## Flicker Finder

When "Flicker Finder" is checked, the display changes to show level differences relative to when the checkbox was
checked.

Changes are marked with specific colors:

```{list-table}
:header-rows: 1
:stub-columns: 1
:widths: auto
:class: table table-flickerfinder

- - Color
  - Description
- - Clear
  - Level has not changed at all since Flicker Finder was activated.
- - Blue
  - Level is higher than when Flicker Finder was activated.
- - Green
  - Level is lower than when Flicker Finder was activated.
- - Gray
  - Level has changed since Flicker Finder was activated, but is now the same as its original value.
```

```{figure} screenshots/flickerfinder_list_light.png
:alt: Screenshot of Flicker Finder
:figclass: only-light
:class: only-light screenshot
:align: center
:scale: 50%

In this instance, address 1 has increased in value, address 2 has descreased in value, and address 3 has changed but 
returned to its original value.
```

```{figure} screenshots/flickerfinder_list_dark.png
:alt: Screenshot of Flicker Finder
:figclass: only-dark
:class: only-dark screenshot
:align: center
:scale: 50%

In this instance, address 1 has increased in value, address 2 has descreased in value, and address 3 has changed but 
returned to its original value.
```
