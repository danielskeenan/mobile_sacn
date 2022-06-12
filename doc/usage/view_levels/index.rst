.. |btn_list| button::
   :color: secondary

   .. image:: /img/icons/list-solid.svg

.. |btn_grid| button::
   :color: secondary

   .. image:: /img/icons/table-cells-solid.svg

.. index:: View Levels

.. _viewlevels:

View Levels
===========

View Levels mode shows all sACN levels on the network and their merge result.

.. figure:: screenshots/viewlevels_list.png
   :class: screenshot

   View Levels screen

.. contents::
   :local:

Usage
-----

If any sACN sources are active on the network, their levels are shown for the given universe.  If more than one sACN
source is active, the levels from all sources is merged following the standard sACN merge rules.  Each source is
assigned a different color; the color legend is shown by opening the "Sources" dropdown.

.. note:: Per-address-priority is not currently supported in the View Levels screen.

.. _viewlevels-modes:

Modes
-----

Multiple display modes are available.

.. _viewlevels-modes-list:

List
^^^^

Press |btn_list| to show list mode.

.. figure:: screenshots/viewlevels_list.png
   :class: screenshot

   List mode

The given universe is shown as a list of bars, where each address has a bar showing its current level.  The color of the
bar corresponds to the color of the winning source.


.. _viewlevels-modes-grid:

Grid
^^^^

Press |btn_grid| to show grid mode.

.. figure:: screenshots/viewlevels_grid.png
   :class: screenshot

   Grid mode

The given universe is shown as a grid with the first address in each row shown on the left.  The number of columns in
each row is configurable.  Much like :ref:`List mode <viewlevels-modes-list>`, the color of the bar corresponds to the
color of the winning source.
