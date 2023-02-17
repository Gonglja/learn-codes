#include <stdio.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


class EHeader {
public:
	EHeader(const char* file, uint8_t type) : type(type) {
		if (type == 0) { // 无效

		} else if ( type == 1) { // 32 位
			header = (Elf32_Ehdr *)file;
		} else if ( type == 2) { // 64 位
			header = (Elf64_Ehdr *)file;
		}
	}
	uint8_t getType() {
		return type;
	}

	Elf32_Ehdr *getEhdr32(){
		return (Elf32_Ehdr *)header;
	}

	Elf64_Ehdr *getEhdr64(){
		return (Elf64_Ehdr *)header;
	}

private:
	void *header;
	uint8_t type;
};

char *dataToStr(uint8_t val) {
	switch (val) {
		case 0: 
		case 1: return "2's complement, little endian";
		case 2: return "2's complement, big endian";
		default: return "Invalid data encoding";
	}
}
char *typeToStr(uint16_t val) {
	switch (val) {
		case 0: return "NONE";
		case 1: return "REL";
		case 2: return "EXEC";
		case 3: return "DYN";
		case 4: return "CORE";
		case 0xff00: return "LOPROC";
		case 0xffff: return "HIPROC";
		default: return "";
	}
}

/**
 * 功能：实现类似 readelf -h ./a.out 打印 ELF 文件头部信息
 * 编译：g++ main.cpp
 * 运行：./a.out ./a.out
 */
int main(int argc, char **argv) {
	// Elf64_Ehdr
	char *path = NULL;
	int fd=0;
	if (argc >= 2) {
		path = argv[1];
	}

	if (path) {
		fd = open(path, O_RDONLY);
	}
	printf("%s fd:%d\r\n", path, fd);

	uint8_t buffer[64]; // Elf32 Header 52 bytes; Elf64 Header 64 bytes. 此处最大读取64字节
	ssize_t count = read(fd, buffer, sizeof(buffer));
	printf("count: %ld ", count);
	for (ssize_t i=0; i<count; i++) {
		printf(" %02x", buffer[i]);
	}
	printf("\r\n");
	if (buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F') {
		// 是 elf 文件
		printf("ELF Header:\r\n");
		printf("  Magic:   ");
		for (int i=0; i<16; ++i) {
			if (i == 15) {
				printf("%02x \r\n", buffer[i]);
			} else {
				printf("%02x ", buffer[i]);
			}	
		}
		EHeader header((const char *)buffer, buffer[4]);

		printf("  %-34s %s\r\n", "Class:", header.getType() == 1 ? "ELF32" : "ELF64");
		printf("  %-34s %s\r\n", "Data:", dataToStr(header.getEhdr64()->e_ident[5]));
		printf("  %-34s %d\r\n", "Version:", header.getEhdr64()->e_ident[6]);
		printf("  %-34s %s\r\n", "OS/ABI:");
		printf("  %-34s %s\r\n", "ABI Version:");
		printf("  %-34s %s\r\n", "Type:", typeToStr(header.getEhdr64()->e_type));
		printf("  %-34s %s\r\n", "Machine:");
		printf("  %-34s %s\r\n", "Version:");
		printf("  %-34s %s\r\n", "Entry point address:");
		printf("  %-34s %s\r\n", "Start of program headers:");
		printf("  %-34s %s\r\n", "Start of section headers:");
		printf("  %-34s %s\r\n", "Flags:");
		printf("  %-34s %s\r\n", "Size of this header:");
		printf("  %-34s %s\r\n", "Size of program headers:");
		printf("  %-34s %s\r\n", "Number of program headers:");
		printf("  %-34s %s\r\n", "Size of section headers:");
		printf("  %-34s %s\r\n", "Size of section headers:");
		printf("  %-34s %s\r\n", "Section header string table index:");
		
		
	}
	

	return close(fd);
}


