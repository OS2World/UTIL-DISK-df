/*
 * File: df.c
 *
 * Unix-like free disk space utility for OS/2
 *
 * Bob Eager   April 2002
 *
 */

/*
 * History:
 *
 *	1.0	- Initial version.
 *	1.1	- Fixed for volumes over 4GB.
 *	1.2	- Fixed failure to check for error when querying file system.
 *
 */

#pragma	strings(readonly)

/* Program version information */

#define	VERSION		1
#define	EDIT		2

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define	INCL_DOSFILEMGR
#include <os2.h>

/* Definitions */

#define	DIGLEN		14		/* Length of string for long numbers */
#define	FSQBUFSIZE	100		/* Size of FS query buffer */

/* Forward references */

/* Local storage */

static	PUCHAR	progname;		/* Name of program, as a string */

/* Forward references */

static	VOID	display(INT);
static	VOID	error(PUCHAR, ...);
static	ULONG	getdrives(BOOL);
static	VOID	putusage(VOID);

/* Help text */

static	const	PUCHAR helpinfo[] = {
"%s: display disk free space",
"Synopsis: %s [options] [volume] ...",
" Options:",
"    -h  display help",
" ",
"If no volumes are specified, all non-removable volumes are displayed;",
"one or more volumes may be specified, as 'd:' or just 'd'.",
" ",
"Examples:  %s",
"           %s a:",
""
};


/*
 * Main program
 *
 */

INT main(INT argc, PUCHAR argv[])
{	PUCHAR p;
	INT q = 1;			/* First real arg index */
	INT drive;
	ULONG drivemap;

	/* Derive program name for use in messages */

	progname = strrchr(argv[0], '\\');
	if(progname != (PUCHAR) NULL)
		progname++;
	else
		progname = argv[0];
	p = strchr(progname, '.');
	if(p != (PUCHAR) NULL) *p = '\0';
	strlwr(progname);

	/* Process input options */

	while(--argc > 0 && (*++argv)[0] == '-') {
		for(p = argv[0]+1; *p != '\0'; p++) {
			switch(*p) {
				case 'h':	/* -h: print help */
					putusage();
					exit(EXIT_SUCCESS);
					break;

				default:
					fprintf(stderr,
					"%s: invalid flag -%c\n", progname, *p);
					exit(EXIT_FAILURE);
			}
		}
	}

	if(argc == 0) {			/* Default to all drives */
		drivemap = getdrives(FALSE);
	} else {
		/* Cycle through args and build drive map */

		ULONG drivebit;
		ULONG alldrives = getdrives(TRUE);
		INT i, len;

		drivemap = 0;

		for(i = 0; i < argc; i++) {
			p = argv[i];
			len = strlen(p);
			if((len > 2) ||
                           (len == 2 && p[1] != ':') ||
                           !isalpha(p[0])) {
				putusage();
				exit(EXIT_FAILURE);
			}
			drive = (INT) toupper(p[0]);
			drivebit = (ULONG) (1L << (drive - 'A'));
			if((drivebit & alldrives) == 0) {
				error(
				"%s: invalid volume '%c:'",
				progname,
				p[0]);
				exit(EXIT_FAILURE);
			}
			drivemap |= drivebit;
		}
	}

	fprintf(
		stdout,
		"vol  kbytes     used       avail      alloc"
		"   capacity     volume     type\n");
	for(drive = ('A'-'A'); drive <= ('Z'-'A'); drive++) {
		if(drivemap & (1L << drive))
			display(drive+1);
	}

	return(EXIT_SUCCESS);
}


/*
 * Get bitmap of drives to process.
 * If 'all' is TRUE, returns map of all available drives.
 * Otherwise, eliminates drives with removable media.
 *
 */

static ULONG getdrives(BOOL all)
{	UINT drive;
	APIRET rc;
	ULONG curdrive;
	ULONG drivemap, drivebit;
	UCHAR parmlist[2];
	UCHAR dataarea;
	ULONG parmlength = 8;
	ULONG datalength = 4;
	PULONG parmlengthio = &parmlength;
	PULONG datalengthio = &datalength;

	rc = DosQueryCurrentDisk(&curdrive, &drivemap);
	if(rc != 0) return(0);
	if(all) return(drivemap);

	for(drive = ('A'-'A'); drive <= ('Z'-'A'); drive++) {
		drivebit = drivemap & (1L << drive);
		if(drivebit) {
			parmlist[0] = 0;
			parmlist[1] = (UCHAR) drive;
			dataarea = 0;

			/* See if medium removable */

			rc = (APIRET) DosDevIOCtl(
				(HFILE) -1,
				IOCTL_DISK,
				DSK_BLOCKREMOVABLE,
				(PVOID)parmlist,
				parmlength,
				parmlengthio,
				(PVOID)&dataarea,
				datalength,
				datalengthio);

			if(rc != 0) continue;

			if(dataarea == 0)
				drivemap &= ~drivebit;	/* remove from map */
		}
	}
	return(drivemap);
}


/*
 * Display information for specified drive
 *
 */

static void display(INT drive)
{	ULONG cs;
	ULONG total;
	ULONG avail;
	ULONG fsqbuflen = FSQBUFSIZE;
	PUCHAR p;
	UCHAR drv[3];
	UCHAR fsqbuf[FSQBUFSIZE];
	UCHAR fsqbuft[FSQBUFSIZE];
	PUCHAR vol;
	UCHAR total_num[DIGLEN];
	UCHAR avail_num[DIGLEN];
	UCHAR used_num[DIGLEN];
	FSALLOCATE fsall;

	APIRET rc;

	drv[0] = (UCHAR) (drive + 'A' - 1);
	drv[1] = ':';
	drv[2] = '\0';

	rc = DosQueryFSAttach(
		drv,
		0,
		FSAIL_QUERYNAME,
		(PFSQBUFFER2) fsqbuft,
		&fsqbuflen);
	if(rc != 0) {
		printf(" %c:  ***\n", drive + 'A' - 1);
		return;
	}
	p = fsqbuft;			/* Point at device type */
	p += sizeof(USHORT);		/* Point past device type */
	p += (USHORT) *p + 3*sizeof(USHORT) + 1;
					/* Point past drive name and FS name */
					/* and FSDA length */

	/* Calculate values. Note that ALL capacities are calculated in
	   kilobytes to avoid having to use more than 32 bits. */

	rc = DosQueryFSInfo(
		(ULONG) drive,
		FSIL_ALLOC,
		&fsall,
		sizeof(fsall));
	if(rc != 0) {
		printf(" %c:  ***\n", drive + 'A' - 1);
		return;
	}
	cs = fsall.cSectorUnit * fsall.cbSector / 512;	/* Bytes/cluster */
	avail = (cs*fsall.cUnitAvail)/2;
	total = (cs*fsall.cUnit)/2;
	(void) DosQueryFSInfo(
		(ULONG) drive,
		FSIL_VOLSER,
		&fsqbuf,
		sizeof(fsqbuf));
	vol = &fsqbuf[5];	/* ULONG serial and BYTE length precede */
	printf(" %c:  %-9d  %-9d  %-9d   %-4d     %3d%%     %-11s  %s\n",
		drive + 'A' - 1,
		total,
		total-avail,
		avail,
		cs*512,
		(total-avail)*100/total,
		vol,
		p);
}


/*
 * Print message on standard error in printf style,
 * accompanied by program name.
 *
 */

static VOID error(PUCHAR mes, ...)
{	va_list ap;

	fprintf(stderr, "%s: ", progname);

	va_start(ap, mes);
	vfprintf(stderr, mes, ap);
	va_end(ap);

	fputc('\n', stderr);
}


static VOID putusage(VOID)
{	PUCHAR *p = (PUCHAR *) helpinfo;
	PUCHAR q;

	for(;;) {
		q = *p++;
		if(*q == '\0') break;

		fprintf(stderr, q, progname);
		fputc('\n', stderr);
	}
	fprintf(stderr, "\nThis is version %d.%d\n", VERSION, EDIT);
}

/*
 * End of file: df.c
 *
 */
