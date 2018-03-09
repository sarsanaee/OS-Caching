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

#define PTCP_WATCH_PORT     11211  /* HTTP port */

int nf_register_net_hook(struct net *net, const struct nf_hook_ops *ops);
void nf_unregister_net_hook(struct net *net, const struct nf_hook_ops *ops);

static struct nf_hook_ops pkt_ctrl;
static struct nf_hook_ops pkt_ctrl2;



static uint16_t csum(uint16_t* buff, int nwords) {
    uint32_t sum;
    for (sum = 0; nwords > 0; nwords--)
            sum += *buff++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ((uint16_t) ~sum);
}

unsigned int custom_hook_forward(void *priv,
                   struct sk_buff *skb,
                   const struct nf_hook_state *state)
{
  struct iphdr *iph;          /* IPv4 header */
  struct tcphdr *tcph;        /* TCP header */
  u16 sport, dport;           /* Source and destination ports */
  u32 saddr, daddr;           /* Source and destination addresses */
  unsigned char *user_data;   /* TCP data begin pointer */
  unsigned char *tail;        /* TCP data end pointer */
  unsigned char *it;          /* TCP data iterator */

  if (!skb)
    return NF_ACCEPT;

  iph = ip_hdr(skb); // Getting IP Header
  tcph = tcp_hdr(skb); // Getting TCP Header
  
  /* Skip if it's not TCP packet */
  if (iph->protocol != IPPROTO_TCP)
      return NF_ACCEPT;

  /* Convert network endianness to host endiannes */
  saddr = ntohl(iph->saddr);
  daddr = ntohl(iph->daddr);
  sport = ntohs(tcph->source);
  dport = ntohs(tcph->dest);

  /* Watch only port of interest */
  if (sport != PTCP_WATCH_PORT)
      return NF_ACCEPT;

  user_data = (unsigned char *)((unsigned char *)tcph + (tcph->doff * 4));
  tail = skb_tail_pointer(skb);  
  
  printk("FORWARD, print_tcp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport,
                              &daddr, dport);
  //skb->ip_summed = NETIF_F_HW_CSUM;

  // printk("print_tcp: data:\n");
  // for (it = user_data; it != tail; ++it) {
  //     char c = *(char *)it;

  //     if (c == '\0')
  //         break;

  //     printk("%c", c);
  // }
  // printk("\n\n");


  return NF_ACCEPT;
  //return NF_DROP | NF_ACCEPT | NF_QUEUE;
}

unsigned int custom_hook_receive(void *priv,
                   struct sk_buff *skb,
                   const struct nf_hook_state *state)
{
  struct iphdr *iph;          /* IPv4 header */
  struct tcphdr *tcph;        /* TCP header */
  u16 sport, dport, pswap;           /* Source and destination ports */
  u32 saddr, daddr, aswap;           /* Source and destination addresses */
  unsigned char *user_data;   /* TCP data begin pointer */
  unsigned char *tail;        /* TCP data end pointer */
  unsigned char *it;          /* TCP data iterator */

  unsigned int tcplen;

  if (!skb)
    return NF_ACCEPT;

  iph = ip_hdr(skb); // Getting IP Header
  tcph = tcp_hdr(skb); // Getting TCP Header
  
  /* Skip if it's not TCP packet */
  if (iph->protocol != IPPROTO_TCP)
      return NF_ACCEPT;

  /* Convert network endianness to host endiannes */
  saddr = ntohl(iph->saddr);
  daddr = ntohl(iph->daddr);
  sport = ntohs(tcph->source);
  dport = ntohs(tcph->dest);

  /* Watch only port of interest */
  if (dport != PTCP_WATCH_PORT)
      return NF_ACCEPT;

  user_data = (unsigned char *)((unsigned char *)tcph + (tcph->doff * 4));
  tail = skb_tail_pointer(skb);  
  
  printk("RECEIVE, print_tcp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport, &daddr, dport);

  printk("print_tcp: data:\n");
  for (it = user_data; it != tail; ++it) {
      char c = *(char *)it;

      if (c == '\0')
          break;

      printk("%c", c);
  }
  printk("\n\n");

  // //Swapping addresses
  // aswap = iph->saddr;
  // iph->saddr = iph->daddr;
  // iph->daddr = aswap;

  // //Swapping ports
  // pswap = tcph->source;
  // tcph->source = tcph->dest;
  // tcph->dest = pswap;
  
  // tcplen = skb->len - ip_hdrlen(skb);
  // tcph->check = 0;
  // tcph->check = tcp_v4_check(tcplen,
	//               iph->saddr,
	//               iph->daddr,
	//               csum_partial((char *)tcph, tcplen, 0));
  // ip_send_check (iph);


  // saddr = ntohl(iph->saddr);
  // daddr = ntohl(iph->daddr);
  // sport = ntohs(tcph->source);
  // dport = ntohs(tcph->dest);
  // printk("RECEIVE2, print_tcp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport,
  //                             &daddr, dport);

  return NF_ACCEPT;
  //return NF_DROP | NF_ACCEPT | NF_QUEUE;
}

int init_module(void)
{
  pkt_ctrl.hook = custom_hook_receive;
  pkt_ctrl.pf = PF_INET;
  pkt_ctrl.hooknum = NF_INET_PRE_ROUTING;
  pkt_ctrl.priority = NF_IP_PRI_FIRST;
  printk(KERN_INFO "salam\n");
  nf_register_net_hook(&init_net, &pkt_ctrl);
  printk(KERN_INFO "test\n");

  pkt_ctrl2.hook = custom_hook_forward;
  pkt_ctrl2.pf = PF_INET;
  pkt_ctrl2.hooknum = NF_INET_POST_ROUTING;
  pkt_ctrl2.priority = NF_IP_PRI_FIRST;
  printk(KERN_INFO "salam\n");
  nf_register_net_hook(&init_net, &pkt_ctrl2);

  return 0;
}


void cleanup_module (void)
{ 
  printk(KERN_INFO "khodafez\n");
  nf_unregister_net_hook(&init_net, &pkt_ctrl);
  nf_unregister_net_hook(&init_net, &pkt_ctrl2);

}

//MODULE_LICENSE("GPL");

