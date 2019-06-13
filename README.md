System monitorowania parametrów sieci komputerowej 
w systemie operacyjnym GNU /Linux z użyciem gniazd sieciowych PF_NETLINK.

Program z wykorzystaniem biblioteki rtnetlink wykrywa i wypisuje na wyjście
informacje o dodaniu/usunięciu:
- interfejsu sieciowego
- adresu IP
- wpisu tablicy routingu
- wpisu tablicy arp


Compilation:  gcc -Wall ./NetlinkMonitor.c -o ./NetlinkMonitor

Usage:        ./NetlinkMonitor