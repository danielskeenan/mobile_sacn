.. index:: WiFi Hotspot

WiFi Hotspot
============

If your lighting network does not have a wireless access point, you can use the hotspot feature in your operating system
to create a wireless network for your mobile device.


Windows
-------

.. note:: Because of limitations in Windows, this feature is only available if your computer has an internet connection
   available.

1. Open the `Settings app <ms-settings:network-mobilehotspot>`_.
2. Toggle on "Mobile hotspot".
3. Open `Advanced network settings <ms-settings:network-advancedsettings>`_ and click "Hardware and connection properties".
4. A list of all network adapters on your system will appear.  Find the one where Description starts with ``Microsoft
   Wi-Fi Direct Virtual Adapter``, has an IPv4 address that does not start with ``169.254``, and does not
   show "Connectivity (IPv4/IPv6)" as ``Disconnected``.  This is the hotspot's network interface.  Make note of the
   "Name" field; you'll need it in a moment.  See the figures below for examples:

   .. figure:: img/screenshots/hotspot_wrong.svg
      :class: screenshot

      Description is correct, but IP Address and Connectivity are wrong.

   .. figure:: img/screenshots/hotspot_right.svg
      :class: screenshot

      All items are correct.  Note the connection name.

5. Open Mobile sACN.  Be sure to set the correct network adapters!  The "Web UI Interface" must be set to the connection
   name noted before.  The "sACN Interface" is the network adapter connected to your lighting network.
