#ifndef nlmonitor
#define nlmonitor
#include <linux/rtnetlink.h>

int open_nl();
void route_hdlr(struct nlmsghdr* nlp);
void iface_hdlr(struct nlmsghdr* nlp);
void arp_hdlr(struct nlmsghdr* nlp);
void link_hdlr(struct nlmsghdr* nlp);

#endif
