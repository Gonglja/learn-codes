/***************************************************************************
* COPYRIGHT NOTICE
* Copyright © 2022 
* All Rights Reserved.
*
* @file     net.h
* @brief    概述
*
* 详细概述
*
* @author   Zero
* @date     22/12/23 10:10
* @version  1.0.0
*
* Remark         : Description
*----------------------------------------------------------------------------
* Change History :
* <Date>     | <Version> | <Author>       | <Description>
*----------------------------------------------------------------------------
* 2022/10/07 | 1.0.0     |  Zero          | Create file
*----------------------------------------------------------------------------

******************************************************************************/

#ifndef LEARN_NET__NET_H_
#define LEARN_NET__NET_H_

#include <stdio.h>
#include <stdint.h>

/**
 *
 *  TCP/UDP 结构
 *                                             +--------------+
 *                                             |   data       |
 *                            +----------------+--------------+
 *                            | tcp/udp header | tcp/udp data |
 *                +-----------+-------------------------------+
 *                | ip header |         ip  data              |
 *  +-------------+-------------------------------------------+
 *  | link header |        link data                          |
 *  +-------------+-------------------------------------------+
 *
*/



// ubuntu x86_64 little_endian

#pragma pack(1)

struct udp_header {           // 共 8字节
  uint16_t src_port;        // 16位源端口
  uint16_t dst_port;        // 16位目的端口
  uint16_t length;          // 16位数据报的长度，包含数据和首部，最小为 8字节
  uint16_t crc;             // 16位校验和
};

struct udp_data {
  char data[1024];
};

struct udp {
  struct udp_header header; // 8 字节
  struct udp_data data;
};

struct tcp_header {
  uint16_t src_port;      // 16
  uint16_t dst_port;      // 16
  uint32_t seq_no;           //
  uint32_t ack_no;           //
#if defined(LITTLE_ENDIAN)
  uint16_t header_length: 4;//
  uint16_t reserve: 6;    //
  uint16_t flags: 6;      //
#elif define(BIG_ENDIAN)
  uint16_t flags:6;
  uint16_t reserve: 6;    //
  uint16_t header_length: 4;//
#endif
  uint16_t win;            //
  uint16_t crc;            //
  uint16_t urm;            //
};
struct tcp_data {
};

struct tcp {
  struct tcp_header header;
  struct tcp_data data;
};

struct ip_header {          // 最小20字节
#if defined(LITTLE_ENDIAN)
  uint8_t header_length: 4; // 4位头部长度
  uint8_t version: 4;       // 4位版本号
#elif defined(BIG_ENDIAN)
  uint8_t version: 4;
  uint8_t header_length: 4;
#endif
  uint8_t typeofservice;    // 8位服务类型
  uint16_t total_length;    // 16位总长度       -- 4字节

  uint16_t id;              // 16位标识
  uint16_t flags: 3;        // 3位标志位
  uint16_t offset: 13;      // 13位片偏移       -- 4字节

  uint8_t ttl;              // 8位生存时间
  uint8_t protocol;         // 8位 上层协议类型
  uint16_t header_crc;      // 16位 头部校验和  -- 4字节

  uint32_t src_ip;          // 32位源ip地址   -- 4字节
  uint32_t dst_ip;          // 32位目的ip地址 -- 4字节

  // 可选项
};

union ip_data {
  struct udp udp_data;
  struct tcp tcp_data;
};

struct ip {                 // ip 数据报
  struct ip_header header;  // 20 字节
  ip_data data;
};

struct link_header {        // 共 14 字节
  uint8_t dst_mac[6];       // 6字节目的 mac 地址
  uint8_t src_mac[6];       // 6字节源 mac 地址
  uint16_t type;            // 2字节 类型
};

struct link_footer {
  uint8_t crc[4];           // 4字节crc校验和
};

typedef ip link_data;

struct link {
  struct link_header header; // 14 字节
  link_data data;            // 46-1500 字节
//  struct link_footer footer; // 4 字节
};

#pragma pack()

#endif //LEARN_NET__NET_H_
