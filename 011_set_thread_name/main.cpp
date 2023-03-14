#include <chrono>
#include <iostream>
#include <thread>

/**
 * 主函数
 * 通过 pthread_setname_np 接口设置线程名称
 * 可通过：top -H -p $(pidof 可执行文件名) 查看，会得到如下结果
 * 
 * top - 15:54:10 up  1:53,  2 users,  load average: 0.00, 0.00, 0.00
 * Threads:   4 total,   0 running,   4 sleeping,   0 stopped,   0 zombie
 * %Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
 * MiB Mem :   3916.4 total,   3223.4 free,    309.0 used,    383.9 buff/cache
 * MiB Swap:   8192.0 total,   8192.0 free,      0.0 used.   3386.8 avail Mem 
 *
 * PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                                                                                      
 * 1725 u         20   0   30416   2944   2816 S   0.0   0.1   0:00.00 a.out                                                                                                        
 * 1726 u         20   0   30416   2944   2816 S   0.0   0.1   0:00.00 master                                                                                                       
 * 1727 u         20   0   30416   2944   2816 S   0.0   0.1   0:00.00 slave1                                                                                                       
 * 1728 u         20   0   30416   2944   2816 S   0.0   0.1   0:00.00 slave2                                                                                                       
 *
*/
int main(int argc, char** argv) {
  std::thread master([] {
    while (1) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  std::thread slave1([] {
    while (1)
      std::this_thread::sleep_for(std::chrono::seconds(1));
  });

  std::thread slave2([] {
    while (1)
      std::this_thread::sleep_for(std::chrono::seconds(1));
  });

  pthread_setname_np(master.native_handle(), "master");
  pthread_setname_np(slave1.native_handle(), "slave1");
  pthread_setname_np(slave2.native_handle(), "slave2");

  master.join();
  slave1.join();
  slave2.join();
  return 0;
}