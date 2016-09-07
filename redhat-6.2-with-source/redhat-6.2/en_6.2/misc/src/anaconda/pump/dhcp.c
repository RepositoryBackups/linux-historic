#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_packet.h>
#include <net/route.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <popt.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>

#include "pump.h"

typedef int bp_int32;
typedef short bp_int16;

#define _(a) (a)

#define BOOTP_OPTION_NETMASK		1
#define BOOTP_OPTION_GATEWAY		3
#define BOOTP_OPTION_DNS		6
#define BOOTP_OPTION_HOSTNAME		12
#define BOOTP_OPTION_BOOTFILE		13
#define BOOTP_OPTION_DOMAIN		15
#define BOOTP_OPTION_BROADCAST		28

#define DHCP_OPTION_LOGSRVS		7
#define DHCP_OPTION_LPRSRVS		9
#define DHCP_OPTION_NTPSRVS		42
#define DHCP_OPTION_XFNTSRVS		48
#define DHCP_OPTION_XDMSRVS		49
#define DHCP_OPTION_REQADDR		50
#define DHCP_OPTION_LEASE		51
#define DHCP_OPTION_OVERLOAD		52
#define DHCP_OPTION_TYPE		53
#define DHCP_OPTION_SERVER		54
#define DHCP_OPTION_OPTIONREQ		55
#define DHCP_OPTION_MAXSIZE		57
#define DHCP_OPTION_T1			58

#define BOOTP_CLIENT_PORT	68
#define BOOTP_SERVER_PORT	67

#define BOOTP_OPCODE_REQUEST	1
#define BOOTP_OPCODE_REPLY	2

#define NORESPONSE		-10
#define DHCP_TYPE_DISCOVER	1
#define DHCP_TYPE_OFFER		2
#define DHCP_TYPE_REQUEST	3
#define DHCP_TYPE_DECLINE	4
#define DHCP_TYPE_ACK		5
#define DHCP_TYPE_NAK		6
#define DHCP_TYPE_RELEASE	7
#define DHCP_TYPE_INFORM	8

#define DEFAULT_NUM_RETRIES	5
#define DEFAULT_TIMEOUT 	30

#define BOOTP_VENDOR_LENGTH	64
#define DHCP_VENDOR_LENGTH	312

struct bootpRequest {
    char opcode;
    char hw;
    char hwlength;
    char hopcount;
    bp_int32 id;
    bp_int16 secs;
    bp_int16 flags;
    bp_int32 ciaddr, yiaddr, server_ip, bootp_gw_ip;
    char hwaddr[16];
    char servername[64];
    char bootfile[128];
    char vendor[DHCP_VENDOR_LENGTH];
} ;

struct psuedohUdpHeader {
    bp_int32 source, dest;
    char zero;
    char protocol;
    bp_int16 len;
};

static void parseReply(struct bootpRequest * breq, struct pumpNetIntf * intf);
static char * prepareRequest(struct bootpRequest * breq,
			     int sock, char * device, time_t startTime);
static void parseLease(struct bootpRequest * bresp, struct pumpNetIntf * intf);
static void initVendorCodes(struct bootpRequest * breq);
static char * handleTransaction(int s, struct pumpOverrideInfo * override, 
				struct bootpRequest * breq,
			        struct bootpRequest * bresp, 
			        struct sockaddr_in * serverAddr,
			        struct sockaddr_in * respondant,
				int useBootPacket, int dhcpResponseType);
static int dhcpMessageType(struct bootpRequest * response);
static int newKernel(void);
static char * getInterfaceInfo(struct pumpNetIntf * intf, int s);
static char * perrorstr(char * msg);

static const char vendCookie[] = { 99, 130, 83, 99, 255 };

static int newKernel(void) {
    struct utsname ubuf;
    int major1, major2;

    uname(&ubuf);
    if (!strcasecmp(ubuf.sysname, "linux")) {
	if (sscanf(ubuf.release, "%d.%d", &major1, &major2) != 2 ||
		(major1 < 2) || (major1 == 2 && major2 == 0)) {
	    return 0;
	}
    }

    return 1;
}

static char * getInterfaceInfo(struct pumpNetIntf * intf, int s) {
    struct ifreq req;
    struct sockaddr_in * addrp;

    strcpy(req.ifr_name, intf->device);
    if (ioctl(s, SIOCGIFBRDADDR, &req))
	return perrorstr("SIOCGIFBRDADDR");

    addrp = (struct sockaddr_in *) &req.ifr_addr;
    intf->broadcast = addrp->sin_addr;
    intf->set = PUMP_INTFINFO_HAS_BROADCAST;

    return NULL;
}


static char * perrorstr(char * msg) {
    static char * err = NULL;
    static int errsize = 0;
    static int newsize;

    newsize = strlen(msg) + strlen(strerror(errno)) + 3;
    if (!errsize) {
	errsize = newsize;
	err = malloc(errsize);
    } else if (errsize < newsize) {
	free(err);
	errsize = newsize;
	err = malloc(errsize);
    } 
 
    if (err)
        sprintf(err, "%s: %s", msg, strerror(errno));
    else
	err = "out of memory!";

    return err;
}


char * pumpDisableInterface(char * device) {
    struct ifreq req;
    int s;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    strcpy(req.ifr_name, device);
    if (ioctl(s, SIOCGIFFLAGS, &req)) {
	close(s);
	return perrorstr("SIOCGIFFLAGS");
    }

    req.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
    if (ioctl(s, SIOCSIFFLAGS, &req)) {
	close(s);
	return perrorstr("SIOCSIFFLAGS");
    }

    close(s);

    return NULL;
}

char * pumpSetupInterface(struct pumpNetIntf * intf) {
    char * rc;
    struct sockaddr_in * addrp;
    struct ifreq req;
    struct rtentry route;
    int s;

    if ((rc = pumpDisableInterface(intf->device))) return rc;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    /* we have to have basic information to get this far */
    addrp = (struct sockaddr_in *) &req.ifr_addr;
    addrp->sin_family = AF_INET;
    strcpy(req.ifr_name, intf->device);
   
    addrp->sin_addr = intf->ip;
    if (ioctl(s, SIOCSIFADDR, &req))
	return perrorstr("SIOCSIFADDR");

    addrp->sin_addr = intf->netmask;
    if (ioctl(s, SIOCSIFNETMASK, &req))
	return perrorstr("SIOCSIFNETMASK");

    addrp->sin_addr = intf->broadcast;
    if (ioctl(s, SIOCSIFBRDADDR, &req))
	return perrorstr("SIOCSIFBRDADDR");

    /* bring up the device, and specifically allow broadcasts through it */
    req.ifr_flags = IFF_UP | IFF_RUNNING | IFF_BROADCAST;
    if (ioctl(s, SIOCSIFFLAGS, &req))
	return perrorstr("SIOCSIFFLAGS");

    /* add a route for this network */
    route.rt_dev = intf->device;
    route.rt_flags = RTF_UP;
    route.rt_metric = 0;

    if (!newKernel()) {
	addrp->sin_family = AF_INET;
	addrp->sin_port = 0;
	addrp->sin_addr = intf->network;
	memcpy(&route.rt_dst, addrp, sizeof(*addrp));
	addrp->sin_addr = intf->netmask;
	memcpy(&route.rt_genmask, addrp, sizeof(*addrp));

	if (ioctl(s, SIOCADDRT, &route)) {
	    /* the route cannot already exist, as we've taken the device down */
	    return perrorstr("SIOCADDRT 1");
	}
    }

    return NULL;
}

int pumpSetupDefaultGateway(struct in_addr * gw) {
    struct sockaddr_in addr;
    struct rtentry route;
    int s;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    memcpy(&route.rt_dst, &addr, sizeof(addr));
    memcpy(&route.rt_genmask, &addr, sizeof(addr));
    addr.sin_addr = *gw;
    memcpy(&route.rt_gateway, &addr, sizeof(addr));
    
    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_metric = 0;
    route.rt_dev = NULL;

    if (ioctl(s, SIOCADDRT, &route)) {
	return -1;
    }

    return 0;
}

char * pumpPrepareInterface(struct pumpNetIntf * intf, int s) {
    struct sockaddr_in * addrp;
    struct ifreq req;
    struct rtentry route;

    addrp = (struct sockaddr_in *) &req.ifr_addr;

    strcpy(req.ifr_name, intf->device);
    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    memset(&addrp->sin_addr, 0, sizeof(addrp->sin_addr));

    addrp->sin_family = AF_INET;
    addrp->sin_addr.s_addr = htonl(0);

    if (ioctl(s, SIOCSIFADDR, &req))
	return perrorstr("SIOCSIFADDR");

    if (!newKernel()) {
	if (ioctl(s, SIOCSIFNETMASK, &req))
	    return perrorstr("SIOCSIFNETMASK");

	/* the broadcast address is 255.255.255.255 */
	memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
	if (ioctl(s, SIOCSIFBRDADDR, &req))
	    return perrorstr("SIOCSIFBRDADDR");
    }

    req.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, &req))
	return perrorstr("SIOCSIFFLAGS");

    memset(&route, 0, sizeof(route));
    memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = INADDR_ANY;
    memcpy(&route.rt_dst, addrp, sizeof(*addrp));
    memcpy(&route.rt_genmask, addrp, sizeof(*addrp));

    route.rt_dev = intf->device;
    route.rt_flags = RTF_UP;
    route.rt_metric = 0;

    if (ioctl(s, SIOCADDRT, &route)) {
	if (errno != EEXIST) {
	    close(s);
	    return perrorstr("SIOCADDRT 3");
	}
    }

    return NULL;
}

static int dhcpMessageType(struct bootpRequest * response) {
    unsigned char * chptr;
    unsigned char option, length;
   
    chptr = response->vendor;

    chptr += 4;
    while (*chptr != 0xFF) {
	option = *chptr++;
	if (!option) continue;
	length = *chptr++;
	if (option == DHCP_OPTION_TYPE)
	    return *chptr;

	chptr += length;
    }

    return -1;
}

static void setMissingIpInfo(struct pumpNetIntf * intf) {
    bp_int32 ipNum = *((bp_int32 *) &intf->ip);
    bp_int32 nmNum = *((bp_int32 *) &intf->netmask);
    bp_int32 ipRealNum = ntohl(ipNum);

    if (!(intf->set & PUMP_INTFINFO_HAS_NETMASK)) {
	if (((ipRealNum & 0xFF000000) >> 24) <= 127)
	    nmNum = 0xFF000000;
	else if (((ipRealNum & 0xFF000000) >> 24) <= 191)
	    nmNum = 0xFFFF0000;
	else 
	    nmNum = 0xFFFFFF00;
	*((bp_int32 *) &intf->netmask) = nmNum = htonl(nmNum);
	syslog (LOG_DEBUG, "intf: netmask: %s", inet_ntoa (intf->netmask));
    }

    if (!(intf->set & PUMP_INTFINFO_HAS_BROADCAST)) {
	*((bp_int32 *) &intf->broadcast) = (ipNum & nmNum) | ~(nmNum);
	syslog (LOG_DEBUG, "intf: broadcast: %s", inet_ntoa (intf->broadcast));
    }

    if (!(intf->set & PUMP_INTFINFO_HAS_NETWORK)) {
	*((bp_int32 *) &intf->network) = ipNum & nmNum;
	syslog (LOG_DEBUG, "intf: network: %s", inet_ntoa (intf->network));
    }

    intf->set |= PUMP_INTFINFO_HAS_BROADCAST | PUMP_INTFINFO_HAS_NETWORK | 
		 PUMP_INTFINFO_HAS_NETMASK;
}

static void parseReply(struct bootpRequest * breq, struct pumpNetIntf * intf) {
    unsigned int i;
    unsigned char * chptr;
    unsigned char option, length;

    syslog (LOG_DEBUG, "intf: device: %s", intf->device);
    syslog (LOG_DEBUG, "intf: set: %i", intf->set);
    syslog (LOG_DEBUG, "intf: bootServer: %s", inet_ntoa (intf->bootServer));
    syslog (LOG_DEBUG, "intf: reqLease: %i", intf->reqLease);

    i = ~(PUMP_INTFINFO_HAS_IP | PUMP_INTFINFO_HAS_NETMASK | PUMP_INTFINFO_HAS_NETWORK |
	  PUMP_INTFINFO_HAS_BROADCAST);
    intf->set &= i;

    if (strlen(breq->bootfile)) {
	intf->bootFile = strdup(breq->bootfile);
	intf->set |= PUMP_INTFINFO_HAS_BOOTFILE;
    } else {
	intf->set &= ~PUMP_INTFINFO_HAS_BOOTFILE;
    }
    syslog (LOG_DEBUG, "intf: bootFile: %s", intf->bootFile);

    memcpy(&intf->ip, &breq->yiaddr, 4);
    intf->set |= PUMP_INTFINFO_HAS_IP;
    syslog (LOG_DEBUG, "intf: ip: %s", inet_ntoa (intf->ip));

    memcpy(&intf->nextServer, &breq->server_ip, 4);
    intf->set |= PUMP_INTFINFO_HAS_NEXTSERVER;
    syslog (LOG_DEBUG, "intf: next server: %s", inet_ntoa (intf->nextServer));

    chptr = breq->vendor;
    chptr += 4;
    while (*chptr != 0xFF && (void *) chptr < (void *) breq->vendor + DHCP_VENDOR_LENGTH) {
	option = *chptr++;
	if (!option) continue;
	length = *chptr++;

	switch (option) {
	    case BOOTP_OPTION_DNS:
		intf->numDns = 0;
		for (i = 0; i < length; i += 4) {
		    if (intf->numDns < MAX_DNS_SERVERS) {
			memcpy(&intf->dnsServers[intf->numDns++], chptr + i, 4);
			syslog(LOG_DEBUG, "intf: dnsServers[%i]: %s", 
			       i/4, inet_ntoa (intf->dnsServers[i/4]));
		    }
		}
		intf->set |= PUMP_NETINFO_HAS_DNS;
		syslog (LOG_DEBUG, "intf: numDns: %i", intf->numDns);
		break;

	    case BOOTP_OPTION_NETMASK:
		memcpy(&intf->netmask, chptr, 4);
		intf->set |= PUMP_INTFINFO_HAS_NETMASK;
		syslog (LOG_DEBUG, "intf: netmask: %s", inet_ntoa (intf->netmask));
		break;

	    case BOOTP_OPTION_DOMAIN:
		if ((intf->domain = malloc(length + 1))) {
		    memcpy(intf->domain, chptr, length);
		    intf->domain[length] = '\0';
		    intf->set |= PUMP_NETINFO_HAS_DOMAIN;
		    syslog (LOG_DEBUG, "intf: domain: %s", intf->domain);
		}
		break;

	    case BOOTP_OPTION_BROADCAST:
		memcpy(&intf->broadcast, chptr, 4);
		intf->set |= PUMP_INTFINFO_HAS_BROADCAST;
		syslog (LOG_DEBUG, "intf: broadcast: %s", inet_ntoa (intf->broadcast));
		break;

	    case BOOTP_OPTION_GATEWAY:
		memcpy(&intf->gateway, chptr, 4);
		intf->set |= PUMP_NETINFO_HAS_GATEWAY;
		syslog (LOG_DEBUG, "intf: gateway: %s", inet_ntoa (intf->gateway));
		break;

	    case BOOTP_OPTION_HOSTNAME:
		if ((intf->hostname = malloc(length + 1))) {
		    memcpy(intf->hostname, chptr, length);
		    intf->hostname[length] = '\0';
		    intf->set |= PUMP_NETINFO_HAS_HOSTNAME;
		    syslog (LOG_DEBUG, "intf: hostname: %s", intf->hostname);
		}
		break;

	    case BOOTP_OPTION_BOOTFILE:
		/* we ignore this right now */
		break;

	    case DHCP_OPTION_LOGSRVS:
		intf->numLog = 0;
		for (i = 0; i < length; i += 4) {
		    if (intf->numLog < MAX_LOG_SERVERS) {
			memcpy(&intf->logServers[intf->numLog++], chptr + i, 4);
			syslog(LOG_DEBUG, "intf: logServers[%i]: %s", 
			       i/4, inet_ntoa (intf->logServers[i/4]));
		    }
		}
		intf->set |= PUMP_NETINFO_HAS_LOGSRVS;
		syslog (LOG_DEBUG, "intf: numLog: %i", intf->numLog);
		break;

	    case DHCP_OPTION_LPRSRVS:
		intf->numLpr = 0;
		for (i = 0; i < length; i += 4) {
		    if (intf->numLpr < MAX_LPR_SERVERS) {
			memcpy(&intf->lprServers[intf->numLpr++], chptr + i, 4);
			syslog(LOG_DEBUG, "intf: lprServers[%i]: %s", 
			       i/4, inet_ntoa (intf->lprServers[i/4]));
		    }
		}
		intf->set |= PUMP_NETINFO_HAS_LPRSRVS;
		syslog (LOG_DEBUG, "intf: numLpr: %i", intf->numLpr);
		break;

	    case DHCP_OPTION_NTPSRVS:
		intf->numNtp = 0;
		for (i = 0; i < length; i += 4) {
		    if (intf->numNtp < MAX_NTP_SERVERS) {
			memcpy(&intf->ntpServers[intf->numNtp++], chptr + i, 4);
			syslog(LOG_DEBUG, "intf: ntpServers[%i]: %s", 
			       i/4, inet_ntoa (intf->ntpServers[i/4]));
		    }
		}
		intf->set |= PUMP_NETINFO_HAS_NTPSRVS;
		syslog (LOG_DEBUG, "intf: numNtp: %i", intf->numNtp);
		break;

	    case DHCP_OPTION_XFNTSRVS:
		intf->numXfs = 0;
		for (i = 0; i < length; i += 4) {
		    if (intf->numXfs < MAX_XFS_SERVERS) {
			memcpy(&intf->xfntServers[intf->numXfs++], chptr + i, 4);
			syslog(LOG_DEBUG, "intf: xfntServers[%i]: %s", 
			       i/4, inet_ntoa (intf->xfntServers[i/4]));
		    }
		}
		intf->set |= PUMP_NETINFO_HAS_XFNTSRVS;
		syslog (LOG_DEBUG, "intf: numXfs: %i", intf->numXfs);
		break;

	    case DHCP_OPTION_XDMSRVS:
		intf->numXdm = 0;
		for (i = 0; i < length; i += 4) {
		    if (intf->numXdm < MAX_XDM_SERVERS) {
			memcpy(&intf->xdmServers[intf->numXdm++], chptr + i, 4);
			syslog(LOG_DEBUG, "intf: xdmServers[%i]: %s", 
			       i/4, inet_ntoa (intf->xdmServers[i/4]));
		    }
		}
		intf->set |= PUMP_NETINFO_HAS_XDMSRVS;
		syslog (LOG_DEBUG, "intf: numXdm: %i", intf->numXdm);
		break;

	    case DHCP_OPTION_OVERLOAD:
		/* FIXME: we should pay attention to this */
		break;
	}

	chptr += length;
    }

    setMissingIpInfo(intf);
}

static void initVendorCodes(struct bootpRequest * breq) {
    memcpy(breq->vendor, vendCookie, sizeof(vendCookie));
}

static char * prepareRequest(struct bootpRequest * breq,
			     int sock, char * device, time_t startTime) {
    struct ifreq req;
    int i;

    memset(breq, 0, sizeof(*breq));

    breq->opcode = BOOTP_OPCODE_REQUEST;

    strcpy(req.ifr_name, device);
    if (ioctl(sock, SIOCGIFHWADDR, &req))
	return perrorstr("SIOCSIFHWADDR");

    breq->hw = req.ifr_hwaddr.sa_family;
    breq->hwlength = IFHWADDRLEN;	
    memcpy(breq->hwaddr, req.ifr_hwaddr.sa_data, IFHWADDRLEN);

    /* we should use something random here, but I don't want to start using
       stuff from the math library */
    breq->id = pumpUptime();
    for (i = 0; i < IFHWADDRLEN; i++)
	breq->id ^= breq->hwaddr[i] << (8 * (i % 4));

    breq->hopcount = 0;
    breq->secs = pumpUptime() - startTime;

    initVendorCodes(breq);

    return NULL;
}

static unsigned int verifyChecksum(void * buf, int length, void * buf2,
				   int length2) {
    unsigned int csum;
    unsigned short * sp;

    csum = 0;
    for (sp = (unsigned short *) buf; length > 0; (length -= 2), sp++)
	csum += *sp;

    /* this matches rfc 1071, but not Steven's */
    if (length)
	csum += *((unsigned char *) sp);

    for (sp = (unsigned short *) buf2; length2 > 0; (length2 -= 2), sp++)
	csum += *sp;

    /* this matches rfc 1071, but not Steven's */
    if (length)
	csum += *((unsigned char *) sp);

    while (csum >> 16)
	csum = (csum & 0xffff) + (csum >> 16);

    if (csum!=0x0000 && csum != 0xffff) return 0; else return 1;
}

void debugbootpRequest(char *name, struct bootpRequest *breq)  {
    char vendor[28], vendor2[28];
    int i;
    struct in_addr address;
    unsigned char *vndptr;
    unsigned char option, length;
    
    syslog (LOG_DEBUG, "%s: opcode: %i", name, breq->opcode);
    syslog (LOG_DEBUG, "%s: hw: %i", name, breq->hw);
    syslog (LOG_DEBUG, "%s: hwlength: %i", name, breq->hwlength);
    syslog (LOG_DEBUG, "%s: hopcount: %i", name, breq->hopcount);
    syslog (LOG_DEBUG, "%s: id: 0x%08x", name, breq->id);
    syslog (LOG_DEBUG, "%s: secs: %i", name, breq->secs);
    syslog (LOG_DEBUG, "%s: flags: 0x%04x", name, breq->flags);
    
    address.s_addr = breq->ciaddr;
    syslog (LOG_DEBUG, "%s: ciaddr: %s", name, inet_ntoa (address));
    
    address.s_addr = breq->yiaddr;
    syslog (LOG_DEBUG, "%s: yiaddr: %s", name, inet_ntoa (address));
    
    address.s_addr = breq->server_ip;
    syslog (LOG_DEBUG, "%s: server_ip: %s", name, inet_ntoa (address));
    
    address.s_addr = breq->bootp_gw_ip;
    syslog (LOG_DEBUG, "%s: bootp_gw_ip: %s", name, inet_ntoa (address));
    
    syslog (LOG_DEBUG, "%s: hwaddr: %s", name, breq->hwaddr);
    syslog (LOG_DEBUG, "%s: servername: %s", name, breq->servername);
    syslog (LOG_DEBUG, "%s: bootfile: %s", name, breq->bootfile);
    
    vndptr = breq->vendor;
    sprintf (vendor, "0x%02x 0x%02x 0x%02x 0x%02x", *vndptr++, *vndptr++, *vndptr++, *vndptr++);
    syslog (LOG_DEBUG, "%s: vendor: %s", name, vendor);
    
    
    for (; (void *) vndptr < (void *) breq->vendor + DHCP_VENDOR_LENGTH;)
      {
	option = *vndptr++;
	if (option == 0xFF)
	  {
	    sprintf (vendor, "0x%02x", option);
	    vndptr = breq->vendor + DHCP_VENDOR_LENGTH;
	  }
	else if (option == 0x00)
	  {
	    for (i = 1; *vndptr == 0x00; i++, vndptr++);
	    sprintf (vendor, "0x%02x x %i", option, i);
	  }
	else
	  {
	    length = *vndptr++;
	    sprintf (vendor, "%3u %3u", option, length);
	    for (i = 0; i < length; i++)
	      {
		if (strlen (vendor) > 22)
		  {
		    syslog (LOG_DEBUG, "%s: vendor: %s", name, vendor);
		    strcpy (vendor, "++++++");
		  }
		snprintf (vendor2, 27, "%s 0x%02x", vendor, *vndptr++);
		strcpy (vendor, vendor2);
		
	      }
	  }
	
	syslog (LOG_DEBUG, "%s: vendor: %s", name, vendor);
      }

    return;

}

static char * handleTransaction(int s, struct pumpOverrideInfo * override, 
				struct bootpRequest * breq,
			        struct bootpRequest * bresp, 
			        struct sockaddr_in * serverAddr,
				struct sockaddr_in * respondant,
				int useBootpPacket, int dhcpResponseType) {
    struct timeval tv;
    fd_set readfs;
    int i, j;
    struct sockaddr_pkt tmpAddress;
    int gotit = 0;
    int tries;
    int nextTimeout = 2;
    time_t timeoutTime;
    int sin;
    int resend = 1;
    struct ethhdr;
    char ethPacket[ETH_FRAME_LEN];
    struct iphdr * ipHdr;
    struct udphdr * udpHdr;
    struct psuedohUdpHeader pHdr;
    time_t start = pumpUptime();
    
    debugbootpRequest("breq", breq);

    if (!override) {
	override = alloca(sizeof(*override));
	pumpInitOverride(override);
    }

    tries = override->numRetries + 1;

    sin = socket(AF_PACKET, SOCK_DGRAM, ntohs(ETH_P_IP));
    if (sin < 0) {
	return strerror(errno);
    }

    while (!gotit && tries) {
	i = sizeof(*breq);
	if (useBootpPacket)
	    i -= (DHCP_VENDOR_LENGTH - BOOTP_VENDOR_LENGTH);

	if (resend) {
	    if (sendto(s, breq, i, 0, (struct sockaddr *) serverAddr, 
		       sizeof(*serverAddr)) != i) {
		close(sin);
		return perrorstr("sendto");
	    }

	    tries--;
	    nextTimeout *= 2;

	    switch (pumpUptime() & 4) {
		case 0:	if (nextTimeout >= 2) nextTimeout--; break;
		case 1:	nextTimeout++; break;
	    }

	    timeoutTime = pumpUptime() + nextTimeout;
	    i = override->timeout + start;
	    if (timeoutTime > i) timeoutTime = i;

	    resend = 0;
	}

	if (dhcpResponseType == NORESPONSE) {
	    close(sin);
	    return NULL;
	}

	tv.tv_usec = 0;
 	tv.tv_sec = timeoutTime - pumpUptime();
	if (timeoutTime < pumpUptime()) {
	    tries = 0;
	    continue;
	}

	FD_ZERO(&readfs);
	FD_SET(sin, &readfs);
	switch ((select(sin + 1, &readfs, NULL, NULL, &tv))) {
	  case 0:
	    resend = 1;
	    break;

	  case 1:
	    i = sizeof(tmpAddress);
	    if ((j = recvfrom(sin, ethPacket, sizeof(ethPacket), 0, 
		     (struct sockaddr *) &tmpAddress, &i)) < 0)
		return perrorstr("recvfrom");

	    /* We need to do some basic sanity checking of the header */
	    if (j < (sizeof(*ipHdr) + sizeof(*udpHdr))) continue;

	    ipHdr = (void *) ethPacket;
	    if (!verifyChecksum(NULL, 0, ipHdr, sizeof(*ipHdr)))
		continue;

	    if (ntohs(ipHdr->tot_len) > j)
		continue;
	    j = ntohs(ipHdr->tot_len);

	    if (ipHdr->protocol != IPPROTO_UDP) continue;

	    udpHdr = (void *) (ethPacket + sizeof(*ipHdr));
	    pHdr.source = ipHdr->saddr;
	    pHdr.dest = ipHdr->daddr;
	    pHdr.zero = 0;
	    pHdr.protocol = ipHdr->protocol;
	    pHdr.len = udpHdr->len;

/*
	    egcs bugs make this problematic

	    if (udpHdr->check && !verifyChecksum(&pHdr, sizeof(pHdr), 
 				udpHdr, j - sizeof(*ipHdr)))
	    continue;
*/

	    if (ntohs(udpHdr->source) != BOOTP_SERVER_PORT)
		continue;
	    if (ntohs(udpHdr->dest) != BOOTP_CLIENT_PORT) 
		continue;
	    /* Go on with this packet; it looks sane */

	  /* Originally copied sizeof (*bresp) - this is a security
	     problem due to a potential underflow of the source
	     buffer.  Also, it trusted that the packet was properly
	     0xFF terminated, which is not true in the case of the
	     DHCP server on Cisco 800 series ISDN router. */

	  memset (bresp, 0xFF, sizeof (*bresp));
	  memcpy (bresp, (char *) udpHdr + sizeof (*udpHdr), j - sizeof (*ipHdr) - sizeof (*udpHdr));

	    /* sanity checks */
	    if (bresp->id != breq->id) {
		syslog(LOG_DEBUG, "reject: id: 0x%08x <--> 0x%08x",
			breq->id, bresp->id);
		continue;
	    }
	    if (bresp->opcode != BOOTP_OPCODE_REPLY) {
		syslog(LOG_DEBUG, "reject: opcode: %i <--> %i",
			BOOTP_OPCODE_REPLY, bresp->opcode);
		continue;
	    }
	    if (bresp->hwlength != breq->hwlength) {
		syslog(LOG_DEBUG, "reject: hwlength: %i <--> %i",
			breq->hwlength, bresp->hwlength);
		continue;
	    }
	    if (memcmp(bresp->hwaddr, breq->hwaddr, bresp->hwlength)) {
		syslog(LOG_DEBUG, "reject: hwaddr: %s <--> %s",
			breq->hwaddr, bresp->hwaddr);
		continue;
	    }
	    i = dhcpMessageType(bresp);
	    if (!(i == -1 && useBootpPacket) && (i != dhcpResponseType)) {
		syslog(LOG_DEBUG, "reject: msgtyp: %i", i);
		continue;
	    }
	    if (memcmp(bresp->vendor, vendCookie, 4)) {
		syslog(LOG_DEBUG, "reject: vendor: 0x%02x 0x%02x 0x%02x 0x%02x"
				  " <--> 0x%02x 0x%02x 0x%02x 0x%02x",
		       vendCookie[0], vendCookie[1], vendCookie[2], 
		       vendCookie[3], bresp->vendor[0], bresp->vendor[1],
		       bresp->vendor[2], bresp->vendor[3]);
		continue;
	    }
	    /* if (respondant) *respondant = tmpAddress; */
	    gotit = 1;

	    break;

	  default:
	    close(sin);
	    return perrorstr("select");
	}
    }

    if (!gotit) {
	close(sin);
	return _("No DHCP reply received");
    }

    close(sin);

    debugbootpRequest("bresp", bresp);

    return NULL;
}

static void addVendorCode(struct bootpRequest * breq, unsigned char option,
			  unsigned char length, void * data) {
    unsigned char * chptr;
    int theOption, theLength;

    chptr = breq->vendor;
    chptr += 4;
    while (*chptr != 0xFF && *chptr != option) {
	theOption = *chptr++;
	if (!theOption) continue;
	theLength = *chptr++;
	chptr += theLength;
    }

    *chptr++ = option;
    *chptr++ = length;
    memcpy(chptr, data, length);
    chptr[length] = 0xff;
}

static int getVendorCode(struct bootpRequest * bresp, unsigned char option,
			  void * data) {
    unsigned char * chptr;
    unsigned int length, theOption;

    chptr = bresp->vendor;
    chptr += 4;
    while (*chptr != 0xFF && *chptr != option) {
	theOption = *chptr++;
	if (!theOption) continue;
	length = *chptr++;
	chptr += length;
    }

    if (*chptr++ == 0xff) return 1;

    length = *chptr++;
    memcpy(data, chptr, length);

    return 0;
}

static int createSocket(void) {
    struct sockaddr_in clientAddr;
    int s;
    int true = 1;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
	return -1;

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &true, sizeof(true))) {
	close(s);
	return -1;
    }

    memset(&clientAddr.sin_addr, 0, sizeof(&clientAddr.sin_addr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(BOOTP_CLIENT_PORT);	/* bootp client */

    if (bind(s, (struct sockaddr *) &clientAddr, sizeof(clientAddr))) {
	close(s); 
	return -1;
    }

    return s;
}

int pumpDhcpRelease(struct pumpNetIntf * intf) {
    struct bootpRequest breq, bresp;
    unsigned char messageType;
    struct sockaddr_in serverAddr;
    char * chptr;
    int s;
    char hostname[1024];

    if (!(intf->set & PUMP_INTFINFO_HAS_LEASE)) {
	pumpDisableInterface(intf->device);
	syslog(LOG_INFO, "disabling interface %s", intf->device);

	return 0;
    }

    if ((s = createSocket()) < 0) return 1;

    if ((chptr = prepareRequest(&breq, s, intf->device, pumpUptime()))) {
	close(s);
	while (1) {
	    pumpDisableInterface(intf->device);
	    return 0;
	}
    }

    messageType = DHCP_TYPE_RELEASE;
    addVendorCode(&breq, DHCP_OPTION_TYPE, 1, &messageType);
    memcpy(&breq.ciaddr, &intf->ip, sizeof(breq.ciaddr));

    /* Dynamic DHCP implementations need the hostname here. */
    if (intf->set & PUMP_NETINFO_HAS_HOSTNAME) {
	addVendorCode(&breq, BOOTP_OPTION_HOSTNAME,
		      strlen(intf->hostname) + 1, intf->hostname);
    } else {
	gethostname(hostname, sizeof(hostname));
	if (strcmp(hostname, "localhost") && 
	    strcmp(hostname, "localhost.localdomain")) {
	    addVendorCode(&breq, BOOTP_OPTION_HOSTNAME, 
			  strlen(hostname) + 1, hostname);
	}
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(BOOTP_SERVER_PORT);	/* bootp server */
    serverAddr.sin_addr = intf->bootServer;

    if (!handleTransaction(s, NULL, &breq, &bresp, &serverAddr, NULL, 0,
			   NORESPONSE)) {
	pumpDisableInterface(intf->device);
	close(s);
	return 0;
    }

    pumpDisableInterface(intf->device);
    close(s);

    if (intf->set & PUMP_NETINFO_HAS_HOSTNAME)
	free(intf->hostname);
    if (intf->set & PUMP_NETINFO_HAS_DOMAIN)
	free(intf->domain);

    syslog(LOG_INFO, "disabling interface %s", intf->device);

    return 1;
}
    
/* This is somewhat broken. We try only to renew the lease. If we fail,
   we don't try to completely rebind. This doesn't follow the DHCP spec,
   but for the install it should be a reasonable compromise. */
int pumpDhcpRenew(struct pumpNetIntf * intf) {
    struct bootpRequest breq, bresp;
    unsigned char messageType;
    struct sockaddr_in serverAddr;
    char * chptr;
    int s;
    int i;
    char hostname[1024];

    s = createSocket();

    if ((chptr = prepareRequest(&breq, s, intf->device, pumpUptime()))) {
	close(s);
	while (1);	/* problem */
    }

    messageType = DHCP_TYPE_REQUEST;
    addVendorCode(&breq, DHCP_OPTION_TYPE, 1, &messageType);
    memcpy(&breq.ciaddr, &intf->ip, sizeof(breq.ciaddr));

    /* Dynamic DHCP implementations need the hostname here. */
    if (intf->set & PUMP_NETINFO_HAS_HOSTNAME) {
	addVendorCode(&breq, BOOTP_OPTION_HOSTNAME,
		      strlen(intf->hostname) + 1,
		      intf->hostname);
    } else {
	gethostname(hostname, sizeof(hostname));
	if (strcmp(hostname, "localhost") && 
	    strcmp(hostname, "localhost.localdomain")) {
	    addVendorCode(&breq, BOOTP_OPTION_HOSTNAME, 
			  strlen(hostname) + 1, hostname);
	}
    }

    i = htonl(intf->reqLease);
    addVendorCode(&breq, DHCP_OPTION_LEASE, 4, &i);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(BOOTP_SERVER_PORT);	/* bootp server */
    serverAddr.sin_addr = intf->bootServer;

    if (handleTransaction(s, NULL, &breq, &bresp, &serverAddr, NULL, 0,
			   DHCP_TYPE_ACK)) {
	close(s);
	return 1;
    }

    parseLease(&bresp, intf);

    syslog(LOG_INFO, "renewed lease for interface %s", intf->device);

    close(s);
    return 0;
}

static void parseLease(struct bootpRequest * bresp, struct pumpNetIntf * intf) {
    int lease;
    time_t now;

    intf->set &= ~PUMP_INTFINFO_HAS_LEASE;
    if (getVendorCode(bresp, DHCP_OPTION_LEASE, &lease)) 
	return;

    lease = ntohl(lease);

    if (lease && lease != 0xffffffff) {
	now = pumpUptime();
	intf->set |= PUMP_INTFINFO_HAS_LEASE;
	intf->leaseExpiration = now + lease;
	intf->renewAt = now + (7 * lease / 8);
    }
}

char * pumpDhcpRun(char * device, int flags, int reqLease,
		   char * reqHostname, struct pumpNetIntf * intf,
		   struct pumpOverrideInfo * override) {
    int s, i;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    struct sockaddr_in broadcastAddr;
    struct bootpRequest breq, bresp;
    struct bootpRequest protoReq;
    unsigned char * chptr;
    unsigned char messageType;
    time_t startTime = pumpUptime();
    int true = 1;
    char optionsRequested[50];
    int numOptions;
    short aShort;

    memset(intf, 0, sizeof(*intf));
    strcpy(intf->device, device);
    intf->reqLease = reqLease;
    intf->set |= PUMP_INTFINFO_HAS_REQLEASE;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	return perrorstr("socket");
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &true, sizeof(true))) {
	close(s);
	return perrorstr("setsockopt");
    }

    if (flags & PUMP_FLAG_NOCONFIG) {
	if ((chptr = getInterfaceInfo(intf, s))) {
	    close(s);
 	    return chptr;
	}
    } else if ((chptr = pumpPrepareInterface(intf, s))) {
	close(s);
	return chptr;
    }

    if ((chptr = prepareRequest(&breq, s, intf->device, startTime))) {
	close(s);
	pumpDisableInterface(intf->device);
	return chptr;
    }

    messageType = DHCP_TYPE_DISCOVER;
    addVendorCode(&breq, DHCP_OPTION_TYPE, 1, &messageType);

    if (reqHostname) {
	syslog(LOG_DEBUG, "HOSTNAME: requesting %s\n", reqHostname);
	addVendorCode(&breq, BOOTP_OPTION_HOSTNAME,
		      strlen(reqHostname) + 1, reqHostname);
    }

    memset(&clientAddr.sin_addr, 0, sizeof(&clientAddr.sin_addr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(BOOTP_CLIENT_PORT);	/* bootp client */

    if (bind(s, (struct sockaddr *) &clientAddr, sizeof(clientAddr))) {
	pumpDisableInterface(intf->device);
	close(s);
	return perrorstr("bind");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(BOOTP_SERVER_PORT);	/* bootp server */

#if 0
    /* seems like a good idea?? */
    if (intf->set & PUMP_INTFINFO_HAS_BOOTSERVER)
	serverAddr.sin_addr = intf->bootServer;
#endif 

    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(BOOTP_SERVER_PORT);

#if 0
    /* this too! */
    if (intf->set & PUMP_INTFINFO_HAS_BROADCAST)
	broadcastAddr.sin_addr = intf->broadcast;
#endif

    memset(&broadcastAddr.sin_addr, 0xff, 
	   sizeof(broadcastAddr.sin_addr));  /* all 1's broadcast */

    syslog (LOG_DEBUG, "PUMP: sending discover\n");

    if ((chptr = handleTransaction(s, override, &breq, &bresp, &broadcastAddr,
				   NULL, 1, DHCP_TYPE_OFFER))) {
	close(s);
	pumpDisableInterface(intf->device);
	return chptr;
    }

    /* Otherwise we're in the land of bootp */
    if (dhcpMessageType(&bresp) == DHCP_TYPE_OFFER) {
	/* Admittedly, this seems a bit odd. If we find a dhcp server, we
	   rerun the dhcp discover broadcast, but with the proper option
	   field this time. This makes me rfc compliant. */
	syslog (LOG_DEBUG, "got dhcp offer\n");

	initVendorCodes(&breq);

	aShort = ntohs(sizeof(struct bootpRequest));
	addVendorCode(&breq, DHCP_OPTION_MAXSIZE, 2, &aShort);

	numOptions = 0;
	optionsRequested[numOptions++] = BOOTP_OPTION_NETMASK;
	optionsRequested[numOptions++] = BOOTP_OPTION_GATEWAY;
	optionsRequested[numOptions++] = BOOTP_OPTION_DNS;
	optionsRequested[numOptions++] = BOOTP_OPTION_DOMAIN;
	optionsRequested[numOptions++] = BOOTP_OPTION_BROADCAST;
	optionsRequested[numOptions++] = BOOTP_OPTION_HOSTNAME;
	optionsRequested[numOptions++] = DHCP_OPTION_LOGSRVS;
	optionsRequested[numOptions++] = DHCP_OPTION_LPRSRVS;
	optionsRequested[numOptions++] = DHCP_OPTION_NTPSRVS;
	optionsRequested[numOptions++] = DHCP_OPTION_XFNTSRVS;
	optionsRequested[numOptions++] = DHCP_OPTION_XDMSRVS;
	addVendorCode(&breq, DHCP_OPTION_OPTIONREQ, numOptions, 
		      optionsRequested);

	if (!reqHostname) {
	    reqHostname = alloca(200);
	    gethostname(reqHostname, 200);
	    if (!strcmp(reqHostname, "localhost") ||
		!strcmp(reqHostname, "localhost.localdomain"))
	        reqHostname = NULL;
	}

	if (reqHostname) {
	    addVendorCode(&breq, BOOTP_OPTION_HOSTNAME,
			  strlen(reqHostname) + 1, reqHostname);
	}

	i = htonl(intf->reqLease);
	addVendorCode(&breq, DHCP_OPTION_LEASE, 4, &i);

	protoReq = breq;

	syslog (LOG_DEBUG, "PUMP: sending second discover");

	messageType = DHCP_TYPE_DISCOVER;
	addVendorCode(&breq, DHCP_OPTION_TYPE, 1, &messageType);

	/* Send another DHCP_REQUEST with the proper option list */
	if ((chptr = handleTransaction(s, override, &breq, &bresp, 
				       &broadcastAddr, NULL, 0, 
				       DHCP_TYPE_OFFER))) {
	    close(s);
	    pumpDisableInterface(intf->device);
	    return chptr;
	}


	if (dhcpMessageType(&bresp) != DHCP_TYPE_OFFER) {
	    close(s);
	    pumpDisableInterface(intf->device);
	    return "dhcp offer expected";
	}

	syslog (LOG_DEBUG, "PUMP: got an offer");

	if (getVendorCode(&bresp, DHCP_OPTION_SERVER, &serverAddr.sin_addr)) {
	    syslog (LOG_DEBUG, "DHCPOFFER didn't include server address");
	    intf->bootServer = broadcastAddr.sin_addr;
	}

	breq = protoReq;
	messageType = DHCP_TYPE_REQUEST;
	addVendorCode(&breq, DHCP_OPTION_TYPE, 1, &messageType);

	addVendorCode(&breq, DHCP_OPTION_SERVER, 4, &serverAddr.sin_addr);
	addVendorCode(&breq, DHCP_OPTION_REQADDR, 4, &bresp.yiaddr);

	/* why do we need to use the broadcast address here? better reread the
	   spec! */
	if ((chptr = handleTransaction(s, override, &breq, &bresp, 
				       &broadcastAddr, NULL, 0, 
				       DHCP_TYPE_ACK))) {
	    close(s);
	    pumpDisableInterface(intf->device);
	    return chptr;
	}

	syslog (LOG_DEBUG, "PUMP: got lease");

	parseLease(&bresp, intf);

	if (getVendorCode(&bresp, DHCP_OPTION_SERVER, &intf->bootServer)) {
	    syslog (LOG_DEBUG, "DHCPACK didn't include server address");
	    intf->bootServer = broadcastAddr.sin_addr;
	}

	intf->set |= PUMP_INTFINFO_HAS_BOOTSERVER;
    }

    close(s);

    parseReply(&bresp, intf);
    if (flags & PUMP_FLAG_FORCEHNLOOKUP)
	intf->set &= ~(PUMP_NETINFO_HAS_DOMAIN | PUMP_NETINFO_HAS_HOSTNAME);

    return NULL;
}

void pumpInitOverride(struct pumpOverrideInfo * override) {
    strcpy(override->intf.device, "MASTER");
    override->timeout = DEFAULT_TIMEOUT;
    override->numRetries = DEFAULT_NUM_RETRIES;
    override->script = NULL;
}

/*
* If time(2) is used, changing the date on a system can cause
* pump to miss the lease renewal time.  The result is a system
* that doesn't seem to want to talk until it is rebooted!  We
* need elapsed time, NOT absolute (wall clock) time to measure
* the expiration of the lease.  Unfortunately I can't see any
* other interface to an unmolested elapsed time value other than
* /proc/uptime (and this is how uptime(1), top(1), w(1) use it).
* No matter, overhead is not an issue here, accuracy is.
* duanev@io.com 9/99
*/
time_t pumpUptime() {
    FILE * fd;
    long secs;
    static int first = 1;

    if ((fd = fopen("/proc/uptime", "r")) == NULL) {
	if (first) {
	    syslog (LOG_INFO, "error opening /proc/uptime: %s", 
		    strerror(errno));
	    syslog (LOG_INFO, "warning: might miss lease renewal if date 
		    changes");
	    first = 0;
	}
	return time(NULL);
    }

    fscanf(fd, "%ld", &secs);
    fclose(fd);

    return (time_t)secs;
}

