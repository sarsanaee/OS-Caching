#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <linux/netpoll.h>


#define MESSAGE_SIZE 50


#define INADDR_LOCAL ((unsigned long int)0xc0a80066) //192.168.0.102
#define INADDR_SEND ((unsigned long int)0xc0a80065) //192.168.0.101


static struct netpoll* np = NULL;
static struct netpoll np_t;

static int first_packet = 1;


#define UDP_WATCH_PORT     11211//31337  /* UDP port */

int nf_register_net_hook(struct net *net, const struct nf_hook_ops *ops);
void nf_unregister_net_hook(struct net *net, const struct nf_hook_ops *ops);

static struct nf_hook_ops pkt_ctrl_post;
static struct nf_hook_ops pkt_ctrl_receive;
//static struct nf_hook_ops pkt_ctrl_local_in;
//static struct nf_hook_ops pkt_ctrl_forward;


void init_np_poll(u16 local_port, u16 remote_port)
{
    np_t.name = "LRNG";
    strlcpy(np_t.dev_name, "enp3s0", IFNAMSIZ);
    np_t.local_ip.ip = htonl(INADDR_LOCAL);
    np_t.remote_ip.ip = htonl(INADDR_SEND);
    np_t.local_port = local_port;
    np_t.remote_port = remote_port;
    memset(np_t.remote_mac, 0xff, ETH_ALEN);
    //netpoll_print_options(&np_t);
    netpoll_setup(&np_t);
    np = &np_t; 
}


void send_np_poll(void)
{
    int len;
    char message[MESSAGE_SIZE];
    sprintf(message,"%s\n","aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    len = strlen(message);
    netpoll_send_udp(np, message, len);
}

unsigned int custom_hook_forward(void *priv,
                   struct sk_buff *skb,
                   const struct nf_hook_state *state)
{
  struct iphdr *iph;          /* IPv4 header */
  struct udphdr *udph;        /* UDP header */
  u16 sport, dport;           /* Source and destination ports */
  u32 saddr, daddr;           /* Source and destination addresses */
  unsigned char *user_data;   /* UDP data begin pointer */
  unsigned char *tail;        /* UDP data end pointer */
  unsigned char *it;          /* UDP data iterator */

  if (!skb)
    return NF_ACCEPT;

  iph = ip_hdr(skb); // Getting IP Header
  udph = udp_hdr(skb); // Getting UDP Header
  
  /* Skip if it's not UDP packet */
  if (iph->protocol != IPPROTO_UDP)
      return NF_ACCEPT;

  /* Convert network endianness to host endiannes */
  saddr = ntohl(iph->saddr);
  daddr = ntohl(iph->daddr);
  sport = ntohs(udph->source);
  dport = ntohs(udph->dest);

  /* Watch only port of interest */
  if (sport != UDP_WATCH_PORT)
      return NF_ACCEPT;

  user_data = (unsigned char *)((unsigned char *)udph + (4 * 2)); // 4 fields each 2 bytes
  tail = skb_tail_pointer(skb);  
  
  printk("FORWARD, print_udp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport,
                              &daddr, dport);
  //skb->ip_summed = NETIF_F_HW_CSUM;

  printk("FORWARD, print_udp: data: %d\n", skb->ip_summed);
  // for (it = user_data; it != tail; ++it) {
  //     char c = *(char *)it;

  //     if (c == '\0')
  //         break;

  //     printk("%c", c);
  // }
  // printk("\n\n");
  return NF_ACCEPT;
}

unsigned int custom_hook_local_in(void *priv,
                   struct sk_buff *skb,
                   const struct nf_hook_state *state)
{
  struct iphdr *iph;          /* IPv4 header */
  struct udphdr *udph;        /* UDP header */
  u16 sport, dport;           /* Source and destination ports */
  u32 saddr, daddr;           /* Source and destination addresses */
  unsigned char *user_data;   /* UDP data begin pointer */
  unsigned char *tail;        /* UDP data end pointer */
  unsigned char *it;          /* UDP data iterator */

  if (!skb)
    return NF_ACCEPT;

  iph = ip_hdr(skb); // Getting IP Header
  udph = udp_hdr(skb); // Getting UDP Header
  
  /* Skip if it's not UDP packet */
  if (iph->protocol != IPPROTO_UDP)
      return NF_ACCEPT;

  /* Convert network endianness to host endiannes */
  saddr = ntohl(iph->saddr);
  daddr = ntohl(iph->daddr);
  sport = ntohs(udph->source);
  dport = ntohs(udph->dest);

  /* Watch only port of interest */
  if (dport != UDP_WATCH_PORT)
      return NF_ACCEPT;

  user_data = (unsigned char *)((unsigned char *)udph + (4 * 2)); // 4 fields each 2 bytes
  tail = skb_tail_pointer(skb);  
  
  printk("LOCAL_IN, print_udp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport,
                              &daddr, dport);
  //skb->ip_summed = NETIF_F_HW_CSUM;

  printk("LOCAL_IN, print_udp: data: %d\n", skb->ip_summed);
  // for (it = user_data; it != tail; ++it) {
  //     char c = *(char *)it;

  //     if (c == '\0')
  //         break;

  //     printk("%c", c);
  // }
  // printk("\n\n");
  return NF_ACCEPT;
}

unsigned int custom_hook_post(void *priv,
                   struct sk_buff *skb,
                   const struct nf_hook_state *state)
{
  struct iphdr *iph;          /* IPv4 header */
  struct udphdr *udph;        /* UDP header */
  u16 sport, dport;           /* Source and destination ports */
  u32 saddr, daddr;           /* Source and destination addresses */
  unsigned char *user_data;   /* UDP data begin pointer */
  unsigned char *tail;        /* UDP data end pointer */
  unsigned char *it;          /* UDP data iterator */

  if (!skb)
    return NF_ACCEPT;

  iph = ip_hdr(skb); // Getting IP Header
  udph = udp_hdr(skb); // Getting UDP Header
  
  /* Skip if it's not UDP packet */
  if (iph->protocol != IPPROTO_UDP)
      return NF_ACCEPT;

  /* Convert network endianness to host endiannes */
  saddr = ntohl(iph->saddr);
  daddr = ntohl(iph->daddr);
  sport = ntohs(udph->source);
  dport = ntohs(udph->dest);

  /* Watch only port of interest */
  if (sport != UDP_WATCH_PORT)
      return NF_ACCEPT;

  user_data = (unsigned char *)((unsigned char *)udph + (4 * 2)); // 4 fields each 2 bytes
  tail = skb_tail_pointer(skb);  
  
//   printk("POST, print_udp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport,
//                               &daddr, dport);

//   printk("POST, print_udp: data: %d\n", skb->ip_summed);
//   printk("POST, CHECKSUMS %d %d\n", iph->check, udph->check);
//   for (it = user_data; it != tail; ++it) {
//       char c = *(char *)it;

//       if (c == '\0')
//           break;

//       printk("%c", c);
//   }
//   printk("\n\n");

  return NF_ACCEPT;
}

unsigned int custom_hook_receive(void *priv,
                   struct sk_buff *skb,
                   const struct nf_hook_state *state)
{
    struct iphdr *iph;          /* IPv4 header */
    struct udphdr *udph;        /* UDP header */
    u16 sport, dport, pswap;    /* Source and destination ports */
    u32 saddr, daddr, aswap;    /* Source and destination addresses */
    unsigned char *user_data;   /* UDP data begin pointer */
    unsigned char *tail;        /* UDP data end pointer */
    unsigned char *it;          /* UDP data iterator */

    unsigned int udplen;

    if (!skb)
        return NF_ACCEPT;

    iph = ip_hdr(skb); // Getting IP Header
    udph = udp_hdr(skb); // Getting UDP Header
    
    /* Skip if it's not TCP packet */
    if (iph->protocol != IPPROTO_UDP)
        return NF_ACCEPT;

    /* Convert network endianness to host endiannes */
    saddr = ntohl(iph->saddr);
    daddr = ntohl(iph->daddr);
    sport = ntohs(udph->source);
    dport = ntohs(udph->dest);

    /* Watch only port of interest */
    if (dport != UDP_WATCH_PORT)
        return NF_ACCEPT;

    user_data = (unsigned char *)((unsigned char *)udph + (4 * 2)); // 4 fields each 2 bytes
    tail = skb_tail_pointer(skb);  
    
    // printk("RECEIVE, print_udp: %pI4h:%d -> %pI4h:%d\n", &saddr,
    //                                                      sport,
    //                                                      &daddr, 
    //                                                      dport);
    // printk("RECEIVE, print_udp: data: %d\n", skb->ip_summed);
    // for (it = user_data; it != tail; ++it) {
    //     char c = *(char *)it;

    //     if (c == '\0')
    //         break;

    //     printk("%c", c);
    // }
    // printk("\n\n");
    
    if(first_packet){
        init_np_poll(dport, sport);
        first_packet = 0;
    }
    send_np_poll();


  return NF_DROP;
}



int init_module(void)
{
    pkt_ctrl_receive.hook = custom_hook_receive;
    pkt_ctrl_receive.pf = PF_INET;
    pkt_ctrl_receive.hooknum = NF_INET_PRE_ROUTING;
    pkt_ctrl_receive.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &pkt_ctrl_receive);
    printk(KERN_INFO "custom_hook_receive\n");

    pkt_ctrl_post.hook = custom_hook_post;
    pkt_ctrl_post.pf = PF_INET;
    pkt_ctrl_post.hooknum = NF_INET_POST_ROUTING;
    pkt_ctrl_post.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &pkt_ctrl_post);
    printk(KERN_INFO "custom_hook_post\n");

    // pkt_ctrl_local_in.hook = custom_hook_local_in;
    // pkt_ctrl_local_in.pf = PF_INET;
    // pkt_ctrl_local_in.hooknum = 1;//NF_IP_LOCAL_IN;
    // pkt_ctrl_local_in.priority = NF_IP_PRI_FIRST;
    // nf_register_net_hook(&init_net, &pkt_ctrl_local_in);
    // printk(KERN_INFO "custom_hook_local_in\n");

    // pkt_ctrl_forward.hook = custom_hook_forward;
    // pkt_ctrl_forward.pf = PF_INET;
    // pkt_ctrl_forward.hooknum = 2;//NF_IP_FORWARD;
    // pkt_ctrl_forward.priority = NF_IP_PRI_FIRST;
    // nf_register_net_hook(&init_net, &pkt_ctrl_forward);
    // printk(KERN_INFO "custom_hook_local_in\n");
    first_packet = 1;
    printk(KERN_INFO "Module Initialization is Done\n");
    return 0;
}


void cleanup_module (void)
{ 
    nf_unregister_net_hook(&init_net, &pkt_ctrl_receive);
    nf_unregister_net_hook(&init_net, &pkt_ctrl_post);
    // nf_unregister_net_hook(&init_net, &pkt_ctrl_local_in);
    // nf_unregister_net_hook(&init_net, &pkt_ctrl_forward);
    printk(KERN_INFO "khodafez\n");
}

MODULE_LICENSE("MIT");

