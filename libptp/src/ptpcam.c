/* ptpcam.c
 *
 * Copyright (C) 2001-2005 Mariusz Woloszyn <emsi@ipartners.pl>
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include "ptp.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <usb.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "ptpcam.h"

/* some defines comes here */

/* USB interface class */
#ifndef USB_CLASS_PTP
#define USB_CLASS_PTP		6
#endif

/* USB control message data phase direction */
#ifndef USB_DP_HTD
#define USB_DP_HTD		(0x00 << 7)	/* host to device */
#endif
#ifndef USB_DP_DTH
#define USB_DP_DTH		(0x01 << 7)	/* device to host */
#endif

/* PTP class specific requests */
#ifndef USB_REQ_DEVICE_RESET
#define USB_REQ_DEVICE_RESET		0x66
#endif
#ifndef USB_REQ_GET_DEVICE_STATUS
#define USB_REQ_GET_DEVICE_STATUS	0x67
#endif

/* USB Feature selector HALT */
#ifndef USB_FEATURE_HALT
#define USB_FEATURE_HALT	0x00
#endif

/* OUR APPLICATION USB URB (2MB) ;) */
#define PTPCAM_USB_URB		2097152

#define USB_TIMEOUT		5000
#define USB_CAPTURE_TIMEOUT	20000

/* one global variable (yes, I know it sucks) */
short verbose=0;
/* the other one, it sucks definitely ;) */
int ptpcam_usb_timeout = USB_TIMEOUT;

/* we need it for a proper signal handling :/ */
PTPParams* globalparams;


void
usage()
{
	printf("USAGE: ptpcam [OPTION]\n\n");
}

void
help()
{
	printf("USAGE: ptpcam [OPTION]\n\n");
	printf("Options:\n"
	"  --bus=BUS-NUMBER             USB bus number\n"
	"  --dev=DEV-NUMBER             USB assigned device number\n"
	"  -r, --reset                  Reset the device\n"
	"  -l, --list-devices           List all PTP devices\n"
	"                               \n"
	"  -i, --info                   Show device info\n"
	"  -o, --list-operations        List all supported operations\n"
	"  -p, --list-properties        List all supported device properties\n"
	"                               "
					"(e.g. focus mode, focus distance, etc.)\n"
	"  -s, --show-property=NUMBER   Display property details "
					"(or set its value,\n"
	"                               if used in conjunction with --val)\n"
	"  --set-property=NUMBER        Set property value (--val required)\n"
	"  --set=PROP-NAME              Set property by name (abbreviations allowed)\n"
	"  --val=VALUE                  Property value (numeric for --set-property and\n"
	"                               string or numeric for --set)\n"
	"  --show-all-properties        Show all properties values\n"
	"  --show-unknown-properties    Show unknown properties values\n"
	"                               \n"
	"  -L, --list-files             List all files\n"
	"  -g, --get-file=HANDLE        Get file by given handler\n"
	"  -G, --get-all-files          Get all files\n"
	"                               \n"
	"  -d, --delete-object=HANDLE   Delete object (file) by given handle\n"
	"  -D, --delete-all-files       Delete all files form camera\n"
	"                               \n"
	"  -c, --capture                Initiate capture\n"
	"  --loop-capture=N             Perform N times capture/get/delete sequence\n"
	"  --interval=N                 Set loop capture interval in seconds\n"
	"                               Following Nikon commands are EXPERIMENTAL!\n"
	"  --nikon-ic, --nic            Initiate Nikon Direct Capture (no download!)\n"
	"  --nikon-dc, --ndc            Initiate Nikon Direct Capture and download\n"
	"                               \n"
	"  -R reqCode[,P1,P2,P3,P4,P5,d] Send a raw generic request with parameters\n"
	"                               parameters Pn are optionnal and can be set to 0\n"
	"                               if not used (params values must be hexadecimal)\n"
	"                               d, is the data direction (r=read, n=no data, \n"
	"                               filename or value to send in hexadecimal).\n"
	"                               The default data direction is 'r'.\n"
	"                               If you want to send data, give the file name\n"
	"                               containing data to send, or, if the data is short,\n"
	"                               you can directly write the value in hexadecimal.\n"
	"                               The result data and response will be displayed\n"
	"                      example: ptpcam -R 0x1015,0x5003,0,0,0,0,r\n"
	"                               ptpcam -R 0x1016,0x5003,0,0,0,0,image_size.bin\n"
	"                               ptpcam -R 0x1016,0x501b,0,0,0,0,0x88130000\n"
	"                               ptpcam -R 0x100e,0,0,0,0,0,n\n"
	"                               \n"
	"  --overwrite                  Force file overwrite while saving"
					"to disk\n"
	"  -f, --force                  Talk to non PTP devices\n"
	"  -v, --verbose                Be verbose (print more debug)\n"
	"  -h, --help                   Print this help message\n"
	"\n");
}

void
display_hexdump(char *data, size_t size)
{
	uint i=0;
	char buffer[50];
	char charBuf[17];
	
	memset((void*)buffer, 0, 49);
	memset((void*)charBuf, 0, 17);
	for (; i < size; ++i)
	{
		snprintf(&(buffer[(i%16) * 3]) , 4, "%02x ", *(data+i)&0xff);
		if ((data[i] >= ' ' && data[i] <= '^') ||
			(data[i] >= 'a' && data[i] <= '~')) // printable characters
			charBuf[i%16] = data[i];
		else
			charBuf[i%16] = '.';
		if ((i % 16) == 15 || i == (size - 1))
		{
			charBuf[(i%16)+1] = '\0';
			buffer[((i%16) * 3) + 2] = '\0';
			printf("%-48s- %-16s\n", buffer, charBuf);
			memset((void*)buffer, 0, 49);
			memset((void*)charBuf, 0, 17);
		}
	}
}

void
ptpcam_siginthandler(int signum)
{
    PTP_USB* ptp_usb=(PTP_USB *)globalparams->data;
    struct usb_device *dev=usb_device(ptp_usb->handle);

    if (signum==SIGINT)
    {
	/* hey it's not that easy though... but at least we can try! */
	printf("Got SIGINT, trying to clean up and close...\n");
	usleep(5000);
	close_camera (ptp_usb, globalparams, dev);
	exit (-1);
    }
}

static short
ptp_read_func (unsigned char *bytes, unsigned int size, void *data)
{
	int result=-1;
	PTP_USB *ptp_usb=(PTP_USB *)data;
	int toread=0;
	signed long int rbytes=size;

	do {
		bytes+=toread;
		if (rbytes>PTPCAM_USB_URB) 
			toread = PTPCAM_USB_URB;
		else
			toread = rbytes;
		result=USB_BULK_READ(ptp_usb->handle, ptp_usb->inep,(char *)bytes, toread,ptpcam_usb_timeout);
		/* sometimes retry might help */
		if (result==0)
			result=USB_BULK_READ(ptp_usb->handle, ptp_usb->inep,(char *)bytes, toread,ptpcam_usb_timeout);
		if (result < 0)
			break;
		rbytes-=PTPCAM_USB_URB;
	} while (rbytes>0);

	if (result >= 0) {
		return (PTP_RC_OK);
	}
	else 
	{
		if (verbose) perror("usb_bulk_read");
		return PTP_ERROR_IO;
	}
}

static short
ptp_write_func (unsigned char *bytes, unsigned int size, void *data)
{
	int result;
	PTP_USB *ptp_usb=(PTP_USB *)data;

	result=USB_BULK_WRITE(ptp_usb->handle,ptp_usb->outep,(char *)bytes,size,ptpcam_usb_timeout);
	if (result >= 0)
		return (PTP_RC_OK);
	else 
	{
		if (verbose) perror("usb_bulk_write");
		return PTP_ERROR_IO;
	}
}

/* XXX this one is suposed to return the number of bytes read!!! */
static short
ptp_check_int (unsigned char *bytes, unsigned int size, void *data)
{
	int result;
	PTP_USB *ptp_usb=(PTP_USB *)data;

	result=USB_BULK_READ(ptp_usb->handle, ptp_usb->intep,(char *)bytes,size,ptpcam_usb_timeout);
	if (result==0)
	    result=USB_BULK_READ(ptp_usb->handle, ptp_usb->intep,(char *)bytes,size,ptpcam_usb_timeout);
	if (verbose>2) fprintf (stderr, "USB_BULK_READ returned %i, size=%i\n", result, size);

	if (result >= 0) {
		return result;
	} else {
		if (verbose) perror("ptp_check_int");
		return result;
	}
}


void
ptpcam_debug (void *data, const char *format, va_list args);
void
ptpcam_debug (void *data, const char *format, va_list args)
{
	if (verbose<2) return;
	vfprintf (stderr, format, args);
	fprintf (stderr,"\n");
	fflush(stderr);
}

void
ptpcam_error (void *data, const char *format, va_list args);
void
ptpcam_error (void *data, const char *format, va_list args)
{
	vfprintf (stderr, format, args);
	fprintf (stderr,"\n");
	fflush(stderr);
}



void
init_ptp_usb (PTPParams* params, PTP_USB* ptp_usb, struct usb_device* dev)
{
	usb_dev_handle *device_handle;

	params->write_func=ptp_write_func;
	params->read_func=ptp_read_func;
	params->check_int_func=ptp_check_int;
	params->check_int_fast_func=ptp_check_int;
	params->error_func=ptpcam_error;
	params->debug_func=ptpcam_debug;
	params->sendreq_func=ptp_usb_sendreq;
	params->senddata_func=ptp_usb_senddata;
	params->getresp_func=ptp_usb_getresp;
	params->getdata_func=ptp_usb_getdata;
	params->data=ptp_usb;
	params->transaction_id=0;
	params->byteorder = PTP_DL_LE;

	if ((device_handle=usb_open(dev))){
		if (!device_handle) {
			perror("usb_open()");
			exit(0);
		}
		ptp_usb->handle=device_handle;
		usb_set_configuration(device_handle, dev->config->bConfigurationValue);
		usb_claim_interface(device_handle,
			dev->config->interface->altsetting->bInterfaceNumber);
	}
	globalparams=params;
}

void
clear_stall(PTP_USB* ptp_usb)
{
	uint16_t status=0;
	int ret;

	/* check the inep status */
	ret=usb_get_endpoint_status(ptp_usb,ptp_usb->inep,&status);
	if (ret<0) perror ("inep: usb_get_endpoint_status()");
	/* and clear the HALT condition if happend */
	else if (status) {
		printf("Resetting input pipe!\n");
		ret=usb_clear_stall_feature(ptp_usb,ptp_usb->inep);
        	/*usb_clear_halt(ptp_usb->handle,ptp_usb->inep); */
		if (ret<0)perror ("usb_clear_stall_feature()");
	}
	status=0;

	/* check the outep status */
	ret=usb_get_endpoint_status(ptp_usb,ptp_usb->outep,&status);
	if (ret<0) perror ("outep: usb_get_endpoint_status()");
	/* and clear the HALT condition if happend */
	else if (status) {
		printf("Resetting output pipe!\n");
        	ret=usb_clear_stall_feature(ptp_usb,ptp_usb->outep);
		/*usb_clear_halt(ptp_usb->handle,ptp_usb->outep); */
		if (ret<0)perror ("usb_clear_stall_feature()");
	}

        /*usb_clear_halt(ptp_usb->handle,ptp_usb->intep); */
}

void
close_usb(PTP_USB* ptp_usb, struct usb_device* dev)
{
	//clear_stall(ptp_usb);
        usb_release_interface(ptp_usb->handle,
                dev->config->interface->altsetting->bInterfaceNumber);
	usb_reset(ptp_usb->handle);
        usb_close(ptp_usb->handle);
}


struct usb_bus*
init_usb()
{
	usb_init();
	usb_find_busses();
	usb_find_devices();
	return (usb_get_busses());
}

/*
   find_device() returns the pointer to a usb_device structure matching
   given busn, devicen numbers. If any or both of arguments are 0 then the
   first matching PTP device structure is returned. 
*/
struct usb_device*
find_device (int busn, int devicen, short force);
struct usb_device*
find_device (int busn, int devn, short force)
{
	struct usb_bus *bus;
	struct usb_device *dev;

	bus=init_usb();
	for (; bus; bus = bus->next)
	for (dev = bus->devices; dev; dev = dev->next)
	if (dev->config)
	if ((dev->config->interface->altsetting->bInterfaceClass==
		USB_CLASS_PTP)||force)
	if (dev->descriptor.bDeviceClass!=USB_CLASS_HUB)
	{
		int curbusn, curdevn;

		curbusn=strtol(bus->dirname,NULL,10);
		curdevn=strtol(dev->filename,NULL,10);

		if (devn==0) {
			if (busn==0) return dev;
			if (curbusn==busn) return dev;
		} else {
			if ((busn==0)&&(curdevn==devn)) return dev;
			if ((curbusn==busn)&&(curdevn==devn)) return dev;
		}
	}
	return NULL;
}

void
find_endpoints(struct usb_device *dev, int* inep, int* outep, int* intep);
void
find_endpoints(struct usb_device *dev, int* inep, int* outep, int* intep)
{
	int i,n;
	struct usb_endpoint_descriptor *ep;

	ep = dev->config->interface->altsetting->endpoint;
	n=dev->config->interface->altsetting->bNumEndpoints;

	for (i=0;i<n;i++) {
	if (ep[i].bmAttributes==USB_ENDPOINT_TYPE_BULK)	{
		if ((ep[i].bEndpointAddress&USB_ENDPOINT_DIR_MASK)==
			USB_ENDPOINT_DIR_MASK)
		{
			*inep=ep[i].bEndpointAddress;
			if (verbose>1)
				fprintf(stderr, "Found inep: 0x%02x\n",*inep);
		}
		if ((ep[i].bEndpointAddress&USB_ENDPOINT_DIR_MASK)==0)
		{
			*outep=ep[i].bEndpointAddress;
			if (verbose>1)
				fprintf(stderr, "Found outep: 0x%02x\n",*outep);
		}
		} else if ((ep[i].bmAttributes==USB_ENDPOINT_TYPE_INTERRUPT) &&
			((ep[i].bEndpointAddress&USB_ENDPOINT_DIR_MASK)==
				USB_ENDPOINT_DIR_MASK))
		{
			*intep=ep[i].bEndpointAddress;
			if (verbose>1)
				fprintf(stderr, "Found intep: 0x%02x\n",*intep);
		}
	}
}

int
open_camera (int busn, int devn, short force, PTP_USB *ptp_usb, PTPParams *params, struct usb_device **dev)
{
#ifdef DEBUG
	printf("dev %i\tbus %i\n",devn,busn);
#endif
	
	*dev=find_device(busn,devn,force);
	if (*dev==NULL) {
		fprintf(stderr,"could not find any device matching given "
		"bus/dev numbers\n");
		exit(-1);
	}
	find_endpoints(*dev,&ptp_usb->inep,&ptp_usb->outep,&ptp_usb->intep);

	init_ptp_usb(params, ptp_usb, *dev);
	if (ptp_opensession(params,1)!=PTP_RC_OK) {
		fprintf(stderr,"ERROR: Could not open session!\n");
		close_usb(ptp_usb, *dev);
		return -1;
	}
	if (ptp_getdeviceinfo(params,&params->deviceinfo)!=PTP_RC_OK) {
		fprintf(stderr,"ERROR: Could not get device info!\n");
		close_usb(ptp_usb, *dev);
		return -1;
	}
	return 0;
}

void
close_camera (PTP_USB *ptp_usb, PTPParams *params, struct usb_device *dev)
{
	if (ptp_closesession(params)!=PTP_RC_OK)
		fprintf(stderr,"ERROR: Could not close session!\n");
	close_usb(ptp_usb, dev);
}


void
list_devices(short force)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	int found=0;


	bus=init_usb();
  	for (; bus; bus = bus->next)
    	for (dev = bus->devices; dev; dev = dev->next) {
		/* if it's a PTP device try to talk to it */
		if (dev->config)
		if ((dev->config->interface->altsetting->bInterfaceClass==
			USB_CLASS_PTP)||force)
		if (dev->descriptor.bDeviceClass!=USB_CLASS_HUB)
		{
			PTPParams params;
			PTP_USB ptp_usb;
			PTPDeviceInfo deviceinfo;

			if (!found){
				printf("\nListing devices...\n");
				printf("bus/dev\tvendorID/prodID\tdevice model\n");
				found=1;
			}

			find_endpoints(dev,&ptp_usb.inep,&ptp_usb.outep,
				&ptp_usb.intep);
			init_ptp_usb(&params, &ptp_usb, dev);

			CC(ptp_opensession (&params,1),
				"Could not open session!\n"
				"Try to reset the camera.\n");
			CC(ptp_getdeviceinfo (&params, &deviceinfo),
				"Could not get device info!\n");

      			printf("%s/%s\t0x%04X/0x%04X\t%s\n",
				bus->dirname, dev->filename,
				dev->descriptor.idVendor,
				dev->descriptor.idProduct, deviceinfo.Model);

			CC(ptp_closesession(&params),
				"Could not close session!\n");
			close_usb(&ptp_usb, dev);
		}
	}
	if (!found) printf("\nFound no PTP devices\n");
	printf("\n");
}

void
show_info (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;

	printf("\nCamera information\n");
	printf("==================\n");
	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Model: %s\n",params.deviceinfo.Model);
	printf("  manufacturer: %s\n",params.deviceinfo.Manufacturer);
	printf("  serial number: '%s'\n",params.deviceinfo.SerialNumber);
	printf("  device version: %s\n",params.deviceinfo.DeviceVersion);
	printf("  extension ID: 0x%08lx\n",(long unsigned)
					params.deviceinfo.VendorExtensionID);
	printf("  extension description: %s\n",
					params.deviceinfo.VendorExtensionDesc);
	printf("  extension version: 0x%04x\n",
				params.deviceinfo.VendorExtensionVersion);
	printf("\n");
	close_camera(&ptp_usb, &params, dev);
}

void
capture_image (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	PTPContainer event;
	int ExposureTime=0;
	struct usb_device *dev;
	short ret;

	printf("\nInitiating captue...\n");
	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	if (!ptp_operation_issupported(&params, PTP_OC_InitiateCapture))
	{
	    printf ("Your camera does not support InitiateCapture operation!\nSorry, blame the %s!\n", params.deviceinfo.Manufacturer);
	    goto out;
	}

	/* obtain exposure time in miliseconds */
	if (ptp_property_issupported(&params, PTP_DPC_ExposureTime))
	{
	    PTPDevicePropDesc dpd;
	    memset(&dpd,0,sizeof(dpd));
	    ret=ptp_getdevicepropdesc(&params,PTP_DPC_ExposureTime,&dpd);
	    if (ret==PTP_RC_OK) ExposureTime=(*(int32_t*)(dpd.CurrentValue))/10;
	}

	/* adjust USB timeout */
	if (ExposureTime>USB_TIMEOUT) ptpcam_usb_timeout=ExposureTime;

	CR(ptp_initiatecapture (&params, 0x0, 0), "Could not capture.\n");
	
	ret=ptp_usb_event_wait(&params,&event);
	if (ret!=PTP_RC_OK) goto err;
	if (verbose) printf ("Event received %08x, ret=%x\n", event.Code, ret);
	if (event.Code==PTP_EC_CaptureComplete) {
		printf ("Camera reported 'capture completed' but the object information is missing.\n");
		goto out;
	}
		
	while (event.Code==PTP_EC_ObjectAdded) {
		printf ("Object added 0x%08lx\n", (long unsigned) event.Param1);
		if (ptp_usb_event_wait(&params, &event)!=PTP_RC_OK)
			goto err;
		if (verbose) printf ("Event received %08x, ret=%x\n", event.Code, ret);
		if (event.Code==PTP_EC_CaptureComplete) {
			printf ("Capture completed successfully!\n");
			goto out;
		}
	}
	
err:
	printf("Events receiving error. Capture status unknown.\n");
out:

	ptpcam_usb_timeout=USB_TIMEOUT;
	close_camera(&ptp_usb, &params, dev);
}

static void sig_alrm(int signo)
{
}

void
loop_capture (int busn, int devn, short force, int n, int interval, int overwrite)
{
	PTPParams params;
	PTP_USB ptp_usb;
	PTPContainer event;
	struct usb_device *dev;
	int file;
	PTPObjectInfo oi;
	uint32_t handle=0;
	char *image;
	int ret;
	char *filename;
	time_t start_time;
	time_t rest_time;

	// Catch the SIGALARM
	signal(SIGALRM, sig_alrm);

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	/* capture timeout should be longer */
	ptpcam_usb_timeout=USB_CAPTURE_TIMEOUT;

	printf("Camera: %s\n",params.deviceinfo.Model);

	/* local loop */
	while (n>0) {
		/* capture */
		time(&start_time);
		printf("\nInitiating captue...\n");
		CR(ptp_initiatecapture (&params, 0x0, 0),"Could not capture\n");
		n--;

		ret=ptp_usb_event_wait(&params,&event);
		if (verbose) printf ("Event received %08x, ret=%x\n", event.Code, ret);
		if (ret!=PTP_RC_OK) goto err;
		if (event.Code==PTP_EC_CaptureComplete) {
			printf ("CANNOT DOWNLOAD: got 'capture completed' but the object information is missing.\n");
			goto out;
		}
			
		while (event.Code==PTP_EC_ObjectAdded) {
			printf ("Object added 0x%08lx\n",(long unsigned) event.Param1);
			handle=event.Param1;
			if (ptp_usb_event_wait(&params, &event)!=PTP_RC_OK)
				goto err;
			if (verbose) printf ("Event received %08x, ret=%x\n", event.Code, ret);
			if (event.Code==PTP_EC_CaptureComplete)
				goto download;
		}
download:	

		memset(&oi, 0, sizeof(PTPObjectInfo));
		if (verbose) printf ("Downloading: 0x%08lx\n",(long unsigned) handle);
		if ((ret=ptp_getobjectinfo(&params,handle, &oi))!=PTP_RC_OK){
			fprintf(stderr,"ERROR: Could not get object info\n");
			ptp_perror(&params,ret);
			if (ret==PTP_ERROR_IO) clear_stall(&ptp_usb);
			continue;
		}
	
		if (oi.ObjectFormat == PTP_OFC_Association)
				goto out;
		filename=(oi.Filename);
		file=open(filename, (overwrite==OVERWRITE_EXISTING?0:O_EXCL)|O_RDWR|O_CREAT|O_TRUNC,S_IRWXU|S_IRGRP);
		if (file==-1) {
			if (errno==EEXIST) {
				printf("Skipping file: \"%s\", file exists!\n",filename);
				goto out;
			}
			perror("open");
			goto out;
		}
		lseek(file,oi.ObjectCompressedSize-1,SEEK_SET);
		ret=write(file,"",1);
		if (ret==-1) {
			perror("write");
			goto out;
		}
		image=mmap(0,oi.ObjectCompressedSize,PROT_READ|PROT_WRITE,MAP_SHARED,
			file,0);
		if (image==MAP_FAILED) {
			perror("mmap");
			close(file);
			goto out;
		}
		printf ("Saving file: \"%s\" ",filename);
		fflush(NULL);
		ret=ptp_getobject(&params,handle,&image);
		munmap(image,oi.ObjectCompressedSize);
		close(file);
		if (ret!=PTP_RC_OK) {
			printf ("error!\n");
			ptp_perror(&params,ret);
			if (ret==PTP_ERROR_IO) clear_stall(&ptp_usb);
		} else {
			/* and delete from camera! */
			printf("is done...\nDeleting from camera.\n");
			CR(ptp_deleteobject(&params, handle,0),
					"Could not delete object\n");
			printf("Object 0x%08lx (%s) deleted.\n",(long unsigned) handle, oi.Filename);
		}
out:
		rest_time=interval-(time(NULL)-start_time);
		if (rest_time>0 && n>0) {
		    printf("Sleeping for remaining %u seconds.\n",rest_time);
		    alarm(rest_time);
		    pause();
		}
		;
	}
err:

	ptpcam_usb_timeout=USB_TIMEOUT;
	close_camera(&ptp_usb, &params, dev);
}

void
nikon_initiate_dc (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	uint16_t result;
    
	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s\n",params.deviceinfo.Model);
	printf("\nInitiating direct captue...\n");
	
	if (params.deviceinfo.VendorExtensionID!=PTP_VENDOR_NIKON)
	{
	    printf ("Your camera is not Nikon!\nDo not buy from %s!\n",params.deviceinfo.Manufacturer);
	    goto out;
	}

	if (!ptp_operation_issupported(&params,PTP_OC_NIKON_DirectCapture)) {
	    printf ("Sorry, your camera dows not support Nikon DirectCapture!\nDo not buy from %s!\n",params.deviceinfo.Manufacturer);
	    goto out;
	}

	/* perform direct capture */
	result=ptp_nikon_directcapture (&params, 0xffffffff);
	if (result!=PTP_RC_OK) {
	    ptp_perror(&params,result);
	    fprintf(stderr,"ERROR: Could not capture.\n");
	    if (result!=PTP_RC_StoreFull) {
		close_camera(&ptp_usb, &params, dev);
		return;
	    }
	}
	usleep(300*1000);
out:	
	close_camera(&ptp_usb, &params, dev);

}

void
nikon_direct_capture (int busn, int devn, short force, char* filename,int overwrite)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	uint16_t result;
	uint16_t nevent=0;
	PTPUSBEventContainer* events=NULL;
	int ExposureTime=0;	/* exposure time in miliseconds */
	int BurstNumber=1;
	PTPDevicePropDesc dpd;
	PTPObjectInfo oi;
	int i;
    
	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s\n",params.deviceinfo.Model);

	if ((result=ptp_getobjectinfo(&params,0xffff0001, &oi))==PTP_RC_OK) {
	    if (filename==NULL) filename=oi.Filename;
	    save_object(&params, 0xffff0001, filename, oi, overwrite);
	    goto out;
	}

	printf("\nInitiating direct captue...\n");
	
	if (params.deviceinfo.VendorExtensionID!=PTP_VENDOR_NIKON)
	{
	    printf ("Your camera is not Nikon!\nDo not buy from %s!\n",params.deviceinfo.Manufacturer);
	    goto out;
	}

	if (!ptp_operation_issupported(&params,PTP_OC_NIKON_DirectCapture)) {
	    printf ("Sorry, your camera dows not support Nikon DirectCapture!\nDo not buy from %s!\n",params.deviceinfo.Manufacturer);
	    goto out;
	}

	/* obtain exposure time in miliseconds */
	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(&params,PTP_DPC_ExposureTime,&dpd);
	if (result==PTP_RC_OK) ExposureTime=(*(int32_t*)(dpd.CurrentValue))/10;

	/* obtain burst number */
	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(&params,PTP_DPC_BurstNumber,&dpd);
	if (result==PTP_RC_OK) BurstNumber=*(uint16_t*)(dpd.CurrentValue);
/*
	if ((result=ptp_getobjectinfo(&params,0xffff0001, &oi))==PTP_RC_OK)
	{
	    if (filename==NULL) filename=oi.Filename;
	    save_object(&params, 0xffff0001, filename, oi, overwrite);
	    ptp_nikon_keepalive(&params);
	    ptp_nikon_keepalive(&params);
	    ptp_nikon_keepalive(&params);
	    ptp_nikon_keepalive(&params);
	}
*/

	/* perform direct capture */
	result=ptp_nikon_directcapture (&params, 0xffffffff);
	if (result!=PTP_RC_OK) {
	    ptp_perror(&params,result);
	    fprintf(stderr,"ERROR: Could not capture.\n");
	    if (result!=PTP_RC_StoreFull) {
		close_camera(&ptp_usb, &params, dev);
		return;
	    }
	}
	if (BurstNumber>1) printf("Capturing %i frames in burst.\n",BurstNumber);

	/* sleep in case of exposure longer than 1/100 */
	//if (ExposureTime>10) {
	    printf ("sleeping %i miliseconds\n", 500+ExposureTime);
	    usleep (ExposureTime*1000+500000);
	//}

	while (BurstNumber>0) {

#if 0	    /* Is this really needed??? */
	    ptp_nikon_keepalive(&params);
#endif

	    result=ptp_nikon_checkevent (&params, &events, &nevent);
	    if (result != PTP_RC_OK) {
		fprintf(stderr, "Error checking Nikon events\n");
		ptp_perror(&params,result);
		goto out;
	    }
	    for(i=0;i<nevent;i++) {
	    ptp_nikon_keepalive(&params);
		void *prop;
		if (events[i].code==PTP_EC_DevicePropChanged) {
		    printf ("Checking: %s\n", ptp_prop_getname(&params, events[i].param1));
		    ptp_getdevicepropvalue(&params, events[i].param1, &prop, PTP_DTC_UINT64);
		}

		printf("Event [%i] = 0x%04x,\t param: %08x\n",i, events[i].code ,events[i].param1);
		if (events[i].code==PTP_EC_NIKON_CaptureOverflow) {
		    printf("Ram cache overflow? Shooting to fast!\n");
		    if ((result=ptp_getobjectinfo(&params,0xffff0001, &oi))!=PTP_RC_OK) {
		        fprintf(stderr, "Could not get object info\n");
		        ptp_perror(&params,result);
		        goto out;
		    }
		    if (filename==NULL) filename=oi.Filename;
		    save_object(&params, 0xffff0001, filename, oi, overwrite);
		    BurstNumber=0;
		    usleep(100);
		} else
		if (events[i].code==PTP_EC_NIKON_ObjectReady) 
		{
		    if ((result=ptp_getobjectinfo(&params,0xffff0001, &oi))!=PTP_RC_OK) {
		        fprintf(stderr, "Could not get object info\n");
		        ptp_perror(&params,result);
		        goto out;
		    }
		    if (filename==NULL) filename=oi.Filename;
		    save_object(&params, 0xffff0001, filename, oi, overwrite);
		    BurstNumber--;
		}
	    }
	    free (events);
	}

out:	
	ptpcam_usb_timeout=USB_TIMEOUT;
	close_camera(&ptp_usb, &params, dev);
}


void
nikon_direct_capture2 (int busn, int devn, short force, char* filename, int overwrite)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	uint16_t result;
	PTPObjectInfo oi;

	dev=find_device(busn,devn,force);
	if (dev==NULL) {
		fprintf(stderr,"could not find any device matching given "
		"bus/dev numbers\n");
		exit(-1);
	}
	find_endpoints(dev,&ptp_usb.inep,&ptp_usb.outep,&ptp_usb.intep);

	init_ptp_usb(&params, &ptp_usb, dev);

	if (ptp_opensession(&params,1)!=PTP_RC_OK) {
		fprintf(stderr,"ERROR: Could not open session!\n");
		close_usb(&ptp_usb, dev);
		return ;
	}
/*
	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(&params,PTP_DPC_BurstNumber,&dpd);
	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(&params,PTP_DPC_ExposureTime,&dpd);
*/

	/* perform direct capture */
	result=ptp_nikon_directcapture (&params, 0xffffffff);
	if (result!=PTP_RC_OK) {
	    ptp_perror(&params,result);
	    fprintf(stderr,"ERROR: Could not capture.\n");
	    if (result!=PTP_RC_StoreFull) {
	        close_camera(&ptp_usb, &params, dev);
	        return;
	    }
	}

	if (ptp_closesession(&params)!=PTP_RC_OK)
	{
	    fprintf(stderr,"ERROR: Could not close session!\n");
	    return;
	}

	usleep(300*1000);

	if (ptp_opensession(&params,1)!=PTP_RC_OK) {
    		fprintf(stderr,"ERROR: Could not open session!\n");
    		close_usb(&ptp_usb, dev);
    		return;
    	}
loop:
    	if ((result=ptp_getobjectinfo(&params,0xffff0001, &oi))==PTP_RC_OK) {
    	    if (filename==NULL) filename=oi.Filename;
    	    save_object(&params, 0xffff0001, filename, oi, overwrite);
    	} else {
	    ptp_nikon_keepalive(&params);
	    goto loop;
	}

/*out:	*/
	close_camera(&ptp_usb, &params, dev);


#if 0
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	uint16_t result;
	uint16_t nevent=0;
	PTPUSBEventContainer* events=NULL;
	int ExposureTime=0;	/* exposure time in miliseconds */
	int BurstNumber=1;
	PTPDevicePropDesc dpd;
	PTPObjectInfo oi;
	int i;
	char *filename=NULL;
    
	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s\n",params.deviceinfo.Model);
	printf("\nInitiating direct captue...\n");
	
	if (params.deviceinfo.VendorExtensionID!=PTP_VENDOR_NIKON)
	{
	    printf ("Your camera is not Nikon!\nDo not buy from %s!\n",params.deviceinfo.Manufacturer);
	    goto out;
	}

	if (!ptp_operation_issupported(&params,PTP_OC_NIKON_DirectCapture)) {
	    printf ("Sorry, your camera dows not support Nikon DirectCapture!\nDo not buy from %s!\n",params.deviceinfo.Manufacturer);
	    goto out;
	}

	/* obtain exposure time in miliseconds */
	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(&params,PTP_DPC_ExposureTime,&dpd);
	if (result==PTP_RC_OK) ExposureTime=(*(int32_t*)(dpd.CurrentValue))/10;

	/* obtain burst number */
	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(&params,PTP_DPC_BurstNumber,&dpd);
	if (result==PTP_RC_OK) BurstNumber=*(uint16_t*)(dpd.CurrentValue);

	if (BurstNumber>1) printf("Capturing %i frames in burst.\n",BurstNumber);
#if 0
	/* sleep in case of exposure longer than 1/100 */
	if (ExposureTime>10) {
	    printf ("sleeping %i miliseconds\n", ExposureTime);
	    usleep (ExposureTime*1000);
	}
#endif

	while (num>0) {
	    /* perform direct capture */
	    result=ptp_nikon_directcapture (&params, 0xffffffff);
	    if (result!=PTP_RC_OK) {
	        if (result==PTP_ERROR_IO) {
	    	close_camera(&ptp_usb, &params, dev);
	    	return;
	        }
	    }

#if 0	    /* Is this really needed??? */
	    ptp_nikon_keepalive(&params);
#endif
	    ptp_nikon_keepalive(&params);
	    ptp_nikon_keepalive(&params);
	    ptp_nikon_keepalive(&params);
	    ptp_nikon_keepalive(&params);

	    result=ptp_nikon_checkevent (&params, &events, &nevent);
	    if (result != PTP_RC_OK) goto out;
	    
	    for(i=0;i<nevent;i++) {
		printf("Event [%i] = 0x%04x,\t param: %08x\n",i, events[i].code ,events[i].param1);
		if (events[i].code==PTP_EC_NIKON_ObjectReady) 
		{
		    num--;
		    if ((result=ptp_getobjectinfo(&params,0xffff0001, &oi))!=PTP_RC_OK) {
		        fprintf(stderr, "Could not get object info\n");
		        ptp_perror(&params,result);
		        goto out;
		    }
		    if (filename==NULL) filename=oi.Filename;
		    save_object(&params, 0xffff0001, filename, oi, overwrite);
		}
		if (events[i].code==PTP_EC_NIKON_CaptureOverflow) {
		    printf("Ram cache overflow, capture terminated\n");
		    //BurstNumber=0;
		}
	    }
	    free (events);
	}

out:	
	ptpcam_usb_timeout=USB_TIMEOUT;
	close_camera(&ptp_usb, &params, dev);
#endif
}




void
list_files (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	int i;
	PTPObjectInfo oi;
	struct tm *tm;

	printf("\nListing files...\n");
	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);
	CR(ptp_getobjecthandles (&params,0xffffffff, 0x000000, 0x000000,
		&params.handles),"Could not get object handles\n");
	printf("Handler:           Size: \tCaptured:      \tname:\n");
	for (i = 0; i < params.handles.n; i++) {
		CR(ptp_getobjectinfo(&params,params.handles.Handler[i],
			&oi),"Could not get object info\n");
		if (oi.ObjectFormat == PTP_OFC_Association)
			continue;
		tm=gmtime(&oi.CaptureDate);
		printf("0x%08lx: %12u\t%4i-%02i-%02i %02i:%02i\t%s\n",
			(long unsigned)params.handles.Handler[i],
			(unsigned) oi.ObjectCompressedSize, 
			tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday,
			tm->tm_hour, tm->tm_min,
			oi.Filename);
	}
	printf("\n");
	close_camera(&ptp_usb, &params, dev);
}

void
delete_object (int busn, int devn, short force, uint32_t handle)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	PTPObjectInfo oi;

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	CR(ptp_getobjectinfo(&params,handle,&oi),
		"Could not get object info\n");
	CR(ptp_deleteobject(&params, handle,0), "Could not delete object\n");
	printf("\nObject 0x%08lx (%s) deleted.\n",(long unsigned) handle, oi.Filename);
	close_camera(&ptp_usb, &params, dev);
}

void
delete_all_files (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	PTPObjectInfo oi;
	uint32_t handle;
	int i;
	int ret;

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);
	CR(ptp_getobjecthandles (&params,0xffffffff, 0x000000, 0x000000,
		&params.handles),"Could not get object handles\n");

	for (i=0; i<params.handles.n; i++) {
		handle=params.handles.Handler[i];
		if ((ret=ptp_getobjectinfo(&params,handle, &oi))!=PTP_RC_OK){
			fprintf(stderr,"Handle: 0x%08lx\n",(long unsigned) handle);
			fprintf(stderr,"ERROR: Could not get object info\n");
			ptp_perror(&params,ret);
			if (ret==PTP_ERROR_IO) clear_stall(&ptp_usb);
			continue;
		}
		if (oi.ObjectFormat == PTP_OFC_Association)
			continue;
		CR(ptp_deleteobject(&params, handle,0),
				"Could not delete object\n");
		printf("Object 0x%08lx (%s) deleted.\n",(long unsigned) handle, oi.Filename);
	}
	close_camera(&ptp_usb, &params, dev);
}

void
save_object(PTPParams *params, uint32_t handle, char* filename, PTPObjectInfo oi, int overwrite)
{
	int file;
	char *image;
	int ret;
	struct utimbuf timebuf;

	file=open(filename, (overwrite==OVERWRITE_EXISTING?0:O_EXCL)|O_RDWR|O_CREAT|O_TRUNC,S_IRWXU|S_IRGRP);
	if (file==-1) {
		if (errno==EEXIST) {
			printf("Skipping file: \"%s\", file exists!\n",filename);
			goto out;
		}
		perror("open");
		goto out;
	}
	lseek(file,oi.ObjectCompressedSize-1,SEEK_SET);
	ret=write(file,"",1);
	if (ret==-1) {
	    perror("write");
	    goto out;
	}
	image=mmap(0,oi.ObjectCompressedSize,PROT_READ|PROT_WRITE,MAP_SHARED,
		file,0);
	if (image==MAP_FAILED) {
		perror("mmap");
		close(file);
		goto out;
	}
	printf ("Saving file: \"%s\" ",filename);
	fflush(NULL);
	ret=ptp_getobject(params,handle,&image);
	munmap(image,oi.ObjectCompressedSize);
	if (close(file)==-1) {
	    perror("close");
	}
	timebuf.actime=oi.ModificationDate;
	timebuf.modtime=oi.CaptureDate;
	utime(filename,&timebuf);
	if (ret!=PTP_RC_OK) {
		printf ("error!\n");
		ptp_perror(params,ret);
		if (ret==PTP_ERROR_IO) clear_stall((PTP_USB *)(params->data));
	} else {
		printf("is done.\n");
	}
out:
	return;
}

void
get_save_object (PTPParams *params, uint32_t handle, char* filename, int overwrite)
{

	PTPObjectInfo oi;
	int ret;

    	memset(&oi, 0, sizeof(PTPObjectInfo));
	if (verbose)
		printf ("Handle: 0x%08lx\n",(long unsigned) handle);
	if ((ret=ptp_getobjectinfo(params,handle, &oi))!=PTP_RC_OK) {
	    fprintf(stderr, "Could not get object info\n");
	    ptp_perror(params,ret);
	    if (ret==PTP_ERROR_IO) clear_stall((PTP_USB *)(params->data));
	    goto out;
	}
	if (oi.ObjectFormat == PTP_OFC_Association)
			goto out;
	if (filename==NULL) filename=(oi.Filename);

	save_object(params, handle, filename, oi, overwrite);
out:
	return;

}

void
get_file (int busn, int devn, short force, uint32_t handle, char* filename,
int overwrite)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);

	get_save_object(&params, handle, filename, overwrite);

	close_camera(&ptp_usb, &params, dev);

}

void
get_all_files (int busn, int devn, short force, int overwrite)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	int i;

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);

	CR(ptp_getobjecthandles (&params,0xffffffff, 0x000000, 0x000000,
		&params.handles),"Could not get object handles\n");

	for (i=0; i<params.handles.n; i++) {
	    get_save_object (&params, params.handles.Handler[i], NULL,
		    overwrite);
	}
	close_camera(&ptp_usb, &params, dev);
}

void
send_generic_request (int busn, int devn, uint16_t reqCode, uint32_t *reqParams, uint32_t direction, char *data_file)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	char *data = NULL;
	long fsize = 0;

	if (direction == PTP_DP_SENDDATA) { // read data from file
		if (strncmp(data_file, "0x", 2) == 0) {
		    uint len = strlen(data_file);
			char num[3];
			uint i;
			data = (char*)calloc(1,len / 2);

			num[2] = 0;
			for (i = 2; i < len ; i += 2) {
				num[1] = data_file[i];
				if (i < len-1) {
					num[0] = data_file[i];
				    num[1] = data_file[i+1];
				}
				else {
				    num[0] = data_file[i];
					num[1] = 0;
				}
				data[fsize] = (char)strtol(num, NULL, 16);
				++fsize;
			}
		}
		else {
		    FILE *f = fopen(data_file, "r");
			if (f) {
				fseek(f, 0, SEEK_END);
				fsize = ftell(f);
				fseek(f, 0, SEEK_SET);
				data = (char*)calloc(1,fsize + 1);
				if (fread(data, 1, fsize, f) != fsize) {
					fprintf(stderr, "PTP: ERROR: can't read data to send from file '%s'\n", data_file);
					free (data);
					return;
				}
				else {
					printf("--- data to send ---\n");
					display_hexdump(data, fsize);
					printf("--------------------\n");
				}
			} else { // error no data to send
				fprintf(stderr, "PTP: ERROR: file not found '%s'\n", data_file);
				return;
			}
		}
	}

	if (open_camera(busn, devn, 0, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);

	printf("Sending generic request: reqCode=0x%04x, params=[0x%08x,0x%08x,0x%08x,0x%08x,0x%08x]\n",
			(uint)reqCode, reqParams[0], reqParams[1], reqParams[2], reqParams[3], reqParams[4]);
	uint16_t result=ptp_sendgenericrequest (&params, reqCode, reqParams, &data, direction, fsize);
	if((result)!=PTP_RC_OK) {
		ptp_perror(&params,result);
		if (result > 0x2000)
			fprintf(stderr,"PTP: ERROR: response 0x%04x\n", result);
	} else {
	    if (data != NULL && direction == PTP_DP_GETDATA) {
		    display_hexdump(data, malloc_usable_size ((void*)data));
			free(data);
		}
		printf("PTP: response OK\n");
	}
	if (data != NULL && direction == PTP_DP_SENDDATA) {
		free(data);
	}
	close_camera(&ptp_usb, &params, dev);
	return;
	
}

void
list_operations (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	int i;
	const char* name;

	printf("\nListing supported operations...\n");

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);
	for (i=0; i<params.deviceinfo.OperationsSupported_len; i++)
	{
		name=ptp_get_operation_name(&params,
			params.deviceinfo.OperationsSupported[i]);

		if (name==NULL)
			printf("  0x%04x: UNKNOWN\n",
				params.deviceinfo.OperationsSupported[i]);
		else
			printf("  0x%04x: %s\n",
				params.deviceinfo.OperationsSupported[i],name);
	}
	close_camera(&ptp_usb, &params, dev);

}

void
list_properties (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	const char* propname;
	int i;

	printf("\nListing properties...\n");

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;
	printf("Camera: %s\n",params.deviceinfo.Model);
	for (i=0; i<params.deviceinfo.DevicePropertiesSupported_len;i++){
		propname=ptp_prop_getname(&params,
			params.deviceinfo.DevicePropertiesSupported[i]);
		if (propname!=NULL) 
			printf("  0x%04x: %s\n",
				params.deviceinfo.DevicePropertiesSupported[i],
				propname);
		else
			printf("  0x%04x: UNKNOWN\n",
				params.deviceinfo.DevicePropertiesSupported[i]);
	}
	close_camera(&ptp_usb, &params, dev);
}

short
print_propval (uint16_t datatype, void* value, short hex);
short
print_propval (uint16_t datatype, void* value, short hex)
{
	switch (datatype) {
		case PTP_DTC_INT8:
			printf("%hhi",*(char*)value);
			return 0;
		case PTP_DTC_UINT8:
			printf("%hhu",*(unsigned char*)value);
			return 0;
		case PTP_DTC_INT16:
			printf("%hi",*(int16_t*)value);
			return 0;
		case PTP_DTC_UINT16:
			if (hex==PTPCAM_PRINT_HEX)
				printf("0x%04hX (%hi)",*(uint16_t*)value,
					*(uint16_t*)value);
			else
				printf("%hi",*(uint16_t*)value);
			return 0;
		case PTP_DTC_INT32:
			printf("%li",(long int)*(int32_t*)value);
			return 0;
		case PTP_DTC_UINT32:
			if (hex==PTPCAM_PRINT_HEX)
				printf("0x%08lX (%lu)",
					(long unsigned) *(uint32_t*)value,
					(long unsigned) *(uint32_t*)value);
			else
				printf("%lu",(long unsigned)*(uint32_t*)value);
			return 0;
		case PTP_DTC_STR:
			printf("\"%s\"",(char *)value);
	}
	return -1;
}

uint16_t
set_property (PTPParams* params,
		uint16_t property, const char* value, uint16_t datatype);
uint16_t
set_property (PTPParams* params,
		uint16_t property, const char* value, uint16_t datatype)
{
	void* val=NULL;

	switch(datatype) {
	case PTP_DTC_INT8:
		val=malloc(sizeof(int8_t));
		*(int8_t*)val=(int8_t)strtol(value,NULL,0);
		break;
	case PTP_DTC_UINT8:
		val=malloc(sizeof(uint8_t));
		*(uint8_t*)val=(uint8_t)strtol(value,NULL,0);
		break;
	case PTP_DTC_INT16:
		val=malloc(sizeof(int16_t));
		*(int16_t*)val=(int16_t)strtol(value,NULL,0);
		break;
	case PTP_DTC_UINT16:
		val=malloc(sizeof(uint16_t));
		*(uint16_t*)val=(uint16_t)strtol(value,NULL,0);
		break;
	case PTP_DTC_INT32:
		val=malloc(sizeof(int32_t));
		*(int32_t*)val=(int32_t)strtol(value,NULL,0);
		break;
	case PTP_DTC_UINT32:
		val=malloc(sizeof(uint32_t));
		*(uint32_t*)val=(uint32_t)strtol(value,NULL,0);
		break;
	case PTP_DTC_STR:
		val=(void *)value;
	}
	return(ptp_setdevicepropvalue(params, property, val, datatype));
	free(val);
	return 0;
}
void
getset_property_internal (PTPParams* params, uint16_t property,const char* value, short force);
void
getset_property_internal (PTPParams* params, uint16_t property,const char* value, short force)
{
	PTPDevicePropDesc dpd;
	const char* propname;
	const char *propdesc;
	uint16_t result;


	memset(&dpd,0,sizeof(dpd));
	result=ptp_getdevicepropdesc(params,property,&dpd);
	if (result!=PTP_RC_OK&&!force) {
		ptp_perror(params,result); 
		fprintf(stderr,"ERROR: "
		"Could not get device property description!\n"
		"Try to reset the camera.\n");
		return ;
	}
	/* until this point dpd has to be free()ed */
	propdesc=ptp_prop_getdesc(params, &dpd, NULL);
	propname=ptp_prop_getname(params,property);

	if (value==NULL) { /* property GET */
		if (!verbose) { /* short output, default */
			printf("'%s' is set to: ", propname==NULL?"UNKNOWN":propname);
			if (propdesc!=NULL)
				printf("[%s]", propdesc);
			else 
			{
				if (dpd.FormFlag==PTP_DPFF_Enumeration)
					PRINT_PROPVAL_HEX(dpd.CurrentValue);
				else 
					PRINT_PROPVAL_DEC(dpd.CurrentValue);
			}
			printf("\n");
	
		} else { /* verbose output */
	
			printf("%s: [0x%04x, ",propname==NULL?"UNKNOWN":propname,
					property);
			if (dpd.GetSet==PTP_DPGS_Get)
				printf ("readonly, ");
			else
				printf ("readwrite, ");
			printf ("%s] ",
				ptp_get_datatype_name(params, dpd.DataType));

			printf ("\n  Current value: ");
			if (dpd.FormFlag==PTP_DPFF_Enumeration)
				PRINT_PROPVAL_HEX(dpd.CurrentValue);
			else 
				PRINT_PROPVAL_DEC(dpd.CurrentValue);

			if (propdesc!=NULL)
				printf(" [%s]", propdesc);
			printf ("\n  Factory value: ");
			if (dpd.FormFlag==PTP_DPFF_Enumeration)
				PRINT_PROPVAL_HEX(dpd.FactoryDefaultValue);
			else 
				PRINT_PROPVAL_DEC(dpd.FactoryDefaultValue);
			propdesc=ptp_prop_getdesc(params, &dpd,
						dpd.FactoryDefaultValue);
			if (propdesc!=NULL)
				printf(" [%s]", propdesc);
			printf("\n");

			switch (dpd.FormFlag) {
			case PTP_DPFF_Enumeration:
				{
					int i;
					printf ("Enumerated:\n");
					for(i=0;i<dpd.FORM.Enum.NumberOfValues;i++){
						PRINT_PROPVAL_HEX(
						dpd.FORM.Enum.SupportedValue[i]);
						propdesc=ptp_prop_getdesc(params, &dpd, dpd.FORM.Enum.SupportedValue[i]);
						if (propdesc!=NULL) printf("\t[%s]", propdesc);
						printf("\n");
					}
				}
				break;
			case PTP_DPFF_Range:
				printf ("Range [");
				PRINT_PROPVAL_DEC(dpd.FORM.Range.MinimumValue);
				printf(" - ");
				PRINT_PROPVAL_DEC(dpd.FORM.Range.MaximumValue);
				printf("; step ");
				PRINT_PROPVAL_DEC(dpd.FORM.Range.StepSize);
				printf("]\n");
				break;
			case PTP_DPFF_None:
				break;
			}
		}
	} else {
		uint16_t r;
		propdesc=ptp_prop_getdesc(params, &dpd, NULL);
		printf("'%s' is set to: ", propname==NULL?"UNKNOWN":propname);
		if (propdesc!=NULL)
			printf("[%s]", propdesc);
		else
		{
			if (dpd.FormFlag==PTP_DPFF_Enumeration)
				PRINT_PROPVAL_HEX(dpd.CurrentValue);
			else 
				PRINT_PROPVAL_DEC(dpd.CurrentValue);
		}
		printf("\n");

		propdesc=ptp_prop_getdescbystring(params, &dpd, value);
/*
		if (propdesc==NULL) {
			fprintf(stderr, "ERROR: Unable to set property to unidentified value: '%s'\n",
				value);
			goto out;
		}
*/
		printf("Changing property value to %s [%s] ",
			value,propdesc);
		r=(set_property(params, property, value, dpd.DataType));
		if (r!=PTP_RC_OK)
		{
			printf ("FAILED!!!\n");
			fflush(NULL);
		        ptp_perror(params,r);
		}
		else 
			printf ("succeeded.\n");
	}
/*	out: */

	ptp_free_devicepropdesc(&dpd);
}

void
getset_propertybyname (int busn,int devn,char* property,char* value,short force);
void
getset_propertybyname (int busn,int devn,char* property,char* value,short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	char *p;
	uint16_t dpc;
	const char *propval=NULL;

	printf("\n");

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s",params.deviceinfo.Model);
	if ((devn!=0)||(busn!=0)) 
		printf(" (bus %i, dev %i)\n",busn,devn);
	else
		printf("\n");

	if (property==NULL) {
		fprintf(stderr,"ERROR: no such property\n");
		return;
	}
		
	/* 
	1. change all '-' in property and value to ' '
	2. change all '  ' in property and value to '-'
	3. get property code by name
	4. get value code by name
	5. set property
	*/
	while ((p=index(property,'-'))!=NULL) {
		*p=' ';
	}

	dpc=ptp_prop_getcodebyname(&params, property);
	if (dpc==0) {
		fprintf(stderr, "ERROR: Could not find property '%s'\n",
			property);
		close_camera(&ptp_usb, &params, dev);
		return;
	}

	if (!ptp_property_issupported(&params, dpc))
	{
		fprintf(stderr,"The device does not support this property!\n");
		close_camera(&ptp_usb, &params, dev);
		return;
	}

	if (value!=NULL) {
		while ((p=index(value,'-'))!=NULL) {
			*p=' ';
		}
		propval=ptp_prop_getvalbyname(&params, value, dpc);
		if (propval==NULL) propval=value;
	}

	getset_property_internal (&params, dpc,propval, force);

	close_camera(&ptp_usb, &params, dev);
}

void
getset_property (int busn,int devn,uint16_t property,char* value,short force);
void
getset_property (int busn,int devn,uint16_t property,char* value,short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;

	printf ("\n");

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s",params.deviceinfo.Model);
	if ((devn!=0)||(busn!=0)) 
		printf(" (bus %i, dev %i)\n",busn,devn);
	else
		printf("\n");
	if (!ptp_property_issupported(&params, property)&&!force)
	{
		fprintf(stderr,"The device does not support this property!\n");
		close_camera(&ptp_usb, &params, dev);
		return;
	}

	getset_property_internal (&params, property,value, force);

	close_camera(&ptp_usb, &params, dev);
}

void
getset_property_value (int busn,int devn,uint16_t property,char* value,short force);
void
getset_property_value (int busn,int devn,uint16_t property,char* value,short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;

	printf ("\n");

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s",params.deviceinfo.Model);
	if ((devn!=0)||(busn!=0)) 
		printf(" (bus %i, dev %i)\n",busn,devn);
	else
		printf("\n");
	if (!ptp_property_issupported(&params, property)&&!force)
	{
		fprintf(stderr,"The device does not support this property!\n");
		close_camera(&ptp_usb, &params, dev);
		return;
	}

	

/* Transaction data phase description */
#define PTP_DP_NODATA		0x0000	/* no data phase */
#define PTP_DP_SENDDATA		0x0001	/* sending data */
#define PTP_DP_GETDATA		0x0002	/* receiving data */
#define PTP_DP_DATA_MASK	0x00ff	/* data phase mask */
#define PTP_CNT_INIT(cnt) {memset(&cnt,0,sizeof(cnt));}
	PTPContainer ptp;
	uint16_t ret;
	char* dpv=NULL;

	PTP_CNT_INIT(ptp);
	ptp.Code=PTP_OC_GetDevicePropValue;
	ptp.Param1=property;
	ptp.Nparam=1;
	ret=ptp_transaction(&params, &ptp, PTP_DP_GETDATA, 0, &dpv);
	//result =ptp_getdevicepropvalue (&params, property, &prop, PTP_DTC_UINT8);

	if (ret!=PTP_RC_OK) {
		ptp_perror(&params,ret); 
		fprintf(stderr,"ERROR: "
		"Could not get device property description!\n"
		"Try to reset the camera.\n");
		if (!force) return ;
	}
	if (dpv) printf("%x is set to: %08x\n", property, *((uint8_t*)dpv));

	close_camera(&ptp_usb, &params, dev);
}


void
show_all_properties (int busn,int devn,short force, int unknown);
void
show_all_properties (int busn,int devn,short force, int unknown)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	PTPDevicePropDesc dpd;
	const char* propname;
	const char *propdesc;
	int i;

	printf ("\n");

	if (open_camera(busn, devn, force, &ptp_usb, &params, &dev)<0)
		return;

	printf("Camera: %s",params.deviceinfo.Model);
	if ((devn!=0)||(busn!=0)) 
		printf(" (bus %i, dev %i)\n",busn,devn);
	else
		printf("\n");

	for (i=0; i<params.deviceinfo.DevicePropertiesSupported_len;i++) {
		propname=ptp_prop_getname(&params,
				params.deviceinfo.DevicePropertiesSupported[i]);
		if ((unknown) && (propname!=NULL)) continue;

		printf("  0x%04x: ",
				params.deviceinfo.DevicePropertiesSupported[i]);
		memset(&dpd,0,sizeof(dpd));
		CR(ptp_getdevicepropdesc(&params,
			params.deviceinfo.DevicePropertiesSupported[i],&dpd),
			"Could not get device property description!\n"
			"Try to reset the camera.\n");
		propdesc= ptp_prop_getdesc(&params, &dpd, NULL);

		if (verbose) {
			printf ("(%s",propname==NULL?"UNKNOWN":propname);
			if (propdesc!=NULL)
				printf(": %s) ",propdesc);
			else
				printf(") ");
		}
		PRINT_PROPVAL_DEC(dpd.CurrentValue);
	
		printf("\n");
		ptp_free_devicepropdesc(&dpd);
	}

	close_camera(&ptp_usb, &params, dev);
}

int
usb_get_endpoint_status(PTP_USB* ptp_usb, int ep, uint16_t* status)
{
	 return (usb_control_msg(ptp_usb->handle,
		USB_DP_DTH|USB_RECIP_ENDPOINT, USB_REQ_GET_STATUS,
		USB_FEATURE_HALT, ep, (char *)status, 2, 3000));
}

int
usb_clear_stall_feature(PTP_USB* ptp_usb, int ep)
{

	return (usb_control_msg(ptp_usb->handle,
		USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE, USB_FEATURE_HALT,
		ep, NULL, 0, 3000));
}

int
usb_ptp_get_device_status(PTP_USB* ptp_usb, uint16_t* devstatus);
int
usb_ptp_get_device_status(PTP_USB* ptp_usb, uint16_t* devstatus)
{
	return (usb_control_msg(ptp_usb->handle,
		USB_DP_DTH|USB_TYPE_CLASS|USB_RECIP_INTERFACE,
		USB_REQ_GET_DEVICE_STATUS, 0, 0,
		(char *)devstatus, 4, 3000));
}

int
usb_ptp_device_reset(PTP_USB* ptp_usb);
int
usb_ptp_device_reset(PTP_USB* ptp_usb)
{
	return (usb_control_msg(ptp_usb->handle,
		USB_TYPE_CLASS|USB_RECIP_INTERFACE,
		USB_REQ_DEVICE_RESET, 0, 0, NULL, 0, 3000));
}

void
reset_device (int busn, int devn, short force);
void
reset_device (int busn, int devn, short force)
{
	PTPParams params;
	PTP_USB ptp_usb;
	struct usb_device *dev;
	uint16_t status;
	uint16_t devstatus[2] = {0,0};
	int ret;

#ifdef DEBUG
	printf("dev %i\tbus %i\n",devn,busn);
#endif
	dev=find_device(busn,devn,force);
	if (dev==NULL) {
		fprintf(stderr,"could not find any device matching given "
		"bus/dev numbers\n");
		exit(-1);
	}
	find_endpoints(dev,&ptp_usb.inep,&ptp_usb.outep,&ptp_usb.intep);

	init_ptp_usb(&params, &ptp_usb, dev);
	
	/* get device status (devices likes that regardless of its result)*/
	usb_ptp_get_device_status(&ptp_usb,devstatus);
	
	/* check the in endpoint status*/
	ret = usb_get_endpoint_status(&ptp_usb,ptp_usb.inep,&status);
	if (ret<0) perror ("usb_get_endpoint_status()");
	/* and clear the HALT condition if happend*/
	if (status) {
		printf("Resetting input pipe!\n");
		ret=usb_clear_stall_feature(&ptp_usb,ptp_usb.inep);
		if (ret<0)perror ("usb_clear_stall_feature()");
	}
	status=0;
	/* check the out endpoint status*/
	ret = usb_get_endpoint_status(&ptp_usb,ptp_usb.outep,&status);
	if (ret<0) perror ("usb_get_endpoint_status()");
	/* and clear the HALT condition if happend*/
	if (status) {
		printf("Resetting output pipe!\n");
		ret=usb_clear_stall_feature(&ptp_usb,ptp_usb.outep);
		if (ret<0)perror ("usb_clear_stall_feature()");
	}
	status=0;
	/* check the interrupt endpoint status*/
	ret = usb_get_endpoint_status(&ptp_usb,ptp_usb.intep,&status);
	if (ret<0)perror ("usb_get_endpoint_status()");
	/* and clear the HALT condition if happend*/
	if (status) {
		printf ("Resetting interrupt pipe!\n");
		ret=usb_clear_stall_feature(&ptp_usb,ptp_usb.intep);
		if (ret<0)perror ("usb_clear_stall_feature()");
	}

	/* get device status (now there should be some results)*/
	ret = usb_ptp_get_device_status(&ptp_usb,devstatus);
	if (ret<0) 
		perror ("usb_ptp_get_device_status()");
	else	{
		if (devstatus[1]==PTP_RC_OK) 
			printf ("Device status OK\n");
		else
			printf ("Device status 0x%04x\n",devstatus[1]);
	}
	
	/* finally reset the device (that clears prevoiusly opened sessions)*/
	ret = usb_ptp_device_reset(&ptp_usb);
	if (ret<0)perror ("usb_ptp_device_reset()");
	/* get device status (devices likes that regardless of its result)*/
	usb_ptp_get_device_status(&ptp_usb,devstatus);

	close_usb(&ptp_usb, dev);

}


uint16_t
ptp_transaction_nodata (PTPParams* params, PTPContainer* ptp);
uint16_t
ptp_transaction_nodata (PTPParams* params, PTPContainer* ptp)
{
	return (ptp_transaction(params, ptp, PTP_DP_NODATA, 0, 0));
}

uint16_t
ptp_transaction_getdata (PTPParams* params, PTPContainer* ptp, unsigned int *getlen, char** data);
uint16_t
ptp_transaction_getdata (PTPParams* params, PTPContainer* ptp, unsigned int *getlen, char** data)
{
	return (ptp_transaction(params, ptp, PTP_DP_GETDATA, (unsigned int) getlen, data));
}

uint16_t
ptp_transaction_senddata (PTPParams* params, PTPContainer* ptp, unsigned int sendlen, char* data);
uint16_t
ptp_transaction_senddata (PTPParams* params, PTPContainer* ptp, unsigned int sendlen, char* data)
{
	return (ptp_transaction(params, ptp, PTP_DP_SENDDATA, sendlen, &data));
}

void ptphack()
{
	PTPParams params={};
	PTP_USB ptp_usb={};
	struct usb_device *dev;
	PTPContainer ptp={};
	int getlen=0;

	char* data=NULL;

	PTPDeviceInfo 	PTPDeviceInfo={};
	PTPStorageIDs 	PTPStorageIDs={};
	PTPStorageInfo 	PTPStorageInfo={};
	PTPObjectHandles 	PTPObjectHandles={};
	PTPObjectInfo	PTPObjectInfo={};
	PTPPropDescRangeForm 	PTPPropDescRangeForm={};
	PTPPropDescEnumForm 	PTPPropDescEnumForm={};
	PTPDevicePropDesc 	PTPDevicePropDesc={};
	PTPCANONFolderEntry 	PTPCANONFolderEntry={};

	if (open_camera(0, 0, 0, &ptp_usb, &params, &dev)<0)
		return;

	raise(SIGINT);

	//ptp_transaction(&params, &ptp, PTP_DP_GETDATA, 0, &dpv);
	/*
	 * ptp_transaction_nodata(&params, &ptp)
	 * ptp_transaction_getdata(&params, &ptp, &getlen, &data)
	 * ptp_transaction_senddata(&params, &ptp, sendlen, data)
	 *
	 */
	
	close_camera(&ptp_usb, &params, dev);
}

/* main program  */

int
main(int argc, char ** argv)
{
	int busn=0,devn=0;
	int action=0;
	short force=0;
	int overwrite=SKIP_IF_EXISTS;
	uint16_t property=0;
	char* value=NULL;
	char* propstr=NULL;
	uint32_t handle=0;
	char *filename=NULL;
	int num=0;
	int interval=0;
	uint16_t reqCode=0;
	uint32_t reqParams[5];
	uint32_t direction=PTP_DP_GETDATA;
	char data_file[256];
	/* parse options */
	int option_index = 0,opt;
	static struct option loptions[] = {
		{"help",0,0,'h'},
		{"bus",1,0,0},
		{"dev",1,0,0},
		{"reset",0,0,'r'},
		{"list-devices",0,0,'l'},
		{"list-files",0,0,'L'},
		{"list-operations",1,0,'o'},
		{"list-properties",0,0,'p'},
		{"show-all-properties",0,0,0},
		{"show-unknown-properties",0,0,0},
		{"show-property",1,0,'s'},
		{"set-property",1,0,'s'},
		{"set",1,0,0},
		{"get-file",1,0,'g'},
		{"get-all-files",0,0,'G'},
		{"capture",0,0,'c'},
		{"nikon-dc",0,0,0},
		{"ndc",0,0,0},
		{"nikon-ic",0,0,0},
		{"nic",0,0,0},
		{"nikon-dc2",0,0,0},
		{"ndc2",0,0,0},
		{"loop-capture",1,0,0},
		{"interval",1,0,0},
		{"delete-object",1,0,'d'},
		{"delete-all-files",1,0,'D'},
		{"info",0,0,'i'},
		{"val",1,0,0},
		{"filename",1,0,0},
		{"overwrite",0,0,0},
		{"force",0,0,'f'},
		{"verbose",2,0,'v'},
		{0,0,0,0}
	};

	/* register signal handlers */
	signal(SIGINT, ptpcam_siginthandler);
	
	while(1) {
		opt = getopt_long (argc, argv, "LhlcipfroGg:Dd:s:v::R:", loptions, &option_index);
		if (opt==-1) break;
	
		switch (opt) {
		/* set parameters */
		case 0:
			if (!(strcmp("val",loptions[option_index].name)))
				value=strdup(optarg);
			if (!(strcmp("filename",loptions[option_index].name)))
				filename=strdup(optarg);
			if (!(strcmp("overwrite",loptions[option_index].name)))
				overwrite=OVERWRITE_EXISTING;
			if (!(strcmp("bus",loptions[option_index].name)))
				busn=strtol(optarg,NULL,10);
			if (!(strcmp("dev",loptions[option_index].name)))
				devn=strtol(optarg,NULL,10);
			if (!(strcmp("loop-capture",loptions[option_index].name)))
			{
				action=ACT_LOOP_CAPTURE;
				num=strtol(optarg,NULL,10);
			}
			if (!(strcmp("show-all-properties", loptions[option_index].name)))
				action=ACT_SHOW_ALL_PROPERTIES;
			if (!(strcmp("show-unknown-properties", loptions[option_index].name)))
				action=ACT_SHOW_UNKNOWN_PROPERTIES;
			if (!(strcmp("set",loptions[option_index].name)))
			{
				propstr=strdup(optarg);
				action=ACT_SET_PROPBYNAME;
			}
			if (!(strcmp("interval",loptions[option_index].name)))
				interval=strtol(optarg,NULL,10);
			if (!strcmp("nikon-dc", loptions[option_index].name) ||
			    !strcmp("ndc", loptions[option_index].name))
			{
			    action=ACT_NIKON_DC;
			}
			if (!strcmp("nikon-ic", loptions[option_index].name) ||
			    !strcmp("nic", loptions[option_index].name))
			{
			    action=ACT_NIKON_IC;
			}
			if (!strcmp("nikon-dc2", loptions[option_index].name) ||
			    !strcmp("ndc2", loptions[option_index].name))
			{
			    action=ACT_NIKON_DC2;
			}
			break;
		case 'f':
			force=~force;
			break;
		case 'v':
			if (optarg) 
				verbose=strtol(optarg,NULL,10);
			else
				verbose=1;
			/*printf("VERBOSE LEVEL  = %i\n",verbose);*/
			break;
		/* actions */
		case 'h':
			help();
			break;
		case 'r':
			action=ACT_DEVICE_RESET;
			break;
		case 'l':
			action=ACT_LIST_DEVICES;
			break;
		case 'p':
			action=ACT_LIST_PROPERTIES;
			break;
		case 's':
			action=ACT_GETSET_PROPERTY;
			property=strtol(optarg,NULL,16);
			break;
		case 'o':
			action=ACT_LIST_OPERATIONS;
			break;
		case 'i':
			action=ACT_SHOW_INFO;
			break;
		case 'c':
			action=ACT_CAPTURE;
			break;
		case 'L':
			action=ACT_LIST_FILES;
			break;
		case 'g':
			action=ACT_GET_FILE;
			handle=strtol(optarg,NULL,16);
			break;
		case 'G':
			action=ACT_GET_ALL_FILES;
			break;
		case 'd':
			action=ACT_DELETE_OBJECT;
			handle=strtol(optarg,NULL,16);
			break;
		case 'D':
			action=ACT_DELETE_ALL_FILES;
			break;
		case 'R':
			action=ACT_GENERIC_REQ;
			memset((void*)reqParams, 0, sizeof(uint32_t) * 5);
			memset((void*)data_file, 0, 256);
			sscanf(optarg, "%x,%x,%x,%x,%x,%x,%s", (uint*)&reqCode, &reqParams[0], &reqParams[1],
				   &reqParams[2], &reqParams[3], &reqParams[4], data_file);
			if (data_file[0] == 'r') {
			  direction = PTP_DP_GETDATA;
			} else if (data_file[0] == 'n') {
			  direction = PTP_DP_NODATA;
			} else if (strlen(data_file)>0) {
			  direction = PTP_DP_SENDDATA;
			  printf("data to send : '%s'\n", data_file);
			}
			break;
		case '?':
			break;
		default:
			fprintf(stderr,"getopt returned character code 0%o\n",
				opt);
			break;
		}
	}
	if (argc==1) {
		usage();
		return 0;
	}
	switch (action) {
		case ACT_DEVICE_RESET:
			reset_device(busn,devn,force);
			break;
		case ACT_LIST_DEVICES:
			list_devices(force);
			break;
		case ACT_LIST_PROPERTIES:
			list_properties(busn,devn,force);
			break;
		case ACT_GETSET_PROPERTY:
			//getset_property_value(busn,devn,property,value,force);
			getset_property(busn,devn,property,value,force);
			break;
		case ACT_SHOW_INFO:
			show_info(busn,devn,force);
			break;
		case ACT_LIST_OPERATIONS:
			list_operations(busn,devn,force);
			break;
		case ACT_LIST_FILES:
			list_files(busn,devn,force);
			break;
		case ACT_GET_FILE:
			get_file(busn,devn,force,handle,filename,overwrite);
			break;
		case ACT_GENERIC_REQ:
		    send_generic_request (busn, devn, reqCode, reqParams, direction, data_file);
			break;
		case ACT_GET_ALL_FILES:
			get_all_files(busn,devn,force,overwrite);
			break;
		case ACT_CAPTURE:
			capture_image(busn,devn,force);
			break;
		case ACT_DELETE_OBJECT:
			delete_object(busn,devn,force,handle);
			break;
		case ACT_DELETE_ALL_FILES:
			delete_all_files(busn,devn,force);
			break;
		case ACT_LOOP_CAPTURE:
			loop_capture (busn,devn,force,num,interval, overwrite);
			break;
		case ACT_SHOW_ALL_PROPERTIES:
			show_all_properties(busn,devn,force,0);
			break;
		case ACT_SHOW_UNKNOWN_PROPERTIES:
			show_all_properties(busn,devn,force,1);
			break;
		case ACT_SET_PROPBYNAME:
			getset_propertybyname(busn,devn,propstr,value,force);
			break;
		case ACT_NIKON_DC:
			nikon_direct_capture(busn,devn,force,filename,overwrite);
			break;
		case ACT_NIKON_IC:
			nikon_initiate_dc (busn,devn,force);
			break;
		case ACT_NIKON_DC2:
			nikon_direct_capture2(busn,devn,force,filename,overwrite);
	}

	return 0;
}
