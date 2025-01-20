#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>
#include <libbluray/bluray.h>

void dvd_info_logger_cb(void *p, dvd_logger_level_t dvdread_log_level, const char *msg, va_list dvd_log_va);

void dvd_info_logger_cb(void *p, dvd_logger_level_t dvdread_log_level, const char *msg, va_list dvd_log_va) {

	return;

}

int main(int argc, char **argv) {

	bool debug = false;

	char device_filename[PATH_MAX];
	memset(device_filename, '\0', PATH_MAX);

	if(argc == 1)
		strcpy(device_filename, "/dev/sr0");

	if(argc == 2)
		strcpy(device_filename, realpath(argv[1], NULL));

	if(debug)
		fprintf(stderr, "%s\n", device_filename);

	if(argc > 2) {
		debug = true;
		return 0;
	}

	if(strstr(device_filename, "/dev/")) {

		int cdrom;
		int drive_status;

		cdrom = open(device_filename, O_RDONLY | O_NONBLOCK);
		if(cdrom < 0) {
			fprintf(stderr, "error opening %s\n", device_filename);
			return 1;
		}

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
