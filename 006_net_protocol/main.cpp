#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ether.h>

#include "net.h"

union linkdata {
  struct link l;
  unsigned char str[sizeof(struct link)];
};

void printRaw(uint8_t *buffer, size_t sz) {
  printf("\n---------------------- raw -------------------\n");
  for (size_t i = 0; i < sz; i++) {
    printf("%02X ", buffer[i]);
  }
  printf("\r\n");
}

void printLink(struct link *l) {
  static int count = 0;
  printf("\n---------------------- link %d -------------------\n", count++);
  printf("header: %02x:%02x:%02x:%02x:%02x:%02x >> %02x:%02x:%02x:%02x:%02x:%02x type:%d\n",
         l->header.dst_mac[0],
         l->header.dst_mac[1],
         l->header.dst_mac[2],
         l->header.dst_mac[3],
         l->header.dst_mac[4],
         l->header.dst_mac[5],
         l->header.src_mac[0],
         l->header.src_mac[1],
         l->header.src_mac[2],
         l->header.src_mac[3],
         l->header.src_mac[4],
         l->header.src_mac[5],
         l->header.type);
}

void printIP(struct ip *i) {
  printf("\n---------------------- ip -------------------\n");
  printf("header: version:%d len:%d tos:%x total_len:%d\n",
         i->header.version,
         i->header.header_length,
         i->header.typeofservice,
         ntohs(i->header.total_length));
  uint32_t src_ip = ntohl(i->header.src_ip);
  uint32_t dst_ip = ntohl(i->header.dst_ip);
  printf("        src_ip:%d.%d.%d.%d dst_ip:%d.%d.%d.%d\n",
         (src_ip & 0xff000000) >> 24,
         (src_ip & 0xff0000) >> 16,
         (src_ip & 0xff00) >> 8,
         (src_ip & 0xff),
         (dst_ip & 0xff000000) >> 24,
         (dst_ip & 0xff0000) >> 16,
         (dst_ip & 0xff00) >> 8,
         (dst_ip & 0xff)
  );
  printf("        protocol:%d\n", i->header.protocol);
}

void printICMP() {

}

void printTCP() {

}

void printUDP(struct udp *u) {
  printf("\n---------------------- udp -------------------\n");
  printf("header: src_port:%d dst_port:%d length:%d crc:%x\n",
         ntohs(u->header.src_port),
         ntohs(u->header.dst_port),
         ntohs(u->header.length),
         ntohs(u->header.crc));
  printf("data: ");
  for (int i = 0; i < ntohs(u->header.length); i++) {
    printf("%02X ", (uint8_t) u->data.data[i]);
  }
  printf("\n");
}

bool isLittleEndian() {
  // little endian: low byte -- low address
  int a = 0x12345678;
  return (char) (*(char *) &a) == 0x78;
}

int main(int argc, char *argv[]) {
  printf("isLittleEndian:%d\r\n", isLittleEndian());

  //创建链路层原始套接字
  int sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

  while (1) {
    linkdata ld;
    memset(ld.str, '\0', sizeof(linkdata));
    //获取链路层的数据帧
    ssize_t recvlen = recvfrom(sock_raw_fd, ld.str, sizeof(ld), 0, NULL, NULL);
    if (recvlen != -1) {
      // 解析原始数据
      printRaw(ld.str, recvlen);

      // 解析数据链路层数据
      printLink(&ld.l);

      // 解析网络层协议
      printIP(&ld.l.data);

      // 解析传输层 -- 1 ICMP; 6 TCP; 17 UDP
      uint8_t protocol = ld.l.data.header.protocol;
      switch (protocol) {
        case 1: // 解析 ICMP 协议
          break;

        case 6: // 解析 TCP 协议
          break;

        case 17: // UDP
          printUDP((struct udp *) &ld.l.data.data);
          break;
      }
    }
  }
  return 0;
}