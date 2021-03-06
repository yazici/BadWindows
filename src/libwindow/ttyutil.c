/*
* 	ttyutil.c:
*
*	Contains routines to aid writing serial device drivers
*	These print their own error messages on stderr.
*	To have complete info printed on stdout about each read or write,
*	declare 'extern int ttyutil_debug;' and set ttyutil_debug to 1.
*
*	ken  ??
*	dead 10/84		added ttyutil_read_all()
*	dead 10/30/85	added ttyutil_nread()
*	vmb   3/14/86	added ttyutil_break()
*	dead  7/14/87	rewrote for HPUX
*
* 	fd = ttyutil_open (dev);
* 	int	fd;     * returned file descriptor
* 	char *dev;  * "/dev/ttya" or "/dev/ttyb"
*
*
*	cc = ttyutil_break (fd, duration);
*	int	cc;         * 0, we hope; -1 on error
*	int	duration;   * number of milliseconds to hold break condition
*
*
* 	cc = ttyutil_nread (fd, buf, num, timeout);
*
* 	int	cc;         * actual read count, -1 for error 
* 	int	fd;         * device file descriptor 
* 	char *buf;      * place to put the input 
* 	int	num;        * how many bytes to read 
*	int	timeout;    * secs to wait before quiting, -1 for indefinite, 0 polls
*
*
* 	cc = ttyutil_read_all (fd, buf, maxbytes);
*
* 	int	cc;         * actual read count, -1 for error 
* 	int	fd;         * device file descriptor 
* 	char *buf;      * place to put the input 
* 	int	maxbytes;   * max # of bytes to read 
*
*
* 	cc = ttyutil_write (fd, buf, num, pause);
*
* 	int	cc;         * actual write count, -1 for error 
* 	int	fd;         * device file descriptor 
* 	char *buf;      * stuff to write 
* 	int	num;        * how many bytes to write 
* 	int	pause;      * millisecs to pause between writing each char 
*
*		NOTE: catches SIGALRM if using pause
*
*
*	cc = ttyutil_drain( fd );
*
* 	int	cc;     * 0 for successful operation, -1 for error 
* 	int	fd;     * device file descriptor 
*
*
* 	cc = ttyutil_clear (fd);
*
* 	int	cc;     * number of chars discarded, -1 for error 
* 	int	fd;     * device file descriptor 
*/

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <errno.h>

#define	min(a, b)	((a) < (b)) ? (a) : (b)
#define	max(a, b)	((a) > (b)) ? (a) : (b)

typedef unsigned char byte;

int	ttyutil_debug = 0;
extern errno;


/*
==========================================================================
ttyutil_open()

Open tty, get file discriptor, flush the port, set baud rates, etc.
==========================================================================
*/
ttyutil_open( dev )
char *dev;
{
register fd;
struct termio tio;
int one = 1;

	/*
	*	Open for non-blocking read
	*/
	if ((fd = open(dev, O_RDWR, 0)) < 0)			
	{	fprintf(stderr, "ttyutil_open [failed on <%s>]", dev);
		perror("");
		return(-1);
	}

	/*
	*	Initialize device: 9600, 8bits, 1stop, no parity
	*/
	if (ioctl(fd, TCGETA, &tio) == -1)
	{	perror("ttyutil_open [ioctl TCGETA]");
		return(-1);
	}
	tio.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
	tio.c_cc[VMIN]  = 0;
	tio.c_cc[VTIME] = 0;

	if (ioctl(fd, TCSETA, &tio) == -1)
	{	perror("ttyutil_open [ioctl TCSETA]");
		return(-1);
	}

	if (ttyutil_clear(fd) < 0)
		return(-1);

	return(fd);
}
/* ttyutil_open */


/*
==========================================================================
ttyutil_break()	-- send break.  NB: HPUX ignores "duration"

==========================================================================
*/
ttyutil_break( fd, duration )
int fd, duration;
{
	if (ioctl(fd, TCSBRK, 0) == -1)	
	{	perror("ttyutil_break [ioctl TCSBRK]");
		return(-1);
	}
	return(0);
}
/* ttyutil_break */


/*
==========================================================================
ttyutil_get_flags(), ttyutil_set_flags()

Query and change HPUX termio "c_cflag".  This sets buad rate, etc.
==========================================================================
*/
ttyutil_get_flags( fd, flags )
int	fd;
short *flags;
{
struct termio tio;

	if (ioctl(fd, TCGETA, &tio) == -1)
	{	perror("ttyutil_get_flags [ioctl TCGETA]");
		return(-1);
	}
	*flags = tio.c_cflag;
	return(0);
}
/* ttyutil_get_flags */


ttyutil_set_flags( fd, flags )
int	fd;
short flags;
{
struct termio tio;

	if (ioctl(fd, TCGETA, &tio) == -1)
	{	perror("ttyutil_set_flags [ioctl TCGETA]");
		return(-1);
	}

	if (tio.c_cflag != flags)
	{
		tio.c_cflag = flags;

		if (ioctl(fd, TCSETA, &tio) == -1)
		{	perror("ttyutil_set_flags [ioctl TCSETA]");
			return(-1);
		}
	}
	return(0);
}
/* ttyutil_set_flags */




/* Added by S.Librande  May 23, 1991 */

ttyutil_read (fd, buf, readnum, timeoutval)
     int	fd;
     char       *buf;		/* place to put the input */
     int	readnum;	/* how many bytes to read */
     int	timeoutval;	/* millisecs to wait before quiting, 0 for indefinite*/
{
  int	Timeout = 0;		/* flag indicating a timeout */
  int	num = 0;
  int	i;

/*  if (timeoutval>0)
    set_timeout (timeoutval, &Timeout);
*/

  /* wait till enough chars available */
  for (i=0; i<readnum && (Timeout!=1);)	
    if (ioctl (fd,FIONREAD,&i)==-1)
      perror ("ttyutil_read: error with FIONREAD");

  if (Timeout != 1) num = read(fd,buf,readnum);
/*
  if (timeoutval > 0)
    set_timeout (0, &Timeout);
*/

  if (ttyutil_debug)
    {	if (Timeout==1) printf ("ttyutil_read: TIMEOUT\n");
	else if (num != -1)
	  {	printf ("ttyutil_read: read");
		for (i=0; i<num; i++)
		  printf (" [%x]",(byte)buf[i]);
		printf (".\n");
	      }
      }
  if (Timeout == 1) return (-2);
  if (num == -1)
    perror ("ttyutil_read: read error, probably: ");

  return (num);
}
/* ttyutil_read */




/*
==========================================================================
ttyutil_set_speed()

Set input and output speed of the tty.  Speeds are defined in
<termio.h> and are of the form B9600 B1200 etc.
==========================================================================
*/
ttyutil_set_speed( fd, speed )	
int	fd;
short speed;
{
short flags;

	if (ttyutil_get_flags(fd, &flags) < 0)
		return(-1);

	if ((flags & CBAUD) != speed)
	{
		flags &= ~CBAUD;
		flags |= CBAUD & speed;
		if (ttyutil_set_flags(fd, flags) < 0)
			return(-1);
	}
	return(0);
}
/* ttyutil_set_speed */


/*
==========================================================================
ttyutil_set_no_stopbits()

==========================================================================
*/
ttyutil_set_no_stopbits( fd, no )	
int	fd, no;
{
short flags, sb_flag;

	if (no == 1) sb_flag = 0;
	else if (no == 2) sb_flag = CSTOPB;
	else
	{	fprintf(stderr, "ttyutil_set_no_stopbits: <%d> must be 1 or 2\n", no);
		return(-1);
	}

	if (ttyutil_get_flags(fd, &flags) < 0)
		return(-1);

	if ((flags & CSTOPB) != sb_flag)
	{
		flags &= ~CSTOPB;
		flags |= sb_flag;
		if (ttyutil_set_flags(fd, flags) < 0)
			return(-1);
	}
	return(0);
}
/* ttyutil_set_no_stopbits */


/*
==========================================================================
ttyutil_set_charsize()

Sizes are defined in <termio.h> and are of the form CS7, CS8 etc.
==========================================================================
*/
ttyutil_set_charsize( fd, size )	
int	fd;
short size;
{
short flags;

	if (ttyutil_get_flags(fd, &flags) < 0)
		return(-1);

	if ((flags & CSIZE) != size)
	{
		flags &= ~CSIZE;
		flags |= CSIZE & size;
		if (ttyutil_set_flags(fd, flags) < 0)
			return(-1);
	}
	return(0);
}
/* ttyutil_set_charsize */


/*
==========================================================================
ttyutil_parity_odd(), ttyutil_parity_even(), ttyutil_parity_off()

Set parity characteristic of tty.
==========================================================================
*/
ttyutil_parity_odd( fd )
int	fd;
{
short flags;

	if (ttyutil_get_flags(fd, &flags) < 0)
		return(-1);

	if ((flags & PARENB) != PARENB || (flags & PARODD) != PARODD)
	{
		flags |= PARENB;
		flags |= PARODD;
		if (ttyutil_set_flags(fd, flags) < 0)
			return(-1);
	}
	return(0);
}
/* ttyutil_parity_odd */


ttyutil_parity_even( fd )
int	fd;
{
short flags;

	if (ttyutil_get_flags(fd, &flags) < 0)
		return(-1);

	if ((flags & PARENB) != PARENB || (flags & PARODD) != 0)
	{
		flags |= PARENB;
		flags &= ~PARODD;
		if (ttyutil_set_flags(fd, flags) < 0)
			return(-1);
	}
	return(0);
}
/* ttyutil_parity_even */


ttyutil_parity_off( fd )
int	fd;
{
short flags;

	if (ttyutil_get_flags(fd, &flags) < 0)
		return(-1);

	if ((flags & PARENB) != 0)
	{
		flags &= ~PARENB;
		if (ttyutil_set_flags(fd, flags) < 0)
			return(-1);
	}
	return(0);
}
/* ttyutil_parity_off */


/*
==========================================================================
Read commands from tty

	cc = ttyutil_nread (fd, buf, num, timeout);	* interrupt driven read 
	cc = ttyutil_read_all (fd, buf, maxnum);  	* read immeadiately
==========================================================================
*/
ttyutil_read_all (fd, buf, maxbytes)	
int	fd;
char *buf;
int	maxbytes;
{
int	i, num;

	if (ttyutil_debug)
		printf ("ttyutil_read_all: reading...\n");

	if (ttyutil_set_min_and_time(fd, 0, 0) == -1)
	{	fprintf(stderr, "ttyutil_read_all: ttyutil_set_min_and_time failed\n");
		return(-1);
	}

	if ((num = read(fd, buf, maxbytes)) < 0)
	{
		perror("ttyutil_read_all [read failed]");
		return(-1);
	}

	if (ttyutil_debug)
	{	
		if (num != 0)
		{	for (i=0; i<num-1; i++)
				printf("[0x%x] ", (byte)buf[i]);

			printf("[0x%x].\n", (byte)buf[num-1]);
		}
		printf ("ttyutil_read_all: done.\n");
	}
	return (num);
}
/* ttyutil_read_all */


ttyutil_nread (fd, buf, readnum, timeoutval)
int fd, readnum;
register char *buf;
int timeoutval;			/* In millisecs, 0 for poll, -1 for indefinite */
{
register i;
int readmask = 1<<fd;
int readfds;
register num;
struct timeval timeout;
struct timeval *toptr;
char *buf_save = buf;

	if (ttyutil_debug) 
		printf ("ttyutil_nread: reading...\n");

	if (ttyutil_set_min_and_time(fd, 0, 0) == -1)
	{	fprintf(stderr, "ttyutil_nread: ttyutil_set_min_and_time failed\n");
		return(-1);
	}

	if (timeoutval == -1) toptr = NULL;
	else
	{	timeout.tv_sec = timeoutval / 1000;
		timeout.tv_usec = (timeoutval % 1000) * 1000;
		toptr = &timeout;
	}

	/*
	*	Read available bytes
	*/
	if ((num = read(fd, buf, readnum)) < 0)
	{	perror("ttyutil_nread [read failed]");
		return(-1);
	}
	buf += num;

	/*
	*	Read single bytes with timeout
	*/
	while (num < readnum)
	{
		readfds = readmask;

		if ((i = select(32, &readfds, 0, 0, toptr)) == 0)
		{	if (ttyutil_debug) printf ("ttyutil_nread: TIMEOUT\n");
			break;
		}
		else if (i < 0)
		{	perror("ttyutil_nread [select failed]");
			return(-1);
		}
		else
		{	if (read(fd, buf, 1) != 1)
			{   perror("ttyutil_nread [read failed]");
				break;
			}
			buf++;
			num++;
		}
	}
	if (num == 0) num = -2;


	if (ttyutil_debug)
	{
		if (num == -2) printf ("ttyutil_nread: TIMEOUT\n");
		else
		{	for (i=0, buf = buf_save; i<num-1; i++, buf++)
				printf("[0x%x] ", (byte)(*buf));

			printf("[0x%x].\n", (byte)(*buf));
		}
		printf ("ttyutil_nread: done.\n");
	}
	return (num);
}
/* ttyutil_nread */


/*
==========================================================================
ttyutil_write() -- Write buf to tty

==========================================================================
*/
ttyutil_write (fd, buf, writenum, pauselength)

register fd;
register char *buf;
register writenum;
register pauselength;	/* millisecs */
{
register i;

	if (ttyutil_debug)
	{	
		printf("ttyutil_write: writing...\n");

		for (i=0; i<writenum-1; i++)
			printf("[0x%x] ", (byte)buf[i]);

		if (writenum != 0)
			printf("[0x%x].\n", (byte)buf[writenum-1]);
	}
	if (pauselength == 0)
	{
		if (write(fd, buf, writenum) != writenum)
		{	perror ("ttyutil_write [write failed]");
			return (-1);
		}
	}
	else
	{	for (i=0; i<writenum; i++, buf++)
		{	
			if (write(fd, buf, 1) != 1)
			{	perror ("ttyutil_write [write failed]");
				return (-1);
			}
			msleep( fd/*pauselength*/ );
		}
	}
	if (ttyutil_debug)
		printf ("ttyutil_write: done.\n");

	return (writenum);
}
/* ttyutil_write */


/*
==========================================================================
ttyutil_drain() -- Drain output buffer

==========================================================================
*/
ttyutil_drain( fd )
int fd;
{
	struct termio tio;

	if (ioctl(fd, TCGETA, &tio) == -1)
	{	perror("ttyutil_drain [ioctl TCGETA]");
		return(-1);
	}
	if (ioctl(fd, TCSETAW, &tio) == -1)
	{	perror("ttyutil_drain [ioctl TCSETA]");
		return(-1);
	}
	return 0;
}


/*
==========================================================================
ttyutil_clear() -- Clear input and output buffers

==========================================================================
*/
ttyutil_clear( fd )
int fd;
{	return( ioctl(fd, TCFLSH, 2) );
}


/*
==========================================================================
ttyutil_set_min_and_time() -- Set timeout for read.

==========================================================================
*/
static ttyutil_set_min_and_time( fd, minimum, time )

int	fd;
int minimum, time;
{
struct termio tio;

	if (ioctl(fd, TCGETA, &tio) == -1)
	{	perror("ttyutil_set_min_and_time [ioctl TCGETA]");
		return(-1);
	}

	if (tio.c_cc[VMIN] != minimum || tio.c_cc[VTIME] != time)
	{
		tio.c_cc[VMIN]  = minimum;
		tio.c_cc[VTIME] = time;

		if (ioctl(fd, TCSETA, &tio) == -1)
		{	perror("ttyutil_set_min_and_time [ioctl TCSETA]");
			return(-1);
		}
	}
	return(0);
}
/* ttyutil_set_min_and_time */

msleep( x )
	int x;
{
#if 0
	ttyutil_drain( x );
#endif
}
