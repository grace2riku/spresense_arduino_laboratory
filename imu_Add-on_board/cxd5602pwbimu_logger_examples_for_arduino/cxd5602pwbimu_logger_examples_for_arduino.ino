#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <nuttx/sensors/cxd5602pwbimu.h>
#include <arch/board/cxd56_cxd5602pwbimu.h>

#define CXD5602PWBIMU_DEVPATH      "/dev/imu0"
#define MAX_NFIFO (4)

static cxd5602pwbimu_data_t g_data[MAX_NFIFO];

static int start_sensing(int fd, int rate, int adrange, int gdrange,
                         int nfifos)
{
  cxd5602pwbimu_range_t range;

  ioctl(fd, SNIOC_SSAMPRATE, rate);
  range.accel = adrange;
  range.gyro = gdrange;
  ioctl(fd, SNIOC_SDRANGE, (unsigned long)(uintptr_t)&range);
  ioctl(fd, SNIOC_SFIFOTHRESH, nfifos);
  ioctl(fd, SNIOC_ENABLE, 1);

  return 0;
}

static int drop_50msdata(int fd, int samprate)
{
  int cnt = samprate / 20; /* data size of 50ms */

  cnt = ((cnt + MAX_NFIFO - 1) / MAX_NFIFO) * MAX_NFIFO;
  if (cnt == 0) cnt = MAX_NFIFO;

  while (cnt)
    {
      read(fd, g_data, sizeof(g_data[0]) * MAX_NFIFO);
      cnt -= MAX_NFIFO;
    }

  return 0;
}

static int log2uart(cxd5602pwbimu_data_t *dat, int num)
{
  int i;
  for (i = 0; i < num; i++)
    {
      printf("%08lx,%f,%f,%f,%f,%f,%f,%f\n", dat[i].timestamp,
              dat[i].temp, dat[i].gx, dat[i].gy, dat[i].gz,
                           dat[i].ax, dat[i].ay, dat[i].az);
    }

  return 0;
}

static int dump_data(int fd)
{
  int c;
  int ret;

  while (1)
    {
      ret = read(fd, g_data, sizeof(g_data[0]) * MAX_NFIFO);
      if (ret == sizeof(g_data[0]) * MAX_NFIFO)
        {
          log2uart(g_data, MAX_NFIFO);
        }
    }
}

void setup() {
  int devfd;

  board_cxd5602pwbimu_initialize(5);

  devfd = open(CXD5602PWBIMU_DEVPATH, O_RDONLY);

  start_sensing(devfd, 960, 16, 4000, MAX_NFIFO);
  drop_50msdata(devfd, 960);
  dump_data(devfd);
}

void loop() {
}
