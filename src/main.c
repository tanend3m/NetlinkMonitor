#include "lib/nlmonitor.h"
#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) 
{
  int sfd = open_nl();
  struct nlmsghdr *nlp;
  int rclen, nllen, atlen;
  char buf[8192];
  char *ptr;
  struct rtmsg *rtp;

  printf("Starting monitor...\n");
   while(1) 
   {
    memset(&buf, 0, sizeof(buf));
    ptr = buf;
    nllen = 0;


    //Receiving kernel msg
    do 
    {
      rclen = recv(sfd, ptr, sizeof(buf) - nllen, 0);
      nlp = (struct nlmsghdr*) ptr;
      ptr += rclen;
      nllen += rclen;
    } while(nlp->nlmsg_type == NLMSG_DONE);

    nlp = (struct nlmsghdr*) buf;

    //Iterating over received msgs
    for(;NLMSG_OK(nlp, nllen); nlp = NLMSG_NEXT(nlp, nllen)) 
    {
      switch(nlp->nlmsg_type)
      {
        case RTM_DELROUTE:
        case RTM_NEWROUTE:
                           route_hdlr(nlp);
                           break;
        case RTM_NEWADDR:
        case RTM_DELADDR:
                           iface_hdlr(nlp);
                           break;
        case RTM_NEWNEIGH:
        case RTM_DELNEIGH:
                           arp_hdlr(nlp);
                           break;

        case RTM_NEWLINK:
        case RTM_DELLINK:
                          link_hdlr(nlp);
                          break;
       
      }

    }
   }
  close(sfd);
  return EXIT_SUCCESS;
}
