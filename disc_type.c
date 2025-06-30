#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>
#include <libbluray/bluray.h>

// Default DVD device
#if defined (__linux__)
#include <linux/cdrom.h>
#define DEFAULT_DVD_DEVICE "/dev/sr0"
#elif defined (__DragonFly__)
#define DEFAULT_DVD_DEVICE "/dev/cd0"
#elif defined (__FreeBSD__)
#define DEFAULT_DVD_DEVICE "/dev/cd0"
#elif defined (__NetBSD__)
#define DEFAULT_DVD_DEVICE "/dev/cd0d"
#elif defined (__OpenBSD__)
#define DEFAULT_DVD_DEVICE "/dev/rcd0c"
#elif defined (__APPLE__) && defined (__MACH__)
#define DEFAULT_DVD_DEVICE "/dev/disk1"
#elif defined (__CYGWIN__)
#define DEFAULT_DVD_DEVICE "D:\\"
#elif defined(_WIN32)
#define DEFAULT_DVD_DEVICE "D:\\"
#else
#define DEFAULT_DVD_DEVICE "/dev/dvd"
#endif

void dvd_info_logger_cb(void *p, dvd_logger_level_t dvdread_log_level, const char *msg, va_list dvd_log_va);

void dvd_info_logger_cb(void *p, dvd_logger_level_t dvdread_log_level, const char *msg, va_list dvd_log_va) {
	return;
}

void print_usage();

void print_usage(void) {
	printf("disc_type [options] [device]\n");
	printf("  -h     this help output\n");
	printf("  -z     display debug output\n");
	printf("\n");
	printf("Default device is %s\n", DEFAULT_DVD_DEVICE);
}

int main(int argc, char **argv) {

	int opt;
	bool debug = false;
	bool exit_program = false;

	while ((opt = getopt(argc, argv, "hz")) != -1) {

		switch(opt) {
			case 'z':
				debug = true;
				break;
			case 'h':
			case '?':
				print_usage();
				exit_program = true;
				break;
			default:
				break;
		}

	}

	if(exit_program)
		return 0;

	char device_filename[PATH_MAX];
	memset(device_filename, '\0', PATH_MAX);

	if(argv[optind]) {
		strncpy(device_filename, argv[optind], PATH_MAX - 1);
	} else {
		strncpy(device_filename, DEFAULT_DVD_DEVICE, PATH_MAX - 1);
	}

	int retval;
	struct stat device_stat;

	retval = stat(device_filename, &device_stat);
	if(retval) {
		fprintf(stderr, "Could not open device %s\n", device_filename);
		return 1;
	}

	strcpy(device_filename, realpath(device_filename, NULL));

	if(S_ISBLK(device_stat.st_mode)) {

		int cdrom;

		cdrom = open(device_filename, O_RDONLY | O_NONBLOCK);
		if(cdrom < 0) {
			fprintf(stderr, "Could not open block device %s\n", device_filename);
			return 1;
		}

#if defined (__linux__)

		int drive_status;
		drive_status = ioctl(cdrom, CDROM_DRIVE_STATUS);

		if(drive_status != CDS_DISC_OK) {
			fprintf(stderr, "%s tray open\n", device_filename);
			return 1;
		}

		char system_command[PATH_MAX];
		int retval;

		memset(system_command, '\0', PATH_MAX);
		sprintf(system_command, "udevadm info %s | grep -q ID_CDROM_MEDIA_DVD=1", device_filename);

		if(debug)
			fprintf(stderr, "%s\n", system_command);
		retval = system(system_command);

		if(retval == 0) {
			printf("dvd\n");
			return 0;
		}

		memset(system_command, '\0', PATH_MAX);
		sprintf(system_command, "udevadm info %s | grep -q ID_CDROM_MEDIA_BD=1", device_filename);

		if(debug)
			fprintf(stderr, "%s\n", system_command);
		retval = system(system_command);

		if(retval == 0) {
			printf("bluray\n");
			return 0;
		}

		printf("unknown\n");
		return 1;

#endif

	}

	int fd;
	char directory[PATH_MAX];

	memset(directory, '\0', PATH_MAX);
	sprintf(directory, "%s/VIDEO_TS", device_filename);

	if(debug)
		fprintf(stderr, "open(\"%s\")\n", directory);

	if(open(directory, O_RDONLY) > 0) {
		printf("dvd\n");
		return 0;
	}

	memset(directory, '\0', PATH_MAX);
	sprintf(directory, "%s/BDMV", device_filename);

	if(debug)
		fprintf(stderr, "open(\"%s\")\n", directory);

	if(open(directory, O_RDONLY) > 0) {
		printf("bluray\n");
		return 0;
	}

	dvd_reader_t *dvdread_dvd = NULL;
	dvd_logger_cb dvdread_logger_cb = { dvd_info_logger_cb };

	if(debug)
		fprintf(stderr, "DVDOpen2(%s)\n", device_filename);

	dvdread_dvd = DVDOpen2(NULL, &dvdread_logger_cb, device_filename);

	ifo_handle_t *vmg_ifo = NULL;
	vmg_ifo = ifoOpen(dvdread_dvd, 0);

	if(vmg_ifo) {
		printf("dvd\n");
		return 0;
	}

	BLURAY *bd = NULL;

	if(debug)
		fprintf(stderr, "bd_open(%s)\n", device_filename);

	bd = bd_open(device_filename, NULL);

	if(bd) {
		printf("bluray\n");
		return 0;
	}

	printf("unknown\n");

	return 0;

}
