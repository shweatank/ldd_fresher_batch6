#ifndef IOCTL_HEADER
#define IOCTL_HEADER

#define IOCTL_MAGIC 'A'

#define SET_MODE _IOW(IOCTL_MAGIC, 1, int)
#define GET_MODE _IOR(IOCTL_MAGIC, 2, int)

#define CLEAR_BUFFER _IO(IOCTL_MAGIC, 3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC, 4, int)

struct control_io
{
        int device_mode;
        char buffer[256];
	int buf_len;
        int wr_counter;
};

#endif
