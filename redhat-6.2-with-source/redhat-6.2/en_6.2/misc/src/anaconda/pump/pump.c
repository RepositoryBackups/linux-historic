/*
 * Copyright 1999 Red Hat Software, Inc.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* This probably doesn't work on anything other then Ethernet! It may work
   on PLIP as well, but ARCnet and Token Ring are unlikely at best. */

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

#include "config.h"
#include "pump.h"

#define N_(foo) (foo)

#define PROGNAME "pump"
#define CONTROLSOCKET "/var/run/pump.sock"

#define _(foo) ((foo))
#include <stdarg.h>

struct command {
    enum { CMD_STARTIFACE, CMD_RESULT, CMD_DIE, CMD_STOPIFACE, 
	   CMD_FORCERENEW, CMD_REQSTATUS, CMD_STATUS } type;
    union {
	struct {
	    char device[20];
	    int flags;
	    int reqLease;			/* in hours */
	    char reqHostname[200];
	} start;
	int result;				/* 0 for success */
	struct {
	    char device[20];
	} stop;
	struct {
	    char device[20];
	} renew;
	struct {
	    char device[20];
	} reqstatus;
	struct {
	    struct pumpNetIntf intf;
	    char hostname[1024];
	    char domain[1024];
	    char bootFile[1024];
	} status;
    } u;
};

static int openControlSocket(char * configFile, struct pumpOverrideInfo * override);

char * readSearchPath(void) {
    int fd;
    struct stat sb;
    char * buf;
    char * start;

    fd = open("/etc/resolv.conf", O_RDONLY);
    if (fd < 0) return NULL;

    fstat(fd, &sb);
    buf = alloca(sb.st_size + 2);
    if (read(fd, buf, sb.st_size) != sb.st_size) return NULL;
    buf[sb.st_size] = '\n';
    buf[sb.st_size + 1] = '\0';
    close(fd);

    start = buf;
    while (start && *start) {
	while (isspace(*start) && (*start != '\n')) start++;
	if (*start == '\n') {
	    start++;
	    continue;
	}

	if (!strncmp("search", start, 6) && isspace(start[6])) {
	    start += 6;
	    while (isspace(*start) && *start != '\n') start++;
	    if (*start == '\n') return NULL;

	    buf = strchr(start, '\n');
	    *buf = '\0';
	    return strdup(start);
	}

	while (*start && (*start != '\n')) start++;
    }

    return NULL;
}

static void createResolvConf(struct pumpNetIntf * intf, char * domain,
			     int isSearchPath) {
    FILE * f;
    int i;
    char * chptr;

    /* force a reread of /etc/resolv.conf if we need it again */
    res_close();

    if (!domain) {
	domain = readSearchPath();
 	if (domain) {
	    chptr = alloca(strlen(domain) + 1);
	    strcpy(chptr, domain);
	    free(domain);
	    domain = chptr;
	    isSearchPath = 1;
	}
    }

    f = fopen("/etc/resolv.conf", "w");
    if (!f) {
	syslog(LOG_ERR, "cannot create /etc/resolv.conf: %s\n",
	       strerror(errno));
	return;
    }

    if (domain && isSearchPath) {
	fprintf(f, "search %s\n", domain);
    } else if (domain) {
	fprintf(f, "search");
	chptr = domain;
	do {
	    /* If there is a single . in the search path, write it out
	       only if the toplevel domain is com, edu, gov, mil, org */
	    if (!strchr(strchr(chptr, '.') + 1, '.')) {
		char * tail = strchr(chptr, '.');
		if (strcmp(tail, ".com") && strcmp(tail, ".edu") &&
		    strcmp(tail, ".gov") && strcmp(tail, ".mil") &&
		    strcmp(tail, ".org") && strcmp(tail, ".int")) break;
	    }

	    fprintf(f, " %s", chptr);
	    chptr = strchr(chptr, '.');
	    if (chptr) {
		chptr++;
		if (!strchr(chptr, '.'))
		    chptr = NULL;
	    }
	} while (chptr);

	fprintf(f, "\n");
    }

    for (i = 0; i < intf->numDns; i++)
	fprintf(f, "nameserver %s\n", inet_ntoa(intf->dnsServers[i]));

    fclose(f);

    /* force a reread of /etc/resolv.conf */
    endhostent();
}

void setupDns(struct pumpNetIntf * intf, struct pumpOverrideInfo * override) {
    char * hn, * dn = NULL;
    struct hostent * he;

    if (override->flags & OVERRIDE_FLAG_NODNS) {
	return;
    }

    if (override->searchPath) {
	createResolvConf(intf, override->searchPath, 1);
	return;
    }

    if (intf->set & PUMP_NETINFO_HAS_DNS) {
	if (!(intf->set & PUMP_NETINFO_HAS_DOMAIN))  {
	    if (intf->set & PUMP_NETINFO_HAS_HOSTNAME) {
		hn = intf->hostname;
	    } else {
		createResolvConf(intf, NULL, 0);

		he = gethostbyaddr((char *) &intf->ip, sizeof(intf->ip),
				   AF_INET);
		if (he) {
		    hn = he->h_name;
		} else {
		    hn = NULL;
		}
	    }

	    if (hn) {
		dn = strchr(hn, '.');
		if (dn)
		    dn++;
	    }
	} else {
	    dn = intf->domain;
	}

	createResolvConf(intf, dn, 0);
    }
}

static void callScript(char* script,int msg,struct pumpNetIntf* intf) {
    pid_t child;
    char * argv[20];
    char ** nextArg;
    char * class, * chptr;

    if (!script) return;

    argv[0] = script;
    argv[2] = intf->device;
    nextArg = argv + 3;

    switch (msg) {
	case PUMP_SCRIPT_NEWLEASE:
	    class = "up";
	    chptr = inet_ntoa(intf->ip);
	    *nextArg = alloca(strlen(chptr) + 1);
	    strcpy(*nextArg, chptr);
	    nextArg++;
	    break;

	case PUMP_SCRIPT_RENEWAL:
	    class = "renewal";
	    chptr = inet_ntoa(intf->ip);
	    *nextArg = alloca(strlen(chptr) + 1);
	    strcpy(*nextArg, chptr);
	    nextArg++;
	    break;

	case PUMP_SCRIPT_DOWN:
	    class = "down";
	    break;
    }

    argv[1] = class;
    *nextArg = NULL;

    if (!(child = fork())) {
	/* send the script to init */
	if (fork()) _exit(0);

	execvp(argv[0], argv);

	syslog(LOG_INFO,"failed to run %s: %s", argv[0], strerror(errno));

	_exit(0);
    }

    waitpid(child, NULL, 0);
}

static void runDaemon(int sock, char * configFile, struct pumpOverrideInfo * overrides) {
    int conn;
    struct sockaddr_un addr;
    int addrLength;
    struct command cmd;
    struct pumpNetIntf intf[20];
    int numInterfaces = 0;
    int i;
    int closest;
    struct timeval tv;
    fd_set fds;
    struct pumpOverrideInfo emptyOverride, * o;

    if (!overrides)
        readPumpConfig(configFile, &overrides);

    if (!overrides) {
	overrides = &emptyOverride;
	overrides->intf.device[0] = '\0';
    }

    while (1) {
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	tv.tv_sec = tv.tv_usec = 0;
	closest = -1;
	if (numInterfaces) {
	    for (i = 0; i < numInterfaces; i++)
		if ((intf[i].set & PUMP_INTFINFO_HAS_LEASE) && 
			(closest == -1 || 
			       (intf[closest].renewAt > intf[i].renewAt)))
		    closest = i;
	    if (closest != -1) {
		tv.tv_sec = intf[closest].renewAt - pumpUptime();
		if (tv.tv_sec <= 0) {
		    if (pumpDhcpRenew(intf + closest)) {
			syslog(LOG_INFO,
				"failed to renew lease for device %s",
				intf[closest].device);

			if ((intf[closest].renewAt += pumpUptime() + 30) >
			    intf[closest].leaseExpiration) {
			    o = overrides;
			    while (*o->intf.device &&
				   strcmp(o->intf.device,cmd.u.start.device)) 
				o++;
			    
			    if (!*o->intf.device) o = overrides;

			    intf[closest].set &= ~PUMP_INTFINFO_HAS_LEASE;

			    if (pumpDhcpRun(intf[closest].device, 0, 
				  intf[closest].reqLease,
				  intf[closest].set & PUMP_NETINFO_HAS_HOSTNAME
				    ? intf[closest].hostname : NULL,
				  intf + closest, o)) {
				if (numInterfaces == 1) {
				    callScript(o->script, PUMP_SCRIPT_DOWN,
					       &intf[closest]);
				    syslog(LOG_INFO,
					    "terminating as there are no "
					    "more devices under management");
					    exit(0);
				}

				intf[i] = intf[numInterfaces - 1];
				numInterfaces--;
			    } else {
				callScript(o->script, PUMP_SCRIPT_NEWLEASE,
					   &intf[closest]);
                            }
			}
		    } else {
			callScript(o->script, PUMP_SCRIPT_RENEWAL,
				   &intf[closest]);
		    }

		    continue;	    /* recheck timeouts */
		}
	    }
	}

	if (select(sock + 1, &fds, NULL, NULL, 
		   closest != -1 ? &tv : NULL) > 0) {
	    conn = accept(sock, &addr, &addrLength);

	    if (read(conn, &cmd, sizeof(cmd)) != sizeof(cmd)) {
		close(conn);
		continue;
	    }

	    switch (cmd.type) {
	      case CMD_DIE:
		for (i = 0; i < numInterfaces; i++) {
		    pumpDhcpRelease(intf + i);
		    callScript(o->script, PUMP_SCRIPT_DOWN, &intf[i]);
		}

		syslog(LOG_INFO, "terminating at root's request");

		cmd.type = CMD_RESULT;
		cmd.u.result = 0;
		write(conn, &cmd, sizeof(cmd));
		exit(0);

	      case CMD_STARTIFACE:
		o = overrides; 
		while (*o->intf.device && 
			strcmp(o->intf.device, cmd.u.start.device)) {
		    o++;
		}
		if (!*o->intf.device) o = overrides;

		if (pumpDhcpRun(cmd.u.start.device,
			        cmd.u.start.flags, cmd.u.start.reqLease, 
			        cmd.u.start.reqHostname[0] ? 
			            cmd.u.start.reqHostname : NULL,
			        intf + numInterfaces, o)) {
		    cmd.u.result = 1;
		} else {
		    pumpSetupInterface(intf + numInterfaces);

		    syslog(LOG_INFO, "configured interface %s", intf->device);

		    if (intf->set & PUMP_NETINFO_HAS_GATEWAY) {
			pumpSetupDefaultGateway(&intf->gateway);
		    }

		    setupDns(intf, o);

		    cmd.u.result = 0;
		    numInterfaces++;
		}

                callScript(o->script, PUMP_SCRIPT_NEWLEASE, intf);
		break;

	      case CMD_FORCERENEW:
		for (i = 0; i < numInterfaces; i++)
		    if (!strcmp(intf[i].device, cmd.u.renew.device)) break;
		if (i == numInterfaces)
		    cmd.u.result = RESULT_UNKNOWNIFACE;
		else {
		    cmd.u.result = pumpDhcpRenew(intf + i);
		    if (!cmd.u.result) {
			callScript(o->script, PUMP_SCRIPT_RENEWAL, intf);
		    }
		}
		break;

	      case CMD_STOPIFACE:
		for (i = 0; i < numInterfaces; i++)
		    if (!strcmp(intf[i].device, cmd.u.stop.device)) break;
		if (i == numInterfaces)
		    cmd.u.result = RESULT_UNKNOWNIFACE;
		else {
		    cmd.u.result = pumpDhcpRelease(intf + i);
		    callScript(o->script, PUMP_SCRIPT_DOWN, &intf[i]);
		    if (numInterfaces == 1) {
			cmd.type = CMD_RESULT;
			write(conn, &cmd, sizeof(cmd));

			syslog(LOG_INFO, "terminating as there are no "
				"more devices under management");

			exit(0);
		    }

		    intf[i] = intf[numInterfaces - 1];
		    numInterfaces--;
		}
		break;

	      case CMD_REQSTATUS:
		for (i = 0; i < numInterfaces; i++)
		    if (!strcmp(intf[i].device, cmd.u.stop.device)) break;
		if (i == numInterfaces) {
		    cmd.u.result = RESULT_UNKNOWNIFACE;
		} else {
		    cmd.type = CMD_STATUS;
		    cmd.u.status.intf = intf[i];
		    if (intf[i].set & PUMP_NETINFO_HAS_HOSTNAME)
			strncpy(cmd.u.status.hostname,
			    intf->hostname, sizeof(cmd.u.status.hostname));
		    cmd.u.status.hostname[sizeof(cmd.u.status.hostname)] = '\0';

		    if (intf[i].set & PUMP_NETINFO_HAS_DOMAIN)
			strncpy(cmd.u.status.domain,
			    intf->domain, sizeof(cmd.u.status.domain));
		    cmd.u.status.domain[sizeof(cmd.u.status.domain) - 1] = '\0';

		    if (intf[i].set & PUMP_INTFINFO_HAS_BOOTFILE)
			strncpy(cmd.u.status.bootFile,
			    intf->bootFile, sizeof(cmd.u.status.bootFile));
		    cmd.u.status.bootFile[sizeof(cmd.u.status.bootFile) - 1] = 
		    							'\0';
		}

	      case CMD_STATUS:
	      case CMD_RESULT:
		/* can't happen */
		break;
	    }

	    if (cmd.type != CMD_STATUS) cmd.type = CMD_RESULT;
	    write(conn, &cmd, sizeof(cmd));

	    close(conn);
	}
    }

    exit(0);
}

static int openControlSocket(char * configFile, struct pumpOverrideInfo * override) {
    struct sockaddr_un addr;
    int sock;
    size_t addrLength;
    pid_t child;
    int status;

    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
	return -1;

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, CONTROLSOCKET);
    addrLength = sizeof(addr.sun_family) + strlen(addr.sun_path);

    if (!connect(sock, (struct sockaddr *) &addr, addrLength)) 
	return sock;

    if (errno != ENOENT && errno != ECONNREFUSED) {
	fprintf(stderr, "failed to connect to %s: %s\n", CONTROLSOCKET,
		strerror(errno));
	close(sock);
	return -1;
    }

    if (!(child = fork())) {
	close(sock);

	close(0);
	close(1);
	close(2);

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
	    syslog(LOG_ERR, "failed to create socket: %s\n", strerror(errno));
	    exit(1);
	}

	unlink(CONTROLSOCKET);
	umask(077);
	if (bind(sock, (struct sockaddr *) &addr, addrLength)) {
	    syslog(LOG_ERR, "bind to %s failed: %s\n", CONTROLSOCKET,
		    strerror(errno));
	    exit(1);
	}
	umask(033);

	listen(sock, 5);

	if (fork()) _exit(0);

	openlog("pumpd", LOG_PID, LOG_DAEMON);
	{
	    time_t now,upt;
	    int updays,uphours,upmins,upsecs;

	    now = time(NULL);
	    upt = pumpUptime();
	    if (now <= upt)
		syslog(LOG_INFO, "starting at %s\n", ctime(&now));
	    else {
		upsecs = upt % 60;
		upmins = (upt / 60) % 60;
		uphours = (upt / 3600) % 24;
		updays = upt / 86400;
		syslog(LOG_INFO, "starting at (uptime %d days, %d:%02d:%02d) %s\n", updays, uphours, upmins, upsecs, ctime(&now));
	    }
	}

	runDaemon(sock, configFile, override);
    }

    waitpid(child, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status))
	return -1;

    if (!connect(sock, (struct sockaddr *) &addr, addrLength)) 
	return sock;

    fprintf(stderr, "failed to connect to %s: %s\n", CONTROLSOCKET,
	    strerror(errno));

    return 0;
}

void printStatus(struct pumpNetIntf i, char * hostname, char * domain,
		 char * bootFile) {
    int j;
    time_t now,upnow,localAt,localExpiration;

    printf("Device %s\n", i.device);
    printf("\tIP: %s\n", inet_ntoa(i.ip));
    printf("\tNetmask: %s\n", inet_ntoa(i.netmask));
    printf("\tBroadcast: %s\n", inet_ntoa(i.broadcast));
    printf("\tNetwork: %s\n", inet_ntoa(i.network));
    printf("\tBoot server %s\n", inet_ntoa(i.bootServer));
    printf("\tNext server %s\n", inet_ntoa(i.nextServer));

    if (i.set & PUMP_NETINFO_HAS_GATEWAY)
	printf("\tGateway: %s\n", inet_ntoa(i.gateway));

    if (i.set & PUMP_INTFINFO_HAS_BOOTFILE)
	printf("\tBoot file: %s\n", bootFile);

    if (i.set & PUMP_NETINFO_HAS_HOSTNAME)
	printf("\tHostname: %s\n", hostname);

    if (i.set & PUMP_NETINFO_HAS_DOMAIN)
	printf("\tDomain: %s\n", domain);

    if (i.numDns) {
	printf("\tNameservers:");
	for (j = 0; j < i.numDns; j++)
	    printf(" %s", inet_ntoa(i.dnsServers[j]));
	printf("\n");
    }

    if (i.numLog) {
	printf("\tLogservers:");
	for (j = 0; j < i.numLog; j++)
	    printf(" %s", inet_ntoa(i.logServers[j]));
	printf("\n");
    }
 
    if (i.numLpr) {
	printf("\tLprservers:");
	for (j = 0; j < i.numLpr; j++)
	    printf(" %s", inet_ntoa(i.lprServers[j]));
	printf("\n");
    }

    if (i.numNtp) {
	printf("\tNtpservers:");
	for (j = 0; j < i.numNtp; j++)
	    printf(" %s", inet_ntoa(i.ntpServers[j]));
	printf("\n");
    }

    if (i.numXfs) {
	printf("\tXfontservers:");
	for (j = 0; j < i.numXfs; j++)
	    printf(" %s", inet_ntoa(i.xfntServers[j]));
	printf("\n");
    }

    if (i.numXdm) {
	printf("\tXdmservers:");
	for (j = 0; j < i.numXdm; j++)
	    printf(" %s", inet_ntoa(i.xdmServers[j]));
	printf("\n");
    }

    if (i.set & PUMP_INTFINFO_HAS_LEASE) {
	upnow = pumpUptime();
	tzset();
	now = time(NULL);
	localAt = now + (i.renewAt - upnow);
	localExpiration = now + (i.leaseExpiration - upnow);
	printf("\tRenewal time: %s", ctime(&localAt)); 
	printf("\tExpiration time: %s", ctime(&localExpiration)); 
    }
}

int main (int argc, const char ** argv) {
    char * device = "eth0";
    char * hostname = "";
    poptContext optCon;
    int rc;
    int test = 0;
    int flags = 0;
    int lease = 12;
    int killDaemon = 0;
    int release = 0, renew = 0, status = 0, lookupHostname = 0, nodns = 0;
    struct command cmd, response;
    char * configFile = "/etc/pump.conf";
    struct pumpOverrideInfo * overrides;
    int cont;
    struct poptOption options[] = {
	    { "config-file", 'c', POPT_ARG_STRING, &configFile, 0,
			N_("Configuration file to use instead of "
			   "/etc/pump.conf") },
            { "hostname", 'h', POPT_ARG_STRING, &hostname, 0, 
			N_("Hostname to request"), N_("hostname") },
            { "interface", 'i', POPT_ARG_STRING, &device, 0, 
			N_("Interface to configure (normally eth0)"), 
			N_("iface") },
	    { "kill", 'k', POPT_ARG_NONE, &killDaemon, 0,
			N_("Kill daemon (and disable all interfaces)"), NULL },
	    { "lease", 'l', POPT_ARG_INT, &lease, 0,
			N_("Lease time to request (in hours)"), N_("hours") },
	    { "lookup-hostname", '\0', POPT_ARG_NONE, &lookupHostname, 0,
			N_("Force lookup of hostname") },
	    { "release", 'r', POPT_ARG_NONE, &release, 0,
			N_("Release interface"), NULL },
	    { "renew", 'R', POPT_ARG_NONE, &renew, 0,
			N_("Force immediate lease renewal"), NULL },
	    { "status", 's', POPT_ARG_NONE, &status, 0,
			N_("Display interface status"), NULL },
	    { "no-dns", 'd', POPT_ARG_NONE, &nodns, 0,
			N_("Don't update resolv.conf"), NULL },
	    /*{ "test", 't', POPT_ARG_NONE, &test, 0,
			N_("Don't change the interface configuration or "
			   "run as a deamon.") },*/
	    POPT_AUTOHELP
	    { NULL, '\0', 0, NULL, 0 }
        };

    optCon = poptGetContext(PROGNAME, argc, argv, options,0);
    poptReadDefaultConfig(optCon, 1);

    if ((rc = poptGetNextOpt(optCon)) < -1) {
	fprintf(stderr, _("%s: bad argument %s: %s\n"), PROGNAME,
		poptBadOption(optCon, POPT_BADOPTION_NOALIAS), 
		poptStrerror(rc));
	return 1;
    }

    if (poptGetArg(optCon)) {
	fprintf(stderr, _("%s: no extra parameters are expected\n"), PROGNAME);
	return 1;
    }

    /* make sure the config file is parseable before going on any further */
    if (readPumpConfig(configFile, &overrides)) return 1;

    if (geteuid()) {
	fprintf(stderr, _("%s: must be run as root\n"), PROGNAME);
	exit(1);
    }

    if (test)
	flags = PUMP_FLAG_NODAEMON | PUMP_FLAG_NOCONFIG;
    if (lookupHostname)
	flags |= PUMP_FLAG_FORCEHNLOOKUP;
    if (nodns)
	overrides->flags |= OVERRIDE_FLAG_NODNS;

    cont = openControlSocket(configFile, overrides);
    if (cont < 0) 
	exit(1);

    if (killDaemon) {
	cmd.type = CMD_DIE;
    } else if (status) {
	cmd.type = CMD_REQSTATUS;
	strcpy(cmd.u.reqstatus.device, device);
    } else if (renew) {
	cmd.type = CMD_FORCERENEW;
	strcpy(cmd.u.renew.device, device);
    } else if (release) {
	cmd.type = CMD_STOPIFACE;
	strcpy(cmd.u.stop.device, device);
    } else {
	cmd.type = CMD_STARTIFACE;
	strcpy(cmd.u.start.device, device);
	cmd.u.start.flags = flags;
	cmd.u.start.reqLease = lease * 60 * 60;
	strcpy(cmd.u.start.reqHostname, hostname);
    }

    write(cont, &cmd, sizeof(cmd));
    read(cont, &response, sizeof(response));

    if (response.type == CMD_RESULT && response.u.result &&
	    cmd.type == CMD_STARTIFACE) {
	cont = openControlSocket(configFile, overrides);
	if (cont < 0) 
	    exit(1);
	write(cont, &cmd, sizeof(cmd));
	read(cont, &response, sizeof(response));
    }

    if (response.type == CMD_RESULT) {
	if (response.u.result) {
	    fprintf(stderr, "Operation failed.\n");
	    return 1;
	}
    } else if (response.type == CMD_STATUS) {
	printStatus(response.u.status.intf, response.u.status.hostname, 
		    response.u.status.domain, response.u.status.bootFile);
    }

    return 0;
}