.. |btn_start| button::
   :color: success

   Start

.. |btn_stop| button::
   :color: danger

   Stop

.. |btn_faders| button::
   :color: secondary

   .. image:: /img/icons/sliders-solid.svg

.. |btn_keypad| button::
   :color: secondary

   .. image:: /img/icons/keyboard-solid.svg

.. |btn_0| button::
   :color: light

   0

.. |btn_1| button::
   :color: light

   1

.. |btn_2| button::
   :color: light

   2

.. |btn_3| button::
   :color: light

   3

.. |btn_4| button::
   :color: light

   4

.. |btn_5| button::
   :color: light

   5

.. |btn_6| button::
   :color: light

   6

.. |btn_7| button::
   :color: light

   7

.. |btn_8| button::
   :color: light

   8

.. |btn_9| button::
   :color: light

   9

.. |btn_clear| button::
   :color: light

   Clear

.. |btn_at| button::
   :color: light

   At

.. |btn_enter| button::
   :color: light

   Enter

.. |btn_plus| button::
   :color: light

   +

.. |btn_thru| button::
   :color: light

   Thru

.. |btn_minus| button::
   :color: light

   −

.. index:: Control

.. _control:

Level Control
=============

Control mode allows you to set levels for individual addresses.  Only one universe may be controlled at a time.

.. figure:: screenshots/control_faders.png
   :class: screenshot

   Control screen

.. contents::
   :local:

Usage
-----

1. Select the universe in the :ref:`Config <control-config-universe>` menu.
2. Press |btn_start|.
3. Adjust levels.
4. Press |btn_stop| to stop transmitting.

   .. note:: If this program was the only device transmitting sACN on the network, receivers will follow their
      programmed data loss behavior. This means that lights may not turn off immediately.

.. _control-config:

Config
------

.. figure:: screenshots/config.png
   :class: screenshot

   Configuration menu

Universe
   .. _control-config-universe:

   sACN universe to broadcast on.

Priority
   .. _control-config-priority:

   sACN priority. Levels with higher priority will take precedence over levels with lower priority. If two
   sources are broadcasting with the same priority, the highest level will take precedence. The default priority on most
   sACN sources, including this program, is ``100``.

.. _control-modes:

Modes
-----

Multiple control modes are available.  Set levels are retained when switching control modes.

.. index:: Faders

.. _control-modes-faders:

Faders
^^^^^^

Press |btn_faders| to show faders mode.

.. figure:: screenshots/control_faders.png
   :class: screenshot

   Faders mode

The faders mode presents each address as a fader.  Slide the fader from left (zero) to right (full) to adjust the level.

.. index:: Keypad

.. _control-modes-keypad:

Keypad
^^^^^^

Press |btn_keypad| to show keypad mode.

.. figure:: screenshots/control_keypad.png
   :class: screenshot

   Keypad mode

The keypad mode allows more fine-grained control of levels.  Set levels using the keypad as you would on a command-line
lighting console, like the ETC Eos family.

To assist entering correct commands, buttons that are not allowed for the current command line are shaded a darker color
and cannot be pressed.

All commands must be committed by pressing |btn_enter|.  Press |btn_clear| to backspace.  If the current command line
has been committed (i.e. the last button pressed was |btn_enter|), pressing any button will begin a new command line.

Some example command lines:

* |btn_1| |btn_at| |btn_1| |btn_0| |btn_0| |btn_enter| sets address 1 to full.
* |btn_1| |btn_thru| |btn_5| |btn_at| |btn_5| |btn_0| |btn_enter| sets addresses 1, 2, 3, 4, and 5 to 50.
* |btn_1| |btn_plus| |btn_3| |btn_at| |btn_5| |btn_0| |btn_enter| sets addresses 1 and 3 to 50.
* |btn_1| |btn_thru| |btn_5| |btn_minus| |btn_3| |btn_at| |btn_5| |btn_0| |btn_enter| sets addresses 1, 2, 4, and 5
  to 50.
* |btn_1| |btn_thru| |btn_5| |btn_plus| |btn_1| |btn_0| |btn_at| |btn_5| |btn_0| |btn_enter| sets addresses 1, 2,
  3, 4, 5, and 10 to 50.
* |btn_1| |btn_at| |btn_plus| |btn_1| |btn_0| |btn_enter| sets address 1 to 10 points above its current level.
* |btn_1| |btn_at| |btn_minus| |btn_5| |btn_enter| sets address 1 to 5 points below its current level.
* |btn_1| |btn_thru| |btn_5| |btn_at| |btn_1| |btn_0| |btn_thru| |btn_5| |btn_0| |btn_enter| sets address 1 to
  10, 2 to 20, 3 to 30, 4 to 40, and 5 to 50.
