FreeDCC - Digital Command Control System
========================================

This project is comprised of the following:

* **Command Station**: C firmware for an AVR micro *functional for baseline packets*
    * Integrated DSL parser to control system from a simple serial terminal, eg:

             > forward addr 10 speed 5
             > stop addr 10
             > reverse addr 10 speed 3
             > stop all

    * Packet scheduler (packet cache & auto-refreshing)
    * Simple internal design
    * *Raw* DCC packet passthrough allows hex-encoded DCC packets to be sent to the track, eg:

            > raw 0xdeadbeed

* **Booster**: design for a DCC signal booster designed to work with the command station *functional*
    * Schematics & board design based on minidcc.com's booster
* **Repeater**: takes a 5V DCC signal and outputs n identical signals for feeding into more boosters
* **DCC library**: A common DCC library to be shared between the different components
    * Coming soon...
* **Daemon**: Daemon to run on a host computer to achieve the following
    * Coordinate access to command station serial link
    * Send/receive JSON messages from clients via web sockets & TCP connections
    * Save state & system configuration information
* **Frontend**: an HTML/javascript frontend to communicate with the daemon
    * Communicate with daemon over network
    * To run in different web/mobile browsers
    * Web sockets to receive updates from daemon and send commands

Currently the command station & booster are functional and stable for NMRA baseline packets (according to S 9.2). I am an amateur electronics designer so that area definately can be improved. Still need to do a proper command station schematic & board design (still on a breadboard).

Daemon & frontend components have not yet been implemented.

Roadmap
-------

* Implement generic DCC library (with tests) to handle both baseline & extended format packets
* Implement daemon in Perl & C (probably a C extension for the DCC library, with the rest in Perl)
* Implement client
* Design better boards
* Extend command station to have separate programming track & programmer capabilities

Basic Design
------------

With a single booster and track section.

                                                       /  /
    +----------+------+         +---------+           /  / 
    | Command Station |-------->| Booster |-------> Track
    +----------+------+         +---------+         /  /
                                                   /  /

Or with a repeater in the mix and multiple boosters & isolated track sections:

    +----------+------+        +----------+
    | Command Station |------->| Repeater |
    +----------+------+        +----------+
                                    |
                                    |
                       +----------+-+--------+
                       |          |          |
                  +----+-----+    |      +---+-----+
                  |  Booster |   ...     | Booster |
                  +----+-----+           +-----+---+
                       |                       |
                       |                       |

                     /  /                    /  /
                    /  /                    /  /
                   Track         ...       Track
                  Section                 Section
                 /  /                    /  /
                /  /                    /  /
