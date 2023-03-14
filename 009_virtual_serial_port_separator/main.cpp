#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <pty.h>
#include <utmp.h>



#define SERIALPORT_RECEIVE_MAX  2048

static char receive_buffer[SERIALPORT_RECEIVE_MAX];
static char waitfor_buffer1[SERIALPORT_RECEIVE_MAX];
static char waitfor_buffer2[SERIALPORT_RECEIVE_MAX];
static int receive_size;

const char *ENV_VAR= "DEBUG";


std::mutex mtx1;
std::condition_variable cv1;
std::mutex mtx2;
std::condition_variable cv2;

int main(int argc, char** argv) {
  int receive_fd;
  int pty_fd1m, pty_fd1s, pty_fd2m, pty_fd2s;

  int speed_arr[] = {B4000000, B3500000, B3000000, B2500000, B2000000, B1500000, B1152000, B1000000, B921600, B460800, B230400, B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
  int name_arr[] = {4000000, 3500000, 3000000, 2500000, 2000000, 1500000, 1152000, 1000000, 921600, 460800, 230400, 115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};

  // 从环境变量中获取数据，比如 DEBUG=1 ./vsps 则env_debug=1
  const char *tmp = std::getenv(ENV_VAR);
  int env_debug = (tmp) ? atoi(tmp) : 0;

  // for(int i=0; i<argc; i++) {
  //   printf("argv[%d]:%s\r\n", i,  argv[i]);
  // }

  if(argc != 1 && argc != 3) {
    printf("The current parameter is incorrect.");
    printf("sample: ./vsps \r\n");
    printf("sample: ./vsps /dev/ttymxc2 115200 \r\n");
    printf("sample: DEBUG=1 ./vsps \r\n");
    return 0;
  }
    

  // 打开串口
  if(argc == 1) {
    receive_fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY);
  } else {
    receive_fd = open(argv[1], O_RDWR | O_NOCTTY);
  }
  
  if (receive_fd < 0) {
    perror("Unable to open the serial port");
    return -1;
  }

  // 设置串口属性
  struct termios options;
  tcgetattr(receive_fd, &options);

  //设置串口输入波特率和输出波特率
  int speed = (argc != 1) ? atoi(argv[2]) : 115200 ;
  for (uint8_t i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
      if (speed == name_arr[i]) {
          cfsetispeed(&options, speed_arr[i]);
          cfsetospeed(&options, speed_arr[i]);
          break;
      }
  }

  //修改控制模式，保证程序不会占用串口, 使得能够从串口中读取输入数据
  options.c_cflag |= CLOCAL | CREAD;
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag &= ~OPOST;
  options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  //不使用流控制
  options.c_cflag &= ~CRTSCTS;

  //设置数据位
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  //无奇偶校验位。
  options.c_cflag &= ~PARENB;
  options.c_iflag &= ~INPCK;

  //修改输出模式，原始数据输出
  options.c_oflag &= ~OPOST;
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  //设置等待时间和最小接收字符
  options.c_cc[VTIME] = 1; 
  options.c_cc[VMIN] = 1; 

  tcflush(receive_fd, TCIFLUSH);
  tcsetattr(receive_fd, TCSANOW, &options);

#define SLAVE_DEV_NAME_MAX_LEN    128

  // 虚拟串口
  char spty1_name[SLAVE_DEV_NAME_MAX_LEN]={0};
  strcpy(spty1_name, "/dev/pts/1");
  int ret  = openpty(&pty_fd1m, &pty_fd1s, spty1_name, &options, NULL);
  if(-1 == ret){
    perror("Failed to get a pty");
    return -1;
  }
  // printf("Get a pty pair, FD -- master[%d], slave[%d]\n", pty_fd1m, pty_fd1s);
  printf("Slave name is %s\n", spty1_name);


  char spty2_name[SLAVE_DEV_NAME_MAX_LEN]={0};
  strcpy(spty2_name, "/dev/pts/2");
  ret  = openpty(&pty_fd2m, &pty_fd2s, spty2_name, &options, NULL);
  if(-1 == ret){
    perror("Failed to get a pty");
    return -1;
  }
  // printf("Get a pty pair, FD -- master[%d], slave[%d]\n", pty_fd2m, pty_fd2s);
  printf("Slave name is %s\n", spty2_name);

  // 添加 select
  std::thread master([=]{

    fd_set rd{-1};
    struct timeval tv;
    int err{-1};

    while(1) {
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      FD_ZERO(&rd);
      FD_SET(receive_fd, &rd);
      err = select(receive_fd + 1, &rd, NULL, NULL, &tv); 
      if(err == 0) { // 超时

      } else if(err == -1) { // 失败

      } else { // 成功

        if (FD_ISSET(receive_fd, &rd)) {

          int bytes_read = read(receive_fd, receive_buffer, sizeof(receive_buffer));
          if (bytes_read < 0) {
            perror("Unable to read from the serial port");
            return -1;
          }

          if(env_debug) {
            std::chrono::microseconds  microsec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            std::cout<<"master, " << microsec.count() << std::endl;
          }

          receive_size = bytes_read;

          {
            std::unique_lock<std::mutex>_lck{mtx1};
            memcpy(waitfor_buffer1, receive_buffer, sizeof(receive_buffer));
            cv1.notify_one();
          }
          
          {
            std::unique_lock<std::mutex>_lck{mtx2};
            memcpy(waitfor_buffer2, receive_buffer, sizeof(receive_buffer));
            cv2.notify_one();
          }
         
          receive_buffer[receive_size] = '\0';
          if(env_debug) {
            printf("%s\r\n", receive_buffer);
          }
          memset(receive_buffer, 0, sizeof(receive_buffer));
        }
      } 
    }
  });

  std::thread slave1([=]{
    
    while(1) {
      {
        std::unique_lock<std::mutex>_lck{mtx1};
        cv1.wait(_lck);
      }

      write(pty_fd1m, waitfor_buffer1, receive_size);
      memset(&waitfor_buffer1, 0, sizeof(waitfor_buffer1));
      if(env_debug) {
        std::chrono::microseconds  microsec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::cout<<"slave1, " << microsec.count() << std::endl;
      }
    }
  });


  std::thread slave2([=]{
    
    while(1) {
      {
        std::unique_lock<std::mutex>_lck{mtx2};
        cv2.wait(_lck);
      }

      write(pty_fd2m, waitfor_buffer2, receive_size);
      memset(&waitfor_buffer2, 0, sizeof(waitfor_buffer2));
      if(env_debug) {
        std::chrono::microseconds  microsec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::cout<<"slave2, " << microsec.count() << std::endl;
      }
    }
  });

  pthread_setname_np(master.native_handle(), "master");
  pthread_setname_np(slave1.native_handle(), "slave1");
  pthread_setname_np(slave2.native_handle(), "slave2");

  master.join();
  slave1.join();
  slave2.join();
  return 0;
}
