Network monitor using PF_NETLINK kernel level sockets.

By grabbing kernel info monitor displays:
- changes in network interfaces
- changes in routing tables
- changes in ARP tables

Compilation:  gcc -Wall ./NetlinkMonitor.c -o ./NetlinkMonitor

Usage:        ./NetlinkMonitor
