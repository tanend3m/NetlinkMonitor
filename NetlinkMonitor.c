#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NIP(addr) \
	((unsigned char*) &addr)[0], \
	((unsigned char*) &addr)[1], \
	((unsigned char*) &addr)[2], \
	((unsigned char*) &addr)[3] \

#define HRD(addr) \
	((unsigned char*) &addr)[0], \
	((unsigned char*) &addr)[1], \
	((unsigned char*) &addr)[2], \
	((unsigned char*) &addr)[3], \
	((unsigned char*) &addr)[4], \
	((unsigned char*) &addr)[5] \



  int rclen, nllen, atlen;
  char buf[8192];
  struct rtmsg *rtp;

//GETTING SOCKET
  int open_nl()
  {
    struct sockaddr_nl snl;
    int sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    memset(&snl, 0, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = RTMGRP_IPV4_ROUTE | RTMGRP_NOTIFY | RTMGRP_IPV4_IFADDR | RTMGRP_LINK;
    bind(sock, (struct sockaddr*) &snl, sizeof(struct sockaddr_nl));

    return sock;
  }


//ROUTING MSG HANDLER
  void route_hdlr(struct nlmsghdr* nlp)
  {
    char dst[32], msk[32], gwy[32], dev[32];
    //Macro NLMSG_DATA returns a pointer to routing data
    struct rtmsg *rtp = (struct rtmsg*) NLMSG_DATA(nlp);
    if(rtp->rtm_table != RT_TABLE_MAIN)
      return;
    struct rtattr *atp = (struct rtattr*) RTM_RTA(rtp);
    int atlen = RTM_PAYLOAD(nlp);
    memset(dst, 0, sizeof(dst));
    memset(msk, 0, sizeof(msk));
    memset(gwy, 0, sizeof(gwy));     
    memset(dev, 0, sizeof(dev));
    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) 
    {
      switch(atp->rta_type) 
      {
        case RTA_DST:     inet_ntop(AF_INET, RTA_DATA(atp), dst,
                                    sizeof(dst));
                          break;
        case RTA_GATEWAY: inet_ntop(AF_INET, RTA_DATA(atp), gwy,
                                    sizeof(gwy));
                          break;
        case RTA_OIF:     sprintf(dev, "%d", *((int*) RTA_DATA(atp)));
                          break;
      }
    }
    sprintf(msk, "%d", rtp->rtm_dst_len);
    if (nlp->nlmsg_type == RTM_DELROUTE)
      printf("[DEL ROUTE] ");
    else if (nlp->nlmsg_type == RTM_NEWROUTE)       
    printf("[ADD ROUTE] ");
    if (strlen(dst) == 0)
      printf("default via %s dev %s\n", gwy, dev);
    else if (strlen(gwy) == 0)
      printf("%s/%s dev %s\n", dst, msk, dev);
    else
      printf("dst %s/%s gwy %s dev %s\n", dst, msk, gwy, dev);
  }


//INTERFACE ADDRRESS MSG HANDLER
  void iface_hdlr(struct nlmsghdr* nlp)
  {
    struct ifaddrmsg *ifp = (struct ifaddrmsg*) NLMSG_DATA(nlp);
    char ifname[1024], ifadd[1024];
    struct in_addr *inp;

    memset(ifname, 0, sizeof(ifname));
    memset(ifadd, 0, sizeof(ifadd));

    struct rtattr *atp = (struct rtattr*) IFA_RTA(ifp);
    int atlen = RTM_PAYLOAD(nlp);

    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) 
    {
      switch(atp->rta_type) 
      {
        case IFA_LABEL:   sprintf(ifname, "%s", (char*) RTA_DATA(atp));
                          break;
        case IFA_ADDRESS: inp = (struct in_addr *) RTA_DATA(atp);
			  //inet_ntop(AF_INET, RTA_DATA(atp), ifadd, sizeof(ifadd));
                          break;
        
      }
    }

    switch(nlp->nlmsg_type)
    {
      case RTM_NEWADDR:
          printf("[NEW IF]");
          break;
      case RTM_DELADDR:
          printf("[DEL IF]");
          break;
    }
    printf("Ifname: %s | Hardware add:  %u.%u.%u.%u\n", ifname, NIP(*inp));
  }


//ARP MSG HANDLER
  void arp_hdlr(struct nlmsghdr* nlp)
  {
    char nda_dst[32], lladdr[32];
    struct ndmsg *arp = (struct ndmsg*) NLMSG_DATA(nlp);
    struct rtattr *atp = (struct rtattr*) RTM_RTA(arp);
    int atlen = RTM_PAYLOAD(nlp);
    
    memset(nda_dst, 0, sizeof(nda_dst));
    memset(lladdr, 0, sizeof(lladdr));
    
    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) 
    {
      switch(atp->rta_type) 
      {
        case NDA_DST:     inet_ntop(AF_INET, RTA_DATA(atp), nda_dst,
                                    sizeof(nda_dst));
                          break;
        case NDA_LLADDR: inet_ntop(AF_INET, RTA_DATA(atp), lladdr,
                                    sizeof(lladdr));
                          break;
      }
    }

    switch(nlp->nlmsg_type)
    {
      case RTM_NEWNEIGH:
          printf("[NEW ARP]");
          break;
      case RTM_DELNEIGH:
          printf("[DEL ARP]");
          break;
    }
    printf("Layer dest addr: %s Link layer add: %s\n", nda_dst, lladdr);
  }

  //NEW INTERFACE CONFIG MSG HANDLER
  void link_hdlr(struct nlmsghdr* nlp)
  {
    char hdradd[32], dev[1024];

    memset(hdradd, 0, sizeof(hdradd));
    memset(dev, 0, sizeof(dev));

    struct ifinfomsg *ifi = (struct ifinfomsg*) NLMSG_DATA(nlp);
    struct rtattr *atp = (struct rtattr*) IFLA_RTA(ifi);
    struct in_addr *inp;

    int atlen = RTM_PAYLOAD(nlp);
    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) 
    {
      switch(atp->rta_type) 
      {
        case IFLA_IFNAME:      sprintf(dev, "%s", (char*) RTA_DATA(atp));
                               break;

        case IFLA_ADDRESS:     inp = (struct in_addr*) RTA_DATA(atp);
			       //sprintf(hdradd, "%s", (char*) RTA_DATA(atp));
                               break;
      }
    }

    switch(nlp->nlmsg_type)
    {
      case RTM_NEWLINK:
          printf("[NEW IF]");
          break;
      case RTM_DELLINK:
          printf("[DEL IF]");
          break;
    }
    printf("Ifname : %s", dev);
    printf(" | Ifaddress : %02x:%02x:%02x:%02x:%02x:%02x\n", HRD(*inp));

  }

  //RULE MSG HANDLER
  void rule_hdlr(struct nlmsghdr* nlp)
  {
    char dst[32], msk[32], gwy[32], dev[32];
    struct rtmsg *rtp = (struct rtmsg*) NLMSG_DATA(nlp);
    if(rtp->rtm_table != RT_TABLE_MAIN)
      return;
    struct rtattr *atp = (struct rtattr*) RTM_RTA(rtp);
    int atlen = RTM_PAYLOAD(nlp);
    
    memset(dst, 0, sizeof(dst));
    memset(msk, 0, sizeof(msk));
    memset(gwy, 0, sizeof(gwy));     
    memset(dev, 0, sizeof(dev));
    
    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) 
    {
      switch(atp->rta_type) 
      {
        case RTA_DST:     inet_ntop(AF_INET, RTA_DATA(atp), dst,
                                    sizeof(dst));
                          break;
        case RTA_GATEWAY: inet_ntop(AF_INET, RTA_DATA(atp), gwy,
                                    sizeof(gwy));
                          break;
        case RTA_OIF:     sprintf(dev, "%d", *((int*) RTA_DATA(atp)));
                          break;
      }
    }
    
    sprintf(msk, "%d", rtp->rtm_dst_len);
    if (nlp->nlmsg_type == RTM_DELRULE)
      printf("[DEL ROUTE] ");
    else if (nlp->nlmsg_type == RTM_NEWRULE)       
    printf("[ADD ROUTE] ");
    if (strlen(dst) == 0)
      printf("default via %s dev %s\n", gwy, dev);
    else if (strlen(gwy) == 0)
      printf("%s/%s dev %s\n", dst, msk, dev);
    else
      printf("dst %s/%s gwy %s dev %s\n", dst, msk, gwy, dev);

  }

  //CLASS MSG HANDLER
  void class_hdlr(struct nlmsghdr* nlp)
  {

    char kind[1024];
    struct rtmsg *rtp = (struct rtmsg*) NLMSG_DATA(nlp);
    struct rtattr *atp = (struct rtattr*) TCA_RTA(rtp);
    int atlen = RTM_PAYLOAD(nlp);
    
    memset(kind, 0, sizeof(kind));
    
    for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) 
    {
      switch(atp->rta_type) 
      {
       case TCA_KIND:     sprintf(kind, "%s", (char*) RTA_DATA(atp));
                          break;
      }
    }

    switch(nlp->nlmsg_type)
    {
      case RTM_DELQDISC:
          printf("[NEW CLASS]");
          break;
      case RTM_NEWQDISC:
          printf("[DEL CLASS]");
          break;
    }
    printf("TCA_KIND: %s\n", kind);
  }

int main(int argc, char **argv) 
{
  char *ptr;
  int sfd = open_nl();
  struct nlmsghdr *nlp;
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
        case RTM_DELRULE:
        case RTM_NEWRULE:
                           rule_hdlr(nlp);
                           break;
        case RTM_NEWTCLASS:
        case RTM_DELTCLASS:
                           class_hdlr(nlp);
                           break;
                  
        
      }

    }
   }
  close(sfd);
  return EXIT_SUCCESS;
}
